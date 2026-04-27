cmake_minimum_required(VERSION 3.20)
project(godot_gdk LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ── Submodule check ─────────────────────────────────────────────
if(NOT EXISTS "${CMAKE_SOURCE_DIR}/godot-cpp/CMakeLists.txt")
    message(FATAL_ERROR
        "godot-cpp submodule not found. Run:\n"
        "  git submodule update --init --recursive")
endif()

add_subdirectory(godot-cpp)

# ── GDK detection ───────────────────────────────────────────────
# Prefer the GRDKLatest env var set by the GDK installer, fall back to standard path.
if(DEFINED ENV{GRDKLatest})
    set(_GDK_GAMEKIT_DEFAULT "$ENV{GRDKLatest}/GameKit")
else()
    set(_GDK_ROOT "C:/Program Files (x86)/Microsoft GDK")
    file(GLOB _GDK_EDITIONS "${_GDK_ROOT}/[0-9]*")
    if(NOT _GDK_EDITIONS)
        message(FATAL_ERROR
            "Microsoft GDK not found.\n"
            "Install via: winget install Microsoft.Gaming.GDK\n"
            "Or set GDK_GAMEKIT to your GameKit path.")
    endif()
    list(SORT _GDK_EDITIONS ORDER DESCENDING)
    list(GET _GDK_EDITIONS 0 _GDK_EDITION_PATH)
    set(_GDK_GAMEKIT_DEFAULT "${_GDK_EDITION_PATH}/GRDK/GameKit")
endif()

set(GDK_GAMEKIT "${_GDK_GAMEKIT_DEFAULT}" CACHE PATH "Path to GDK GameKit directory")

if(NOT EXISTS "${GDK_GAMEKIT}/Include/XGameRuntimeInit.h")
    message(FATAL_ERROR
        "GDK GameKit headers not found at: ${GDK_GAMEKIT}\n"
        "Verify your GDK installation or set -DGDK_GAMEKIT=<path>")
endif()

message(STATUS "GDK GameKit: ${GDK_GAMEKIT}")

# ── Xbox Services API (XSAPI) detection ─────────────────────────
set(_XSAPI_EXT "${GDK_GAMEKIT}/../ExtensionLibraries")
get_filename_component(_XSAPI_EXT "${_XSAPI_EXT}" ABSOLUTE)

set(XSAPI_ROOT "${_XSAPI_EXT}/Xbox.Services.API.C" CACHE PATH "Path to Xbox Services API C")
set(LIBHTTPCLIENT_ROOT "${_XSAPI_EXT}/Xbox.LibHttpClient" CACHE PATH "Path to libHttpClient")

if(NOT EXISTS "${XSAPI_ROOT}/Include/xsapi-c/services_c.h")
    message(FATAL_ERROR
        "Xbox Services API (XSAPI) not found at: ${XSAPI_ROOT}\n"
        "Ensure the GDK is installed with Xbox Extensions, or set -DXSAPI_ROOT=<path>")
endif()

if(NOT EXISTS "${LIBHTTPCLIENT_ROOT}/Include/httpClient/httpClient.h")
    message(FATAL_ERROR
        "libHttpClient not found at: ${LIBHTTPCLIENT_ROOT}\n"
        "Ensure the GDK is installed with Xbox Extensions.")
endif()

message(STATUS "XSAPI: ${XSAPI_ROOT}")
message(STATUS "libHttpClient: ${LIBHTTPCLIENT_ROOT}")

# ── Plugin library ──────────────────────────────────────────────
add_library(godot_gdk SHARED
    src/register_types.cpp
    src/gdk_core.cpp
    src/gdk_user.cpp
    src/gdk_input.cpp
    src/gdk_achievements.cpp
)

target_include_directories(godot_gdk PRIVATE
    src/
    "${GDK_GAMEKIT}/Include"
    "${XSAPI_ROOT}/Include"
    "${LIBHTTPCLIENT_ROOT}/Include"
)

target_link_directories(godot_gdk PRIVATE
    "${GDK_GAMEKIT}/Lib/amd64"
    "${XSAPI_ROOT}/Lib/x64/$<IF:$<CONFIG:Debug>,Debug,Release>"
    "${LIBHTTPCLIENT_ROOT}/Lib/x64"
)

target_link_libraries(godot_gdk PRIVATE
    godot::cpp
    xgameruntime
    GameInput
    Microsoft.Xbox.Services.GDK.C.Thunks
    libHttpClient.GDK
)

# GDK platform identifier — required for XSAPI/libHttpClient headers
target_compile_definitions(godot_gdk PRIVATE _GAMING_DESKTOP)

# ── Standalone GameInput extension scaffold ──────────────────────
add_library(godot_gameinput SHARED
    src/godot_gameinput/register_types.cpp
    src/godot_gameinput/gameinput_extension.cpp
)

target_include_directories(godot_gameinput PRIVATE
    src/godot_gameinput/
)

target_link_libraries(godot_gameinput PRIVATE
    godot::cpp
)

# ── Output naming ───────────────────────────────────────────────
# Godot expects: <name>.windows.<profile>.x86_64.dll
set(ADDON_BIN "${CMAKE_SOURCE_DIR}/addons/godot_gdk/bin")
set(SAMPLE_BIN "${CMAKE_SOURCE_DIR}/sample/addons/godot_gdk/bin")

set_target_properties(godot_gdk PROPERTIES
    OUTPUT_NAME                      "godot_gdk.windows.release.x86_64"
    OUTPUT_NAME_DEBUG                "godot_gdk.windows.debug.x86_64"
    OUTPUT_NAME_RELWITHDEBINFO       "godot_gdk.windows.release.x86_64"
    OUTPUT_NAME_MINSIZEREL           "godot_gdk.windows.release.x86_64"
    RUNTIME_OUTPUT_DIRECTORY         "${ADDON_BIN}"
    RUNTIME_OUTPUT_DIRECTORY_DEBUG   "${ADDON_BIN}"
    RUNTIME_OUTPUT_DIRECTORY_RELEASE "${ADDON_BIN}"
    LIBRARY_OUTPUT_DIRECTORY         "${ADDON_BIN}"
    LIBRARY_OUTPUT_DIRECTORY_DEBUG   "${ADDON_BIN}"
    LIBRARY_OUTPUT_DIRECTORY_RELEASE "${ADDON_BIN}"
    PREFIX ""
    SUFFIX ".dll"
)

set(GAMEINPUT_ADDON_BIN "${CMAKE_SOURCE_DIR}/addons/godot_gameinput/bin")
set(GAMEINPUT_ADDON_SRC "${CMAKE_SOURCE_DIR}/addons/godot_gameinput")
set(GAMEINPUT_SAMPLE_ADDON_DST "${CMAKE_SOURCE_DIR}/sample/addons/godot_gameinput")
set(GAMEINPUT_SAMPLE_BIN "${GAMEINPUT_SAMPLE_ADDON_DST}/bin")

set_target_properties(godot_gameinput PROPERTIES
    OUTPUT_NAME                      "godot_gameinput.windows.release.x86_64"
    OUTPUT_NAME_DEBUG                "godot_gameinput.windows.debug.x86_64"
    OUTPUT_NAME_RELWITHDEBINFO       "godot_gameinput.windows.release.x86_64"
    OUTPUT_NAME_MINSIZEREL           "godot_gameinput.windows.release.x86_64"
    RUNTIME_OUTPUT_DIRECTORY         "${GAMEINPUT_ADDON_BIN}"
    RUNTIME_OUTPUT_DIRECTORY_DEBUG   "${GAMEINPUT_ADDON_BIN}"
    RUNTIME_OUTPUT_DIRECTORY_RELEASE "${GAMEINPUT_ADDON_BIN}"
    LIBRARY_OUTPUT_DIRECTORY         "${GAMEINPUT_ADDON_BIN}"
    LIBRARY_OUTPUT_DIRECTORY_DEBUG   "${GAMEINPUT_ADDON_BIN}"
    LIBRARY_OUTPUT_DIRECTORY_RELEASE "${GAMEINPUT_ADDON_BIN}"
    PREFIX ""
    SUFFIX ".dll"
)

add_custom_command(TARGET godot_gameinput POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory
        "${GAMEINPUT_SAMPLE_BIN}"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "$<TARGET_FILE:godot_gameinput>"
        "${GAMEINPUT_SAMPLE_BIN}/$<TARGET_FILE_NAME:godot_gameinput>"
    COMMENT "Copying GameInput DLL to sample project"
)

add_custom_command(TARGET godot_gameinput POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E
        $<IF:$<CONFIG:Debug>,copy_if_different,true>
        $<IF:$<CONFIG:Debug>,$<TARGET_FILE_DIR:godot_gameinput>/$<TARGET_FILE_BASE_NAME:godot_gameinput>.pdb,>
        $<IF:$<CONFIG:Debug>,${GAMEINPUT_SAMPLE_BIN}/,>
    COMMENT "Copying GameInput PDB to sample project (Debug only)"
)

add_custom_command(TARGET godot_gameinput POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory
        "${GAMEINPUT_SAMPLE_ADDON_DST}"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${GAMEINPUT_ADDON_SRC}/godot_gameinput.gdextension"
        "${GAMEINPUT_SAMPLE_ADDON_DST}/godot_gameinput.gdextension"
    COMMENT "Syncing GameInput addon files to sample project"
)

# ── Post-build: copy DLL + PDB to sample ────────────────────────
add_custom_command(TARGET godot_gdk POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "$<TARGET_FILE:godot_gdk>"
        "${SAMPLE_BIN}/$<TARGET_FILE_NAME:godot_gdk>"
    COMMENT "Copying DLL to sample project"
)

add_custom_command(TARGET godot_gdk POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E
        $<IF:$<CONFIG:Debug>,copy_if_different,true>
        $<IF:$<CONFIG:Debug>,$<TARGET_FILE_DIR:godot_gdk>/$<TARGET_FILE_BASE_NAME:godot_gdk>.pdb,>
        $<IF:$<CONFIG:Debug>,${SAMPLE_BIN}/,>
    COMMENT "Copying PDB to sample project (Debug only)"
)

# ── Post-build: copy XSAPI + libHttpClient runtime DLLs ─────────
set(_XSAPI_DLL "${XSAPI_ROOT}/Lib/x64/$<IF:$<CONFIG:Debug>,Debug,Release>/Microsoft.Xbox.Services.GDK.C.Thunks.dll")
set(_HTTPCLIENT_DLL "${LIBHTTPCLIENT_ROOT}/Redist/x64/libHttpClient.GDK.dll")

add_custom_command(TARGET godot_gdk POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different "${_XSAPI_DLL}" "${ADDON_BIN}/"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different "${_HTTPCLIENT_DLL}" "${ADDON_BIN}/"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different "${_XSAPI_DLL}" "${SAMPLE_BIN}/"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different "${_HTTPCLIENT_DLL}" "${SAMPLE_BIN}/"
    COMMENT "Copying XSAPI and libHttpClient runtime DLLs"
)

# ── Post-build: sync addon scripts to sample ────────────────────
set(ADDON_SRC "${CMAKE_SOURCE_DIR}/addons/godot_gdk")
set(ADDON_DST "${CMAKE_SOURCE_DIR}/sample/addons/godot_gdk")

add_custom_command(TARGET godot_gdk POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${ADDON_SRC}/plugin.cfg"
        "${ADDON_DST}/plugin.cfg"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${ADDON_SRC}/godot_gdk.gdextension"
        "${ADDON_DST}/godot_gdk.gdextension"
    COMMAND ${CMAKE_COMMAND} -E make_directory
        "${ADDON_DST}/editor"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${ADDON_SRC}/editor/gdk_editor_plugin.gd"
        "${ADDON_DST}/editor/gdk_editor_plugin.gd"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${ADDON_SRC}/editor/gdk_export_platform.gd"
        "${ADDON_DST}/editor/gdk_export_platform.gd"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${ADDON_SRC}/editor/gdk_setup_panel.gd"
        "${ADDON_DST}/editor/gdk_setup_panel.gd"
    COMMENT "Syncing addon scripts to sample project"
)
