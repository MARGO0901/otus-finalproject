#include <atomic>
#include <boost/format/format_fwd.hpp>
#include <boost/move/utility_core.hpp>
#include <chrono>
#include <game.h>

#include <iomanip>
#include <ios>
#include <mutex>
#include <termios.h>
#include <unistd.h>
#include <fcntl.h>

#include <boost/format.hpp>

#include <consolemanager.h>
#include <devices/deviceregistry.h>

static std::mutex outputMutex;

Game::Game(const std::vector<std::string>& deviceNames) : currentLevel(1), totalScore(0) {

    //penguin.say("Привет!");

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


void Game::inputLoop() {

    // Ждем сигнала старта
    {
        std::unique_lock<std::mutex> lock(startMutex);
        
        // Ожидание пока running не станет true;
        startCV.wait(lock, [this]() { 
            return running.load(std::memory_order_acquire); 
        });
    }

    ConsoleManager::gotoxy(2, 11);

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
                    std::lock_guard<std::mutex> lock(inputMtx);
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


// Неблокирующее получение команды
bool Game::getCommand(std::string& cmd) {
    std::lock_guard<std::mutex> lock(inputMtx);
    if (!inputBuffer.empty()) {
        cmd = inputBuffer;
        inputBuffer.clear();
        return true;
    }
    return false;
}


void Game::start() {
        
    inputThread = std::thread(&Game::inputLoop, this);
    uiThread = std::thread([this]() { uiThreadFunc(); });

    for (std::size_t i = 0; i < devices.size(); i++) {
        deviceThreads.emplace_back([this, i]() {
            deviceThread(i);
        });
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(50));

    // Показываем приглашение для ввода
    ConsoleManager::clearInputLine(11);
    ConsoleManager::showPrompt(11);

    // ТОЛЬКО ТЕПЕРЬ устанавливаем running и уведомляем
    {
        std::lock_guard<std::mutex> lock(startMutex);
        running.store(true, std::memory_order_release);
    }
    startCV.notify_all();
}


void Game::stop() {
    running = false;

    for (auto& thread : deviceThreads) {
        if (thread.joinable()) thread.join();
    }
    deviceThreads.clear();

    if (uiThread.joinable()) uiThread.join();
}


void Game::deviceThread(std::size_t deviceIndex) {
    // Ждем сигнала старта
    {
        std::unique_lock<std::mutex> lock(startMutex);
        
        // Ожидание пока running не станет true;
        startCV.wait(lock, [this]() { 
            return running.load(std::memory_order_acquire); 
        });
    }
    
    auto &device = devices[deviceIndex];

    while(running.load(std::memory_order_acquire)) {
        device->update();
        needsRedrawDevice = true;
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    }
}


void Game::uiThreadFunc() {
    // Ждем сигнала старта
    {
        std::unique_lock<std::mutex> lock(startMutex);
        
        // Ожидание пока running не станет true;
        startCV.wait(lock, [this]() { 
            return running.load(std::memory_order_acquire); 
        });
    }

    auto lastRedraw = std::chrono::steady_clock::now();

    while(running.load(std::memory_order_acquire)) {
        auto now = std::chrono::steady_clock::now();
        bool timeToRedraw = (now - lastRedraw) > std::chrono::milliseconds(100);

        if (needsRedrawDevice && timeToRedraw) {
            std::lock_guard<std::mutex> lock(deviceDrawMtx);
            showDevicesStatus();
            lastRedraw = now;
            needsRedrawDevice = false;
        }

        // Не грузить CPU
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }
}


void Game::runLevel(int level) {
    currentLevel = level; 

    penguin.say("Начинаем уровень " + std::to_string(currentLevel));

    int tasks = (level == 1) ? 3 : (level == 2) ? 6 : 9;
    int problemsAtOnce = level;

    for (int task = 0; task < tasks; ++task) {
        // Генерируем проблемы
        generateProblems(problemsAtOnce);

        //показать состояние приборов
        showDevicesStatus();

        std::string msg = "Задача " + std::to_string(task + 1) + "/" + 
                         std::to_string(tasks) + ". Выбери устройство для починки!";
        penguin.say(msg);
        ConsoleManager::gotoxy(2, 11);

        // 5. Ждем ввод пользователя
        std::string userInput;
        bool validInput = false;
        
        while (!validInput && running) {
            // Ждем команду от пользователя
            if (getCommand(userInput)) {
                processUserInput(userInput);
                
                // Проверяем валидность ввода
                if (userInput == "1" || userInput == "2" || userInput == "3" || 
                    userInput == "4" || userInput == "5") {
                    int choice = std::stoi(userInput) - 1;
                    if (choice >= 0 && choice < static_cast<int>(devices.size())) {
                        validInput = true;
                    }
                } else {
                    penguin.say("Введи номер 1-" + std::to_string(devices.size()));
                }
                
                validInput = true; // временно
            }
            
            std::this_thread::sleep_for(std::chrono::milliseconds(50));
        }

        if(!running) break;
        
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
    if (running) {
        penguin.setMood("happy");
        penguin.say("Уровень " + std::to_string(level) + " завершен! Счет: " + 
                    std::to_string(totalScore));
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
    // Сохраняем позицию курсора
    ConsoleManager::savePosition();

    // Очищаем область под пингвином (строки 5-9)
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
            std::ostringstream val;
            val << std::fixed << std::setprecision(1) << value;
            std::string str = (boost::format("%-26s") % (param + " = " + val.str())).str();
            params += str;
        }
        params += "\n";
        ConsoleManager::print(params);
    }
    
    // Восстанавливаем позицию курсора
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