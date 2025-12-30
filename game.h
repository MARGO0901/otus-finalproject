#pragma once

#include <memory>

#include "penguin.h"
#include "device.h"

class Game {
private:
    int currentLevel;
    int totalScore;
    std::vector<std::unique_ptr<Device>> devices;
    Penguin penguin;

    //Текущие проблемы { прибор, неисправность}
    std::vector<std::pair<Device*, Malfunction>> currentProblems;
public:
    Game();

    void runLevel(int level);
    void generateProblems(int count);
    void showDevicesStatus();
    int getTotalScore() const { return totalScore; }

    std::string getQualification() const;
};