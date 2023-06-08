#ifndef RAYLIB_SANDBOX_GAME_H
#define RAYLIB_SANDBOX_GAME_H

#include "raylib.h"

typedef struct Game {

    bool running;

    struct Window {
        int width;
        int height;
    } window;

    struct Assets {
        Texture texture;
    } assets;

} Game;

extern Game game;

void initialize();
void input();
void update();
void draw();
void shutdown();

#endif //RAYLIB_SANDBOX_GAME_H
