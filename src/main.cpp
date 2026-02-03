#include <game.h>

#include <iostream>

void setupConsoleWindow() {
    std::cout << "\033[8;35;140t";      // высота = 40, ширина = 120
}

int main() {

    setupConsoleWindow();

    Game game({"Fan", "Pump","Compressor"});
    game.start();

    // Ждем, пока running не станет false
    while (true) {
        // Проверяем состояние каждые 100 мс
        std::this_thread::sleep_for(std::chrono::milliseconds(100));

    }

    return 0;
}