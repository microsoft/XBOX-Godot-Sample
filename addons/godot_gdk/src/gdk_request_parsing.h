// gdk_request_parsing.h — Engine-free seam for GDK request-input parsing.
//
// Several GDK service wrappers (title storage, profile, game UI) share the same
// input-validation helpers: parse a decimal XUID, bounds-check a uint32 field,
// copy a UTF-8 string into a fixed native buffer, and convert between
// PackedByteArray and std::vector<uint8_t>. These operate on caller-supplied
// (ultimately script-supplied) values and touch raw memory (strtoull, memcpy
// into fixed buffers, byte-buffer round-trips), so they are the addon's
// memory-relevant request surface.
//
// The helpers use only godot-cpp built-in types plus GDKResult (which is itself
// SDK-free), with no dependency on the Xbox GDK service headers, so they can be
// compiled into both the production addon and the standalone fuzz harness (see
// tests/cpp/fuzz/fuzz_gdk_request_parsing.cpp).
#ifndef GDK_REQUEST_PARSING_H
#define GDK_REQUEST_PARSING_H

#include <cstddef>
#include <cstdint>
#include <vector>

// gdk_result.h must precede the godot-cpp variant headers below: it includes
// <windows.h> (for HRESULT) which defines a CONNECT_DEFERRED macro, and the
// godot-cpp include chain it pulls in runs godot_cpp/core/defs.hpp's
// `#undef CONNECT_DEFERRED` *after* that macro is defined. Including a plain
// godot header (string.hpp / packed_byte_array.hpp) first would mark defs.hpp's
// include guard early, so its undef would be skipped and Object's
// CONNECT_DEFERRED enum would fail to parse.
#include "gdk_result.h"

#include <godot_cpp/variant/packed_byte_array.hpp>
#include <godot_cpp/variant/string.hpp>

namespace godot {
namespace gdk_request_parsing {

// Parse a decimal XUID string. Returns false on empty/whitespace input, a
// non-numeric or partially-numeric value, or strtoull range error. When
// p_reject_zero is true a parsed value of 0 is also rejected (the game-UI call
// path treats 0 as invalid; the title-storage and profile paths accept it).
bool try_parse_xuid(const String &p_xuid, uint64_t *r_xuid, bool p_reject_zero);

// Validate that p_value fits in a non-negative 32-bit unsigned integer and
// write it to *r_value. Returns a GDKResult describing success or the failure.
Ref<GDKResult> parse_uint32(const String &p_name, int64_t p_value, uint32_t *r_value);

// Copy the UTF-8 encoding of p_value (including a trailing NUL) into the fixed
// buffer p_buffer of capacity p_buffer_size. Returns a GDKResult describing
// success, a null/zero-size buffer, or an over-length value.
Ref<GDKResult> copy_utf8_to_buffer(const String &p_value, char *p_buffer, size_t p_buffer_size, const String &p_field_name);

// Convert a byte vector to a PackedByteArray.
PackedByteArray to_packed_byte_array(const std::vector<uint8_t> &p_data);

// Convert a PackedByteArray to a byte vector.
std::vector<uint8_t> to_byte_vector(const PackedByteArray &p_data);

} // namespace gdk_request_parsing
} // namespace godot

#endif // GDK_REQUEST_PARSING_H
