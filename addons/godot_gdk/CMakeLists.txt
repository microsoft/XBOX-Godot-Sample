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
    src/gdk_accessibility.cpp
    src/gdk_runtime.cpp
    src/gdk_result.cpp
    src/gdk_result_codes_internal.cpp
    src/gdk_pending_signal.cpp
    src/gdk_signal_xasync_context.cpp
    src/gdk_user.cpp
    src/gdk_game_ui.cpp
    src/gdk_achievement.cpp
    src/gdk_presence.cpp
    src/gdk_social.cpp
    src/gdk_launcher.cpp
    src/gdk_multiplayer_activity.cpp
    src/gdk_system.cpp
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

godot_addon_configure_target(
    TARGET godot_gdk
    ADDON_NAME godot_gdk
    PROJECT_DIRS ${GODOT_GDK_PROJECT_DIRS}
)

if(GODOTCPP_TARGET MATCHES "editor|template_debug")
    file(GLOB_RECURSE DOC_XML LIST_DIRECTORIES NO CONFIGURE_DEPENDS
         "${CMAKE_CURRENT_SOURCE_DIR}/doc_classes/*.xml")
    godot_addon_doc_sources(
        TARGET     godot_gdk
        ADDON_NAME godot_gdk
        SOURCES    ${DOC_XML}
    )
endif()

godot_addon_copy_runtime_files(
    TARGET godot_gdk
    ADDON_NAME godot_gdk
    PROJECT_DIRS ${GODOT_GDK_PROJECT_DIRS}
    FILES
        "${XSAPI_RUNTIME_DLL}"
        "${LIBHTTPCLIENT_RUNTIME_DLL}"
)

godot_addon_sync_files_to_sample(
    TARGET godot_gdk
    ADDON_NAME godot_gdk
    ADDON_ROOT "${CMAKE_CURRENT_SOURCE_DIR}"
    PROJECT_DIRS ${GODOT_GDK_PROJECT_DIRS}
    FILES
        plugin.cfg
        godot_gdk.gdextension
        runtime/gdk_bootstrap.gd
        editor/gdk_editor_plugin.gd
        editor/gdk_export_platform.gd
        editor/gdk_setup_panel.gd
)
