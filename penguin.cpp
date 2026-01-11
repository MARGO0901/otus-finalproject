#include "penguin.h"

#include <iostream>
#include <atomic>
#include <chrono>

// Перемещение курсора
void gotoxy(int x, int y) {
    std::cout << "\033[" << (y + 1) << ";" << (x + 1) << "H" << std::flush;
}

// Очистить текущую строку
void clear_line() {
    std::cout << "\033[2K" << std::flush;
}

void hide_cursor() {
    std::cout << "\033[?25l" << std::flush;
}

void show_cursor() {
    std::cout << "\033[?25h" << std::flush;
}

// Очистить экран
void clearScreen() {
    // \033[2J - очистить весь экран
    // \033[3J - очистить буфер прокрутки (не все терминалы)
    // \033[H - переместить курсор в начало (0,0)
    std::cout << "\033[2J\033[3J\033[H" << std::flush;
}


// Установить строку ввода
void Penguin::setInputRow(int row) {
    inputRow = row;
}

// Получить текущую строку ввода
int Penguin::getInputRow() const {
    return inputRow;
}


Penguin::Penguin() : mood("normal"), currentMessage("Привет!"), running(true) {
    // Очищаем и рисуем пингвина
    std::cout << "\033[2J\033[H" << std::flush;
    std::cout << "\033[?25l" << std::flush;
    
    drawPenguin();
    
    // Показываем курсор в области ввода
    gotoxy(0, 5);
    std::cout << "Введите команду: " << std::endl;
    std::cout << "\033[?25h" << std::flush;
    
    // Запускаем поток для чтения ввода
    //inputThread = std::thread(&Penguin::inputLoop, this);
}


Penguin::~Penguin() {
    running = false;
    if (inputThread.joinable()) {
        inputThread.join();
    }
    std::cout << "\033[?25h\033[0m\033[2J\033[H" << std::flush;
}

void Penguin::drawPenguin() {
    gotoxy(0, 0);
    std::cout << "    -" << std::endl;
    std::cout << "  ('v')   " << currentMessage << std::endl;
    std::cout << " //   \\\\" << std::endl;
    std::cout << " (\\_=_/)" << std::endl;
}


// Обновляем только лицо и сообщение
void Penguin::updateFace() {
    // Сохраняем позицию курсора
    std::cout << "\033[s" << std::flush;
    
    gotoxy(0, 1);
    clear_line();
    
    if (mood == "happy") {
        std::cout << "  (^v^)   " << currentMessage;
    } else if(mood == "sad") {
        std::cout << "  (-v-)   " << currentMessage;
    } else {
        std::cout << "  ('v')   " << currentMessage;
    }
    
    // Восстанавливаем позицию курсора
    std::cout << "\033[u" << std::flush;
}



/*
// Блокирующее ожидание команды
void Penguin::waitForCommand(std::string& cmd) {
    std::unique_lock<std::mutex> lock(mtx);
    cv.wait(lock, [this]() { return !inputBuffer.empty() || !running; });
    
    if (!inputBuffer.empty()) {
        cmd = inputBuffer;
        inputBuffer.clear();
    } else {
        cmd = "exit";
    }
}*/

void Penguin::show()
{
    updateFace();
}

void Penguin::say(const std::string &message)
{
    currentMessage = message;
    show();
}

void Penguin::setMood(const std::string &newMood)
{
    mood = newMood;
}