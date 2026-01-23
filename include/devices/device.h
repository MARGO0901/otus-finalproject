#pragma once

#include <algorithm>
#include <malfunction.h>
#include "deviceparameter.h"

class Device {
protected:
    std::string name_;
    enum State { WORKING, BROCKEN, FIXING} state;
    std::unordered_map<DeviceParameter, double> params;
    std::vector<Malfunction> malfunctions;
    std::pair<double, double> norma;

    void optChangeParam(const DeviceParameter &param, double &value) {
        // Генерируем изменение ±5-10%
        double changePercent = ((rand() % 2 ? -1 : 1) * (5 + rand() % 6)) / 100.0;        
        // Применяем изменение
        double newValue = value * (1.0 + changePercent);       
        // Ограничиваем оптимальным диапазоном
        value = std::clamp(newValue, param.optRange_.first, param.optRange_.second);
    }

    // Метод для изменения параметров
    bool setParamValue(const std::string& name, double &newValue) {
        for (auto& [param, value] : params) {
            if(param.name_ == name) {
                value = newValue;
                return true;
            }
        }
        return false;
    }


public:
    Device(const std::string & name) : name_(name), state(WORKING) {}

    // Создать список возможных неисправностей и их решений
    virtual std::vector<Malfunction> createMalfunctions() = 0;

    // Обновить таймеры
    virtual void update() = 0;

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
    const std::unordered_map<DeviceParameter, double> &getParams() const { return params; }

    

    virtual ~Device() = default;
};