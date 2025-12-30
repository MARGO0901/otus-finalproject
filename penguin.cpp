#include "penguin.h"

void Penguin::show() {
    if (mood == "happy") {
        std::cout << "  (^v^)   " << currentMessage << std::endl;
    } else if(mood == "sad") {
        std::cout << "  (uvu)   " << currentMessage << std::endl;
    } else {
        std::cout << "  ('v')   " << currentMessage << std::endl;
    }
    std::cout << " //   \\\\" << std::endl;
    std::cout << " (\\_=_/)" << std::endl;
}

void Penguin::say(const std::string& message) {
    currentMessage = message;
    show();
}

void Penguin::setMood(const std::string& newMood) {
    mood = newMood;
}