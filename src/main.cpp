#include <game.h>

int main() {

    Game game({"Pump", "Fan", "Compressor"});

    game.start();

    //game.runLevel(1);
/*    if(game.getTotalScore() >= 200) {
        std::cout << "Level 1 completed!" << std::endl;
        
        // Уровень 2
        game.runLevel(2);
        if(game.getTotalScore() >= 600) {  // 200 + 400
            std::cout << "Level 2 completed!" << std::endl;
            
            // Уровень 3
            game.runLevel(3);
        }
    }

    std::cout << "Game over!" << std::endl;
    std::cout << "Your qualifications: " << game.getQualification() << std::endl;
*/

    // Ждем, пока running не станет false
    while (true) {
        // Проверяем состояние каждые 100 мс
        std::this_thread::sleep_for(std::chrono::milliseconds(100));

    }

    return 0;
}