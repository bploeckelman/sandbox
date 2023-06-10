#ifndef RAYLIB_SANDBOX_GAME_H
#define RAYLIB_SANDBOX_GAME_H

#include "raylib.h"
#include "../gamelib/gamelib.h"

typedef struct Game {

    bool running;

    struct Window {
        int width;
        int height;
    } window;

    GameLib lib;

} Game;

extern Game game;

void initialize();
void input();
void update();
void draw();
void shutdown();

#endif //RAYLIB_SANDBOX_GAME_H
