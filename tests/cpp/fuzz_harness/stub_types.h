// Internal stub type definitions shared across all stub_*.cpp translation units.
// Not installed; only used within tests/cpp/fuzz_harness/.
//
// Blob layout summary (matching extension_api-4-5.json float_64 sizes):
//   String      8 bytes → stores std::u32string *
//   StringName  8 bytes → stores std::string * (interned globally)
//   NodePath    8 bytes → stores std::string * (interned globally, same pool)
//   Dictionary  8 bytes → stores StubDict *
//   Array       8 bytes → stores StubArray *
//   Variant    24 bytes → { uint32_t type_id; uint32_t _pad; StubVariantData data; }
//
// All heap objects are owned by the blob that contains their pointer.
// Copying a blob clones the heap object (no reference counting).

#pragma once

#include <cstddef>
#include <cstdint>
#include <cstring>
#include <deque>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

// Variant type ids (mirrors GDExtensionVariantType)
enum StubVariantType : uint32_t {
    SVT_NIL          =  0,
    SVT_BOOL         =  1,
    SVT_INT          =  2,
    SVT_FLOAT        =  3,
    SVT_STRING       =  4,
    SVT_STRING_NAME  = 21,
    SVT_NODE_PATH    = 22,
    SVT_DICTIONARY   = 27,
    SVT_ARRAY        = 28,
};

// ─── Forward declarations ──────────────────────────────────────────────────

struct StubDict;
struct StubArray;

// ─── Variant blob ─────────────────────────────────────────────────────────
// Matches 24 bytes on float_64 builds.

struct StubVariantData {
    union {
        int64_t  as_int;
        double   as_double;
        uint8_t  as_bool;
        void    *as_ptr;    // for String*, StringName*, StubDict*, StubArray*
        uint8_t  raw[16];
    };
};

struct StubVariant {
    uint32_t       type_id;  // StubVariantType
    uint32_t       _pad;
    StubVariantData data;
};
static_assert(sizeof(StubVariant) == 24, "StubVariant must be 24 bytes");

// ─── Dictionary ───────────────────────────────────────────────────────────

struct StubDict {
    // Keyed by string form of the key Variant (covers all string and int keys).
    // A parallel entries vector preserves key ordering and provides stable
    // pointers for dictionary_operator_index.
    struct Entry {
        std::string   key_repr;  // for lookup
        StubVariant   key;
        StubVariant   value;
    };
    std::vector<Entry> entries;

    StubVariant *find(std::string_view key_repr);
    StubVariant *insert_or_find(std::string_view key_repr, const StubVariant &key);
};

// ─── Array ────────────────────────────────────────────────────────────────

struct StubArray {
    // std::deque gives push_back without pointer invalidation of existing elements.
    std::deque<StubVariant> elements;
};

// ─── Helpers (defined in stub_variant.cpp) ───────────────────────────────

// Deep-clone a blob of the given type_id.  Ownership of the clone is transferred
// to the caller (i.e. the caller owns any heap objects in the clone).
void stub_variant_clone(StubVariant *dst, const StubVariant *src);

// Free any heap objects owned by this variant blob.  Does NOT free the blob
// itself (which is usually stack-allocated or part of a container).
void stub_variant_free_owned(StubVariant *v);

// Convert a Variant key to a string representation suitable for dictionary lookup.
std::string stub_variant_to_key(const StubVariant *v);

// ─── StringName helpers (defined in stub_string.cpp) ─────────────────────

// Intern a string into the global StringName pool and return a stable pointer.
std::string *sn_intern(const char *p, ptrdiff_t len = -1);

// ─── String helpers (defined in stub_string.cpp) ─────────────────────────

// Create a new std::u32string from a UTF-8 C string.
std::u32string *stub_str_from_utf8(const char *p, ptrdiff_t len = -1);

// Create a new std::u32string from a Latin-1 (ISO 8859-1) C string.
std::u32string *stub_str_from_latin1(const char *p, ptrdiff_t len = -1);

// Convert std::u32string back to UTF-8.  Returns byte count written (excluding NUL).
ptrdiff_t stub_str_to_utf8(const std::u32string *s, char *out, ptrdiff_t out_len);

// Convert std::u32string back to Latin-1.
ptrdiff_t stub_str_to_latin1(const std::u32string *s, char *out, ptrdiff_t out_len);

// ─── String blob accessors ────────────────────────────────────────────────

inline std::u32string **str_blob_ptr(void *blob) {
    return reinterpret_cast<std::u32string **>(blob);
}
inline std::u32string * const *str_blob_ptr(const void *blob) {
    return reinterpret_cast<std::u32string * const *>(blob);
}
inline std::u32string *str_blob_get(void *blob) {
    return *str_blob_ptr(blob);
}
inline std::u32string *str_blob_get(const void *blob) {
    return *str_blob_ptr(blob);
}
inline void str_blob_set(void *blob, std::u32string *s) {
    *str_blob_ptr(blob) = s;
}

// ─── StringName / NodePath blob accessors ────────────────────────────────

inline std::string **sn_blob_ptr(void *blob) {
    return reinterpret_cast<std::string **>(blob);
}
inline std::string *sn_blob_get(void *blob) {
    return *sn_blob_ptr(blob);
}
inline std::string *sn_blob_get(const void *blob) {
    return *reinterpret_cast<std::string * const *>(blob);
}
inline void sn_blob_set(void *blob, std::string *s) {
    *sn_blob_ptr(blob) = s;
}

// ─── Dictionary / Array blob accessors ───────────────────────────────────

inline StubDict **dict_blob_ptr(void *blob) {
    return reinterpret_cast<StubDict **>(blob);
}
inline StubDict *dict_blob_get(void *blob) {
    return *dict_blob_ptr(blob);
}
inline StubDict *dict_blob_get(const void *blob) {
    return *reinterpret_cast<StubDict * const *>(blob);
}
inline void dict_blob_set(void *blob, StubDict *d) {
    *dict_blob_ptr(blob) = d;
}

inline StubArray **arr_blob_ptr(void *blob) {
    return reinterpret_cast<StubArray **>(blob);
}
inline StubArray *arr_blob_get(void *blob) {
    return *arr_blob_ptr(blob);
}
inline StubArray *arr_blob_get(const void *blob) {
    return *reinterpret_cast<StubArray * const *>(blob);
}
inline void arr_blob_set(void *blob, StubArray *a) {
    *arr_blob_ptr(blob) = a;
}
