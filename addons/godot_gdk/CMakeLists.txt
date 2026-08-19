include(GodotExtensionCommon)
include(GDKDependencies)

gdk_require_ms_gdk()
gdk_xsapi_thunks_dlls(XSAPI_THUNKS_DLLS)

add_library(godot_gdk SHARED
    src/register_types.cpp
    src/xbox.cpp
    src/xbox_accessibility.cpp
    src/xbox_runtime.cpp
    src/xbox_result.cpp
    src/xbox_result_codes_internal.cpp
    src/xbox_request_parsing.cpp
    src/xbox_pending_signal.cpp
    src/xbox_signal_xasync_context.cpp
    src/xbox_user.cpp
    src/xbox_game_ui.cpp
    src/xbox_achievement.cpp
    src/xbox_package.cpp
    src/xbox_stats.cpp
    src/xbox_leaderboards.cpp
    src/xbox_privacy.cpp
    src/xbox_presence.cpp
    src/xbox_social.cpp
    src/xbox_store.cpp
    src/xbox_profile.cpp
    src/xbox_string_verify.cpp
    src/xbox_title_storage.cpp
    src/xbox_error_reporting.cpp
    src/xbox_launcher.cpp
    src/xbox_multiplayer_activity.cpp
    src/xbox_capture.cpp
    src/xbox_system.cpp
    src/xbox_services.cpp
    src/xbox_display.cpp
    src/xbox_activation.cpp
    src/xbox_speech_synthesizer.cpp
    src/xbox_events.cpp
    src/xbox_game_save.cpp
    src/xbox_game_chat.cpp
    src/xbox_networking.cpp
)

target_include_directories(godot_gdk PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}/src"
)

target_link_libraries(godot_gdk PRIVATE
    godot::cpp
    Xbox::GameRuntime
    Xbox::XSAPI
    Xbox::HTTPClient
    Xbox::GameChat2
)

godot_cpp_config_guard(godot_gdk)

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

# Runtime DLLs to deploy alongside the addon. `Xbox::XSAPI` transitively links
# `Xbox::XCurl` (vcpkg ms-gdk port), which adds an import on XCurl.dll — ship
# it. The Thunks DLLs are the XSAPI runtime bridge; they aren't proper imported
# targets so we resolve their absolute paths explicitly via
# gdk_xsapi_thunks_dlls. Both Debug- and Release-named variants ship in every
# config (see comment on gdk_xsapi_thunks_dlls in cmake/GDKDependencies.cmake).
godot_addon_copy_runtime_files(
    TARGET godot_gdk
    ADDON_NAME godot_gdk
    PROJECT_DIRS ${GODOT_GDK_PROJECT_DIRS}
    FILES
        "$<TARGET_FILE:Xbox::HTTPClient>"
        "$<TARGET_FILE:Xbox::XCurl>"
        "$<TARGET_FILE:Xbox::GameChat2>"
        ${XSAPI_THUNKS_DLLS}
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
        editor/gdk_export_features_plugin.gd
        editor/gdk_setup_panel.gd
)
