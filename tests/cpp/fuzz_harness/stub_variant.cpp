// stub_variant.cpp — GDExtension Variant interface stubs.
//
// Implements:
//   variant_new_nil / variant_new_copy / variant_destroy
//   variant_get_type / variant_get_type_name
//   get_variant_from_type_constructor / get_variant_to_type_constructor
//   variant_get_ptr_constructor / variant_get_ptr_destructor
//   variant_get_ptr_builtin_method (dispatched by type + method name)
//   variant_stringify
//   variant_call / variant_call_static / variant_construct (stubs)
//   variant_has_key / variant_get_keyed / variant_set_keyed (forwarded to dict stubs)
//
// All other variant_* functions (operators, iterators, etc.) are stubbed to
// no-ops or trivial returns.  They are not used by our Phase 3 fuzz targets.

#include "stub_types.h"

#include <cstdio>
#include <cstring>
#include <string>
#include <string_view>
#include <unordered_map>

// ─── StubVariant deep-clone / free ───────────────────────────────────────

void stub_variant_clone(StubVariant *dst, const StubVariant *src) {
    if (!dst || !src) return;
    dst->type_id = src->type_id;
    dst->_pad    = 0;
    switch (src->type_id) {
    case SVT_STRING: {
        const std::u32string *orig = (const std::u32string *)src->data.as_ptr;
        dst->data.as_ptr = orig ? new std::u32string(*orig) : new std::u32string();
        break;
    }
    case SVT_STRING_NAME:
    case SVT_NODE_PATH:
        // Interned: just copy the pointer; pool owns the object.
        dst->data.as_ptr = src->data.as_ptr;
        break;
    case SVT_DICTIONARY: {
        auto *orig = (StubDict *)src->data.as_ptr;
        if (orig) {
            auto *copy = new StubDict();
            for (const auto &e : orig->entries) {
                StubDict::Entry ne;
                ne.key_repr = e.key_repr;
                stub_variant_clone(&ne.key, &e.key);
                stub_variant_clone(&ne.value, &e.value);
                copy->entries.push_back(std::move(ne));
            }
            dst->data.as_ptr = copy;
        } else {
            dst->data.as_ptr = new StubDict();
        }
        break;
    }
    case SVT_ARRAY: {
        auto *orig = (StubArray *)src->data.as_ptr;
        if (orig) {
            auto *copy = new StubArray();
            for (const auto &elem : orig->elements) {
                StubVariant cv;
                stub_variant_clone(&cv, &elem);
                copy->elements.push_back(cv);
            }
            dst->data.as_ptr = copy;
        } else {
            dst->data.as_ptr = new StubArray();
        }
        break;
    }
    default:
        ::memcpy(dst->data.raw, src->data.raw, sizeof(dst->data.raw));
        break;
    }
}

void stub_variant_free_owned(StubVariant *v) {
    if (!v) return;
    switch (v->type_id) {
    case SVT_STRING:
        delete (std::u32string *)v->data.as_ptr;
        break;
    case SVT_DICTIONARY:
        if (auto *d = (StubDict *)v->data.as_ptr) {
            for (auto &e : d->entries) {
                stub_variant_free_owned(&e.key);
                stub_variant_free_owned(&e.value);
            }
            delete d;
        }
        break;
    case SVT_ARRAY:
        if (auto *a = (StubArray *)v->data.as_ptr) {
            for (auto &elem : a->elements)
                stub_variant_free_owned(&elem);
            delete a;
        }
        break;
    default:
        break;
    }
    v->type_id = SVT_NIL;
    v->data.as_ptr = nullptr;
}

std::string stub_variant_to_key(const StubVariant *v) {
    if (!v) return "";
    switch (v->type_id) {
    case SVT_STRING: {
        auto *s = (const std::u32string *)v->data.as_ptr;
        if (!s) return "";
        std::string r;
        r.reserve(s->size());
        for (char32_t c : *s) r += (c < 128) ? (char)c : '?';
        return r;
    }
    case SVT_STRING_NAME:
    case SVT_NODE_PATH: {
        auto *s = (const std::string *)v->data.as_ptr;
        return s ? *s : "";
    }
    case SVT_INT:
        return std::to_string(v->data.as_int);
    default:
        return "key_type" + std::to_string(v->type_id);
    }
}

// ─── StubDict helpers ────────────────────────────────────────────────────

StubVariant *StubDict::find(std::string_view key_repr) {
    for (auto &e : entries)
        if (e.key_repr == key_repr) return &e.value;
    return nullptr;
}

StubVariant *StubDict::insert_or_find(std::string_view key_repr, const StubVariant &key) {
    for (auto &e : entries)
        if (e.key_repr == key_repr) return &e.value;
    Entry ne;
    ne.key_repr = std::string(key_repr);
    stub_variant_clone(&ne.key, &key);
    ne.value = StubVariant{};
    entries.push_back(std::move(ne));
    return &entries.back().value;
}

// ─── GDExtensionVariantType enum (mirror of the public header) ───────────

// ─── Variant blob accessors ───────────────────────────────────────────────

static inline StubVariant *vblob(void *p) { return reinterpret_cast<StubVariant *>(p); }
static inline const StubVariant *vblob(const void *p) { return reinterpret_cast<const StubVariant *>(p); }

// ─── variant_new_nil / new_copy / destroy ────────────────────────────────

extern "C" {

static void s_variant_new_nil(void *r_dest) {
    StubVariant *v = vblob(r_dest);
    ::memset(v, 0, sizeof(*v));
}

static void s_variant_new_copy(void *r_dest, const void *p_src) {
    stub_variant_clone(vblob(r_dest), vblob(p_src));
}

static void s_variant_destroy(void *p_self) {
    stub_variant_free_owned(vblob(p_self));
}

static uint32_t s_variant_get_type(const void *p_self) {
    return vblob(p_self)->type_id;
}

static void s_variant_get_type_name(uint32_t p_type, void *r_name) {
    static const char *names[] = {
        "Nil","bool","int","float","String","Vector2","Vector2i","Rect2",
        "Rect2i","Vector3","Vector3i","Transform2D","Vector4","Vector4i",
        "Plane","Quaternion","AABB","Basis","Transform3D","Projection",
        "Color","StringName","NodePath","RID","Object","Callable","Signal",
        "Dictionary","Array","PackedByteArray","PackedInt32Array",
        "PackedInt64Array","PackedFloat32Array","PackedFloat64Array",
        "PackedStringArray","PackedVector2Array","PackedVector3Array",
        "PackedColorArray","PackedVector4Array",
    };
    const char *name = (p_type < 39) ? names[p_type] : "Unknown";
    // r_name is an uninitialized StringName blob; intern it
    auto *interned = new std::string(name);
    sn_blob_set(r_name, interned);
}

static void s_variant_stringify(const void *p_self, void *r_str) {
    const StubVariant *v = vblob(p_self);
    std::u32string *s = nullptr;
    switch (v->type_id) {
    case SVT_STRING:
        s = v->data.as_ptr
            ? new std::u32string(*(const std::u32string *)v->data.as_ptr)
            : new std::u32string();
        break;
    case SVT_INT: {
        auto tmp = std::to_string(v->data.as_int);
        s = new std::u32string(tmp.begin(), tmp.end());
        break;
    }
    case SVT_BOOL:
        s = new std::u32string(v->data.as_bool ? U"true" : U"false");
        break;
    default:
        s = new std::u32string(U"[variant]");
        break;
    }
    delete str_blob_get(r_str);
    str_blob_set(r_str, s);
}

static uint8_t s_variant_booleanize(const void *p_self) {
    const StubVariant *v = vblob(p_self);
    switch (v->type_id) {
    case SVT_NIL:   return 0;
    case SVT_BOOL:  return v->data.as_bool ? 1 : 0;
    case SVT_INT:   return v->data.as_int != 0 ? 1 : 0;
    case SVT_FLOAT: return v->data.as_double != 0.0 ? 1 : 0;
    case SVT_STRING:
        return (v->data.as_ptr && !((std::u32string *)v->data.as_ptr)->empty()) ? 1 : 0;
    default: return 1;
    }
}

static int64_t s_variant_hash(const void *p_self) {
    const StubVariant *v = vblob(p_self);
    switch (v->type_id) {
    case SVT_INT:  return v->data.as_int;
    case SVT_BOOL: return v->data.as_bool ? 1 : 0;
    case SVT_STRING: {
        const std::u32string *s = (const std::u32string *)v->data.as_ptr;
        if (!s) return 0;
        size_t h = 0;
        for (char32_t c : *s) h = h * 31 + c;
        return (int64_t)h;
    }
    default: return (int64_t)v->type_id;
    }
}

static uint8_t s_variant_hash_compare(const void *p_self, const void *p_other) {
    return (s_variant_hash(p_self) == s_variant_hash(p_other)) ? 1 : 0;
}

// ── from/to type constructors ─────────────────────────────────────────────

// Called as: from_type_ctor(void* r_variant_blob, void* p_type_blob)
// Packs a typed value into a Variant blob.

static void s_from_bool(void *r_dest, void *p_src) {
    StubVariant *v = vblob(r_dest);
    v->type_id = SVT_BOOL;
    v->data.as_bool = *(uint8_t *)p_src;
}
static void s_from_int(void *r_dest, void *p_src) {
    StubVariant *v = vblob(r_dest);
    v->type_id = SVT_INT;
    v->data.as_int = *(int64_t *)p_src;
}
static void s_from_float(void *r_dest, void *p_src) {
    StubVariant *v = vblob(r_dest);
    v->type_id = SVT_FLOAT;
    v->data.as_double = *(double *)p_src;
}
static void s_from_string(void *r_dest, void *p_src) {
    StubVariant *v = vblob(r_dest);
    v->type_id = SVT_STRING;
    const std::u32string *orig = str_blob_get(p_src);
    v->data.as_ptr = orig ? new std::u32string(*orig) : new std::u32string();
}
static void s_from_string_name(void *r_dest, void *p_src) {
    StubVariant *v = vblob(r_dest);
    v->type_id = SVT_STRING_NAME;
    v->data.as_ptr = sn_blob_get(p_src); // interned, no clone needed
}
static void s_from_node_path(void *r_dest, void *p_src) {
    StubVariant *v = vblob(r_dest);
    v->type_id = SVT_NODE_PATH;
    v->data.as_ptr = sn_blob_get(p_src);
}
static void s_from_dictionary(void *r_dest, void *p_src) {
    StubVariant *v = vblob(r_dest);
    v->type_id = SVT_DICTIONARY;
    StubDict *orig = dict_blob_get(p_src);
    // Clone dictionary
    if (orig) {
        StubVariant tmp;
        tmp.type_id = SVT_DICTIONARY;
        tmp.data.as_ptr = orig;
        StubVariant cloned;
        stub_variant_clone(&cloned, &tmp);
        v->data.as_ptr = cloned.data.as_ptr;
    } else {
        v->data.as_ptr = new StubDict();
    }
}
static void s_from_array(void *r_dest, void *p_src) {
    StubVariant *v = vblob(r_dest);
    v->type_id = SVT_ARRAY;
    StubArray *orig = arr_blob_get(p_src);
    if (orig) {
        StubVariant tmp;
        tmp.type_id = SVT_ARRAY;
        tmp.data.as_ptr = orig;
        StubVariant cloned;
        stub_variant_clone(&cloned, &tmp);
        v->data.as_ptr = cloned.data.as_ptr;
    } else {
        v->data.as_ptr = new StubArray();
    }
}
// Generic from_type for unsupported types: zero-fill + set type
static void s_from_generic_nil(void *r_dest, void *) {
    vblob(r_dest)->type_id = SVT_NIL;
    ::memset(vblob(r_dest)->data.raw, 0, 16);
}

// to_type constructors (Variant → typed value)
static void s_to_bool(void *r_dest, void *p_variant) {
    *(uint8_t *)r_dest = (uint8_t)s_variant_booleanize(p_variant);
}
static void s_to_int(void *r_dest, void *p_variant) {
    const StubVariant *v = vblob(p_variant);
    switch (v->type_id) {
    case SVT_INT:   *(int64_t *)r_dest = v->data.as_int;                     break;
    case SVT_BOOL:  *(int64_t *)r_dest = v->data.as_bool ? 1 : 0;            break;
    case SVT_FLOAT: *(int64_t *)r_dest = (int64_t)v->data.as_double;         break;
    default:        *(int64_t *)r_dest = 0;                                  break;
    }
}
static void s_to_float(void *r_dest, void *p_variant) {
    const StubVariant *v = vblob(p_variant);
    switch (v->type_id) {
    case SVT_FLOAT: *(double *)r_dest = v->data.as_double;                   break;
    case SVT_INT:   *(double *)r_dest = (double)v->data.as_int;              break;
    default:        *(double *)r_dest = 0.0;                                 break;
    }
}
static void s_to_string(void *r_dest, void *p_variant) {
    const StubVariant *v = vblob(p_variant);
    if (v->type_id == SVT_STRING) {
        const std::u32string *orig = (const std::u32string *)v->data.as_ptr;
        delete str_blob_get(r_dest);
        str_blob_set(r_dest, orig ? new std::u32string(*orig) : new std::u32string());
    }
}
static void s_to_dictionary(void *r_dest, void *p_variant) {
    const StubVariant *v = vblob(p_variant);
    if (v->type_id == SVT_DICTIONARY) {
        // Clone from variant into the dict blob
        StubVariant tmp;
        stub_variant_clone(&tmp, v);
        StubDict *old = dict_blob_get(r_dest);
        if (old) {
            StubVariant ov; ov.type_id = SVT_DICTIONARY; ov.data.as_ptr = old;
            stub_variant_free_owned(&ov);
        }
        dict_blob_set(r_dest, (StubDict *)tmp.data.as_ptr);
    }
}
static void s_to_array(void *r_dest, void *p_variant) {
    const StubVariant *v = vblob(p_variant);
    if (v->type_id == SVT_ARRAY) {
        StubVariant tmp;
        stub_variant_clone(&tmp, v);
        StubArray *old = arr_blob_get(r_dest);
        if (old) {
            StubVariant ov; ov.type_id = SVT_ARRAY; ov.data.as_ptr = old;
            stub_variant_free_owned(&ov);
        }
        arr_blob_set(r_dest, (StubArray *)tmp.data.as_ptr);
    }
}
static void s_to_generic_nil(void *r_dest, void *) {
    ::memset(r_dest, 0, 8);
}

typedef void (*FromTypeFunc)(void *, void *);
typedef void (*ToTypeFunc)(void *, void *);

static FromTypeFunc s_get_variant_from_type_constructor(uint32_t p_type) {
    switch (p_type) {
    case SVT_BOOL:        return s_from_bool;
    case SVT_INT:         return s_from_int;
    case SVT_FLOAT:       return s_from_float;
    case SVT_STRING:      return s_from_string;
    case SVT_STRING_NAME: return s_from_string_name;
    case SVT_NODE_PATH:   return s_from_node_path;
    case SVT_DICTIONARY:  return s_from_dictionary;
    case SVT_ARRAY:       return s_from_array;
    default:              return s_from_generic_nil;
    }
}

static ToTypeFunc s_get_variant_to_type_constructor(uint32_t p_type) {
    switch (p_type) {
    case SVT_BOOL:        return s_to_bool;
    case SVT_INT:         return s_to_int;
    case SVT_FLOAT:       return s_to_float;
    case SVT_STRING:      return s_to_string;
    case SVT_DICTIONARY:  return s_to_dictionary;
    case SVT_ARRAY:       return s_to_array;
    default:              return s_to_generic_nil;
    }
}

// ── variant_get_ptr_constructor / destructor ──────────────────────────────

// GDExtensionPtrConstructor: void f(void *p_base, const void *const *p_args)

static void s_string_ctor_default(void *p_base, const void *const *) {
    delete str_blob_get(p_base);
    str_blob_set(p_base, new std::u32string());
}
static void s_string_ctor_copy(void *p_base, const void *const *p_args) {
    const std::u32string *orig = str_blob_get(p_args[0]);
    delete str_blob_get(p_base);
    str_blob_set(p_base, orig ? new std::u32string(*orig) : new std::u32string());
}
static void s_string_dtor(void *p_base) {
    delete str_blob_get(p_base);
    str_blob_set(p_base, nullptr);
}

static void s_string_name_ctor_default(void *p_base, const void *const *) {
    sn_blob_set(p_base, sn_intern(""));
}
static void s_string_name_ctor_copy(void *p_base, const void *const *p_args) {
    sn_blob_set(p_base, sn_blob_get(p_args[0]));
}
static void s_string_name_dtor(void *) { /* interned; pool owns */ }

static void s_dict_ctor_default(void *p_base, const void *const *) {
    StubDict *old = dict_blob_get(p_base);
    if (old) { StubVariant ov{SVT_DICTIONARY,0,{}}; ov.data.as_ptr=old; stub_variant_free_owned(&ov); }
    dict_blob_set(p_base, new StubDict());
}
static void s_dict_ctor_copy(void *p_base, const void *const *p_args) {
    StubVariant src{SVT_DICTIONARY,0,{}};
    src.data.as_ptr = dict_blob_get(p_args[0]);
    StubVariant cloned; stub_variant_clone(&cloned, &src);
    StubDict *old = dict_blob_get(p_base);
    if (old) { StubVariant ov{SVT_DICTIONARY,0,{}}; ov.data.as_ptr=old; stub_variant_free_owned(&ov); }
    dict_blob_set(p_base, (StubDict *)cloned.data.as_ptr);
}
static void s_dict_dtor(void *p_base) {
    StubDict *d = dict_blob_get(p_base);
    if (!d) return;
    for (auto &e : d->entries) { stub_variant_free_owned(&e.key); stub_variant_free_owned(&e.value); }
    delete d;
    dict_blob_set(p_base, nullptr);
}

static void s_array_ctor_default(void *p_base, const void *const *) {
    StubArray *old = arr_blob_get(p_base);
    if (old) { StubVariant ov{SVT_ARRAY,0,{}}; ov.data.as_ptr=old; stub_variant_free_owned(&ov); }
    arr_blob_set(p_base, new StubArray());
}
static void s_array_ctor_copy(void *p_base, const void *const *p_args) {
    StubVariant src{SVT_ARRAY,0,{}};
    src.data.as_ptr = arr_blob_get(p_args[0]);
    StubVariant cloned; stub_variant_clone(&cloned, &src);
    StubArray *old = arr_blob_get(p_base);
    if (old) { StubVariant ov{SVT_ARRAY,0,{}}; ov.data.as_ptr=old; stub_variant_free_owned(&ov); }
    arr_blob_set(p_base, (StubArray *)cloned.data.as_ptr);
}
static void s_array_dtor(void *p_base) {
    StubArray *a = arr_blob_get(p_base);
    if (!a) return;
    for (auto &elem : a->elements) stub_variant_free_owned(&elem);
    delete a;
    arr_blob_set(p_base, nullptr);
}

// ── PackedByteArray ctor / copy / dtor ────────────────────────────────────
static void s_pba_ctor_default(void *p_base, const void *const *) {
    delete pba_blob_get(p_base);
    pba_blob_set(p_base, new std::vector<uint8_t>());
}
static void s_pba_ctor_copy(void *p_base, const void *const *p_args) {
    const std::vector<uint8_t> *orig = pba_blob_get(p_args[0]);
    delete pba_blob_get(p_base);
    pba_blob_set(p_base, orig ? new std::vector<uint8_t>(*orig) : new std::vector<uint8_t>());
}
static void s_pba_dtor(void *p_base) {
    delete pba_blob_get(p_base);
    pba_blob_set(p_base, nullptr);
}

static void s_noop_ctor(void *p_base, const void *const *) { ::memset(p_base, 0, 8); }
static void s_noop_dtor(void *) {}

typedef void (*PFNCtor)(void *, const void *const *);
typedef void (*PFNDtor)(void *);

static PFNCtor s_variant_get_ptr_constructor(uint32_t p_type, int32_t p_constructor) {
    switch (p_type) {
    case SVT_STRING:
        return (p_constructor == 0) ? s_string_ctor_default :
               (p_constructor == 1) ? s_string_ctor_copy : s_noop_ctor;
    case SVT_STRING_NAME:
    case SVT_NODE_PATH:
        return (p_constructor == 0) ? s_string_name_ctor_default :
               (p_constructor == 1) ? s_string_name_ctor_copy : s_noop_ctor;
    case SVT_DICTIONARY:
        return (p_constructor == 0) ? s_dict_ctor_default :
               (p_constructor == 1) ? s_dict_ctor_copy : s_noop_ctor;
    case SVT_ARRAY:
        return (p_constructor == 0) ? s_array_ctor_default :
               (p_constructor == 1) ? s_array_ctor_copy : s_noop_ctor;
    case SVT_PACKED_BYTE_ARRAY:
        return (p_constructor == 0) ? s_pba_ctor_default :
               (p_constructor == 1) ? s_pba_ctor_copy : s_noop_ctor;
    default:
        return s_noop_ctor;
    }
}

static PFNDtor s_variant_get_ptr_destructor(uint32_t p_type) {
    switch (p_type) {
    case SVT_STRING:      return s_string_dtor;
    case SVT_STRING_NAME: return s_string_name_dtor;
    case SVT_NODE_PATH:   return s_string_name_dtor;
    case SVT_DICTIONARY:  return s_dict_dtor;
    case SVT_ARRAY:       return s_array_dtor;
    case SVT_PACKED_BYTE_ARRAY: return s_pba_dtor;
    default:              return s_noop_dtor;
    }
}

// ── variant_get_ptr_builtin_method ────────────────────────────────────────
//
// GDExtensionPtrBuiltInMethod signature:
//   void f(void *p_base, const void *const *p_args, void *r_return, int32_t p_arg_count)

typedef void (*PFNBuiltin)(void *, const void *const *, void *, int32_t);
static void s_noop_builtin(void *, const void *const *, void *, int32_t) {}

// ── Array built-in method stubs ───────────────────────────────────────────

static void s_array_push_back(void *p_base, const void *const *p_args, void *, int32_t) {
    StubArray *a = arr_blob_get(p_base);
    if (!a) return;
    StubVariant elem;
    stub_variant_clone(&elem, vblob(p_args[0]));
    a->elements.push_back(elem);
}

static void s_array_size(void *p_base, const void *const *, void *r_ret, int32_t) {
    StubArray *a = arr_blob_get(p_base);
    if (r_ret) *(int64_t *)r_ret = a ? (int64_t)a->elements.size() : 0LL;
}

static void s_array_is_empty(void *p_base, const void *const *, void *r_ret, int32_t) {
    StubArray *a = arr_blob_get(p_base);
    if (r_ret) *(uint8_t *)r_ret = (!a || a->elements.empty()) ? 1 : 0;
}

static void s_array_clear(void *p_base, const void *const *, void *, int32_t) {
    StubArray *a = arr_blob_get(p_base);
    if (!a) return;
    for (auto &elem : a->elements) stub_variant_free_owned(&elem);
    a->elements.clear();
}

static void s_array_get(void *p_base, const void *const *p_args, void *r_ret, int32_t) {
    StubArray *a = arr_blob_get(p_base);
    if (!a || !r_ret) return;
    int64_t idx = *(const int64_t *)p_args[0];
    if (idx < 0 || (size_t)idx >= a->elements.size()) return;
    stub_variant_clone(vblob(r_ret), &a->elements[(size_t)idx]);
}

static void s_array_set(void *p_base, const void *const *p_args, void *, int32_t) {
    StubArray *a = arr_blob_get(p_base);
    if (!a) return;
    int64_t idx = *(const int64_t *)p_args[0];
    if (idx < 0 || (size_t)idx >= a->elements.size()) return;
    stub_variant_free_owned(&a->elements[(size_t)idx]);
    stub_variant_clone(&a->elements[(size_t)idx], vblob(p_args[1]));
}

// ── Dictionary built-in method stubs ─────────────────────────────────────

static void s_dict_get(void *p_base, const void *const *p_args, void *r_ret, int32_t p_arg_count) {
    StubDict *d = dict_blob_get(p_base);
    if (!r_ret) return;
    std::string key = stub_variant_to_key(vblob(p_args[0]));
    StubVariant *found = d ? d->find(key) : nullptr;
    if (found) {
        stub_variant_clone(vblob(r_ret), found);
    } else if (p_arg_count >= 2) {
        stub_variant_clone(vblob(r_ret), vblob(p_args[1]));
    } else {
        s_variant_new_nil(r_ret);
    }
}

static void s_dict_has(void *p_base, const void *const *p_args, void *r_ret, int32_t) {
    StubDict *d = dict_blob_get(p_base);
    std::string key = stub_variant_to_key(vblob(p_args[0]));
    if (r_ret) *(uint8_t *)r_ret = (d && d->find(key)) ? 1 : 0;
}

static void s_dict_keys(void *p_base, const void *const *, void *r_ret, int32_t) {
    StubDict *d = dict_blob_get(p_base);
    if (!r_ret) return;
    StubArray *a = new StubArray();
    if (d) {
        for (const auto &e : d->entries) {
            StubVariant kv; stub_variant_clone(&kv, &e.key);
            a->elements.push_back(kv);
        }
    }
    arr_blob_set(r_ret, a);
}

static void s_dict_size(void *p_base, const void *const *, void *r_ret, int32_t) {
    StubDict *d = dict_blob_get(p_base);
    if (r_ret) *(int64_t *)r_ret = d ? (int64_t)d->entries.size() : 0LL;
}

static void s_dict_is_empty(void *p_base, const void *const *, void *r_ret, int32_t) {
    StubDict *d = dict_blob_get(p_base);
    if (r_ret) *(uint8_t *)r_ret = (!d || d->entries.empty()) ? 1 : 0;
}

// ── String built-in method stubs ─────────────────────────────────────────

static void s_string_is_empty(void *p_base, const void *const *, void *r_ret, int32_t) {
    std::u32string *s = str_blob_get(p_base);
    if (r_ret) *(uint8_t *)r_ret = (!s || s->empty()) ? 1 : 0;
}

static void s_string_length(void *p_base, const void *const *, void *r_ret, int32_t) {
    std::u32string *s = str_blob_get(p_base);
    if (r_ret) *(int64_t *)r_ret = s ? (int64_t)s->size() : 0LL;
}

// ── PackedByteArray built-in method stubs ────────────────────────────────
// Builtin-method ABI: void f(void *p_base, const void *const *p_args,
//                            void *r_return, int32_t p_arg_count)
// int parameters are PtrToArg<int64_t>-encoded (8-byte little-endian); the
// PackedByteArray methods set/push_back/get treat the value as a byte.

static void s_pba_resize(void *p_base, const void *const *p_args, void *r_ret, int32_t) {
    std::vector<uint8_t> *v = pba_blob_get(p_base);
    int64_t new_size = p_args ? *(const int64_t *)p_args[0] : 0;
    if (v && new_size >= 0) v->resize((size_t)new_size, 0);
    if (r_ret) *(int64_t *)r_ret = 0; // Error::OK
}

static void s_pba_size(void *p_base, const void *const *, void *r_ret, int32_t) {
    std::vector<uint8_t> *v = pba_blob_get(p_base);
    if (r_ret) *(int64_t *)r_ret = v ? (int64_t)v->size() : 0LL;
}

static void s_pba_is_empty(void *p_base, const void *const *, void *r_ret, int32_t) {
    std::vector<uint8_t> *v = pba_blob_get(p_base);
    if (r_ret) *(uint8_t *)r_ret = (!v || v->empty()) ? 1 : 0;
}

static void s_pba_set(void *p_base, const void *const *p_args, void *, int32_t) {
    std::vector<uint8_t> *v = pba_blob_get(p_base);
    if (!v || !p_args) return;
    int64_t idx = *(const int64_t *)p_args[0];
    int64_t val = *(const int64_t *)p_args[1];
    if (idx < 0 || (size_t)idx >= v->size()) return;
    (*v)[(size_t)idx] = (uint8_t)val;
}

static void s_pba_push_back(void *p_base, const void *const *p_args, void *r_ret, int32_t) {
    std::vector<uint8_t> *v = pba_blob_get(p_base);
    if (v && p_args) v->push_back((uint8_t)*(const int64_t *)p_args[0]);
    if (r_ret) *(uint8_t *)r_ret = 1; // success
}

static void s_pba_get(void *p_base, const void *const *p_args, void *r_ret, int32_t) {
    std::vector<uint8_t> *v = pba_blob_get(p_base);
    if (!r_ret) return;
    int64_t idx = (v && p_args) ? *(const int64_t *)p_args[0] : -1;
    *(int64_t *)r_ret = (v && idx >= 0 && (size_t)idx < v->size()) ? (int64_t)(*v)[(size_t)idx] : 0LL;
}

static void s_pba_clear(void *p_base, const void *const *, void *, int32_t) {
    std::vector<uint8_t> *v = pba_blob_get(p_base);
    if (v) v->clear();
}

// ── Dispatch ──────────────────────────────────────────────────────────────

static PFNBuiltin s_variant_get_ptr_builtin_method(uint32_t p_type, const void *p_method_name, int64_t /*p_hash*/) {
    // p_method_name is a StringName blob (points to an interned std::string*)
    const std::string *sn = sn_blob_get(p_method_name);
    if (!sn) return s_noop_builtin;
    std::string_view name = *sn;

    if (p_type == SVT_ARRAY) {
        if (name == "push_back" || name == "append") return s_array_push_back;
        if (name == "size")     return s_array_size;
        if (name == "is_empty") return s_array_is_empty;
        if (name == "clear")    return s_array_clear;
        if (name == "get")      return s_array_get;
        if (name == "set")      return s_array_set;
    }
    if (p_type == SVT_DICTIONARY) {
        if (name == "get")      return s_dict_get;
        if (name == "has")      return s_dict_has;
        if (name == "keys")     return s_dict_keys;
        if (name == "size")     return s_dict_size;
        if (name == "is_empty") return s_dict_is_empty;
    }
    if (p_type == SVT_STRING) {
        if (name == "is_empty") return s_string_is_empty;
        if (name == "length")   return s_string_length;
    }
    if (p_type == SVT_PACKED_BYTE_ARRAY) {
        if (name == "resize")    return s_pba_resize;
        if (name == "size")      return s_pba_size;
        if (name == "is_empty")  return s_pba_is_empty;
        if (name == "set")       return s_pba_set;
        if (name == "push_back" || name == "append") return s_pba_push_back;
        if (name == "get")       return s_pba_get;
        if (name == "clear")     return s_pba_clear;
    }
    return s_noop_builtin;
}

// ── Variant keyed access (forwarded to dictionary) ────────────────────────

static void s_variant_get_keyed(const void *p_self, const void *p_key, void *r_value, uint8_t *r_valid) {
    const StubVariant *v = vblob(p_self);
    if (v->type_id != SVT_DICTIONARY) { if (r_valid) *r_valid = 0; return; }
    StubDict *d = (StubDict *)v->data.as_ptr;
    std::string key = stub_variant_to_key(vblob(p_key));
    StubVariant *found = d ? d->find(key) : nullptr;
    if (found) {
        stub_variant_clone(vblob(r_value), found);
        if (r_valid) *r_valid = 1;
    } else {
        s_variant_new_nil(r_value);
        if (r_valid) *r_valid = 0;
    }
}

static void s_variant_set_keyed(void *p_self, const void *p_key, const void *p_value, uint8_t *r_valid) {
    StubVariant *v = vblob(p_self);
    if (v->type_id != SVT_DICTIONARY) { if (r_valid) *r_valid = 0; return; }
    StubDict *d = (StubDict *)v->data.as_ptr;
    if (!d) { if (r_valid) *r_valid = 0; return; }
    std::string key = stub_variant_to_key(vblob(p_key));
    StubVariant *slot = d->insert_or_find(key, *vblob(p_key));
    stub_variant_free_owned(slot);
    stub_variant_clone(slot, vblob(p_value));
    if (r_valid) *r_valid = 1;
}

static uint32_t s_variant_has_key(const void *p_self, const void *p_key, uint8_t *r_valid) {
    const StubVariant *v = vblob(p_self);
    if (v->type_id != SVT_DICTIONARY) { if (r_valid) *r_valid = 0; return 0; }
    StubDict *d = (StubDict *)v->data.as_ptr;
    std::string key = stub_variant_to_key(vblob(p_key));
    uint32_t found = (d && d->find(key)) ? 1 : 0;
    if (r_valid) *r_valid = 1;
    return found;
}

static void s_variant_call(void *p_self, const void *p_method, const void *const *p_args, int32_t p_arg_count, void *r_return, void *r_error) {
    (void)p_self; (void)p_method; (void)p_args; (void)p_arg_count; (void)r_error;
    if (r_return) s_variant_new_nil(r_return);
}

static void s_variant_call_static(uint32_t p_type, const void *p_method, const void *const *p_args, int32_t p_arg_count, void *r_return, void *r_error) {
    (void)p_type; (void)p_method; (void)p_args; (void)p_arg_count; (void)r_error;
    if (r_return) s_variant_new_nil(r_return);
}

static void s_variant_construct(uint32_t p_type, void *r_base, const void *const *p_args, int32_t p_argc, void *r_error) {
    (void)r_error;
    PFNCtor ctor = s_variant_get_ptr_constructor(p_type, p_argc > 0 ? 1 : 0);
    ctor(r_base, p_args);
}

static void s_variant_duplicate(const void *p_self, void *r_ret, uint8_t /*deep*/) {
    stub_variant_clone(vblob(r_ret), vblob(p_self));
}

static void s_variant_get(const void *p_self, const void *p_key, void *r_ret, uint8_t *r_valid) {
    s_variant_get_keyed(p_self, p_key, r_ret, r_valid);
}
static void s_variant_set(void *p_self, const void *p_key, const void *p_value, uint8_t *r_valid) {
    s_variant_set_keyed(p_self, p_key, p_value, r_valid);
}
static void s_variant_get_named(const void *p_self, const void *p_name, void *r_ret, uint8_t *r_valid) {
    s_variant_get_keyed(p_self, p_name, r_ret, r_valid);
}
static void s_variant_set_named(void *p_self, const void *p_name, const void *p_value, uint8_t *r_valid) {
    s_variant_set_keyed(p_self, p_name, p_value, r_valid);
}
static void s_variant_get_indexed(const void *p_self, int64_t p_index, void *r_ret, uint8_t *r_oob) {
    // For Array access by index
    const StubVariant *v = vblob(p_self);
    if (v->type_id == SVT_ARRAY) {
        StubArray *a = (StubArray *)v->data.as_ptr;
        if (a && p_index >= 0 && (size_t)p_index < a->elements.size()) {
            stub_variant_clone(vblob(r_ret), &a->elements[(size_t)p_index]);
            if (r_oob) *r_oob = 0;
            return;
        }
    }
    s_variant_new_nil(r_ret);
    if (r_oob) *r_oob = 1;
}
static void s_variant_set_indexed(void *p_self, int64_t p_index, const void *p_value, uint8_t *r_valid, uint8_t *r_oob) {
    StubVariant *v = vblob(p_self);
    if (v->type_id == SVT_ARRAY) {
        StubArray *a = (StubArray *)v->data.as_ptr;
        if (a && p_index >= 0 && (size_t)p_index < a->elements.size()) {
            stub_variant_free_owned(&a->elements[(size_t)p_index]);
            stub_variant_clone(&a->elements[(size_t)p_index], vblob(p_value));
            if (r_valid) *r_valid = 1;
            if (r_oob) *r_oob = 0;
            return;
        }
    }
    if (r_valid) *r_valid = 0;
    if (r_oob) *r_oob = 1;
}

static uint8_t s_variant_can_convert(uint32_t /*p_from*/, uint32_t /*p_to*/) { return 1; }
static uint8_t s_variant_can_convert_strict(uint32_t p_from, uint32_t p_to) { return p_from == p_to ? 1 : 0; }
static void s_variant_evaluate(uint32_t /*op*/, const void */*a*/, const void */*b*/, void *r_ret, uint8_t *r_valid) {
    if (r_ret) s_variant_new_nil(r_ret);
    if (r_valid) *r_valid = 0;
}
static int64_t s_variant_recursive_hash(const void *p_self, int64_t /*seed*/) { return s_variant_hash(p_self); }
static void s_variant_has_member(uint32_t /*type*/, const void */*member*/, uint8_t *r_has) { if (r_has) *r_has = 0; }
static void s_variant_has_method(const void */*self*/, const void */*method*/, uint8_t *r_has) { if (r_has) *r_has = 0; }
static void s_variant_get_object_instance_id(const void */*self*/, uint64_t *r_id) { if (r_id) *r_id = 0; }
static void s_variant_get_constant_value(uint32_t /*type*/, const void */*name*/, void *r_ret) { if (r_ret) s_variant_new_nil(r_ret); }
static void *s_variant_get_ptr_setter(uint32_t, const void *) { return nullptr; }
static void *s_variant_get_ptr_getter(uint32_t, const void *) { return nullptr; }
static void *s_variant_get_ptr_indexed_setter(uint32_t) { return nullptr; }
static void *s_variant_get_ptr_indexed_getter(uint32_t) { return nullptr; }
static void *s_variant_get_ptr_keyed_setter(uint32_t) { return nullptr; }
static void *s_variant_get_ptr_keyed_getter(uint32_t) { return nullptr; }
static uint32_t s_variant_get_ptr_keyed_checker(const void *, const void *) { return 0; }

// Operator evaluator: void f(const void *p_left, const void *p_right, void *r_result)
// r_result is a pre-constructed (default) blob of the same type as p_left.
// OP_ADD=0, TYPE_STRING=4.
static void s_noop_op_eval(const void *, const void *, void *) {}
static void s_string_op_add(const void *p_left, const void *p_right, void *r_result) {
    const std::u32string *L = str_blob_get(p_left);
    const std::u32string *R = str_blob_get(p_right);
    delete str_blob_get(r_result);
    std::u32string *res = new std::u32string();
    if (L) *res = *L;
    if (R) *res += *R;
    str_blob_set(r_result, res);
}
static void *s_variant_get_ptr_operator_evaluator(uint32_t p_op, uint32_t p_type_a, uint32_t p_type_b) {
    // OP_ADD=0, TYPE_STRING=4
    if (p_op == 0 && p_type_a == 4 && p_type_b == 4) return (void *)s_string_op_add;
    // Return a real (but no-op) function for all other operators so callers never
    // dereference a null function pointer.
    return (void *)s_noop_op_eval;
}

// Utility function: void f(void *r_return, const void *const *p_args, int32_t p_arg_count)
// Return a real noop to prevent null-pointer dereference when godot-cpp calls utility
// functions that the fuzz targets don't exercise.
static void s_noop_utility(void *, const void *const *, int32_t) {}
static void *s_variant_get_ptr_utility_function(const void *, int64_t) {
    return (void *)s_noop_utility;
}

static void *s_variant_get_ptr_internal_getter(uint32_t, uint32_t) { return nullptr; }
static void s_variant_iter_init(const void *, void *, uint8_t *r_valid) { if (r_valid) *r_valid = 0; }
static void s_variant_iter_next(const void *, void *, uint8_t *r_valid) { if (r_valid) *r_valid = 0; }
static void s_variant_iter_get(const void *, void *, void *r_ret, uint8_t *r_valid) {
    if (r_ret) s_variant_new_nil(r_ret);
    if (r_valid) *r_valid = 0;
}

} // extern "C"

// ─── Exported function pointer table ─────────────────────────────────────
void *stub_variant_new_nil_ptr                        = (void *)s_variant_new_nil;
void *stub_variant_new_copy_ptr                       = (void *)s_variant_new_copy;
void *stub_variant_destroy_ptr                        = (void *)s_variant_destroy;
void *stub_variant_get_type_ptr                       = (void *)s_variant_get_type;
void *stub_variant_get_type_name_ptr                  = (void *)s_variant_get_type_name;
void *stub_variant_booleanize_ptr                     = (void *)s_variant_booleanize;
void *stub_variant_hash_ptr                           = (void *)s_variant_hash;
void *stub_variant_hash_compare_ptr                   = (void *)s_variant_hash_compare;
void *stub_variant_stringify_ptr                      = (void *)s_variant_stringify;
void *stub_variant_call_ptr                           = (void *)s_variant_call;
void *stub_variant_call_static_ptr                    = (void *)s_variant_call_static;
void *stub_variant_construct_ptr                      = (void *)s_variant_construct;
void *stub_variant_duplicate_ptr                      = (void *)s_variant_duplicate;
void *stub_variant_get_ptr                            = (void *)s_variant_get;
void *stub_variant_set_ptr                            = (void *)s_variant_set;
void *stub_variant_get_named_ptr                      = (void *)s_variant_get_named;
void *stub_variant_set_named_ptr                      = (void *)s_variant_set_named;
void *stub_variant_get_indexed_ptr                    = (void *)s_variant_get_indexed;
void *stub_variant_set_indexed_ptr                    = (void *)s_variant_set_indexed;
void *stub_variant_get_keyed_ptr                      = (void *)s_variant_get_keyed;
void *stub_variant_set_keyed_ptr                      = (void *)s_variant_set_keyed;
void *stub_variant_has_key_ptr                        = (void *)s_variant_has_key;
void *stub_variant_can_convert_ptr                    = (void *)s_variant_can_convert;
void *stub_variant_can_convert_strict_ptr             = (void *)s_variant_can_convert_strict;
void *stub_variant_evaluate_ptr                       = (void *)s_variant_evaluate;
void *stub_variant_recursive_hash_ptr                 = (void *)s_variant_recursive_hash;
void *stub_variant_has_member_ptr                     = (void *)s_variant_has_member;
void *stub_variant_has_method_ptr                     = (void *)s_variant_has_method;
void *stub_variant_get_object_instance_id_ptr         = (void *)s_variant_get_object_instance_id;
void *stub_variant_get_constant_value_ptr             = (void *)s_variant_get_constant_value;
void *stub_variant_iter_init_ptr                      = (void *)s_variant_iter_init;
void *stub_variant_iter_next_ptr                      = (void *)s_variant_iter_next;
void *stub_variant_iter_get_ptr                       = (void *)s_variant_iter_get;
void *stub_variant_get_ptr_setter_ptr                 = (void *)s_variant_get_ptr_setter;
void *stub_variant_get_ptr_getter_ptr                 = (void *)s_variant_get_ptr_getter;
void *stub_variant_get_ptr_indexed_setter_ptr         = (void *)s_variant_get_ptr_indexed_setter;
void *stub_variant_get_ptr_indexed_getter_ptr         = (void *)s_variant_get_ptr_indexed_getter;
void *stub_variant_get_ptr_keyed_setter_ptr           = (void *)s_variant_get_ptr_keyed_setter;
void *stub_variant_get_ptr_keyed_getter_ptr           = (void *)s_variant_get_ptr_keyed_getter;
void *stub_variant_get_ptr_keyed_checker_ptr          = (void *)s_variant_get_ptr_keyed_checker;
void *stub_variant_get_ptr_operator_evaluator_ptr     = (void *)s_variant_get_ptr_operator_evaluator;
void *stub_variant_get_ptr_utility_function_ptr       = (void *)s_variant_get_ptr_utility_function;
void *stub_variant_get_ptr_internal_getter_ptr        = (void *)s_variant_get_ptr_internal_getter;
void *stub_variant_get_ptr_constructor_ptr            = (void *)s_variant_get_ptr_constructor;
void *stub_variant_get_ptr_destructor_ptr             = (void *)s_variant_get_ptr_destructor;
void *stub_variant_get_ptr_builtin_method_ptr         = (void *)s_variant_get_ptr_builtin_method;
void *stub_get_variant_from_type_constructor_ptr      = (void *)s_get_variant_from_type_constructor;
void *stub_get_variant_to_type_constructor_ptr        = (void *)s_get_variant_to_type_constructor;
