project(thread_support CXX)

cmake_minimum_required(VERSION 3.2)
set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O2 -march=native -ffast-math")

find_package (Threads)

file(GLOB sources *.cpp)
add_library(thread_support STATIC ${sources})
target_link_libraries(thread_support Threads::Threads)
