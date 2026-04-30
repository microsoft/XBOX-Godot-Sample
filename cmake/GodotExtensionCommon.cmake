include_guard(GLOBAL)

include(CMakeParseArguments)

function(_godot_addon_get_paths addon_name addon_bin_out)
    if(NOT DEFINED GODOT_ADDONS_ROOT)
        message(FATAL_ERROR "The root superproject must define GODOT_ADDONS_ROOT before including GodotExtensionCommon.cmake.")
    endif()

    set(addon_bin "${GODOT_ADDONS_ROOT}/${addon_name}/bin")
    set(${addon_bin_out} "${addon_bin}" PARENT_SCOPE)
endfunction()

function(godot_addon_configure_target)
    set(one_value_args TARGET ADDON_NAME)
    cmake_parse_arguments(ARG "" "${one_value_args}" "" ${ARGN})

    if(NOT ARG_TARGET OR NOT ARG_ADDON_NAME)
        message(FATAL_ERROR "godot_addon_configure_target requires TARGET and ADDON_NAME.")
    endif()

    _godot_addon_get_paths("${ARG_ADDON_NAME}" addon_bin)

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

    foreach(sample_dir IN LISTS GODOT_SAMPLE_DIRS)
        set(sample_bin "${sample_dir}/addons/${ARG_ADDON_NAME}/bin")

        add_custom_command(TARGET ${ARG_TARGET} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E make_directory "${sample_bin}"
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
                "$<TARGET_FILE:${ARG_TARGET}>"
                "${sample_bin}/$<TARGET_FILE_NAME:${ARG_TARGET}>"
            COMMENT "Copying ${ARG_ADDON_NAME} DLL to ${sample_dir}"
        )

        add_custom_command(TARGET ${ARG_TARGET} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E make_directory "${sample_bin}"
            COMMAND ${CMAKE_COMMAND} -E
                $<IF:$<CONFIG:Debug>,copy_if_different,true>
                $<IF:$<CONFIG:Debug>,$<TARGET_FILE_DIR:${ARG_TARGET}>/$<TARGET_FILE_BASE_NAME:${ARG_TARGET}>.pdb,>
                $<IF:$<CONFIG:Debug>,${sample_bin}/,>
            COMMENT "Copying ${ARG_ADDON_NAME} PDB to ${sample_dir} (Debug only)"
        )
    endforeach()
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

    _godot_addon_get_paths("${ARG_ADDON_NAME}" addon_bin)

    set(copy_commands
        COMMAND ${CMAKE_COMMAND} -E make_directory "${addon_bin}"
    )

    foreach(runtime_file IN LISTS ARG_FILES)
        list(APPEND copy_commands
            COMMAND ${CMAKE_COMMAND} -E copy_if_different "${runtime_file}" "${addon_bin}/"
        )
    endforeach()

    foreach(sample_dir IN LISTS GODOT_SAMPLE_DIRS)
        set(sample_bin "${sample_dir}/addons/${ARG_ADDON_NAME}/bin")
        list(APPEND copy_commands
            COMMAND ${CMAKE_COMMAND} -E make_directory "${sample_bin}"
        )
        foreach(runtime_file IN LISTS ARG_FILES)
            list(APPEND copy_commands
                COMMAND ${CMAKE_COMMAND} -E copy_if_different "${runtime_file}" "${sample_bin}/"
            )
        endforeach()
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

    foreach(sample_dir IN LISTS GODOT_SAMPLE_DIRS)
        set(sample_addon_dir "${sample_dir}/addons/${ARG_ADDON_NAME}")

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
            COMMENT "Syncing ${ARG_ADDON_NAME} addon files to ${sample_dir}"
        )
    endforeach()
endfunction()


#[[ godot_addon_doc_sources

Wrapper around godot-cpp's `target_doc_sources` that uses an addon-unique
custom-target name. Required because `target_doc_sources` (in
godot-cpp/cmake/GodotCPPModule.cmake) hardcodes `add_custom_target(generate_doc_source ...)`,
so calling it from more than one addon in the same superproject collides
with CMake's "logical target names must be globally unique" rule.

Usage:
    godot_addon_doc_sources(
        TARGET     <library_target>
        ADDON_NAME <addon_name>
        SOURCES    <list of .xml paths>
    )
]]
function(godot_addon_doc_sources)
    set(one_value_args TARGET ADDON_NAME)
    set(multi_value_args SOURCES)
    cmake_parse_arguments(ARG "" "${one_value_args}" "${multi_value_args}" ${ARGN})

    if(NOT ARG_TARGET OR NOT ARG_ADDON_NAME)
        message(FATAL_ERROR "godot_addon_doc_sources requires TARGET and ADDON_NAME.")
    endif()

    if(NOT ARG_SOURCES)
        return()
    endif()

    # Python3 is normally found by godot-cpp, but its directory scope doesn't
    # propagate variables up to addon scopes. Re-find here so Python3_EXECUTABLE
    # is reliably defined regardless of which subdirectory called us.
    find_package(Python3 3.4 REQUIRED COMPONENTS Interpreter)

    if(NOT DEFINED godot-cpp_SOURCE_DIR)
        set(godot-cpp_SOURCE_DIR "${CMAKE_SOURCE_DIR}/godot-cpp")
    endif()

    set(doc_target "${ARG_ADDON_NAME}_generate_doc_source")
    set(doc_source_file "${CMAKE_CURRENT_BINARY_DIR}/gen/doc_source.cpp")

    get_filename_component(doc_output_dir "${doc_source_file}" DIRECTORY)
    file(MAKE_DIRECTORY "${doc_output_dir}")

    set(_dispatcher "${CMAKE_SOURCE_DIR}/cmake/run_doc_source_generator.py")

    add_custom_command(
        OUTPUT "${doc_source_file}"
        COMMAND "${Python3_EXECUTABLE}"
                "${_dispatcher}"
                "${godot-cpp_SOURCE_DIR}"
                "${doc_source_file}"
                ${ARG_SOURCES}
        VERBATIM
        DEPENDS
            "${_dispatcher}"
            "${godot-cpp_SOURCE_DIR}/doc_source_generator.py"
            ${ARG_SOURCES}
        COMMENT "Generating doc source for ${ARG_ADDON_NAME}"
    )

    add_custom_target(${doc_target} DEPENDS "${doc_source_file}")
    set_target_properties(${doc_target} PROPERTIES FOLDER "godot-cpp")

    target_sources(${ARG_TARGET} PRIVATE "${doc_source_file}")
    add_dependencies(${ARG_TARGET} ${doc_target})
endfunction()
