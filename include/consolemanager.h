#pragma once

#include <mutex>
#include <iostream>

class ConsoleManager{
private:
    static std::mutex consoleMutex_;

    static void gotoxy(int x, int y) {
        std::lock_guard<std::mutex> lock(consoleMutex_);
        std::cout << "\033[" << (y + 1) << ";" << (x + 1) << "H" << std::flush;
    }
public:
    ConsoleManager() = delete;

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
        gotoxy(2, 15);
    }

    static void clearInputLine() {
        std::lock_guard<std::mutex> lock(consoleMutex_);
        std::cout << "\033[" << 16 << ";1H\033[2K" << std::flush;
    }

    static void gotoDeviceLine() {
        gotoxy(0, 6);
    }

    static void gotoActionLine() {
        gotoxy(0, 11);
    }


    static void gotoPenguinLine() {
        gotoxy(0,1);
    }

    static void gotoPenguinFaceLine() {
        gotoxy(0,2);
    }

    static void clearPenguinFaceLine() {
        std::lock_guard<std::mutex> lock(consoleMutex_);
        std::cout << "\033[" << 3 << ";1H\033[2K" << std::flush;
    }

    static void clearDeviceLine() {
        std::lock_guard<std::mutex> lock(consoleMutex_);
        std::cout << "\033[" << 6 << ";1H\033[2K" << std::flush;
        std::cout << "\033[" << 7 << ";1H\033[2K" << std::flush;
        std::cout << "\033[" << 8 << ";1H\033[2K" << std::flush;
        std::cout << "\033[" << 9 << ";1H\033[2K" << std::flush;
    }

    static void clearActionArea() {
        std::lock_guard<std::mutex> lock(consoleMutex_);
        std::cout << "\033[" << 12 << ";1H\033[2K" << std::flush;
        std::cout << "\033[" << 13 << ";1H\033[2K" << std::flush;
        std::cout << "\033[" << 14 << ";1H\033[2K" << std::flush;
        std::cout << "\033[" << 15 << ";1H\033[2K" << std::flush;
    }

    static void showPrompt() {
        std::lock_guard<std::mutex> lock(consoleMutex_);
        //на позицию 
        std::cout << "\033[" << 16 << ";1H> \033[?25h" << std::flush;
        std::cout << "\033[" << (16) << ";" << 2 << "H" << std::flush;
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

    static void printDebug(const std::string& str, int num = 17) {
        savePosition();
        std::cout << "\033[" << num + 1 << ";1H\033[2K" << std::flush;
        gotoxy(0, num);
        print(str);
        restorePosition();
    }
};

inline std::mutex ConsoleManager::consoleMutex_;