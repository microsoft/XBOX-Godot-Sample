// stub_packed_array.cpp — PackedByteArray element-access interface functions.
//
// godot-cpp implements PackedByteArray::operator[](), ptr() and ptrw() in terms
// of the GDExtension interface functions packed_byte_array_operator_index and
// packed_byte_array_operator_index_const (see godot-cpp
// src/variant/packed_arrays.cpp).  The constructor / destructor / builtin
// methods (resize, size, set, …) are handled by the dispatch switches in
// stub_variant.cpp; this translation unit only provides the raw indexing slots.
//
// p_self points at the 16-byte opaque blob, whose first 8 bytes store the
// std::vector<uint8_t> * (see pba_blob_* accessors in stub_types.h).

#include "stub_types.h"

#include <cstdint>
#include <vector>

extern "C" {

// GDExtensionInterfacePackedByteArrayOperatorIndex:
//   uint8_t *f(GDExtensionTypePtr p_self, GDExtensionInt p_index)
static uint8_t *s_packed_byte_array_operator_index(void *p_self, int64_t p_index) {
    std::vector<uint8_t> *v = pba_blob_get(p_self);
    if (!v || p_index < 0 || (size_t)p_index >= v->size()) return nullptr;
    return v->data() + p_index;
}

// GDExtensionInterfacePackedByteArrayOperatorIndexConst:
//   const uint8_t *f(GDExtensionConstTypePtr p_self, GDExtensionInt p_index)
static const uint8_t *s_packed_byte_array_operator_index_const(const void *p_self, int64_t p_index) {
    const std::vector<uint8_t> *v = pba_blob_get(p_self);
    if (!v || p_index < 0 || (size_t)p_index >= v->size()) return nullptr;
    return v->data() + p_index;
}

} // extern "C"

void *stub_packed_byte_array_operator_index_ptr       = (void *)s_packed_byte_array_operator_index;
void *stub_packed_byte_array_operator_index_const_ptr = (void *)s_packed_byte_array_operator_index_const;
