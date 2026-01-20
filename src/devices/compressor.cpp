#include <devices/deviceregistry.h>
#include <devices/compressor.h>

namespace {
    bool _ = []() {
        DeviceRegistry::registerType<Compressor>("Compressor");
        return true;
    }();
}

Compressor::Compressor() : Device("Compressor") {
    params = {
        {"Press", 7.0},
        {"Oil Temperature", 70.0},
        {"Oil Level", 85.0},
        {"Start Counter", 5.0}
    };
    malfunctions = createMalfunctions();
}


void Compressor::update() {
    //arams[""] += (rand() % 3 - 1) * 0.5;
    //params["Press"] += (rand() % 5 - 2) * 0.1;
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
    oilLeak.name = "Oil Leak";
    oilLeak.description = "Oil Level under 60";
    
    // Условия
    oilLeak.conditions["Rpm"] = {60.0, 90.0};
    
    // Создание решений   
    oilLeak.solutions.push_back(Optimal("Остановить → Проверить уплотнения → Долить масло", 100));
    oilLeak.solutions.push_back(Accept("Долить масло", 60));
    oilLeak.solutions.push_back(Critical("Увеличить давление", 30));
    oilLeak.solutions.push_back(Failure("Заменить масло на более густое", 0));

    return oilLeak;
}


Malfunction Compressor::createOverPressure() {
    // неисправность "Высокое давление"
    Malfunction overPressure;
    overPressure.name = "Over Pressure";
    overPressure.description = "Pressure above 9";
    
    // Условия
    overPressure.conditions["Pressure"] = {6.0, 9.0};
    
    // Создание решений
    overPressure.solutions.push_back(Optimal("Сбросить давление → Проверить реле → Отрегулировать настройки", 100));
    overPressure.solutions.push_back(Accept("Отключить нагрузку → Проверить манометр", 75));
    overPressure.solutions.push_back(Critical("Стравить давление через аварийный клапан", 40));
    overPressure.solutions.push_back(Failure("Отключить датчик давления", 0));
   
    return overPressure;
}


Malfunction Compressor::createOftenStart() {
    // неисправность "Частые включения"
    Malfunction overPressure;
    overPressure.name = "Often Start";
    overPressure.description = "Start Counter above 10";
    
    // Условия
    overPressure.conditions["Start Counter"] = {0, 10.0};
    
    // Создание решений
    overPressure.solutions.push_back(Optimal("Проверить утечки → Проверить ресивер → Настроить реле давления", 100));
    overPressure.solutions.push_back(Accept("Проверить давление отключения → Проверить клапаны", 85));
    overPressure.solutions.push_back(Critical("Отключить автовключение → Включать вручную", 25));
    overPressure.solutions.push_back(Failure("Заблокировать выключатель во включенном положении", 0));
   
    return overPressure;
}
