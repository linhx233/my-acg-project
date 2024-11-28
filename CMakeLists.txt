cmake_minimum_required(VERSION 3.20)

project(renderer CXX)

set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -Wall -O3 -O2 -Ofast")

set(CMAKE_CXX_STANDARD 11)

# add_subdirectory(src)
add_executable(render src/main.cpp)