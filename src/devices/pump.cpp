#include "malfunction.h"
#include <devices/pump.h>

#include <devices/deviceregistry.h>

namespace {
    bool _ = []() {
        DeviceRegistry::registerType<Pump>("Pump");
        return true;
    }();
}

Pump::Pump() : Device("Pump") {
    params = { 
        {"Press", 5.0},
        {"Temperature", 65.0},
        {"Current", 32.0},
        {"Vibro", 2.0}
    };

    malfunctions = createMalfunctions();
}


void Pump::update() {
    params["Temperature"] += (rand() % 3 - 1) * 0.5;
    params["Press"] += (rand() % 5 - 2) * 0.1;
}


int Pump::applySolution(const Solution& solution) {
    // Здесь логика применения решения
    // Например, если решение "включить охлаждение"
    if(solution.description.find("cooling") != std::string::npos) {
        params["Temperature"] -= 10.0;
    }
    return solution.score;
}

std::vector<Malfunction> Pump::createMalfunctions() {
    std::vector<Malfunction> malfunctions;
    
    malfunctions.push_back(createOverheat());
    malfunctions.push_back(createVibration());
    malfunctions.push_back(createNoPressure());

    return malfunctions;
}


Malfunction Pump::createOverheat() {
    // неисправность "Перегрев"
    Malfunction overheating;
    overheating.name = "Overheat";
    overheating.description = "Temperature above 80";
    
    // Условия
    overheating.conditions["Temperature"] = {60.0, 80.0}; 
    
    // Создание решений   
    overheating.solutions.push_back(Optimal("Включить охлаждение -> Снизить нагрузку -> Сбросить ошибку", 100));
    overheating.solutions.push_back(Accept("Снизить нагрузку -> Сбросить ошибку", 80));
    overheating.solutions.push_back(Critical("Остановить насос -> Вызвать ремонт", 40));
    overheating.solutions.push_back(Failure("Увеличить нагрузку -> Отключить сигнализацию", 0));

    return overheating;
}


Malfunction Pump::createVibration() {
    // неисправность Вибрация
    Malfunction vibration;
    vibration.name = "Vibration";
    vibration.description = "Vibro above 3";

    // Условия
    vibration.conditions["Vibro"] = {0.0, 3.0};

    // Создание решений   
    vibration.solutions.push_back(Optimal("Включить охлаждение -> Снизить нагрузку -> Сбросить ошибку", 100));
    vibration.solutions.push_back(Accept("Проверить крепления → Уменьшить обороты → Сбросить ошибку", 80));
    vibration.solutions.push_back(Critical("Проверить работу → Увеличить охлаждение", 30));
    vibration.solutions.push_back(Failure("Закрепить дополнительными болтами → Увеличить нагрузку", 0));

    return vibration;
}


Malfunction Pump::createNoPressure() {
    // неисправность Нет давления
    Malfunction noPress;
    noPress.name = "No Pressure";
    noPress.description = "Press under 4";

    // Условия
    noPress.conditions["Press"] = {4.0, 6.0};

    // Создание решений  
    noPress.solutions.push_back(Optimal("Запустить резервный насос → Проверить клапаны → Сбросить ошибку", 100));
    noPress.solutions.push_back(Accept("Запустить резервный насос → Сбросить ошибку", 80));
    noPress.solutions.push_back(Critical("Постучать по насосу → Перезапустить", 20));
    noPress.solutions.push_back(Failure("Увеличить мощность", 0));

    return noPress;
}

