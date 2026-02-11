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

Game::Game(const std::vector<std::string>& deviceNames) : currentLevel_(0), totalScore_(0), maxScore_(0) {

    for (const auto& name : deviceNames) {
        devices_.push_back(DeviceRegistry::create(name));
    }

    {
        std::lock_guard<std::mutex> lock(ConsoleManager::getMutex());
        ConsoleManager::clearScreen();
        ConsoleManager::printMenu();
    }

    penguin_ = std::make_shared<Penguin>();
    observerManager_.addObserver(penguin_);    

    ConsoleManager::setDeviceCount(devices_.size());
}


void Game::startGame() {
    running_.store(true, std::memory_order_release);
        
    inputThread_ = std::thread(&Game::inputLoop, this);
    mainThread_ = std::thread([this] () { mainGameLoop(); });
    deviceThread_ = std::thread([this]() { deviceUpdateLoop(); });

    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    startCV_.notify_all();

    // Отсоединяем потоки - они завершатся самостоятельно
    inputThread_.detach();
    mainThread_.detach();
    deviceThread_.detach();

    // Приглашение для ввода
    std::lock_guard<std::mutex> lock(ConsoleManager::getMutex());
    ConsoleManager::clearInputLine();
    ConsoleManager::showPrompt();   
}


void Game::exitGame() {  
    running_.store(false, std::memory_order_release);

    if (inputThread_.joinable()) {
        inputThread_.join();
    }
    if (deviceThread_.joinable()) {
        deviceThread_.join();
    }

    std::lock_guard<std::mutex> lock(ConsoleManager::getMutex());
    ConsoleManager::clearScreen();
}

void Game::redrawAll() {
    {
        std::lock_guard<std::mutex> lock(ConsoleManager::getMutex());

        // 1. Восстанавливаем размер
        std::cout << "\033[8;" << 35 << ";" << 140 << "t";
        
        ConsoleManager::clearScreen();
        ConsoleManager::printMenu();
        ConsoleManager::printLevel(currentLevel_, totalScore_, maxScore_);
        ConsoleManager::showPrompt();
    }
    observerManager_.notifyRedraw();
    observerManager_.notifyMood("");
    observerManager_.notifyMessage("");

    std::lock_guard<std::mutex> lock(ConsoleManager::getMutex());
    ConsoleManager::gotoInputLine();
}


bool Game::isRunning() const {
    return running_.load(std::memory_order_acquire);
}


void Game::inputLoop() {

    // Ждем сигнала старта
    {
        std::unique_lock<std::mutex> lock(startMutex_);

        startCV_.wait(lock, [this]() { 
            return running_.load(std::memory_order_acquire); 
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
    while (running_.load(std::memory_order_acquire)) {

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
                    std::lock_guard<std::mutex> lock(inputMutex_);
                    inputBuffer_ = line;
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
        std::unique_lock<std::mutex> lock(startMutex_);
        startCV_.wait(lock, [this]() { 
            return running_.load(std::memory_order_acquire); 
        });
    }
    
    while(running_.load(std::memory_order_acquire)) {
        std::string cmd;
        if(getCommand(cmd)) {
            processCommand(cmd);
        }

        if(needsRedrawDevice_) {
            showDevicesStatus();
            needsRedrawDevice_ = 0;
        }

        if(currentState_ == State::PLAYING) {
            static bool levelInProgress = false;

            if(!levelInProgress) {
                levelInProgress = true;
                runLevelInLoop();
                levelInProgress = false;
            }
        }
            
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }
}


void Game::deviceUpdateLoop() {
    // Ждем запуска
    {
        std::unique_lock<std::mutex> lock(startMutex_);
        startCV_.wait(lock, [this]() { return running_.load(); });
    }

    while (running_.load(std::memory_order_acquire)) {
        {
            std::lock_guard<std::mutex> lock(deviceMutex_);
            std::for_each(devices_.begin(), devices_.end(),[] (auto& device) {
                device->update();
            });
        }

        needsRedrawDevice_ = true;
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    }
}


void Game::processCommand(std::string &command) {
    switch (currentState_) {
        case State::MENU:
            handleMenuCommand(command);
            break;
        case State::PLAYING:
            break;
        // ... другие состояния
        default:
            break;
    }
}


void Game::handleMenuCommand(const std::string& command) {
    if (command == "start") {
        totalScore_ = 0;
        maxScore_ = 0;
        currentLevel_ = 1;

        currentState_ = State::PLAYING;
        std::lock_guard<std::mutex> lock(ConsoleManager::getMutex());
        ConsoleManager::savePosition();
        ConsoleManager::printLevel(1, 0, maxScore_);    
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


// Неблокирующее получение команды
bool Game::getCommand(std::string& cmd) {
    std::lock_guard<std::mutex> lock(inputMutex_);
    if (!inputBuffer_.empty()) {
        cmd = inputBuffer_;
        inputBuffer_.clear();
        return true;
    }
    return false;
}


void Game::runLevelInLoop() {
    {
        std::lock_guard<std::mutex> lock(ConsoleManager::getMutex());
        ConsoleManager::savePosition();
        ConsoleManager::printLevel(currentLevel_, totalScore_, maxScore_);
        ConsoleManager::restorePosition();
    }

    int tasks = 3;
    int problemsAtOnce = currentLevel_;

    for (int task = 0; task < tasks && currentState_ == State::PLAYING && running_.load(); task++) {

        // Генерируем проблемы
        std::vector<CurrentTask> currentTasks = generateProblemsWithSolutions(problemsAtOnce);

        observerManager_.notifyMood("neutral");
        std::string msg = "Задача " + std::to_string(task + 1) + "/" + std::to_string(tasks);
        if (currentLevel_ == 1 ) {
            observerManager_.notifyMessage(msg + ". Неисправен 1 прибор. Введи номер прибора");
        } else {
            observerManager_.notifyMessage(msg + ". Неисправено " + std::to_string(currentLevel_) 
                + " приборa. Введи номер прибора");
        }

        ConsoleManager::hideCursor();
        if (running_) std::this_thread::sleep_for(std::chrono::milliseconds(2500));
        ConsoleManager::showCursor();

        // Обработка задач по одной, пока все не будут исправлены
        while(!currentTasks.empty() && running_ && currentState_ == State::PLAYING) {
            int selectedIndex;
            if (!askToSelectDevice(selectedIndex)) {
                return;
            }

            auto it = std::find_if(currentTasks.begin(), currentTasks.end(), 
                [selectedIndex](const CurrentTask& t) {
                return t.deviceIndex_ == selectedIndex;
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
                devices_[selectedIndex]->clearMalfunctions();
                showDevicesStatus();

                // Пауза для чтения
                std::this_thread::sleep_for(std::chrono::milliseconds(1500));

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
        if (running_) std::this_thread::sleep_for(std::chrono::milliseconds(1000));
        ConsoleManager::showCursor();
    }

    // Завершение уровня
    completeLevel();
}


std::vector<CurrentTask> Game::generateProblemsWithSolutions(int count) {
    std::vector<CurrentTask> tasks;

    // Создание списка индексов приборов
    std::vector<int> deviceInd;
    for (std::size_t i = 0; i < devices_.size(); ++i) {
        deviceInd.push_back(i);
    }

    // Перемешивание индексов
    std::shuffle(deviceInd.begin(), deviceInd.end(), std::mt19937(std::random_device{}()));


    for (int i = 0; i < std::min(count, (int)deviceInd.size()); i++) {
        // Выбор случайного прибора
        int deviceIndex = deviceInd[i];
        Device* device = devices_[deviceIndex].get();

        auto possibleMalfunctions = device->getMalfunctions();
        if(!possibleMalfunctions.empty()) {
            int malfunctionIndex = rand() % possibleMalfunctions.size();
            Malfunction selectedMalfunction = possibleMalfunctions[malfunctionIndex];

            /*
            {
                std::lock_guard<std::mutex> lock(ConsoleManager::getMutex());
                ConsoleManager::printDebug("deviceIndex = " + std::to_string(deviceIndex) + " malfunctionIndex=" +
                std::to_string(malfunctionIndex));
            }*/

            // Установка неисправность в прибор
            device->addMalfunctions(selectedMalfunction);  
            
            // Создание задачи с перемешанными решениями
            CurrentTask task;
            task.deviceIndex_ = deviceIndex;
            task.malfunction_ = selectedMalfunction;

            auto solution = selectedMalfunction.solutions_;
            std::shuffle(solution.begin(), solution.end(), std::mt19937{std::random_device{}()});
            task.shuffledSolutions_ = solution;

            tasks.push_back(task);
        }
    }

    return tasks;
}


bool Game::askToSelectDevice(int& selectedIndex) {
    auto lastStatusUpdate = std::chrono::steady_clock::now();

    while(running_ && currentState_ == State::PLAYING) {
        // Обновление статуса устройств раз в секунду
        updateDeviceStatusWithTimer(lastStatusUpdate);

        std::string input;
        if (!getCommand(input)) continue;

        if (input == "stop") {
            clearDeviceMalfunctions();
            std::string msg = "Принудительное завершение игры. Счет " + std::to_string(totalScore_) 
                + "/" + std::to_string(maxScore_);
            stopGame("sad", msg);
            return false;
        }
        if (input == "exit") {
            exitGame();
            return false;
        }

        int num;
        if(std::istringstream(input) >> num) {
            if (num >= 1 && num <= devices_.size()) {
                selectedIndex = num - 1;
                return true;
            } else {
                observerManager_.notifyMessage("Введи номер от 1 до " + std::to_string(devices_.size()));
            }
        }
    }
    return false;
}


void Game::showProblemAndSolutions(const CurrentTask& task) {   
    Device* device = devices_[task.deviceIndex_].get();

    std::string output;
    for (size_t i = 0; i < task.shuffledSolutions_.size(); i++) {
        output += std::to_string(i+1) + ". " + task.shuffledSolutions_[i].description_ + "\n";
    }

    observerManager_.notifyMood("neutral");
    std::string msg = "Прибор: " + device->getName() + ". Неисправность: " + task.malfunction_.name_;

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
        std::to_string(task.malfunction_.solutions_.size())+ "):");   
}


bool Game::getUserSolutionChoice(CurrentTask& task) {
    auto lastStatusUpdate = std::chrono::steady_clock::now();

    while (running_ && currentState_ == State::PLAYING) {
        // Обновление статуса устройств раз в секунду
        updateDeviceStatusWithTimer(lastStatusUpdate);

        std::string input;
        if (!getCommand(input)) continue;
        
        if (input == "stop") {
            clearDeviceMalfunctions();
            std::string msg = "Принудительное завершение игры. Счет " + std::to_string(totalScore_) 
                + "/" + std::to_string(maxScore_);
            stopGame("sad", msg);
            return false;
        }
        if (input == "exit") {
            exitGame();
            return false;
        }

        int choice;
        if (std::istringstream(input) >> choice) {
            if (choice >= 1 && choice <= task.shuffledSolutions_.size()) {
                task.selectedSolutionIndex_ = choice - 1;
                return true;
            }
            observerManager_.notifyMessage("Введи номер от 1 до " + std::to_string(task.shuffledSolutions_.size()));
        }
    }
    return false;
}


void Game::checkAndScore(CurrentTask& task) {
    if (task.selectedSolutionIndex_ < 0 || task.selectedSolutionIndex_ >= task.shuffledSolutions_.size()) {
        return;
    }

    const Solution& chosen = task.shuffledSolutions_[task.selectedSolutionIndex_];
    int points = chosen.score_;
    task.correctPoints_ = points;

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
    observerManager_.notifyMessage(chosen.comment_);

    updateScore(points);

    {
        std::lock_guard<std::mutex> lock(ConsoleManager::getMutex());
        ConsoleManager::savePosition();
        ConsoleManager::printLevel(currentLevel_, totalScore_, maxScore_);

        // Очищаем экран с вариантами
        ConsoleManager::clearActionArea();
        ConsoleManager::restorePosition();
    }
}


void Game::updateDeviceStatusWithTimer(std::chrono::steady_clock::time_point& lastUpdate) {
    auto now = std::chrono::steady_clock::now();
    if (std::chrono::duration_cast<std::chrono::seconds>(now - lastUpdate).count() >= 1) {
        showDevicesStatus();
        lastUpdate = now;
    }
}


void Game::completeLevel() {
    if (currentState_ == State::PLAYING && running_.load()) {
        observerManager_.notifyMood("happy");
        observerManager_.notifyMessage("Уровень " + std::to_string(currentLevel_) + " завершен! Счет: " + 
                    std::to_string(totalScore_));

        ConsoleManager::hideCursor();
        std::this_thread::sleep_for(std::chrono::seconds(3));
        ConsoleManager::showCursor();

        if (currentLevel_ == 3) {
            std::string msg = "Игра завершена! Твоя кваллификация: " + getQualification();
            stopGame("happy", msg);
            return;
        }

        if ((double)totalScore_/maxScore_ > 0.5) {
            currentLevel_++;
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
    for (auto &device : devices_) {

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
                param_str = pad_to_width(param_str, 33);
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
    for (auto& device : devices_) {
        device->clearMalfunctions();
    }
}


std::string Game::getQualification() const {
    if(totalScore_ >= 1600) return "эксперт";
    if(totalScore_ >= 1400) return "специалист";
    if(totalScore_ >= 1200) return "оператор";
    if(totalScore_ >= 1000) return "стажер";
    return "ученик";
}


void Game::updateScore(int points) {
    totalScore_ += points;
    maxScore_ += 100;
}

void Game::stopGame(const std::string& mood, const std::string& msg) {
    currentState_ = State::MENU;
    currentLevel_ = 0;
    clearDeviceMalfunctions();

    observerManager_.notifyMood(mood);
    observerManager_.notifyMessage(msg);
    
    std::lock_guard<std::mutex> lock(ConsoleManager::getMutex());
    ConsoleManager::savePosition();
    ConsoleManager::clearLevelLine();
    ConsoleManager::clearActionArea();
    ConsoleManager::restorePosition();
}