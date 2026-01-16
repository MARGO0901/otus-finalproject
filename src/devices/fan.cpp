#include <devices/device.h>

Fan::Fan() : Device("Fan") {
    params = {
        {"Rpm", 5.0},
        {"Current", 70.0},
        {"Temperature", 55.0}
    };
    malfunctions = createFanMalfunctions();
}


void Fan::update() {
    //params["Temperature"] += (rand() % 3 - 1) * 0.5;
    //params["Press"] += (rand() % 5 - 2) * 0.1;
}


std::vector<Malfunction> Fan::createFanMalfunctions() {
    std::vector<Malfunction> malfunctions;
    return malfunctions;
}


int Fan::applySolution(const Solution& solution) {
    return 0;
}

std::vector<Malfunction> Fan::checkMalfunctions() {
    std::vector<Malfunction> malfunctions;
    return malfunctions;
}

