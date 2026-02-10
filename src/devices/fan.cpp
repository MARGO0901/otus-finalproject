#include "devices/device.h"
#include <devices/deviceregistry.h>
#include <devices/fan.h>
#include <devices/literals.h>

using namespace literals;

namespace {
    bool _ = []() {
        DeviceRegistry::registerType<Fan>("Вентилятор");
        return true;
    }();
}


Fan::Fan() : Device("Вентилятор") {
    params_.emplace(DeviceParameter("Обороты", {0, 2000}, {1200, 1500}), 1350_rpm);
    params_.emplace(DeviceParameter("Ток", {5.f, 20.f}, { 8.f, 12.f}), 10._amper);
    params_.emplace(DeviceParameter("Температура", {20.f, 120.f}, {40.f, 70.f}), 65._celsies);

    malfunctions_ = createMalfunctions();
}


std::vector<Malfunction> Fan::createMalfunctions() {
    std::vector<Malfunction> malfunctions;

    malfunctions.push_back(createLowRpm());
    malfunctions.push_back(createOverCurrent());
    malfunctions.push_back(createOverheat());
    
    return malfunctions;
}


Malfunction Fan::createLowRpm() {
    // неисправность "Низкие обороты"
    Malfunction lowRpm;
    lowRpm.name_ = "Низкие обороты";
    
    // Условия: Rpm under 1200
    lowRpm.conditions_["Обороты"] = {0, 1199};
    
    // Создание решений   
    lowRpm.solutions_.push_back(Optimal("Проверить щетки, измерить напряжение", 100));
    lowRpm.solutions_.push_back(Accept("Почистить воздуховод, увеличить скорость", 80));
    lowRpm.solutions_.push_back(Critical("Заменить двигатель", 40));
    lowRpm.solutions_.push_back(Failure("Вручную раскрутить лопасти", 0));

    return lowRpm;
}


Malfunction Fan::createOverheat() {
    // неисправность "Перегрев"
    Malfunction overheating;
    overheating.name_ = "Перегрев";
    
    // Условия: температура должна быть > 70°C
    overheating.conditions_["Температура"] = {71.0, 120.0};
    
    // Создание решений 
    overheating.solutions_.push_back(Optimal("Остановить, дать остыть, проверить ток", 100));
    overheating.solutions_.push_back(Accept("Снизить скорость, включить обдув", 70));
    overheating.solutions_.push_back(Critical("Временно отключить термозащиту", 30));
    overheating.solutions_.push_back(Failure("Залить корпус водой для охлаждения", 0));

    return overheating;
}


Malfunction Fan::createOverCurrent() {
    // неисправность "Повышенный ток"
    Malfunction overCurrent;
    overCurrent.name_ = "Повышенный ток";
    
    // Условия: Current above 12
    overCurrent.conditions_["Ток"] = {13.0, 20.0};
    
    // Создание решений 
    overCurrent.solutions_.push_back(Optimal("Проверить сопротивление обмоток", 100));
    overCurrent.solutions_.push_back(Accept("Снизить нагрузку на привод", 80));
    overCurrent.solutions_.push_back(Critical("Увеличить напряжение на 10%", 20));
    overCurrent.solutions_.push_back(Failure("Заменить предохранитель на более мощный", 0));

    return overCurrent;
}

