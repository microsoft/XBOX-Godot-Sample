include(GodotExtensionCommon)
include(GDKDependencies)

# godot_gameinput only consumes GameInput.h + gameinput.lib — no XGameRuntime
# API usage anywhere in src/. Therefore it links Microsoft::GameInput (from
# the vcpkg `gameinput` port) and does not depend on the ms-gdk port at all.
# gdk_require_gameinput() restores only the `gameinput` port, so the
# `gameinput-only` preset does not pay the cost of restoring `ms-gdk`.
gdk_require_gameinput()

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
)

target_link_libraries(godot_gameinput PRIVATE
    godot::cpp
    Microsoft::GameInput
)

target_compile_definitions(godot_gameinput PRIVATE _GAMING_DESKTOP)

godot_addon_configure_target(
    TARGET godot_gameinput
    ADDON_NAME godot_gameinput
    PROJECT_DIRS ${GODOT_GAMEINPUT_PROJECT_DIRS}
)

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
    PROJECT_DIRS ${GODOT_GAMEINPUT_PROJECT_DIRS}
    FILES
        godot_gameinput.gdextension
        plugin.cfg
        editor/gameinput_editor_plugin.gd
        runtime/gameinput_bootstrap.gd
)
