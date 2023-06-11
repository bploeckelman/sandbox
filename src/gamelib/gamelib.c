#include <stdio.h>

#include "../state/state.h"

#define STB_SPRINTF_IMPLEMENTATION
#include "../../lib/stb_sprintf.h"

#ifdef _MSC_VER
    #define EXPORT __declspec(dllexport)
#elif __GNUC__ >= 4
    #define EXPORT __attribute__((visibility("default")))
#else
    #define EXPORT
#endif

EXPORT void init(GameState *state) {
    printf("INFO: GAMELIB: initialize\n");

    InitWindow(1280, 720, "Raylib Sandbox");
    SetTargetFPS(60);

    state->assets.textures[0] = LoadTexture("assets/game-pads.png");
    state->assets.textures[1] = LoadTexture("assets/game-space.png");
    state->assets.textureIndex = 1;

    state->running = true;
}

// TODO - separate update/input/draw
EXPORT void update(GameState *state) {
    if (WindowShouldClose()) {
        printf("Window should close");
        state->running = false;
        return;
    }

    if (IsKeyPressed(KEY_SPACE)) {
        state->assets.textureIndex = (state->assets.textureIndex + 1) % NUM_TEXTURES;
    }
    if (IsKeyPressed(KEY_ENTER)) {
        state->assets.textureIndex = (state->assets.textureIndex + 1) % NUM_TEXTURES;
    }

    BeginDrawing();
    {
        Texture texture = state->assets.textures[state->assets.textureIndex];
        int window_width = GetScreenWidth();
        int window_height = GetScreenHeight();

        ClearBackground(SKYBLUE);
        DrawTexture(texture, (window_width - texture.width) / 2, (window_height- texture.height) / 2, WHITE);
        DrawText("Raylib", 150, 280, 80, LIGHTGRAY);
    }
    EndDrawing();
}

EXPORT void shutdown(GameState *state) {
    printf("INFO: GAMELIB: shutdown\n");
    UnloadTexture(state->assets.textures[0]);
    UnloadTexture(state->assets.textures[1]);
    *state = (GameState) { 0 };

    CloseWindow();
}

EXPORT void reload(GameState *state) {
    printf("INFO: GAMELIB: reload\n");
    state->running = true;
}

EXPORT void unload(GameState *state) {
    printf("INFO: GAMELIB: unload\n");
    state->running = false;
}
