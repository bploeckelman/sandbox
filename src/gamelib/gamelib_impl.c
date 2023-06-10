#include <stdio.h>
#include "gamelib.h"
#include "raylib.h"

#define STB_SPRINTF_IMPLEMENTATION
#include "../../lib/stb_sprintf.h"

// TODO(brian) - split out loader from lib
void load(GameLib *lib) {}
void unload(GameLib *lib) {}

#define NUM_TEXTURES 2

struct State {
    Texture textures[NUM_TEXTURES];
    int textureIndex;
} state = { 0 };

DLL_EXPORT void initialize() {
    printf("INFO: GAMELIB: initialize\n");
    state.textures[0] = LoadTexture("assets/game-pads.png");
    state.textures[1] = LoadTexture("assets/game-space.png");
    state.textureIndex = 0;
}

DLL_EXPORT void shutdown() {
    printf("INFO: GAMELIB: shutdown\n");
    UnloadTexture(state.textures[0]);
    UnloadTexture(state.textures[1]);
    state = (struct State) { 0 };
}

DLL_EXPORT void input() {
    if (IsKeyPressed(KEY_SPACE)) {
        state.textureIndex = (state.textureIndex + 1) % NUM_TEXTURES;
    }
}

DLL_EXPORT void update() {
}

DLL_EXPORT void draw() {
    Texture texture = state.textures[state.textureIndex];
    int window_width = GetScreenWidth();
    int window_height = GetScreenHeight();

    BeginDrawing();
    {
        ClearBackground(SKYBLUE);
        DrawTexture(texture, (window_width - texture.width) / 2, (window_height- texture.height) / 2, WHITE);
        DrawText("Raylib", 150, 280, 80, LIGHTGRAY);
    }
    EndDrawing();
}
