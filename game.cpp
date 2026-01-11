#include "game.h"

#include <termios.h>
#include <unistd.h>
#include <fcntl.h>

Game::Game() : currentLevel(1), totalScore(0) {
    devices.push_back(std::make_unique<Pump>());
    //devices.push_back(std::make_unique<Fan>());
    //devices.push_back(std::make_unique<Compressor>());

    // Поток ввода (НЕБЛОКИРУЮЩИЙ)
    inputThread = std::thread(&Game::inputLoop, this);

    penguin.say("Привет!");
}

Game::~Game() {
    running = false;
    if (inputThread.joinable()) {
        inputThread.join();
    }
}

// Перемещение курсора
void Game::gotoxy(int x, int y) {
    std::cout << "\033[" << (y + 1) << ";" << (x + 1) << "H" << std::flush;
}


void Game::inputLoop() {
    // нормальные настройки терминала    
    struct termios oldt, newt;
    tcgetattr(STDIN_FILENO, &oldt);
    newt = oldt;
    // канонический режим для getline
    newt.c_lflag |= (ICANON | ECHO);  
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);
    
    while (running) {
        std::string line;
        
        // Используем getline - он будет блокировать
        if (std::getline(std::cin, line)) {
            if (!line.empty()) {
                {
                    std::lock_guard<std::mutex> lock(mtx);
                    inputBuffer = line;
                }
                cv.notify_one();
                clearInputLine();
            }
        } else {
            // Произошла ошибка или EOF
            break;
        }
    }
    
    // Восстанавливаем настройки
    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
}


// Неблокирующее получение команды
bool Game::getCommand(std::string& cmd) {
    std::lock_guard<std::mutex> lock(mtx);
    if (!inputBuffer.empty()) {
        cmd = inputBuffer;
        inputBuffer.clear();
        return true;
    }
    return false;
}

void Game::clearInputLine() {
    // Очищаем строку 9 (где вводится команда)
    gotoxy(0, 9);
    std::cout << "\033[2K";  // Очистить всю строку
    // Возвращаем курсор в начало строки 9
    gotoxy(0, 9);
    std::cout.flush();
}

void Game::runLevel(int level) {
    currentLevel = level;  // <- добавьте эту строку!
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
        
        // 6. Проверяем решение
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
    //Показать пингвина
    penguin.show();
    std::cout << std::endl;

    //Показать приборы
    std::cout << "===Devices===\n";
    for (auto &device : devices) {
        std::cout << device->getName() << ": ";
        for(auto& [param, value] : device->getParams()) {
            std::cout << param << " = " << value << " ";
        }
        std::cout << std::endl;
    }
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