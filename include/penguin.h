#pragma once

#include <string>

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