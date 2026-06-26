// godot_stub_init.cpp — GDExtension stub harness dispatch table and init.
//
// Provides stub_get_proc_address() which maps all 176 GDExtension interface
// function names to the stub implementations defined in the sibling .cpp files.
// Functions not needed by our fuzz targets are mapped to stub_noop_ptr.
//
// Usage: call godot_stub::init() once from LLVMFuzzerInitialize before using
// any Godot type.  See godot_stub_init.h for the full API contract.

#include "godot_stub_init.h"
#include "stub_types.h"

#include <cstring>
#include <string_view>
#include <unordered_map>

// ─── Extern declarations for all function pointer tables ──────────────────

// stub_memory.cpp
extern void *stub_mem_alloc_ptr;
extern void *stub_mem_alloc2_ptr;
extern void *stub_mem_realloc_ptr;
extern void *stub_mem_realloc2_ptr;
extern void *stub_mem_free_ptr;
extern void *stub_mem_free2_ptr;
extern void *stub_print_error_ptr;
extern void *stub_print_error_with_message_ptr;
extern void *stub_print_warning_ptr;
extern void *stub_print_warning_with_message_ptr;

// stub_string.cpp
extern void *stub_string_new_with_latin1_chars_ptr;
extern void *stub_string_new_with_latin1_chars_and_len_ptr;
extern void *stub_string_new_with_utf8_chars_ptr;
extern void *stub_string_new_with_utf8_chars_and_len_ptr;
extern void *stub_string_new_with_utf8_chars_and_len2_ptr;
extern void *stub_string_new_with_utf32_chars_ptr;
extern void *stub_string_new_with_utf32_chars_and_len_ptr;
extern void *stub_string_new_with_wide_chars_ptr;
extern void *stub_string_new_with_wide_chars_and_len_ptr;
extern void *stub_string_new_with_utf16_chars_ptr;
extern void *stub_string_new_with_utf16_chars_and_len_ptr;
extern void *stub_string_new_with_utf16_chars_and_len2_ptr;
extern void *stub_string_operator_plus_eq_string_ptr;
extern void *stub_string_operator_plus_eq_cstr_ptr;
extern void *stub_string_operator_plus_eq_wcstr_ptr;
extern void *stub_string_operator_plus_eq_c32str_ptr;
extern void *stub_string_operator_plus_eq_char_ptr;
extern void *stub_string_operator_index_ptr;
extern void *stub_string_operator_index_const_ptr;
extern void *stub_string_to_latin1_chars_ptr;
extern void *stub_string_to_utf8_chars_ptr;
extern void *stub_string_to_utf16_chars_ptr;
extern void *stub_string_to_utf32_chars_ptr;
extern void *stub_string_to_wide_chars_ptr;
extern void *stub_string_resize_ptr;
extern void *stub_string_name_new_with_latin1_chars_ptr;
extern void *stub_string_name_new_with_utf8_chars_ptr;
extern void *stub_string_name_new_with_utf8_chars_and_len_ptr;

// stub_variant.cpp
extern void *stub_variant_new_nil_ptr;
extern void *stub_variant_new_copy_ptr;
extern void *stub_variant_destroy_ptr;
extern void *stub_variant_get_type_ptr;
extern void *stub_variant_get_type_name_ptr;
extern void *stub_variant_booleanize_ptr;
extern void *stub_variant_hash_ptr;
extern void *stub_variant_hash_compare_ptr;
extern void *stub_variant_stringify_ptr;
extern void *stub_variant_call_ptr;
extern void *stub_variant_call_static_ptr;
extern void *stub_variant_construct_ptr;
extern void *stub_variant_duplicate_ptr;
extern void *stub_variant_get_ptr;
extern void *stub_variant_set_ptr;
extern void *stub_variant_get_named_ptr;
extern void *stub_variant_set_named_ptr;
extern void *stub_variant_get_indexed_ptr;
extern void *stub_variant_set_indexed_ptr;
extern void *stub_variant_get_keyed_ptr;
extern void *stub_variant_set_keyed_ptr;
extern void *stub_variant_has_key_ptr;
extern void *stub_variant_can_convert_ptr;
extern void *stub_variant_can_convert_strict_ptr;
extern void *stub_variant_evaluate_ptr;
extern void *stub_variant_recursive_hash_ptr;
extern void *stub_variant_has_member_ptr;
extern void *stub_variant_has_method_ptr;
extern void *stub_variant_get_object_instance_id_ptr;
extern void *stub_variant_get_constant_value_ptr;
extern void *stub_variant_iter_init_ptr;
extern void *stub_variant_iter_next_ptr;
extern void *stub_variant_iter_get_ptr;
extern void *stub_variant_get_ptr_setter_ptr;
extern void *stub_variant_get_ptr_getter_ptr;
extern void *stub_variant_get_ptr_indexed_setter_ptr;
extern void *stub_variant_get_ptr_indexed_getter_ptr;
extern void *stub_variant_get_ptr_keyed_setter_ptr;
extern void *stub_variant_get_ptr_keyed_getter_ptr;
extern void *stub_variant_get_ptr_keyed_checker_ptr;
extern void *stub_variant_get_ptr_operator_evaluator_ptr;
extern void *stub_variant_get_ptr_utility_function_ptr;
extern void *stub_variant_get_ptr_internal_getter_ptr;
extern void *stub_variant_get_ptr_constructor_ptr;
extern void *stub_variant_get_ptr_destructor_ptr;
extern void *stub_variant_get_ptr_builtin_method_ptr;
extern void *stub_get_variant_from_type_constructor_ptr;
extern void *stub_get_variant_to_type_constructor_ptr;

// stub_dict_array.cpp
extern void *stub_dictionary_operator_index_ptr;
extern void *stub_dictionary_operator_index_const_ptr;
extern void *stub_dictionary_set_typed_ptr;
extern void *stub_array_operator_index_ptr;
extern void *stub_array_operator_index_const_ptr;
extern void *stub_array_ref_ptr;
extern void *stub_array_set_typed_ptr;

// stub_packed_array.cpp
extern void *stub_packed_byte_array_operator_index_ptr;
extern void *stub_packed_byte_array_operator_index_const_ptr;

// stub_object.cpp
extern void *stub_classdb_construct_object_ptr;
extern void *stub_classdb_construct_object2_ptr;
extern void *stub_object_set_instance_ptr;
extern void *stub_object_set_instance_binding_ptr;
extern void *stub_object_get_instance_binding_ptr;
extern void *stub_object_destroy_ptr;
extern void *stub_classdb_get_method_bind_ptr;
extern void *stub_object_method_bind_ptrcall_ptr;

// stub_noop.cpp
extern void *stub_noop_ptr;
extern void *stub_get_godot_version_ptr;
extern void *stub_get_godot_version2_ptr;

// ─── Dispatch table ───────────────────────────────────────────────────────

static const std::unordered_map<std::string_view, void **> &dispatch_table() {
    // Each entry maps the function name (string) → pointer to the function-pointer variable.
    // Indirection through void** allows the table to be static-initialized without
    // depending on global constructor ordering.
    static const std::unordered_map<std::string_view, void **> table = {
        // ── Memory ──────────────────────────────────────────────────────
        { "mem_alloc",                            &stub_mem_alloc_ptr },
        { "mem_alloc2",                           &stub_mem_alloc2_ptr },
        { "mem_realloc",                          &stub_mem_realloc_ptr },
        { "mem_realloc2",                         &stub_mem_realloc2_ptr },
        { "mem_free",                             &stub_mem_free_ptr },
        { "mem_free2",                            &stub_mem_free2_ptr },
        { "print_error",                          &stub_print_error_ptr },
        { "print_error_with_message",             &stub_print_error_with_message_ptr },
        { "print_warning",                        &stub_print_warning_ptr },
        { "print_warning_with_message",           &stub_print_warning_with_message_ptr },

        // ── Version ──────────────────────────────────────────────────────
        { "get_godot_version",                    &stub_get_godot_version_ptr },
        { "get_godot_version2",                   &stub_get_godot_version2_ptr },

        // ── String ───────────────────────────────────────────────────────
        { "string_new_with_latin1_chars",            &stub_string_new_with_latin1_chars_ptr },
        { "string_new_with_latin1_chars_and_len",    &stub_string_new_with_latin1_chars_and_len_ptr },
        { "string_new_with_utf8_chars",              &stub_string_new_with_utf8_chars_ptr },
        { "string_new_with_utf8_chars_and_len",      &stub_string_new_with_utf8_chars_and_len_ptr },
        { "string_new_with_utf8_chars_and_len2",     &stub_string_new_with_utf8_chars_and_len2_ptr },
        { "string_new_with_utf32_chars",             &stub_string_new_with_utf32_chars_ptr },
        { "string_new_with_utf32_chars_and_len",     &stub_string_new_with_utf32_chars_and_len_ptr },
        { "string_new_with_wide_chars",              &stub_string_new_with_wide_chars_ptr },
        { "string_new_with_wide_chars_and_len",      &stub_string_new_with_wide_chars_and_len_ptr },
        { "string_new_with_utf16_chars",             &stub_string_new_with_utf16_chars_ptr },
        { "string_new_with_utf16_chars_and_len",     &stub_string_new_with_utf16_chars_and_len_ptr },
        { "string_new_with_utf16_chars_and_len2",    &stub_string_new_with_utf16_chars_and_len2_ptr },
        { "string_operator_plus_eq_string",          &stub_string_operator_plus_eq_string_ptr },
        { "string_operator_plus_eq_cstr",            &stub_string_operator_plus_eq_cstr_ptr },
        { "string_operator_plus_eq_wcstr",           &stub_string_operator_plus_eq_wcstr_ptr },
        { "string_operator_plus_eq_c32str",          &stub_string_operator_plus_eq_c32str_ptr },
        { "string_operator_plus_eq_char",            &stub_string_operator_plus_eq_char_ptr },
        { "string_operator_index",                   &stub_string_operator_index_ptr },
        { "string_operator_index_const",             &stub_string_operator_index_const_ptr },
        { "string_to_latin1_chars",                  &stub_string_to_latin1_chars_ptr },
        { "string_to_utf8_chars",                    &stub_string_to_utf8_chars_ptr },
        { "string_to_utf16_chars",                   &stub_string_to_utf16_chars_ptr },
        { "string_to_utf32_chars",                   &stub_string_to_utf32_chars_ptr },
        { "string_to_wide_chars",                    &stub_string_to_wide_chars_ptr },
        { "string_resize",                           &stub_string_resize_ptr },

        // ── StringName ────────────────────────────────────────────────────
        { "string_name_new_with_latin1_chars",       &stub_string_name_new_with_latin1_chars_ptr },
        { "string_name_new_with_utf8_chars",         &stub_string_name_new_with_utf8_chars_ptr },
        { "string_name_new_with_utf8_chars_and_len", &stub_string_name_new_with_utf8_chars_and_len_ptr },

        // ── Variant ───────────────────────────────────────────────────────
        { "variant_new_nil",                         &stub_variant_new_nil_ptr },
        { "variant_new_copy",                        &stub_variant_new_copy_ptr },
        { "variant_destroy",                         &stub_variant_destroy_ptr },
        { "variant_get_type",                        &stub_variant_get_type_ptr },
        { "variant_get_type_name",                   &stub_variant_get_type_name_ptr },
        { "variant_booleanize",                      &stub_variant_booleanize_ptr },
        { "variant_hash",                            &stub_variant_hash_ptr },
        { "variant_hash_compare",                    &stub_variant_hash_compare_ptr },
        { "variant_stringify",                       &stub_variant_stringify_ptr },
        { "variant_call",                            &stub_variant_call_ptr },
        { "variant_call_static",                     &stub_variant_call_static_ptr },
        { "variant_construct",                       &stub_variant_construct_ptr },
        { "variant_duplicate",                       &stub_variant_duplicate_ptr },
        { "variant_get",                             &stub_variant_get_ptr },
        { "variant_set",                             &stub_variant_set_ptr },
        { "variant_get_named",                       &stub_variant_get_named_ptr },
        { "variant_set_named",                       &stub_variant_set_named_ptr },
        { "variant_get_indexed",                     &stub_variant_get_indexed_ptr },
        { "variant_set_indexed",                     &stub_variant_set_indexed_ptr },
        { "variant_get_keyed",                       &stub_variant_get_keyed_ptr },
        { "variant_set_keyed",                       &stub_variant_set_keyed_ptr },
        { "variant_has_key",                         &stub_variant_has_key_ptr },
        { "variant_can_convert",                     &stub_variant_can_convert_ptr },
        { "variant_can_convert_strict",              &stub_variant_can_convert_strict_ptr },
        { "variant_evaluate",                        &stub_variant_evaluate_ptr },
        { "variant_recursive_hash",                  &stub_variant_recursive_hash_ptr },
        { "variant_has_member",                      &stub_variant_has_member_ptr },
        { "variant_has_method",                      &stub_variant_has_method_ptr },
        { "variant_get_object_instance_id",          &stub_variant_get_object_instance_id_ptr },
        { "variant_get_constant_value",              &stub_variant_get_constant_value_ptr },
        { "variant_iter_init",                       &stub_variant_iter_init_ptr },
        { "variant_iter_next",                       &stub_variant_iter_next_ptr },
        { "variant_iter_get",                        &stub_variant_iter_get_ptr },
        { "variant_get_ptr_setter",                  &stub_variant_get_ptr_setter_ptr },
        { "variant_get_ptr_getter",                  &stub_variant_get_ptr_getter_ptr },
        { "variant_get_ptr_indexed_setter",          &stub_variant_get_ptr_indexed_setter_ptr },
        { "variant_get_ptr_indexed_getter",          &stub_variant_get_ptr_indexed_getter_ptr },
        { "variant_get_ptr_keyed_setter",            &stub_variant_get_ptr_keyed_setter_ptr },
        { "variant_get_ptr_keyed_getter",            &stub_variant_get_ptr_keyed_getter_ptr },
        { "variant_get_ptr_keyed_checker",           &stub_variant_get_ptr_keyed_checker_ptr },
        { "variant_get_ptr_operator_evaluator",      &stub_variant_get_ptr_operator_evaluator_ptr },
        { "variant_get_ptr_utility_function",        &stub_variant_get_ptr_utility_function_ptr },
        { "variant_get_ptr_internal_getter",         &stub_variant_get_ptr_internal_getter_ptr },
        { "variant_get_ptr_constructor",             &stub_variant_get_ptr_constructor_ptr },
        { "variant_get_ptr_destructor",              &stub_variant_get_ptr_destructor_ptr },
        { "variant_get_ptr_builtin_method",          &stub_variant_get_ptr_builtin_method_ptr },
        { "get_variant_from_type_constructor",       &stub_get_variant_from_type_constructor_ptr },
        { "get_variant_to_type_constructor",         &stub_get_variant_to_type_constructor_ptr },

        // ── Dictionary ────────────────────────────────────────────────────
        { "dictionary_operator_index",               &stub_dictionary_operator_index_ptr },
        { "dictionary_operator_index_const",         &stub_dictionary_operator_index_const_ptr },
        { "dictionary_set_typed",                    &stub_dictionary_set_typed_ptr },

        // ── Array ─────────────────────────────────────────────────────────
        { "array_operator_index",                    &stub_array_operator_index_ptr },
        { "array_operator_index_const",              &stub_array_operator_index_const_ptr },
        { "array_ref",                               &stub_array_ref_ptr },
        { "array_set_typed",                         &stub_array_set_typed_ptr },

        // ── No-ops: class system, objects, scripts, editor, packed arrays ──
        { "classdb_construct_object",                   &stub_classdb_construct_object_ptr },
        { "classdb_construct_object2",                  &stub_classdb_construct_object2_ptr },
        { "classdb_get_class_tag",                      &stub_noop_ptr },
        { "classdb_get_method_bind",                    &stub_classdb_get_method_bind_ptr },
        { "classdb_register_extension_class",           &stub_noop_ptr },
        { "classdb_register_extension_class2",          &stub_noop_ptr },
        { "classdb_register_extension_class3",          &stub_noop_ptr },
        { "classdb_register_extension_class4",          &stub_noop_ptr },
        { "classdb_register_extension_class5",          &stub_noop_ptr },
        { "classdb_register_extension_class_integer_constant", &stub_noop_ptr },
        { "classdb_register_extension_class_method",    &stub_noop_ptr },
        { "classdb_register_extension_class_property",  &stub_noop_ptr },
        { "classdb_register_extension_class_property_group", &stub_noop_ptr },
        { "classdb_register_extension_class_property_indexed", &stub_noop_ptr },
        { "classdb_register_extension_class_property_subgroup", &stub_noop_ptr },
        { "classdb_register_extension_class_signal",    &stub_noop_ptr },
        { "classdb_register_extension_class_virtual_method", &stub_noop_ptr },
        { "classdb_unregister_extension_class",         &stub_noop_ptr },
        { "callable_custom_create",                     &stub_noop_ptr },
        { "callable_custom_create2",                    &stub_noop_ptr },
        { "callable_custom_get_userdata",               &stub_noop_ptr },
        { "editor_add_plugin",                          &stub_noop_ptr },
        { "editor_help_load_xml_from_utf8_chars",       &stub_noop_ptr },
        { "editor_help_load_xml_from_utf8_chars_and_len", &stub_noop_ptr },
        { "editor_register_get_classes_used_callback",  &stub_noop_ptr },
        { "editor_remove_plugin",                       &stub_noop_ptr },
        { "file_access_get_buffer",                     &stub_noop_ptr },
        { "file_access_store_buffer",                   &stub_noop_ptr },
        { "get_library_path",                           &stub_noop_ptr },
        { "get_native_struct_size",                     &stub_noop_ptr },
        { "global_get_singleton",                       &stub_noop_ptr },
        { "image_ptr",                                  &stub_noop_ptr },
        { "image_ptrw",                                 &stub_noop_ptr },
        { "object_call_script_method",                  &stub_noop_ptr },
        { "object_cast_to",                             &stub_noop_ptr },
        { "object_destroy",                             &stub_object_destroy_ptr },
        { "object_free_instance_binding",               &stub_noop_ptr },
        { "object_get_class_name",                      &stub_noop_ptr },
        { "object_get_instance_binding",                &stub_object_get_instance_binding_ptr },
        { "object_get_instance_from_id",                &stub_noop_ptr },
        { "object_get_instance_id",                     &stub_noop_ptr },
        { "object_get_script_instance",                 &stub_noop_ptr },
        { "object_has_script_method",                   &stub_noop_ptr },
        { "object_method_bind_call",                    &stub_noop_ptr },
        { "object_method_bind_ptrcall",                 &stub_object_method_bind_ptrcall_ptr },
        { "object_set_instance",                        &stub_object_set_instance_ptr },
        { "object_set_instance_binding",                &stub_object_set_instance_binding_ptr },
        { "object_set_script_instance",                 &stub_noop_ptr },
        { "packed_byte_array_operator_index",           &stub_packed_byte_array_operator_index_ptr },
        { "packed_byte_array_operator_index_const",     &stub_packed_byte_array_operator_index_const_ptr },
        { "packed_color_array_operator_index",          &stub_noop_ptr },
        { "packed_color_array_operator_index_const",    &stub_noop_ptr },
        { "packed_float32_array_operator_index",        &stub_noop_ptr },
        { "packed_float32_array_operator_index_const",  &stub_noop_ptr },
        { "packed_float64_array_operator_index",        &stub_noop_ptr },
        { "packed_float64_array_operator_index_const",  &stub_noop_ptr },
        { "packed_int32_array_operator_index",          &stub_noop_ptr },
        { "packed_int32_array_operator_index_const",    &stub_noop_ptr },
        { "packed_int64_array_operator_index",          &stub_noop_ptr },
        { "packed_int64_array_operator_index_const",    &stub_noop_ptr },
        { "packed_string_array_operator_index",         &stub_noop_ptr },
        { "packed_string_array_operator_index_const",   &stub_noop_ptr },
        { "packed_vector2_array_operator_index",        &stub_noop_ptr },
        { "packed_vector2_array_operator_index_const",  &stub_noop_ptr },
        { "packed_vector3_array_operator_index",        &stub_noop_ptr },
        { "packed_vector3_array_operator_index_const",  &stub_noop_ptr },
        { "packed_vector4_array_operator_index",        &stub_noop_ptr },
        { "packed_vector4_array_operator_index_const",  &stub_noop_ptr },
        { "placeholder_script_instance_create",         &stub_noop_ptr },
        { "placeholder_script_instance_update",         &stub_noop_ptr },
        { "print_script_error",                         &stub_noop_ptr },
        { "print_script_error_with_message",            &stub_noop_ptr },
        { "ref_get_object",                             &stub_noop_ptr },
        { "ref_set_object",                             &stub_noop_ptr },
        { "register_main_loop_callbacks",               &stub_noop_ptr },
        { "script_instance_create",                     &stub_noop_ptr },
        { "script_instance_create2",                    &stub_noop_ptr },
        { "script_instance_create3",                    &stub_noop_ptr },
        { "worker_thread_pool_add_native_group_task",   &stub_noop_ptr },
        { "worker_thread_pool_add_native_task",         &stub_noop_ptr },
        { "xml_parser_open_buffer",                     &stub_noop_ptr },
    };
    return table;
}

// ─── stub_get_proc_address ────────────────────────────────────────────────

static void *stub_get_proc_address(const char *p_function_name) {
    const auto &table = dispatch_table();
    auto it = table.find(std::string_view(p_function_name));
    if (it != table.end()) return *it->second;
    // Unknown function: return the no-op rather than nullptr to avoid
    // a null-pointer dereference in godot-cpp's LOAD_PROC_ADDRESS macro.
    return stub_noop_ptr;
}

// ─── Public API ───────────────────────────────────────────────────────────

// godot-cpp's GDExtensionBinding::init is declared in godot_cpp/godot.hpp.
#include <godot_cpp/godot.hpp>

namespace godot_stub {

static void noop_init_level(godot::ModuleInitializationLevel) {}

void init() {
    // GDExtensionBinding::init() requires:
    //   1. A non-null init_callback (set via register_initializer).
    //   2. A non-null GDExtensionInitialization* (written by init, not read back by us).
    //
    // We pass a no-op initializer — the fuzz targets only use built-in Godot
    // types (String, Dictionary, Variant, Array), which are initialized by
    // Variant::init_bindings() inside GDExtensionBinding::init() before any
    // user-level initialization callbacks run.
    GDExtensionInitialization gdext_init = {};
    godot::GDExtensionBinding::InitObject init_obj(
        reinterpret_cast<GDExtensionInterfaceGetProcAddress>(stub_get_proc_address),
        nullptr,
        &gdext_init);
    init_obj.register_initializer(noop_init_level);
    GDExtensionBool ok = init_obj.init();
    if (!ok) {
        ::fprintf(stderr, "godot_stub::init() — GDExtensionBinding::init failed!\n");
        ::abort();
    }
}

void shutdown() {
    // Optional; the process exits cleanly either way for fuzz targets.
}

} // namespace godot_stub
