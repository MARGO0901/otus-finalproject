#pragma once

#include <memory>
#include <thread>
#include <atomic>
#include <mutex>
#include <condition_variable>

#include "penguin.h"
#include "./devices/device.h"


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
    Game(const std::vector<std::string>& deviceNames);
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