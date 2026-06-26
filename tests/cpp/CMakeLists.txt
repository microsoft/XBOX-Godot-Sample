# C++ unit tests (doctest). Built when GDK_BUILD_TESTS is enabled.
# This subdirectory is added from the root CMakeLists guarded by
# GDK_BUILD_TESTS OR GDK_BUILD_FUZZ_TARGETS. The doctest exe itself is gated
# on GDK_BUILD_TESTS so the fuzz preset (which sets GDK_BUILD_TESTS=OFF) only
# builds the fuzz subdirectories.
#
# Layout (see spec/testing-strategy.md "doctest layout" and "libFuzzer layout"):
#   main.cpp              — lone TU defining DOCTEST_CONFIG_IMPLEMENT
#   result_codes/         — pure HRESULT formatting helper coverage
#   request_key/          — pure PlayFab request key-match helper coverage (Phase 2)
#   fuzz/                 — libFuzzer fuzz targets, guarded by GDK_BUILD_FUZZ_TARGETS
#   fuzz_harness/         — GDExtension stub harness (Phase 3), linked by fuzz targets
#   third_party/doctest/  — vendored upstream header (do not modify)

if(GDK_BUILD_TESTS)
    # Wave 2: pull the per-addon HRESULT formatting helpers
    # (gdk_internal:: and playfab_internal::) directly into the doctest TU.
    # These helpers are pure and self-contained — the headers self-include
    # <windows.h> for HRESULT and only depend on godot::cpp for godot::String —
    # so we can compile them straight in here without a dedicated static lib.
    # See tests/cpp/result_codes/test_*.cpp and spec/testing-strategy.md
    # ("C++ test scope rules") for the contract under test.
    set(_GDK_RC_SRC "${CMAKE_SOURCE_DIR}/addons/godot_gdk/src")
    set(_PF_RC_SRC  "${CMAKE_SOURCE_DIR}/addons/godot_playfab/src")

    godot_addon_doctest_target(
        TARGET_NAME gdk_unit_tests
        IMPL_TU     "${CMAKE_CURRENT_SOURCE_DIR}/main.cpp"
        SOURCES
            "${CMAKE_CURRENT_SOURCE_DIR}/result_codes/test_gdk_result_codes.cpp"
            "${CMAKE_CURRENT_SOURCE_DIR}/result_codes/test_playfab_result_codes.cpp"
            "${CMAKE_CURRENT_SOURCE_DIR}/request_key/test_playfab_request_key.cpp"
            "${_GDK_RC_SRC}/gdk_result_codes_internal.cpp"
            "${_PF_RC_SRC}/playfab_result_codes_internal.cpp"
            "${_PF_RC_SRC}/playfab_request_key.cpp"
        LINK_LIBS   godot::cpp
        INCLUDES
            "${CMAKE_CURRENT_SOURCE_DIR}"
            "${_GDK_RC_SRC}"
            "${_PF_RC_SRC}"
    )
endif()

# ── libFuzzer fuzz targets ────────────────────────────────────────
# Requires the `fuzz` configure preset (toolset: ClangCL).
# GDK_BUILD_FUZZ_TARGETS is declared at the root CMakeLists level and
# inherited here. Do NOT enable this option with the default MSVC preset —
# it will fail at godot_addon_fuzzer_target() with a clear error message.
if(GDK_BUILD_FUZZ_TARGETS)
    add_subdirectory(fuzz_harness)
    add_subdirectory(fuzz)
endif()
