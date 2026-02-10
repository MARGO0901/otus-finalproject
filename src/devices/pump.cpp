#include "malfunction.h"
#include <devices/pump.h>
#include <devices/deviceregistry.h>
#include <devices/literals.h>

using namespace literals;

namespace {
    bool _ = []() {
        DeviceRegistry::registerType<Pump>("Насос");
        return true;
    }();
}

Pump::Pump() : Device("Насос") {
    params_.emplace(DeviceParameter("Давление", {0.f,10.f}, {4.f, 6.f}), 5.0_bar);
    params_.emplace(DeviceParameter("Температура", {20.f, 120.f}, {60.f, 80.f}), 65.0_celsies);
    params_.emplace(DeviceParameter("Ток", {10.f, 50.f}, {30.f, 35.f}), 32.5_amper);
    params_.emplace(DeviceParameter("Вибрация", {0.f, 15.f}, {0.f, 3.f}), 1.5_mms);

    malfunctions_ = createMalfunctions();
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
    overheating.name_ = "Перегрев";
    
    // Условия: Temperature above 80
    overheating.conditions_["Температура"] = {81.0, 120.0}; 
    
    // Создание решений   
    overheating.solutions_.push_back(Optimal("Снизить обороты, проверить термодатчик", 100));
    overheating.solutions_.push_back(Accept("Включить допохлаждение, сбросить аварию", 80));
    overheating.solutions_.push_back(Critical("Остановить, вызвать ремонтников", 40));
    overheating.solutions_.push_back(Failure("Увеличить нагрузку для прогона", 0));

    return overheating;
}


Malfunction Pump::createVibration() {
    // неисправность Вибрация
    Malfunction vibration;
    vibration.name_ = "Повышенная вибрация";

    // Условия: Vibro above 3
    vibration.conditions_["Вибрация"] = {4.0, 15.0};

    // Создание решений   
    vibration.solutions_.push_back(Optimal("Запустить резерв, проверить балансировку", 100));
    vibration.solutions_.push_back(Accept("Ослабить крепеж, перезапустить", 80));
    vibration.solutions_.push_back(Critical("Увеличить подачу смазки", 30));
    vibration.solutions_.push_back(Failure("Закрепить струбцинами, не останавливая", 0));

    return vibration;
}


Malfunction Pump::createNoPressure() {
    // неисправность Нет давления
    Malfunction noPress;
    noPress.name_ = "Нет давления";

    // Условия: Press under 4
    noPress.conditions_["Давление"] = {0.0, 3.0};

    // Создание решений  
    noPress.solutions_.push_back(Optimal("Переключить на резервный контур", 100));
    noPress.solutions_.push_back(Accept("Проверить обратный клапан, перезапустить", 80));
    noPress.solutions_.push_back(Critical("Продуть импульсно на полной мощности", 20));
    noPress.solutions_.push_back(Failure("Временно отключить датчик давления", 0));

    return noPress;
}

