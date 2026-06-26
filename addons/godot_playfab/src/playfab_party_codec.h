// playfab_party_codec.h — Engine-free seam for the PlayFab Party transport
// wire codec.
//
// PlayFabParty layers its own length-prefixed packet format on top of the
// opaque data messages delivered by PlayFab Party. Every packet begins with a
// one-byte kind marker; handshake packets carry entity-id/peer-id fields and
// gameplay packets wrap an arbitrary payload. The parse_* functions consume
// bytes that originate from a remote peer, so they are the addon's primary
// untrusted-input surface and the reason this codec lives in its own seam:
// the functions use only godot-cpp built-in types (PackedByteArray, String)
// and the MultiplayerPeer::TransferMode enum, with no dependency on the
// PlayFab Party SDK headers, which makes them fuzzable in the standalone
// harness (see tests/cpp/fuzz/fuzz_playfab_party_codec.cpp).
//
// Per-addon copy: lives in godot_playfab/src, matching the repo convention
// that avoids cross-addon include paths.
#ifndef GODOT_PLAYFAB_PARTY_CODEC_H
#define GODOT_PLAYFAB_PARTY_CODEC_H

#include <cstdint>

#include <godot_cpp/classes/multiplayer_peer.hpp>
#include <godot_cpp/variant/packed_byte_array.hpp>
#include <godot_cpp/variant/string.hpp>

namespace godot {
namespace pf_party_codec {

// Wire packet kind markers. The first byte of every envelope identifies the
// packet kind; gameplay packets are forwarded to the multiplayer peer, control
// packets are consumed internally by the handshake state machine.
constexpr uint8_t PACKET_KIND_GAMEPLAY = 0x00;
constexpr uint8_t PACKET_KIND_HANDSHAKE_REQUEST = 0x01;
constexpr uint8_t PACKET_KIND_HANDSHAKE_REPLY = 0x02;

// Build a handshake request packet: kind(1) nonce(4) id_len(2) id type_len(2)
// type. The entity id/type are UTF-8 encoded; lengths are truncated to 16 bits.
PackedByteArray build_handshake_request(uint32_t p_nonce, const String &p_entity_id, const String &p_entity_type);

// Build a handshake reply packet: kind(1) nonce(4) assigned_peer_id(4).
PackedByteArray build_handshake_reply(uint32_t p_nonce, int32_t p_assigned_peer_id);

// Parse a handshake request from untrusted peer bytes. Returns false on any
// truncation, kind mismatch, or out-of-range length field. Output pointers may
// be null to skip a field.
bool parse_handshake_request(const uint8_t *p_buffer, uint32_t p_size, uint32_t *r_nonce, String *r_entity_id, String *r_entity_type);

// Parse a handshake reply from untrusted peer bytes. Returns false on
// truncation or kind mismatch.
bool parse_handshake_reply(const uint8_t *p_buffer, uint32_t p_size, uint32_t *r_nonce, int32_t *r_assigned_peer_id);

// Wrap an arbitrary gameplay payload: kind(1) source_peer(4) channel(4)
// mode(1) payload.
PackedByteArray wrap_gameplay_payload(int32_t p_source_peer, int32_t p_channel, MultiplayerPeer::TransferMode p_mode, const uint8_t *p_buffer, uint32_t p_size);

// Unwrap a gameplay envelope from untrusted peer bytes. Returns false on
// truncation or kind mismatch. The trailing payload (which may be empty) is
// copied into *r_payload when non-null.
bool unwrap_gameplay_payload(const uint8_t *p_buffer, uint32_t p_size, int32_t *r_source_peer, int32_t *r_channel, MultiplayerPeer::TransferMode *r_mode, PackedByteArray *r_payload);

} // namespace pf_party_codec
} // namespace godot

#endif // GODOT_PLAYFAB_PARTY_CODEC_H
