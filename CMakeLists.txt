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

set(GODOT_SAMPLE_DIRS
    "${CMAKE_SOURCE_DIR}/sample/gdk_demo"
    "${CMAKE_SOURCE_DIR}/sample/multiplayer_pong"
    "${CMAKE_SOURCE_DIR}/sample/gdk_launch_point"
    "${CMAKE_SOURCE_DIR}/sample/playfab_demo"
)

include(GodotExtensionCommon)

option(BUILD_GODOT_GDK "Build the godot_gdk addon" ON)
option(BUILD_GODOT_GAMEINPUT "Build the godot_gameinput addon" ON)
option(BUILD_GODOT_PLAYFAB "Build the godot_playfab addon" ON)
option(BUILD_GODOT_GDK_PACKAGING "Sync the godot_gdk_packaging editor plugin into samples" ON)

set(GODOT_GDK_PACKAGING_MIRROR_DIRS)

foreach(sample_dir IN LISTS GODOT_SAMPLE_DIRS)
    list(APPEND GODOT_GDK_PACKAGING_MIRROR_DIRS
        "${sample_dir}/addons/godot_gdk_packaging"
    )
endforeach()

godot_addon_sync_directory(
    TARGET_NAME sync_godot_gdk_packaging
    SOURCE_DIR "${CMAKE_SOURCE_DIR}/addons/godot_gdk_packaging"
    MIRROR_DIRS ${GODOT_GDK_PACKAGING_MIRROR_DIRS}
)

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
