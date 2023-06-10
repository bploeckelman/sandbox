#ifndef RAYLIB_SANDBOX_GAMELIB_H
#define RAYLIB_SANDBOX_GAMELIB_H

#include <stdbool.h>
#include "raylib.h"

#ifdef _MSC_VER
#define DLL_EXPORT __declspec(dllexport)
#elif __GNUC__ >= 4
#define DLL_EXPORT __attribute__((visibility("default")))
#else
#define DLL_EXPORT
#endif

typedef struct GameLib {
    bool loaded;
    void *handle;
    void (*initialize)();
    void (*shutdown)();
    void (*input)();
    void (*update)();
    void (*draw)();
} GameLib;

void load(GameLib *lib);
void unload(GameLib *lib);

#endif //RAYLIB_SANDBOX_GAMELIB_H
