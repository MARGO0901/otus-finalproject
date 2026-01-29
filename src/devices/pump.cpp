#include "malfunction.h"
#include <devices/pump.h>
#include <devices/deviceregistry.h>
#include <devices/literals.h>

using namespace literals;

namespace {
    bool _ = []() {
        DeviceRegistry::registerType<Pump>("Pump");
        return true;
    }();
}

Pump::Pump() : Device("Pump") {
    params.emplace(DeviceParameter("Press", {0.f,10.f}, {4.f, 6.f}), 5.0_bar);
    params.emplace(DeviceParameter("Temperature", {20.f, 120.f}, {60.f, 80.f}), 65.0_celsies);
    params.emplace(DeviceParameter("Current", {10.f, 50.f}, {30.f, 35.f}), 32.5_amper);
    params.emplace(DeviceParameter("Vibro", {0.f, 15.f}, {0.f, 3.f}), 1.5_mms);

    malfunctions = createMalfunctions();
}


int Pump::applySolution(const Solution& solution) {
    // Здесь логика применения решения
    // Например, если решение "включить охлаждение"
    if(solution.description.find("cooling") != std::string::npos) {
        //params["Temperature"] -= 10.0;
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
    overheating.name = "Перегрев";
    overheating.description = "Temperature above 80";
    
    // Условия
    overheating.conditions["Temperature"] = {81.0, 120.0}; 
    
    // Создание решений   
    overheating.solutions.push_back(Optimal("Снизить обороты, проверить термодатчик", 100));
    overheating.solutions.push_back(Accept("Включить допохлаждение, сбросить аварию", 80));
    overheating.solutions.push_back(Critical("Остановить, вызвать ремонтников", 40));
    overheating.solutions.push_back(Failure("Увеличить нагрузку для прогона", 0));

    return overheating;
}


Malfunction Pump::createVibration() {
    // неисправность Вибрация
    Malfunction vibration;
    vibration.name = "Вибрация";
    vibration.description = "Vibro above 3";

    // Условия
    vibration.conditions["Vibro"] = {4.0, 15.0};

    // Создание решений   
    vibration.solutions.push_back(Optimal("Запустить резерв, проверить балансировку", 100));
    vibration.solutions.push_back(Accept("Ослабить крепеж, перезапустить", 80));
    vibration.solutions.push_back(Critical("Увеличить подачу смазки", 30));
    vibration.solutions.push_back(Failure("Закрепить струбцинами, не останавливая", 0));

    return vibration;
}


Malfunction Pump::createNoPressure() {
    // неисправность Нет давления
    Malfunction noPress;
    noPress.name = "Нет давления";
    noPress.description = "Press under 4";

    // Условия
    noPress.conditions["Press"] = {0.0, 3.0};

    // Создание решений  
    noPress.solutions.push_back(Optimal("Переключить на резервный контур", 100));
    noPress.solutions.push_back(Accept("Проверить обратный клапан, перезапустить", 80));
    noPress.solutions.push_back(Critical("Продуть импульсно на полной мощности", 20));
    noPress.solutions.push_back(Failure("Временно отключить датчик давления", 0));

    return noPress;
}

