include(GodotExtensionCommon)
include(GDKDependencies)

gdk_detect_dependencies(
    GDK_GAMEKIT_OUT GDK_GAMEKIT
    XSAPI_ROOT_OUT XSAPI_ROOT
    LIBHTTPCLIENT_ROOT_OUT LIBHTTPCLIENT_ROOT
    XSAPI_RUNTIME_DLL_OUT XSAPI_RUNTIME_DLL
    LIBHTTPCLIENT_RUNTIME_DLL_OUT LIBHTTPCLIENT_RUNTIME_DLL
)

add_library(godot_gdk SHARED
    src/register_types.cpp
    src/gdk_core.cpp
    src/gdk_user.cpp
    src/gdk_input.cpp
    src/gdk_achievements.cpp
)

target_include_directories(godot_gdk PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}/src"
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

target_compile_definitions(godot_gdk PRIVATE _GAMING_DESKTOP)

godot_addon_configure_target(TARGET godot_gdk ADDON_NAME godot_gdk)

godot_addon_copy_runtime_files(
    TARGET godot_gdk
    ADDON_NAME godot_gdk
    FILES
        "${XSAPI_RUNTIME_DLL}"
        "${LIBHTTPCLIENT_RUNTIME_DLL}"
)

godot_addon_sync_files_to_sample(
    TARGET godot_gdk
    ADDON_NAME godot_gdk
    ADDON_ROOT "${CMAKE_CURRENT_SOURCE_DIR}"
    FILES
        plugin.cfg
        godot_gdk.gdextension
        editor/gdk_editor_plugin.gd
        editor/gdk_export_platform.gd
        editor/gdk_setup_panel.gd
)
