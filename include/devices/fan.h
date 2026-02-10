#pragma once

#include "device.h"
#include "malfunction.h"

class Fan : public Device {
public:
    Fan();

    std::vector<Malfunction> createMalfunctions() override;

private:
    Malfunction createLowRpm();
    Malfunction createOverheat();
    Malfunction createOverCurrent();
};

