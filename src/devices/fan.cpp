#include "devices/device.h"
#include <devices/deviceregistry.h>
#include <devices/fan.h>
#include <devices/units.h>

using namespace units;

namespace {
    bool _ = []() {
        DeviceRegistry::registerType<Fan>("Fan");
        return true;
    }();
}


Fan::Fan() : Device("Fan") {
    params.emplace(DeviceParameter("Rpm", {0, 2000}, {1200, 1500}), 1350._rpm);
    params.emplace(DeviceParameter("Current", {5, 20}, { 8, 12}), 10._amper);
    params.emplace(DeviceParameter("Temperature", {20, 120}, {40, 70}), 65._celsies);

    malfunctions = createMalfunctions();
}


std::vector<Malfunction> Fan::createMalfunctions() {
    std::vector<Malfunction> malfunctions;

    malfunctions.push_back(createLowRpm());
    malfunctions.push_back(createOverCurrent());
    malfunctions.push_back(createOverheat());
    
    return malfunctions;
}


int Fan::applySolution(const Solution& solution) {
    return 0;
}


Malfunction Fan::createLowRpm() {
    // неисправность "Низкие обороты"
    Malfunction lowRpm;
    lowRpm.name = "Low Rpm";
    lowRpm.description = "Rpm under 1200";
    
    // Условия
    lowRpm.conditions["Rpm"] = {1200.0, 1500.0};
    
    // Создание решений   
    lowRpm.solutions.push_back(Optimal("Проверить питание → Очистить лопатки → Отрегулировать скорость", 100));
    lowRpm.solutions.push_back(Accept("Увеличить напряжение → Проверить скорость", 80));
    lowRpm.solutions.push_back(Critical("Заменить вентилятор → Вызвать мастера", 40));
    lowRpm.solutions.push_back(Failure("Отключить вентилятор", 0));

    return lowRpm;
}


Malfunction Fan::createOverheat() {
    // неисправность "Перегрев"
    Malfunction overheating;
    overheating.name = "Overheat";
    overheating.description = "Temperature above 70°C";
    
    // Условия: температура должна быть > 70°C
    overheating.conditions["Temperature"] = {40.0, 70.0};
    
    // Создание решений 
    overheating.solutions.push_back(Optimal("Остановить → Очистить от пыли → Проверить подшипники", 100));
    overheating.solutions.push_back(Accept("Снизить нагрузку → Увеличить охлаждение", 70));
    overheating.solutions.push_back(Critical("Обдуть сжатым воздухом на ходу", 30));
    overheating.solutions.push_back(Failure("Залить водой для охлаждения", 0));

    return overheating;
}


Malfunction Fan::createOverCurrent() {
    // неисправность "Повышенный ток"
    Malfunction overCurrent;
    overCurrent.name = "Overcurrent";
    overCurrent.description = "Current above 12";
    
    // Условия
    overCurrent.conditions["Current"] = {8.0, 12.0};
    
    // Создание решений 
    overCurrent.solutions.push_back(Optimal("Проверить нагрузку → Проверить подшипники → Отключить для диагностики", 100));
    overCurrent.solutions.push_back(Accept("Снизить скорость → Контролировать температуру", 80));
    overCurrent.solutions.push_back(Critical("Увеличить напряжение", 20));
    overCurrent.solutions.push_back(Failure("Отключить защиту по току", 0));

    return overCurrent;
}

