#include <stdio.h>
#include "gamelib.h"

#define STB_SPRINTF_IMPLEMENTATION
#include "../../lib/stb_sprintf.h"

void load(GameLib *lib) {}
void unload(GameLib *lib) {}

struct State {
    int value;
    char ch;
} state = { 0 };

DLL_EXPORT void initialize() {
    state.value = 1;
    printf("\tstate: %d\n", state.value);
}

DLL_EXPORT void shutdown() {
    state.value = 0;
    printf("\tstate: %d\n", state.value);
}

DLL_EXPORT void input() {
    state.ch = 'i';
}

DLL_EXPORT void update() {
    state.value++;
    state.ch = 'u';
}

DLL_EXPORT void draw() {
    state.value--;
    state.ch = 'd';
}
