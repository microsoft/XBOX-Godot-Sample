// playfab_request_value.h — Engine-free seam for the PlayFab request
// Dictionary field-lookup helper.
//
// get_request_value resolves a field from a GDScript request Dictionary by
// trying the snake_case key first and falling back to the PascalCase key.
// This is the same priority used by every PlayFab API service binding when
// reading caller-supplied request parameters.
//
// The function uses only godot-cpp built-in types (Dictionary, StringName,
// Variant) and has no dependency on the GDK or PlayFab SDK headers, which
// makes it suitable for the standalone fuzz harness.
//
// Per-addon copy: lives in godot_playfab/src rather than being shared with
// godot_gdk, per the repo convention that avoids cross-addon include paths.
#ifndef GODOT_PLAYFAB_REQUEST_VALUE_H
#define GODOT_PLAYFAB_REQUEST_VALUE_H

#include <godot_cpp/variant/dictionary.hpp>
#include <godot_cpp/variant/variant.hpp>

namespace godot {
namespace playfab_api {

// Resolve p_field_name or p_snake_name from p_request.
//
// Priority: snake_case (p_snake_name) is tried first; PascalCase
// (p_field_name) is the fallback. This mirrors the GDScript convention
// where callers may pass either casing style.
//
// Returns true and writes the matched value into *r_value if found.
// Returns false (and leaves *r_value unchanged) if neither key is present.
// Returns false immediately if r_value is nullptr.
bool get_request_value(const Dictionary &p_request,
                       const char *p_field_name,
                       const char *p_snake_name,
                       Variant *r_value);

} // namespace playfab_api
} // namespace godot

#endif // GODOT_PLAYFAB_REQUEST_VALUE_H
