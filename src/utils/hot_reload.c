#include <stdio.h>

#include "../state/state.h"

// if functions from the shared lib fail to load
// these empty stub functions will be used instead.
void stub_init(GameState *state) {}
void stub_shutdown(GameState *state) {}
void stub_reload(GameState *state) {}
void stub_unload(GameState *state) {}
void stub_update(GameState *state) {}

#ifdef _WIN32

#include "windows_raylib_safe.h"
#include <libloaderapi.h>

static bool is_library_dirty(char *mainPath, GameLib *lib) {
    long libCurrentWriteTime = GetFileModTime(mainPath);
    return (libCurrentWriteTime != lib->lastWriteTime);
}

static GameLib load_library(char *mainPath, char *tempPath, char *lockFilePath) {
    // spin wait while the lib is actively compiling
    while (FileExists(lockFilePath)) { Sleep(50); }

    GameLib lib;
    lib.lastWriteTime = GetFileModTime(mainPath);

    // prevent locking the main library by making and loading a copy
    CopyFileA((LPCSTR) mainPath, (LPCSTR) tempPath, FALSE);

    lib.handle = LoadLibraryA(tempPath);
    lib.valid = (lib.handle != NULL);
    if (!lib.valid) {
        lib.init   = stub_init;
        lib.update = stub_update;
        lib.shutdown = stub_shutdown;
        lib.reload = stub_reload;
        lib.unload = stub_unload;
        printf("ERROR: LIBRARY: failed to load shared library '%s'\n", tempPath);
    }

    if (lib.valid) {
        lib.init   = (FuncInit)   GetProcAddress(lib.handle, "init");
        lib.update = (FuncUpdate) GetProcAddress(lib.handle, "update");
        lib.shutdown = (FuncShutdown) GetProcAddress(lib.handle, "shutdown");
        lib.reload = (FuncReload) GetProcAddress(lib.handle, "reload");
        lib.unload = (FuncUnload) GetProcAddress(lib.handle, "unload");

        lib.valid = (lib.init != NULL)
                 && (lib.update != NULL)
                 && (lib.shutdown != NULL)
                 && (lib.reload != NULL)
                 && (lib.unload != NULL);

        printf("INFO: LIBRARY: loaded shared library '%s'\n", tempPath);
    }

    return lib;
}

static void unload_library(GameLib *lib) {
    // msvc locks the pdb even when a dll is unloaded for some stupid reason
    // https://guide.handmadehero.org/code/day022/#358
    if (lib) {
        FreeLibrary(lib->handle);
        lib->handle = NULL;
        lib->valid  = false;
        lib->init   = stub_init;
        lib->shutdown = stub_shutdown;
        lib->update = stub_update;
        lib->reload = stub_reload;
        lib->unload = stub_unload;
        printf("INFO: LIBRARY: unloaded shared library\n");
    }
}

#else

const char *EXT = "";

#endif //_WIN32
