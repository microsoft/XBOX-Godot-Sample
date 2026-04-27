include_guard(GLOBAL)

include(CMakeParseArguments)

function(_godot_addon_get_paths addon_name addon_bin_out sample_addon_dir_out sample_bin_out)
    if(NOT DEFINED GODOT_ADDONS_ROOT OR NOT DEFINED GODOT_SAMPLE_ADDONS_ROOT)
        message(FATAL_ERROR "The root superproject must define GODOT_ADDONS_ROOT and GODOT_SAMPLE_ADDONS_ROOT before including GodotExtensionCommon.cmake.")
    endif()

    set(addon_bin "${GODOT_ADDONS_ROOT}/${addon_name}/bin")
    set(sample_addon_dir "${GODOT_SAMPLE_ADDONS_ROOT}/${addon_name}")
    set(sample_bin "${sample_addon_dir}/bin")

    set(${addon_bin_out} "${addon_bin}" PARENT_SCOPE)
    set(${sample_addon_dir_out} "${sample_addon_dir}" PARENT_SCOPE)
    set(${sample_bin_out} "${sample_bin}" PARENT_SCOPE)
endfunction()

function(godot_addon_configure_target)
    set(one_value_args TARGET ADDON_NAME)
    cmake_parse_arguments(ARG "" "${one_value_args}" "" ${ARGN})

    if(NOT ARG_TARGET OR NOT ARG_ADDON_NAME)
        message(FATAL_ERROR "godot_addon_configure_target requires TARGET and ADDON_NAME.")
    endif()

    _godot_addon_get_paths("${ARG_ADDON_NAME}" addon_bin sample_addon_dir sample_bin)

    set_target_properties(${ARG_TARGET} PROPERTIES
        OUTPUT_NAME                      "${ARG_ADDON_NAME}.windows.release.x86_64"
        OUTPUT_NAME_DEBUG                "${ARG_ADDON_NAME}.windows.debug.x86_64"
        OUTPUT_NAME_RELWITHDEBINFO       "${ARG_ADDON_NAME}.windows.release.x86_64"
        OUTPUT_NAME_MINSIZEREL           "${ARG_ADDON_NAME}.windows.release.x86_64"
        RUNTIME_OUTPUT_DIRECTORY         "${addon_bin}"
        RUNTIME_OUTPUT_DIRECTORY_DEBUG   "${addon_bin}"
        RUNTIME_OUTPUT_DIRECTORY_RELEASE "${addon_bin}"
        LIBRARY_OUTPUT_DIRECTORY         "${addon_bin}"
        LIBRARY_OUTPUT_DIRECTORY_DEBUG   "${addon_bin}"
        LIBRARY_OUTPUT_DIRECTORY_RELEASE "${addon_bin}"
        PREFIX ""
        SUFFIX ".dll"
    )

    add_custom_command(TARGET ${ARG_TARGET} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E make_directory "${sample_bin}"
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "$<TARGET_FILE:${ARG_TARGET}>"
            "${sample_bin}/$<TARGET_FILE_NAME:${ARG_TARGET}>"
        COMMENT "Copying ${ARG_ADDON_NAME} DLL to sample project"
    )

    add_custom_command(TARGET ${ARG_TARGET} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E make_directory "${sample_bin}"
        COMMAND ${CMAKE_COMMAND} -E
            $<IF:$<CONFIG:Debug>,copy_if_different,true>
            $<IF:$<CONFIG:Debug>,$<TARGET_FILE_DIR:${ARG_TARGET}>/$<TARGET_FILE_BASE_NAME:${ARG_TARGET}>.pdb,>
            $<IF:$<CONFIG:Debug>,${sample_bin}/,>
        COMMENT "Copying ${ARG_ADDON_NAME} PDB to sample project (Debug only)"
    )
endfunction()

function(godot_addon_copy_runtime_files)
    set(one_value_args TARGET ADDON_NAME)
    set(multi_value_args FILES)
    cmake_parse_arguments(ARG "" "${one_value_args}" "${multi_value_args}" ${ARGN})

    if(NOT ARG_TARGET OR NOT ARG_ADDON_NAME)
        message(FATAL_ERROR "godot_addon_copy_runtime_files requires TARGET and ADDON_NAME.")
    endif()

    if(NOT ARG_FILES)
        return()
    endif()

    _godot_addon_get_paths("${ARG_ADDON_NAME}" addon_bin sample_addon_dir sample_bin)

    set(copy_commands
        COMMAND ${CMAKE_COMMAND} -E make_directory "${addon_bin}"
        COMMAND ${CMAKE_COMMAND} -E make_directory "${sample_bin}"
    )

    foreach(runtime_file IN LISTS ARG_FILES)
        list(APPEND copy_commands
            COMMAND ${CMAKE_COMMAND} -E copy_if_different "${runtime_file}" "${addon_bin}/"
            COMMAND ${CMAKE_COMMAND} -E copy_if_different "${runtime_file}" "${sample_bin}/"
        )
    endforeach()

    add_custom_command(TARGET ${ARG_TARGET} POST_BUILD
        ${copy_commands}
        COMMENT "Copying ${ARG_ADDON_NAME} runtime files"
    )
endfunction()

function(godot_addon_sync_files_to_sample)
    set(one_value_args TARGET ADDON_NAME ADDON_ROOT)
    set(multi_value_args FILES)
    cmake_parse_arguments(ARG "" "${one_value_args}" "${multi_value_args}" ${ARGN})

    if(NOT ARG_TARGET OR NOT ARG_ADDON_NAME OR NOT ARG_ADDON_ROOT)
        message(FATAL_ERROR "godot_addon_sync_files_to_sample requires TARGET, ADDON_NAME, and ADDON_ROOT.")
    endif()

    if(NOT ARG_FILES)
        return()
    endif()

    _godot_addon_get_paths("${ARG_ADDON_NAME}" addon_bin sample_addon_dir sample_bin)

    set(sync_commands
        COMMAND ${CMAKE_COMMAND} -E make_directory "${sample_addon_dir}"
    )

    foreach(sync_file IN LISTS ARG_FILES)
        get_filename_component(sync_dir "${sync_file}" DIRECTORY)
        if(sync_dir)
            list(APPEND sync_commands
                COMMAND ${CMAKE_COMMAND} -E make_directory "${sample_addon_dir}/${sync_dir}"
            )
        endif()

        list(APPEND sync_commands
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
                "${ARG_ADDON_ROOT}/${sync_file}"
                "${sample_addon_dir}/${sync_file}"
        )
    endforeach()

    add_custom_command(TARGET ${ARG_TARGET} POST_BUILD
        ${sync_commands}
        COMMENT "Syncing ${ARG_ADDON_NAME} addon files to sample project"
    )
endfunction()
