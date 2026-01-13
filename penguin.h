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
        
    void drawPenguin();
    void updateFace();

public:
    Penguin();

    void say(const std::string& message);
    void setMood(const std::string& newMood);
};