// playfab_party_codec.cpp — Implementation of the PlayFab Party wire codec.
//
// See playfab_party_codec.h for the contract. This translation unit depends
// only on godot-cpp built-in types and the C++ standard library (no PlayFab
// Party SDK headers) so it can be compiled into both the production addon and
// the standalone fuzz harness.

#include "playfab_party_codec.h"

#include <cstring>

namespace godot {
namespace pf_party_codec {

PackedByteArray build_handshake_request(uint32_t p_nonce, const String &p_entity_id, const String &p_entity_type) {
    const CharString id_utf8 = p_entity_id.utf8();
    const CharString type_utf8 = p_entity_type.utf8();
    const uint16_t id_len = static_cast<uint16_t>(id_utf8.length());
    const uint16_t type_len = static_cast<uint16_t>(type_utf8.length());

    PackedByteArray packet;
    packet.resize(1 + sizeof(uint32_t) + sizeof(uint16_t) + id_len + sizeof(uint16_t) + type_len);
    int32_t offset = 0;
    packet[offset++] = static_cast<uint8_t>(PACKET_KIND_HANDSHAKE_REQUEST);
    std::memcpy(packet.ptrw() + offset, &p_nonce, sizeof(uint32_t));
    offset += sizeof(uint32_t);
    std::memcpy(packet.ptrw() + offset, &id_len, sizeof(uint16_t));
    offset += sizeof(uint16_t);
    if (id_len > 0) {
        std::memcpy(packet.ptrw() + offset, id_utf8.get_data(), id_len);
        offset += id_len;
    }
    std::memcpy(packet.ptrw() + offset, &type_len, sizeof(uint16_t));
    offset += sizeof(uint16_t);
    if (type_len > 0) {
        std::memcpy(packet.ptrw() + offset, type_utf8.get_data(), type_len);
    }
    return packet;
}

PackedByteArray build_handshake_reply(uint32_t p_nonce, int32_t p_assigned_peer_id) {
    PackedByteArray packet;
    packet.resize(1 + sizeof(uint32_t) + sizeof(int32_t));
    packet[0] = static_cast<uint8_t>(PACKET_KIND_HANDSHAKE_REPLY);
    std::memcpy(packet.ptrw() + 1, &p_nonce, sizeof(uint32_t));
    std::memcpy(packet.ptrw() + 1 + sizeof(uint32_t), &p_assigned_peer_id, sizeof(int32_t));
    return packet;
}

bool parse_handshake_request(const uint8_t *p_buffer, uint32_t p_size, uint32_t *r_nonce, String *r_entity_id, String *r_entity_type) {
    if (p_buffer == nullptr || p_size < 1 + sizeof(uint32_t) + sizeof(uint16_t) + sizeof(uint16_t)) {
        return false;
    }
    if (p_buffer[0] != PACKET_KIND_HANDSHAKE_REQUEST) {
        return false;
    }
    uint32_t offset = 1;
    if (r_nonce != nullptr) {
        std::memcpy(r_nonce, p_buffer + offset, sizeof(uint32_t));
    }
    offset += sizeof(uint32_t);
    uint16_t id_len = 0;
    std::memcpy(&id_len, p_buffer + offset, sizeof(uint16_t));
    offset += sizeof(uint16_t);
    if (offset + id_len > p_size) {
        return false;
    }
    if (r_entity_id != nullptr && id_len > 0) {
        *r_entity_id = String::utf8(reinterpret_cast<const char *>(p_buffer + offset), id_len);
    }
    offset += id_len;
    if (offset + sizeof(uint16_t) > p_size) {
        return false;
    }
    uint16_t type_len = 0;
    std::memcpy(&type_len, p_buffer + offset, sizeof(uint16_t));
    offset += sizeof(uint16_t);
    if (offset + type_len > p_size) {
        return false;
    }
    if (r_entity_type != nullptr && type_len > 0) {
        *r_entity_type = String::utf8(reinterpret_cast<const char *>(p_buffer + offset), type_len);
    }
    return true;
}

bool parse_handshake_reply(const uint8_t *p_buffer, uint32_t p_size, uint32_t *r_nonce, int32_t *r_assigned_peer_id) {
    if (p_buffer == nullptr || p_size < 1 + sizeof(uint32_t) + sizeof(int32_t)) {
        return false;
    }
    if (p_buffer[0] != PACKET_KIND_HANDSHAKE_REPLY) {
        return false;
    }
    if (r_nonce != nullptr) {
        std::memcpy(r_nonce, p_buffer + 1, sizeof(uint32_t));
    }
    if (r_assigned_peer_id != nullptr) {
        std::memcpy(r_assigned_peer_id, p_buffer + 1 + sizeof(uint32_t), sizeof(int32_t));
    }
    return true;
}

PackedByteArray wrap_gameplay_payload(int32_t p_source_peer, int32_t p_channel, MultiplayerPeer::TransferMode p_mode, const uint8_t *p_buffer, uint32_t p_size) {
    PackedByteArray envelope;
    envelope.resize(1 + sizeof(int32_t) + sizeof(int32_t) + 1 + p_size);
    int32_t offset = 0;
    envelope[offset++] = static_cast<uint8_t>(PACKET_KIND_GAMEPLAY);
    std::memcpy(envelope.ptrw() + offset, &p_source_peer, sizeof(int32_t));
    offset += sizeof(int32_t);
    std::memcpy(envelope.ptrw() + offset, &p_channel, sizeof(int32_t));
    offset += sizeof(int32_t);
    envelope[offset++] = static_cast<uint8_t>(p_mode);
    if (p_size > 0) {
        std::memcpy(envelope.ptrw() + offset, p_buffer, p_size);
    }
    return envelope;
}

bool unwrap_gameplay_payload(const uint8_t *p_buffer, uint32_t p_size, int32_t *r_source_peer, int32_t *r_channel, MultiplayerPeer::TransferMode *r_mode, PackedByteArray *r_payload) {
    if (p_buffer == nullptr || p_size < 1 + sizeof(int32_t) + sizeof(int32_t) + 1) {
        return false;
    }
    if (p_buffer[0] != PACKET_KIND_GAMEPLAY) {
        return false;
    }
    int32_t source_peer = 0;
    int32_t channel = 0;
    std::memcpy(&source_peer, p_buffer + 1, sizeof(int32_t));
    std::memcpy(&channel, p_buffer + 1 + sizeof(int32_t), sizeof(int32_t));
    const uint8_t mode_value = p_buffer[1 + sizeof(int32_t) * 2];
    const uint32_t payload_size = p_size - (1 + sizeof(int32_t) * 2 + 1);
    if (r_source_peer != nullptr) {
        *r_source_peer = source_peer;
    }
    if (r_channel != nullptr) {
        *r_channel = channel;
    }
    if (r_mode != nullptr) {
        *r_mode = static_cast<MultiplayerPeer::TransferMode>(mode_value);
    }
    if (r_payload != nullptr) {
        r_payload->resize(payload_size);
        if (payload_size > 0) {
            std::memcpy(r_payload->ptrw(), p_buffer + (1 + sizeof(int32_t) * 2 + 1), payload_size);
        }
    }
    return true;
}

} // namespace pf_party_codec
} // namespace godot
