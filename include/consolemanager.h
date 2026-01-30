#pragma once

#include <mutex>
#include <iostream>
#include <string>

class ConsoleManager{
private:
    static std::mutex consoleMutex_;
    inline static int deviceCount_;

    // Константы для позиционирования
    static const int LEVEL_LINE = 1;
    static const int PENGUIN_LINE = 2;
    static const int PENGUIN_FACE_LINE = 3;
    static const int DEVICE_LINE = 7;
    
    // Динамические позиции (можно вычислять)
    static int getActionLine() {
        return DEVICE_LINE + 1 + deviceCount_ + 1;                // кол-во приборов + 1
    }

    static int getInputLine() {
        return DEVICE_LINE + 1 + deviceCount_ + 1 + 4 + 1;        // кол-во приборов + 1 + 4 варианта ответа + 1
    }
    
    static int getDebugLine() {
        return DEVICE_LINE + 1 + deviceCount_ + 1 + 4 + 1 + 1;    // кол-во приборов + 1 + 4 варианта ответа + 1 + строка ввода
    }

    static void gotoxy(int x, int y) {
        std::lock_guard<std::mutex> lock(consoleMutex_);
        std::cout << "\033[" << (y + 1) << ";" << (x + 1) << "H" << std::flush;
    }

public:
    ConsoleManager() = delete;

    // Метод для установки количества устройств
    static void setDeviceCount(int count) {
        std::lock_guard<std::mutex> lock(consoleMutex_);
        deviceCount_ = count;
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

    static void gotoInputLine() {
        gotoxy(2, getInputLine());
    }

    static void clearInputLine() {
        std::lock_guard<std::mutex> lock(consoleMutex_);
        std::cout << "\033[" << getInputLine() + 1 << ";1H\033[2K" << std::flush;
    }

    static void gotoDeviceLine() {
        gotoxy(0, DEVICE_LINE);
    }

    static void gotoActionLine() {
        gotoxy(0, getActionLine());
    }


    static void gotoPenguinLine() {
        gotoxy(0, PENGUIN_LINE);
    }

    static void gotoPenguinFaceLine() {
        gotoxy(0,PENGUIN_FACE_LINE);
    }

    static void clearPenguinFaceLine() {
        std::lock_guard<std::mutex> lock(consoleMutex_);
        std::cout << "\033[" << PENGUIN_FACE_LINE + 1 << ";1H\033[2K" << std::flush;
    }

    static void clearDeviceLine() {
        std::lock_guard<std::mutex> lock(consoleMutex_);
        for (int i = 0; i <= deviceCount_; ++i) {
            std::cout << "\033[" << DEVICE_LINE + i + 1 << ";1H\033[2K" << std::flush;
        }
    }

    static void clearActionArea() {
        std::lock_guard<std::mutex> lock(consoleMutex_);
        for (int i = 0; i < 4; ++i) {
            std::cout << "\033[" << getActionLine() + i + 1 << ";1H\033[2K" << std::flush;
        }
    }

    static void showPrompt() {
        std::lock_guard<std::mutex> lock(consoleMutex_);
        //на позицию 
        std::cout << "\033[" << getInputLine() + 1 << ";1H> \033[?25h" << std::flush;
        std::cout << "\033[" << (getInputLine() + 1) << ";" << 2 << "H" << std::flush;
    }

    static void savePosition() {
        std::lock_guard<std::mutex> lock(consoleMutex_);
        std::cout << "\033[s";
        std::cout.flush();
    }

    static void restorePosition() {
        std::lock_guard<std::mutex> lock(consoleMutex_);
        std::cout << "\033[u";
        std::cout.flush();
    }

    static void printDebug(const std::string& str, int num = 19) {
        savePosition();
        std::cout << "\033[" << num + 1 << ";1H\033[2K" << std::flush;
        gotoxy(0, num);
        print(str);
        restorePosition();
    }

    static void printLevel(int level, int score) {
        gotoxy(0, LEVEL_LINE);
        std::cout << "\033[" << LEVEL_LINE + 1 << ";1H\033[2K" << std::flush;
        print("Уровень: " + std::to_string(level) + " Счет: " + std::to_string(score));
    }

};

inline std::mutex ConsoleManager::consoleMutex_;