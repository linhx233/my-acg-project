cmake_minimum_required(VERSION 3.20)

project(renderer C CXX)

set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -Wall -O3 -O2 -Ofast")
set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)

set(CMAKE_CXX_STANDARD 17)

set(STB_DIR ${CMAKE_SOURCE_DIR}/external/stb)
set(ASSIMP_DIR ${CMAKE_SOURCE_DIR}/external/assimp/include)
set(ASSIMP_BUILD_DIR ${CMAKE_SOURCE_DIR}/build/external/assimp)

add_subdirectory(external/assimp)

add_subdirectory(src)
