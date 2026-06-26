// gdk_request_parsing.cpp — Implementation of the GDK request-input parsing
// seam.  See gdk_request_parsing.h for the contract.  This translation unit
// depends only on godot-cpp built-in types and GDKResult (no Xbox GDK service
// headers) so it can be compiled into both the production addon and the
// standalone fuzz harness.

#include "gdk_request_parsing.h"

#include <cerrno>
#include <cstdlib>
#include <cstring>

namespace godot {
namespace gdk_request_parsing {

bool try_parse_xuid(const String &p_xuid, uint64_t *r_xuid, bool p_reject_zero) {
    if (r_xuid == nullptr) {
        return false;
    }

    const String trimmed = p_xuid.strip_edges();
    if (trimmed.is_empty()) {
        return false;
    }

    // Defensive: a null buffer can never parse to a valid XUID. The original
    // per-call-site helpers passed get_data() straight to strtoull; guarding
    // here avoids UB without changing observable behavior for any non-empty
    // trimmed string. We deliberately do NOT reject an empty-content buffer
    // (first byte '\0') here: for a U+0000-prefixed value strtoull yields 0,
    // which game-UI (p_reject_zero) rejects below and which title-storage /
    // profile historically accepted as XUID 0 — matching each original helper.
    const CharString utf8 = trimmed.utf8();
    if (utf8.get_data() == nullptr) {
        return false;
    }

    char *end = nullptr;
    errno = 0;
    const unsigned long long parsed = std::strtoull(utf8.get_data(), &end, 10);
    if (errno != 0 || end == nullptr || *end != '\0') {
        return false;
    }
    if (p_reject_zero && parsed == 0) {
        return false;
    }

    *r_xuid = static_cast<uint64_t>(parsed);
    return true;
}

Ref<GDKResult> parse_uint32(const String &p_name, int64_t p_value, uint32_t *r_value) {
    if (r_value == nullptr) {
        return GDKResult::error_result(E_POINTER, "invalid_output", "Output storage is unavailable.");
    }
    if (p_value < 0 || p_value > static_cast<int64_t>(UINT32_MAX)) {
        return GDKResult::error_result(E_INVALIDARG, String("invalid_") + p_name, p_name + String(" must fit in a non-negative 32-bit unsigned integer."));
    }

    *r_value = static_cast<uint32_t>(p_value);
    return GDKResult::ok_result();
}

Ref<GDKResult> copy_utf8_to_buffer(const String &p_value, char *p_buffer, size_t p_buffer_size, const String &p_field_name) {
    if (p_buffer == nullptr || p_buffer_size == 0) {
        return GDKResult::error_result(E_POINTER, "invalid_output", "String output storage is unavailable.");
    }

    const CharString utf8 = p_value.utf8();
    const size_t length = std::strlen(utf8.get_data());
    if (length >= p_buffer_size) {
        return GDKResult::error_result(E_INVALIDARG, String("invalid_") + p_field_name, p_field_name + String(" is too long."));
    }

    std::memset(p_buffer, 0, p_buffer_size);
    std::memcpy(p_buffer, utf8.get_data(), length);
    return GDKResult::ok_result();
}

PackedByteArray to_packed_byte_array(const std::vector<uint8_t> &p_data) {
    PackedByteArray result;
    result.resize(static_cast<int64_t>(p_data.size()));
    for (int64_t i = 0; i < result.size(); ++i) {
        result.set(i, p_data[static_cast<size_t>(i)]);
    }
    return result;
}

std::vector<uint8_t> to_byte_vector(const PackedByteArray &p_data) {
    std::vector<uint8_t> result;
    result.reserve(static_cast<size_t>(p_data.size()));
    for (int64_t i = 0; i < p_data.size(); ++i) {
        result.push_back(static_cast<uint8_t>(p_data[i]));
    }
    return result;
}

} // namespace gdk_request_parsing
} // namespace godot
