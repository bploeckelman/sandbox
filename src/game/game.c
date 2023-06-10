#include <stdbool.h>
#include <stdio.h>

#include "game.h"
#include "raylib.h"

Game game = { 0 };

void initialize() {
    game.window.width = 1280;
    game.window.height = 720;

    InitWindow(game.window.width, game.window.height, "Raylib Sandbox");
    SetTargetFPS(60);

    load(&game.lib);
    game.lib.initialize();

    game.running = true;
}

void input() {
    game.lib.input();
}

void update() {
    if (WindowShouldClose()) {
        game.running = false;
        return;
    }

    game.lib.update();
}

void draw() {
    game.lib.draw();
}

void shutdown() {
    game.lib.shutdown();
    unload(&game.lib);

    game = (Game) { 0 };

    CloseWindow();
}
