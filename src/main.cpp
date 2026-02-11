#include <atomic>
#include <csignal>
#include <game.h>

#include <iostream>
#include <locale>
#include <csignal>

void setupConsoleWindow() {
    std::cout << "\033[8;35;140t";      // высота = 35, ширина = 140

    std::locale::global(std::locale(""));
    std::cout.imbue(std::locale());
    
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
}


// Обработчик изменения размера
void setupSignalHandler(std::atomic<bool>& redraw_flag) {
    static std::atomic<bool>* flag_ptr = nullptr;
    flag_ptr = &redraw_flag;

    signal(SIGWINCH, [](int sig) {
        static bool ignore_first = true;

        if (ignore_first) {
            ignore_first = false;
            return;
        }
        
        if (flag_ptr) {
            *flag_ptr = true;
        }
    });
}


int main() {
    setupConsoleWindow();

    Game game({"Насос", "Вентилятор","Компрессор"});

    std::atomic<bool> needs_redraw{false};
    setupSignalHandler(needs_redraw);
    
    game.startGame();

    // Ждем, пока running не станет false
    while (game.isRunning()) {
        // Проверяем состояние каждые 100 мс
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        if (needs_redraw) {
            game.redrawAll();
            needs_redraw = false;
        }
    }

    // После выхода из цикла даем время на завершение
    std::this_thread::sleep_for(std::chrono::milliseconds(200));

    game.exitGame();

    return 0;
}