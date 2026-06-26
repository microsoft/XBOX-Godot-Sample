// fuzz_gdk_request_parsing.cpp — Phase 3 libFuzzer target.
//
// Fuzzes the GDK request-input parsing seam (addons/godot_gdk/src/
// gdk_request_parsing.cpp). These helpers validate caller-supplied (ultimately
// script-supplied) request values and touch raw memory: strtoull over a UTF-8
// buffer (try_parse_xuid), memcpy into a fixed native buffer (copy_utf8_to_buffer),
// integer bounds-checks (parse_uint32), and PackedByteArray<->std::vector
// round-trips. A malformed value must never read/write out of bounds or crash.
//
// Two complementary strategies run on every input:
//   (A) Raw decode — a String carved from the fuzz buffer is fed to try_parse_xuid
//       (both reject_zero modes), parse_uint32, and copy_utf8_to_buffer against a
//       fixed stack buffer of fuzz-chosen capacity, with both populated and null
//       outputs, to surface out-of-bounds access on attacker-controlled lengths.
//   (B) Round-trip oracle — bytes carved from the fuzz buffer are converted
//       vector -> PackedByteArray -> vector; any mismatch trips __builtin_trap().

#include "godot_stub_init.h"

#include "gdk_request_parsing.h"

#include <cstdint>
#include <cstring>
#include <vector>

namespace {

namespace parsing = godot::gdk_request_parsing;

using godot::PackedByteArray;
using godot::String;

// Carve a length-prefixed string out of the cursor. Bytes are masked to
// 0x01..0x7F so the carved characters survive String round-tripping and the
// parsed UTF-8 stays single-byte.
size_t take_ascii(const uint8_t *d, size_t rem, char *out, size_t out_max, size_t &out_len) {
    if (rem == 0) {
        out[0] = '\0';
        out_len = 0;
        return 0;
    }
    size_t len = static_cast<size_t>(d[0]) & 0x3F; // 0..63
    if (len > rem - 1) {
        len = rem - 1;
    }
    if (len >= out_max) {
        len = out_max - 1;
    }
    for (size_t i = 0; i < len; ++i) {
        uint8_t b = d[1 + i] & 0x7F;
        out[i] = static_cast<char>(b == 0 ? 0x20 : b);
    }
    out[len] = '\0';
    out_len = len;
    return 1 + len;
}

// Read a fixed-width little-endian integer from the cursor, or 0 if short.
template <typename T>
size_t take_int(const uint8_t *d, size_t rem, T &out) {
    if (rem < sizeof(T)) {
        out = T(0);
        return rem;
    }
    std::memcpy(&out, d, sizeof(T));
    return sizeof(T);
}

} // namespace

extern "C" int LLVMFuzzerInitialize(int *, char ***) {
    godot_stub::init();
    return 0;
}

extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size) {
    size_t pos = 0;

    // ── (A) Raw decode of an untrusted request value ─────────────────────
    char text_buf[128] = {};
    size_t text_len = 0;
    pos += take_ascii(data + pos, size - pos, text_buf, sizeof(text_buf), text_len);
    const String value = String::utf8(text_buf, static_cast<int64_t>(text_len));

    {
        uint64_t xuid = 0;
        parsing::try_parse_xuid(value, &xuid, /*p_reject_zero=*/false);
        parsing::try_parse_xuid(value, &xuid, /*p_reject_zero=*/true);
        // Null output must be handled gracefully (returns false, no deref).
        parsing::try_parse_xuid(value, nullptr, /*p_reject_zero=*/false);
    }

    {
        int64_t raw_value = 0;
        pos += take_int(data + pos, size - pos, raw_value);
        uint32_t parsed = 0;
        parsing::parse_uint32(value, raw_value, &parsed);
        parsing::parse_uint32(value, raw_value, nullptr);
    }

    {
        // Copy into a fixed buffer whose capacity is chosen by the fuzzer so the
        // over-length and exact-fit boundaries are both exercised.
        uint8_t cap_byte = 0;
        pos += take_int(data + pos, size - pos, cap_byte);
        const size_t cap = static_cast<size_t>(cap_byte) % 65; // 0..64
        char dest[64] = {};
        parsing::copy_utf8_to_buffer(value, dest, cap, String("field"));
        // Null / zero-size buffer paths.
        parsing::copy_utf8_to_buffer(value, nullptr, sizeof(dest), String("field"));
        parsing::copy_utf8_to_buffer(value, dest, 0, String("field"));
    }

    // ── (B) PackedByteArray <-> std::vector round-trip oracle ────────────
    {
        size_t blob_len = size - pos;
        if (blob_len > 4096) {
            blob_len = 4096;
        }
        std::vector<uint8_t> in(data + pos, data + pos + blob_len);

        const PackedByteArray packed = parsing::to_packed_byte_array(in);
        if (static_cast<size_t>(packed.size()) != in.size()) {
            __builtin_trap();
        }

        const std::vector<uint8_t> out = parsing::to_byte_vector(packed);
        if (out.size() != in.size()) {
            __builtin_trap();
        }
        if (!in.empty() && std::memcmp(out.data(), in.data(), in.size()) != 0) {
            __builtin_trap();
        }
    }

    return 0;
}
