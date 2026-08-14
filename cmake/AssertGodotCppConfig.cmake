# AssertGodotCppConfig.cmake
#
# Standalone script (cmake -P) that fails the build when the configuration being
# built disagrees with the godot-cpp target the binary directory was configured
# for.
#
# godot-cpp's GODOTCPP_TARGET is a configure-time cache string; it is not tied to
# $<CONFIG>. It drives DEBUG_FEATURES (godot-cpp/cmake/godotcpp.cmake), which in
# turn drives the PUBLIC DEBUG_ENABLED / HOT_RELOAD_ENABLED definitions in
# godot-cpp/cmake/common_compiler_flags.cmake. Those propagate to every target
# that links godot-cpp. With a multi-config generator a single configure serves
# both configurations, so `--config Release` against a template_debug configure
# silently produces a release-named DLL carrying debug-only ABI. That mismatch
# is not diagnosable from the binary at load time: it loads fine and corrupts
# the heap later (STATUS_HEAP_CORRUPTION, 0xC0000374) on shutdown.
#
# Required -D arguments:
#   GODOTCPP_TARGET  the configured godot-cpp target
#   BUILD_CONFIG     the configuration currently being built ($<CONFIG>)
#   TARGET_NAME      the target being built, for the error message

if(NOT DEFINED GODOTCPP_TARGET OR GODOTCPP_TARGET STREQUAL "")
    message(FATAL_ERROR "AssertGodotCppConfig.cmake requires -DGODOTCPP_TARGET=<target>.")
endif()

if(NOT DEFINED BUILD_CONFIG OR BUILD_CONFIG STREQUAL "")
    # Single-config generators can legitimately build with no configuration
    # name. The configure-time check in GodotCppConfigGuard.cmake covers those.
    return()
endif()

if(NOT DEFINED TARGET_NAME)
    set(TARGET_NAME "<unknown>")
endif()

if(GODOTCPP_TARGET STREQUAL "template_release")
    set(_allowed_configs Release RelWithDebInfo MinSizeRel)
    set(_fix_hint "cmake --preset default\n      cmake --build --preset debug")
    set(_consequence "link godot-cpp's template_release library and omit the debug-only DEBUG_ENABLED / HOT_RELOAD_ENABLED definitions that a debug build expects")
else()
    set(_allowed_configs Debug)
    set(_fix_hint "cmake --preset default-release\n      cmake --build --preset release")
    set(_consequence "link godot-cpp's template_debug library and compile the debug-only DEBUG_ENABLED / HOT_RELOAD_ENABLED definitions into the output")
endif()

# list(FIND) rather than if(... IN_LIST ...): IN_LIST needs policy CMP0057 set
# to NEW, which is not guaranteed in `cmake -P` script mode on CMake 3.x.
list(FIND _allowed_configs "${BUILD_CONFIG}" _config_index)
if(NOT _config_index EQUAL -1)
    return()
endif()

string(REPLACE ";" ", " _allowed_display "${_allowed_configs}")
message(FATAL_ERROR
    "godot-cpp configuration mismatch while building '${TARGET_NAME}'.\n"
    "This binary directory was configured with GODOTCPP_TARGET=${GODOTCPP_TARGET}, "
    "which only supports building: ${_allowed_display}. You asked to build: ${BUILD_CONFIG}.\n"
    "Building '${BUILD_CONFIG}' here would ${_consequence}. The resulting DLL loads "
    "successfully and then corrupts the heap (STATUS_HEAP_CORRUPTION, 0xC0000374) inside a "
    "Godot build of the other configuration, so this is failing now rather than shipping a "
    "broken binary.\n"
    "GODOTCPP_TARGET is a configure-time cache variable, so one binary directory can only "
    "ever serve one configuration correctly. Configure the matching preset into its own "
    "binary directory instead:\n"
    "      ${_fix_hint}\n"
    "See docs/troubleshooting.md ('Release build linked the debug godot-cpp').")
