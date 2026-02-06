#include "malfunction.h"
#include <algorithm>
#include <atomic>
#include <chrono>
#include <cstddef>
#include <game.h>

#include <memory>
#include <mutex>
#include <random>
#include <sstream>
#include <string>
#include <termios.h>

#include <boost/format.hpp>

#include <consolemanager.h>
#include <devices/deviceregistry.h>
#include <utils.h>

Game::Game(const std::vector<std::string>& deviceNames) : currentLevel(0), totalScore(0), maxScore(0) {

    for (const auto& name : deviceNames) {
        devices.push_back(DeviceRegistry::create(name));
    }

    {
        std::lock_guard<std::mutex> lock(ConsoleManager::getMutex());
        ConsoleManager::clearScreen();
        ConsoleManager::printMenu();
    }

    penguin_ = std::make_shared<Penguin>();
    observerManager_.addObserver(penguin_);    

    ConsoleManager::setDeviceCount(devices.size());
}


Game::~Game() {
    // exitGame();

    // if (inputThread.joinable()) {
    //     inputThread.join();
    // }
    // // Восстанавливаем видимость курсора
    // std::cout << "\033[?25h\033[0m" << std::flush;
}


void Game::startGame() {
    //showMainMenu();
    running.store(true, std::memory_order_release);
        
    inputThread = std::thread(&Game::inputLoop, this);
    mainThread = std::thread([this] () { mainGameLoop(); });
    deviceThread = std::thread([this]() { deviceUpdateLoop(); });

    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    startCV.notify_all();

    // Отсоединяем потоки - они завершатся самостоятельно
    inputThread.detach();
    mainThread.detach();
    deviceThread.detach();

    // Приглашение для ввода
    std::lock_guard<std::mutex> lock(ConsoleManager::getMutex());
    ConsoleManager::clearInputLine();
    ConsoleManager::showPrompt();   
}


void Game::exitGame() {  
    running.store(false, std::memory_order_release);
    startCV.notify_all();

    std::lock_guard<std::mutex> lock(ConsoleManager::getMutex());
    ConsoleManager::gotoExitLine();
}


bool Game::isRunning() const {
    return running.load(std::memory_order_acquire);
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
    newt.c_cc[VMIN] = 0;    //минимальное кол-во символов
    newt.c_cc[VTIME] = 0;   //таймаут 0
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);

    std::string line;
            
    // считывать символы по одному
    while (running.load(std::memory_order_acquire)) {

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

                std::lock_guard<std::mutex> lock(ConsoleManager::getMutex());
                ConsoleManager::clearInputLine();
                ConsoleManager::showPrompt();
                ConsoleManager::gotoInputLine();
            }
        }
        else if (ch == 127 || ch == 8) {  // Backspace
            if (!line.empty()) {
                line.pop_back();
                std::lock_guard<std::mutex> lock(ConsoleManager::getMutex());
                ConsoleManager::print("\b \b");
            }
        }
        else if (ch >= 32 && ch <= 126) {  // Печатные символы
            line += ch;
            std::lock_guard<std::mutex> lock(ConsoleManager::getMutex());
            ConsoleManager::print(ch);
        }
        // Игнорируем остальные символы

        std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }
    
    // Восстанавливаем настройки
    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
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

    while (running.load(std::memory_order_acquire)) {
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
    if (command == "start") {
        totalScore = 0;
        maxScore = 0;
        currentLevel = 1;

        currentState = State::PLAYING;
        std::lock_guard<std::mutex> lock(ConsoleManager::getMutex());
        ConsoleManager::savePosition();
        ConsoleManager::printLevel(1, 0, maxScore);    
        ConsoleManager::restorePosition();   
    }
    else if (command == "exit") {
        exitGame();
    }
    else if (command == "stop") {
        observerManager_.notifyMessage("Введи 'start' для начала игры");
    }
    else {
        observerManager_.notifyMessage("Неизвестная команда. Введи 'start' для начала игры");
    }
}


void Game::handleGameCommand(const std::string& command) {
    if (command == "menu") {
        currentState = State::MENU;
        //showMainMenu();
    }
    else if (command == "exit") {
        exitGame();
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
    std::lock_guard<std::mutex> lock(ConsoleManager::getMutex());
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
    {
        std::lock_guard<std::mutex> lock(ConsoleManager::getMutex());
        ConsoleManager::savePosition();
        ConsoleManager::printLevel(currentLevel, totalScore, maxScore);
        ConsoleManager::restorePosition();
    }

    int tasks = 3;
    int problemsAtOnce = level;

    for (int task = 0; task < tasks && currentState == State::PLAYING && running.load(); task++) {

        // Генерируем проблемы
        std::vector<CurrentTask> currentTasks = generateProblemsWithSolutions(problemsAtOnce);

        observerManager_.notifyMood("neutral");
        std::string msg = "Задача " + std::to_string(task + 1) + "/" + std::to_string(tasks);
        if (currentLevel == 1 ) {
            observerManager_.notifyMessage(msg + ". Неисправен 1 прибор. Введи номер прибора");
        } else {
            observerManager_.notifyMessage(msg + ". Неисправено " + std::to_string(currentLevel) 
                + " приборa. Введи номер прибора");
        }

        ConsoleManager::hideCursor();
        if (running) std::this_thread::sleep_for(std::chrono::milliseconds(2500));
        ConsoleManager::showCursor();

        // Обработка задач по одной, пока все не будут исправлены
        while(!currentTasks.empty() && running && currentState == State::PLAYING) {
            int selectedIndex;
            if (!askToSelectDevice(selectedIndex)) {
                return;
            }

            auto it = std::find_if(currentTasks.begin(), currentTasks.end(), 
                [selectedIndex](const CurrentTask& t) {
                return t.deviceIndex == selectedIndex;
            });

            if (it != currentTasks.end()) {
                CurrentTask& task = *it;

                // Показать неисправность и варианты решений
                showProblemAndSolutions(task);

                // Получить выбор пользователя
                if (!getUserSolutionChoice(task)) {
                    return;
                }

                // Проверить и начислить очки
                checkAndScore(task);

                currentTasks.erase(it);

                // Убрать неисправность из прибора
                devices[selectedIndex]->clearMalfunctions();

                if (!currentTasks.empty()) {
                    observerManager_.notifyMood("neutral");
                    observerManager_.notifyMessage("Хорошо! Но остались другие неисправные приборы. Введи номер прибора");
                    ConsoleManager::hideCursor();
                    std::this_thread::sleep_for(std::chrono::milliseconds(1500));
                    ConsoleManager::showCursor();
                }               
            } else {
                // Этот прибор не был в списке неисправных
                observerManager_.notifyMood("sad");
                observerManager_.notifyMessage("Этот прибор исправен! Попробуй другой.");
                ConsoleManager::hideCursor();
                std::this_thread::sleep_for(std::chrono::milliseconds(1500));
                ConsoleManager::showCursor();
            }
        }

        // Пауза между задачами
        ConsoleManager::hideCursor();
        if (running) std::this_thread::sleep_for(std::chrono::milliseconds(1000));
        ConsoleManager::showCursor();
    }

    // Завершение уровня
    completeLevel();
}


std::vector<CurrentTask> Game::generateProblemsWithSolutions(int count) {
    std::vector<CurrentTask> tasks;

    // Создание списка индексов приборов
    std::vector<int> deviceInd;
    for (std::size_t i = 0; i < devices.size(); ++i) {
        deviceInd.push_back(i);
    }

    // Перемешивание индексов
    std::shuffle(deviceInd.begin(), deviceInd.end(), std::mt19937(std::random_device{}()));


    for (int i = 0; i < std::min(count, (int)deviceInd.size()); i++) {
        // Выбор случайного прибора
        int deviceIndex = deviceInd[i];
        Device* device = devices[deviceIndex].get();

        auto possibleMalfunctions = device->getMalfunctions();
        if(!possibleMalfunctions.empty()) {
            int malfunctionIndex = rand() % possibleMalfunctions.size();
            Malfunction selectedMalfunction = possibleMalfunctions[malfunctionIndex];

            {
                std::lock_guard<std::mutex> lock(ConsoleManager::getMutex());
                ConsoleManager::printDebug("deviceIndex = " + std::to_string(deviceIndex) + " malfunctionIndex=" +
                std::to_string(malfunctionIndex));
            }

            // Установка неисправность в прибор
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


bool Game::askToSelectDevice(int& selectedIndex) {
    auto lastStatusUpdate = std::chrono::steady_clock::now();

    while(running && currentState == State::PLAYING) {
        // Обновление статуса устройств раз в секунду
        updateDeviceStatusWithTimer(lastStatusUpdate);

        std::string input;
        if (!getCommand(input)) continue;

        if (input == "stop") {
            clearDeviceMalfunctions();
            std::string msg = "Принудительное завершение игры. Счет " + std::to_string(totalScore) 
                + "/" + std::to_string(maxScore);
            stopGame("sad", msg);
            return false;
        }
        if (input == "exit") {
            exitGame();
            return false;
        }

        int num;
        if(std::istringstream(input) >> num) {
            if (num >= 1 && num <= devices.size()) {
                selectedIndex = num - 1;
                return true;
            } else {
                observerManager_.notifyMessage("Введи номер от 1 до " + std::to_string(devices.size()));
            }
        }
    }
    return false;
}


void Game::showProblemAndSolutions(const CurrentTask& task) {   
    Device* device = devices[task.deviceIndex].get();

    std::string output;
    for (size_t i = 0; i < task.shuffledSolutions.size(); i++) {
        output += std::to_string(i+1) + ". " + task.shuffledSolutions[i].description + "\n";
    }

    observerManager_.notifyMood("neutral");
    std::string msg = "Прибор: " + device->getName() + ". Неисправность: " + task.malfunction.name;

    // Показываем варианты  
    {
        std::lock_guard<std::mutex> lock(ConsoleManager::getMutex());
        ConsoleManager::clearActionArea();  
        ConsoleManager::savePosition();
        ConsoleManager::gotoActionLine();
        ConsoleManager::print(output);
        ConsoleManager::restorePosition();  
    } 

    observerManager_.notifyMessage(msg + ". Выбери номер действия (1-" + 
        std::to_string(task.malfunction.solutions.size())+ "):");   
}


bool Game::getUserSolutionChoice(CurrentTask& task) {
    auto lastStatusUpdate = std::chrono::steady_clock::now();

    while (running && currentState == State::PLAYING) {
        // Обновление статуса устройств раз в секунду
        updateDeviceStatusWithTimer(lastStatusUpdate);

        std::string input;
        if (!getCommand(input)) continue;
        
        if (input == "stop") {
            clearDeviceMalfunctions();
            std::string msg = "Принудительное завершение игры. Счет " + std::to_string(totalScore) 
                + "/" + std::to_string(maxScore);
            stopGame("sad", msg);
            return false;
        }
        if (input == "exit") {
            exitGame();
            return false;
        }

        int choice;
        if (std::istringstream(input) >> choice) {
            if (choice >= 1 && choice <= task.shuffledSolutions.size()) {
                task.selectedSolutionIndex = choice - 1;
                return true;
            }
            observerManager_.notifyMessage("Введи номер от 1 до " + std::to_string(task.shuffledSolutions.size()));
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
        observerManager_.notifyMood("happy");
    } else if (points >= 60) {
        observerManager_.notifyMood("neutral");
    } else if (points >= 20) {
        observerManager_.notifyMood("sad");
    } else {
        observerManager_.notifyMood("angry");
    }
    observerManager_.notifyMessage(chosen.comment);

    updateScore(points);

    {
        std::lock_guard<std::mutex> lock(ConsoleManager::getMutex());
        ConsoleManager::savePosition();
        ConsoleManager::printLevel(currentLevel, totalScore, maxScore);

        // Очищаем экран с вариантами
        ConsoleManager::clearActionArea();
        ConsoleManager::restorePosition();
    }
    
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
        observerManager_.notifyMood("happy");
        observerManager_.notifyMessage("Уровень " + std::to_string(currentLevel) + " завершен! Счет: " + 
                    std::to_string(totalScore));

        ConsoleManager::hideCursor();
        std::this_thread::sleep_for(std::chrono::seconds(3));
        ConsoleManager::showCursor();

        if (currentLevel == 3) {
            std::string msg = "Игра завершена! Твоя кваллификация: " + getQualification();
            stopGame("happy", msg);
            return;
        }

        if ((double)totalScore/maxScore > 0.5) {
            currentLevel++;
        } else {
            std::string msg = "Игра завершена. Ты не набрал достаточно очков для перехода на следующий уровень";
            stopGame("sad", msg);
        }
    }
}


void Game::showDevicesStatus() {   
    std::lock_guard<std::mutex> lock(ConsoleManager::getMutex()); 
    // Сохранение позиции курсора
    ConsoleManager::savePosition();

    // Очистка области приборов
    ConsoleManager::clearDeviceLine();

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


void Game::clearDeviceMalfunctions() {
    for (auto& device : devices) {
        device->clearMalfunctions();
    }
}


std::string Game::getQualification() const {
    if(totalScore >= 1600) return "expert";
    if(totalScore >= 1400) return "specialist";
    if(totalScore >= 1200) return "operator";
    if(totalScore >= 1000) return "improver";
    return "student";
}


void Game::updateScore(int points) {
    totalScore += points;
    maxScore += 100;
}

void Game::stopGame(const std::string& mood, const std::string& msg) {
    currentState = State::MENU;
    clearDeviceMalfunctions();

    observerManager_.notifyMood(mood);
    observerManager_.notifyMessage(msg);
    
    std::lock_guard<std::mutex> lock(ConsoleManager::getMutex());
    ConsoleManager::savePosition();
    ConsoleManager::clearLevelLine();
    ConsoleManager::clearActionArea();
    ConsoleManager::restorePosition();
}