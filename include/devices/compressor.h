#pragma once

#include "device.h"
#include "malfunction.h"

class Compressor : public Device {
public:
    Compressor();

    std::vector<Malfunction> createMalfunctions() override;

private:
    Malfunction createOilLeak();
    Malfunction createOverPressure();
    Malfunction createOftenStart();

};