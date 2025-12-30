#include "device.h"

void Device::update() {
    //базовая реализация - случайные колебания
    for (auto &[key, value]: params) {
        value +=(rand() % 10 - 5) * 0.1;
    }
}


Pump::Pump() : Device("Pump") {
    params = { 
        {"Press", 5.0},
        {"Temperature", 65.0},
        {"Current", 32.0},
        {"Vibro", 2.0}
    };

    malfunctions = createPumpMalfunctions();
}

void Pump::update() {
    params["Temperature"] += (rand() % 3 - 1) * 0.5;
    params["Press"] += (rand() % 5 - 2) * 0.1;
}

std::vector<Malfunction> Pump::checkMalfunctions() {
    std::vector<Malfunction> active;
    for(auto& malfunction : malfunctions) {
        if(malfunction.isActive(params)) {
            active.push_back(malfunction);
        }
    }
    return active;
}

int Pump::applySolution(const Solution& solution) {
    // Здесь логика применения решения
    // Например, если решение "включить охлаждение"
    if(solution.description.find("cooling") != std::string::npos) {
        params["Temperature"] -= 10.0;
    }
    return solution.score;
}

std::vector<Malfunction> Pump::createPumpMalfunctions() {
    std::vector<Malfunction> malfunctions;

    // Создаем неисправность "Перегрев"
    Malfunction overheating;
    overheating.name = "Overheat";
    overheating.description = "Temperature above 80°C";
    
    // Условия: температура должна быть > 80°C
    overheating.conditions["Temperature"] = {0.0, 80.0}; // норма: 0-80°C
    
    // Создаем решения
    Solution optimal;
    optimal.description = "Включить охлаждение -> Снизить нагрузку -> Сбросить ошибку";
    optimal.score = 100;
    optimal.comment = "Правильно!";
    
    Solution acceptable;
    acceptable.description = "Снизить нагрузку -> Сбросить ошибку";
    acceptable.score = 80;
    acceptable.comment = "Хорошо, но можно лучше";
    
    overheating.solutions.push_back(optimal);
    overheating.solutions.push_back(acceptable);
    
    malfunctions.push_back(overheating);

    return malfunctions;
}


