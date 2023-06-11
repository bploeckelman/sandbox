# Hot Reloading Shared Libraries

It's very convenient to be able to be able to speed up the development flow by
building an application as a shared library (`.dll` on windows, `.so` on linux, `.dylib` on mac) 
and reload the library on demand after making changes while keeping the application itself running.

The downside to this approach are the headaches involved in getting it setup and working correctly.

This document details a bunch of resources and approaches I've tried to get this process working.
Spoiler alert... haven't quite gotten there yet, but each time I've run into one problem there's 
been a way to knock it down and move on to a different problem.

## Resources

https://guide.handmadehero.org/code/day022/#358

https://hero.handmade.network/forums/code-discussion/t/177-dll_pdb_doesn%2527t_autoload_when_hotloading_the_dll#1702


https://github.com/krzosa/raylib_hot_reload_template

https://medium.com/@TheElkantor/how-to-add-hot-reload-to-your-raylib-proj-in-c-698caa33eb74


https://ruby0x1.github.io/machinery_blog_archive/post/dll-hot-reloading-in-theory-and-practice/index.html

https://ruby0x1.github.io/machinery_blog_archive/post/little-machines-working-together-part-1/

https://ruby0x1.github.io/machinery_blog_archive/post/little-machines-working-together-part-2/


https://fungos.github.io/cr-simple-c-hot-reload/

https://github.com/fungos/cr/


## Attempts

Setting up the main source file to be essentially a dumb wrapper for library code is fairly straightforward.

```c
int main() {
    // setup paths to find shared lib...
    char *path = "...";
    char *libName = "...";
    
    // load the library from those paths
    GameLib gamelib = load_library(path, libName);
    
    // need to be able to share state between the library and the main executable
    // so instantiate and initialize that shared state
    GameState state = {0};
    gamelib.init(&state);
    
    // run the main program loop
    while (state.running) {
        // check whether the library has been rebuilt
        if (is_library_dirty(path, &gamelib)) {
            // unload the out of date library
            gamelib.unload(&state);
            unload_library(path, libName);
            
            // reload the updated library
            gamelib = load_library(path, libName);            
            gamelib.reload(&state);
        }
        
        gamelib.update(&state);
    }
    gamelib.shutdown(&state);
    
    return 0;
}
```

The functions `load_library(...)` and `unload_library(...)` here are intended to be platform independent 
wrappers for platform specific shared library functions.

In Windows, these wrap the following (from `libloaderapi.h`):
- `HMODULE LoadLibraryA(LPCSTR lpLibFileName)` which loads the specified shared library file
- `FARPROC GetProcAddress(HMODULE hModule, LPCSTR lpProcName)` which loads function pointers or structure pointers from the shared library file
- `BOOL FreeLibrary(HMODULE hModule)` which unloads the shared library

In Linux (and maybe MacOS too?) these wrap the following (from `dlfcn.h`):
- `void *dlopen(const char *filename, int flag)` which loads the specified shared library file
- `void *dlsym(void *handle, const char *symbol)` which loads function pointeres or structure pointers from the shared library
- `int dlclose(void *handle)` which unloads the shared library

## Problems 

The actual program in this case is a game using the very nice [raylib](https://github.com/raysan5/raylib) library,
so some of these details are going to be Raylib specific and may not apply in other use cases.

### Including `windows.h` in a Raylib project

Raylib is intended to be a very simple cross-platform game development library and in support of this
it includes functions that have name conflicts with functions in `windows.h` (which must be included 
in order to include `libloaderapi.h` that provides the actual Win32 API calls we need to work with 
the shared library code).

One option here is to change the raylib source to rename the functions and structs that conflict.
However, changing the raylib source seemed like a heavy-handed and potentially brittle
way to approach solving this problem, so I dug around for other options and found the following:

https://github.com/raysan5/raylib/issues/1217#issuecomment-618428626

Creating a header file based off the code in the 3rd option and including that resolved the symbol conflicts
and allowed the inclusion of `libloaderapi.h` and usage of `LoadLibraryA(...)`, etc...

### OpenGL Context

Raylib creates and manages an OpenGL context that stores all the state associated with 
a running OpenGL process. While my original expectation was that the shared library code 
could seamlessly use raylib functions that relied on a valid GL context that had been 
created in the main executable since my understanding is that a shared library 
gets loaded into the host process address space, it turns out that this wasn't accurate.

My initial attempt had the main executable handle things like 
`InitiWindow(...)`, `Begin|EndDrawing()`, `CloseWindow()`, etc... 
while the shared library code tried to handle things like loading textures
and making `DrawTexture(...)` calls. This didn't work.

When the library got reloaded, the library code didn't have 
a valid OpenGL context and so raylib functions that required 
a valid GL context (like `LoadTexture(...)` and `DrawTexture(...)`)
understandably crashed the program.

According to the OpenGL wiki, a context and the objects in it should be shareable:
> "Each context has its own set of OpenGL Objects, which are independent of those from other contexts.
> A context's objects can be shared with other contexts. Most OpenGL objects are sharable..."

So I investigated a couple options to try to figure out how to share the OpenGL context 
with the hot-reloaded library code, which seems to be technically possible but would
likely require making changes to the raylib code since it doesn't expose the underlying 
GLFW calls it uses to manage the context. 

For those who are interested, here's some example steps describing how to change raylib code 
to support multiple windows, which isn't exactly the same thing, but it's in the ballpark:
https://github.com/raysan5/raylib/wiki/Use-multiple-windows

So I shifted gears to having the library manage the GL context 
by moving the raylib usage code into the shared library.

### Visual Studio and PDB Lock

If you've ever tried something like this using the msvc toolchain, you've almost certainly 
run into this problem. It's mentioned in a bunch of the resource links listed above, and 
unfortunately none of them really seem to have a consistent straightforward solution.

For reference, a [`.pdb`](https://learn.microsoft.com/en-us/visualstudio/debugger/specify-symbol-dot-pdb-and-source-files-in-the-visual-studio-debugger?view=vs-2022)
file stands for 'program database' (or debug symbol map), which is how visual studio handles 
mapping symbols (names of functions and structs and other such identifiers) 
to actual locations in compiled object code. This allows nice debugging where we can 
see the names of things instead of just memory addresses like one would see if they viewed 
disassembly of an object file directly.

Visual studio is particularly obnoxious when it comes to working with pdb files since it 
holds a lock on the pdb when running in debug mode, preventing the associated object files 
from being rebuilt, *even after the shared library has been unloaded!*  :facepalm:

Some of the resources linked above get pretty in the weeds on options to work around this.
One resource that I didn't link even went so far as to use undocumented Windows API calls 
to forcibly release a file lock. While technically interesting, this is well beyond what 
I'd consider a reasonable approach to dealing with this situation.

There's lots of options presented in the other resources:
- copy the lib with a different name and load that instead of the 'real' lib file so the lock is held on the 'temp' lib file
  - this fails when debugging because msvc holds a lock on the pdb as well, even after the library is unloaded
- copy the pdb with a different name as well as the lib so hopefully the lock is held on the renamed pdb instead of the 'real' one
  - this fails because the full path to the pdb gets built into the shared library on compilation so it locks the original rather than the renamed 'temp' pdb
- rename / move the pdb file after loading the shared library
  - this sort of works? windows tracks the rename and keeps the pdb attached to the library, but it still didn't quite work?
- ???

Other things I've tried are:

#### Embed debug symbols directly in object files

MSVC has a couple compiler and linker flags / properties that should, in theory, embed debug symbols directly in object files 
rather than creating a pdb file and storing debug symbols there. In practice this is very fiddly, especially with CMAKE.

I was able to get it to work (I think), with the following mishmash of CMAKE settings.

```cmake
# https://cmake.org/cmake/help/latest/policy/CMP0141.html
# Note that this must be set before any `project()` entry in CMakeLists.txt
cmake_policy(SET CMP0141 NEW)

# only try these dumb things if we're using MSVC
if (CMAKE_C_COMPILER_ID MATCHES "MSVC")
  # https://learn.microsoft.com/en-us/cpp/build/reference/z7-zi-zi-debug-information-format?view=msvc-170#z7
  # The '/Z7' compiler flag is supposed to allow embed debug symbols into object files without a pdb being required
  # however, according to the docs above "a PDB file can still be generated from these object files or libraries if the linker is passed the /DEBUG option"
  set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} /Z7")

  # since a pdb file still gets made even with the '/Z7' flag
  # (and in my case the compiler overrode '/Z7' with '/Zi' which doesn't have the same behavior)
  # I tried stripping out the '/debug' flag from the shared linker flags to see if that made a difference...
  # 
  string(REPLACE "/debug" "" MY_SHARED_LINKER_FLAGS_DEBUG ${CMAKE_SHARED_LINKER_FLAGS_DEBUG})
  set(CMAKE_SHARED_LINKER_FLAGS_DEBUG "${MY_SHARED_LINKER_FLAGS_DEBUG}")

  # CMAKE has it's own setting for changing how debugging info is handled, so I tried that too
  # https://cmake.org/cmake/help/latest/prop_tgt/MSVC_DEBUG_INFORMATION_FORMAT.html
  # Note that I'm not sure if a 'CMAKE_' prefix is required on this property or not...
  # CMake can be quite frustrating to work with.
  set_property(TARGET ${GAMELIB} PROPERTY
          MSVC_DEBUG_INFORMATION_FORMAT "$<$<CONFIG:Debug,RelWithDebInfo>:Embedded>")
endif()
```

After all the above nonsense, I was indeed able to build the shared library without a pdb file being generated. 
But of course this wasn't the last of the roadblocks.

### Breakpoints

After a shared library is reloaded while the main executable is still running in debug mode, 
the addresses for the library functions change, meaning any breakpoints set in the library code 
before the hot-reload are no longer valid.

One resource suggested that the debugger could be detached before rebuilding the lib and then 
re-attached after it's been reloaded. Clion doesn't seem to support this approach so far as I've been 
able to tell. There are options for 'attaching' a debugger to a process (though it's not particularly 
friendly since it lists all running processes rather than the relevant one for the project at hand),
but I haven't found a way to detach the debugger while keeping the executable running.

I don't have a solution for this at the moment.

### State Loss

When a shared library gets unloaded and reloaded, any state that the previous library held is not maintained 
in the new library. So managing this state and making sure it's available and up to date in the reloaded library 
can be tricky.

In the example code, there's a 'running' flag in the `GameState` struct that determines whether 
the game loop should continue. The shared library code checks with raylib via `WindowShouldClose()` 
to determine if this flag should be turned off.

After reloading the shared library, the `state.running` value gets set to false, likely in the `gamelib.update()` 
call (though this is hard to verify due to the breakpoint loss issue mentioned above).
I assume this has something to do with state loss in the sense that the only thing setting `state.running` to false 
is the raylib check for whether `WindowShouldClose()`. I haven't dug into this yet, but it seems likely that 
there's a mismatch with state here since raylib is linked with the gamelib which gets unloaded and reloaded.

One resource I read on this suggests that raylib would need to be linked to the main executable and then 
making a bridge between the main executable and the shared library to give raylib function pointers to the library.

I don't have a working solution for this yet either.
