#include <cmath>
#include <devices/device.h>

#include "consolemanager.h"

void Device::update() {
    for(auto& [param, value] : params_) {
        std::visit([param_ptr = &param, this](auto& val) -> void {
            optChangeParam(*param_ptr, val);
        }, value);            
    }

    applyMalfunctionEffect();
}


void Device::applyMalfunctionEffect() {
    for(const auto& malfunction : activeMalfunctions_) {
        applyMalfunction(malfunction);
    }
}


void Device::applyMalfunction(const Malfunction& malfunction) {
    /*
    {
        std::lock_guard<std::mutex> lock(ConsoleManager::getMutex());   
        ConsoleManager::printDebug("Device: " + this->name_ + " Applying malfunction: " + malfunction.name_);
    }*/

    for (auto& [param, value] : params_) {
        auto it = malfunction.conditions_.find(param.name_);
        if (it != malfunction.conditions_.end()) {
            /*
            {
                std::lock_guard<std::mutex> lock(ConsoleManager::getMutex());
                ConsoleManager::printDebug("  Parameter: " + param.name_ 
                        + ", range: [" + std::to_string(it->second.first) 
                        + ", " + std::to_string(it->second.second) + "]" , 21);
            }*/

            param.currentRange_ = it->second;
        }
    }
}


void Device::resetMalfunction() {
    for (auto& param_value : params_) {

        const auto& param = param_value.first; 
        auto& value = param_value.second;

        // Возвращаем текущие диапазоны к нормальным
        param.currentRange_ = param.optRange_;
        
        // Возвращаем значение в середину нормального диапазона
        std::visit([&param](auto& val) -> void {
            using T = std::decay_t<decltype(val)>;
            
            if constexpr (std::is_arithmetic_v<T>) {
                double target = (param.optRange_.first + param.optRange_.second) / 2.0;

                if constexpr (std::is_integral_v<T>) {
                    target = std::round(target);
                } else {
                    target = std::round(target * 10.0) / 10.0;
                }
                val = static_cast<T>(target);
            }
        }, value);
    }
    activeMalfunctions_.clear();
}

void Device::addMalfunctions(const Malfunction& malfunction) {
    activeMalfunctions_.push_back(malfunction);
}


