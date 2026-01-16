#pragma once

#include <mutex>
#include <iostream>

class ConsoleManager{
private:
    static std::mutex consoleMutex_;
public:
    ConsoleManager() = delete;

    static void gotoxy(int x, int y) {
        std::lock_guard<std::mutex> lock(consoleMutex_);
        std::cout << "\033[" << (y + 1) << ";" << (x + 1) << "H" << std::flush;
    }

    template<typename T>
    static void print(const T& value) {
        std::lock_guard<std::mutex> lock(consoleMutex_);
        std::cout << value << std::flush;
    }

     static void clearScreen() {
        std::lock_guard<std::mutex> lock(consoleMutex_);
        std::cout << "\033[2J\033[H" << std::flush;
    }
    
    static void hideCursor() {
        std::lock_guard<std::mutex> lock(consoleMutex_);
        std::cout << "\033[?25l" << std::flush;
    }
    
    static void showCursor() {
        std::lock_guard<std::mutex> lock(consoleMutex_);
        std::cout << "\033[?25h" << std::flush;
    }

    static void clearInputLine(int row) {
        std::lock_guard<std::mutex> lock(consoleMutex_);
        std::cout << "\033[" << row + 1 << ";1H\033[2K" << std::flush;
    }

    static void showPrompt(int row) {
        std::lock_guard<std::mutex> lock(consoleMutex_);
        //на позицию 
        std::cout << "\033[" << row + 1 << ";1H> \033[?25h" << std::flush;
        std::cout << "\033[" << (row + 1) << ";" << 2 << "H" << std::flush;
    }
};

inline std::mutex ConsoleManager::consoleMutex_;