#pragma once

#include "observer.h"

#include <string>

class Penguin : public IObserver {
private:
    std::string mood;       // "happy", "sad", "thinking"
    std::string currentMessage;
    bool is_drawing = true;
        
    void drawPenguin();
    void updateFace();
    void displayMessage(const std::string& message);
    void setMood(const std::string& newMood);

public:
    Penguin(bool is_drawing = true);

    void updateMessage(const std::string& message) override {
        // Это сообщение для показа пингвином
        displayMessage(message);
    }
    void updateMood(const std::string& newMood) override {
        // Это сообщение для показа пингвином
        mood = newMood;
        updateFace();
    }
};