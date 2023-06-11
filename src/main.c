#include "state/state.h"

#include "utils/hot_reload.c" // :side-eye:

#define STB_SPRINTF_IMPLEMENTATION
#include "../lib/stb_sprintf.h"

#define PATH_SIZE 2048
const char *PATH = "./build";
const char *NAME = "gamelib";
#ifdef _WIN32
const char *EXT = "dll";
#else
const char *EXT = "";
#endif

int main()
{
    char mainPath[PATH_SIZE];
    char tempPath[PATH_SIZE];
    char lockFilePath[PATH_SIZE];
    stbsp_sprintf(mainPath, "%s/%s.%s", PATH, NAME, EXT);
    stbsp_sprintf(tempPath, "%s/%s_temp.%s", PATH, NAME, EXT);
    stbsp_sprintf(lockFilePath, "%s/lock.file", PATH);

    GameLib gamelib = load_library(mainPath, tempPath, lockFilePath);

    GameState state = {0};
    gamelib.init(&state);

    while (state.running) {
        // hot reload the game library if it's been modified
        if (is_library_dirty(mainPath, &gamelib)) {
            gamelib.unload(&state);
            unload_library(&gamelib);

            gamelib = load_library(mainPath, tempPath, lockFilePath);
            gamelib.reload(&state);
        }

        gamelib.update(&state);
    }
    gamelib.shutdown(&state);

    return 0;
}
