#include "state/state.h"

#define STB_SPRINTF_IMPLEMENTATION
#include "../lib/stb_sprintf.h"

#include "utils/hot_reload.c" // :side-eye:

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

    GameLib gamelib = load_library(PATH, NAME, lockFilePath);

    GameState state = {0};
    gamelib.init(&state);

    while (state.running) {
        // hot reload the game library if it's been modified
        // NOTE - this is kind of a mess,
        //   can't build the lib while it's loaded since the pdb file is locked by msvc
        //   but can't automatically trigger the unload/reload by dll timestamp,
        //   since it can't be rebuilt while loaded....
        //   need to wangle something where
        //     a temp pdb and temp dll are loaded
        //   and a rebuild can create the non-temp pdb/dll,
        //     which are then copied to overwrite the temp version
        //     once the temp is unloaded, which unlocks the temp versions for copying
        // https://hero.handmade.network/forums/code-discussion/t/177-dll_pdb_doesn%2527t_autoload_when_hotloading_the_dll#1702
        //   can potentially use a bunch of cmake custom commands at the pre_link and post_link stages?
        //   not sure how to get the temp version of the pdb loaded though
        if (is_library_dirty(mainPath, &gamelib)) {
            gamelib.unload(&state);
            unload_library(&gamelib);

            gamelib = load_library(PATH, NAME, lockFilePath);
            gamelib.reload(&state);
        }

        gamelib.update(&state);
    }
    gamelib.shutdown(&state);

    return 0;
}
