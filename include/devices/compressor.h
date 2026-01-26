#pragma once

#include "device.h"
#include "malfunction.h"

class Compressor : public Device {
public:
    Compressor();
    int applySolution(const Solution& solution) override;
    std::vector<Malfunction> createMalfunctions() override;

private:
    Malfunction createOilLeak();
    Malfunction createOverPressure();
    Malfunction createOftenStart();

};