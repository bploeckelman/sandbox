#include "raylib.h"

int main()
{
    const int screen_width = 1280;
    const int screen_height = 720;

    InitWindow(screen_width, screen_height, "Raylib Sandbox");
    SetTargetFPS(60);

    Texture texture = LoadTexture("data/game-space.png");

    while (!WindowShouldClose()) {
        // update
        {
            // ...
        }

        // draw
        BeginDrawing();
        {
            ClearBackground(SKYBLUE);
            DrawTexture(texture, (screen_width - texture.width) / 2, (screen_height - texture.height) / 2, WHITE);
            DrawText("Raylib", 190, 200, 80, GRAY);
        }
        EndDrawing();
    }

    UnloadTexture(texture);

    CloseWindow();

    return 0;
}
