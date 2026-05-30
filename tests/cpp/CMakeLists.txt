# C++ unit tests (doctest). Built when GDK_BUILD_TESTS is enabled.
# This subdirectory is added from the root CMakeLists guarded by that option,
# so we do NOT re-check the option here.
#
# Layout (see spec/testing-strategy.md "doctest layout"):
#   main.cpp              — lone TU defining DOCTEST_CONFIG_IMPLEMENT
#   result_codes/         — pure HRESULT formatting helper coverage
#   third_party/doctest/  — vendored upstream header (do not modify)

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
        "${CMAKE_CURRENT_SOURCE_DIR}/playfab/test_owned_utf8.cpp"
        "${_GDK_RC_SRC}/gdk_result_codes_internal.cpp"
        "${_PF_RC_SRC}/playfab_result_codes_internal.cpp"
    LINK_LIBS   godot::cpp
    INCLUDES
        "${CMAKE_CURRENT_SOURCE_DIR}"
        "${_GDK_RC_SRC}"
        "${_PF_RC_SRC}"
)
