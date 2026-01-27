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
public:
    enum class State {
        MENU, 
        PLAYING, 
        GAME_OVER, 
        EXIT
    };

private:
    std::thread inputThread;    // ввод пользователя
    std::thread mainThread;     // основная логика ( UI + уровень )
    std::thread deviceThread;   // обновление состояния приборов

    // состояние игры
    State currentState{State::MENU};
    std::atomic<bool> running{false};

    // синхронизация
    std::mutex startMutex;
    std::condition_variable startCV;

    // ввод
    std::mutex inputMutex;
    std::string inputBuffer;

    // устройства
    std::vector<std::unique_ptr<Device>> devices;
    std::mutex deviceMutex;
    bool needsRedrawDevice{false};     // Флаг необходимости перерисовки приборов

    // игровые данные
    int currentLevel;
    Penguin penguin;
    int totalScore;

    // Проблемы
    std::vector<DeviceProblem> currentProblems;

public:
    Game(const std::vector<std::string>& deviceNames);
    ~Game();

    void start();
    void stop();

private:
    // потоки
    void inputLoop();
    void mainGameLoop();
    void deviceUpdateLoop();

    // обработка команд
    void processCommand(std::string &cmd);
    void handleMenuCommand(const std::string& command);
    void handleGameCommand(const std::string& command);

    // игровая логика
    //void showMainMenu();
    void runLevelInLoop(int level);
    void generateProblems(int count);
    void showDevicesStatus();
    bool getCommand(std::string& cmd);
    void processUserInput(const std::string& input);
    bool checkSolution(int deviceIndex, const std::string& action);
    bool isActionValidForMalfunction(const std::string& action, const Malfunction& malfunction);
    void updateScore(bool correct);
    
           
    // результаты    
    std::string getQualification() const;
    int getTotalScore() const { return totalScore; }   
};
