#pragma once

#include "device.h"

class Pump : public Device {
public:
    Pump();

    void update() override;
    int applySolution(const Solution& solution) override;

    std::vector<Malfunction> createMalfunctions() override; 

private:
    Malfunction createOverheat();
    Malfunction createVibration();
    Malfunction createNoPressure();
};