#pragma once

#include <mutex>

#include "device.h"

class Pump : public Device {
public:
    Pump();

    int applySolution(const Solution& solution) override;
    std::vector<Malfunction> createMalfunctions() override; 

private:
    Malfunction createOverheat();
    Malfunction createVibration();
    Malfunction createNoPressure();

    std::mutex mtx;
};