#ifndef RAYLIB_SANDBOX_STATE_H
#define RAYLIB_SANDBOX_STATE_H

#include "raylib.h"

#define NUM_TEXTURES 2

typedef struct GameState {
    bool running;
    struct Assets {
        Texture textures[NUM_TEXTURES];
        int textureIndex;
    } assets;
} GameState;

// function pointer prototypes
typedef void (*FuncInit)(GameState *state);
typedef void (*FuncUpdate)(GameState *state);
typedef void (*FuncShutdown)(GameState *state);
typedef void (*FuncReload)(GameState *state);
typedef void (*FuncUnload)(GameState *state);

typedef struct GameLib {
    void *handle;
    bool valid;

    long lastWriteTime;

    FuncInit init;
    FuncUpdate update;
    FuncShutdown shutdown;
    FuncReload reload;
    FuncUnload unload;
} GameLib;

#endif //RAYLIB_SANDBOX_STATE_H
