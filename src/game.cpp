#include <game.h>

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
    ConsoleManager::clearInputLine(11);
    ConsoleManager::showPrompt(11);   
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

                ConsoleManager::clearInputLine(11);
                ConsoleManager::showPrompt(11);
                ConsoleManager::gotoxy(2, 11);
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
    }
    else if (command == "exit" || command == "quit") {
        running.store(false);
    }
    else if (command == "help") {
        penguin.say("Доступные команды: start, exit, help");
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
        generateProblems(problemsAtOnce);

        std::string msg = "Задача " + std::to_string(task + 1) + "/" + 
                         std::to_string(tasks) + ". Выбери устройство для починки!";
        penguin.say(msg);

        // Ожидание ввода пользователя
        bool validInput = false;
        
        while (!validInput && running.load() && currentState == State::PLAYING) {
            // Ждем команду от пользователя
            std::string userInput;
            if (getCommand(userInput)) {                
                // Проверка валидности ввода
                if (userInput == "1" || userInput == "2" || userInput == "3" || userInput == "4" || userInput == "5") {
                    // Обработка выбора
                    bool correct = checkSolution();
                    updateScore(correct);
                    
                    if (correct) {
                        penguin.setMood("happy");
                        penguin.say("Правильно! +10 очков!");
                    } else {
                        penguin.setMood("sad");
                        penguin.say("Ой... Неправильно.");
                    }
                    
                    userInput = true;
                }
                else if (userInput == "menu") {
                    currentState = State::MENU;
                    return;
                }
                else {
                    penguin.say("Введи номер 1-" + std::to_string(devices.size()));
                }
            }

            if (needsRedrawDevice) {
                showDevicesStatus();
                needsRedrawDevice = false;
            }
            
            std::this_thread::sleep_for(std::chrono::milliseconds(50));
        }

        if (currentState != State::PLAYING || !running.load()) {
            break;
        }
        
        //Проверяем решение
        bool correct = checkSolution(/* параметры */);
        updateScore(correct);
        
        // Реакция пингвина
        if (correct) {
            penguin.setMood("happy");
            penguin.say("Правильно! +10 очков!");
        } else {
            penguin.setMood("sad");
            penguin.say("Ой... Неправильно. -5 очков");
        }
        
        // Пауза
        for (int i = 0; i < 20 && running; i++) {
            std::this_thread::sleep_for(std::chrono::milliseconds(50));
        }
    }

    // Завершение уровня
    if (currentState == State::PLAYING && running.load()) {
        penguin.setMood("happy");
        penguin.say("Уровень " + std::to_string(level) + " завершен! Счет: " + 
                    std::to_string(totalScore));

        std::this_thread::sleep_for(std::chrono::seconds(2));
        
        // Возвращаемся в меню
        currentState = State::MENU;
        //showMainMenu();
    }
}


void Game::processUserInput(const std::string& input) {
    // Пример обработки ввода:
    if (input == "1" || input == "2" || input == "3" || 
        input == "4" || input == "5") {
        int choice = std::stoi(input) - 1;
        
        if (choice >= 0 && choice < devices.size()) {
            penguin.say("Выбрано: " + devices[choice]->getName());
        }
    } else if (input == "help") {
        penguin.say("Выбери номер устройства (1-5) для починки!");
    } else if (input == "status") {
        showDevicesStatus();
    } else {
        penguin.say("Не понял. Введи номер устройства (1-5)");
    }
}


void Game::generateProblems(int count) {
    currentProblems.clear();

    for (int i = 0; i < count; i++) {
        //выбираем случайный прибор
        int deviceIndex = rand() % devices.size();
        Device* device = devices[deviceIndex].get();

        //проверяем его неисправности
        auto malfunctions = device->checkMalfunctions();
        if (!malfunctions.empty()) {
            int malfuntionIndex = rand() % malfunctions.size();
            currentProblems.push_back(std::make_pair(device, malfunctions[malfuntionIndex]));
        }
    }
}


void Game::showDevicesStatus() {    
    // Сохранение позиции курсора
    ConsoleManager::savePosition();

    // Очистка области под пингвином (строки 5-9)
    for (int i = 5; i <= 9; i++) {
        ConsoleManager::clearInputLine(i);
    }

    // Показать приборы
    ConsoleManager::gotoxy(0, 6);
    ConsoleManager::print("===Devices===\n");
    for (auto &device : devices) {
        std::string params;
        std::cout << std::left << std::setw(11) << device->getName() << ":\t";
        for(auto& [param, value] : device->getParams()) {
            std::string val_str = variantToString(value);
            // Создание строки
            std::string param_str =
                (boost::format("%s(%.0f..%.0f) = %s") % param.name_ %
                param.optRange_.first % param.optRange_.second % val_str)
                    .str();
            // Форматирование ширины
            std::string str = (boost::format("%-32s") % param_str).str();
            params += str;
        }
        params += "\n";
        ConsoleManager::print(params);
    }
    
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


bool Game::checkSolution() {
    return true;
}


void Game::updateScore(bool) {

}