cmake_minimum_required(VERSION 3.25)

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

## CMake configuration
## --------------------------
cmake_policy(SET CMP0054 NEW)               # suppress warnings for 'if()' interpolating unquoted args

set(CMAKE_C_STANDARD 17)                    # have some standards
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)       # generate compile_commands.json
set(CMAKE_SHARED_LIBRARY_PREFIX "")         # disable automatic library prefix 'lib' that occurs on some platforms
set(CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake" ${CMAKE_MODULE_PATH})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${BUILD}")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG "${BUILD}/debug")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE "${BUILD}/release")

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
        ${SRC}/gamelib/gamelib_impl.c
        )
target_link_libraries(${GAMELIB} PUBLIC raylib)

# move the shared lib into a well known directory after building
add_custom_command(TARGET ${GAMELIB} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        $<TARGET_FILE:${GAMELIB}> "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/gamelib${CMAKE_SHARED_LIBRARY_SUFFIX}"
        )

## Project executable
## --------------------------
add_executable(${GAME}
        ${SRC}/gamelib/gamelib.c
        ${SRC}/game/game.c
        ${SRC}/main.c
)
target_link_libraries(${GAME} PUBLIC raylib)

## Platform customizations
## --------------------------

# web configuration
if (${PLATFORM} STREQUAL "Web")
    # tell emscripten to build an example.html file
    set_target_properties(${PROJECT_NAME} PROPERTIES SUFFIX ".html")
endif()

# link appropriate frameworks if running on macos
if (APPLE)
    target_link_libraries(${PROJECT_NAME} "-framework IOKit")
    target_link_libraries(${PROJECT_NAME} "-framework Cocoa")
    target_link_libraries(${PROJECT_NAME} "-framework OpenGL")
endif()
