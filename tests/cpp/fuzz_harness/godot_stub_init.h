#ifndef GODOT_STUB_INIT_H
#define GODOT_STUB_INIT_H

// GDExtension stub harness — Phase 3 fuzz testing infrastructure.
//
// Provides a minimal implementation of all 176 GDExtension interface functions
// so that godot::String, godot::Dictionary, godot::Variant, godot::Array, and
// related Godot types work in a standalone exe without a real Godot engine.
//
// Usage in a fuzz target:
//
//   #include "godot_stub_init.h"
//
//   extern "C" int LLVMFuzzerInitialize(int *, char ***) {
//       godot_stub::init();
//       return 0;
//   }
//
//   extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size) {
//       // godot::String, Dictionary, Variant etc. are safe to use here.
//       return 0;
//   }
//
// The harness is not thread-safe. All fuzz targets are single-threaded.
//
// Implementation notes:
//   - godot::String: each instance owns a heap-allocated std::u32string.
//     The 8-byte godot::String opaque blob stores a `std::u32string *`.
//   - godot::StringName: interned in a global std::unordered_set<std::string>.
//   - godot::Dictionary: heap-allocated StubDict wrapping an ordered
//     std::vector<Entry> (each Entry holds the string key-repr, key, value).
//   - godot::Array: heap-allocated StubArray wrapping std::deque<StubVariant>.
//   - godot::Variant: tagged union over NIL / bool / int64 / float64 /
//     String* / Dictionary* / Array*.
//   - Memory: malloc / realloc / free.
//   - All 85 class-system / engine functions: no-op stubs.

namespace godot_stub {

// Initialize the stub harness. Calls GDExtensionBinding::init() with the
// stub dispatch table. Must be called exactly once before any Godot type is
// used. Calling it from LLVMFuzzerInitialize is the recommended pattern.
void init();

// Tear down the stub harness. Optional; the process exits cleanly either way.
void shutdown();

} // namespace godot_stub

#endif // GODOT_STUB_INIT_H
