include(GodotExtensionCommon)
include(GDKDependencies)

gdk_detect_dependencies(
    GDK_WINDOWS_OUT GDK_WINDOWS
    LIBHTTPCLIENT_RUNTIME_DLL_OUT LIBHTTPCLIENT_RUNTIME_DLL
    PLAYFAB_CORE_RUNTIME_DLL_OUT PLAYFAB_CORE_RUNTIME_DLL
    PLAYFAB_SERVICES_RUNTIME_DLL_OUT PLAYFAB_SERVICES_RUNTIME_DLL
    PLAYFAB_MULTIPLAYER_RUNTIME_DLL_OUT PLAYFAB_MULTIPLAYER_RUNTIME_DLL
    PLAYFAB_PARTY_RUNTIME_DLL_OUT PLAYFAB_PARTY_RUNTIME_DLL
)

add_library(godot_playfab SHARED
    src/GenericPlayer.cpp
    src/pch.cpp
    src/PFUtils.cpp
    src/PlayFabManager.cpp
    src/register_types.cpp
)

target_include_directories(godot_playfab PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}/src"
    "${CMAKE_CURRENT_SOURCE_DIR}/WinGDK"
    "${GDK_WINDOWS}/include"
    "${GDK_WINDOWS}/include/httpClient"
    "${GDK_WINDOWS}/include/playfab/core"
    "${GDK_WINDOWS}/include/playfab/services"
    "${GDK_WINDOWS}/include/playfab/multiplayer"
    "${GDK_WINDOWS}/include/playfab/party"
)

target_link_directories(godot_playfab PRIVATE
    "${GDK_WINDOWS}/lib/x64/"
    "${CMAKE_CURRENT_SOURCE_DIR}/lib"
)

target_link_libraries(godot_playfab PRIVATE
    godot::cpp
    Party
    PlayFabMultiplayer
    PlayFabCore
    PlayFabServices
    xgameruntime
    libHttpClient
)

target_compile_definitions(godot_playfab PRIVATE _GAMING_DESKTOP)

godot_addon_configure_target(TARGET godot_playfab ADDON_NAME godot_playfab)

godot_addon_copy_runtime_files(
    TARGET godot_playfab
    ADDON_NAME godot_playfab
    FILES
        "${XSAPI_RUNTIME_DLL}"
        "${LIBHTTPCLIENT_RUNTIME_DLL}"
        "${PLAYFAB_CORE_RUNTIME_DLL}"
        "${PLAYFAB_SERVICES_RUNTIME_DLL}"
        "${PLAYFAB_MULTIPLAYER_RUNTIME_DLL}"
        "${PLAYFAB_PARTY_RUNTIME_DLL}"
)

godot_addon_sync_files_to_sample(
    TARGET godot_playfab
    ADDON_NAME godot_playfab
    ADDON_ROOT "${CMAKE_CURRENT_SOURCE_DIR}"
    FILES
        godot_playfab.gdextension
)
