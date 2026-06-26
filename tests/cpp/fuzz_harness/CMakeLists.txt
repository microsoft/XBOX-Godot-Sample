# godot_stub_harness — static library that satisfies all 176 GDExtension interface
# function slots so that godot::String, Dictionary, Variant, and Array work in a
# standalone fuzz exe with no live Godot engine.
#
# Built automatically when GDK_BUILD_FUZZ_TARGETS=ON (the `fuzz` configure preset
# enables this). Fuzz targets that use Godot types link against this library.
#
# See godot_stub_init.h for the public API and usage pattern.

add_library(godot_stub_harness STATIC
    stub_memory.cpp
    stub_string.cpp
    stub_variant.cpp
    stub_dict_array.cpp
    stub_object.cpp
    stub_noop.cpp
    godot_stub_init.cpp
)

target_include_directories(godot_stub_harness PUBLIC
    "${CMAKE_CURRENT_SOURCE_DIR}"
)

target_link_libraries(godot_stub_harness PUBLIC godot::cpp)

# ASan+fuzzer flags must be consistent across the entire link unit.
# Note: clang-cl uses '-' prefix (not '/') for sanitizer flags.
target_compile_options(godot_stub_harness PRIVATE -fsanitize=fuzzer,address)

set_target_properties(godot_stub_harness PROPERTIES
    FOLDER "tests/fuzz"
)
