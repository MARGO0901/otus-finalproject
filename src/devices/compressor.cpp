#include "devices/deviceparameter.h"
#include <devices/deviceregistry.h>
#include <devices/compressor.h>
#include <devices/literals.h>

using namespace literals;

namespace {
    bool _ = []() {
        DeviceRegistry::registerType<Compressor>("Компрессор");
        return true;
    }();
}


Compressor::Compressor() : Device("Компрессор") {
    params_.emplace(DeviceParameter("Давление", {0.f, 15.f}, {6.f, 9.f}), 7._bar);
    params_.emplace(DeviceParameter("Температура масла", {20.f, 100.f}, {60.f, 80.f}), 70._celsies);
    params_.emplace(DeviceParameter("Уровень масла", { 0, 100}, {60, 90}), 85);
    params_.emplace(DeviceParameter("Частота пусков", {0, 100}, {0, 10}), 5_times);

    malfunctions_ = createMalfunctions();
}


std::vector<Malfunction> Compressor::createMalfunctions() {
    std::vector<Malfunction> malfunctions;

    malfunctions.push_back(createOilLeak());
    malfunctions.push_back(createOverPressure());
    malfunctions.push_back(createOftenStart());
    
    return malfunctions;
}


Malfunction Compressor::createOilLeak() {
    // неисправность "Утечка масла"
    Malfunction oilLeak;
    oilLeak.name_ = "Утечка масла";
    
    // Условия: Oil Level under 60
    oilLeak.conditions_["Уровень масла"] = {0, 59};
    
    // Создание решений   
    oilLeak.solutions_.push_back(Optimal("Остановить, найти течь, подтянуть соединения", 100));
    oilLeak.solutions_.push_back(Accept("Долить масло, мониторить уровень", 60));
    oilLeak.solutions_.push_back(Critical("Увеличить вязкость масла добавкой", 30));
    oilLeak.solutions_.push_back(Failure("Загерметизировать течь силиконом", 0));

    return oilLeak;
}


Malfunction Compressor::createOverPressure() {
    // неисправность "Высокое давление"
    Malfunction overPressure;
    overPressure.name_ = "Высокое давление";
    
    // Условия: Pressure above 9
    overPressure.conditions_["Давление"] = {10.0, 15.0};
    
    // Создание решений
    overPressure.solutions_.push_back(Optimal("Сбросить через клапан, проверить реле", 100));
    overPressure.solutions_.push_back(Accept("Отключить нагрузку, проверить манометр", 75));
    overPressure.solutions_.push_back(Critical("Временно стравить в атмосферу", 40));
    overPressure.solutions_.push_back(Failure("Закоротить контакты датчика", 0));
   
    return overPressure;
}


Malfunction Compressor::createOftenStart() {
    // неисправность "Частые включения"
    Malfunction overPressure;
    overPressure.name_ = "Частые включения";
    
    // Условия: Start Counter above 10
    overPressure.conditions_["Частота пусков"] = {11, 100};
    
    // Создание решений
    overPressure.solutions_.push_back(Optimal("Найти утечку, проверить ресивер", 100));
    overPressure.solutions_.push_back(Accept("Отрегулировать гистерезис реле", 85));
    overPressure.solutions_.push_back(Critical("Включить в ручной режим", 25));
    overPressure.solutions_.push_back(Failure("Заблокировать пускатель", 0));
   
    return overPressure;
}
