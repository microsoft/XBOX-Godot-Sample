include_guard(GLOBAL)

# GDK build-time dependencies.
#
# This module brings in the Microsoft GDK (Xbox::* + Xbox::PlayFab* targets)
# and GameInput (Microsoft::GameInput), and exposes a small consumer surface:
#
#   gdk_require_ms_gdk()                  -- Xbox::* + Xbox::PlayFab*
#   gdk_require_gameinput()               -- Microsoft::GameInput
#   gdk_xsapi_thunks_dlls(OUT_VAR)        -- runtime DLLs to deploy
#
# The `ms-gdk` dependency can be satisfied from either of two sources:
#
#   - **vcpkg** (default): the `ms-gdk[playfab]` port. Requires only a
#     vcpkg checkout (no machine-wide GDK install). This is what CI uses
#     and what the `default` preset gives you.
#
#   - **installed GDK**: a developer's Microsoft GDK install on disk,
#     typically at `C:\Program Files (x86)\Microsoft GDK\<version>\`.
#     This module consumes the modern `windows\` subdirectory layout
#     (`windows\include`, `windows\lib\x64`, `windows\bin\x64`) first
#     shipped by the October 2025 GDK. Which edition to
#     build against is chosen by `GDK_VERSION` from the pre-approved
#     `GDK_SUPPORTED_VERSIONS` allowlist (see below); when unset, the oldest
#     approved edition that is installed is used. The legacy `GRDK\` peer
#     layout (`ExtensionLibraries\*\Lib\x64\...`, per-library flat
#     `Include\`) is **not** supported. Use the vcpkg source for older GDK
#     versions.
#     Selected by the `installed-gdk` preset, which does *not* load the
#     vcpkg toolchain at all -- a vcpkg checkout (or `VCPKG_ROOT`) is not
#     required when building in installed mode.
#
# Source selection is controlled by the `GDK_DEPENDENCY_SOURCE` cache
# variable, with values:
#
#   - `vcpkg` (default): use the pinned `ms-gdk[playfab]` vcpkg port.
#     Errors if no toolchain file is set.
#   - `installed`: use a discovered GDK install. Errors if none is found.
#     Set by the `installed-gdk` preset, which also drops the vcpkg
#     toolchain so the build needs no vcpkg checkout. The default preset
#     cannot be retargeted to installed via `-DGDK_DEPENDENCY_SOURCE=installed`
#     alone because the vcpkg toolchain processes manifest features
#     before this module runs -- always go through the `installed-gdk`
#     preset.
#
# GameInput v3 is sourced separately from the GDK because the installed
# GDK ships GameInput v1 only -- v3 is published independently by
# Microsoft. Source selection is controlled by `GAMEINPUT_SOURCE`:
#
#   - `vcpkg` (default): the `gameinput` vcpkg port. Used by the default
#     preset and the other vcpkg-based presets.
#   - `nuget`: pull the `Microsoft.GameInput` NuGet package directly from
#     nuget.org via `file(DOWNLOAD)`, cached under
#     `${CMAKE_BINARY_DIR}/_deps/`. No vcpkg toolchain required. Used by
#     the `installed-gdk` preset so GameInput stays available even when
#     vcpkg is not present.
#
# Both GameInput sources expose the same `Microsoft::GameInput` IMPORTED
# target -- the godot_gameinput addon's CMakeLists is identical in both
# cases. The vcpkg port and the NuGet path wrap the same upstream archive,
# so behavior is identical at runtime.
#
# An explicit GDK install path can also be supplied as `-DGDK_INSTALL_DIR=<path>`.
# The path may point at the version root (e.g. `...\260400\`), its
# `windows\` subdir, or its `GRDK\` peer (the latter is auto-redirected to
# the sibling `windows\` directory).
#
# Consumer pattern (per-addon CMakeLists):
#
#   include(GDKDependencies)
#   gdk_require_ms_gdk()
#   target_link_libraries(my_addon PRIVATE Xbox::XSAPI Xbox::HTTPClient ...)
#
# Imported targets exposed by `gdk_require_ms_gdk()` (identical names in
# both modes -- addon CMakeLists need no changes):
#
#   Xbox::GameRuntime, Xbox::HTTPClient, Xbox::XCurl, Xbox::XSAPI,
#   Xbox::GameChat2, Xbox::PlayFabCore, Xbox::PlayFabServices,
#   Xbox::PlayFabMultiplayer, Xbox::PlayFabParty, Xbox::PlayFabPartyLIVE,
#   Xbox::PlayFabGameSave

set(GDK_DEPENDENCY_SOURCE "vcpkg" CACHE STRING
    "Source for the Microsoft GDK dependency. One of: vcpkg, installed. \
Defaults to `vcpkg`: the pinned `ms-gdk[playfab]` vcpkg port. To consume \
an installed Microsoft GDK on disk instead, use the `installed-gdk` \
preset (it sets `GDK_DEPENDENCY_SOURCE=installed` *and* drops the vcpkg \
toolchain so no `VCPKG_ROOT` is required). Setting `installed` on the \
default preset will not work: the vcpkg toolchain processes manifest \
features before this module runs.")
set_property(CACHE GDK_DEPENDENCY_SOURCE PROPERTY STRINGS vcpkg installed)

set(GDK_INSTALL_DIR "" CACHE PATH
    "Optional path to a Microsoft GDK install. May point at the version root \
(e.g. `C:/Program Files (x86)/Microsoft GDK/251001/`), its `windows/` subdir, \
or its `GRDK/` peer (auto-redirected to the sibling `windows/`). Takes \
precedence over GDK_VERSION. Only used when GDK_DEPENDENCY_SOURCE is \
`installed`.")

# Pre-approved set of Microsoft GDK editions (6-digit, e.g. 251001 = October
# 2025, 260400 = April 2026) that the installed-GDK source may build against.
# Maintained in-repo: extend this default when a new edition is validated.
# `GDK_VERSION` must name one of these unless `GDK_ALLOW_UNAPPROVED` is ON.
set(GDK_SUPPORTED_VERSIONS "251001;251002;251003;260400;260401" CACHE STRING
    "Semicolon-separated list of pre-approved 6-digit Microsoft GDK editions \
the installed-GDK source may select. GDK_VERSION must be one of these unless \
GDK_ALLOW_UNAPPROVED is ON.")

# Select a specific installed Microsoft GDK edition to build against. Empty
# (default) selects the oldest edition from GDK_SUPPORTED_VERSIONS that is
# actually installed -- the broadest-compatible baseline. Only used when
# GDK_DEPENDENCY_SOURCE is `installed` and GDK_INSTALL_DIR is not set.
set(GDK_VERSION "" CACHE STRING
    "6-digit Microsoft GDK edition to build against (e.g. 251001). Must be a \
member of GDK_SUPPORTED_VERSIONS unless GDK_ALLOW_UNAPPROVED is ON. Empty \
selects the oldest approved edition that is installed. Ignored when \
GDK_INSTALL_DIR is set.")

# Escape hatch: when ON, GDK_VERSION may name an edition outside
# GDK_SUPPORTED_VERSIONS (a warning is emitted instead of a hard error),
# letting a developer build against an edition we have not pre-approved.
option(GDK_ALLOW_UNAPPROVED
    "Allow GDK_VERSION to name a Microsoft GDK edition not in GDK_SUPPORTED_VERSIONS (warns instead of failing)."
    OFF)

set(GAMEINPUT_SOURCE "vcpkg" CACHE STRING
    "Source for the GameInput v3 dependency. One of: vcpkg, nuget. \
Defaults to `vcpkg`: the `gameinput` port (which itself wraps the \
`Microsoft.GameInput` NuGet package). The `installed-gdk` preset \
overrides this to `nuget` so GameInput stays available without a vcpkg \
toolchain -- the NuGet package is fetched directly from nuget.org via \
`file(DOWNLOAD)` and cached under `${CMAKE_BINARY_DIR}/_deps/`. Both \
sources expose the same `Microsoft::GameInput` IMPORTED target.")
set_property(CACHE GAMEINPUT_SOURCE PROPERTY STRINGS vcpkg nuget)

# Pinned version of the Microsoft.GameInput NuGet package fetched when
# GAMEINPUT_SOURCE=nuget. Matches the version vcpkg's `gameinput` port
# currently wraps (same upstream archive, same SHA512). Bump together
# with the vcpkg port when refreshing.
set(GDK_GAMEINPUT_NUGET_VERSION "3.1.26100.6879" CACHE STRING
    "Version of the Microsoft.GameInput NuGet package fetched when GAMEINPUT_SOURCE=nuget.")
set(GDK_GAMEINPUT_NUGET_SHA512
    "7377a8cf9291318b99db4f94b6e2db6d8bd2a5afdac0b35bd38b3f51c75948a247e74dab155f2ba67d4ece78899e87c3e0e35510f1547bbc9b7c8202573a8ff6"
    CACHE STRING
    "SHA512 of the Microsoft.GameInput NuGet package archive. Used to verify the download.")
mark_as_advanced(GDK_GAMEINPUT_NUGET_VERSION GDK_GAMEINPUT_NUGET_SHA512)

# Internal cache. Populated by _gdk_resolve_dependency_source().
set(_GDK_RESOLVED_SOURCE "" CACHE INTERNAL "Resolved GDK source: vcpkg or installed.")
set(_GDK_RESOLVED_WINDOWS_PATH "" CACHE INTERNAL "Resolved GDK install `windows/` subdir path (installed mode only).")
set(_GDK_RESOLVED_FOR_INPUT "" CACHE INTERNAL "Input signature the resolution was computed from. Invalidates the resolution when the user changes GDK_DEPENDENCY_SOURCE, GDK_INSTALL_DIR, or the discovery env vars.")

# Validate the vcpkg toolchain wiring. Fails with a targeted message if
# VCPKG_ROOT was unset (the preset expands to a path that still ends in
# vcpkg.cmake but doesn't exist) or if the file isn't on disk.
function(_gdk_assert_vcpkg_toolchain)
    if(NOT DEFINED CMAKE_TOOLCHAIN_FILE OR NOT CMAKE_TOOLCHAIN_FILE MATCHES "vcpkg.cmake$")
        message(FATAL_ERROR
            "vcpkg toolchain file is required to satisfy this dependency.\n"
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

# Normalize a user-supplied or env-derived path into the `windows/` subdir
# of a Microsoft GDK install. Accepts:
#   - `<root>\<version>\windows\` (canonical -- passes through)
#   - `<root>\<version>\`        (with a `windows\` subdir -- joined)
#   - `<root>\<version>\GRDK\`   (auto-redirects to the sibling `windows\`)
# Sets `<OUT_VAR>` to the resolved `windows/` path, or empty if the input
# doesn't look like a usable install. The fingerprint file is
# `include/XGameRuntime.h`.
function(_gdk_normalize_install_path INPUT OUT_VAR)
    if(NOT INPUT)
        set(${OUT_VAR} "" PARENT_SCOPE)
        return()
    endif()

    file(TO_CMAKE_PATH "${INPUT}" _path)
    string(REGEX REPLACE "/+$" "" _path "${_path}")

    # Form 1: already pointing at the `windows/` subdir.
    if(EXISTS "${_path}/include/XGameRuntime.h" AND EXISTS "${_path}/lib/x64/xgameruntime.lib")
        set(${OUT_VAR} "${_path}" PARENT_SCOPE)
        return()
    endif()

    # Form 2: pointing at the version root; join `windows`.
    if(EXISTS "${_path}/windows/include/XGameRuntime.h")
        set(${OUT_VAR} "${_path}/windows" PARENT_SCOPE)
        return()
    endif()

    # Form 3: pointing at the GRDK peer; redirect to `../windows`.
    get_filename_component(_basename "${_path}" NAME)
    if(_basename STREQUAL "GRDK")
        get_filename_component(_parent "${_path}" DIRECTORY)
        if(EXISTS "${_parent}/windows/include/XGameRuntime.h")
            set(${OUT_VAR} "${_parent}/windows" PARENT_SCOPE)
            return()
        endif()
    endif()

    set(${OUT_VAR} "" PARENT_SCOPE)
endfunction()

# Validate an explicit GDK_VERSION value: must be a 6-digit edition, and a
# member of GDK_SUPPORTED_VERSIONS unless GDK_ALLOW_UNAPPROVED is ON. Fails
# loudly (or warns, with the escape hatch) so a typo'd or unvetted edition is
# caught up front rather than surfacing as a confusing "no install found".
function(_gdk_assert_version_value VERSION)
    if(NOT VERSION MATCHES "^[0-9][0-9][0-9][0-9][0-9][0-9]$")
        message(FATAL_ERROR
            "GDK_VERSION='${VERSION}' is not a valid Microsoft GDK edition. "
            "Expected a 6-digit edition such as 251001 (October 2025) or 260400 (April 2026).")
    endif()
    if(NOT VERSION IN_LIST GDK_SUPPORTED_VERSIONS)
        if(GDK_ALLOW_UNAPPROVED)
            message(WARNING
                "GDK_VERSION='${VERSION}' is not in the pre-approved list "
                "(GDK_SUPPORTED_VERSIONS='${GDK_SUPPORTED_VERSIONS}'). Proceeding because "
                "GDK_ALLOW_UNAPPROVED is ON -- this edition has not been validated.")
        else()
            message(FATAL_ERROR
                "GDK_VERSION='${VERSION}' is not a pre-approved Microsoft GDK edition.\n"
                "Approved editions: ${GDK_SUPPORTED_VERSIONS}.\n"
                "Pick one of those, or set -DGDK_ALLOW_UNAPPROVED=ON to build against an "
                "unvetted edition at your own risk.")
        endif()
    endif()
endfunction()

# Resolve a GDK install `windows/` subdir under an install root (e.g.
# `%GameDK%` or `C:/Program Files (x86)/Microsoft GDK`). When GDK_VERSION is
# set, only that edition's `<root>/<version>/windows` is considered. When
# unset, the oldest edition from GDK_SUPPORTED_VERSIONS that is installed
# under this root is chosen (favoring the broadest-compatible baseline).
# Sets `<OUT_VAR>` to the resolved `windows/` path, or empty string.
function(_gdk_pick_approved_version ROOT OUT_VAR)
    set(${OUT_VAR} "" PARENT_SCOPE)
    if(NOT ROOT OR NOT EXISTS "${ROOT}")
        return()
    endif()

    if(GDK_VERSION)
        if(EXISTS "${ROOT}/${GDK_VERSION}/windows/include/XGameRuntime.h")
            set(${OUT_VAR} "${ROOT}/${GDK_VERSION}/windows" PARENT_SCOPE)
        endif()
        return()
    endif()

    # Unset: oldest approved edition installed under this root. NATURAL sort
    # orders editions numerically (251001 < 260400) regardless of digit count.
    set(_approved ${GDK_SUPPORTED_VERSIONS})
    list(SORT _approved COMPARE NATURAL)
    foreach(_v IN LISTS _approved)
        if(EXISTS "${ROOT}/${_v}/windows/include/XGameRuntime.h")
            set(${OUT_VAR} "${ROOT}/${_v}/windows" PARENT_SCOPE)
            return()
        endif()
    endforeach()
endfunction()

# Discover a GDK install `windows/` path. Resolution order:
#   1. GDK_INSTALL_DIR (explicit override -- wins over edition selection).
#   2. The edition chosen by GDK_VERSION (or the oldest approved installed
#      edition when GDK_VERSION is unset), searched under %GameDK%, the
#      default install root, and the %GRDKLatest% install root.
# Returns empty string if none found.
function(_gdk_discover_install_path OUT_VAR)
    set(${OUT_VAR} "" PARENT_SCOPE)
    set(_default_root "C:/Program Files (x86)/Microsoft GDK")

    # 1. Explicit override.
    if(GDK_INSTALL_DIR)
        _gdk_normalize_install_path("${GDK_INSTALL_DIR}" _normalized)
        if(_normalized)
            set(${OUT_VAR} "${_normalized}" PARENT_SCOPE)
            return()
        else()
            message(FATAL_ERROR
                "GDK_INSTALL_DIR='${GDK_INSTALL_DIR}' does not look like a GDK install with "
                "the modern `windows/` layout.\n"
                "Expected a path to the version root (e.g. C:/Program Files (x86)/Microsoft GDK/251001/), "
                "its `windows/` subdir, or its `GRDK/` peer. The resolved `windows/` directory must "
                "contain `include/XGameRuntime.h`. GDK editions older than the October 2025 GDK do not "
                "ship this layout -- use -DGDK_DEPENDENCY_SOURCE=vcpkg instead.")
        endif()
    endif()

    # 2. Edition selection (GDK_VERSION pinned, or oldest approved installed).
    if(GDK_VERSION)
        _gdk_assert_version_value("${GDK_VERSION}")
    endif()

    # Candidate install roots that hold `<edition>/windows/...` subdirs.
    set(_roots "")
    if(DEFINED ENV{GameDK})
        file(TO_CMAKE_PATH "$ENV{GameDK}" _gamedk)
        list(APPEND _roots "${_gamedk}")
    endif()
    list(APPEND _roots "${_default_root}")
    # %GRDKLatest% points at <root>/<edition>/GRDK; its grandparent is the root.
    if(DEFINED ENV{GRDKLatest})
        file(TO_CMAKE_PATH "$ENV{GRDKLatest}" _grdk)
        get_filename_component(_grdk_edition_dir "${_grdk}" DIRECTORY)
        get_filename_component(_grdk_root "${_grdk_edition_dir}" DIRECTORY)
        list(APPEND _roots "${_grdk_root}")
    endif()
    list(REMOVE_DUPLICATES _roots)

    foreach(_root IN LISTS _roots)
        _gdk_pick_approved_version("${_root}" _win)
        if(_win)
            set(${OUT_VAR} "${_win}" PARENT_SCOPE)
            return()
        endif()
    endforeach()
endfunction()

# Validate GDK_DEPENDENCY_SOURCE and, when set to `installed`, discover
# and cache the resolved `windows/` directory. There is no auto-detection
# fallback: `vcpkg` is the default, and `installed` must be selected
# explicitly (normally by the `installed-gdk` preset). Idempotent -- safe
# to call from each addon.
#
# Re-resolves whenever GDK_DEPENDENCY_SOURCE, GDK_INSTALL_DIR, or the env
# vars used for installed-GDK discovery change across reconfigures
# (tracked via _GDK_RESOLVED_FOR_INPUT). Without this, switching presets
# in the same build dir would silently keep the prior resolution.
function(_gdk_resolve_dependency_source)
    set(_input_signature "${GDK_DEPENDENCY_SOURCE}|${GDK_INSTALL_DIR}|${GDK_VERSION}|${GDK_SUPPORTED_VERSIONS}|${GDK_ALLOW_UNAPPROVED}|$ENV{GRDKLatest}|$ENV{GameDK}")
    if(_GDK_RESOLVED_SOURCE AND _GDK_RESOLVED_FOR_INPUT STREQUAL "${_input_signature}")
        return()
    endif()

    if(NOT GDK_DEPENDENCY_SOURCE MATCHES "^(vcpkg|installed)$")
        message(FATAL_ERROR
            "Invalid GDK_DEPENDENCY_SOURCE='${GDK_DEPENDENCY_SOURCE}'. "
            "Allowed values: vcpkg, installed.")
    endif()

    set(_source "${GDK_DEPENDENCY_SOURCE}")

    if(_source STREQUAL "installed")
        _gdk_discover_install_path(_win)
        if(NOT _win)
            if(GDK_VERSION)
                set(_sel_desc "GDK_VERSION='${GDK_VERSION}'")
            else()
                set(_sel_desc "the oldest approved edition (GDK_SUPPORTED_VERSIONS='${GDK_SUPPORTED_VERSIONS}')")
            endif()
            message(FATAL_ERROR
                "GDK_DEPENDENCY_SOURCE=installed was requested but no matching GDK install was "
                "found for ${_sel_desc}.\n"
                "Looked for `<root>/<edition>/windows/include/XGameRuntime.h` under %GameDK%, "
                "`C:/Program Files (x86)/Microsoft GDK`, and the %GRDKLatest% root.\n"
                "Install one of the approved editions (e.g. `winget install Microsoft.Gaming.GDK`), "
                "select an installed edition with -DGDK_VERSION=<edition>, point at it with "
                "-DGDK_INSTALL_DIR=<path>, or use the default vcpkg preset "
                "(-DGDK_DEPENDENCY_SOURCE=vcpkg).")
        endif()
        set(_GDK_RESOLVED_WINDOWS_PATH "${_win}" CACHE INTERNAL "" FORCE)
        message(STATUS "GDK dependency source: installed (${_win})")
    else() # vcpkg
        set(_GDK_RESOLVED_WINDOWS_PATH "" CACHE INTERNAL "" FORCE)
        message(STATUS "GDK dependency source: vcpkg (ms-gdk[playfab] port)")
    endif()

    set(_GDK_RESOLVED_SOURCE "${_source}" CACHE INTERNAL "" FORCE)
    set(_GDK_RESOLVED_FOR_INPUT "${_input_signature}" CACHE INTERNAL "" FORCE)
endfunction()

# Resolve the Xbox Services (XSAPI) C static lib + its Debug sibling under
# `lib/x64`. Prefers the v143 (VS2022) variant and falls back to v142
# (VS2019), which is binary-compatible (MSVC guarantees ABI compatibility
# across VS2015-2022). The October 2025 GDK (251001) ships only the
# v142 variant; April 2026 (260400) ships both. Sets `<OUT_LIB>` and
# `<OUT_DEBUG>` to full paths, or empty strings if neither variant is present.
function(_gdk_resolve_xsapi_lib LIB_DIR OUT_LIB OUT_DEBUG)
    set(${OUT_LIB} "" PARENT_SCOPE)
    set(${OUT_DEBUG} "" PARENT_SCOPE)
    foreach(_ts IN ITEMS 143 142)
        if(EXISTS "${LIB_DIR}/Microsoft.Xbox.Services.${_ts}.C.lib")
            set(${OUT_LIB} "${LIB_DIR}/Microsoft.Xbox.Services.${_ts}.C.lib" PARENT_SCOPE)
            set(${OUT_DEBUG} "${LIB_DIR}/Microsoft.Xbox.Services.${_ts}.C.Debug.lib" PARENT_SCOPE)
            return()
        endif()
    endforeach()
endfunction()

# Define the Xbox::* + Xbox::PlayFab* imported targets pointing at an
# installed GDK's `windows/` subdir. The target shape (names, STATIC vs
# SHARED, transitive link relationships, MAP_IMPORTED_CONFIG_* properties)
# mirrors the vcpkg `ms-gdk` port's exported config so addon CMakeLists
# are agnostic.
#
# The `windows/` subdir lays out libs flat under `lib/x64/`, DLLs under
# `bin/x64/`, and headers under `include/`. PlayFab headers are nested at
# `include/playfab/{core,services,multiplayer,party,gamesave}/`, matching
# the vcpkg port's layout -- no header shimming is required.
#
# Idempotent -- guarded by Xbox::GameRuntime existence.
function(_gdk_define_installed_targets WIN_ROOT)
    if(TARGET Xbox::GameRuntime)
        return()
    endif()

    set(_inc "${WIN_ROOT}/include")
    set(_lib "${WIN_ROOT}/lib/x64")
    set(_bin "${WIN_ROOT}/bin/x64")

    # Resolve the toolset-versioned XSAPI C lib (v143 preferred, v142 fallback).
    _gdk_resolve_xsapi_lib("${_lib}" _xsapi_lib _xsapi_lib_debug)
    if(NOT _xsapi_lib)
        message(FATAL_ERROR
            "Installed GDK at ${WIN_ROOT} has no Xbox Services C lib (looked for "
            "Microsoft.Xbox.Services.143.C.lib then Microsoft.Xbox.Services.142.C.lib under "
            "${_lib}). The install may be incomplete -- reinstall the full GDK or use "
            "-DGDK_DEPENDENCY_SOURCE=vcpkg.")
    endif()
    get_filename_component(_xsapi_name "${_xsapi_lib}" NAME)
    message(STATUS "GDK XSAPI lib: ${_xsapi_name}")
    # Sanity check: every consumer expects these specific paths to exist.
    # Fail loudly here with the actual missing file rather than letting the
    # downstream linker complain about LNK1181 on a generated lib path.
    #
    # Core files are always required (godot_gdk and godot_gameinput both
    # depend on this surface). PlayFab files are only required when
    # BUILD_GODOT_PLAYFAB is ON.
    set(_required_files
        "${_inc}/XGameRuntime.h"
        "${_lib}/xgameruntime.lib"
        "${_xsapi_lib}"
        "${_xsapi_lib_debug}"
        "${_lib}/Microsoft.Xbox.Services.C.Thunks.lib"
        "${_bin}/Microsoft.Xbox.Services.C.Thunks.dll"
        "${_bin}/Microsoft.Xbox.Services.C.Thunks.Debug.dll"
        "${_lib}/libHttpClient.lib"
        "${_bin}/libHttpClient.dll"
        "${_lib}/XCurl.lib"
        "${_bin}/XCurl.dll"
        "${_lib}/GameChat2.lib"
        "${_bin}/GameChat2.dll")
    if(BUILD_GODOT_PLAYFAB)
        list(APPEND _required_files
            "${_lib}/PlayFabCore.lib"
            "${_bin}/PlayFabCore.dll"
            "${_lib}/PlayFabServices.lib"
            "${_bin}/PlayFabServices.dll"
            "${_lib}/PlayFabGameSave.lib"
            "${_bin}/PlayFabGameSave.dll"
            "${_lib}/PlayFabMultiplayer.lib"
            "${_bin}/PlayFabMultiplayer.dll"
            "${_inc}/playfab/multiplayer/PFMultiplayer.h"
            "${_inc}/playfab/multiplayer/PFLobby.h"
            "${_inc}/playfab/multiplayer/PFMatchmaking.h"
            "${_inc}/playfab/multiplayer/PFEntityKey.h"
            "${_lib}/Party.lib"
            "${_bin}/Party.dll"
            "${_inc}/playfab/party/Party.h"
            "${_inc}/playfab/party/PartyImpl.h"
            "${_inc}/playfab/party/PartyTypes.h"
            "${_lib}/PartyXboxLive.lib"
            "${_bin}/PartyXboxLive.dll")
    endif()
    foreach(_f IN LISTS _required_files)
        if(NOT EXISTS "${_f}")
            message(FATAL_ERROR
                "Installed GDK at ${WIN_ROOT} is missing required file:\n  ${_f}\n"
                "This usually means the install is incomplete or this GDK edition pre-dates the "
                "`windows/` layout this addon expects (October 2025 or later). Reinstall the "
                "full GDK from https://github.com/microsoft/GDK or switch to the vcpkg source with "
                "-DGDK_DEPENDENCY_SOURCE=vcpkg.")
        endif()
    endforeach()

    # Xbox::GameRuntime (STATIC)
    add_library(Xbox::GameRuntime STATIC IMPORTED)
    set_target_properties(Xbox::GameRuntime PROPERTIES
        IMPORTED_LOCATION "${_lib}/xgameruntime.lib"
        MAP_IMPORTED_CONFIG_MINSIZEREL ""
        MAP_IMPORTED_CONFIG_RELWITHDEBINFO ""
        INTERFACE_INCLUDE_DIRECTORIES "${_inc}"
        INTERFACE_COMPILE_FEATURES "cxx_std_11"
        IMPORTED_LINK_INTERFACE_LANGUAGES "CXX")

    # Xbox::XCurl (SHARED)
    add_library(Xbox::XCurl SHARED IMPORTED)
    set_target_properties(Xbox::XCurl PROPERTIES
        IMPORTED_LOCATION "${_bin}/XCurl.dll"
        IMPORTED_IMPLIB "${_lib}/XCurl.lib"
        MAP_IMPORTED_CONFIG_MINSIZEREL ""
        MAP_IMPORTED_CONFIG_RELWITHDEBINFO ""
        INTERFACE_INCLUDE_DIRECTORIES "${_inc}")

    # Xbox::HTTPClient (SHARED)
    add_library(Xbox::HTTPClient SHARED IMPORTED)
    set_target_properties(Xbox::HTTPClient PROPERTIES
        IMPORTED_LOCATION "${_bin}/libHttpClient.dll"
        IMPORTED_IMPLIB "${_lib}/libHttpClient.lib"
        MAP_IMPORTED_CONFIG_MINSIZEREL ""
        MAP_IMPORTED_CONFIG_RELWITHDEBINFO ""
        INTERFACE_INCLUDE_DIRECTORIES "${_inc}")

    # Xbox::XSAPI (STATIC, v143 preferred / v142 fallback)
    #
    # _xsapi_lib was resolved above (prefers the v143 VS2022 variant, falls
    # back to the v142 VS2019 variant for editions that ship only v142, e.g.
    # the October 2025 GDK). CMAKE_MAP_IMPORTED_CONFIG_DEBUG=Release (set
    # globally by the default preset and inherited by installed-gdk) maps the
    # Debug addon build to this Release lib, matching the vcpkg ms-gdk port's
    # behavior.
    add_library(Xbox::XSAPI STATIC IMPORTED)
    set_target_properties(Xbox::XSAPI PROPERTIES
        IMPORTED_LOCATION "${_xsapi_lib}"
        MAP_IMPORTED_CONFIG_MINSIZEREL ""
        MAP_IMPORTED_CONFIG_RELWITHDEBINFO ""
        INTERFACE_INCLUDE_DIRECTORIES "${_inc}"
        IMPORTED_LINK_INTERFACE_LANGUAGES "CXX")
    target_link_libraries(Xbox::XSAPI INTERFACE
        Xbox::HTTPClient Xbox::XCurl appnotify.lib winhttp.lib crypt32.lib)

    # Xbox::GameChat2 (SHARED)
    add_library(Xbox::GameChat2 SHARED IMPORTED)
    set_target_properties(Xbox::GameChat2 PROPERTIES
        IMPORTED_LOCATION "${_bin}/GameChat2.dll"
        IMPORTED_IMPLIB "${_lib}/GameChat2.lib"
        MAP_IMPORTED_CONFIG_MINSIZEREL ""
        MAP_IMPORTED_CONFIG_RELWITHDEBINFO ""
        INTERFACE_INCLUDE_DIRECTORIES "${_inc}")

    if(BUILD_GODOT_PLAYFAB)
        # Xbox::PlayFabCore (SHARED)
        add_library(Xbox::PlayFabCore SHARED IMPORTED)
        set_target_properties(Xbox::PlayFabCore PROPERTIES
            IMPORTED_LOCATION "${_bin}/PlayFabCore.dll"
            IMPORTED_IMPLIB "${_lib}/PlayFabCore.lib"
            MAP_IMPORTED_CONFIG_MINSIZEREL ""
            MAP_IMPORTED_CONFIG_RELWITHDEBINFO ""
            INTERFACE_INCLUDE_DIRECTORIES "${_inc}"
            IMPORTED_LINK_INTERFACE_LANGUAGES "CXX")

        # Xbox::PlayFabServices (SHARED, links PlayFabCore + XCurl)
        add_library(Xbox::PlayFabServices SHARED IMPORTED)
        set_target_properties(Xbox::PlayFabServices PROPERTIES
            IMPORTED_LOCATION "${_bin}/PlayFabServices.dll"
            IMPORTED_IMPLIB "${_lib}/PlayFabServices.lib"
            IMPORTED_LINK_DEPENDENT_LIBRARIES Xbox::XCurl
            MAP_IMPORTED_CONFIG_MINSIZEREL ""
            MAP_IMPORTED_CONFIG_RELWITHDEBINFO ""
            INTERFACE_INCLUDE_DIRECTORIES "${_inc}"
            IMPORTED_LINK_INTERFACE_LANGUAGES "CXX")
        target_link_libraries(Xbox::PlayFabServices INTERFACE Xbox::PlayFabCore Xbox::XCurl)

        # Xbox::PlayFabGameSave (SHARED)
        add_library(Xbox::PlayFabGameSave SHARED IMPORTED)
        set_target_properties(Xbox::PlayFabGameSave PROPERTIES
            IMPORTED_LOCATION "${_bin}/PlayFabGameSave.dll"
            IMPORTED_IMPLIB "${_lib}/PlayFabGameSave.lib"
            MAP_IMPORTED_CONFIG_MINSIZEREL ""
            MAP_IMPORTED_CONFIG_RELWITHDEBINFO ""
            INTERFACE_INCLUDE_DIRECTORIES "${_inc}"
            IMPORTED_LINK_INTERFACE_LANGUAGES "CXX")

        # Xbox::PlayFabMultiplayer (SHARED, links XCurl)
        add_library(Xbox::PlayFabMultiplayer SHARED IMPORTED)
        set_target_properties(Xbox::PlayFabMultiplayer PROPERTIES
            IMPORTED_LOCATION "${_bin}/PlayFabMultiplayer.dll"
            IMPORTED_IMPLIB "${_lib}/PlayFabMultiplayer.lib"
            IMPORTED_LINK_DEPENDENT_LIBRARIES Xbox::XCurl
            MAP_IMPORTED_CONFIG_MINSIZEREL ""
            MAP_IMPORTED_CONFIG_RELWITHDEBINFO ""
            INTERFACE_INCLUDE_DIRECTORIES "${_inc}")
        target_link_libraries(Xbox::PlayFabMultiplayer INTERFACE Xbox::XCurl)

        # Xbox::PlayFabParty (SHARED)
        add_library(Xbox::PlayFabParty SHARED IMPORTED)
        set_target_properties(Xbox::PlayFabParty PROPERTIES
            IMPORTED_LOCATION "${_bin}/Party.dll"
            IMPORTED_IMPLIB "${_lib}/Party.lib"
            MAP_IMPORTED_CONFIG_MINSIZEREL ""
            MAP_IMPORTED_CONFIG_RELWITHDEBINFO ""
            INTERFACE_INCLUDE_DIRECTORIES "${_inc}")

        # Xbox::PlayFabPartyLIVE (SHARED, links PlayFabParty)
        add_library(Xbox::PlayFabPartyLIVE SHARED IMPORTED)
        set_target_properties(Xbox::PlayFabPartyLIVE PROPERTIES
            IMPORTED_LOCATION "${_bin}/PartyXboxLive.dll"
            IMPORTED_IMPLIB "${_lib}/PartyXboxLive.lib"
            IMPORTED_LINK_DEPENDENT_LIBRARIES Xbox::PlayFabParty
            MAP_IMPORTED_CONFIG_MINSIZEREL ""
            MAP_IMPORTED_CONFIG_RELWITHDEBINFO ""
            INTERFACE_INCLUDE_DIRECTORIES "${_inc}")
        target_link_libraries(Xbox::PlayFabPartyLIVE INTERFACE Xbox::PlayFabParty)
    endif()
endfunction()

# Backfill `Xbox::PlayFabGameSave` on the vcpkg path when the resolved
# `ms-gdk` port did not export it.
#
# The October 2025 (2510.x) ms-gdk ports ship the PlayFab Game Save binaries
# (`lib/PlayFabGameSave.lib` + `bin/PlayFabGameSave.dll`) and headers, but
# their exported `ms-gdk-config.cmake` omits the `Xbox::PlayFabGameSave`
# IMPORTED target -- that target was only added in the April 2026 (2604.x)
# port. Without this backfill, godot_playfab (which links Xbox::PlayFabGameSave)
# fails to configure against an October 2025 ms-gdk port even though the bits
# are present.
#
# Mirrors the port's own single-location Xbox::PlayFab* target shape: the
# PlayFab NuGet does not split Debug vs Release bits, so a single
# IMPORTED_LOCATION/IMPORTED_IMPLIB (the release layout) is correct for every
# config. The install prefix is derived from the always-present
# Xbox::PlayFabCore target so this works regardless of triplet/install path.
function(_gdk_backfill_vcpkg_playfab_gamesave)
    if(TARGET Xbox::PlayFabGameSave)
        return()
    endif()
    if(NOT TARGET Xbox::PlayFabCore)
        return()
    endif()

    get_target_property(_implib Xbox::PlayFabCore IMPORTED_IMPLIB)
    if(NOT _implib)
        get_target_property(_implib Xbox::PlayFabCore IMPORTED_IMPLIB_RELEASE)
    endif()
    if(NOT _implib)
        return()
    endif()

    get_filename_component(_lib_dir "${_implib}" DIRECTORY)
    get_filename_component(_root "${_lib_dir}" DIRECTORY)
    set(_gs_lib "${_root}/lib/PlayFabGameSave.lib")
    set(_gs_dll "${_root}/bin/PlayFabGameSave.dll")

    if(NOT (EXISTS "${_gs_lib}" AND EXISTS "${_gs_dll}"))
        message(FATAL_ERROR
            "The resolved ms-gdk vcpkg port did not export Xbox::PlayFabGameSave "
            "and the PlayFab Game Save binaries were not found under '${_root}' "
            "(expected lib/PlayFabGameSave.lib + bin/PlayFabGameSave.dll). This "
            "edition of the ms-gdk port does not provide PlayFab Game Save; pin a "
            "newer ms-gdk version or disable PlayFab (-DBUILD_GODOT_PLAYFAB=OFF).")
    endif()

    add_library(Xbox::PlayFabGameSave SHARED IMPORTED)
    set_target_properties(Xbox::PlayFabGameSave PROPERTIES
        IMPORTED_LOCATION "${_gs_dll}"
        IMPORTED_IMPLIB "${_gs_lib}"
        MAP_IMPORTED_CONFIG_MINSIZEREL ""
        MAP_IMPORTED_CONFIG_RELWITHDEBINFO ""
        INTERFACE_INCLUDE_DIRECTORIES "${_root}/include")
    message(STATUS
        "ms-gdk port omitted Xbox::PlayFabGameSave; backfilled from ${_root}/{lib,bin}")
endfunction()

# Require the GDK (Xbox::* + Xbox::PlayFab* targets). Dispatches on the
# resolved source -- vcpkg's ms-gdk port or an installed GDK on disk.
# Idempotent.
function(gdk_require_ms_gdk)
    _gdk_resolve_dependency_source()
    if(_GDK_RESOLVED_SOURCE STREQUAL "vcpkg")
        _gdk_assert_vcpkg_toolchain()
        find_package(ms-gdk CONFIG REQUIRED)
        if(BUILD_GODOT_PLAYFAB)
            _gdk_backfill_vcpkg_playfab_gamesave()
        endif()
    else() # installed
        _gdk_define_installed_targets("${_GDK_RESOLVED_WINDOWS_PATH}")
    endif()
endfunction()

# Fetch the Microsoft.GameInput NuGet package directly from nuget.org and
# cache it under `${CMAKE_BINARY_DIR}/_deps/gameinput-nuget-<version>/`.
# Idempotent -- second and subsequent configures reuse the cached extract.
# Sets `<OUT_DIR>` in the parent scope to the extracted package root, which
# contains:
#
#   native/include/GameInput.h          (v3 header; v0/v1/v2 subdirs ship
#                                        backward-compat headers we don't use)
#   native/lib/x64/GameInput.lib        (import lib)
#   redist/GameInputRedist.msi          (runtime redistributable for
#                                        deployment on target machines)
#   LICENSE.txt
#
# The package is verified against GDK_GAMEINPUT_NUGET_SHA512 before
# extraction. Bumping GDK_GAMEINPUT_NUGET_VERSION re-downloads because the
# cache directory name embeds the version.
function(_gdk_fetch_gameinput_nuget OUT_DIR)
    set(_ver "${GDK_GAMEINPUT_NUGET_VERSION}")
    set(_sha "${GDK_GAMEINPUT_NUGET_SHA512}")
    set(_cache_dir "${CMAKE_BINARY_DIR}/_deps/gameinput-nuget-${_ver}")
    set(_pkg "${CMAKE_BINARY_DIR}/_deps/gameinput-nuget-${_ver}.nupkg")
    set(_fingerprint "${_cache_dir}/native/lib/x64/GameInput.lib")

    if(NOT EXISTS "${_fingerprint}")
        message(STATUS "Fetching Microsoft.GameInput ${_ver} from nuget.org "
                       "(GAMEINPUT_SOURCE=nuget)")
        file(MAKE_DIRECTORY "${_cache_dir}")
        file(DOWNLOAD
            "https://www.nuget.org/api/v2/package/Microsoft.GameInput/${_ver}"
            "${_pkg}"
            EXPECTED_HASH SHA512=${_sha}
            STATUS _dl_status
            SHOW_PROGRESS)
        list(GET _dl_status 0 _dl_code)
        if(NOT _dl_code EQUAL 0)
            list(GET _dl_status 1 _dl_msg)
            file(REMOVE "${_pkg}")
            message(FATAL_ERROR
                "Failed to download Microsoft.GameInput ${_ver} from nuget.org "
                "(code ${_dl_code}): ${_dl_msg}.\n"
                "Network access is required the first time you configure with "
                "GAMEINPUT_SOURCE=nuget. To use a different version, override "
                "-DGDK_GAMEINPUT_NUGET_VERSION=<version> "
                "-DGDK_GAMEINPUT_NUGET_SHA512=<hash>. To skip GameInput, "
                "set -DBUILD_GODOT_GAMEINPUT=OFF.")
        endif()
        file(ARCHIVE_EXTRACT INPUT "${_pkg}" DESTINATION "${_cache_dir}")
        file(REMOVE "${_pkg}")
        if(NOT EXISTS "${_fingerprint}")
            message(FATAL_ERROR
                "Microsoft.GameInput ${_ver} extracted from nuget.org but "
                "expected file is missing: ${_fingerprint}. The NuGet package "
                "layout may have changed.")
        endif()
    endif()

    set(${OUT_DIR} "${_cache_dir}" PARENT_SCOPE)
endfunction()

# Define the `Microsoft::GameInput` IMPORTED target from an extracted
# NuGet package directory. Target shape mirrors what vcpkg's `gameinput`
# port exports (INTERFACE library with linked .lib + include dir) so
# addon CMakeLists work against either source unchanged.
#
# Idempotent -- guarded by Microsoft::GameInput existence.
function(_gdk_define_gameinput_nuget_target NUGET_DIR)
    if(TARGET Microsoft::GameInput)
        return()
    endif()

    set(_inc "${NUGET_DIR}/native/include")
    set(_lib "${NUGET_DIR}/native/lib/x64/GameInput.lib")

    if(NOT EXISTS "${_inc}/GameInput.h")
        message(FATAL_ERROR
            "Microsoft.GameInput NuGet package missing header: ${_inc}/GameInput.h")
    endif()
    if(NOT EXISTS "${_lib}")
        message(FATAL_ERROR
            "Microsoft.GameInput NuGet package missing import lib: ${_lib}")
    endif()

    add_library(Microsoft::GameInput INTERFACE IMPORTED)
    set_target_properties(Microsoft::GameInput PROPERTIES
        INTERFACE_LINK_LIBRARIES      "${_lib}"
        INTERFACE_INCLUDE_DIRECTORIES "${_inc}")
endfunction()

# Require GameInput (Microsoft::GameInput). Dispatches on GAMEINPUT_SOURCE:
#
#   - `vcpkg` (default): the `gameinput` vcpkg port. Requires the vcpkg
#     toolchain. This is what the `default` preset uses.
#
#   - `nuget`: pull the `Microsoft.GameInput` NuGet package from nuget.org
#     directly (the same upstream the vcpkg port wraps). Requires no vcpkg
#     toolchain -- the package is fetched via `file(DOWNLOAD)` and cached
#     under `${CMAKE_BINARY_DIR}/_deps/`. This is what the `installed-gdk`
#     preset uses so GameInput v3 stays available even without vcpkg.
#
# Both paths expose the same `Microsoft::GameInput` IMPORTED INTERFACE
# target -- addon CMakeLists need no changes.
function(gdk_require_gameinput)
    if(GAMEINPUT_SOURCE STREQUAL "nuget")
        _gdk_fetch_gameinput_nuget(_nuget_dir)
        _gdk_define_gameinput_nuget_target("${_nuget_dir}")
    elseif(GAMEINPUT_SOURCE STREQUAL "vcpkg")
        _gdk_assert_vcpkg_toolchain()
        find_package(gameinput CONFIG REQUIRED)
    else()
        message(FATAL_ERROR
            "Invalid GAMEINPUT_SOURCE='${GAMEINPUT_SOURCE}'. "
            "Allowed values: vcpkg, nuget.")
    endif()
endfunction()

# Absolute paths to the XSAPI Thunks DLLs that must be deployed alongside
# the addon binaries. The Thunks DLLs are the XSAPI runtime bridge and
# are not exposed as proper imported targets in either source.
#
# Usage:
#   gdk_xsapi_thunks_dlls(OUT_VAR)
#
# Returns TWO paths in both modes -- the Release
# `Microsoft.Xbox.Services.C.Thunks.dll` and the renamed Debug
# `Microsoft.Xbox.Services.C.Thunks.Debug.dll`. Both ship in every config
# because (a) CMAKE_MAP_IMPORTED_CONFIG_DEBUG=Release means the Debug
# addon links Release `.lib` import entries which reference
# `Microsoft.Xbox.Services.C.Thunks.dll`, and (b) XSAPI internals probe
# for a `.Debug.dll` variant at runtime in Debug builds even though it
# isn't a static import. Shipping both prevents a deterministic SIGSEGV
# in xsapi teardown.
function(gdk_xsapi_thunks_dlls OUT_VAR)
    _gdk_resolve_dependency_source()
    if(_GDK_RESOLVED_SOURCE STREQUAL "vcpkg")
        if(NOT DEFINED VCPKG_INSTALLED_DIR)
            message(FATAL_ERROR "VCPKG_INSTALLED_DIR is not set; call gdk_require_ms_gdk() first.")
        endif()
        set(${OUT_VAR}
            "${VCPKG_INSTALLED_DIR}/${VCPKG_TARGET_TRIPLET}/bin/Microsoft.Xbox.Services.C.Thunks.dll"
            "${VCPKG_INSTALLED_DIR}/${VCPKG_TARGET_TRIPLET}/debug/bin/Microsoft.Xbox.Services.C.Thunks.Debug.dll"
            PARENT_SCOPE)
    else() # installed
        set(${OUT_VAR}
            "${_GDK_RESOLVED_WINDOWS_PATH}/bin/x64/Microsoft.Xbox.Services.C.Thunks.dll"
            "${_GDK_RESOLVED_WINDOWS_PATH}/bin/x64/Microsoft.Xbox.Services.C.Thunks.Debug.dll"
            PARENT_SCOPE)
    endif()
endfunction()
