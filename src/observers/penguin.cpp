#include <observers/penguin.h>
#include <consolemanager.h>

Penguin::Penguin() : mood("normal"), currentMessage("Привет! Введи 'start', чтобы начать") {
    {
        std::lock_guard<std::mutex> lock(ConsoleManager::getMutex());
        ConsoleManager::clearScreen();
    }
    drawPenguin();
}


void Penguin::drawPenguin() {
    std::string emoji = "  ('v')   " + currentMessage + "\n";

    std::lock_guard<std::mutex> lock(ConsoleManager::getMutex());

    ConsoleManager::gotoPenguinLine();
    ConsoleManager::print("    -\n"); 
    ConsoleManager::print(emoji);
    ConsoleManager::print(" //   \\\\\n");
    ConsoleManager::print(" (\\_=_/)\n");
}


// Обновляем только лицо и сообщение
void Penguin::updateFace() {
    std::string emoji;
    if (mood == "happy") {
        emoji = "  (^v^)   " + currentMessage;
    } else if(mood == "sad") {
        emoji = "  (´v`)   " + currentMessage;
    } else if (mood == "angry") {
        emoji = "  (>v<)   " + currentMessage;
    } else {
        emoji = "  ('v')   " + currentMessage;
    }

    std::lock_guard<std::mutex> lock(ConsoleManager::getMutex());
    // Перемещаемся к лицу пингвина (строка 2)
    ConsoleManager::clearPenguinFaceLine();
    ConsoleManager::gotoPenguinFaceLine();
  
    ConsoleManager::print(emoji);
    ConsoleManager::gotoInputLine();
}


void Penguin::displayMessage(const std::string &message)
{
    currentMessage = message;
    updateFace();
}


void Penguin::setMood(const std::string &newMood)
{
    mood = newMood;
}