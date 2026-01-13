#pragma once

#include <memory>
#include <queue>

#include "penguin.h"
#include "device.h"


class Game {
private:
    Penguin penguin;
    int currentLevel;
    int totalScore;
    std::atomic<bool> running{true};
    std::vector<std::unique_ptr<Device>> devices;

    // Поток ввода
    std::thread inputThread;
    std::mutex mtx;
    std::condition_variable cv;
    std::string inputBuffer;

    // Проблемы
    std::vector<std::pair<Device*, Malfunction>> currentProblems;
    
    // Методы
    void inputLoop();

public:
    Game();
    ~Game();

    int getTotalScore() const { return totalScore; }

    void runLevel(int level);
    bool getCommand(std::string& cmd);
    void processUserInput(const std::string& input);
    void generateProblems(int count);
    void showDevicesStatus();
    bool checkSolution();
    void updateScore(bool correct);
    std::string getQualification() const;
};