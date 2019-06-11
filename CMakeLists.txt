cmake_minimum_required(VERSION 3.9)
project(2phaselocking)

set(CMAKE_CXX_STANDARD 11)

include_directories(include)
add_executable(2phaselocking src/main.cpp)