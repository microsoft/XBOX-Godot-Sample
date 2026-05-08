// Pure HRESULT formatting helpers extracted from gdk_result.cpp.
//
// These free functions live in the gdk_internal:: namespace and are
// deliberately kept free of Godot Object/Signal/Ref dependencies so they
// can be exercised from the doctest target without dragging in the full
// GDExtension surface. See spec/testing-strategy.md ("C++ test scope rules"):
// production and tests share the same code; never duplicate logic for tests.
//
// Per-addon copy. The PlayFab addon ships an analogous header
// (playfab_result_codes_internal.h) with identical semantics. They are kept
// disjoint per addon to avoid cross-addon include paths and ODR risk.
#ifndef GDK_RESULT_CODES_INTERNAL_H
#define GDK_RESULT_CODES_INTERNAL_H

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>

#include <cstddef>

#include <godot_cpp/variant/string.hpp>

namespace gdk_internal {

// Minimum buffer size required by format_hresult_hex (10 chars + null).
inline constexpr std::size_t HRESULT_HEX_BUFFER_SIZE = 11;

// Pure: format an HRESULT as the upper-case hex literal "0xXXXXXXXX" into the
// supplied caller-owned buffer. buffer_size must be >= HRESULT_HEX_BUFFER_SIZE.
// Returns out_buffer for chaining. If buffer_size is too small the buffer is
// left empty (out_buffer[0] == '\0') and out_buffer is still returned.
// No allocation, no Godot types, safe to call from any thread.
char *format_hresult_hex(HRESULT p_hresult, char *out_buffer, std::size_t buffer_size) noexcept;

// Convenience wrapper that returns the formatted hex literal as a godot::String.
// This is the public-API-shape helper that GDKResult::format_hresult forwards to.
godot::String format_hresult_string(HRESULT p_hresult);

// Build the standard "<action> (HRESULT 0xXXXXXXXX)" message string used by
// GDKResult::hresult_error. If action is empty the leading "<action> " is omitted.
godot::String format_hresult_message(const godot::String &p_action, HRESULT p_hresult);

// Fallback resolver for the "code" field on a GDKResult: returns p_provided
// when non-empty, else format_hresult_string(p_hresult).
godot::String code_or_format_hresult(const godot::String &p_provided, HRESULT p_hresult);

} // namespace gdk_internal

#endif // GDK_RESULT_CODES_INTERNAL_H
