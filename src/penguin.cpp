#include <penguin.h>
#include <consolemanager.h>

Penguin::Penguin() : mood("normal"), currentMessage("Привет! Введи 'start', чтобы начать") {
    ConsoleManager::clearScreen();
    drawPenguin();
}


void Penguin::drawPenguin() {
    // Пингвин всегда рисуется с начала экрана
    ConsoleManager::gotoPenguinLine();

    ConsoleManager::print("    -\n"); 

    std::string emoji = "  ('v')   " + currentMessage + "\n";
    ConsoleManager::print(emoji);

    ConsoleManager::print(" //   \\\\\n");
    ConsoleManager::print(" (\\_=_/)\n");
}


// Обновляем только лицо и сообщение
void Penguin::updateFace() {
    // Перемещаемся к лицу пингвина (строка 2)
    ConsoleManager::clearPenguinFaceLine();
    ConsoleManager::gotoPenguinFaceLine();
  
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
    ConsoleManager::print(emoji);
    ConsoleManager::gotoInputLine();
}


void Penguin::say(const std::string &message)
{
    currentMessage = message;
    updateFace();
}


void Penguin::setMood(const std::string &newMood)
{
    mood = newMood;
}