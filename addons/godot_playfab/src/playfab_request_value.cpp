// playfab_request_value.cpp — Implementation of get_request_value.
//
// See playfab_request_value.h for the contract. This translation unit has no
// dependency on the GDK or PlayFab SDK headers so it can be compiled as part
// of the standalone fuzz harness.

#include "playfab_request_value.h"

#include <godot_cpp/variant/string_name.hpp>

namespace godot {
namespace playfab_api {

bool get_request_value(const Dictionary &p_request,
                       const char *p_field_name,
                       const char *p_snake_name,
                       Variant *r_value) {
    if (r_value == nullptr) {
        return false;
    }

    // Priority: snake_case name first, PascalCase fallback. The pure equivalent
    // of this selection logic is playfab_internal::match_request_key, which
    // is fuzz-tested in tests/cpp/fuzz/fuzz_playfab_key_lookup.cpp and
    // doctest-covered in tests/cpp/request_key/test_playfab_request_key.cpp.
    // Production code uses O(1) hash lookups via StringName to avoid the linear
    // scan in match_request_key.
    const StringName snake_name(p_snake_name);
    if (p_request.has(snake_name)) {
        *r_value = p_request[snake_name];
        return true;
    }

    const StringName field_name(p_field_name);
    if (p_request.has(field_name)) {
        *r_value = p_request[field_name];
        return true;
    }

    return false;
}

} // namespace playfab_api
} // namespace godot
