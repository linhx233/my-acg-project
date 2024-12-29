add_executable(render test/main.cpp)
target_include_directories(render PRIVATE lib ${STB_DIR} ${ASSIMP_DIR})
target_link_libraries(render PRIVATE ${ASSIMP_BUILD_DIR}/lib/Release/assimp-vc143-mt.lib)
add_custom_command(TARGET render POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
    ${ASSIMP_BUILD_DIR}/bin/Release/assimp-vc143-mt.dll
    ${CMAKE_CURRENT_BINARY_DIR}/Release)