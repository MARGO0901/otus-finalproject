#pragma once

#include "device.h"
#include "malfunction.h"

class Fan : public Device {
public:
    Fan();
    void update() override;
    int applySolution(const Solution& solution) override;
    std::vector<Malfunction> createMalfunctions() override;

private:
    Malfunction createLowRpm();
    Malfunction createOverheat();
    Malfunction createOverCurrent();
};

