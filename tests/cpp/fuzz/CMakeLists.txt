# libFuzzer fuzz targets. Built when GDK_BUILD_FUZZ_TARGETS=ON (the `fuzz`
# configure preset sets this). Requires clang-cl toolset.
#
# Phase 2 targets: pure C++ seams, no Godot types.
# Phase 3 targets: link against godot_stub_harness; full Godot-type coverage.
#
# Corpus convention: tests/cpp/fuzz/corpus/<target_name>/
# To run a target: .\build\fuzz\bin\Debug\<target>.exe -runs=100000 -max_len=256

set(_PF_SRC "${CMAKE_SOURCE_DIR}/addons/godot_playfab/src")
set(_GDK_SRC "${CMAKE_SOURCE_DIR}/addons/godot_gdk/src")

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

# ── Wrapped API fuzz targets (factory + accessor round-trips) ────────────
# fuzz_gdk_result_factories: exercises GDKResult and PlayFabResult factory
# methods (ok_result, error_result, hresult_error, cancelled) and accessors.
# Requires the object lifecycle stubs (stub_object.cpp) in godot_stub_harness.
godot_addon_fuzzer_target(
    TARGET_NAME gdk_fuzz_result_factories
    SOURCES
        "${CMAKE_CURRENT_SOURCE_DIR}/fuzz_gdk_result_factories.cpp"
        "${_GDK_SRC}/gdk_result.cpp"
        "${_GDK_SRC}/gdk_result_codes_internal.cpp"
        "${_PF_SRC}/playfab_result.cpp"
        "${_PF_SRC}/playfab_result_codes_internal.cpp"
    LINK_LIBS
        godot_stub_harness
        godot::cpp
    INCLUDES
        "${_GDK_SRC}"
        "${_PF_SRC}"
        "${CMAKE_SOURCE_DIR}/tests/cpp/fuzz_harness"
)

# fuzz_playfab_request_dictionary: exercises playfab_api::get_request_value
# with arbitrary Dictionary content and field-name pairs.  No SDK headers needed.
godot_addon_fuzzer_target(
    TARGET_NAME playfab_fuzz_request_dictionary
    SOURCES
        "${CMAKE_CURRENT_SOURCE_DIR}/fuzz_playfab_request_dictionary.cpp"
        "${_PF_SRC}/playfab_request_value.cpp"
    LINK_LIBS
        godot_stub_harness
        godot::cpp
    INCLUDES
        "${_PF_SRC}"
        "${CMAKE_SOURCE_DIR}/tests/cpp/fuzz_harness"
)

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
