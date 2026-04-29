cmake_minimum_required(VERSION 3.20)
project(godot_public_gdk_ext LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(NOT EXISTS "${CMAKE_SOURCE_DIR}/godot-cpp/CMakeLists.txt")
    message(FATAL_ERROR
        "godot-cpp submodule not found. Run:\n"
        "  git submodule update --init --recursive")
endif()

list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")

set(GODOT_ADDONS_ROOT "${CMAKE_SOURCE_DIR}/addons")
set(GODOT_SAMPLE_PROJECT_ROOTS
    "${CMAKE_SOURCE_DIR}/sample"
    "${CMAKE_SOURCE_DIR}/sample_shamwow"
)
set(GODOT_SAMPLE_ADDONS_ROOT "${CMAKE_SOURCE_DIR}/sample/addons")

option(BUILD_GODOT_GDK "Build the godot_gdk addon" ON)
option(BUILD_GODOT_GAMEINPUT "Build the godot_gameinput addon" ON)
option(BUILD_GODOT_PLAYFAB "Build the godot_playfab addon" ON)

add_subdirectory(godot-cpp)

if(BUILD_GODOT_GDK)
    add_subdirectory(addons/godot_gdk)
endif()

if(BUILD_GODOT_GAMEINPUT)
    add_subdirectory(addons/godot_gameinput)
endif()

if(BUILD_GODOT_PLAYFAB)
    add_subdirectory(addons/godot_playfab)
endif()
