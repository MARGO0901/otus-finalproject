#pragma once

#include "device.h"

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