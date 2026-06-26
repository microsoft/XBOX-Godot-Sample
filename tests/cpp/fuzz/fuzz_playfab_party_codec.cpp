// fuzz_playfab_party_codec.cpp — Phase 3 libFuzzer target.
//
// Fuzzes the PlayFab Party wire codec (addons/godot_playfab/src/
// playfab_party_codec.cpp). The parse_*/unwrap_* functions decode bytes that
// originate from a remote peer, so they are the addon's primary untrusted-input
// surface: a malformed packet must never read out of bounds or crash.
//
// Two complementary strategies run on every input:
//   (A) Raw decode — the whole fuzz buffer is fed to every parse/unwrap entry
//       point (with both populated and null output pointers) to surface
//       out-of-bounds reads on attacker-controlled length fields.
//   (B) Round-trip oracles — values carved from the fuzz buffer are encoded
//       with the build_*/wrap_* helpers and decoded back; any mismatch trips
//       __builtin_trap(), catching encode/decode asymmetry bugs.

#include "godot_stub_init.h"

#include "playfab_party_codec.h"

#include <cstdint>
#include <cstring>

namespace {

using godot::MultiplayerPeer;
using godot::PackedByteArray;
using godot::String;

namespace codec = godot::pf_party_codec;

// Carve a length-prefixed ASCII string out of the cursor. Bytes are masked to
// 0x01..0x7F so the UTF-8 encode/decode performed by the codec is the identity
// transform and the round-trip oracle can compare bytes exactly.
size_t take_ascii(const uint8_t *d, size_t rem, char *out, size_t out_max, size_t &out_len) {
    if (rem == 0) { out[0] = '\0'; out_len = 0; return 0; }
    size_t len = static_cast<size_t>(d[0]) & 0x3F; // 0..63
    if (len > rem - 1) len = rem - 1;
    if (len >= out_max) len = out_max - 1;
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
    if (rem < sizeof(T)) { out = T(0); return rem; }
    std::memcpy(&out, d, sizeof(T));
    return sizeof(T);
}

bool string_utf8_equals(const String &s, const char *buf, size_t len) {
    const godot::CharString cs = s.utf8();
    return static_cast<size_t>(cs.length()) == len &&
           (len == 0 || std::memcmp(cs.get_data(), buf, len) == 0);
}

} // namespace

extern "C" int LLVMFuzzerInitialize(int *, char ***) {
    godot_stub::init();
    return 0;
}

extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size) {
    const uint32_t raw_size = size > 0xFFFFFFFFu ? 0xFFFFFFFFu : static_cast<uint32_t>(size);

    // ── (A) Raw decode of untrusted bytes ────────────────────────────────
    {
        uint32_t nonce = 0;
        int32_t peer = 0, source = 0, channel = 0;
        String entity_id, entity_type;
        MultiplayerPeer::TransferMode mode = MultiplayerPeer::TRANSFER_MODE_RELIABLE;
        PackedByteArray payload;

        codec::parse_handshake_request(data, raw_size, &nonce, &entity_id, &entity_type);
        codec::parse_handshake_request(data, raw_size, nullptr, nullptr, nullptr);
        codec::parse_handshake_reply(data, raw_size, &nonce, &peer);
        codec::parse_handshake_reply(data, raw_size, nullptr, nullptr);
        codec::unwrap_gameplay_payload(data, raw_size, &source, &channel, &mode, &payload);
        codec::unwrap_gameplay_payload(data, raw_size, nullptr, nullptr, nullptr, nullptr);
    }

    // ── (B) Round-trip oracles ───────────────────────────────────────────
    size_t pos = 0;

    // Handshake request: nonce(4) + id + type.
    {
        uint32_t nonce_in = 0;
        pos += take_int(data + pos, size - pos, nonce_in);
        char id_buf[64] = {}, type_buf[64] = {};
        size_t id_len = 0, type_len = 0;
        pos += take_ascii(data + pos, size - pos, id_buf, sizeof(id_buf), id_len);
        pos += take_ascii(data + pos, size - pos, type_buf, sizeof(type_buf), type_len);

        const String id_in = String::utf8(id_buf, static_cast<int64_t>(id_len));
        const String type_in = String::utf8(type_buf, static_cast<int64_t>(type_len));
        PackedByteArray packet = codec::build_handshake_request(nonce_in, id_in, type_in);

        uint32_t nonce_out = 0;
        String id_out, type_out;
        if (!codec::parse_handshake_request(packet.ptr(), static_cast<uint32_t>(packet.size()),
                                            &nonce_out, &id_out, &type_out)) {
            __builtin_trap();
        }
        if (nonce_out != nonce_in) __builtin_trap();
        if (!string_utf8_equals(id_out, id_buf, id_len)) __builtin_trap();
        if (!string_utf8_equals(type_out, type_buf, type_len)) __builtin_trap();
    }

    // Handshake reply: nonce(4) + assigned_peer_id(4).
    {
        uint32_t nonce_in = 0;
        int32_t peer_in = 0;
        pos += take_int(data + pos, size - pos, nonce_in);
        pos += take_int(data + pos, size - pos, peer_in);

        PackedByteArray packet = codec::build_handshake_reply(nonce_in, peer_in);
        uint32_t nonce_out = 0;
        int32_t peer_out = 0;
        if (!codec::parse_handshake_reply(packet.ptr(), static_cast<uint32_t>(packet.size()),
                                          &nonce_out, &peer_out)) {
            __builtin_trap();
        }
        if (nonce_out != nonce_in || peer_out != peer_in) __builtin_trap();
    }

    // Gameplay envelope: source(4) + channel(4) + mode(1) + payload(rest).
    {
        int32_t source_in = 0, channel_in = 0;
        pos += take_int(data + pos, size - pos, source_in);
        pos += take_int(data + pos, size - pos, channel_in);

        uint8_t mode_byte = 0;
        if (pos < size) { mode_byte = data[pos]; ++pos; }
        const MultiplayerPeer::TransferMode mode_in =
            static_cast<MultiplayerPeer::TransferMode>(mode_byte % 3);

        const uint8_t *payload_in = data + pos;
        size_t payload_len = size - pos;
        if (payload_len > 4096) payload_len = 4096;

        PackedByteArray envelope = codec::wrap_gameplay_payload(
            source_in, channel_in, mode_in, payload_in, static_cast<uint32_t>(payload_len));

        int32_t source_out = 0, channel_out = 0;
        MultiplayerPeer::TransferMode mode_out = MultiplayerPeer::TRANSFER_MODE_RELIABLE;
        PackedByteArray payload_out;
        if (!codec::unwrap_gameplay_payload(envelope.ptr(), static_cast<uint32_t>(envelope.size()),
                                            &source_out, &channel_out, &mode_out, &payload_out)) {
            __builtin_trap();
        }
        if (source_out != source_in || channel_out != channel_in) __builtin_trap();
        if (static_cast<uint8_t>(mode_out) != static_cast<uint8_t>(mode_in)) __builtin_trap();
        if (static_cast<size_t>(payload_out.size()) != payload_len) __builtin_trap();
        if (payload_len > 0 && std::memcmp(payload_out.ptr(), payload_in, payload_len) != 0) {
            __builtin_trap();
        }
    }

    return 0;
}
