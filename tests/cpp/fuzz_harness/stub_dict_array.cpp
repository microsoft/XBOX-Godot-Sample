// stub_dict_array.cpp — GDExtension Dictionary and Array direct interface stubs.
//
// These are the "direct" interface functions (dictionary_operator_index,
// array_operator_index, etc.) as opposed to the built-in methods dispatched
// through variant_get_ptr_builtin_method.

#include "stub_types.h"

extern "C" {

// ─── Dictionary ───────────────────────────────────────────────────────────

static void *s_dictionary_operator_index(void *p_self, const void *p_key) {
    StubDict *d = dict_blob_get(p_self);
    if (!d) return nullptr;
    std::string key_repr = stub_variant_to_key(
        reinterpret_cast<const StubVariant *>(p_key));
    StubVariant *slot = d->insert_or_find(
        key_repr, *reinterpret_cast<const StubVariant *>(p_key));
    return slot;
}

static const void *s_dictionary_operator_index_const(const void *p_self, const void *p_key) {
    StubDict *d = dict_blob_get(p_self);
    if (!d) return nullptr;
    std::string key_repr = stub_variant_to_key(
        reinterpret_cast<const StubVariant *>(p_key));
    return d->find(key_repr);
}

// dictionary_set_typed: used when constructing a typed dictionary.
// For our stub, just treat it as a no-op (we don't enforce type constraints).
static void s_dictionary_set_typed(void */*p_self*/, uint32_t /*key_type*/,
                                   const void */*key_class_name*/,
                                   const void */*key_script*/,
                                   uint32_t /*value_type*/,
                                   const void */*value_class_name*/,
                                   const void */*value_script*/) {
}

// ─── Array ────────────────────────────────────────────────────────────────

static void *s_array_operator_index(void *p_self, int64_t p_index) {
    StubArray *a = arr_blob_get(p_self);
    if (!a || p_index < 0 || (size_t)p_index >= a->elements.size())
        return nullptr;
    return &a->elements[(size_t)p_index];
}

static const void *s_array_operator_index_const(const void *p_self, int64_t p_index) {
    StubArray *a = arr_blob_get(p_self);
    if (!a || p_index < 0 || (size_t)p_index >= a->elements.size())
        return nullptr;
    return &a->elements[(size_t)p_index];
}

// array_ref: used for typed array construction; stub as no-op.
static void s_array_ref(void */*p_self*/, const void */*p_from*/) {}

// array_set_typed: type annotation for typed arrays; stub as no-op.
static void s_array_set_typed(void */*p_self*/, uint32_t /*elem_type*/,
                               const void */*elem_class_name*/,
                               const void */*elem_script*/) {}

} // extern "C"

// ─── Exported function pointer table ─────────────────────────────────────
void *stub_dictionary_operator_index_ptr       = (void *)s_dictionary_operator_index;
void *stub_dictionary_operator_index_const_ptr = (void *)s_dictionary_operator_index_const;
void *stub_dictionary_set_typed_ptr            = (void *)s_dictionary_set_typed;
void *stub_array_operator_index_ptr            = (void *)s_array_operator_index;
void *stub_array_operator_index_const_ptr      = (void *)s_array_operator_index_const;
void *stub_array_ref_ptr                       = (void *)s_array_ref;
void *stub_array_set_typed_ptr                 = (void *)s_array_set_typed;
