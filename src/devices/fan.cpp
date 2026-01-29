#include "devices/device.h"
#include <devices/deviceregistry.h>
#include <devices/fan.h>
#include <devices/literals.h>

using namespace literals;

namespace {
    bool _ = []() {
        DeviceRegistry::registerType<Fan>("Fan");
        return true;
    }();
}


Fan::Fan() : Device("Fan") {
    params.emplace(DeviceParameter("Rpm", {0, 2000}, {1200, 1500}), 1350_rpm);
    params.emplace(DeviceParameter("Current", {5.f, 20.f}, { 8.f, 12.f}), 10._amper);
    params.emplace(DeviceParameter("Temperature", {20.f, 120.f}, {40.f, 70.f}), 65._celsies);

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
    lowRpm.name = "Низкие обороты";
    lowRpm.description = "Rpm under 1200";
    
    // Условия
    lowRpm.conditions["Rpm"] = {0, 1199};
    
    // Создание решений   
    lowRpm.solutions.push_back(Optimal("Проверить щетки, измерить напряжение", 100));
    lowRpm.solutions.push_back(Accept("Почистить воздуховод, увеличить скорость", 80));
    lowRpm.solutions.push_back(Critical("Заменить двигатель", 40));
    lowRpm.solutions.push_back(Failure("Вручную раскрутить лопасти", 0));

    return lowRpm;
}


Malfunction Fan::createOverheat() {
    // неисправность "Перегрев"
    Malfunction overheating;
    overheating.name = "Перегрев";
    overheating.description = "Temperature above 70°C";
    
    // Условия: температура должна быть > 70°C
    overheating.conditions["Temperature"] = {71.0, 120.0};
    
    // Создание решений 
    overheating.solutions.push_back(Optimal("Остановить, дать остыть, проверить ток", 100));
    overheating.solutions.push_back(Accept("Снизить скорость, включить обдув", 70));
    overheating.solutions.push_back(Critical("Временно отключить термозащиту", 30));
    overheating.solutions.push_back(Failure("Залить корпус водой для охлаждения", 0));

    return overheating;
}


Malfunction Fan::createOverCurrent() {
    // неисправность "Повышенный ток"
    Malfunction overCurrent;
    overCurrent.name = "Повышенный ток";
    overCurrent.description = "Current above 12";
    
    // Условия
    overCurrent.conditions["Current"] = {13.0, 20.0};
    
    // Создание решений 
    overCurrent.solutions.push_back(Optimal("Проверить сопротивление обмоток", 100));
    overCurrent.solutions.push_back(Accept("Снизить нагрузку на привод", 80));
    overCurrent.solutions.push_back(Critical("Увеличить напряжение на 10%", 20));
    overCurrent.solutions.push_back(Failure("Заменить предохранитель на более мощный", 0));

    return overCurrent;
}

