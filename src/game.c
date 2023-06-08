#include <stdbool.h>

#include "game.h"
#include "raylib.h"

Game game = { 0 };

void initialize() {
    game.window.width = 1280;
    game.window.height = 720;

    InitWindow(game.window.width, game.window.height, "Raylib Sandbox");
    SetTargetFPS(60);

    game.assets.texture = LoadTexture("data/game-space.png");

    game.running = true;
}

void input() {
    // ...
}

void update() {
    if (WindowShouldClose()) {
        game.running = false;
        return;
    }

    // ...
}

void draw() {
    Texture texture = game.assets.texture;

    BeginDrawing();
    {
        ClearBackground(SKYBLUE);
        DrawTexture(texture, (game.window.width - texture.width) / 2, (game.window.height- texture.height) / 2, WHITE);
        DrawText("Raylib", 150, 280, 80, LIGHTGRAY);
    }
    EndDrawing();
}

void shutdown() {
    UnloadTexture(game.assets.texture);

    game = (Game) { 0 };

    CloseWindow();
}
