include(GodotExtensionCommon)
include(GDKDependencies)

gdk_detect_dependencies(
    GDK_WINDOWS_OUT GDK_WINDOWS
    XSAPI_RUNTIME_DLL_OUT XSAPI_RUNTIME_DLL
    LIBHTTPCLIENT_RUNTIME_DLL_OUT LIBHTTPCLIENT_RUNTIME_DLL
)

add_library(godot_gdk SHARED
    src/register_types.cpp
    src/gdk.cpp
    src/gdk_runtime.cpp
    src/gdk_result.cpp
    src/gdk_async_op.cpp
    src/gdk_dispatch_op.cpp
    src/gdk_xasync_context.cpp
    src/gdk_user.cpp
    src/gdk_achievement.cpp
    src/gdk_presence.cpp
    src/gdk_social.cpp
    src/gdk_multiplayer_activity.cpp
    src/gdk_xbox_services.cpp
)

target_include_directories(godot_gdk PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}/src"
    "${GDK_WINDOWS}/include"
)

target_link_directories(godot_gdk PRIVATE
    "${GDK_WINDOWS}/lib/x64"
)

target_link_libraries(godot_gdk PRIVATE
    godot::cpp
    xgameruntime
    "$<IF:$<CONFIG:Debug>,Microsoft.Xbox.Services.C.Thunks.Debug,Microsoft.Xbox.Services.C.Thunks>"
    libHttpClient
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
