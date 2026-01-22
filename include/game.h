#pragma once

#include <memory>
#include <thread>
#include <atomic>
#include <mutex>
#include <condition_variable>
#include <vector>

#include "penguin.h"
#include "./devices/device.h"


class Game {
private:
    Penguin penguin;
    int currentLevel;
    int totalScore;
    std::vector<std::unique_ptr<Device>> devices;
     std::atomic<bool> running{false};

    std::vector<std::thread> deviceThreads;
    std::thread uiThread;
    std::thread inputThread; 
   
    std::mutex startMutex;
    std::mutex deviceDrawMtx;
    std::mutex inputMtx;

    std::condition_variable startCV;
    std::atomic<bool> needsRedrawDevice{false};  // Флаг необходимости перерисовки приборов
    std::string inputBuffer;

    // Проблемы
    std::vector<std::pair<Device*, Malfunction>> currentProblems;
    
    // Методы
    void inputLoop();
    void deviceThread(std::size_t deviceIndex);
    void uiThreadFunc();
    void showDevicesStatus();
    bool getCommand(std::string& cmd);

    void processUserInput(const std::string& input);
    void generateProblems(int count);

    bool checkSolution();
    void updateScore(bool correct);
    std::string getQualification() const;

    int getTotalScore() const { return totalScore; }

public:
    Game(const std::vector<std::string>& deviceNames);
    ~Game();

    void start();
    void stop();

    void runLevel(int level);
};