include_guard(GLOBAL)

include(CMakeParseArguments)

function(_godot_addon_get_paths addon_name addon_bin_out)
    if(NOT DEFINED GODOT_ADDONS_ROOT)
        message(FATAL_ERROR "The root superproject must define GODOT_ADDONS_ROOT before including GodotExtensionCommon.cmake.")
    endif()

    set(addon_bin "${GODOT_ADDONS_ROOT}/${addon_name}/bin")
    set(${addon_bin_out} "${addon_bin}" PARENT_SCOPE)
endfunction()

function(_godot_addon_get_project_dirs project_dirs_out)
    set(project_dirs ${ARGN})

    if(NOT project_dirs)
        if(DEFINED GODOT_PROJECT_DIRS)
            set(project_dirs ${GODOT_PROJECT_DIRS})
        elseif(DEFINED GODOT_SAMPLE_DIRS)
            set(project_dirs ${GODOT_SAMPLE_DIRS})
        else()
            message(FATAL_ERROR
                "The root superproject must define GODOT_PROJECT_DIRS or GODOT_SAMPLE_DIRS.")
        endif()
    endif()

    set(${project_dirs_out} ${project_dirs} PARENT_SCOPE)
endfunction()

function(godot_addon_configure_target)
    set(one_value_args TARGET ADDON_NAME)
    set(multi_value_args PROJECT_DIRS)
    cmake_parse_arguments(ARG "" "${one_value_args}" "${multi_value_args}" ${ARGN})

    if(NOT ARG_TARGET OR NOT ARG_ADDON_NAME)
        message(FATAL_ERROR "godot_addon_configure_target requires TARGET and ADDON_NAME.")
    endif()

    _godot_addon_get_paths("${ARG_ADDON_NAME}" addon_bin)
    _godot_addon_get_project_dirs(project_dirs ${ARG_PROJECT_DIRS})

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

    foreach(project_dir IN LISTS project_dirs)
        set(project_bin "${project_dir}/addons/${ARG_ADDON_NAME}/bin")

        add_custom_command(TARGET ${ARG_TARGET} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E make_directory "${project_bin}"
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
                "$<TARGET_FILE:${ARG_TARGET}>"
                "${project_bin}/$<TARGET_FILE_NAME:${ARG_TARGET}>"
            COMMENT "Copying ${ARG_ADDON_NAME} DLL to ${project_dir}"
        )

        add_custom_command(TARGET ${ARG_TARGET} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E make_directory "${project_bin}"
            COMMAND ${CMAKE_COMMAND} -E
                $<IF:$<CONFIG:Debug>,copy_if_different,true>
                $<IF:$<CONFIG:Debug>,$<TARGET_FILE_DIR:${ARG_TARGET}>/$<TARGET_FILE_BASE_NAME:${ARG_TARGET}>.pdb,>
                $<IF:$<CONFIG:Debug>,${project_bin}/,>
            COMMENT "Copying ${ARG_ADDON_NAME} PDB to ${project_dir} (Debug only)"
        )
    endforeach()
endfunction()

function(godot_addon_copy_runtime_files)
    set(one_value_args TARGET ADDON_NAME)
    set(multi_value_args FILES PROJECT_DIRS)
    cmake_parse_arguments(ARG "" "${one_value_args}" "${multi_value_args}" ${ARGN})

    if(NOT ARG_TARGET OR NOT ARG_ADDON_NAME)
        message(FATAL_ERROR "godot_addon_copy_runtime_files requires TARGET and ADDON_NAME.")
    endif()

    if(NOT ARG_FILES)
        return()
    endif()

    _godot_addon_get_paths("${ARG_ADDON_NAME}" addon_bin)
    _godot_addon_get_project_dirs(project_dirs ${ARG_PROJECT_DIRS})

    set(copy_commands
        COMMAND ${CMAKE_COMMAND} -E make_directory "${addon_bin}"
    )

    foreach(runtime_file IN LISTS ARG_FILES)
        list(APPEND copy_commands
            COMMAND ${CMAKE_COMMAND} -E copy_if_different "${runtime_file}" "${addon_bin}/"
        )
    endforeach()

    foreach(project_dir IN LISTS project_dirs)
        set(project_bin "${project_dir}/addons/${ARG_ADDON_NAME}/bin")
        list(APPEND copy_commands
            COMMAND ${CMAKE_COMMAND} -E make_directory "${project_bin}"
        )
        foreach(runtime_file IN LISTS ARG_FILES)
            list(APPEND copy_commands
                COMMAND ${CMAKE_COMMAND} -E copy_if_different "${runtime_file}" "${project_bin}/"
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
    set(multi_value_args FILES PROJECT_DIRS)
    cmake_parse_arguments(ARG "" "${one_value_args}" "${multi_value_args}" ${ARGN})

    if(NOT ARG_TARGET OR NOT ARG_ADDON_NAME OR NOT ARG_ADDON_ROOT)
        message(FATAL_ERROR "godot_addon_sync_files_to_sample requires TARGET, ADDON_NAME, and ADDON_ROOT.")
    endif()

    if(NOT ARG_FILES)
        return()
    endif()

    _godot_addon_get_project_dirs(project_dirs ${ARG_PROJECT_DIRS})

    foreach(project_dir IN LISTS project_dirs)
        set(project_addon_dir "${project_dir}/addons/${ARG_ADDON_NAME}")

        set(sync_commands
            COMMAND ${CMAKE_COMMAND} -E make_directory "${project_addon_dir}"
        )

        foreach(sync_file IN LISTS ARG_FILES)
            get_filename_component(sync_dir "${sync_file}" DIRECTORY)
            if(sync_dir)
                list(APPEND sync_commands
                    COMMAND ${CMAKE_COMMAND} -E make_directory "${project_addon_dir}/${sync_dir}"
                )
            endif()

            list(APPEND sync_commands
                COMMAND ${CMAKE_COMMAND} -E copy_if_different
                    "${ARG_ADDON_ROOT}/${sync_file}"
                    "${project_addon_dir}/${sync_file}"
            )
        endforeach()

        add_custom_command(TARGET ${ARG_TARGET} POST_BUILD
            ${sync_commands}
            COMMENT "Syncing ${ARG_ADDON_NAME} addon files to ${project_dir}"
        )
    endforeach()
endfunction()

#[[ godot_addon_mirror_test_support

Mirrors a single test-support tree into N coverage hosts at build time.
Source of truth is the SOURCE_DIR; mirrored copies under each host's
`addons/<DEST_SUBDIR>/` are git-ignored and must not be edited — any
change to them is wiped on the next build. Edit the canonical tree under
`addons/<addon>/tests_support/<DEST_SUBDIR>/` instead.

Implementation: configure-time `file(GLOB_RECURSE ... CONFIGURE_DEPENDS)`
collects the source manifest. Per coverage host, an `add_custom_command`
nukes the destination and runs `cmake -E copy_directory` whenever any
source file is newer than the per-host stamp. A single
`add_custom_target(godot_mirror_test_support_<DEST_SUBDIR> ALL ...)`
fans out to all stamps so `cmake --build build --preset debug` always
refreshes mirrored copies. CONFIGURE_DEPENDS makes CMake re-glob (and
re-run configure if the source manifest changes) on the next build.

Usage:
    godot_addon_mirror_test_support(
        SOURCE_DIR  <abs path to vendored dir>
        DEST_SUBDIR <addon subdir name under <host>/addons/>
        HOST_DIRS   <one or more Godot project roots>
    )
]]
function(godot_addon_mirror_test_support)
    set(one_value_args SOURCE_DIR DEST_SUBDIR)
    set(multi_value_args HOST_DIRS SAMPLE_DIRS)
    cmake_parse_arguments(ARG "" "${one_value_args}" "${multi_value_args}" ${ARGN})

    set(host_dirs ${ARG_HOST_DIRS})
    if(ARG_SAMPLE_DIRS)
        list(APPEND host_dirs ${ARG_SAMPLE_DIRS})
    endif()

    if(NOT ARG_SOURCE_DIR OR NOT ARG_DEST_SUBDIR OR NOT host_dirs)
        message(FATAL_ERROR
            "godot_addon_mirror_test_support requires SOURCE_DIR, DEST_SUBDIR and HOST_DIRS.")
    endif()

    if(NOT IS_DIRECTORY "${ARG_SOURCE_DIR}")
        message(FATAL_ERROR
            "godot_addon_mirror_test_support: SOURCE_DIR does not exist: ${ARG_SOURCE_DIR}")
    endif()

    file(GLOB_RECURSE _mirror_src_files LIST_DIRECTORIES false CONFIGURE_DEPENDS
        "${ARG_SOURCE_DIR}/*")

    set(_stamp_dir "${CMAKE_BINARY_DIR}/_mirror_stamps")
    file(MAKE_DIRECTORY "${_stamp_dir}")

    set(_mirror_stamps)
    foreach(host_dir IN LISTS host_dirs)
        set(dest "${host_dir}/addons/${ARG_DEST_SUBDIR}")
        get_filename_component(_host_name "${host_dir}" NAME)
        set(_stamp "${_stamp_dir}/${ARG_DEST_SUBDIR}__${_host_name}.stamp")

        add_custom_command(
            OUTPUT "${_stamp}"
            COMMAND ${CMAKE_COMMAND} -E rm -rf "${dest}"
            COMMAND ${CMAKE_COMMAND} -E copy_directory "${ARG_SOURCE_DIR}" "${dest}"
            COMMAND ${CMAKE_COMMAND} -E touch "${_stamp}"
            DEPENDS ${_mirror_src_files}
            COMMENT "Mirroring test-support '${ARG_DEST_SUBDIR}' -> ${dest}"
            VERBATIM
        )

        list(APPEND _mirror_stamps "${_stamp}")
    endforeach()

    set(_target_name "godot_mirror_test_support_${ARG_DEST_SUBDIR}")
    add_custom_target(${_target_name} ALL DEPENDS ${_mirror_stamps})
endfunction()

function(godot_addon_sync_directory)
    set(one_value_args TARGET_NAME SOURCE_DIR)
    set(multi_value_args MIRROR_DIRS)
    cmake_parse_arguments(ARG "" "${one_value_args}" "${multi_value_args}" ${ARGN})

    if(NOT ARG_TARGET_NAME OR NOT ARG_SOURCE_DIR)
        message(FATAL_ERROR "godot_addon_sync_directory requires TARGET_NAME and SOURCE_DIR.")
    endif()

    if(NOT ARG_MIRROR_DIRS)
        return()
    endif()

    file(GLOB_RECURSE sync_files LIST_DIRECTORIES false RELATIVE "${ARG_SOURCE_DIR}" CONFIGURE_DEPENDS
        "${ARG_SOURCE_DIR}/*")

    if(NOT sync_files)
        return()
    endif()

    set(sync_commands)
    foreach(mirror_dir IN LISTS ARG_MIRROR_DIRS)
        list(APPEND sync_commands
            COMMAND ${CMAKE_COMMAND} -E make_directory "${mirror_dir}"
        )

        foreach(sync_file IN LISTS sync_files)
            get_filename_component(sync_subdir "${sync_file}" DIRECTORY)
            if(sync_subdir)
                list(APPEND sync_commands
                    COMMAND ${CMAKE_COMMAND} -E make_directory "${mirror_dir}/${sync_subdir}"
                )
            endif()

            list(APPEND sync_commands
                COMMAND ${CMAKE_COMMAND} -E copy_if_different
                    "${ARG_SOURCE_DIR}/${sync_file}"
                    "${mirror_dir}/${sync_file}"
            )
        endforeach()
    endforeach()

    add_custom_target(${ARG_TARGET_NAME} ALL
        ${sync_commands}
        COMMENT "Syncing addon mirrors from ${ARG_SOURCE_DIR}"
    )
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


#[[ godot_addon_doctest_target

Defines a doctest-based C++ unit-test executable for an addon. The executable
links against optional helper libraries supplied by the caller (typically a
static helper target that exposes only pure, Godot-free helpers) but does NOT
link against any addon's GDExtension shared library. Pure helpers only — do
not test Godot Object subclasses or signal-bearing types here. See
`spec/testing-strategy.md` ("C++ test scope rules") for the durable contract.

Arguments:
    TARGET_NAME    The CMake target name to create (e.g. gdk_unit_tests).
    IMPL_TU        Path to the single TU that defines DOCTEST_CONFIG_IMPLEMENT
                   (kept separate from SOURCES so multiple test TUs can share
                   one main).
    SOURCES        List of test source files (each contains TEST_CASE(...)).
    LINK_LIBS      Optional libraries to link (e.g. addon static helpers, or
                   `godot::cpp` if a helper happens to use Variant/String).
                   Most pure-helper test exes need nothing extra here.
    INCLUDES       Extra include directories. The vendored doctest header
                   directory is always added automatically.

This function defines the target unconditionally. Callers are responsible
for gating the call site on `GDK_BUILD_TESTS` (see `tests/cpp/CMakeLists.txt`
for the canonical wiring).

The vendored doctest header is required at
`${CMAKE_SOURCE_DIR}/tests/cpp/third_party/doctest/doctest.h` — its absence
is a fatal CMake error.

The output binary lands at `${CMAKE_BINARY_DIR}/bin/<config>/<TARGET_NAME>.exe`
(matching the spike's well-known path) so that contributors can run it via
`& .\build\bin\Debug\<TARGET_NAME>.exe` after a debug build.
]]
function(godot_addon_doctest_target)
    set(one_value_args TARGET_NAME IMPL_TU)
    set(multi_value_args SOURCES LINK_LIBS INCLUDES)
    cmake_parse_arguments(ARG "" "${one_value_args}" "${multi_value_args}" ${ARGN})

    if(NOT ARG_TARGET_NAME OR NOT ARG_IMPL_TU OR NOT ARG_SOURCES)
        message(FATAL_ERROR
            "godot_addon_doctest_target requires TARGET_NAME, IMPL_TU, and SOURCES.")
    endif()

    set(_doctest_dir "${CMAKE_SOURCE_DIR}/tests/cpp/third_party/doctest")
    set(_doctest_header "${_doctest_dir}/doctest.h")
    if(NOT EXISTS "${_doctest_header}")
        message(FATAL_ERROR
            "godot_addon_doctest_target: doctest header not found at ${_doctest_header}. "
            "The vendored doctest single-header (pinned in tests/cpp/third_party/doctest/VERSION.txt) is required.")
    endif()

    add_executable(${ARG_TARGET_NAME}
        "${ARG_IMPL_TU}"
        ${ARG_SOURCES}
    )

    target_include_directories(${ARG_TARGET_NAME} PRIVATE
        "${_doctest_dir}"
        ${ARG_INCLUDES}
    )

    if(ARG_LINK_LIBS)
        target_link_libraries(${ARG_TARGET_NAME} PRIVATE ${ARG_LINK_LIBS})
    endif()

    set_target_properties(${ARG_TARGET_NAME} PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY         "${CMAKE_BINARY_DIR}/bin"
        RUNTIME_OUTPUT_DIRECTORY_DEBUG   "${CMAKE_BINARY_DIR}/bin/Debug"
        RUNTIME_OUTPUT_DIRECTORY_RELEASE "${CMAKE_BINARY_DIR}/bin/Release"
        FOLDER                           "tests"
    )
endfunction()

