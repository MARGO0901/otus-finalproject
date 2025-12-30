#pragma once

#include <string>
#include <iostream>

class Penguin {
private:
    std::string mood;       // "happy", "sad", "thinking"
    std::string currentMessage;

public:
    Penguin() : mood("normal"), currentMessage("Hello") {};

    void show();
    void say(const std::string& message);
    void setMood(const std::string& newMood);
};