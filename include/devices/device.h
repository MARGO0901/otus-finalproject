#pragma once

#include <malfunction.h>

class Device {
protected:
    std::string name;
    enum State { WORKING, BROCKEN, FIXING} state;
    std::map<std::string, double> params;
    std::vector<Malfunction> malfunctions;

public:
    Device(const std::string & name) : name(name), state(WORKING) {}

    // Создать список возможных неисправностей и их решений
    virtual std::vector<Malfunction> createMalfunctions() = 0;

    // Обновить таймеры
    virtual void update() = 0;

    // Применить решение, вернуть баллы
    virtual int applySolution(const Solution& solution) = 0;

    // Проверить, есть ли неисправности
    std::vector<Malfunction> checkMalfunctions() {
        std::vector<Malfunction> active;
        for(auto& malfunction : malfunctions) {
            if(malfunction.isActive(params)) {
                active.push_back(malfunction);
            }
        }
        return active;
    }

    std::string getName() const { return name; }
    State getState() const { return state; }
    const std::map<std::string, double> &getParams() const { return params; }

    // Метод для изменения параметров
    void setParam(const std::string& key, double &value) {
        params[key] = value;
    }

    virtual ~Device() = default;
};