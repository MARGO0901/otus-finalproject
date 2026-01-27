#pragma once

#include <algorithm>
#include <variant>

#include <malfunction.h>
#include "deviceparameter.h"

using ParamValue = std::variant<double, int>;

struct DeviceProblem {
    int deviceIndex;
    Malfunction malfunction;
};


class Device {
protected:
    std::string name_;
    enum State { WORKING, BROCKEN, FIXING} state;
    std::unordered_map<DeviceParameter, ParamValue> params;
    std::vector<Malfunction> malfunctions;
    std::vector<Malfunction> activeMalfunctions;

    template<typename T>
    void optChangeParam(const DeviceParameter &param, T &value) {
        if constexpr (std::is_same_v<T, double>) {
            // Генерирация изменений ±5-10%
            double changePercent = ((rand() % 2 ? -1 : 1) * (5 + rand() % 6)) / 100.0;  
            double newValue = value * (1.0 + changePercent); 
            value = std::clamp(newValue, param.currentRange_.first, param.currentRange_.second);
        }
        else if constexpr (std::is_same_v<T, int>) {
            int change = (rand() % 2 ? -1 : 1) * (1 + rand() % 2);
            int newValue = value + change;
            value = std::clamp(newValue, 
                static_cast<int>(param.currentRange_.first),
                static_cast<int>(param.currentRange_.second));
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

    // Создать список возможных неисправностей и их решений
    virtual std::vector<Malfunction> createMalfunctions() = 0;
    // Применить решение, вернуть баллы
    virtual int applySolution(const Solution& solution) = 0;

public:
    Device(const std::string & name) : name_(name), state(WORKING) {}
    virtual ~Device() = default;

    // Обновить параметры
    virtual void update();
    // Установить неисправность в прибор
    void addMalfunctions(const Malfunction& malfunction);

    const std::vector<Malfunction>& getActiveMalfunctions() const {
        return activeMalfunctions; 
    }
    std::vector<Malfunction> getMalfunctions() const {
        return malfunctions;  // Возврается список созданный в конструкторе
    }
    std::string getName() const { return name_; }
    State getState() const { return state; }
    const std::unordered_map<DeviceParameter, ParamValue> &getParams() const { return params; }

private:
    void applyMalfunctionEffect();
    void applyMalfunction(const Malfunction& malfunction);

    // Метод для сброса неисправности
    void resetMalfunction();
};