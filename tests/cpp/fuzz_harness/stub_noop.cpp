// stub_noop.cpp — A single no-op function used as the fallback stub for all
// GDExtension interface functions that are not needed by our fuzz targets
// (class registration, object binding, script instances, editor APIs, etc.).

#include "stub_types.h"

extern "C" {

static void s_stub_noop() {}

// get_godot_version / get_godot_version2
// Return version 4.6.0 so GDExtensionBinding::init() passes the compatibility check.
// These structs are layout-compatible with GDExtensionGodotVersion{2} from the header;
// we avoid including the engine header here to keep the harness self-contained.

struct StubGDVersion {
    uint32_t major;
    uint32_t minor;
    uint32_t patch;
    const char *string;
};

struct StubGDVersion2 {
    uint32_t    major;
    uint32_t    minor;
    uint32_t    patch;
    uint32_t    hex;
    const char *status;
    const char *build;
    const char *hash;
    uint64_t    timestamp;
    const char *string;
};

static void s_get_godot_version(StubGDVersion *r) {
    if (!r) return;
    r->major  = 4;
    r->minor  = 6;
    r->patch  = 0;
    r->string = "4.6.0.stable";
}

static void s_get_godot_version2(StubGDVersion2 *r) {
    if (!r) return;
    r->major     = 4;
    r->minor     = 6;
    r->patch     = 0;
    r->hex       = 0x040600;
    r->status    = "stable";
    r->build     = "official";
    r->hash      = "0000000000000000000000000000000000000000";
    r->timestamp = 0;
    r->string    = "Godot v4.6.0.stable.official";
}

} // extern "C"

void *stub_noop_ptr             = (void *)s_stub_noop;
void *stub_get_godot_version_ptr  = (void *)s_get_godot_version;
void *stub_get_godot_version2_ptr = (void *)s_get_godot_version2;
