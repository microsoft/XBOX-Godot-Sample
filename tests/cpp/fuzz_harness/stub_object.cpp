// stub_object.cpp — Minimal Object / RefCounted lifecycle stubs.
//
// Enables Ref<T>::instantiate() to work for any T that extends RefCounted
// (e.g. GDKResult, PlayFabResult) without a live Godot engine.
//
// Design:
//   classdb_construct_object2  →  allocates a StubObject on the heap;
//                                  its address is the _owner handle.
//   classdb_get_method_bind    →  returns a tagged sentinel for the three
//                                  RefCounted lifecycle methods; returns
//                                  MB_GENERIC for everything else (non-null
//                                  so CHECK_METHOD_BIND_RET passes).
//   object_method_bind_ptrcall →  dispatches on the sentinel tag to track
//                                  refcount and write the correct bool return.
//   object_destroy             →  deletes the StubObject allocated above.
//
// All other object functions (object_set_instance, _binding, etc.) are
// lightweight stubs that do just enough to keep Wrapped::Wrapped() happy.
//
// Thread safety: each fuzz iteration runs sequentially on one thread;
// no locking is needed.

#include "stub_types.h"

#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <string>

// ─── Internal object representation ───────────────────────────────────────

struct StubObject {
    int32_t  refcount     = 0;
    void    *instance_ptr = nullptr;
};

// ─── Method-bind sentinel tags ─────────────────────────────────────────────
// Small integer values cast to void*.  Cannot overlap with real function
// pointers (OS never maps pages at these addresses on x64).

static void *const MB_INIT_REF    = reinterpret_cast<void *>(static_cast<uintptr_t>(0x10));
static void *const MB_REFERENCE   = reinterpret_cast<void *>(static_cast<uintptr_t>(0x11));
static void *const MB_UNREFERENCE = reinterpret_cast<void *>(static_cast<uintptr_t>(0x12));
// Non-null sentinel for all other method binds (CHECK_METHOD_BIND_RET passes).
static void *const MB_GENERIC     = reinterpret_cast<void *>(static_cast<uintptr_t>(0x01));

// ─── Stub implementations ─────────────────────────────────────────────────

extern "C" {

// classdb_construct_object / classdb_construct_object2
// Allocates a StubObject.  The raw pointer IS the _owner handle.
static void *s_classdb_construct_object(const void * /*class_name*/) {
    return new StubObject{};
}

static void *s_classdb_construct_object2(const void * /*class_name*/) {
    return new StubObject{};
}

// object_set_instance — record the GDExtension C++ instance pointer.
static void s_object_set_instance(void *owner,
                                   const void * /*ext_class_name*/,
                                   void *instance_ptr) {
    if (owner) {
        reinterpret_cast<StubObject *>(owner)->instance_ptr = instance_ptr;
    }
}

// object_set_instance_binding — no-op; we don't need binding callbacks.
static void s_object_set_instance_binding(void * /*owner*/,
                                           void * /*token*/,
                                           void * /*binding*/,
                                           const void * /*callbacks*/) {}

// object_get_instance_binding — return the stored C++ instance pointer.
static void *s_object_get_instance_binding(void *owner,
                                            void * /*token*/,
                                            const void * /*callbacks*/) {
    if (!owner) return nullptr;
    return reinterpret_cast<StubObject *>(owner)->instance_ptr;
}

// object_destroy — called by memdelete<T> for Wrapped subclasses.
static void s_object_destroy(void *owner) {
    delete reinterpret_cast<StubObject *>(owner);
}

// classdb_get_method_bind — return a method-bind sentinel.
// p_methodname is a GDExtensionConstStringNamePtr, which in our stub is
// a pointer to the StringName's opaque blob (8 bytes containing a std::string*
// interned in the global StringName pool).  Same layout as sn_blob_get().
static void *s_classdb_get_method_bind(const void * /*class_name*/,
                                        const void *method_name,
                                        int64_t /*hash*/) {
    if (!method_name) return MB_GENERIC;
    // Double-dereference: method_name → blob → std::string*
    const std::string *sn_ptr =
        *reinterpret_cast<std::string * const *>(method_name);
    if (!sn_ptr) return MB_GENERIC;
    const std::string &name = *sn_ptr;
    if (name == "init_ref")    return MB_INIT_REF;
    if (name == "reference")   return MB_REFERENCE;
    if (name == "unreference") return MB_UNREFERENCE;
    return MB_GENERIC;
}

// object_method_bind_ptrcall — dispatch refcount ops; no-op for all others.
// r_ret for RefCounted bool methods points to an int8_t value-initialized to 0.
static void s_object_method_bind_ptrcall(void *method_bind,
                                          void *owner,
                                          const void * /*args*/,
                                          void *r_ret) {
    if (method_bind == MB_INIT_REF || method_bind == MB_REFERENCE) {
        if (owner) {
            reinterpret_cast<StubObject *>(owner)->refcount++;
        }
        if (r_ret) *reinterpret_cast<int8_t *>(r_ret) = 1;

    } else if (method_bind == MB_UNREFERENCE) {
        bool was_last = false;
        if (owner) {
            auto *obj = reinterpret_cast<StubObject *>(owner);
            was_last = (--obj->refcount <= 0);
        }
        if (r_ret) *reinterpret_cast<int8_t *>(r_ret) = was_last ? 1 : 0;
    }
    // MB_GENERIC and unknown tags: no-op (r_ret stays 0 / void)
}

} // extern "C"

// ─── Exported function-pointer variables ──────────────────────────────────

void *stub_classdb_construct_object_ptr      = reinterpret_cast<void *>(s_classdb_construct_object);
void *stub_classdb_construct_object2_ptr     = reinterpret_cast<void *>(s_classdb_construct_object2);
void *stub_object_set_instance_ptr           = reinterpret_cast<void *>(s_object_set_instance);
void *stub_object_set_instance_binding_ptr   = reinterpret_cast<void *>(s_object_set_instance_binding);
void *stub_object_get_instance_binding_ptr   = reinterpret_cast<void *>(s_object_get_instance_binding);
void *stub_object_destroy_ptr                = reinterpret_cast<void *>(s_object_destroy);
void *stub_classdb_get_method_bind_ptr       = reinterpret_cast<void *>(s_classdb_get_method_bind);
void *stub_object_method_bind_ptrcall_ptr    = reinterpret_cast<void *>(s_object_method_bind_ptrcall);
