#ifndef GODOT_PLAYFAB_REQUEST_KEY_H
#define GODOT_PLAYFAB_REQUEST_KEY_H

// Pure C++ helper for PlayFab request-field key matching.
//
// `get_request_value` in playfab_api_helpers accepts godot::Dictionary
// requests that callers may populate with either the PascalCase field name
// (matching the PlayFab REST schema) or the snake_case Godot-style name.
// This module extracts the pure key-matching logic into a form that:
//
//   1. Has no Godot or GDK dependencies.
//   2. Can be exercised from the doctest suite and the libFuzzer target.
//   3. Is noexcept throughout so it is safe to call from async paths.
//
// Production code in playfab_api_helpers.cpp delegates to match_request_key
// for the key-selection step and then performs the Godot-side lookup by
// position/name.

#include <cstddef>

namespace playfab_internal {

// match_request_key
//
// Given a flat array of `key_count` C-string keys present in a request dict,
// searches for `primary` (the snake_case name) first, then `fallback` (the
// PascalCase name). Returns:
//
//   0  — primary matched
//   1  — fallback matched (primary was absent or null)
//  -1  — neither matched (or both primary and fallback are null)
//
// Null pointers are treated as absent (never match). An empty key array
// always returns -1. The function is noexcept and has no allocations.
int match_request_key(
        const char *const *keys, std::size_t key_count,
        const char *primary,
        const char *fallback) noexcept;

} // namespace playfab_internal

#endif // GODOT_PLAYFAB_REQUEST_KEY_H
