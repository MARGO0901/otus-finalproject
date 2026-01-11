#pragma once

#include <string>
#include <iostream>
#include <thread>
#include <atomic>
#include <mutex>
#include <condition_variable>

class Penguin {
private:
    std::string mood;       // "happy", "sad", "thinking"
    std::string currentMessage;

    
    std::thread inputThread;
    std::atomic<bool> running;
    std::condition_variable cv;

    int inputRow;  // строка для ввода
    
    void drawPenguin();
    void updateFace();

    void setInputRow(int row);  // Установить строку ввода
    int getInputRow() const;    // Получить текущую строку ввода

public:
    Penguin();
    ~Penguin();

    void show();
    void say(const std::string& message);
    void setMood(const std::string& newMood);

    // Получить введенную команду (неблокирующий)
    void waitForCommand(std::string& cmd);
};