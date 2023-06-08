cmake_minimum_required(VERSION 3.25)
project(raylib_sandbox C)

set(CMAKE_C_STANDARD 17)

# generate compile_commands.json
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

### Raylib ----------------------------------------------------------

set(RAYLIB_VERSION 4.5.0)
set(GRAPHICS "GRAPHICS_API_OPENGL_43") # use OpenGL 4 instead of 3 (default)

# suppress warnings 'if()' interpolating unquoted args
cmake_policy(SET CMP0054 NEW)

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

### Project ---------------------------------------------------------

add_executable(${PROJECT_NAME}
        src/main.c
        )

#set(raylib_VERBOSE 1)

target_link_libraries(${PROJECT_NAME} raylib)

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
