include(GodotExtensionCommon)
include(GDKDependencies)

gdk_require_ms_gdk()
gdk_xsapi_thunks_dlls(XSAPI_THUNKS_DLLS)

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
    src/gdk_package.cpp
    src/gdk_stats.cpp
    src/gdk_leaderboards.cpp
    src/gdk_privacy.cpp
    src/gdk_presence.cpp
    src/gdk_social.cpp
    src/gdk_store.cpp
    src/gdk_profile.cpp
    src/gdk_string_verify.cpp
    src/gdk_title_storage.cpp
    src/gdk_error_reporting.cpp
    src/gdk_launcher.cpp
    src/gdk_multiplayer_activity.cpp
    src/gdk_capture.cpp
    src/gdk_system.cpp
    src/gdk_xbox_services.cpp
    src/gdk_display.cpp
    src/gdk_activation.cpp
)

target_include_directories(godot_gdk PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}/src"
)

target_link_libraries(godot_gdk PRIVATE
    godot::cpp
    Xbox::GameRuntime
    Xbox::XSAPI
    Xbox::HTTPClient
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
        editor/gdk_setup_panel.gd
)
