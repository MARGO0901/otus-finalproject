#include <observers/penguin.h>
#include <consolemanager.h>

Penguin::Penguin(bool is_drawing) : mood_("normal"), currentMessage_("Привет! Введи 'start', чтобы начать") {
    if (is_drawing)
        drawPenguin();
}


void Penguin::drawPenguin() {
    std::string emoji = "  ('v')   " + currentMessage_ + "\n";

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
    if (mood_ == "happy") {
        emoji = "  (^v^)   " + currentMessage_;
    } else if(mood_ == "sad") {
        emoji = "  (´v`)   " + currentMessage_;
    } else if (mood_ == "angry") {
        emoji = "  (>v<)   " + currentMessage_;
    } else {
        emoji = "  ('v')   " + currentMessage_;
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
    currentMessage_ = message;
    updateFace();
}


void Penguin::setMood(const std::string &newMood)
{
    mood_ = newMood;
}