cmake_minimum_required(VERSION 3.0)
project(DanmakuParanoia)


# build raylib
set(BUILD_EXAMPLES OFF CACHE BOOL "" FORCE) # don't build the supplied examples
set(BUILD_GAMES    OFF CACHE BOOL "" FORCE) # or games

add_compile_options (-O3 -march=native -ffast-math)

add_subdirectory(external/raylib)
include_directories(external/raylib/src)

include_directories(src/.)
include_directories(DanPaVM/src/.)

file(GLOB_RECURSE SOURCES ${PROJECT_SOURCE_DIR} src/*.c DanPaVM/src/*.c)
file(GLOB_RECURSE HEADERS ${PROJECT_SOURCE_DIR} src/*.h DanPaVM/src/*.h external/raylib/*.h)

add_executable(${PROJECT_NAME} ${SOURCES} ${HEADERS})
target_link_libraries(${PROJECT_NAME} raylib m)
