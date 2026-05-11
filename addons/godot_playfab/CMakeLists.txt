include(GodotExtensionCommon)
include(GDKDependencies)

gdk_detect_dependencies(
    GDK_WINDOWS_OUT GDK_WINDOWS
    LIBHTTPCLIENT_RUNTIME_DLL_OUT LIBHTTPCLIENT_RUNTIME_DLL
    PLAYFAB_CORE_RUNTIME_DLL_OUT PLAYFAB_CORE_RUNTIME_DLL
    PLAYFAB_GAMESAVE_RUNTIME_DLL_OUT PLAYFAB_GAMESAVE_RUNTIME_DLL
    PLAYFAB_SERVICES_RUNTIME_DLL_OUT PLAYFAB_SERVICES_RUNTIME_DLL
)

add_library(godot_playfab SHARED
    src/register_types.cpp
    src/playfab.cpp
    src/playfab_gamesaves.cpp
    src/playfab_runtime.cpp
    src/playfab_result.cpp
    src/playfab_result_codes_internal.cpp
    src/playfab_pending_signal.cpp
    src/playfab_generated_api_helpers.cpp
    src/playfab_signal_xasync_context.cpp
    src/playfab_user.cpp
    src/playfab_users.cpp
    src/playfab_leaderboards.cpp
    src/generated/playfab_generated_models.cpp
    src/generated/playfab_generated_services.cpp
)

target_include_directories(godot_playfab PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}/src"
    "${GDK_WINDOWS}/include"
    "${GDK_WINDOWS}/include/httpClient"
    "${GDK_WINDOWS}/include/playfab/core"
    "${GDK_WINDOWS}/include/playfab/services"
)

target_link_directories(godot_playfab PRIVATE
    "${GDK_WINDOWS}/lib/x64/"
    "${CMAKE_CURRENT_SOURCE_DIR}/lib"
)

target_link_libraries(godot_playfab PRIVATE
    godot::cpp
    PlayFabCore
    PlayFabGameSave
    PlayFabServices
    xgameruntime
    libHttpClient
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

godot_addon_copy_runtime_files(
    TARGET godot_playfab
    ADDON_NAME godot_playfab
    PROJECT_DIRS ${GODOT_PLAYFAB_PROJECT_DIRS}
    FILES
        "${XSAPI_RUNTIME_DLL}"
        "${LIBHTTPCLIENT_RUNTIME_DLL}"
        "${PLAYFAB_CORE_RUNTIME_DLL}"
        "${PLAYFAB_GAMESAVE_RUNTIME_DLL}"
        "${PLAYFAB_SERVICES_RUNTIME_DLL}"
)

godot_addon_sync_files_to_sample(
    TARGET godot_playfab
    ADDON_NAME godot_playfab
    ADDON_ROOT "${CMAKE_CURRENT_SOURCE_DIR}"
    PROJECT_DIRS ${GODOT_PLAYFAB_PROJECT_DIRS}
    FILES
        godot_playfab.gdextension
)
