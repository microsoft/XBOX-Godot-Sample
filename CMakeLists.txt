cmake_minimum_required(VERSION 3.20)
project(godot_public_gdk_ext LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(NOT EXISTS "${CMAKE_SOURCE_DIR}/godot-cpp/CMakeLists.txt")
    message(FATAL_ERROR
        "godot-cpp submodule not found. Run:\n"
        "  git submodule update --init --recursive")
endif()

if(NOT EXISTS "${CMAKE_SOURCE_DIR}/third_party/Gut/addons/gut/plugin.cfg")
    message(FATAL_ERROR
        "GUT submodule not found at third_party/Gut/. Run:\n"
        "  git submodule update --init --recursive")
endif()

list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")

set(GODOT_ADDONS_ROOT "${CMAKE_SOURCE_DIR}/addons")

# Per-sample directories. The two tutorial samples have different
# addon ownership so they are tracked individually rather than as a
# single undifferentiated GODOT_SAMPLE_DIRS list:
#   - tutorial_app:        gdk, playfab, gdk_packaging
#   - tutorial_gameinput:  gameinput only
# `GODOT_SAMPLE_DIRS` aggregates both for tooling that walks every
# sample (e.g. the GodotExtensionCommon fallback path).
set(GODOT_SAMPLE_TUTORIAL_APP_DIR        "${CMAKE_SOURCE_DIR}/sample/tutorial_app")
set(GODOT_SAMPLE_TUTORIAL_GAMEINPUT_DIR  "${CMAKE_SOURCE_DIR}/sample/tutorial_gameinput")
set(GODOT_SAMPLE_DIRS
    ${GODOT_SAMPLE_TUTORIAL_APP_DIR}
    ${GODOT_SAMPLE_TUTORIAL_GAMEINPUT_DIR}
)

set(GODOT_TEST_HOST_DIRS
    "${CMAKE_SOURCE_DIR}/tests/godot/gdk"
    "${CMAKE_SOURCE_DIR}/tests/godot/playfab"
    "${CMAKE_SOURCE_DIR}/tests/godot/gameinput"
)

set(GODOT_PROJECT_DIRS
    ${GODOT_SAMPLE_DIRS}
    ${GODOT_TEST_HOST_DIRS}
)

option(GODOT_PLAYFAB_TEST_HOST_WITH_GDK
    "Mirror godot_gdk into the PlayFab test host for optional Xbox-backed compatibility tests"
    ON)

set(GODOT_GDK_PROJECT_DIRS
    ${GODOT_SAMPLE_TUTORIAL_APP_DIR}
    "${CMAKE_SOURCE_DIR}/tests/godot/gdk"
)

if(GODOT_PLAYFAB_TEST_HOST_WITH_GDK)
    list(APPEND GODOT_GDK_PROJECT_DIRS
        "${CMAKE_SOURCE_DIR}/tests/godot/playfab"
    )
endif()

set(GODOT_PLAYFAB_PROJECT_DIRS
    ${GODOT_SAMPLE_TUTORIAL_APP_DIR}
    "${CMAKE_SOURCE_DIR}/tests/godot/playfab"
    "${CMAKE_SOURCE_DIR}/tests/godot/mp_test_client"
)

set(GODOT_GAMEINPUT_PROJECT_DIRS
    ${GODOT_SAMPLE_TUTORIAL_GAMEINPUT_DIR}
    "${CMAKE_SOURCE_DIR}/tests/godot/gameinput"
)

set(GODOT_SAMPLE_GUT_HOST_DIRS)

set(GODOT_GUT_HOST_DIRS
    ${GODOT_SAMPLE_GUT_HOST_DIRS}
    ${GODOT_TEST_HOST_DIRS}
)

include(GodotExtensionCommon)

option(BUILD_GODOT_GDK "Build the godot_gdk addon" ON)
option(BUILD_GODOT_GAMEINPUT "Build the godot_gameinput addon" ON)
option(BUILD_GODOT_PLAYFAB "Build the godot_playfab addon" ON)
option(BUILD_GODOT_GDK_PACKAGING "Sync the godot_gdk_packaging editor plugin into samples" ON)

set(GODOT_GDK_PACKAGING_MIRROR_DIRS)
set(GODOT_GDK_PACKAGING_PROJECT_DIRS
    ${GODOT_SAMPLE_TUTORIAL_APP_DIR}
    "${CMAKE_SOURCE_DIR}/tests/godot/gdk"
)

foreach(sample_dir IN LISTS GODOT_GDK_PACKAGING_PROJECT_DIRS)
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

# ── GUT mirror ───────────────────────────────────────────────────
# GUT lives as a git submodule at `third_party/Gut/` (upstream
# bitwes/Gut, pinned to v9.6.0). The GUT plugin source is at
# `third_party/Gut/addons/gut/` (the upstream repo's own addon layout).
# Mirror that single source into the coverage hosts that run headless
# GUT suites. Demo/sample projects are intentionally not coverage hosts.
godot_addon_mirror_test_support(
    SOURCE_DIR  "${CMAKE_SOURCE_DIR}/third_party/Gut/addons/gut"
    DEST_SUBDIR "gut"
    HOST_DIRS
        ${GODOT_GUT_HOST_DIRS}
)

# ── Shared GUT base classes mirror (Wave 2) ──────────────────────
# `addons/godot_gdk/tests_support/bases/` is the single source of truth for
# the shared test_env + GUT base classes (gdk / playfab / gameinput). Mirror
# into the same coverage hosts as GUT itself so suites can do
# `extends "res://addons/godot_gdk_tests/<base>.gd"`. Pong is intentionally
# absent — it is not a coverage host.
godot_addon_mirror_test_support(
    SOURCE_DIR  "${CMAKE_SOURCE_DIR}/addons/godot_gdk/tests_support/bases"
    DEST_SUBDIR "godot_gdk_tests"
    HOST_DIRS
        ${GODOT_GUT_HOST_DIRS}
)

# ── C++ unit tests (doctest) ─────────────────────────────────────
# Wired up by the parallel `infra-doctest-target` work; this option default
# stays ON so once that branch lands, builds pick up the production exe.
option(GDK_BUILD_TESTS "Build C++ unit tests (doctest)" ON)
if(GDK_BUILD_TESTS)
    add_subdirectory(tests/cpp)
endif()
