#include "game.h"

int main()
{
    initialize();
    while (game.running) {
        input();
        update();
        draw();
    }
    shutdown();
    return 0;
}
