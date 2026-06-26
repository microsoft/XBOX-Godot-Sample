# libFuzzer fuzz targets. Built when GDK_BUILD_FUZZ_TARGETS=ON (the `fuzz`
# configure preset sets this). Requires clang-cl toolset.
#
# Phase 2 targets: pure C++ seams, no Godot types.
# Phase 3 targets: link against godot_stub_harness; full Godot-type coverage.
#
# Corpus convention: tests/cpp/fuzz/corpus/<target_name>/
# To run a target: .\build\fuzz\bin\Debug\<target>.exe -runs=100000 -max_len=256

set(_PF_SRC "${CMAKE_SOURCE_DIR}/addons/godot_playfab/src")

# ── Phase 2: pure seam fuzz targets ──────────────────────────────
godot_addon_fuzzer_target(
    TARGET_NAME playfab_fuzz_key_lookup
    SOURCES
        "${CMAKE_CURRENT_SOURCE_DIR}/fuzz_playfab_key_lookup.cpp"
        "${_PF_SRC}/playfab_request_key.cpp"
    INCLUDES
        "${_PF_SRC}"
)

# ── Phase 3: Godot-type fuzz targets (stub harness) ───────────────
# These targets link against godot_stub_harness (built in ../fuzz_harness/)
# and godot::cpp. They must call init_godot_stub_harness() in
# LLVMFuzzerInitialize before using any Godot type.

godot_addon_fuzzer_target(
    TARGET_NAME gdk_fuzz_result_formatting
    SOURCES
        "${CMAKE_CURRENT_SOURCE_DIR}/fuzz_gdk_result_formatting.cpp"
        "${CMAKE_SOURCE_DIR}/addons/godot_gdk/src/gdk_result_codes_internal.cpp"
        "${CMAKE_SOURCE_DIR}/addons/godot_playfab/src/playfab_result_codes_internal.cpp"
    LINK_LIBS
        godot_stub_harness
        godot::cpp
    INCLUDES
        "${CMAKE_SOURCE_DIR}/addons/godot_gdk/src"
        "${CMAKE_SOURCE_DIR}/addons/godot_playfab/src"
        "${CMAKE_SOURCE_DIR}/tests/cpp/fuzz_harness"
)

# playfab_fuzz_api_models requires the PlayFab SDK headers (PFTypes.h etc.).
# Enable by setting PLAYFAB_SDK_PATH and turning BUILD_GODOT_PLAYFAB=ON in the
# fuzz configure preset, or by passing -DPLAYFAB_SDK_PATH=<path> explicitly.
# The target is skipped automatically when the SDK is not found.
if(PLAYFAB_SDK_PATH AND EXISTS "${PLAYFAB_SDK_PATH}/include/playfab/core/PFTypes.h")
    godot_addon_fuzzer_target(
        TARGET_NAME playfab_fuzz_api_models
        SOURCES
            "${CMAKE_CURRENT_SOURCE_DIR}/fuzz_playfab_api_models.cpp"
            "${CMAKE_SOURCE_DIR}/addons/godot_playfab/src/api/playfab_api_models.cpp"
            "${CMAKE_SOURCE_DIR}/addons/godot_playfab/src/playfab_request_key.cpp"
        LINK_LIBS
            godot_stub_harness
            godot::cpp
        INCLUDES
            "${CMAKE_SOURCE_DIR}/addons/godot_playfab/src"
            "${PLAYFAB_SDK_PATH}/include"
            "${CMAKE_SOURCE_DIR}/tests/cpp/fuzz_harness"
    )
else()
    message(STATUS
        "playfab_fuzz_api_models: skipped (PLAYFAB_SDK_PATH not set or SDK not found). "
        "Set PLAYFAB_SDK_PATH=<path> to enable.")
endif()
