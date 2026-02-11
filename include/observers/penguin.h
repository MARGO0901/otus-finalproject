#pragma once

#include "observer.h"

#include <string>

class Penguin : public IObserver {
private:
    std::string mood_;       // "happy", "sad", "thinking"
    std::string currentMessage_;
    bool is_drawing_ = true;
        
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
        if (!newMood.empty()) {
            mood_ = newMood;
        }
        updateFace();
    }
    void updateBody() override {
        drawPenguin();
    }
};