#include <devices/device.h>

#include "consolemanager.h"

void Device::update() {
    for(auto& [param, value] : params) {
        std::visit([param_ptr = &param, this](auto& val) -> void {
            optChangeParam(*param_ptr, val);
        }, value);            
    }

    applyMalfunctionEffect();
}


void Device::applyMalfunctionEffect() {
    for(const auto& malfunction : activeMalfunctions) {
        applyMalfunction(malfunction);
    }
}


void Device::applyMalfunction(const Malfunction& malfunction) {
    ConsoleManager::printDebug("Device: " + this->name_ + " Applying malfunction: " + malfunction.name);

    for (auto& param_value : params) {
        DeviceParameter& param = const_cast<DeviceParameter&>(param_value.first); 
        auto& value = param_value.second;

        auto it = malfunction.conditions.find(param.name_);
        if (it != malfunction.conditions.end()) {

            ConsoleManager::printDebug("  Parameter: " + param.name_ 
                    + ", range: [" + std::to_string(it->second.first) 
                    + ", " + std::to_string(it->second.second) + "]" , 13);

            param.currentRange_ = it->second;
        }
    }
}


void Device::resetMalfunction() {
    for (auto& param_value : params) {

        DeviceParameter& param = const_cast<DeviceParameter&>(param_value.first); 
        auto& value = param_value.second;

        // Возвращаем текущие диапазоны к нормальным
        param.currentRange_ = param.optRange_;
        
        // Возвращаем значение в середину нормального диапазона
        std::visit([&param](auto& val) -> void {
            using T = std::decay_t<decltype(val)>;
            if constexpr (std::is_arithmetic_v<T>) {
                double target = (param.optRange_.first + param.optRange_.second) / 2.0;
                val = static_cast<T>(target);
            }
        }, value);
    }
    activeMalfunctions.clear();
}

void Device::addMalfunctions(const Malfunction& malfunction) {
    activeMalfunctions.push_back(malfunction);
}


