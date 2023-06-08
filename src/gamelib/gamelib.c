#include <stdio.h>
#include "gamelib.h"
#include "../../lib/stb_sprintf.h"

const char *PATH = "./build";
const char *NAME = "gamelib";

#ifdef _WIN32

const char *EXTENSION = "dll";

#include <windows.h>
#include <libloaderapi.h>

void load(GameLib *lib) {
    if (lib->loaded) {
        fprintf(stderr, "Game lib: can't load, already loaded\n");
        return;
    }

    char filename[512];
    stbsp_sprintf(filename, "%s/%s.%s", PATH, NAME, EXTENSION);

    lib->handle = (void *) LoadLibraryA(filename);
    if (!lib->handle) {
        fprintf(stderr, "ERROR: LIBRARY: failed to load '%s'\n", filename);
        return;
    }
    printf("INFO: LIBRARY: loaded '%s'\n", filename);

    lib->initialize = (void (*)()) GetProcAddress(lib->handle, "initialize");
    lib->shutdown   = (void (*)()) GetProcAddress(lib->handle, "shutdown");
    lib->input      = (void (*)()) GetProcAddress(lib->handle, "input");
    lib->update     = (void (*)()) GetProcAddress(lib->handle, "update");
    lib->draw       = (void (*)()) GetProcAddress(lib->handle, "draw");
    printf("INFO: LIBRARY: loaded functions\n");

    lib->loaded = true;
}

void unload(GameLib *lib) {
    if (!lib->loaded) {
        fprintf(stderr, "ERROR: LIBRARY: can't unload game library, not loaded\n");
        return;
    }

    FreeLibrary(lib->handle);
    printf("INFO: LIBRARY: unloaded game library\n");

    *lib = (GameLib) { 0 };
}

//#elif __APPLE__
//
//#include <dlfcn.h>
//#define LOAD_LIBRARY dlopen(filename, RTLD_LAZY)
//const char *EXTENSION = "dylib";
//
//#elif linux
//
//    #include <dlfcn.h>
//    #define LOAD_LIBRARY dlopen(filename, RTLD_LAZY)
//    const char *EXTENSION = "so";

#else

    #error "Platform not supported"

#endif

