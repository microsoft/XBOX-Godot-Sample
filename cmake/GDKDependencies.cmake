include_guard(GLOBAL)

# GDK build-time dependencies via vcpkg.
#
# This module brings in the Microsoft GDK (via the `ms-gdk[playfab]` port) and
# GameInput (via the `gameinput` port). It exposes:
#
#   Xbox::GameRuntime, Xbox::HTTPClient, Xbox::XCurl, Xbox::XSAPI,
#   Xbox::GameChat2, Xbox::PlayFab{Core,Services,Multiplayer,Party,
#   PartyLIVE,GameSave}                       -- from ms-gdk
#   Microsoft::GameInput                      -- from gameinput
#
# In addition, it provides cache/cmake variables addons use to deploy SDK
# runtime DLLs that are NOT exposed as proper imported targets — namely the
# Microsoft.Xbox.Services.C.Thunks DLLs (per-config Debug/Release file names)
# and the Microsoft.Xbox.Services.143.C.{Debug.pdb,pdb}. vcpkg lays these out
# as bin/<file>.dll for Release and debug/bin/<file>.Debug.dll for Debug.
#
# Consumer pattern (per-addon CMakeLists):
#
#   include(GDKDependencies)
#   gdk_require_ms_gdk()        # addons that need Xbox::XSAPI / PlayFab*
#   # or
#   gdk_require_gameinput()     # addons that only need Microsoft::GameInput
#   target_link_libraries(my_addon PRIVATE Xbox::XSAPI Xbox::HTTPClient ...)
#
# Local prereq: developers must have a vcpkg checkout and either set
# VCPKG_ROOT or pass `-DCMAKE_TOOLCHAIN_FILE=<vcpkg>/scripts/buildsystems/vcpkg.cmake`.
# The repo-default preset honors $env{VCPKG_ROOT}.

# Validate that a vcpkg toolchain file is wired into the build. Fail with a
# targeted message if VCPKG_ROOT was unset (the preset would expand to an
# invalid path that still ends in vcpkg.cmake) or if the toolchain file
# doesn't exist on disk.
function(_gdk_assert_vcpkg_toolchain)
    if(NOT DEFINED CMAKE_TOOLCHAIN_FILE OR NOT CMAKE_TOOLCHAIN_FILE MATCHES "vcpkg.cmake$")
        message(FATAL_ERROR
            "vcpkg toolchain file is required to build the GDK addons.\n"
            "Set the VCPKG_ROOT environment variable to a vcpkg checkout (e.g. C:/vcpkg) "
            "and reconfigure using the `default` preset, or pass "
            "-DCMAKE_TOOLCHAIN_FILE=<vcpkg-root>/scripts/buildsystems/vcpkg.cmake explicitly.\n"
            "vcpkg manifest mode is configured via vcpkg.json + vcpkg-configuration.json at the repo root.")
    endif()

    if(NOT EXISTS "${CMAKE_TOOLCHAIN_FILE}")
        message(FATAL_ERROR
            "CMAKE_TOOLCHAIN_FILE='${CMAKE_TOOLCHAIN_FILE}' does not exist on disk.\n"
            "This usually means VCPKG_ROOT is unset (the default preset expands "
            "$env{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake with an empty prefix).\n"
            "Set VCPKG_ROOT to a real vcpkg checkout (e.g. C:/vcpkg) and reconfigure.")
    endif()
endfunction()

# Require the `ms-gdk` port (Xbox::*, PlayFab*). Use this for the godot_gdk and
# godot_playfab addons. Kept separate from gdk_require_gameinput() so selective
# presets (gameinput-only, gdk-only, playfab-only) only restore what they need.
function(gdk_require_ms_gdk)
    _gdk_assert_vcpkg_toolchain()
    find_package(ms-gdk CONFIG REQUIRED)
endfunction()

# Require the `gameinput` port (Microsoft::GameInput). Use this for the
# godot_gameinput addon, which does not depend on any ms-gdk component.
function(gdk_require_gameinput)
    _gdk_assert_vcpkg_toolchain()
    find_package(gameinput CONFIG REQUIRED)
endfunction()

# Paths inside the vcpkg install tree for the XSAPI Thunks DLLs which are NOT
# exposed as proper imported targets. vcpkg lays these out as bin/<file>.dll
# for Release and debug/bin/<file>.Debug.dll for Debug.
#
# IMPORTANT: we deploy BOTH variants in BOTH configs (not one per config),
# for two independent reasons:
#
#   1. With CMAKE_MAP_IMPORTED_CONFIG_DEBUG=Release the Debug addon links
#      against Release Xbox::XSAPI .lib import entries (which reference
#      `Microsoft.Xbox.Services.C.Thunks.dll`). A Debug-only deploy that
#      ships only the `.Debug.dll` variant breaks addon loading on a clean
#      machine without GDK on PATH.
#   2. Empirically, dropping the `.Debug.dll` variant for Debug addon
#      builds causes a deterministic signal-11 shutdown crash in xsapi
#      teardown — XSAPI internals probe for the matching Debug Thunks DLL
#      at runtime even though it isn't a static import.
#
# Shipping both is the only configuration that satisfies both constraints,
# and the ~4.6 MiB extra in Release builds is an acceptable tradeoff for
# self-contained loading on machines without a GDK install.
#
# Usage:
#   gdk_xsapi_thunks_dlls(OUT_VAR)
#
# Returns a CMake list of two absolute paths.
function(gdk_xsapi_thunks_dlls OUT_VAR)
    if(NOT DEFINED VCPKG_INSTALLED_DIR)
        message(FATAL_ERROR "VCPKG_INSTALLED_DIR is not set; call gdk_require_ms_gdk() first.")
    endif()
    set(${OUT_VAR}
        "${VCPKG_INSTALLED_DIR}/${VCPKG_TARGET_TRIPLET}/bin/Microsoft.Xbox.Services.C.Thunks.dll"
        "${VCPKG_INSTALLED_DIR}/${VCPKG_TARGET_TRIPLET}/debug/bin/Microsoft.Xbox.Services.C.Thunks.Debug.dll"
        PARENT_SCOPE)
endfunction()
