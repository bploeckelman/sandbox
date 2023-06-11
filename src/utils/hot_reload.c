#include <stdio.h>
#include <time.h>

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

#define PATH_SIZE 2048

static bool is_library_dirty(char *mainPath, GameLib *lib) {
    long libCurrentWriteTime = GetFileModTime(mainPath);
    return (libCurrentWriteTime != lib->lastWriteTime);
}

static GameLib load_library(const char *path, const char *libName, const char *lockFilePath) {
//static GameLib load_library(char *mainPath, char *tempPath, char *lockFilePath) {
    // spin wait while the lib is actively compiling
    while (FileExists(lockFilePath)) { Sleep(50); }

    time_t t = time(0);
    struct tm *now = localtime(&t);
    char timestamp[PATH_SIZE];
    strftime(timestamp, sizeof(timestamp), "%H%M%S", now);

    char mainLibPath[PATH_SIZE];
    char tempLibPath[PATH_SIZE];
    char mainPdbPath[PATH_SIZE];
    char tempPdbPath[PATH_SIZE];
    stbsp_sprintf(mainLibPath, "%s/%s.dll", path, libName);
    stbsp_sprintf(tempLibPath, "%s/%s_temp.dll", path, libName);
    stbsp_sprintf(mainPdbPath, "%s/debug/%s_build.pdb", path, libName);
    stbsp_sprintf(tempPdbPath, "%s/debug/%s_build_%s.pdb", path, libName, timestamp);

    GameLib lib;
    lib.lastWriteTime = GetFileModTime(mainLibPath);

    // prevent locking the library by making and loading a copy
    CopyFileA((LPCSTR) mainLibPath, (LPCSTR) tempLibPath, FALSE);

    lib.handle = LoadLibraryA(tempLibPath);
    lib.valid = (lib.handle != NULL);
    if (!lib.valid) {
        lib.init   = stub_init;
        lib.update = stub_update;
        lib.shutdown = stub_shutdown;
        lib.reload = stub_reload;
        lib.unload = stub_unload;
        printf("ERROR: LIBRARY: failed to load shared library '%s'\n", tempLibPath);
    }

    // prevent locking the library pdb by renaming it after the library is loaded
    MoveFileA((LPCSTR) mainPdbPath, (LPCSTR) tempPdbPath);

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

        printf("INFO: LIBRARY: loaded shared library '%s'\n", tempLibPath);
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
