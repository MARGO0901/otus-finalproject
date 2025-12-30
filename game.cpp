#include "game.h"

Game::Game() : currentLevel(1), totalScore(0) {
    devices.push_back(std::make_unique<Pump>());
    //devices.push_back(std::make_unique<Fan>());
    //devices.push_back(std::make_unique<Compressor>());

    penguin.say("Welcome");
}

void Game::runLevel(int level) {
    penguin.say("Let's start the level " + std::to_string(currentLevel));

    int tasks = (level == 1) ? 3 : (level == 2) ? 6 : 9;
    int problemsAtOnce = level;

    for (int task = 0; task < tasks; ++task) {
        // Генерируем проблемы
        generateProblems(problemsAtOnce);

        //показать состояние приборов
        showDevicesStatus();

        // Пользователь выбирает решение
        // ... здесь логика ввода ...
        
        // Проверяем ответ
        // ... здесь проверка ...
        
        // Обновляем счет
        // ... здесь обновление ...
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
            currentProblems.push_back({device, malfunctions[malfuntionIndex]});
        }
    }
}

void Game::showDevicesStatus() {
    system("cls");

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