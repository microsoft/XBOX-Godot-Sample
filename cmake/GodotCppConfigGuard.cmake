# GodotCppConfigGuard.cmake
#
# Guards against building a configuration that disagrees with the godot-cpp
# target this binary directory was configured for. See
# cmake/AssertGodotCppConfig.cmake for why a mismatch is dangerous and why one
# binary directory cannot serve both configurations.
#
# Provides:
#   godot_cpp_config_guard(<target>...)
#       Attaches a PRE_BUILD check to each target so a mismatched build fails
#       loudly before any object file is produced.
#
# Also performs the single-config-generator check at configure time, where
# CMAKE_BUILD_TYPE is known.

set(_GODOT_CPP_ASSERT_SCRIPT "${CMAKE_CURRENT_LIST_DIR}/AssertGodotCppConfig.cmake")

function(godot_cpp_expected_configs out_var)
    if(GODOTCPP_TARGET STREQUAL "template_release")
        set(${out_var} Release RelWithDebInfo MinSizeRel PARENT_SCOPE)
    else()
        set(${out_var} Debug PARENT_SCOPE)
    endif()
endfunction()

# Configure-time check. Multi-config generators do not know the configuration
# yet, so they are covered by the per-target PRE_BUILD guard below.
get_property(_godot_cpp_multi_config GLOBAL PROPERTY GENERATOR_IS_MULTI_CONFIG)
if(NOT _godot_cpp_multi_config AND CMAKE_BUILD_TYPE)
    godot_cpp_expected_configs(_expected)
    if(NOT CMAKE_BUILD_TYPE IN_LIST _expected)
        string(REPLACE ";" ", " _expected_display "${_expected}")
        message(FATAL_ERROR
            "godot-cpp configuration mismatch.\n"
            "  GODOTCPP_TARGET=${GODOTCPP_TARGET} supports: ${_expected_display}\n"
            "  CMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}\n"
            "  Configure a separate binary directory for this configuration "
            "(see the *-release presets in CMakePresets.json).")
    endif()
endif()

function(godot_cpp_config_guard)
    foreach(target IN LISTS ARGN)
        if(NOT TARGET ${target})
            message(FATAL_ERROR "godot_cpp_config_guard(): '${target}' is not a target.")
        endif()
        add_custom_command(TARGET ${target} PRE_BUILD
            COMMAND "${CMAKE_COMMAND}"
                "-DGODOTCPP_TARGET=${GODOTCPP_TARGET}"
                "-DBUILD_CONFIG=$<CONFIG>"
                "-DTARGET_NAME=${target}"
                -P "${_GODOT_CPP_ASSERT_SCRIPT}"
            COMMENT "Checking ${target} configuration against GODOTCPP_TARGET=${GODOTCPP_TARGET}"
            VERBATIM)
    endforeach()
endfunction()
