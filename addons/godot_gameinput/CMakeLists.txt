include(GodotExtensionCommon)
include(GDKDependencies)

# We only need the GDK include directory (for <GameInput.h>) — godot_gameinput
# does NOT depend on the godot_gdk addon binary, and it does NOT need the XSAPI
# / libHttpClient runtime DLLs that godot_gdk pulls in.
gdk_detect_dependencies(GDK_WINDOWS_OUT GDK_WINDOWS)

add_library(godot_gameinput SHARED
    src/register_types.cpp
    src/gameinput_singleton.cpp
    src/gameinput_device.cpp
    src/gameinput_reading.cpp
    src/gameinput_binding.cpp
    src/gameinput_action_map.cpp
    src/gameinput_mapper.cpp
)

target_include_directories(godot_gameinput PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}/src"
    "${GDK_WINDOWS}/include"
)

target_link_directories(godot_gameinput PRIVATE
    "${GDK_WINDOWS}/lib/x64"
)

target_link_libraries(godot_gameinput PRIVATE
    godot::cpp
    xgameruntime
    GameInput
)

target_compile_definitions(godot_gameinput PRIVATE _GAMING_DESKTOP)

godot_addon_configure_target(TARGET godot_gameinput ADDON_NAME godot_gameinput)

if(GODOTCPP_TARGET MATCHES "editor|template_debug")
    file(GLOB_RECURSE DOC_XML LIST_DIRECTORIES NO CONFIGURE_DEPENDS
         "${CMAKE_CURRENT_SOURCE_DIR}/doc_classes/*.xml")
    godot_addon_doc_sources(
        TARGET     godot_gameinput
        ADDON_NAME godot_gameinput
        SOURCES    ${DOC_XML}
    )
endif()

godot_addon_sync_files_to_sample(
    TARGET godot_gameinput
    ADDON_NAME godot_gameinput
    ADDON_ROOT "${CMAKE_CURRENT_SOURCE_DIR}"
    FILES
        godot_gameinput.gdextension
        plugin.cfg
        editor/gameinput_editor_plugin.gd
        runtime/gameinput_bootstrap.gd
)
