add_executable(render test/main.cpp)
target_include_directories(render PRIVATE lib ${STB_DIR})