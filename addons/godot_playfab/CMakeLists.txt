include(GodotExtensionCommon)
include(GDKDependencies)

gdk_require_ms_gdk()

add_library(godot_playfab SHARED
    src/register_types.cpp
    src/playfab.cpp
    src/playfab_gamesaves.cpp
    src/playfab_runtime.cpp
    src/playfab_result.cpp
    src/playfab_result_codes_internal.cpp
    src/playfab_pending_signal.cpp
    src/playfab_api_helpers.cpp
    src/playfab_signal_xasync_context.cpp
    src/playfab_user.cpp
    src/playfab_users.cpp
    src/playfab_leaderboards.cpp
    src/api/playfab_api_models.cpp
    src/api/playfab_api_services.cpp
    src/playfab_multiplayer.cpp
    src/playfab_party.cpp
)

target_include_directories(godot_playfab PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}/src"
)

target_link_libraries(godot_playfab PRIVATE
    godot::cpp
    Xbox::GameRuntime
    Xbox::HTTPClient
    Xbox::PlayFabCore
    Xbox::PlayFabServices
    Xbox::PlayFabMultiplayer
    Xbox::PlayFabParty
    Xbox::PlayFabGameSave
)

target_compile_definitions(godot_playfab PRIVATE _GAMING_DESKTOP)

godot_addon_configure_target(
    TARGET godot_playfab
    ADDON_NAME godot_playfab
    PROJECT_DIRS ${GODOT_PLAYFAB_PROJECT_DIRS}
)

if(GODOTCPP_TARGET MATCHES "editor|template_debug")
    file(GLOB_RECURSE DOC_XML LIST_DIRECTORIES NO CONFIGURE_DEPENDS
         "${CMAKE_CURRENT_SOURCE_DIR}/doc_classes/*.xml")
    godot_addon_doc_sources(
        TARGET     godot_playfab
        ADDON_NAME godot_playfab
        SOURCES    ${DOC_XML}
    )
endif()

# PlayFab runtime DLLs. vcpkg's ms-gdk port installs the SAME PlayFab DLL bits
# in both bin/ and debug/bin/ (the PF NuGet does not split Debug vs Release),
# so `$<TARGET_FILE:Xbox::PlayFab*>` resolves to a single file regardless of
# config. Behavior matches the previous hand-rolled deployment.
godot_addon_copy_runtime_files(
    TARGET godot_playfab
    ADDON_NAME godot_playfab
    PROJECT_DIRS ${GODOT_PLAYFAB_PROJECT_DIRS}
    FILES
        "$<TARGET_FILE:Xbox::HTTPClient>"
        "$<TARGET_FILE:Xbox::PlayFabCore>"
        "$<TARGET_FILE:Xbox::PlayFabGameSave>"
        "$<TARGET_FILE:Xbox::PlayFabMultiplayer>"
        "$<TARGET_FILE:Xbox::PlayFabParty>"
        "$<TARGET_FILE:Xbox::PlayFabServices>"
)

godot_addon_sync_files_to_sample(
    TARGET godot_playfab
    ADDON_NAME godot_playfab
    ADDON_ROOT "${CMAKE_CURRENT_SOURCE_DIR}"
    PROJECT_DIRS ${GODOT_PLAYFAB_PROJECT_DIRS}
    FILES
        plugin.cfg
        godot_playfab.gdextension
        runtime/playfab_bootstrap.gd
        editor/playfab_editor_plugin.gd
)
