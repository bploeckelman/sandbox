cmake_minimum_required(VERSION 3.25)

cmake_policy(SET CMP0054 NEW)    # suppress warnings for 'if()' interpolating unquoted args
cmake_policy(SET CMP0141 NEW)    # allow MSVC_DEBUG_INFORMATION_FORMAT property to embed debug info into object file for shared lib

## Project configuration
## --------------------------
project(raylib_sandbox
        VERSION 0.0.1
        DESCRIPTION "Game Sandbox using Raylib"
        LANGUAGES C)

set(GAME    "game")
set(GAMELIB "gamelib_build")

set(SRC   "${PROJECT_SOURCE_DIR}/src")
set(LIB   "${PROJECT_SOURCE_DIR}/lib")
set(BUILD "${PROJECT_SOURCE_DIR}/build")
set(ASSETS "${PROJECT_SOURCE_DIR}/assets")

## CMake configuration
## --------------------------

set(CMAKE_C_STANDARD 17)                    # have some standards
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)       # generate compile_commands.json
set(CMAKE_SHARED_LIBRARY_PREFIX "")         # disable automatic library prefix 'lib' that occurs on some platforms
set(CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake" ${CMAKE_MODULE_PATH})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${BUILD}")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG "${BUILD}/debug")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE "${BUILD}/release")

## Build configuration
## --------------------------

# TODO
# try to convince visual studio to not locking the pdb file so hot reloading works
# since the shared library won't link if the pdb file is locked
# this doesn't quite work though...
if (CMAKE_C_COMPILER_ID MATCHES "MSVC")
#    string(TIMESTAMP TIME "%H%M%S")
#    set(CMAKE_SHARED_LINKER_FLAGS_DEBUG "${CMAKE_SHARED_LINKER_FLAGS_DEBUG} /PDBALTPATH:${GAMELIB}_temp.pdb}") # /PDB:${GAMELIB}_${TIME}.pdb")
#    set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} /Z7")
    # should (maybe) force debugging info to be built into object files instead of in a separate pdb file
#    set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT "${CMAKE_MSVC_DEBUG_INFORMATION_FORMAT} Embedded")

    # NOTE - even with the MSVC DEBUG INFORMATION FORMAT property set to 'Embedded'
    # if the linker gets a /debug flag it'll generate the pdb file anyway (though unclear whether it holds a lock on it)
    # so the only way I can see to completely remove the pdb is to force embed the debug symbols
    # and also strip the /debug linker flag...
    # https://github.com/MicrosoftDocs/cpp-docs/blob/main/docs/build/reference/z7-zi-zi-debug-information-format.md#z7
    # still doesn't solve the problem exactly, as the state.running field gets reset to false after reloading
    # and it doesn't seem like we can reset it properly?
    string(REPLACE "/debug" "" MY_SHARED_LINKER_FLAGS_DEBUG ${CMAKE_SHARED_LINKER_FLAGS_DEBUG})
    set(CMAKE_SHARED_LINKER_FLAGS_DEBUG "${MY_SHARED_LINKER_FLAGS_DEBUG}")
#    message("SHARED_LINKER_FLAGS: " "${CMAKE_SHARED_LINKER_FLAGS_DEBUG}")
#    message("MY_SHARED_LINKER_FLAGS: " "${MY_SHARED_LINKER_FLAGS_DEBUG}")
endif()

## Raylib configuration
## --------------------------
set(RAYLIB_VERSION 4.5.0)
set(GRAPHICS "GRAPHICS_API_OPENGL_43")      # use OpenGL 4 instead of 3 (default)
#set(raylib_VERBOSE 1)

find_package(raylib ${RAYLIB_VERSION} QUIET) # QUIET or REQUIRED
if (NOT raylib_FOUND) # if it's not already available, fetch and build it
    include(FetchContent)
    FetchContent_Declare(
            raylib
            DOWNLOAD_EXTRACT_TIMESTAMP OFF
            URL https://github.com/raysan5/raylib/archive/refs/tags/${RAYLIB_VERSION}.tar.gz
    )
    FetchContent_GetProperties(raylib)
    if (NOT raylib_POPULATED) # has it been downloaded yet?
        set(FETCHCONTENT_QUIET NO)
        FetchContent_Populate(raylib)
        set(BUILD_EXAMPLES OFF CACHE BOOL "" FORCE) # no need to build supplied examples
        add_subdirectory(${raylib_SOURCE_DIR} ${raylib_BINARY_DIR})
    endif()
endif()

## Game code shared library
## --------------------------
add_library(${GAMELIB} SHARED
        ${SRC}/gamelib/gamelib.c
)
target_link_libraries(${GAMELIB} PUBLIC raylib)

# move the shared lib into a well known directory after building
add_custom_command(TARGET ${GAMELIB} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        $<TARGET_FILE:${GAMELIB}> "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/gamelib${CMAKE_SHARED_LIBRARY_SUFFIX}"
)
#add_custom_command(TARGET ${GAMELIB} PRE_LINK
#        COMMAND ${CMAKE_COMMAND} -E copy
#        $<TARGET_PDB_FILE:${GAMELIB}>
#)
#add_custom_command(TARGET ${GAMELIB} PRE_LINK
#        COMMAND ${CMAKE_COMMAND} -E rm
#        $<TARGET_PDB_FILE:${GAMELIB}>
#)

if (CMAKE_C_COMPILER_ID MATCHES "MSVC")
    # should (maybe) force debugging info to be built into object files instead of in a separate pdb file
    # https://cmake.org/cmake/help/latest/prop_tgt/MSVC_DEBUG_INFORMATION_FORMAT.html
    # not sure if the 'CMAKE_' prefix is required here or not
    set_property(TARGET ${GAMELIB} PROPERTY
            MSVC_DEBUG_INFORMATION_FORMAT "$<$<CONFIG:Debug,RelWithDebInfo>:Embedded>")
    set_property(TARGET ${GAMELIB} PROPERTY
            CMAKE_MSVC_DEBUG_INFORMATION_FORMAT "$<$<CONFIG:Debug,RelWithDebInfo>:Embedded>")
endif()

## Project executable
## --------------------------
add_executable(${GAME}
        ${SRC}/main.c
)
target_link_libraries(${GAME} PUBLIC raylib)

## Platform customizations
## --------------------------

# web configuration
if (${PLATFORM} STREQUAL "Web")
    # tell emscripten to build an example.html file
    set_target_properties(${PROJECT_NAME} PROPERTIES SUFFIX ".html")
    # TODO - statically link gamelib
endif()

# link appropriate frameworks if running on macos
if (APPLE)
    target_link_libraries(${PROJECT_NAME} "-framework IOKit")
    target_link_libraries(${PROJECT_NAME} "-framework Cocoa")
    target_link_libraries(${PROJECT_NAME} "-framework OpenGL")
endif()
