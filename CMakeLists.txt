cmake_minimum_required(VERSION 3.0)
project(DanmakuParanoia)


# build raylib
set(BUILD_EXAMPLES OFF CACHE BOOL "" FORCE) # don't build the supplied examples
set(BUILD_GAMES    OFF CACHE BOOL "" FORCE) # or games

add_compile_options (-O3 -std=gnu11 -march=native -ffast-math -fopenmp)

add_subdirectory(external/raylib-3.7.0)
include_directories(external/raylib-3.7.0/src)

add_subdirectory(thread_support)
include_directories(thread_support)

include_directories(src/.)
include_directories(DanPaVM/src/.)

file(GLOB_RECURSE SOURCES ${PROJECT_SOURCE_DIR} src/*.c DanPaVM/src/*.c)
file(GLOB_RECURSE HEADERS ${PROJECT_SOURCE_DIR} src/*.h DanPaVM/src/*.h external/raylib/*.h)

add_executable(${PROJECT_NAME} ${SOURCES} ${HEADERS})
target_link_libraries(${PROJECT_NAME} thread_support raylib m)

# Checks if OSX and links appropriate frameworks (Only required on MacOS)
if (APPLE)
    target_link_libraries(${PROJECT_NAME} "-framework IOKit")
    target_link_libraries(${PROJECT_NAME} "-framework Cocoa")
    target_link_libraries(${PROJECT_NAME} "-framework OpenGL")
endif()

message(STATUS "Checking OpenMP")
find_package(OpenMP)
IF(OPENMP_FOUND)
    message("Found OpenMP! ^_^")
    # add flags for OpenMP
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS}")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
    set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} ${OpenMP_SHARED_LINKER_FLAGS}")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${OpenMP_EXE_LINKER_FLAGS}")
ELSE()
    message("Missed OpenMP! x_x")
ENDIF()
