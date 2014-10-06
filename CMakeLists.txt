cmake_minimum_required (VERSION 2.6)
project (toy-engine)

set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-unused-value -std=c++11 -fno-exceptions")

set (CMAKE_MODULE_PATH
    ${CMAKE_MODULE_PATH}
    "${CMAKE_CURRENT_SOURCE_DIR}/cmake"
    )

find_package (SDL2 REQUIRED)

include_directories (
    "${PROJECT_SOURCE_DIR}/Engine"
    "${PROJECT_SOURCE_DIR}"
    ${SDL2_INCLUDE_DIR}
    )
add_subdirectory (Engine)

add_executable (toy-engine main.cpp)

target_link_libraries (toy-engine
    Engine
    ${SDL2_LIBRARY}
    /usr/local/lib/libEGL.dylib
    /usr/local/lib/libGLESv2.dylib
    )
