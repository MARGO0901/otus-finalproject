#include "malfunction.h"
#include <chrono>
#include <game.h>

#include <random>
#include <sstream>
#include <string>
#include <termios.h>

#include <boost/format.hpp>

#include <consolemanager.h>
#include <devices/deviceregistry.h>
#include <utils.h>

static std::mutex outputMutex;

Game::Game(const std::vector<std::string>& deviceNames) : currentLevel(1), totalScore(0) {

    for (const auto& name : deviceNames) {
        devices.push_back(DeviceRegistry::create(name));
    }

    ConsoleManager::setDeviceCount(devices.size());
}


Game::~Game() {
    stop();

    if (inputThread.joinable()) {
        inputThread.join();
    }
    // Восстанавливаем видимость курсора
    std::cout << "\033[?25h\033[0m" << std::flush;
}


void Game::start() {
    //showMainMenu();
    running.store(true, std::memory_order_release);
        
    inputThread = std::thread(&Game::inputLoop, this);
    mainThread = std::thread([this] () { mainGameLoop(); });
    deviceThread = std::thread([this]() { deviceUpdateLoop(); });

    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    startCV.notify_all();

    // Приглашение для ввода
    ConsoleManager::clearInputLine();
    ConsoleManager::showPrompt();   
}


void Game::stop() {
    running.store(false, std::memory_order_release);
    startCV.notify_all();

    if (inputThread.joinable()) inputThread.join();
    if (deviceThread.joinable()) deviceThread.join();
    if (mainThread.joinable()) mainThread.join();
}


void Game::inputLoop() {

    // Ждем сигнала старта
    {
        std::unique_lock<std::mutex> lock(startMutex);

        startCV.wait(lock, [this]() { 
            return running.load(std::memory_order_acquire); 
        });
    }

    // сохранить настройки терминала    
    struct termios oldt, newt;
    tcgetattr(STDIN_FILENO, &oldt);
    newt = oldt;

    // установить неканонический режим
    newt.c_lflag &= ~(ICANON | ECHO);  
    newt.c_cc[VMIN] = 1;    //минимальное кол-во символов
    newt.c_cc[VTIME] = 0;   //таймаут 0
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);

    std::string line;
            
    // считывать символы по одному
    while (this->running.load(std::memory_order_acquire)) {

        char ch;

        // Пытаемся прочитать символ
        int bytesRead = read(STDIN_FILENO, &ch, 1);
        if (bytesRead <= 0) {
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
            continue;
        }

        // Обработка символа
        if (ch == '\n' || ch == '\r') {  // Enter
            if (!line.empty()) {
                {
                    std::lock_guard<std::mutex> lock(inputMutex);
                    inputBuffer = line;
                }
                line.clear();

                ConsoleManager::clearInputLine();
                ConsoleManager::showPrompt();
                ConsoleManager::gotoInputLine();
            }
        }
        else if (ch == 127 || ch == 8) {  // Backspace
            if (!line.empty()) {
                line.pop_back();
                ConsoleManager::print("\b \b");
            }
        }
        else if (ch >= 32 && ch <= 126) {  // Печатные символы
            line += ch;
            ConsoleManager::print(ch);
        }
        // Игнорируем остальные символы
    }
    
    // Восстанавливаем настройки
    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
    // Восстанавливаем видимость курсора
    ConsoleManager::showCursor();
}


void Game::mainGameLoop() {
    // Ожидание сигнала старта
    {
        std::unique_lock<std::mutex> lock(startMutex);
        startCV.wait(lock, [this]() { 
            return running.load(std::memory_order_acquire); 
        });
    }
    
    while(running.load(std::memory_order_acquire)) {
        std::string cmd;
        if(getCommand(cmd)) {
            processCommand(cmd);
        }

        if(needsRedrawDevice) {
            showDevicesStatus();
            needsRedrawDevice = 0;
        }

        if(currentState == State::PLAYING) {
            static bool levelInProgress = false;

            if(!levelInProgress) {
                levelInProgress = true;
                runLevelInLoop(currentLevel);
                levelInProgress = false;

                currentState = State::MENU;
                //showMainMenu();
            }
        }
            
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }
}


void Game::deviceUpdateLoop() {
    // Ждем запуска
    {
        std::unique_lock<std::mutex> lock(startMutex);
        startCV.wait(lock, [this]() { return running.load(); });
    }

    while (running.load()) {
        {
            std::lock_guard<std::mutex> lock(deviceMutex);
            std::for_each(devices.begin(), devices.end(),[] (auto& device) {
                device->update();
            });
        }

        needsRedrawDevice = true;
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    }
}


void Game::processCommand(std::string &command) {
    switch (currentState) {
        case State::MENU:
            handleMenuCommand(command);
            break;
        case State::PLAYING:
            //handleGameCommand(command);
            break;
        case State::GAME_OVER:
            // Команды после игры
            if (command == "menu") {
                currentState = State::MENU;
                //showMainMenu();
            }
            break;
        // ... другие состояния
    }
}


void Game::handleMenuCommand(const std::string& command) {
    if (command == "start" || command == "1") {
        currentState = State::PLAYING;
        ConsoleManager::savePosition();
        ConsoleManager::printLevel(1, 0);    
        ConsoleManager::restorePosition();   
    }
    else if (command == "exit" || command == "quit") {
        running.store(false);
    }
    else if (command == "help") {
        penguin.say("Доступные команды: start, exit");
    }
    else {
        penguin.say("Неизвестная команда. Введи 'start' для начала игры");
    }
}


void Game::handleGameCommand(const std::string& command) {
    if (command == "menu") {
        currentState = State::MENU;
        //showMainMenu();
    }
    else if (command == "exit") {
        running.store(false);
    }
    else {
        // Обработка игрового ввода
        std::lock_guard<std::mutex> lock(inputMutex);
        inputBuffer = command;
    }
}


// Неблокирующее получение команды
bool Game::getCommand(std::string& cmd) {
    std::lock_guard<std::mutex> lock(inputMutex);
    if (!inputBuffer.empty()) {
        cmd = inputBuffer;
        inputBuffer.clear();
        return true;
    }
    return false;
}


/*
// === ГЛАВНОЕ МЕНЮ ===
void Game::showMainMenu() {
    ConsoleManager::clearScreen();
    
    // Рисуем пингвина
    penguin.draw();
    
    // Меню
    ConsoleManager::gotoxy(0, 6);
    std::cout << "========== ГЛАВНОЕ МЕНЮ ==========\n";
    std::cout << "1. Начать игру (start)\n";
    std::cout << "2. Помощь (help)\n";
    std::cout << "3. Выход (exit)\n";
    std::cout << "==================================\n";
    
    if (totalScore > 0) {
        std::cout << "\nТекущий счет: " << totalScore << "\n";
    }
    
    // Приглашение для ввода
    ConsoleManager::clearInputLine(11);
    ConsoleManager::showPrompt(11);
    ConsoleManager::gotoxy(2, 11);
}*/


void Game::runLevelInLoop(int level) {
    int tasks = (level == 1) ? 3 : (level == 2) ? 6 : 9;
    int problemsAtOnce = level;

    for (int task = 0; task < tasks && currentState == State::PLAYING && running.load(); task++) {

        // Генерируем проблемы
        std::vector<CurrentTask> currentTasks = generateProblemsWithSolutions(problemsAtOnce);

        penguin.setMood("neutral");
        std::string msg = "Задача " + std::to_string(task + 1) + "/" + std::to_string(tasks);
        penguin.say(msg + ". Введи номер устройства для починки!");

        for (auto& task : currentTasks) {
            if (!processSingleTask(task)) {
                return;
            }
        }

        // Пауза между задачами
        if (running) std::this_thread::sleep_for(std::chrono::seconds(1));
    }

    // Завершение уровня
    completeLevel();
}


std::vector<CurrentTask> Game::generateProblemsWithSolutions(int count) {
    std::vector<CurrentTask> tasks;

    for (int i = 0; i < count; i++) {
        // Выбираем случайный прибор
        int deviceIndex = rand() % devices.size();
        Device* device = devices[deviceIndex].get();

        auto possibleMalfunctions = device->getMalfunctions();
        if(!possibleMalfunctions.empty()) {
            int malfunctionIndex = rand() % possibleMalfunctions.size();
            Malfunction selectedMalfunction = possibleMalfunctions[malfunctionIndex];

            ConsoleManager::printDebug("deviceIndex = " + std::to_string(deviceIndex) + " malfunctionIndex=" +
                std::to_string(malfunctionIndex));

            // Установить неисправность в прибор
            device->addMalfunctions(selectedMalfunction);  
            
            // Создание задачи с перемешанными решениями
            CurrentTask task;
            task.deviceIndex = deviceIndex;
            task.malfunction = selectedMalfunction;

            auto solution = selectedMalfunction.solutions;
            std::shuffle(solution.begin(), solution.end(), std::mt19937{std::random_device{}()});
            task.shuffledSolutions = solution;

            tasks.push_back(task);
        }
    }

    return tasks;
}


bool Game::processSingleTask(CurrentTask& task) {
    // 1. Показать приборы и просит выбрать
    if (!askToSelectDevice(task.deviceIndex)) {
        return false;
    }

    // 2. Показать неисправность и варианты решений
    showProblemAndSolutions(task);

    // 3. Получить выбор пользователя
    if (!getUserSolutionChoice(task)) {
        return false;
    }

    // 4. Проверить и начислить очки
    checkAndScore(task);

    return true;
}


bool Game::askToSelectDevice(int& selectedIndex) {
    auto lastStatusUpdate = std::chrono::steady_clock::now();

    while(running && currentState == State::PLAYING) {
        // Обновление статуса устройств раз в секунду
        updateDeviceStatusWithTimer(lastStatusUpdate);

        std::string input;
        if (!getCommand(input)) continue;

        if (input == "menu") {
            currentState = State::MENU;
            return false;
        }
        if (input == "exit") {
            running = false;
            return false;
        }

        int num;
        if(std::istringstream(input) >> num) {
            if (num >=1 && num <= devices.size()) {
                if (!devices[num - 1]->getActiveMalfunctions().empty()) {
                    selectedIndex = num - 1;        // здесь меняется task.deviceIndex
                    return true;
                }
            } else {
                penguin.setMood("sad");
                penguin.say("Этот прибор исправен! Попробуй другой.");
            }
        }
        penguin.say("Введи номер от 1 до " + std::to_string(devices.size()));
    }
    return false;
}


void Game::showProblemAndSolutions(const CurrentTask& task) {
    Device* device = devices[task.deviceIndex].get();

    penguin.setMood("neutral");
    std::string msg = "Прибор: " + device->getName() + ". Неисправность: " + task.malfunction.name;

    // Показываем варианты
    ConsoleManager::savePosition();
    ConsoleManager::gotoActionLine();
    
    for (size_t i = 0; i < task.shuffledSolutions.size(); i++) {
        ConsoleManager::print(std::to_string(i+1) + ". " + 
                            task.shuffledSolutions[i].description + "\n");
    }
    
    ConsoleManager::restorePosition();
    penguin.say(msg + ". Выбери номер действия (1-" + std::to_string(task.malfunction.solutions.size())+ "):");
}


bool Game::getUserSolutionChoice(CurrentTask& task) {
    auto lastStatusUpdate = std::chrono::steady_clock::now();

    while (running && currentState == State::PLAYING) {
        // Обновление статуса устройств раз в секунду
        updateDeviceStatusWithTimer(lastStatusUpdate);

        std::string input;
        if (!getCommand(input)) continue;
        
        if (input == "menu") {
            currentState = State::MENU;
            return false;
        }
        if (input == "exit") {
            running = false;
            return false;
        }

        int choice;
        if (std::istringstream(input) >> choice) {
            if (choice >= 1 && choice <= task.shuffledSolutions.size()) {
                task.selectedSolutionIndex = choice - 1;
                return true;
            }
            penguin.say("Введи номер от 1 до " + std::to_string(task.shuffledSolutions.size()));
        }
    }
    return false;
}


void Game::checkAndScore(CurrentTask& task) {
    if (task.selectedSolutionIndex < 0 || task.selectedSolutionIndex >= task.shuffledSolutions.size()) {
        return;
    }

    const Solution& chosen = task.shuffledSolutions[task.selectedSolutionIndex];
    int points = chosen.score;
    task.correctPoints = points;

    // Реакция в зависимости от очков
    if (points == 100) {
        penguin.setMood("happy");
    } else if (points >= 60) {
        penguin.setMood("neutral");
    } else if (points >= 20) {
        penguin.setMood("sad");
    } else {
        penguin.setMood("angry");
    }
    penguin.say(chosen.comment);

    updateScore(points);
    ConsoleManager::savePosition();
    ConsoleManager::printLevel(currentLevel, totalScore);
    ConsoleManager::restorePosition();

    // Очищаем экран с вариантами
    ConsoleManager::savePosition();
    ConsoleManager::clearActionArea();
    ConsoleManager::restorePosition();

    // Снимаем неисправность с прибора
    devices[task.deviceIndex]->clearMalfunctions();
    
    // Пауза для чтения
    std::this_thread::sleep_for(std::chrono::milliseconds(1500));
}


void Game::updateDeviceStatusWithTimer(std::chrono::steady_clock::time_point& lastUpdate) {
    auto now = std::chrono::steady_clock::now();
    if (std::chrono::duration_cast<std::chrono::seconds>(now - lastUpdate).count() >= 1) {
        showDevicesStatus();
        lastUpdate = now;
    }
}


void Game::completeLevel() {
    if (currentState == State::PLAYING && running.load()) {
        penguin.setMood("happy");
        penguin.say("Уровень " + std::to_string(currentLevel) + " завершен! Счет: " + 
                    std::to_string(totalScore));

        ConsoleManager::savePosition();
        ConsoleManager::printLevel(currentLevel + 1, totalScore, true);
        ConsoleManager::restorePosition();

        std::this_thread::sleep_for(std::chrono::seconds(3));
        
        // Возвращаемся в меню
        //currentState = State::MENU;
        currentLevel++;
        //showMainMenu();
    }
}


void Game::showDevicesStatus() {    
    // Сохранение позиции курсора
    ConsoleManager::savePosition();

    // Очистка области приборов
    //ConsoleManager::clearDeviceLine();

    // Показать приборы
    ConsoleManager::gotoDeviceLine();
    std::string output;
    output += "=== Устройства ===\n";

    int number = 1;
    for (auto &device : devices) {

        // Форматируем имя устройства с фиксированной шириной
        std::string deviceLine = std::to_string(number) + ". " 
                               + device->getName();
        
        // Выравнивание
        while (deviceLine.length() < 12) {
            deviceLine += " ";
        }
        deviceLine += ":\t";

        auto params = device->getParams();
        for(auto it = params.begin(); it != params.end(); ++it) {
            auto& [param, value] = *it;
            std::string val_str = variantToString(value);
            // Создание строки
            std::string param_str = (boost::format("%s(%.0f..%.0f) = %s") 
                % param.name_ 
                % param.optRange_.first 
                % param.optRange_.second 
                % val_str)
                .str();
            // Форматирование ширины
            // Если это не последний параметр - добавляем отступ
            if (std::next(it) != params.end()) {
                param_str = (boost::format("%-32s") % param_str).str();
            }
            deviceLine += param_str;
        }
        output += deviceLine + "\n";
        number++;
    }
    ConsoleManager::print(output);
    // Восстанавление позиции курсора
    ConsoleManager::restorePosition();
} 


std::string Game::getQualification() const {
    if(totalScore >= 1500) return "expert";
    if(totalScore >= 1200) return "specialist";
    if(totalScore >= 900) return "operator";
    if(totalScore >= 600) return "improver";
    return "student";
}


void Game::updateScore(int points) {
    totalScore += points;
}