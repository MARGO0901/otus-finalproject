#pragma once

#include "malfunction.h"

class Device {
protected:
    std::string name;
    enum State { WORKING, BROCKEN, FIXING} state;
    std::map<std::string, double> params;

public:
    Device(const std::string & name) : name(name), state(WORKING) {}

    //обновить таймеры
    virtual void update();

    //Проверить, есть ли неисправности
    virtual std::vector<Malfunction> checkMalfunctions() = 0;

    //Применить решение, вернуть баллы
    virtual int applySolution(const Solution& solution) = 0;

    std::string getName() const { return name; }
    State getState() const { return state; }
    const std::map<std::string, double> &getParams() const { return params; }

    //Метод для изменения параметров (для тестирования)
    void setParam(const std::string& key, double &value) {
        params[key] = value;
    }

    virtual ~Device() = default;
};


class Pump : public Device {
public:
    Pump();

    void update() override;
    std::vector<Malfunction> checkMalfunctions() override;  
    int applySolution(const Solution& solution) override;

    std::vector<Malfunction> createPumpMalfunctions();

private:
    std::vector<Malfunction> malfunctions;
};