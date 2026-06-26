#include "playfab_request_key.h"

#include <cstring>

namespace playfab_internal {

int match_request_key(
        const char *const *keys, std::size_t key_count,
        const char *primary,
        const char *fallback) noexcept {
    if (keys == nullptr || key_count == 0) {
        return -1;
    }

    bool found_primary = false;
    bool found_fallback = false;

    for (std::size_t i = 0; i < key_count; ++i) {
        if (keys[i] == nullptr) {
            continue;
        }
        if (!found_primary && primary != nullptr && std::strcmp(keys[i], primary) == 0) {
            found_primary = true;
        }
        if (!found_fallback && fallback != nullptr && std::strcmp(keys[i], fallback) == 0) {
            found_fallback = true;
        }
        if (found_primary) {
            // primary takes precedence; no need to keep scanning for it
            break;
        }
    }

    if (found_primary) {
        return 0;
    }
    if (found_fallback) {
        return 1;
    }
    return -1;
}

} // namespace playfab_internal
