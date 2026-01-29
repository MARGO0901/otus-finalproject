#include "devices/deviceparameter.h"
#include <devices/deviceregistry.h>
#include <devices/compressor.h>
#include <devices/literals.h>

using namespace literals;

namespace {
    bool _ = []() {
        DeviceRegistry::registerType<Compressor>("Compressor");
        return true;
    }();
}


Compressor::Compressor() : Device("Compressor") {
    params.emplace(DeviceParameter("Press", {0.f, 15.f}, {6.f, 9.f}), 7._bar);
    params.emplace(DeviceParameter("Oil Temperature", {20.f, 100.f}, {60.f, 80.f}), 70._celsies);
    params.emplace(DeviceParameter("Oil Level", { 0, 100}, {60, 90}), 85);
    params.emplace(DeviceParameter("On/Off Counter", {0, 100}, {0, 10}), 5_times);

    malfunctions = createMalfunctions();
}


std::vector<Malfunction> Compressor::createMalfunctions() {
    std::vector<Malfunction> malfunctions;

    malfunctions.push_back(createOilLeak());
    malfunctions.push_back(createOverPressure());
    malfunctions.push_back(createOftenStart());
    
    return malfunctions;
}


int Compressor::applySolution(const Solution& solution) {
    return 0;
}


Malfunction Compressor::createOilLeak() {
    // неисправность "Утечка масла"
    Malfunction oilLeak;
    oilLeak.name = "Утечка масла";
    oilLeak.description = "Oil Level under 60";
    
    // Условия
    oilLeak.conditions["Oil Level"] = {0, 59};
    
    // Создание решений   
    oilLeak.solutions.push_back(Optimal("Остановить, найти течь, подтянуть соединения", 100));
    oilLeak.solutions.push_back(Accept("Долить масло, мониторить уровень", 60));
    oilLeak.solutions.push_back(Critical("Увеличить вязкость масла добавкой", 30));
    oilLeak.solutions.push_back(Failure("Загерметизировать течь силиконом", 0));

    return oilLeak;
}


Malfunction Compressor::createOverPressure() {
    // неисправность "Высокое давление"
    Malfunction overPressure;
    overPressure.name = "Высокое давление";
    overPressure.description = "Pressure above 9";
    
    // Условия
    overPressure.conditions["Press"] = {10.0, 15.0};
    
    // Создание решений
    overPressure.solutions.push_back(Optimal("Сбросить через клапан, проверить реле", 100));
    overPressure.solutions.push_back(Accept("Отключить нагрузку, проверить манометр", 75));
    overPressure.solutions.push_back(Critical("Временно стравить в атмосферу", 40));
    overPressure.solutions.push_back(Failure("Закоротить контакты датчика", 0));
   
    return overPressure;
}


Malfunction Compressor::createOftenStart() {
    // неисправность "Частые включения"
    Malfunction overPressure;
    overPressure.name = "Often Start";
    overPressure.description = "Start Counter above 10";
    
    // Условия
    overPressure.conditions["On/Off Counter"] = {11, 100};
    
    // Создание решений
    overPressure.solutions.push_back(Optimal("Найти утечку, проверить ресивер", 100));
    overPressure.solutions.push_back(Accept("Отрегулировать гистерезис реле", 85));
    overPressure.solutions.push_back(Critical("Включить в ручной режим", 25));
    overPressure.solutions.push_back(Failure("Заблокировать пускатель", 0));
   
    return overPressure;
}
