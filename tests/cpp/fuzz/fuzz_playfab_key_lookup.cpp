// Phase 2 fuzz target: playfab_internal::match_request_key
//
// Exercises the pure key-selection helper with arbitrary byte inputs.
// No Godot types are used; no harness initialization is required.
//
// Input layout (best-effort split on NUL bytes):
//   [primary\0fallback\0key0\0key1\0...\0keyN]
// If the input has fewer than 2 NUL-separated fields, the missing segments
// default to empty strings. This layout gives the fuzzer maximum freedom to
// vary all inputs independently.

#include <cstddef>
#include <cstdint>
#include <cstring>
#include <vector>

#include "playfab_request_key.h"

extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size) {
    if (size == 0) {
        return 0;
    }

    // Split the input into NUL-terminated segments.
    std::vector<const char *> segments;
    const char *start = reinterpret_cast<const char *>(data);
    const char *end = start + size;
    const char *seg = start;

    for (const char *p = start; p < end; ++p) {
        if (*p == '\0') {
            segments.push_back(seg);
            seg = p + 1;
        }
    }
    // Last segment (may not be NUL-terminated — skip it to avoid OOB read).
    // We only use segments that are properly NUL-terminated by the scan above.

    const char *primary  = segments.size() > 0 ? segments[0] : "";
    const char *fallback = segments.size() > 1 ? segments[1] : "";

    // Remaining segments are the key array.
    const size_t key_count = segments.size() > 2 ? segments.size() - 2 : 0;
    const char *const *keys = key_count > 0 ? segments.data() + 2 : nullptr;

    playfab_internal::match_request_key(keys, key_count, primary, fallback);
    return 0;
}
