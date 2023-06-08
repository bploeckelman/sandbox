#include "game/game.h"

#define STB_SPRINTF_IMPLEMENTATION
#include "../lib/stb_sprintf.h"

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
