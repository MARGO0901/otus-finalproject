#pragma once

#include <memory>
#include <thread>
#include <atomic>
#include <mutex>
#include <condition_variable>
#include <vector>

#include "malfunction.h"
#include "observers/penguin.h"
#include "devices/device.h"
#include "observers/observer.h"

struct CurrentTask {
    int deviceIndex_;
    Malfunction malfunction_;
    std::vector<Solution> shuffledSolutions_;
    int selectedSolutionIndex_ = -1;
    int correctPoints_ = 0;          //очки за ответ
};


class Game {
public:
    enum class State {
        MENU, 
        PLAYING
    };

private:
    std::thread inputThread_;    // ввод пользователя
    std::thread mainThread_;     // основная логика ( UI + уровень )
    std::thread deviceThread_;   // обновление состояния приборов

    // состояние игры
    State currentState_{State::MENU};
    std::atomic<bool> running_{false};

    // синхронизация
    std::mutex startMutex_;
    std::condition_variable startCV_;

    // ввод
    std::mutex inputMutex_;
    std::string inputBuffer_;

    // устройства
    std::vector<std::unique_ptr<Device>> devices_;
    std::mutex deviceMutex_;
    bool needsRedrawDevice_{false};     // Флаг необходимости перерисовки приборов

    // игровые данные
    int currentLevel_;
    int totalScore_;                     // общий балл за игру
    int maxScore_;                  // максимальное кол-во баллов на данный момент

    // наблюдатели
    ObserverManager observerManager_;
    std::shared_ptr<Penguin> penguin_;

public:
    Game(const std::vector<std::string>& deviceNames);

    void startGame();
    void exitGame();
    bool isRunning() const;

    //перерисовка
    void redrawAll();

private:
    // потоки
    void inputLoop();
    void mainGameLoop();
    void deviceUpdateLoop();

    // обработка команд
    void processCommand(std::string &cmd);
    void handleMenuCommand(const std::string& command);

    // игровая логика
    void runLevelInLoop();
    std::vector<CurrentTask> generateProblemsWithSolutions(int count);
    bool askToSelectDevice(int& selectedIndex);
    void showProblemAndSolutions(const CurrentTask& task);
    bool getUserSolutionChoice(CurrentTask& task);
    void checkAndScore(CurrentTask& task);
    void updateDeviceStatusWithTimer(std::chrono::steady_clock::time_point& lastUpdate);

    void clearDeviceMalfunctions();
    void showDevicesStatus();

    bool getCommand(std::string& cmd);

    void updateScore(int points);
    void completeLevel();
    void stopGame(const std::string& mood, const std::string& msg);
          
    // результаты    
    std::string getQualification() const;
};