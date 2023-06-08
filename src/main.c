#include "raylib.h"

int main()
{
    const int screen_width = 1280;
    const int screen_height = 720;

    InitWindow(screen_width, screen_height, "Raylib Sandbox");
    SetTargetFPS(60);

    while (!WindowShouldClose()) {
        // update ...

        // draw
        BeginDrawing();
        ClearBackground(RAYWHITE);
        DrawText("Raylib", 190, 200, 20, LIGHTGRAY);
        EndDrawing();
    }

    CloseWindow();

    return 0;
}
