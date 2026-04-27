include(GodotExtensionCommon)

add_library(godot_gameinput SHARED
    src/register_types.cpp
    src/gameinput_extension.cpp
)

target_include_directories(godot_gameinput PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}/src"
)

target_link_libraries(godot_gameinput PRIVATE
    godot::cpp
)

godot_addon_configure_target(TARGET godot_gameinput ADDON_NAME godot_gameinput)

godot_addon_sync_files_to_sample(
    TARGET godot_gameinput
    ADDON_NAME godot_gameinput
    ADDON_ROOT "${CMAKE_CURRENT_SOURCE_DIR}"
    FILES
        godot_gameinput.gdextension
)
