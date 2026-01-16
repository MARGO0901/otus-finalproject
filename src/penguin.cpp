#include <penguin.h>
#include <consolemanager.h>

Penguin::Penguin() : mood("normal"), currentMessage("Привет!") {
    ConsoleManager::clearScreen();
    drawPenguin();
}


void Penguin::drawPenguin() {
    // Пингвин всегда рисуется с начала экрана
    ConsoleManager::gotoxy(0, 1);

    ConsoleManager::print("    -\n"); 

    std::string emoji = "  ('v')   " + currentMessage + "\n";
    ConsoleManager::print(emoji);

    ConsoleManager::print(" //   \\\\\n");
    ConsoleManager::print(" (\\_=_/)\n");
}


// Обновляем только лицо и сообщение
void Penguin::updateFace() {
    // Перемещаемся к лицу пингвина (строка 2)
    ConsoleManager::clearInputLine(2);
    ConsoleManager::gotoxy(0, 2);
  
    std::string emoji;
    if (mood == "happy") {
        emoji = "  (^v^)   " + currentMessage;
    } else if(mood == "sad") {
        emoji = "  (-v-)   " + currentMessage;
    } else {
        emoji = "  ('v')   " + currentMessage;
    }
    ConsoleManager::print(emoji);
    ConsoleManager::gotoxy(2, 9);
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