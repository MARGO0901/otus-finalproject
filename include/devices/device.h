#pragma once

#include <algorithm>
#include <variant>

#include <malfunction.h>
#include "deviceparameter.h"

using ParamValue = std::variant<double, int>;

class Device {
protected:
    std::string name_;
    enum State { WORKING, BROCKEN, FIXING} state;
    std::unordered_map<DeviceParameter, ParamValue> params;
    std::vector<Malfunction> malfunctions;

    template<typename T>
    void optChangeParam(const DeviceParameter &param, T &value) {
        if constexpr (std::is_same_v<T, double>) {
            // Генерирация изменений ±5-10%
            double changePercent = ((rand() % 2 ? -1 : 1) * (5 + rand() % 6)) / 100.0;  
            double newValue = value * (1.0 + changePercent); 
            value = std::clamp(newValue, param.optRange_.first, param.optRange_.second);
        }
        else if constexpr (std::is_same_v<T, int>) {
            int change = (rand() % 2 ? -1 : 1) * (1 + rand() % 2);
            int newValue = value + change;
            value = std::clamp(newValue, 
                static_cast<int>(param.optRange_.first),
                static_cast<int>(param.optRange_.second));
        }
    }

    // Метод для изменения параметров
    template<typename T>
    bool setParamValue(const std::string& name, T &newValue) {
        for (auto& [param, value] : params) {
            if(param.name_ == name) {
                if(auto* ptr = std::get_if<T>(&value)) {
                    *ptr = newValue;
                    return true;
                }
            }
        }
        return false;
    }

public:
    Device(const std::string & name) : name_(name), state(WORKING) {}
    virtual ~Device() = default;

    // Создать список возможных неисправностей и их решений
    virtual std::vector<Malfunction> createMalfunctions() = 0;

    // Обновить таймеры
    virtual void update() {
        for(auto& [param, value] : params) {
            std::visit([param_ptr = &param, this](auto& val) -> void {
                optChangeParam(*param_ptr, val);
            }, value);            
        }
    }

    // Применить решение, вернуть баллы
    virtual int applySolution(const Solution& solution) = 0;

    // Проверить, есть ли неисправности
    std::vector<Malfunction> checkMalfunctions() {
        std::vector<Malfunction> active;
        // for(auto& malfunction : malfunctions) {
        //     if(malfunction.isActive(params)) {
        //         active.push_back(malfunction);
        //     }
        // }
        return active;
    }

    std::string getName() const { return name_; }
    State getState() const { return state; }

    const std::unordered_map<DeviceParameter, ParamValue> &getParams() const { return params; }
};