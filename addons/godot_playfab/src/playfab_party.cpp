#include "playfab_party.h"

#include <algorithm>
#include <cstring>
#include <iterator>
#include <random>
#include <string>

#include <playfab/party/Party.h>
#include <playfab/party/PartyImpl.h>

#include <godot_cpp/classes/project_settings.hpp>

#include "playfab.h"
#include "playfab_pending_signal.h"
#include "playfab_result.h"
#include "playfab_runtime.h"
#include "playfab_user.h"

namespace godot {

namespace {

constexpr const char *PARTY_NOT_INITIALIZED = "party_not_initialized";
constexpr const char *PARTY_ALREADY_INITIALIZED = "party_already_initialized";
constexpr const char *PARTY_INVALID_USER = "party_invalid_user";
constexpr const char *PARTY_INVALID_OPTIONS = "party_invalid_options";
constexpr const char *PARTY_NETWORK_ALREADY_ACTIVE = "party_network_already_active";
constexpr const char *PARTY_NETWORK_CREATE_FAILED = "party_network_create_failed";
constexpr const char *PARTY_NETWORK_CONNECT_FAILED = "party_network_connect_failed";
constexpr const char *PARTY_DESCRIPTOR_INVALID = "party_descriptor_invalid";
constexpr const char *PARTY_TRANSPORT_CREATE_FAILED = "party_transport_create_failed";
constexpr const char *PARTY_PEER_NOT_CONNECTED = "party_peer_not_connected";
constexpr const char *PARTY_RESOURCE_NOT_READY = "party_resource_not_ready";
constexpr const char *PARTY_CHAT_CONTROL_CREATE_FAILED = "party_chat_control_create_failed";
constexpr const char *PARTY_CHAT_PERMISSION_FAILED = "party_chat_permission_failed";
constexpr const char *PARTY_STATE_FINISH_FAILED = "party_state_finish_failed";

// Reserved transport-control packet protocol marker. The first byte of every
// envelope identifies the packet kind; gameplay packets are forwarded to
// _get_packet(), control packets are consumed internally.
constexpr uint8_t PACKET_KIND_GAMEPLAY = 0x00;
constexpr uint8_t PACKET_KIND_HANDSHAKE_REQUEST = 0x01;
constexpr uint8_t PACKET_KIND_HANDSHAKE_REPLY = 0x02;

constexpr int32_t HOST_PEER_ID = 1;

Signal detached_error_signal(HRESULT p_hresult, const String &p_code, const String &p_message, const Variant &p_data = Variant()) {
    Ref<PlayFabPendingSignal> pending_signal;
    pending_signal.instantiate();
    pending_signal->complete_deferred(PlayFabResult::error_result(p_hresult, p_code, p_message, p_data));
    return pending_signal->get_completed_signal();
}

Signal detached_ok_signal(const Variant &p_data = Variant()) {
    Ref<PlayFabPendingSignal> pending_signal;
    pending_signal.instantiate();
    pending_signal->complete_deferred(PlayFabResult::ok_result(p_data));
    return pending_signal->get_completed_signal();
}

bool user_has_entity_handle(const Ref<PlayFabUser> &p_user) {
    return p_user.is_valid() && p_user->get_entity_handle() != nullptr;
}

String party_string(const char *p_value) {
    return p_value != nullptr ? String::utf8(p_value) : String();
}

Dictionary entity_id_pair_to_dictionary(const char *p_entity_id, const char *p_entity_type) {
    Dictionary entity_key;
    entity_key["id"] = party_string(p_entity_id);
    entity_key["type"] = party_string(p_entity_type);
    return entity_key;
}

Dictionary entity_key_for_endpoint(Party::PartyEndpoint *p_endpoint) {
    Dictionary entity_key;
    if (p_endpoint == nullptr) {
        return entity_key;
    }
    PartyString id = nullptr;
    PartyString type = nullptr;
    p_endpoint->GetEntityId(&id);
    p_endpoint->GetEntityType(&type);
    return entity_id_pair_to_dictionary(id, type);
}

Dictionary entity_key_for_chat_control(Party::PartyChatControl *p_chat_control) {
    Dictionary entity_key;
    if (p_chat_control == nullptr) {
        return entity_key;
    }
    PartyString id = nullptr;
    PartyString type = nullptr;
    p_chat_control->GetEntityId(&id);
    p_chat_control->GetEntityType(&type);
    return entity_id_pair_to_dictionary(id, type);
}

uint32_t random_handshake_nonce() {
    static thread_local std::mt19937 generator{std::random_device{}()};
    std::uniform_int_distribution<uint32_t> distribution(1, UINT32_MAX);
    return distribution(generator);
}

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

bool dictionary_entity_key_equals(const Dictionary &a, const Dictionary &b) {
    return String(a.get("id", String())) == String(b.get("id", String())) &&
            String(a.get("type", String())) == String(b.get("type", String()));
}

} // namespace

// ---------------------------------------------------------------------------
// PlayFabPartyConfig

void PlayFabPartyConfig::_bind_methods() {
    ClassDB::bind_method(D_METHOD("get_max_players"), &PlayFabPartyConfig::get_max_players);
    ClassDB::bind_method(D_METHOD("set_max_players", "max_players"), &PlayFabPartyConfig::set_max_players);
    ClassDB::bind_method(D_METHOD("get_direct_peer_connectivity"), &PlayFabPartyConfig::get_direct_peer_connectivity);
    ClassDB::bind_method(D_METHOD("set_direct_peer_connectivity", "direct_peer_connectivity"), &PlayFabPartyConfig::set_direct_peer_connectivity);
    ClassDB::bind_method(D_METHOD("get_invitation_id"), &PlayFabPartyConfig::get_invitation_id);
    ClassDB::bind_method(D_METHOD("set_invitation_id", "invitation_id"), &PlayFabPartyConfig::set_invitation_id);
    ClassDB::bind_method(D_METHOD("is_voice_chat_enabled"), &PlayFabPartyConfig::is_voice_chat_enabled);
    ClassDB::bind_method(D_METHOD("set_voice_chat_enabled", "enabled"), &PlayFabPartyConfig::set_voice_chat_enabled);
    ClassDB::bind_method(D_METHOD("is_text_chat_enabled"), &PlayFabPartyConfig::is_text_chat_enabled);
    ClassDB::bind_method(D_METHOD("set_text_chat_enabled", "enabled"), &PlayFabPartyConfig::set_text_chat_enabled);
    ClassDB::bind_method(D_METHOD("is_transcription_enabled"), &PlayFabPartyConfig::is_transcription_enabled);
    ClassDB::bind_method(D_METHOD("set_transcription_enabled", "enabled"), &PlayFabPartyConfig::set_transcription_enabled);
    ClassDB::bind_method(D_METHOD("is_translation_enabled"), &PlayFabPartyConfig::is_translation_enabled);
    ClassDB::bind_method(D_METHOD("set_translation_enabled", "enabled"), &PlayFabPartyConfig::set_translation_enabled);
    ClassDB::bind_method(D_METHOD("get_audio_input"), &PlayFabPartyConfig::get_audio_input);
    ClassDB::bind_method(D_METHOD("set_audio_input", "audio_input"), &PlayFabPartyConfig::set_audio_input);
    ClassDB::bind_method(D_METHOD("get_audio_output"), &PlayFabPartyConfig::get_audio_output);
    ClassDB::bind_method(D_METHOD("set_audio_output", "audio_output"), &PlayFabPartyConfig::set_audio_output);
    ClassDB::bind_method(D_METHOD("get_metadata"), &PlayFabPartyConfig::get_metadata);
    ClassDB::bind_method(D_METHOD("set_metadata", "metadata"), &PlayFabPartyConfig::set_metadata);

    ADD_PROPERTY(PropertyInfo(Variant::INT, "max_players"), "set_max_players", "get_max_players");
    ADD_PROPERTY(PropertyInfo(Variant::INT, "direct_peer_connectivity"), "set_direct_peer_connectivity", "get_direct_peer_connectivity");
    ADD_PROPERTY(PropertyInfo(Variant::STRING, "invitation_id"), "set_invitation_id", "get_invitation_id");
    ADD_PROPERTY(PropertyInfo(Variant::BOOL, "enable_voice_chat"), "set_voice_chat_enabled", "is_voice_chat_enabled");
    ADD_PROPERTY(PropertyInfo(Variant::BOOL, "enable_text_chat"), "set_text_chat_enabled", "is_text_chat_enabled");
    ADD_PROPERTY(PropertyInfo(Variant::BOOL, "enable_transcription"), "set_transcription_enabled", "is_transcription_enabled");
    ADD_PROPERTY(PropertyInfo(Variant::BOOL, "enable_translation"), "set_translation_enabled", "is_translation_enabled");
    ADD_PROPERTY(PropertyInfo(Variant::STRING, "audio_input"), "set_audio_input", "get_audio_input");
    ADD_PROPERTY(PropertyInfo(Variant::STRING, "audio_output"), "set_audio_output", "get_audio_output");
    ADD_PROPERTY(PropertyInfo(Variant::DICTIONARY, "metadata"), "set_metadata", "get_metadata");
}

int64_t PlayFabPartyConfig::get_max_players() const { return m_max_players; }
void PlayFabPartyConfig::set_max_players(int64_t p_max_players) { m_max_players = p_max_players; }
int64_t PlayFabPartyConfig::get_direct_peer_connectivity() const { return m_direct_peer_connectivity; }
void PlayFabPartyConfig::set_direct_peer_connectivity(int64_t p_direct_peer_connectivity) { m_direct_peer_connectivity = p_direct_peer_connectivity; }
String PlayFabPartyConfig::get_invitation_id() const { return m_invitation_id; }
void PlayFabPartyConfig::set_invitation_id(const String &p_invitation_id) { m_invitation_id = p_invitation_id; }
bool PlayFabPartyConfig::is_voice_chat_enabled() const { return m_enable_voice_chat; }
void PlayFabPartyConfig::set_voice_chat_enabled(bool p_enabled) { m_enable_voice_chat = p_enabled; }
bool PlayFabPartyConfig::is_text_chat_enabled() const { return m_enable_text_chat; }
void PlayFabPartyConfig::set_text_chat_enabled(bool p_enabled) { m_enable_text_chat = p_enabled; }
bool PlayFabPartyConfig::is_transcription_enabled() const { return m_enable_transcription; }
void PlayFabPartyConfig::set_transcription_enabled(bool p_enabled) { m_enable_transcription = p_enabled; }
bool PlayFabPartyConfig::is_translation_enabled() const { return m_enable_translation; }
void PlayFabPartyConfig::set_translation_enabled(bool p_enabled) { m_enable_translation = p_enabled; }
String PlayFabPartyConfig::get_audio_input() const { return m_audio_input; }
void PlayFabPartyConfig::set_audio_input(const String &p_audio_input) { m_audio_input = p_audio_input; }
String PlayFabPartyConfig::get_audio_output() const { return m_audio_output; }
void PlayFabPartyConfig::set_audio_output(const String &p_audio_output) { m_audio_output = p_audio_output; }
Dictionary PlayFabPartyConfig::get_metadata() const { return m_metadata; }
void PlayFabPartyConfig::set_metadata(const Dictionary &p_metadata) { m_metadata = p_metadata; }

// ---------------------------------------------------------------------------
// PlayFabPartyTextMessageConfig

void PlayFabPartyTextMessageConfig::_bind_methods() {
    ClassDB::bind_method(D_METHOD("get_language_code"), &PlayFabPartyTextMessageConfig::get_language_code);
    ClassDB::bind_method(D_METHOD("set_language_code", "language_code"), &PlayFabPartyTextMessageConfig::set_language_code);
    ClassDB::bind_method(D_METHOD("get_translate_to_languages"), &PlayFabPartyTextMessageConfig::get_translate_to_languages);
    ClassDB::bind_method(D_METHOD("set_translate_to_languages", "languages"), &PlayFabPartyTextMessageConfig::set_translate_to_languages);
    ClassDB::bind_method(D_METHOD("get_metadata"), &PlayFabPartyTextMessageConfig::get_metadata);
    ClassDB::bind_method(D_METHOD("set_metadata", "metadata"), &PlayFabPartyTextMessageConfig::set_metadata);

    ADD_PROPERTY(PropertyInfo(Variant::STRING, "language_code"), "set_language_code", "get_language_code");
    ADD_PROPERTY(PropertyInfo(Variant::PACKED_STRING_ARRAY, "translate_to_languages"), "set_translate_to_languages", "get_translate_to_languages");
    ADD_PROPERTY(PropertyInfo(Variant::DICTIONARY, "metadata"), "set_metadata", "get_metadata");
}

String PlayFabPartyTextMessageConfig::get_language_code() const { return m_language_code; }
void PlayFabPartyTextMessageConfig::set_language_code(const String &p_language_code) { m_language_code = p_language_code; }
PackedStringArray PlayFabPartyTextMessageConfig::get_translate_to_languages() const { return m_translate_to_languages; }
void PlayFabPartyTextMessageConfig::set_translate_to_languages(const PackedStringArray &p_languages) { m_translate_to_languages = p_languages; }
Dictionary PlayFabPartyTextMessageConfig::get_metadata() const { return m_metadata; }
void PlayFabPartyTextMessageConfig::set_metadata(const Dictionary &p_metadata) { m_metadata = p_metadata; }

// ---------------------------------------------------------------------------
// PlayFabPartyMember

void PlayFabPartyMember::_bind_methods() {
    ClassDB::bind_method(D_METHOD("get_peer_id"), &PlayFabPartyMember::get_peer_id);
    ClassDB::bind_method(D_METHOD("get_entity_key"), &PlayFabPartyMember::get_entity_key);
    ClassDB::bind_method(D_METHOD("get_user"), &PlayFabPartyMember::get_user);
    ClassDB::bind_method(D_METHOD("is_local_member"), &PlayFabPartyMember::is_local_member);

    ADD_PROPERTY(PropertyInfo(Variant::INT, "peer_id", PROPERTY_HINT_NONE, "", PROPERTY_USAGE_SCRIPT_VARIABLE), "", "get_peer_id");
    ADD_PROPERTY(PropertyInfo(Variant::DICTIONARY, "entity_key", PROPERTY_HINT_NONE, "", PROPERTY_USAGE_SCRIPT_VARIABLE), "", "get_entity_key");
    ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "user", PROPERTY_HINT_RESOURCE_TYPE, "PlayFabUser", PROPERTY_USAGE_SCRIPT_VARIABLE), "", "get_user");
    ADD_PROPERTY(PropertyInfo(Variant::BOOL, "is_local", PROPERTY_HINT_NONE, "", PROPERTY_USAGE_SCRIPT_VARIABLE), "", "is_local_member");
}

void PlayFabPartyMember::set_snapshot(int64_t p_peer_id, const Dictionary &p_entity_key, const Ref<PlayFabUser> &p_user, bool p_local) {
    m_peer_id = p_peer_id;
    m_entity_key = p_entity_key;
    m_user = p_user;
    m_local = p_local;
}

int64_t PlayFabPartyMember::get_peer_id() const { return m_peer_id; }
Dictionary PlayFabPartyMember::get_entity_key() const { return m_entity_key; }
Ref<PlayFabUser> PlayFabPartyMember::get_user() const { return m_user; }
bool PlayFabPartyMember::is_local_member() const { return m_local; }

// ---------------------------------------------------------------------------
// PlayFabPartyChatMessage

void PlayFabPartyChatMessage::_bind_methods() {
    ClassDB::bind_method(D_METHOD("get_sender"), &PlayFabPartyChatMessage::get_sender);
    ClassDB::bind_method(D_METHOD("get_sender_entity_key"), &PlayFabPartyChatMessage::get_sender_entity_key);
    ClassDB::bind_method(D_METHOD("get_targets"), &PlayFabPartyChatMessage::get_targets);
    ClassDB::bind_method(D_METHOD("get_text"), &PlayFabPartyChatMessage::get_text);
    ClassDB::bind_method(D_METHOD("get_language_code"), &PlayFabPartyChatMessage::get_language_code);
    ClassDB::bind_method(D_METHOD("get_translated_text"), &PlayFabPartyChatMessage::get_translated_text);
    ClassDB::bind_method(D_METHOD("is_transcription"), &PlayFabPartyChatMessage::is_transcription);
    ClassDB::bind_method(D_METHOD("get_timestamp"), &PlayFabPartyChatMessage::get_timestamp);
    ClassDB::bind_method(D_METHOD("get_metadata"), &PlayFabPartyChatMessage::get_metadata);

    ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "sender", PROPERTY_HINT_RESOURCE_TYPE, "PlayFabPartyChatControl", PROPERTY_USAGE_SCRIPT_VARIABLE), "", "get_sender");
    ADD_PROPERTY(PropertyInfo(Variant::DICTIONARY, "sender_entity_key", PROPERTY_HINT_NONE, "", PROPERTY_USAGE_SCRIPT_VARIABLE), "", "get_sender_entity_key");
    ADD_PROPERTY(PropertyInfo(Variant::ARRAY, "targets", PROPERTY_HINT_NONE, "", PROPERTY_USAGE_SCRIPT_VARIABLE), "", "get_targets");
    ADD_PROPERTY(PropertyInfo(Variant::STRING, "text", PROPERTY_HINT_NONE, "", PROPERTY_USAGE_SCRIPT_VARIABLE), "", "get_text");
    ADD_PROPERTY(PropertyInfo(Variant::STRING, "language_code", PROPERTY_HINT_NONE, "", PROPERTY_USAGE_SCRIPT_VARIABLE), "", "get_language_code");
    ADD_PROPERTY(PropertyInfo(Variant::STRING, "translated_text", PROPERTY_HINT_NONE, "", PROPERTY_USAGE_SCRIPT_VARIABLE), "", "get_translated_text");
    ADD_PROPERTY(PropertyInfo(Variant::BOOL, "is_transcription", PROPERTY_HINT_NONE, "", PROPERTY_USAGE_SCRIPT_VARIABLE), "", "is_transcription");
    ADD_PROPERTY(PropertyInfo(Variant::INT, "timestamp", PROPERTY_HINT_NONE, "", PROPERTY_USAGE_SCRIPT_VARIABLE), "", "get_timestamp");
    ADD_PROPERTY(PropertyInfo(Variant::DICTIONARY, "metadata", PROPERTY_HINT_NONE, "", PROPERTY_USAGE_SCRIPT_VARIABLE), "", "get_metadata");
}

void PlayFabPartyChatMessage::set_values(
        const Ref<PlayFabPartyChatControl> &p_sender,
        const Dictionary &p_sender_entity_key,
        const Array &p_targets,
        const String &p_text,
        const String &p_language_code,
        const String &p_translated_text,
        bool p_transcription,
        int64_t p_timestamp,
        const Dictionary &p_metadata) {
    m_sender = p_sender;
    m_sender_entity_key = p_sender_entity_key;
    m_targets = p_targets;
    m_text = p_text;
    m_language_code = p_language_code;
    m_translated_text = p_translated_text;
    m_transcription = p_transcription;
    m_timestamp = p_timestamp;
    m_metadata = p_metadata;
}

Ref<PlayFabPartyChatControl> PlayFabPartyChatMessage::get_sender() const { return m_sender; }
Dictionary PlayFabPartyChatMessage::get_sender_entity_key() const { return m_sender_entity_key; }
Array PlayFabPartyChatMessage::get_targets() const { return m_targets; }
String PlayFabPartyChatMessage::get_text() const { return m_text; }
String PlayFabPartyChatMessage::get_language_code() const { return m_language_code; }
String PlayFabPartyChatMessage::get_translated_text() const { return m_translated_text; }
bool PlayFabPartyChatMessage::is_transcription() const { return m_transcription; }
int64_t PlayFabPartyChatMessage::get_timestamp() const { return m_timestamp; }
Dictionary PlayFabPartyChatMessage::get_metadata() const { return m_metadata; }

// ---------------------------------------------------------------------------
// PlayFabPartyChatStateChange

void PlayFabPartyChatStateChange::_bind_methods() {
    ClassDB::bind_method(D_METHOD("get_kind"), &PlayFabPartyChatStateChange::get_kind);
    ClassDB::bind_method(D_METHOD("get_chat_control"), &PlayFabPartyChatStateChange::get_chat_control);
    ClassDB::bind_method(D_METHOD("get_result"), &PlayFabPartyChatStateChange::get_result);
    ClassDB::bind_method(D_METHOD("get_reason"), &PlayFabPartyChatStateChange::get_reason);

    ADD_PROPERTY(PropertyInfo(Variant::INT, "kind", PROPERTY_HINT_NONE, "", PROPERTY_USAGE_SCRIPT_VARIABLE), "", "get_kind");
    ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "chat_control", PROPERTY_HINT_RESOURCE_TYPE, "PlayFabPartyChatControl", PROPERTY_USAGE_SCRIPT_VARIABLE), "", "get_chat_control");
    ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "result", PROPERTY_HINT_RESOURCE_TYPE, "PlayFabResult", PROPERTY_USAGE_SCRIPT_VARIABLE), "", "get_result");
    ADD_PROPERTY(PropertyInfo(Variant::STRING, "reason", PROPERTY_HINT_NONE, "", PROPERTY_USAGE_SCRIPT_VARIABLE), "", "get_reason");
}

void PlayFabPartyChatStateChange::set_values(
        int64_t p_kind,
        const Ref<PlayFabPartyChatControl> &p_chat_control,
        const Ref<PlayFabResult> &p_result,
        const String &p_reason) {
    m_kind = p_kind;
    m_chat_control = p_chat_control;
    m_result = p_result;
    m_reason = p_reason;
}

int64_t PlayFabPartyChatStateChange::get_kind() const { return m_kind; }
Ref<PlayFabPartyChatControl> PlayFabPartyChatStateChange::get_chat_control() const { return m_chat_control; }
Ref<PlayFabResult> PlayFabPartyChatStateChange::get_result() const { return m_result; }
String PlayFabPartyChatStateChange::get_reason() const { return m_reason; }

// ---------------------------------------------------------------------------
// PlayFabPartyChatControl

void PlayFabPartyChatControl::_bind_methods() {
    ClassDB::bind_method(D_METHOD("get_id"), &PlayFabPartyChatControl::get_id);
    ClassDB::bind_method(D_METHOD("get_user"), &PlayFabPartyChatControl::get_user);
    ClassDB::bind_method(D_METHOD("is_voice_enabled"), &PlayFabPartyChatControl::is_voice_enabled);
    ClassDB::bind_method(D_METHOD("is_text_enabled"), &PlayFabPartyChatControl::is_text_enabled);
    ClassDB::bind_method(D_METHOD("is_transcription_enabled"), &PlayFabPartyChatControl::is_transcription_enabled);
    ClassDB::bind_method(D_METHOD("is_local"), &PlayFabPartyChatControl::is_local);
    ClassDB::bind_method(D_METHOD("send_text_async", "targets", "message", "config"), &PlayFabPartyChatControl::send_text_async, DEFVAL(Ref<PlayFabPartyTextMessageConfig>()));
    ClassDB::bind_method(D_METHOD("set_permissions_async", "target", "permissions"), &PlayFabPartyChatControl::set_permissions_async);
    ClassDB::bind_method(D_METHOD("set_muted_async", "target", "muted"), &PlayFabPartyChatControl::set_muted_async);
    ClassDB::bind_method(D_METHOD("destroy_async"), &PlayFabPartyChatControl::destroy_async);

    ADD_PROPERTY(PropertyInfo(Variant::STRING, "id", PROPERTY_HINT_NONE, "", PROPERTY_USAGE_SCRIPT_VARIABLE), "", "get_id");
    ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "user", PROPERTY_HINT_RESOURCE_TYPE, "PlayFabUser", PROPERTY_USAGE_SCRIPT_VARIABLE), "", "get_user");
    ADD_PROPERTY(PropertyInfo(Variant::BOOL, "is_voice_enabled", PROPERTY_HINT_NONE, "", PROPERTY_USAGE_SCRIPT_VARIABLE), "", "is_voice_enabled");
    ADD_PROPERTY(PropertyInfo(Variant::BOOL, "is_text_enabled", PROPERTY_HINT_NONE, "", PROPERTY_USAGE_SCRIPT_VARIABLE), "", "is_text_enabled");
    ADD_PROPERTY(PropertyInfo(Variant::BOOL, "is_transcription_enabled", PROPERTY_HINT_NONE, "", PROPERTY_USAGE_SCRIPT_VARIABLE), "", "is_transcription_enabled");
    ADD_PROPERTY(PropertyInfo(Variant::BOOL, "is_local", PROPERTY_HINT_NONE, "", PROPERTY_USAGE_SCRIPT_VARIABLE), "", "is_local");

    ADD_SIGNAL(MethodInfo("state_changed", PropertyInfo(Variant::OBJECT, "change", PROPERTY_HINT_RESOURCE_TYPE, "PlayFabPartyChatStateChange")));
    ADD_SIGNAL(MethodInfo("message_received", PropertyInfo(Variant::OBJECT, "message", PROPERTY_HINT_RESOURCE_TYPE, "PlayFabPartyChatMessage")));
    ADD_SIGNAL(MethodInfo("transcription_received", PropertyInfo(Variant::OBJECT, "message", PROPERTY_HINT_RESOURCE_TYPE, "PlayFabPartyChatMessage")));
}

void PlayFabPartyChatControl::attach(PlayFabParty *p_owner, Party::PartyChatControl *p_handle, bool p_local) {
    m_owner = p_owner;
    m_native_handle = p_handle;
    m_local = p_local;
}

void PlayFabPartyChatControl::set_snapshot(
        const String &p_id,
        const Ref<PlayFabUser> &p_user,
        bool p_voice_enabled,
        bool p_text_enabled,
        bool p_transcription_enabled,
        bool p_local) {
    m_id = p_id;
    m_user = p_user;
    m_voice_enabled = p_voice_enabled;
    m_text_enabled = p_text_enabled;
    m_transcription_enabled = p_transcription_enabled;
    m_local = p_local;
}

Party::PartyChatControl *PlayFabPartyChatControl::get_native_handle() const { return m_native_handle; }

String PlayFabPartyChatControl::get_id() const { return m_id; }
Ref<PlayFabUser> PlayFabPartyChatControl::get_user() const { return m_user; }
bool PlayFabPartyChatControl::is_voice_enabled() const { return m_voice_enabled; }
bool PlayFabPartyChatControl::is_text_enabled() const { return m_text_enabled; }
bool PlayFabPartyChatControl::is_transcription_enabled() const { return m_transcription_enabled; }
bool PlayFabPartyChatControl::is_local() const { return m_local; }

Signal PlayFabPartyChatControl::send_text_async(const Array &p_targets, const String &p_message, const Ref<PlayFabPartyTextMessageConfig> &p_config) {
    if (m_owner == nullptr || m_native_handle == nullptr || !m_local) {
        return detached_error_signal(E_NOT_VALID_STATE, PARTY_RESOURCE_NOT_READY,
                "PlayFabPartyChatControl.send_text_async requires a connected local chat control.");
    }
    Party::PartyLocalChatControl *local_handle = static_cast<Party::PartyLocalChatControl *>(m_native_handle);
    std::vector<Party::PartyChatControl *> targets;
    for (int i = 0; i < p_targets.size(); ++i) {
        Ref<PlayFabPartyChatControl> target = p_targets[i];
        if (target.is_valid() && target->get_native_handle() != nullptr) {
            targets.push_back(target->get_native_handle());
        }
    }
    return m_owner->_send_text_via_chat_control(local_handle, targets, p_message, p_config);
}

Signal PlayFabPartyChatControl::set_permissions_async(const Ref<PlayFabPartyChatControl> &p_target, int64_t p_permissions) {
    if (m_owner == nullptr || m_native_handle == nullptr || !m_local) {
        return detached_error_signal(E_NOT_VALID_STATE, PARTY_CHAT_PERMISSION_FAILED,
                "PlayFabPartyChatControl.set_permissions_async requires a connected local chat control.");
    }
    if (!p_target.is_valid() || p_target->get_native_handle() == nullptr) {
        return detached_error_signal(E_INVALIDARG, PARTY_CHAT_PERMISSION_FAILED,
                "PlayFabPartyChatControl.set_permissions_async requires a target chat control.");
    }
    return m_owner->_set_chat_permissions(static_cast<Party::PartyLocalChatControl *>(m_native_handle), p_target->get_native_handle(), p_permissions);
}

Signal PlayFabPartyChatControl::set_muted_async(const Ref<PlayFabPartyChatControl> &p_target, bool p_muted) {
    if (m_owner == nullptr || m_native_handle == nullptr || !m_local) {
        return detached_error_signal(E_NOT_VALID_STATE, PARTY_CHAT_PERMISSION_FAILED,
                "PlayFabPartyChatControl.set_muted_async requires a connected local chat control.");
    }
    if (!p_target.is_valid() || p_target->get_native_handle() == nullptr) {
        return detached_error_signal(E_INVALIDARG, PARTY_CHAT_PERMISSION_FAILED,
                "PlayFabPartyChatControl.set_muted_async requires a target chat control.");
    }
    return m_owner->_set_incoming_audio_muted(static_cast<Party::PartyLocalChatControl *>(m_native_handle), p_target->get_native_handle(), p_muted);
}

Signal PlayFabPartyChatControl::destroy_async() {
    if (m_owner == nullptr) {
        return detached_ok_signal();
    }
    return m_owner->_destroy_chat_control(Ref<PlayFabPartyChatControl>(this));
}

// ---------------------------------------------------------------------------
// PlayFabPartyChat

void PlayFabPartyChat::_bind_methods() {
    ClassDB::bind_method(D_METHOD("get_local_chat_control", "user"), &PlayFabPartyChat::get_local_chat_control);
    ClassDB::bind_method(D_METHOD("get_chat_controls"), &PlayFabPartyChat::get_chat_controls);

    ADD_SIGNAL(MethodInfo("state_changed", PropertyInfo(Variant::OBJECT, "change", PROPERTY_HINT_RESOURCE_TYPE, "PlayFabPartyChatStateChange")));
}

void PlayFabPartyChat::clear() {
    m_chat_controls.clear();
}

void PlayFabPartyChat::track(const Ref<PlayFabPartyChatControl> &p_chat_control) {
    if (!p_chat_control.is_valid()) {
        return;
    }
    if (m_chat_controls.find(p_chat_control) < 0) {
        m_chat_controls.push_back(p_chat_control);
    }
}

void PlayFabPartyChat::untrack(const Ref<PlayFabPartyChatControl> &p_chat_control) {
    if (!p_chat_control.is_valid()) {
        return;
    }
    int idx = m_chat_controls.find(p_chat_control);
    if (idx >= 0) {
        m_chat_controls.remove_at(idx);
    }
}

Ref<PlayFabPartyChatControl> PlayFabPartyChat::get_local_chat_control(const Ref<PlayFabUser> &p_user) const {
    if (!p_user.is_valid()) {
        return Ref<PlayFabPartyChatControl>();
    }
    for (int i = 0; i < m_chat_controls.size(); ++i) {
        Ref<PlayFabPartyChatControl> control = m_chat_controls[i];
        if (control.is_valid() && control->is_local() && control->get_user() == p_user) {
            return control;
        }
    }
    return Ref<PlayFabPartyChatControl>();
}

Array PlayFabPartyChat::get_chat_controls() const {
    return m_chat_controls.duplicate();
}

// ---------------------------------------------------------------------------
// PlayFabPartyNetworkStateChange

void PlayFabPartyNetworkStateChange::_bind_methods() {
    ClassDB::bind_method(D_METHOD("get_kind"), &PlayFabPartyNetworkStateChange::get_kind);
    ClassDB::bind_method(D_METHOD("get_network"), &PlayFabPartyNetworkStateChange::get_network);
    ClassDB::bind_method(D_METHOD("get_result"), &PlayFabPartyNetworkStateChange::get_result);
    ClassDB::bind_method(D_METHOD("get_user"), &PlayFabPartyNetworkStateChange::get_user);
    ClassDB::bind_method(D_METHOD("get_peer_id"), &PlayFabPartyNetworkStateChange::get_peer_id);
    ClassDB::bind_method(D_METHOD("get_state"), &PlayFabPartyNetworkStateChange::get_state);
    ClassDB::bind_method(D_METHOD("get_reason"), &PlayFabPartyNetworkStateChange::get_reason);

    ADD_PROPERTY(PropertyInfo(Variant::INT, "kind", PROPERTY_HINT_NONE, "", PROPERTY_USAGE_SCRIPT_VARIABLE), "", "get_kind");
    ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "network", PROPERTY_HINT_RESOURCE_TYPE, "PlayFabPartyNetwork", PROPERTY_USAGE_SCRIPT_VARIABLE), "", "get_network");
    ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "result", PROPERTY_HINT_RESOURCE_TYPE, "PlayFabResult", PROPERTY_USAGE_SCRIPT_VARIABLE), "", "get_result");
    ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "user", PROPERTY_HINT_RESOURCE_TYPE, "PlayFabUser", PROPERTY_USAGE_SCRIPT_VARIABLE), "", "get_user");
    ADD_PROPERTY(PropertyInfo(Variant::INT, "peer_id", PROPERTY_HINT_NONE, "", PROPERTY_USAGE_SCRIPT_VARIABLE), "", "get_peer_id");
    ADD_PROPERTY(PropertyInfo(Variant::INT, "state", PROPERTY_HINT_NONE, "", PROPERTY_USAGE_SCRIPT_VARIABLE), "", "get_state");
    ADD_PROPERTY(PropertyInfo(Variant::STRING, "reason", PROPERTY_HINT_NONE, "", PROPERTY_USAGE_SCRIPT_VARIABLE), "", "get_reason");
}

void PlayFabPartyNetworkStateChange::set_values(
        int64_t p_kind,
        const Ref<PlayFabPartyNetwork> &p_network,
        const Ref<PlayFabResult> &p_result,
        const Ref<PlayFabUser> &p_user,
        int64_t p_peer_id,
        int64_t p_state,
        const String &p_reason) {
    m_kind = p_kind;
    m_network = p_network;
    m_result = p_result;
    m_user = p_user;
    m_peer_id = p_peer_id;
    m_state = p_state;
    m_reason = p_reason;
}

int64_t PlayFabPartyNetworkStateChange::get_kind() const { return m_kind; }
Ref<PlayFabPartyNetwork> PlayFabPartyNetworkStateChange::get_network() const { return m_network; }
Ref<PlayFabResult> PlayFabPartyNetworkStateChange::get_result() const { return m_result; }
Ref<PlayFabUser> PlayFabPartyNetworkStateChange::get_user() const { return m_user; }
int64_t PlayFabPartyNetworkStateChange::get_peer_id() const { return m_peer_id; }
int64_t PlayFabPartyNetworkStateChange::get_state() const { return m_state; }
String PlayFabPartyNetworkStateChange::get_reason() const { return m_reason; }

// ---------------------------------------------------------------------------
// PlayFabPartyNetwork

void PlayFabPartyNetwork::_bind_methods() {
    ClassDB::bind_method(D_METHOD("get_network_id"), &PlayFabPartyNetwork::get_network_id);
    ClassDB::bind_method(D_METHOD("get_descriptor"), &PlayFabPartyNetwork::get_descriptor);
    ClassDB::bind_method(D_METHOD("get_state"), &PlayFabPartyNetwork::get_state);
    ClassDB::bind_method(D_METHOD("get_local_user"), &PlayFabPartyNetwork::get_local_user);
    ClassDB::bind_method(D_METHOD("get_local_peer"), &PlayFabPartyNetwork::get_local_peer);
    ClassDB::bind_method(D_METHOD("get_local_chat_control"), &PlayFabPartyNetwork::get_local_chat_control);
    ClassDB::bind_method(D_METHOD("is_host_network"), &PlayFabPartyNetwork::is_host_network);
    ClassDB::bind_method(D_METHOD("leave_async"), &PlayFabPartyNetwork::leave_async);

    ADD_PROPERTY(PropertyInfo(Variant::STRING, "network_id", PROPERTY_HINT_NONE, "", PROPERTY_USAGE_SCRIPT_VARIABLE), "", "get_network_id");
    ADD_PROPERTY(PropertyInfo(Variant::STRING, "descriptor", PROPERTY_HINT_NONE, "", PROPERTY_USAGE_SCRIPT_VARIABLE), "", "get_descriptor");
    ADD_PROPERTY(PropertyInfo(Variant::INT, "state", PROPERTY_HINT_NONE, "", PROPERTY_USAGE_SCRIPT_VARIABLE), "", "get_state");
    ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "local_user", PROPERTY_HINT_RESOURCE_TYPE, "PlayFabUser", PROPERTY_USAGE_SCRIPT_VARIABLE), "", "get_local_user");
    ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "local_peer", PROPERTY_HINT_RESOURCE_TYPE, "PlayFabPartyPeer", PROPERTY_USAGE_SCRIPT_VARIABLE), "", "get_local_peer");
    ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "local_chat_control", PROPERTY_HINT_RESOURCE_TYPE, "PlayFabPartyChatControl", PROPERTY_USAGE_SCRIPT_VARIABLE), "", "get_local_chat_control");
    ADD_PROPERTY(PropertyInfo(Variant::BOOL, "is_host", PROPERTY_HINT_NONE, "", PROPERTY_USAGE_SCRIPT_VARIABLE), "", "is_host_network");

    ADD_SIGNAL(MethodInfo("state_changed", PropertyInfo(Variant::OBJECT, "change", PROPERTY_HINT_RESOURCE_TYPE, "PlayFabPartyNetworkStateChange")));
}

void PlayFabPartyNetwork::set_owner(PlayFabParty *p_owner) { m_owner = p_owner; }

void PlayFabPartyNetwork::set_snapshot(
        const String &p_network_id,
        const String &p_descriptor,
        int64_t p_state,
        const Ref<PlayFabUser> &p_local_user,
        const Ref<PlayFabPartyPeer> &p_local_peer,
        const Ref<PlayFabPartyChatControl> &p_local_chat_control,
        bool p_host) {
    m_network_id = p_network_id;
    m_descriptor = p_descriptor;
    m_state = p_state;
    m_local_user = p_local_user;
    m_local_peer = p_local_peer;
    m_local_chat_control = p_local_chat_control;
    m_host = p_host;
}

void PlayFabPartyNetwork::set_state_value(int64_t p_state) { m_state = p_state; }
void PlayFabPartyNetwork::set_descriptor(const String &p_descriptor) { m_descriptor = p_descriptor; }
void PlayFabPartyNetwork::set_network_id(const String &p_network_id) { m_network_id = p_network_id; }

void PlayFabPartyNetwork::attach_native(
        Party::PartyNetwork *p_network,
        Party::PartyLocalUser *p_local_user,
        Party::PartyLocalEndpoint *p_local_endpoint,
        Party::PartyLocalChatControl *p_local_chat_control) {
    m_native_network = p_network;
    m_native_local_user = p_local_user;
    m_native_local_endpoint = p_local_endpoint;
    m_native_local_chat_control = p_local_chat_control;
    m_destroyed_result = Ref<PlayFabResult>();
}

void PlayFabPartyNetwork::detach_native() {
    m_native_network = nullptr;
    m_native_local_endpoint = nullptr;
    m_native_local_chat_control = nullptr;
    // local_user is owned by PlayFabParty; the wrapper does not unref native resources here.
}

Party::PartyNetwork *PlayFabPartyNetwork::get_native_handle() const { return m_native_network; }
Party::PartyLocalUser *PlayFabPartyNetwork::get_native_local_user() const { return m_native_local_user; }
Party::PartyLocalEndpoint *PlayFabPartyNetwork::get_native_local_endpoint() const { return m_native_local_endpoint; }
Party::PartyLocalChatControl *PlayFabPartyNetwork::get_native_local_chat_control() const { return m_native_local_chat_control; }

String PlayFabPartyNetwork::get_network_id() const { return m_network_id; }
String PlayFabPartyNetwork::get_descriptor() const { return m_descriptor; }
int64_t PlayFabPartyNetwork::get_state() const { return m_state; }
Ref<PlayFabUser> PlayFabPartyNetwork::get_local_user() const { return m_local_user; }
Ref<PlayFabPartyPeer> PlayFabPartyNetwork::get_local_peer() const { return m_local_peer; }
Ref<PlayFabPartyChatControl> PlayFabPartyNetwork::get_local_chat_control() const { return m_local_chat_control; }
bool PlayFabPartyNetwork::is_host_network() const { return m_host; }

Signal PlayFabPartyNetwork::leave_async() {
    if (m_owner == nullptr) {
        return detached_error_signal(E_NOT_VALID_STATE, PARTY_RESOURCE_NOT_READY,
                "PlayFabPartyNetwork.leave_async() requires an owning PlayFabParty service.");
    }
    return m_owner->leave_network_async(Ref<PlayFabPartyNetwork>(this));
}

// ---------------------------------------------------------------------------
// PlayFabPartyPeer

void PlayFabPartyPeer::_bind_methods() {
    ClassDB::bind_method(D_METHOD("get_network"), &PlayFabPartyPeer::get_network);
    ClassDB::bind_method(D_METHOD("get_local_user"), &PlayFabPartyPeer::get_local_user);
    ClassDB::bind_method(D_METHOD("get_descriptor"), &PlayFabPartyPeer::get_descriptor);
    ClassDB::bind_method(D_METHOD("get_peer_entity_key", "peer_id"), &PlayFabPartyPeer::get_peer_entity_key);
    ClassDB::bind_method(D_METHOD("get_peer_member", "peer_id"), &PlayFabPartyPeer::get_peer_member);
    ClassDB::bind_method(D_METHOD("get_peers"), &PlayFabPartyPeer::get_peers);
    ClassDB::bind_method(D_METHOD("get_local_chat_control"), &PlayFabPartyPeer::get_local_chat_control);
    ClassDB::bind_method(D_METHOD("get_peer_chat_control", "peer_id"), &PlayFabPartyPeer::get_peer_chat_control);
    ClassDB::bind_method(D_METHOD("send_text_async", "message", "target_peer_ids", "config"),
            &PlayFabPartyPeer::send_text_async,
            DEFVAL(PackedInt32Array()),
            DEFVAL(Ref<PlayFabPartyTextMessageConfig>()));
    ClassDB::bind_method(D_METHOD("set_peer_chat_permissions_async", "peer_id", "permissions"), &PlayFabPartyPeer::set_peer_chat_permissions_async);
    ClassDB::bind_method(D_METHOD("set_peer_muted_async", "peer_id", "muted"), &PlayFabPartyPeer::set_peer_muted_async);
    ClassDB::bind_method(D_METHOD("close_with_reason", "reason"), &PlayFabPartyPeer::close_with_reason, DEFVAL(String()));

    ADD_SIGNAL(MethodInfo("connection_state_changed", PropertyInfo(Variant::INT, "status")));
    ADD_SIGNAL(MethodInfo("network_error", PropertyInfo(Variant::OBJECT, "result", PROPERTY_HINT_RESOURCE_TYPE, "PlayFabResult")));
    ADD_SIGNAL(MethodInfo("chat_control_added", PropertyInfo(Variant::INT, "peer_id"), PropertyInfo(Variant::OBJECT, "chat_control", PROPERTY_HINT_RESOURCE_TYPE, "PlayFabPartyChatControl")));
    ADD_SIGNAL(MethodInfo("chat_control_removed", PropertyInfo(Variant::INT, "peer_id")));
    ADD_SIGNAL(MethodInfo("text_message_received", PropertyInfo(Variant::INT, "peer_id"), PropertyInfo(Variant::OBJECT, "message", PROPERTY_HINT_RESOURCE_TYPE, "PlayFabPartyChatMessage")));
    ADD_SIGNAL(MethodInfo("transcription_received", PropertyInfo(Variant::INT, "peer_id"), PropertyInfo(Variant::OBJECT, "message", PROPERTY_HINT_RESOURCE_TYPE, "PlayFabPartyChatMessage")));
    ADD_SIGNAL(MethodInfo("chat_permissions_changed", PropertyInfo(Variant::INT, "peer_id"), PropertyInfo(Variant::INT, "permissions")));
    ADD_SIGNAL(MethodInfo("peer_muted_changed", PropertyInfo(Variant::INT, "peer_id"), PropertyInfo(Variant::BOOL, "muted")));
}

void PlayFabPartyPeer::set_network(const Ref<PlayFabPartyNetwork> &p_network) {
    m_network = p_network;
}

void PlayFabPartyPeer::set_unique_id(int32_t p_unique_id) {
    m_unique_id = p_unique_id;
}

void PlayFabPartyPeer::set_connection_status(MultiplayerPeer::ConnectionStatus p_status) {
    if (m_connection_status == p_status) {
        return;
    }
    m_connection_status = p_status;
    emit_signal("connection_state_changed", static_cast<int64_t>(p_status));
}

int32_t PlayFabPartyPeer::allocate_peer_id() {
    return m_next_assigned_peer_id++;
}

bool PlayFabPartyPeer::register_peer(int32_t p_peer_id, Party::PartyEndpoint *p_endpoint, const Dictionary &p_entity_key) {
    if (!insert_peer_record(p_peer_id, p_endpoint, p_entity_key)) {
        return false;
    }
    emit_peer_connected(p_peer_id);
    return true;
}

bool PlayFabPartyPeer::insert_peer_record(int32_t p_peer_id, Party::PartyEndpoint *p_endpoint, const Dictionary &p_entity_key) {
    if (p_peer_id <= 0) {
        return false;
    }
    PeerRecord &record = m_peer_records[p_peer_id];
    record.endpoint = p_endpoint;
    record.entity_key = p_entity_key;
    if (p_peer_id >= m_next_assigned_peer_id) {
        m_next_assigned_peer_id = p_peer_id + 1;
    }
    return true;
}

void PlayFabPartyPeer::emit_peer_connected(int32_t p_peer_id) {
    emit_signal("peer_connected", static_cast<int64_t>(p_peer_id));
}

void PlayFabPartyPeer::update_peer_endpoint(int32_t p_peer_id, Party::PartyEndpoint *p_endpoint) {
    auto it = m_peer_records.find(p_peer_id);
    if (it != m_peer_records.end()) {
        it->second.endpoint = p_endpoint;
    }
}

void PlayFabPartyPeer::update_peer_chat_control(int32_t p_peer_id, const Ref<PlayFabPartyChatControl> &p_chat_control) {
    auto it = m_peer_records.find(p_peer_id);
    if (it == m_peer_records.end()) {
        return;
    }
    it->second.chat_control = p_chat_control;
}

void PlayFabPartyPeer::unregister_peer(int32_t p_peer_id) {
    auto it = m_peer_records.find(p_peer_id);
    if (it == m_peer_records.end()) {
        return;
    }
    m_peer_records.erase(it);
    emit_signal("peer_disconnected", static_cast<int64_t>(p_peer_id));
}

void PlayFabPartyPeer::unregister_endpoint(Party::PartyEndpoint *p_endpoint) {
    if (p_endpoint == nullptr) {
        return;
    }
    for (auto it = m_peer_records.begin(); it != m_peer_records.end(); ++it) {
        if (it->second.endpoint == p_endpoint) {
            int32_t peer_id = it->first;
            m_peer_records.erase(it);
            emit_signal("peer_disconnected", static_cast<int64_t>(peer_id));
            return;
        }
    }
}

int32_t PlayFabPartyPeer::find_peer_by_endpoint(Party::PartyEndpoint *p_endpoint) const {
    if (p_endpoint == nullptr) {
        return 0;
    }
    for (const auto &entry : m_peer_records) {
        if (entry.second.endpoint == p_endpoint) {
            return entry.first;
        }
    }
    return 0;
}

int32_t PlayFabPartyPeer::find_peer_by_entity_key(const Dictionary &p_entity_key) const {
    for (const auto &entry : m_peer_records) {
        if (dictionary_entity_key_equals(entry.second.entity_key, p_entity_key)) {
            return entry.first;
        }
    }
    return 0;
}

int32_t PlayFabPartyPeer::find_peer_by_chat_control(Party::PartyChatControl *p_chat_control) const {
    if (p_chat_control == nullptr) {
        return 0;
    }
    for (const auto &entry : m_peer_records) {
        if (entry.second.chat_control.is_valid() && entry.second.chat_control->get_native_handle() == p_chat_control) {
            return entry.first;
        }
    }
    return 0;
}

Party::PartyEndpoint *PlayFabPartyPeer::get_peer_endpoint(int32_t p_peer_id) const {
    auto it = m_peer_records.find(p_peer_id);
    if (it == m_peer_records.end()) {
        return nullptr;
    }
    return it->second.endpoint;
}

void PlayFabPartyPeer::enqueue_inbound(int32_t p_source_peer, int32_t p_channel, MultiplayerPeer::TransferMode p_mode, const PackedByteArray &p_payload) {
    InboundPacket packet;
    packet.source_peer = p_source_peer;
    packet.channel = p_channel;
    packet.mode = p_mode;
    packet.payload = p_payload;
    m_inbound.push_back(packet);
}

void PlayFabPartyPeer::emit_chat_control_added(int32_t p_peer_id, const Ref<PlayFabPartyChatControl> &p_chat_control) {
    emit_signal("chat_control_added", static_cast<int64_t>(p_peer_id), p_chat_control);
}

void PlayFabPartyPeer::emit_chat_control_removed(int32_t p_peer_id) {
    emit_signal("chat_control_removed", static_cast<int64_t>(p_peer_id));
}

void PlayFabPartyPeer::emit_text_message(int32_t p_peer_id, const Ref<PlayFabPartyChatMessage> &p_message) {
    emit_signal("text_message_received", static_cast<int64_t>(p_peer_id), p_message);
}

void PlayFabPartyPeer::emit_transcription(int32_t p_peer_id, const Ref<PlayFabPartyChatMessage> &p_message) {
    emit_signal("transcription_received", static_cast<int64_t>(p_peer_id), p_message);
}

void PlayFabPartyPeer::emit_chat_permissions_changed(int32_t p_peer_id, int64_t p_permissions) {
    emit_signal("chat_permissions_changed", static_cast<int64_t>(p_peer_id), p_permissions);
}

void PlayFabPartyPeer::emit_peer_muted_changed(int32_t p_peer_id, bool p_muted) {
    emit_signal("peer_muted_changed", static_cast<int64_t>(p_peer_id), p_muted);
}

void PlayFabPartyPeer::emit_network_error(const Ref<PlayFabResult> &p_result) {
    emit_signal("network_error", p_result);
}

Ref<PlayFabPartyNetwork> PlayFabPartyPeer::get_network() const { return m_network; }

Ref<PlayFabUser> PlayFabPartyPeer::get_local_user() const {
    return m_network.is_valid() ? m_network->get_local_user() : Ref<PlayFabUser>();
}

String PlayFabPartyPeer::get_descriptor() const {
    return m_network.is_valid() ? m_network->get_descriptor() : String();
}

Dictionary PlayFabPartyPeer::get_peer_entity_key(int64_t p_peer_id) const {
    auto it = m_peer_records.find(static_cast<int32_t>(p_peer_id));
    if (it == m_peer_records.end()) {
        return Dictionary();
    }
    return it->second.entity_key;
}

Ref<PlayFabPartyMember> PlayFabPartyPeer::get_peer_member(int64_t p_peer_id) const {
    auto it = m_peer_records.find(static_cast<int32_t>(p_peer_id));
    if (it == m_peer_records.end()) {
        return Ref<PlayFabPartyMember>();
    }
    Ref<PlayFabPartyMember> member;
    member.instantiate();
    const bool is_local = static_cast<int32_t>(p_peer_id) == m_unique_id;
    member->set_snapshot(p_peer_id, it->second.entity_key, is_local ? get_local_user() : Ref<PlayFabUser>(), is_local);
    return member;
}

Array PlayFabPartyPeer::get_peers() const {
    Array peers;
    for (const auto &entry : m_peer_records) {
        peers.push_back(static_cast<int64_t>(entry.first));
    }
    return peers;
}

Ref<PlayFabPartyChatControl> PlayFabPartyPeer::get_local_chat_control() const {
    return m_network.is_valid() ? m_network->get_local_chat_control() : Ref<PlayFabPartyChatControl>();
}

Ref<PlayFabPartyChatControl> PlayFabPartyPeer::get_peer_chat_control(int64_t p_peer_id) const {
    auto it = m_peer_records.find(static_cast<int32_t>(p_peer_id));
    if (it == m_peer_records.end()) {
        return Ref<PlayFabPartyChatControl>();
    }
    return it->second.chat_control;
}

Signal PlayFabPartyPeer::send_text_async(const String &p_message, const PackedInt32Array &p_target_peer_ids, const Ref<PlayFabPartyTextMessageConfig> &p_config) {
    if (m_network.is_null() || m_network->m_owner == nullptr || m_connection_status != MultiplayerPeer::CONNECTION_CONNECTED) {
        return detached_error_signal(E_NOT_VALID_STATE, PARTY_PEER_NOT_CONNECTED,
                "PlayFabPartyPeer.send_text_async() requires a connected Party network.");
    }

    Party::PartyLocalChatControl *local = m_network->get_native_local_chat_control();
    if (local == nullptr) {
        return detached_error_signal(E_NOT_VALID_STATE, PARTY_RESOURCE_NOT_READY,
                "PlayFabPartyPeer.send_text_async() requires a local chat control.");
    }

    std::vector<Party::PartyChatControl *> targets;
    if (p_target_peer_ids.size() == 0) {
        for (const auto &entry : m_peer_records) {
            if (entry.second.chat_control.is_valid() && entry.second.chat_control->get_native_handle() != nullptr) {
                targets.push_back(entry.second.chat_control->get_native_handle());
            }
        }
    } else {
        for (int i = 0; i < p_target_peer_ids.size(); ++i) {
            int32_t peer_id = p_target_peer_ids[i];
            auto it = m_peer_records.find(peer_id);
            if (it == m_peer_records.end() || !it->second.chat_control.is_valid() || it->second.chat_control->get_native_handle() == nullptr) {
                return detached_error_signal(E_INVALIDARG, PARTY_PEER_NOT_CONNECTED,
                        String("PlayFabPartyPeer.send_text_async() unknown peer id ") + String::num_int64(peer_id) + ".");
            }
            targets.push_back(it->second.chat_control->get_native_handle());
        }
    }

    return m_network->m_owner->_send_text_via_chat_control(local, targets, p_message, p_config);
}

Signal PlayFabPartyPeer::set_peer_chat_permissions_async(int64_t p_peer_id, int64_t p_permissions) {
    if (m_network.is_null() || m_network->m_owner == nullptr || m_connection_status != MultiplayerPeer::CONNECTION_CONNECTED) {
        return detached_error_signal(E_NOT_VALID_STATE, PARTY_PEER_NOT_CONNECTED,
                "PlayFabPartyPeer.set_peer_chat_permissions_async() requires a connected Party network.");
    }
    auto it = m_peer_records.find(static_cast<int32_t>(p_peer_id));
    if (it == m_peer_records.end() || !it->second.chat_control.is_valid() || it->second.chat_control->get_native_handle() == nullptr) {
        return detached_error_signal(E_INVALIDARG, PARTY_PEER_NOT_CONNECTED,
                String("PlayFabPartyPeer.set_peer_chat_permissions_async() unknown peer id ") + String::num_int64(p_peer_id) + ".");
    }
    Party::PartyLocalChatControl *local = m_network->get_native_local_chat_control();
    if (local == nullptr) {
        return detached_error_signal(E_NOT_VALID_STATE, PARTY_RESOURCE_NOT_READY,
                "PlayFabPartyPeer.set_peer_chat_permissions_async() requires a local chat control.");
    }
    bool succeeded = false;
    Signal completed = m_network->m_owner->_set_chat_permissions(local, it->second.chat_control->get_native_handle(), p_permissions, &succeeded);
    if (succeeded) {
        it->second.permissions = p_permissions;
        emit_signal("chat_permissions_changed", static_cast<int64_t>(p_peer_id), p_permissions);
    }
    return completed;
}

Signal PlayFabPartyPeer::set_peer_muted_async(int64_t p_peer_id, bool p_muted) {
    if (m_network.is_null() || m_network->m_owner == nullptr || m_connection_status != MultiplayerPeer::CONNECTION_CONNECTED) {
        return detached_error_signal(E_NOT_VALID_STATE, PARTY_PEER_NOT_CONNECTED,
                "PlayFabPartyPeer.set_peer_muted_async() requires a connected Party network.");
    }
    auto it = m_peer_records.find(static_cast<int32_t>(p_peer_id));
    if (it == m_peer_records.end() || !it->second.chat_control.is_valid() || it->second.chat_control->get_native_handle() == nullptr) {
        return detached_error_signal(E_INVALIDARG, PARTY_PEER_NOT_CONNECTED,
                String("PlayFabPartyPeer.set_peer_muted_async() unknown peer id ") + String::num_int64(p_peer_id) + ".");
    }
    Party::PartyLocalChatControl *local = m_network->get_native_local_chat_control();
    if (local == nullptr) {
        return detached_error_signal(E_NOT_VALID_STATE, PARTY_RESOURCE_NOT_READY,
                "PlayFabPartyPeer.set_peer_muted_async() requires a local chat control.");
    }
    bool succeeded = false;
    Signal completed = m_network->m_owner->_set_incoming_audio_muted(local, it->second.chat_control->get_native_handle(), p_muted, &succeeded);
    if (succeeded) {
        it->second.muted = p_muted;
        emit_signal("peer_muted_changed", static_cast<int64_t>(p_peer_id), p_muted);
    }
    return completed;
}

void PlayFabPartyPeer::close_with_reason(const String &p_reason) {
    (void)p_reason;
    if (m_network.is_valid() && m_network->m_owner != nullptr) {
        m_network->m_owner->leave_network_async(m_network);
    }
    set_connection_status(MultiplayerPeer::CONNECTION_DISCONNECTED);
    m_inbound.clear();
    m_peer_records.clear();
    m_unique_id = 0;
}

Error PlayFabPartyPeer::_get_packet(const uint8_t **r_buffer, int32_t *r_buffer_size) {
    if (m_inbound.empty()) {
        if (r_buffer != nullptr) {
            *r_buffer = nullptr;
        }
        if (r_buffer_size != nullptr) {
            *r_buffer_size = 0;
        }
        return ERR_UNAVAILABLE;
    }
    InboundPacket packet = m_inbound.front();
    m_inbound.pop_front();
    m_current_packet = packet.payload;
    m_current_packet_peer = packet.source_peer;
    m_current_packet_channel = packet.channel;
    m_current_packet_mode = packet.mode;
    if (r_buffer != nullptr) {
        *r_buffer = m_current_packet.ptr();
    }
    if (r_buffer_size != nullptr) {
        *r_buffer_size = m_current_packet.size();
    }
    return OK;
}

Error PlayFabPartyPeer::_put_packet(const uint8_t *p_buffer, int32_t p_buffer_size) {
    if (m_network.is_null() || m_connection_status != MultiplayerPeer::CONNECTION_CONNECTED) {
        return ERR_UNCONFIGURED;
    }
    Party::PartyLocalEndpoint *local = m_network->get_native_local_endpoint();
    if (local == nullptr) {
        return ERR_UNCONFIGURED;
    }

    PackedByteArray envelope = wrap_gameplay_payload(m_unique_id, m_transfer_channel, m_transfer_mode, p_buffer, static_cast<uint32_t>(p_buffer_size));

    std::vector<Party::PartyEndpoint *> targets;
    if (m_target_peer == MultiplayerPeer::TARGET_PEER_BROADCAST) {
        for (const auto &entry : m_peer_records) {
            if (entry.second.endpoint != nullptr) {
                targets.push_back(entry.second.endpoint);
            }
        }
    } else if (m_target_peer < 0) {
        int32_t excluded = -m_target_peer;
        for (const auto &entry : m_peer_records) {
            if (entry.first != excluded && entry.second.endpoint != nullptr) {
                targets.push_back(entry.second.endpoint);
            }
        }
    } else {
        auto it = m_peer_records.find(m_target_peer);
        if (it != m_peer_records.end() && it->second.endpoint != nullptr) {
            targets.push_back(it->second.endpoint);
        }
    }

    if (targets.empty()) {
        return ERR_UNAVAILABLE;
    }

    Party::PartySendMessageOptions options = Party::PartySendMessageOptions::Default;
    if (m_transfer_mode == MultiplayerPeer::TRANSFER_MODE_RELIABLE) {
        options = static_cast<Party::PartySendMessageOptions>(
                static_cast<uint32_t>(Party::PartySendMessageOptions::GuaranteedDelivery) |
                static_cast<uint32_t>(Party::PartySendMessageOptions::SequentialDelivery));
    } else if (m_transfer_mode == MultiplayerPeer::TRANSFER_MODE_UNRELIABLE_ORDERED) {
        options = Party::PartySendMessageOptions::SequentialDelivery;
    }

    Party::PartyDataBuffer buffer = {};
    buffer.buffer = envelope.ptr();
    buffer.bufferByteCount = static_cast<uint32_t>(envelope.size());

    PartyError err = local->SendMessage(
            static_cast<uint32_t>(targets.size()),
            targets.data(),
            options,
            nullptr,
            1,
            &buffer,
            nullptr);
    if (PARTY_FAILED(err)) {
        return ERR_CANT_RESOLVE;
    }
    return OK;
}

int32_t PlayFabPartyPeer::_get_available_packet_count() const {
    return static_cast<int32_t>(m_inbound.size());
}

int32_t PlayFabPartyPeer::_get_max_packet_size() const {
    return 1024;
}

PackedByteArray PlayFabPartyPeer::_get_packet_script() {
    if (m_inbound.empty()) {
        return PackedByteArray();
    }
    InboundPacket packet = m_inbound.front();
    m_inbound.pop_front();
    m_current_packet = packet.payload;
    m_current_packet_peer = packet.source_peer;
    m_current_packet_channel = packet.channel;
    m_current_packet_mode = packet.mode;
    return packet.payload;
}

Error PlayFabPartyPeer::_put_packet_script(const PackedByteArray &p_buffer) {
    return _put_packet(p_buffer.ptr(), p_buffer.size());
}

int32_t PlayFabPartyPeer::_get_packet_channel() const {
    return m_current_packet_channel;
}

MultiplayerPeer::TransferMode PlayFabPartyPeer::_get_packet_mode() const {
    return m_current_packet_mode;
}

void PlayFabPartyPeer::_set_transfer_channel(int32_t p_channel) {
    m_transfer_channel = p_channel;
}

int32_t PlayFabPartyPeer::_get_transfer_channel() const {
    return m_transfer_channel;
}

void PlayFabPartyPeer::_set_transfer_mode(MultiplayerPeer::TransferMode p_mode) {
    m_transfer_mode = p_mode;
}

MultiplayerPeer::TransferMode PlayFabPartyPeer::_get_transfer_mode() const {
    return m_transfer_mode;
}

void PlayFabPartyPeer::_set_target_peer(int32_t p_peer) {
    m_target_peer = p_peer;
}

int32_t PlayFabPartyPeer::_get_packet_peer() const {
    return m_current_packet_peer;
}

bool PlayFabPartyPeer::_is_server() const {
    return m_unique_id == HOST_PEER_ID;
}

void PlayFabPartyPeer::_poll() {
    // Party state changes are pumped from PlayFab.party.dispatch() and
    // routed into m_inbound. _poll() is a no-op so MultiplayerPeer pulls
    // packets without driving the SDK state-change pump twice.
}

void PlayFabPartyPeer::_close() {
    close_with_reason(String());
}

void PlayFabPartyPeer::_disconnect_peer(int32_t p_peer, bool p_force) {
    (void)p_force;
    unregister_peer(p_peer);
}

int32_t PlayFabPartyPeer::_get_unique_id() const {
    return m_unique_id;
}

void PlayFabPartyPeer::_set_refuse_new_connections(bool p_enable) {
    m_refusing_new_connections = p_enable;
}

bool PlayFabPartyPeer::_is_refusing_new_connections() const {
    return m_refusing_new_connections;
}

bool PlayFabPartyPeer::_is_server_relay_supported() const {
    return false;
}

MultiplayerPeer::ConnectionStatus PlayFabPartyPeer::_get_connection_status() const {
    return m_connection_status;
}

// ---------------------------------------------------------------------------
// PlayFabParty (lifecycle, helpers, validators)

PlayFabParty::PlayFabParty() {
    m_chat.instantiate();
}

PlayFabParty::~PlayFabParty() {
    shutdown();
}

void PlayFabParty::set_owner(PlayFab *p_owner) {
    m_owner = p_owner;
}

bool PlayFabParty::is_initialized() const {
    return m_initialized;
}

PlayFabRuntime *PlayFabParty::_get_runtime() const {
    return m_owner != nullptr ? m_owner->get_runtime() : nullptr;
}

Ref<PlayFabPendingSignal> PlayFabParty::_make_pending_signal() {
    PlayFabRuntime *runtime = _get_runtime();
    if (runtime != nullptr) {
        return runtime->make_pending_signal();
    }
    Ref<PlayFabPendingSignal> pending_signal;
    pending_signal.instantiate();
    return pending_signal;
}

Signal PlayFabParty::_make_error_signal(HRESULT p_hresult, const String &p_code, const String &p_message, const Variant &p_data) {
    PlayFabRuntime *runtime = _get_runtime();
    if (runtime != nullptr) {
        Ref<PlayFabResult> error = PlayFabResult::error_result(p_hresult, p_code, p_message, p_data);
        return runtime->make_error_signal(p_hresult, p_code, p_message, p_data);
    }
    return detached_error_signal(p_hresult, p_code, p_message, p_data);
}

Signal PlayFabParty::_make_ok_signal(const Variant &p_data) {
    Ref<PlayFabPendingSignal> pending_signal = _make_pending_signal();
    pending_signal->complete_deferred(PlayFabResult::ok_result(p_data));
    return pending_signal->get_completed_signal();
}

Ref<PlayFabResult> PlayFabParty::_validate_user(const Ref<PlayFabUser> &p_user) const {
    if (!p_user.is_valid()) {
        return PlayFabResult::error_result(E_INVALIDARG, PARTY_INVALID_USER, "PlayFabParty operation requires a non-null PlayFabUser.");
    }
    if (p_user->get_entity_handle() == nullptr) {
        return PlayFabResult::error_result(E_INVALIDARG, PARTY_INVALID_USER, "PlayFabParty operation requires a signed-in PlayFabUser with an entity handle.");
    }
    return Ref<PlayFabResult>();
}

// Validates a PlayFabPartyConfig.direct_peer_connectivity bitmask against the
// rules documented on PartyDirectPeerConnectivityOptions in the GDK Party
// SDK. Catches the most common authoring mistake — setting platform-type
// flags without an entity-login-provider flag — before reaching the SDK,
// which would otherwise reject the network configuration struct with a
// generic "invalid configuration" PartyError.
bool PlayFabParty::_validate_direct_peer_connectivity(int64_t p_options, String *r_error) {
    constexpr int64_t kPlatformTypeMask = 0x3;
    constexpr int64_t kEntityLoginProviderMask = 0xC;
    constexpr int64_t kOnlyServers = 0x10;
    constexpr int64_t kKnownMask = 0x1F;

    if (r_error == nullptr) {
        return false;
    }

    if ((p_options & ~kKnownMask) != 0) {
        *r_error = vformat("PlayFabPartyConfig.direct_peer_connectivity contains unsupported bits (0x%x). Allowed flags are DIRECT_PEER_CONNECTIVITY_SAME_PLATFORM_TYPE (0x1), DIRECT_PEER_CONNECTIVITY_DIFFERENT_PLATFORM_TYPE (0x2), DIRECT_PEER_CONNECTIVITY_SAME_ENTITY_LOGIN_PROVIDER (0x4), DIRECT_PEER_CONNECTIVITY_DIFFERENT_ENTITY_LOGIN_PROVIDER (0x8), and DIRECT_PEER_CONNECTIVITY_ONLY_SERVERS (0x10).", static_cast<int64_t>(p_options));
        return false;
    }

    const bool has_platform = (p_options & kPlatformTypeMask) != 0;
    const bool has_login_provider = (p_options & kEntityLoginProviderMask) != 0;
    const bool has_only_servers = (p_options & kOnlyServers) != 0;

    if (has_only_servers && (has_platform || has_login_provider)) {
        *r_error = "PlayFabPartyConfig.direct_peer_connectivity cannot combine DIRECT_PEER_CONNECTIVITY_ONLY_SERVERS with platform-type or entity-login-provider flags in the network configuration. Use DIRECT_PEER_CONNECTIVITY_ONLY_SERVERS by itself.";
        return false;
    }

    if (has_platform && !has_login_provider) {
        *r_error = "PlayFabPartyConfig.direct_peer_connectivity has platform-type flags (SAME_PLATFORM_TYPE / DIFFERENT_PLATFORM_TYPE / ANY_PLATFORM_TYPE) but no entity-login-provider flag. Combine with SAME_ENTITY_LOGIN_PROVIDER, DIFFERENT_ENTITY_LOGIN_PROVIDER, or ANY_ENTITY_LOGIN_PROVIDER \u2014 or use DIRECT_PEER_CONNECTIVITY_ANY for the common 'any platform + any login provider' preset.";
        return false;
    }

    if (has_login_provider && !has_platform) {
        *r_error = "PlayFabPartyConfig.direct_peer_connectivity has entity-login-provider flags but no platform-type flag. Combine with SAME_PLATFORM_TYPE, DIFFERENT_PLATFORM_TYPE, or ANY_PLATFORM_TYPE \u2014 or use DIRECT_PEER_CONNECTIVITY_ANY for the common 'any platform + any login provider' preset.";
        return false;
    }

    return true;
}

HRESULT PlayFabParty::_ensure_initialized(int p_local_udp_port_override) {
    if (m_initialized) {
        return S_OK;
    }
    PlayFabRuntime *runtime = _get_runtime();
    if (runtime == nullptr || !runtime->is_initialized()) {
        return E_NOT_VALID_STATE;
    }

    // Opt-in override for PartyOption::LocalUdpSocketBindAddress. Default
    // (no override) leaves the SDK to pick its preferred bind address —
    // the right choice for shipping games. Multi-process same-host
    // scenarios (CI test orchestrators, splitscreen dev iteration) opt
    // in via the explicit initialize_async() port arg or via the
    // [code]playfab/party/local_udp_socket_bind_port[/code] project
    // setting. Per Party.h:1009 it's "safe and recommended" to override
    // this option PRIOR to initializing the Party library, so we apply
    // it BEFORE the Initialize() call below.
    int resolved_port = p_local_udp_port_override;
    if (resolved_port < 0) {
        ProjectSettings *project_settings = ProjectSettings::get_singleton();
        if (project_settings != nullptr) {
            resolved_port = static_cast<int>(project_settings->get_setting(
                    "playfab/party/local_udp_socket_bind_port", -1));
        }
    }
    if (resolved_port > 65535) {
        ERR_PRINT(vformat("PlayFab.party: local_udp_socket_bind_port=%d is out of range (0-65535); falling back to the SDK's default bind address.",
                resolved_port));
        resolved_port = -1;
    }
    if (resolved_port >= 0) {
        Party::PartyLocalUdpSocketBindAddressConfiguration bind_config = {};
        // The ExcludeGameCorePreferredUdpMultiplayerPort flag is documented
        // (Party.h:2787-2791) to only be valid when port == 0 on the
        // Microsoft Game Core build of the Party library. Auto-derive it
        // here: when the caller asks for OS-picked (port == 0), exclude
        // the Game Core preferred port so the OS selection never lands
        // on the platform's reserved slot. For specific ports we MUST
        // NOT set the flag per the same doc.
        bind_config.options = (resolved_port == 0)
                ? Party::PartyLocalUdpSocketBindAddressOptions::ExcludeGameCorePreferredUdpMultiplayerPort
                : Party::PartyLocalUdpSocketBindAddressOptions::None;
        bind_config.port = static_cast<uint16_t>(resolved_port);
        PartyError opt_err = Party::PartyManager::GetSingleton().SetOption(
                nullptr,
                Party::PartyOption::LocalUdpSocketBindAddress,
                &bind_config);
        if (PARTY_FAILED(opt_err)) {
            PartyString opt_err_msg = nullptr;
            Party::PartyManager::GetErrorMessage(opt_err, &opt_err_msg);
            ERR_PRINT(vformat("PlayFab.party: SetOption(LocalUdpSocketBindAddress, port=%d) returned 0x%x (%s). Falling back to the SDK's default bind address.",
                    resolved_port,
                    static_cast<int64_t>(opt_err),
                    String(opt_err_msg != nullptr ? opt_err_msg : "<no message>")));
        }
    }

    const CharString title_id_utf8 = runtime->get_title_id().utf8();
    Party::PartyInitializationConfiguration init_config = {};
    init_config.titleId = title_id_utf8.get_data();
    init_config.audioTaskQueue = nullptr;
    init_config.networkingTaskQueue = nullptr;
    PartyError err = Party::PartyManager::GetSingleton().Initialize(&init_config);
    if (PARTY_FAILED(err)) {
        return E_FAIL;
    }

    m_initialized = true;
    return S_OK;
}

Signal PlayFabParty::initialize_async(const Ref<PlayFabPartyConfig> &p_config, int p_local_udp_port) {
    (void)p_config;
    PlayFabRuntime *runtime = _get_runtime();
    if (runtime == nullptr || !runtime->is_initialized()) {
        return _make_error_signal(E_NOT_VALID_STATE, PARTY_NOT_INITIALIZED,
                "PlayFab.party.initialize_async() requires PlayFab.initialize() to succeed first.");
    }
    if (m_initialized) {
        return _make_error_signal(E_FAIL, PARTY_ALREADY_INITIALIZED,
                "PlayFab.party is already initialized.");
    }

    HRESULT hr = _ensure_initialized(p_local_udp_port);
    if (FAILED(hr)) {
        return _make_error_signal(hr, PARTY_NOT_INITIALIZED,
                "Failed to initialize PlayFab Party (PartyManager::Initialize).");
    }
    return _make_ok_signal();
}

Signal PlayFabParty::shutdown_async() {
    Ref<PlayFabPendingSignal> pending_signal = _make_pending_signal();
    shutdown();
    pending_signal->complete_deferred(PlayFabResult::ok_result());
    return pending_signal->get_completed_signal();
}

void PlayFabParty::shutdown() {
    if (!m_initialized && m_pending_operations.empty() && m_networks.empty() && m_local_users.empty()) {
        return;
    }

    m_shutting_down = true;

    for (PendingOperation *operation : m_pending_operations) {
        if (operation != nullptr && operation->pending_signal.is_valid()) {
            operation->pending_signal->complete(PlayFabResult::cancelled("PlayFab Party is shutting down."));
        }
    }

    for (const Ref<PlayFabPartyNetwork> &network : m_networks) {
        if (network.is_valid() && network->get_native_handle() != nullptr) {
            network->get_native_handle()->LeaveNetwork(nullptr);
        }
    }

    for (int attempt = 0; attempt < 50 && m_initialized; ++attempt) {
        dispatch();
        bool any_active = false;
        for (const Ref<PlayFabPartyNetwork> &network : m_networks) {
            if (network.is_valid() && network->get_native_handle() != nullptr) {
                any_active = true;
                break;
            }
        }
        if (!any_active && m_pending_operations.empty()) {
            break;
        }
        Sleep(10);
    }

    for (PendingOperation *operation : m_pending_operations) {
        delete operation;
    }
    m_pending_operations.clear();

    for (const Ref<PlayFabPartyNetwork> &network : m_networks) {
        if (network.is_valid()) {
            network->detach_native();
            network->set_owner(nullptr);
        }
    }
    m_networks.clear();

    if (m_chat.is_valid()) {
        m_chat->clear();
    }

    m_orphan_chat_controls.clear();

    _release_all_local_users();

    if (m_initialized) {
        Party::PartyManager::GetSingleton().Cleanup();
        m_initialized = false;
    }

    m_processing_state_changes = false;
    m_shutting_down = false;
}

int PlayFabParty::dispatch() {
    if (!m_initialized || m_processing_state_changes) {
        return 0;
    }
    return _pump_state_changes();
}

Ref<PlayFabPartyChat> PlayFabParty::get_chat() const {
    return m_chat;
}

Array PlayFabParty::get_networks() const {
    Array networks;
    for (const Ref<PlayFabPartyNetwork> &network : m_networks) {
        networks.push_back(network);
    }
    return networks;
}

void PlayFabParty::_track_network(const Ref<PlayFabPartyNetwork> &p_network) {
    if (!p_network.is_valid()) {
        return;
    }
    for (const Ref<PlayFabPartyNetwork> &existing : m_networks) {
        if (existing == p_network) {
            return;
        }
    }
    m_networks.push_back(p_network);
}

void PlayFabParty::_untrack_network(const Ref<PlayFabPartyNetwork> &p_network) {
    if (!p_network.is_valid()) {
        return;
    }
    for (auto it = m_networks.begin(); it != m_networks.end(); ++it) {
        if (*it == p_network) {
            m_networks.erase(it);
            return;
        }
    }
}

Ref<PlayFabPartyNetwork> PlayFabParty::_find_network_by_native(Party::PartyNetwork *p_native) const {
    if (p_native == nullptr) {
        return Ref<PlayFabPartyNetwork>();
    }
    for (const Ref<PlayFabPartyNetwork> &network : m_networks) {
        if (network.is_valid() && network->get_native_handle() == p_native) {
            return network;
        }
    }
    return Ref<PlayFabPartyNetwork>();
}

Party::PartyLocalUser *PlayFabParty::_get_or_create_local_user(const Ref<PlayFabUser> &p_user, String *r_error) {
    if (!p_user.is_valid() || p_user->get_entity_handle() == nullptr) {
        if (r_error != nullptr) {
            *r_error = "PlayFabUser has no entity handle.";
        }
        return nullptr;
    }
    PFEntityHandle handle = p_user->get_entity_handle();
    auto it = m_local_users.find(handle);
    if (it != m_local_users.end()) {
        return it->second;
    }
    Party::PartyLocalUser *local_user = nullptr;
    PartyError err = Party::PartyManager::GetSingleton().CreateLocalUser(handle, &local_user);
    if (PARTY_FAILED(err) || local_user == nullptr) {
        if (r_error != nullptr) {
            *r_error = _party_error_message(err);
        }
        return nullptr;
    }
    m_local_users[handle] = local_user;
    return local_user;
}

void PlayFabParty::_release_local_user(PFEntityHandle p_handle) {
    auto it = m_local_users.find(p_handle);
    if (it == m_local_users.end()) {
        return;
    }
    if (it->second != nullptr) {
        Party::PartyManager::GetSingleton().DestroyLocalUser(it->second, nullptr);
    }
    m_local_users.erase(it);
}

void PlayFabParty::_release_all_local_users() {
    for (auto &entry : m_local_users) {
        if (entry.second != nullptr) {
            Party::PartyManager::GetSingleton().DestroyLocalUser(entry.second, nullptr);
        }
    }
    m_local_users.clear();
}

void PlayFabParty::_reset_after_state_change_finish_failure(const Ref<PlayFabResult> &p_result) {
    Ref<PlayFabResult> result = p_result;
    if (result.is_null()) {
        result = PlayFabResult::error_result(E_FAIL, PARTY_STATE_FINISH_FAILED, "PartyManager::FinishProcessingStateChanges failed.");
    }

    if (m_initialized) {
        Party::PartyManager::GetSingleton().Cleanup();
        m_initialized = false;
    }
    m_local_users.clear();
    m_orphan_chat_controls.clear();
    if (m_chat.is_valid()) {
        m_chat->clear();
    }

    std::vector<Ref<PlayFabPartyNetwork>> networks;
    networks.swap(m_networks);
    for (const Ref<PlayFabPartyNetwork> &network : networks) {
        if (!network.is_valid()) {
            continue;
        }
        network->set_state_value(NETWORK_STATE_FAILED);
        network->detach_native();
        _emit_network_state(network, NETWORK_CHANGE_ERROR, 0, result, "PlayFab Party state processing failed; PlayFab.party was reset.");
        network->set_owner(nullptr);
    }

    std::vector<PendingOperation *> pending_operations;
    pending_operations.swap(m_pending_operations);
    for (PendingOperation *operation : pending_operations) {
        if (operation != nullptr && operation->pending_signal.is_valid()) {
            operation->pending_signal->complete(result);
        }
        delete operation;
    }

    ERR_PRINT("PlayFab.party: FinishProcessingStateChanges failed; Party was reset. Call PlayFab.party.initialize_async() before using it again.");
    emit_signal("party_error", result);
    m_processing_state_changes = false;
    m_shutting_down = false;
}

PlayFabParty::PendingOperation *PlayFabParty::_create_pending(int32_t p_kind) {
    PendingOperation *operation = new PendingOperation;
    operation->kind = p_kind;
    operation->pending_signal = _make_pending_signal();
    m_pending_operations.push_back(operation);
    return operation;
}

void PlayFabParty::_release_pending(PendingOperation *p_operation) {
    if (p_operation == nullptr) {
        return;
    }
    auto it = std::find(m_pending_operations.begin(), m_pending_operations.end(), p_operation);
    if (it != m_pending_operations.end()) {
        m_pending_operations.erase(it);
    }
    delete p_operation;
}

void PlayFabParty::_complete_pending(PendingOperation *p_operation, const Ref<PlayFabResult> &p_result) {
    if (p_operation == nullptr) {
        return;
    }
    if (p_operation->pending_signal.is_valid()) {
        Ref<PlayFabResult> final_result = p_result;
        if (p_operation->pending_signal->was_cancel_requested()) {
            final_result = PlayFabResult::cancelled("PlayFab Party operation cancelled.");
        }
        p_operation->pending_signal->complete(final_result);
    }
    _release_pending(p_operation);
}

PlayFabParty::PendingOperation *PlayFabParty::_find_pending(int32_t p_kind, Party::PartyNetwork *p_native_network) {
    for (PendingOperation *op : m_pending_operations) {
        if (op != nullptr && op->kind == p_kind && op->native_network == p_native_network) {
            return op;
        }
    }
    return nullptr;
}

PlayFabParty::PendingOperation *PlayFabParty::_find_pending_join(Party::PartyNetwork *p_native_network) {
    for (PendingOperation *op : m_pending_operations) {
        if (op != nullptr && op->native_network == p_native_network &&
                (op->kind == PENDING_JOIN_HANDSHAKE ||
                        op->kind == PENDING_CREATE_ENDPOINT ||
                        op->kind == PENDING_AUTHENTICATE ||
                        op->kind == PENDING_CONNECT_NETWORK ||
                        op->kind == PENDING_CONNECT_CHAT_CONTROL ||
                        op->kind == PENDING_CREATE_CHAT_CONTROL)) {
            return op;
        }
    }
    return nullptr;
}

bool PlayFabParty::_abort_join_op_if_network_dead(PendingOperation *p_operation) {
    if (p_operation == nullptr) {
        return false;
    }
    // The target network is considered dead when the wrapper has been detached
    // (PartyNetworkDestroyed already processed). detach_native() nulls the
    // wrapper's native handle, so an in-flight join-chain *Completed delivered
    // in the same batch as NetworkDestroyed must not be allowed to dereference
    // the stale native_network pointer.
    if (p_operation->network.is_null() || p_operation->network->get_native_handle() != nullptr) {
        return false;
    }
    // _process_network_destroyed already emitted NETWORK_CHANGE_DESTROYED on
    // this wrapper; do not emit a second NETWORK_CHANGE_ERROR. Resolve the
    // awaiting join_*_async() call with the actual destruction reason if we
    // captured one on the wrapper, falling back to a generic message otherwise.
    Ref<PlayFabResult> result = p_operation->network->m_destroyed_result;
    if (result.is_null()) {
        result = PlayFabResult::error_result(E_FAIL, PARTY_RESOURCE_NOT_READY, "Network destroyed during join.");
    }
    _complete_pending(p_operation, result);
    return true;
}

// ---------------------------------------------------------------------------
// PlayFabParty (network operations)

Signal PlayFabParty::create_and_join_network_async(const Ref<PlayFabUser> &p_user, const Ref<PlayFabPartyConfig> &p_config) {
    // Validate the caller-supplied config bitmask first so the error
    // message points at a static, code-authored mistake instead of a
    // transient sign-in / initialization state issue.
    Ref<PlayFabPartyConfig> config = p_config;
    if (config.is_null()) {
        config.instantiate();
    }
    String connectivity_error;
    if (!_validate_direct_peer_connectivity(config->get_direct_peer_connectivity(), &connectivity_error)) {
        return _make_error_signal(E_INVALIDARG, PARTY_INVALID_OPTIONS, connectivity_error);
    }

    Ref<PlayFabResult> validation = _validate_user(p_user);
    if (validation.is_valid() && !validation->is_ok()) {
        if (_get_runtime() != nullptr) {
        }
        Ref<PlayFabPendingSignal> pending_signal = _make_pending_signal();
        pending_signal->complete_deferred(validation);
        return pending_signal->get_completed_signal();
    }

    HRESULT hr = _ensure_initialized();
    if (FAILED(hr)) {
        return _make_error_signal(hr, PARTY_NOT_INITIALIZED,
                "PlayFab.party requires initialization before creating a network.");
    }

    String error_text;
    Party::PartyLocalUser *local_user = _get_or_create_local_user(p_user, &error_text);
    if (local_user == nullptr) {
        return _make_error_signal(E_FAIL, PARTY_INVALID_USER,
                String("Failed to create Party local user: ") + error_text);
    }

    Party::PartyNetworkConfiguration net_config = {};
    net_config.maxUserCount = static_cast<uint32_t>(std::max<int64_t>(2, std::min<int64_t>(128, config->get_max_players())));
    net_config.maxDeviceCount = net_config.maxUserCount;
    net_config.maxUsersPerDeviceCount = 1;
    net_config.maxDevicesPerUserCount = 1;
    net_config.maxEndpointsPerDeviceCount = 1;
    net_config.directPeerConnectivityOptions = static_cast<Party::PartyDirectPeerConnectivityOptions>(config->get_direct_peer_connectivity());

    Party::PartyInvitationConfiguration invite_config = {};
    const CharString invite_id_utf8 = config->get_invitation_id().utf8();
    invite_config.identifier = invite_id_utf8.length() > 0 ? invite_id_utf8.get_data() : nullptr;
    invite_config.revocability = Party::PartyInvitationRevocability::Anyone;
    invite_config.entityIdCount = 0;
    invite_config.entityIds = nullptr;

    PendingOperation *operation = _create_pending(PENDING_CREATE_NETWORK);
    operation->user = p_user;
    operation->config = config;
    operation->host = true;
    operation->native_user = local_user;
    operation->invitation_id = config->get_invitation_id();
    operation->network.instantiate();
    operation->network->set_owner(this);
    operation->network->set_state_value(NETWORK_STATE_CREATING);
    operation->network->m_local_user = p_user;
    operation->network->m_host = true;
    operation->network->m_native_local_user = local_user;

    Party::PartyNetworkDescriptor descriptor = {};
    char applied_invitation_id[Party::c_maxInvitationIdentifierStringLength + 1] = {};
    PartyError err = Party::PartyManager::GetSingleton().CreateNewNetwork(
            local_user,
            &net_config,
            0,
            nullptr,
            &invite_config,
            operation,
            &descriptor,
            applied_invitation_id);
    if (PARTY_FAILED(err)) {
        Ref<PlayFabResult> result = _party_error_result(err, PARTY_NETWORK_CREATE_FAILED, "PartyManager::CreateNewNetwork");
        _complete_pending(operation, result);
        return _make_error_signal(E_FAIL, PARTY_NETWORK_CREATE_FAILED, result.is_valid() ? result->get_message() : String("PartyManager::CreateNewNetwork failed."));
    }

    operation->invitation_id = String::utf8(applied_invitation_id);
    operation->network->set_network_id(String::utf8(descriptor.networkIdentifier));

    Party::PartyNetwork *network_handle = nullptr;
    err = Party::PartyManager::GetSingleton().ConnectToNetwork(&descriptor, operation, &network_handle);
    if (PARTY_FAILED(err)) {
        Ref<PlayFabResult> result = _party_error_result(err, PARTY_NETWORK_CONNECT_FAILED, "PartyManager::ConnectToNetwork");
        _complete_pending(operation, result);
        return _make_error_signal(E_FAIL, PARTY_NETWORK_CONNECT_FAILED, result.is_valid() ? result->get_message() : String("PartyManager::ConnectToNetwork failed."));
    }

    operation->kind = PENDING_CONNECT_NETWORK;
    operation->native_network = network_handle;
    operation->network->attach_native(network_handle, local_user, nullptr, nullptr);
    operation->network->set_state_value(NETWORK_STATE_CONNECTING);
    _track_network(operation->network);
    return operation->pending_signal->get_completed_signal();
}

Signal PlayFabParty::join_network_async(const Ref<PlayFabUser> &p_user, const String &p_descriptor, const Ref<PlayFabPartyConfig> &p_config) {
    Ref<PlayFabResult> validation = _validate_user(p_user);
    if (validation.is_valid() && !validation->is_ok()) {
        if (_get_runtime() != nullptr) {
        }
        Ref<PlayFabPendingSignal> pending_signal = _make_pending_signal();
        pending_signal->complete_deferred(validation);
        return pending_signal->get_completed_signal();
    }

    if (p_descriptor.is_empty()) {
        return _make_error_signal(E_INVALIDARG, PARTY_DESCRIPTOR_INVALID,
                "PlayFab.party.join_network_async() requires a non-empty descriptor string.");
    }

    HRESULT hr = _ensure_initialized();
    if (FAILED(hr)) {
        return _make_error_signal(hr, PARTY_NOT_INITIALIZED,
                "PlayFab.party requires initialization before joining a network.");
    }

    Party::PartyNetworkDescriptor descriptor = {};
    const CharString descriptor_utf8 = p_descriptor.utf8();
    PartyError deserialize_err = Party::PartyManager::GetSingleton().DeserializeNetworkDescriptor(descriptor_utf8.get_data(), &descriptor);
    if (PARTY_FAILED(deserialize_err)) {
        return _make_error_signal(E_INVALIDARG, PARTY_DESCRIPTOR_INVALID,
                String("Failed to deserialize PartyNetworkDescriptor: ") + _party_error_message(deserialize_err));
    }

    Ref<PlayFabPartyConfig> config = p_config;
    if (config.is_null()) {
        config.instantiate();
    }

    String error_text;
    Party::PartyLocalUser *local_user = _get_or_create_local_user(p_user, &error_text);
    if (local_user == nullptr) {
        return _make_error_signal(E_FAIL, PARTY_INVALID_USER,
                String("Failed to create Party local user: ") + error_text);
    }

    PendingOperation *operation = _create_pending(PENDING_CONNECT_NETWORK);
    operation->user = p_user;
    operation->config = config;
    operation->host = false;
    operation->descriptor = p_descriptor;
    operation->native_user = local_user;
    operation->invitation_id = config->get_invitation_id();
    operation->network.instantiate();
    operation->network->set_owner(this);
    operation->network->set_state_value(NETWORK_STATE_CONNECTING);
    operation->network->m_local_user = p_user;
    operation->network->m_host = false;
    operation->network->m_native_local_user = local_user;
    operation->network->set_descriptor(p_descriptor);
    operation->network->set_network_id(String::utf8(descriptor.networkIdentifier));

    Party::PartyNetwork *network_handle = nullptr;
    PartyError err = Party::PartyManager::GetSingleton().ConnectToNetwork(&descriptor, operation, &network_handle);
    if (PARTY_FAILED(err)) {
        Ref<PlayFabResult> result = _party_error_result(err, PARTY_NETWORK_CONNECT_FAILED, "PartyManager::ConnectToNetwork");
        _complete_pending(operation, result);
        return _make_error_signal(E_FAIL, PARTY_NETWORK_CONNECT_FAILED, result.is_valid() ? result->get_message() : String("PartyManager::ConnectToNetwork failed."));
    }

    operation->native_network = network_handle;
    operation->network->attach_native(network_handle, local_user, nullptr, nullptr);
    _track_network(operation->network);
    return operation->pending_signal->get_completed_signal();
}

Signal PlayFabParty::leave_network_async(const Ref<PlayFabPartyNetwork> &p_network) {
    if (!p_network.is_valid()) {
        return _make_error_signal(E_INVALIDARG, PARTY_INVALID_OPTIONS,
                "PlayFab.party.leave_network_async() requires a non-null network.");
    }

    Ref<PlayFabPartyNetwork> tracked;
    for (const Ref<PlayFabPartyNetwork> &existing : m_networks) {
        if (existing == p_network) {
            tracked = existing;
            break;
        }
    }
    if (!tracked.is_valid() || tracked->get_native_handle() == nullptr) {
        return _make_error_signal(E_NOT_VALID_STATE, PARTY_RESOURCE_NOT_READY,
                "PlayFab.party.leave_network_async() received a network with no active resources.");
    }

    PendingOperation *operation = _create_pending(PENDING_LEAVE_NETWORK);
    operation->network = tracked;
    operation->native_network = tracked->get_native_handle();

    PartyError err = tracked->get_native_handle()->LeaveNetwork(operation);
    if (PARTY_FAILED(err)) {
        Ref<PlayFabResult> result = _party_error_result(err, PARTY_RESOURCE_NOT_READY, "PartyNetwork::LeaveNetwork");
        _complete_pending(operation, result);
        return _make_error_signal(E_FAIL, PARTY_RESOURCE_NOT_READY, result.is_valid() ? result->get_message() : String("PartyNetwork::LeaveNetwork failed."));
    }

    tracked->set_state_value(NETWORK_STATE_DISCONNECTING);
    return operation->pending_signal->get_completed_signal();
}

// ---------------------------------------------------------------------------
// PlayFabParty (state-change pump and routing)

int PlayFabParty::_pump_state_changes() {
    if (!m_initialized) {
        return 0;
    }
    if (m_processing_state_changes) {
        return 0;
    }
    m_processing_state_changes = true;

    uint32_t change_count = 0;
    Party::PartyStateChangeArray changes = nullptr;
    PartyError err = Party::PartyManager::GetSingleton().StartProcessingStateChanges(&change_count, &changes);
    if (PARTY_FAILED(err)) {
        m_processing_state_changes = false;
        return 0;
    }

    int processed = 0;
    for (uint32_t i = 0; i < change_count; ++i) {
        const Party::PartyStateChange *change = changes[i];
        if (change == nullptr) {
            continue;
        }
        _process_state_change(change);
        ++processed;
    }

    err = Party::PartyManager::GetSingleton().FinishProcessingStateChanges(change_count, changes);
    if (PARTY_FAILED(err)) {
        Ref<PlayFabResult> result = _party_error_result(err, PARTY_STATE_FINISH_FAILED, "PartyManager::FinishProcessingStateChanges");
        _reset_after_state_change_finish_failure(result);
        return processed;
    }
    m_processing_state_changes = false;
    return processed;
}

void PlayFabParty::_process_state_change(const Party::PartyStateChange *p_change) {
    if (p_change == nullptr) {
        return;
    }
    switch (p_change->stateChangeType) {
        case Party::PartyStateChangeType::CreateNewNetworkCompleted:
            _process_create_new_network_completed(p_change);
            break;
        case Party::PartyStateChangeType::ConnectToNetworkCompleted:
            _process_connect_to_network_completed(p_change);
            break;
        case Party::PartyStateChangeType::AuthenticateLocalUserCompleted:
            _process_authenticate_local_user_completed(p_change);
            break;
        case Party::PartyStateChangeType::CreateEndpointCompleted:
            _process_create_endpoint_completed(p_change);
            break;
        case Party::PartyStateChangeType::EndpointCreated:
            _process_endpoint_created(p_change);
            break;
        case Party::PartyStateChangeType::EndpointDestroyed:
            _process_endpoint_destroyed(p_change);
            break;
        case Party::PartyStateChangeType::EndpointMessageReceived:
            _process_endpoint_message_received(p_change);
            break;
        case Party::PartyStateChangeType::NetworkDescriptorChanged:
            _process_network_descriptor_changed(p_change);
            break;
        case Party::PartyStateChangeType::LeaveNetworkCompleted:
            _process_leave_network_completed(p_change);
            break;
        case Party::PartyStateChangeType::NetworkDestroyed:
            _process_network_destroyed(p_change);
            break;
        case Party::PartyStateChangeType::CreateChatControlCompleted:
            _process_create_chat_control_completed(p_change);
            break;
        case Party::PartyStateChangeType::ConnectChatControlCompleted:
            _process_connect_chat_control_completed(p_change);
            break;
        case Party::PartyStateChangeType::ChatControlCreated:
            _process_chat_control_created(p_change);
            break;
        case Party::PartyStateChangeType::ChatControlDestroyed:
            _process_chat_control_destroyed(p_change);
            break;
        case Party::PartyStateChangeType::ChatTextReceived:
            _process_chat_text_received(p_change);
            break;
        case Party::PartyStateChangeType::VoiceChatTranscriptionReceived:
            _process_voice_chat_transcription_received(p_change);
            break;
        default:
            // Other state-change types (RegionsChanged, etc.) are absorbed silently.
            break;
    }
}

// ---------------------------------------------------------------------------
// PlayFabParty (state-change handlers)

void PlayFabParty::_process_create_new_network_completed(const Party::PartyStateChange *p_change) {
    const auto *change = static_cast<const Party::PartyCreateNewNetworkCompletedStateChange *>(p_change);
    PendingOperation *operation = static_cast<PendingOperation *>(change->asyncIdentifier);
    if (operation == nullptr) {
        return;
    }
    if (change->result != Party::PartyStateChangeResult::Succeeded) {
        Ref<PlayFabResult> result = _party_error_result(change->errorDetail, PARTY_NETWORK_CREATE_FAILED, "PartyCreateNewNetwork");
        if (operation->network.is_valid()) {
            operation->network->set_state_value(NETWORK_STATE_FAILED);
            _emit_network_state(operation->network, NETWORK_CHANGE_ERROR, 0, result, "Create new network failed.");
            _untrack_network(operation->network);
            operation->network->detach_native();
        }
        _complete_pending(operation, result);
    }
    // Success path: ConnectToNetwork was already invoked synchronously after CreateNewNetwork.
}

void PlayFabParty::_process_connect_to_network_completed(const Party::PartyStateChange *p_change) {
    const auto *change = static_cast<const Party::PartyConnectToNetworkCompletedStateChange *>(p_change);
    PendingOperation *operation = static_cast<PendingOperation *>(change->asyncIdentifier);
    if (operation == nullptr) {
        return;
    }
    if (_abort_join_op_if_network_dead(operation)) {
        return;
    }
    if (change->result != Party::PartyStateChangeResult::Succeeded) {
        Ref<PlayFabResult> result = _party_error_result(change->errorDetail, PARTY_NETWORK_CONNECT_FAILED, "PartyConnectToNetwork");
        if (operation->network.is_valid()) {
            operation->network->set_state_value(NETWORK_STATE_FAILED);
            _emit_network_state(operation->network, NETWORK_CHANGE_ERROR, 0, result, "Connect to network failed.");
            _untrack_network(operation->network);
            operation->network->detach_native();
        }
        _complete_pending(operation, result);
        return;
    }

    if (operation->network.is_valid()) {
        operation->native_network = change->network;
        operation->network->attach_native(change->network, operation->native_user, nullptr, nullptr);
        operation->network->set_state_value(NETWORK_STATE_AUTHENTICATING);
        _emit_network_state(operation->network, NETWORK_CHANGE_STATE, 0, Ref<PlayFabResult>(), "connecting");
    }

    PartyError err = change->network->AuthenticateLocalUser(
            operation->native_user,
            operation->invitation_id.utf8().get_data(),
            operation);
    if (PARTY_FAILED(err)) {
        Ref<PlayFabResult> result = _party_error_result(err, PARTY_NETWORK_CONNECT_FAILED, "PartyNetwork::AuthenticateLocalUser");
        if (operation->network.is_valid()) {
            operation->network->set_state_value(NETWORK_STATE_FAILED);
            _emit_network_state(operation->network, NETWORK_CHANGE_ERROR, 0, result, "Authenticate dispatch failed.");
            _untrack_network(operation->network);
            operation->network->detach_native();
        }
        _complete_pending(operation, result);
        return;
    }
    operation->kind = PENDING_AUTHENTICATE;
}

void PlayFabParty::_process_authenticate_local_user_completed(const Party::PartyStateChange *p_change) {
    const auto *change = static_cast<const Party::PartyAuthenticateLocalUserCompletedStateChange *>(p_change);
    PendingOperation *operation = static_cast<PendingOperation *>(change->asyncIdentifier);
    if (operation == nullptr) {
        return;
    }
    if (_abort_join_op_if_network_dead(operation)) {
        return;
    }
    if (change->result != Party::PartyStateChangeResult::Succeeded) {
        Ref<PlayFabResult> result = _party_error_result(change->errorDetail, PARTY_NETWORK_CONNECT_FAILED, "PartyAuthenticateLocalUser");
        if (operation->network.is_valid()) {
            operation->network->set_state_value(NETWORK_STATE_FAILED);
            _emit_network_state(operation->network, NETWORK_CHANGE_ERROR, 0, result, "Authenticate failed.");
            _untrack_network(operation->network);
            operation->network->detach_native();
        }
        _complete_pending(operation, result);
        return;
    }

    bool need_chat = false;
    if (operation->config.is_valid()) {
        need_chat = operation->config->is_voice_chat_enabled() || operation->config->is_text_chat_enabled();
    }

    if (need_chat) {
        HRESULT hr = _start_create_chat_control_step(operation);
        if (FAILED(hr)) {
            Ref<PlayFabResult> result = PlayFabResult::error_result(hr, PARTY_CHAT_CONTROL_CREATE_FAILED, "Failed to create chat control.");
            if (operation->network.is_valid()) {
                _emit_network_state(operation->network, NETWORK_CHANGE_ERROR, 0, result, "Chat control creation failed.");
            }
            _complete_pending(operation, result);
        }
        return;
    }

    HRESULT hr = _start_create_endpoint_step(operation);
    if (FAILED(hr)) {
        Ref<PlayFabResult> result = PlayFabResult::error_result(hr, PARTY_TRANSPORT_CREATE_FAILED, "Failed to create endpoint.");
        if (operation->network.is_valid()) {
            _emit_network_state(operation->network, NETWORK_CHANGE_ERROR, 0, result, "Endpoint creation failed.");
        }
        _complete_pending(operation, result);
    }
}

void PlayFabParty::_process_create_endpoint_completed(const Party::PartyStateChange *p_change) {
    const auto *change = static_cast<const Party::PartyCreateEndpointCompletedStateChange *>(p_change);
    PendingOperation *operation = static_cast<PendingOperation *>(change->asyncIdentifier);
    if (operation == nullptr) {
        return;
    }
    if (_abort_join_op_if_network_dead(operation)) {
        return;
    }
    if (change->result != Party::PartyStateChangeResult::Succeeded) {
        Ref<PlayFabResult> result = _party_error_result(change->errorDetail, PARTY_TRANSPORT_CREATE_FAILED, "PartyCreateEndpoint");
        if (operation->network.is_valid()) {
            operation->network->set_state_value(NETWORK_STATE_FAILED);
            _emit_network_state(operation->network, NETWORK_CHANGE_ERROR, 0, result, "Create endpoint failed.");
            _untrack_network(operation->network);
            operation->network->detach_native();
        }
        _complete_pending(operation, result);
        return;
    }

    if (operation->network.is_valid()) {
        operation->native_endpoint = change->localEndpoint;
        operation->network->m_native_local_endpoint = change->localEndpoint;
        const String descriptor = _capture_finalized_descriptor(operation->network->get_native_handle());
        if (!descriptor.is_empty()) {
            operation->network->set_descriptor(descriptor);
        }
        const String network_id = _capture_network_identifier(operation->network->get_native_handle());
        if (!network_id.is_empty()) {
            operation->network->set_network_id(network_id);
        }
    }

    if (operation->host) {
        if (operation->network.is_valid()) {
            Ref<PlayFabPartyPeer> local_peer;
            local_peer.instantiate();
            local_peer->set_network(operation->network);
            local_peer->set_unique_id(HOST_PEER_ID);
            local_peer->set_connection_status(MultiplayerPeer::CONNECTION_CONNECTED);
            operation->network->m_local_peer = local_peer;
            operation->network->set_state_value(NETWORK_STATE_CONNECTED);
            _emit_network_state(operation->network, NETWORK_CHANGE_STATE, HOST_PEER_ID, Ref<PlayFabResult>(), "connected");
        }
        _complete_pending(operation, PlayFabResult::ok_result(operation->network));
        return;
    }

    // Client path: create the local peer (peer id assigned by host via handshake)
    // and wait for the host endpoint to appear before sending the handshake request.
    if (operation->network.is_valid() && operation->network->get_local_peer().is_null()) {
        Ref<PlayFabPartyPeer> local_peer;
        local_peer.instantiate();
        local_peer->set_network(operation->network);
        local_peer->set_unique_id(0);
        local_peer->set_connection_status(MultiplayerPeer::CONNECTION_CONNECTING);
        operation->network->m_local_peer = local_peer;
        operation->network->set_state_value(NETWORK_STATE_CONNECTING);
    }

    HRESULT hr = _start_handshake_step(operation);
    if (FAILED(hr)) {
        Ref<PlayFabResult> result = PlayFabResult::error_result(hr, PARTY_PEER_NOT_CONNECTED, "Failed to issue handshake.");
        if (operation->network.is_valid()) {
            // Roll back the speculative client-side state we set above so a
            // subsequent retry sees a clean wrapper instead of a stale
            // CONNECTING peer.
            operation->network->m_local_peer = Ref<PlayFabPartyPeer>();
            operation->network->set_state_value(NETWORK_STATE_FAILED);
            _emit_network_state(operation->network, NETWORK_CHANGE_ERROR, 0, result, "Handshake start failed.");
        }
        _complete_pending(operation, result);
    }
}

void PlayFabParty::_process_endpoint_created(const Party::PartyStateChange *p_change) {
    const auto *change = static_cast<const Party::PartyEndpointCreatedStateChange *>(p_change);
    Ref<PlayFabPartyNetwork> network = _find_network_by_native(change->network);
    if (!network.is_valid()) {
        return;
    }
    Ref<PlayFabPartyPeer> peer = network->get_local_peer();
    if (!peer.is_valid()) {
        return;
    }
    if (change->endpoint == nullptr) {
        return;
    }
    // Skip our own local endpoint.
    Party::PartyLocalEndpoint *local = network->get_native_local_endpoint();
    if (local != nullptr && change->endpoint == static_cast<Party::PartyEndpoint *>(local)) {
        return;
    }

    // Client side: when a new remote endpoint becomes visible, also send the
    // handshake request to it. We cannot tell the host from other remotes a
    // priori; non-hosts ignore the request (the receiver guards on
    // is_host_network), and the host responds idempotently because it tracks
    // assignments by entity_key. Send failures here are not terminal: the
    // host endpoint may still arrive via a later EndpointCreated event, and
    // Party's NetworkDestroyed will surface unrecoverable network errors.
    if (!network->is_host_network()) {
        PendingOperation *op = _find_handshake_pending(network);
        if (op != nullptr) {
            _send_handshake_request_to(op, change->endpoint);
        }
    }
}

void PlayFabParty::_process_endpoint_destroyed(const Party::PartyStateChange *p_change) {
    const auto *change = static_cast<const Party::PartyEndpointDestroyedStateChange *>(p_change);
    Ref<PlayFabPartyNetwork> network = _find_network_by_native(change->network);
    if (!network.is_valid()) {
        return;
    }
    Ref<PlayFabPartyPeer> peer = network->get_local_peer();
    if (!peer.is_valid()) {
        return;
    }
    int32_t peer_id = peer->find_peer_by_endpoint(change->endpoint);
    if (peer_id != 0) {
        peer->unregister_peer(peer_id);
        _emit_network_state(network, NETWORK_CHANGE_PEER_LEFT, peer_id, Ref<PlayFabResult>(), "endpoint destroyed");
    }
}

void PlayFabParty::_process_endpoint_message_received(const Party::PartyStateChange *p_change) {
    const auto *change = static_cast<const Party::PartyEndpointMessageReceivedStateChange *>(p_change);
    Ref<PlayFabPartyNetwork> network = _find_network_by_native(change->network);
    if (!network.is_valid()) {
        return;
    }
    Ref<PlayFabPartyPeer> peer = network->get_local_peer();
    if (!peer.is_valid()) {
        return;
    }
    if (change->messageBuffer == nullptr || change->messageSize == 0) {
        return;
    }
    const uint8_t *buffer = static_cast<const uint8_t *>(change->messageBuffer);
    const uint32_t size = change->messageSize;
    const uint8_t kind = buffer[0];

    if (kind == PACKET_KIND_HANDSHAKE_REQUEST) {
        if (network->is_host_network()) {
            uint32_t nonce = 0;
            String entity_id;
            String entity_type;
            if (!parse_handshake_request(buffer, size, &nonce, &entity_id, &entity_type)) {
                return;
            }
            Dictionary entity_key;
            entity_key["id"] = entity_id;
            entity_key["type"] = entity_type;
            int32_t assigned_id = peer->find_peer_by_entity_key(entity_key);
            bool newly_joined = false;
            if (assigned_id == 0) {
                assigned_id = peer->allocate_peer_id();
                peer->register_peer(assigned_id, change->senderEndpoint, entity_key);
                _attach_orphan_chat_controls();
                newly_joined = true;
            } else {
                peer->update_peer_endpoint(assigned_id, change->senderEndpoint);
            }
            // Echo the client's nonce so it can match the reply to its request
            // and reject replays with a different nonce.
            _send_handshake_assignment(peer.ptr(), change->senderEndpoint, nonce, assigned_id);
            if (newly_joined) {
                _emit_network_state(network, NETWORK_CHANGE_PEER_JOINED, assigned_id, Ref<PlayFabResult>(), "handshake");
            }
        }
        return;
    }

    if (kind == PACKET_KIND_HANDSHAKE_REPLY) {
        if (!network->is_host_network()) {
            uint32_t nonce = 0;
            int32_t assigned_id = 0;
            if (!parse_handshake_reply(buffer, size, &nonce, &assigned_id)) {
                return;
            }
            PendingOperation *operation = _find_handshake_pending(network);
            if (operation != nullptr) {
                // Reject replies whose nonce does not match the pending request.
                if (nonce != operation->handshake_nonce) {
                    return;
                }
                _resolve_handshake_assignment(peer.ptr(), change->senderEndpoint, assigned_id, operation);
                return;
            }
            // No pending op (late/duplicate reply). Only accept if our peer id
            // hasn't already been assigned, to avoid mutating connected state.
            if (peer->get_unique_id() == 0) {
                peer->set_unique_id(assigned_id);
                peer->set_connection_status(MultiplayerPeer::CONNECTION_CONNECTED);
                if (change->senderEndpoint != nullptr && peer->find_peer_by_endpoint(change->senderEndpoint) == 0) {
                    // Populate the host's entity_key from the endpoint so a
                    // later PartyChatControlCreated for the host's chat
                    // control can find peer 1 by entity_key and fire
                    // chat_control_added. With an empty key the lookup
                    // fails and the sample never sets chat permissions for
                    // the host, breaking text + voice in both directions.
                    Dictionary host_entity_key = entity_key_for_endpoint(change->senderEndpoint);
                    peer->register_peer(HOST_PEER_ID, change->senderEndpoint, host_entity_key);
                    _attach_orphan_chat_controls();
                }
                network->set_state_value(NETWORK_STATE_CONNECTED);
                // Mirror the host's NETWORK_CHANGE_PEER_JOINED emit (see
                // line 2205) so client-side listeners see the host as a
                // joined peer. Without this, the autoload's peer_connected
                // signal never fires on the client for the host.
                _emit_network_state(network, NETWORK_CHANGE_PEER_JOINED, HOST_PEER_ID, Ref<PlayFabResult>(), "handshake reply");
                _emit_network_state(network, NETWORK_CHANGE_STATE, assigned_id, Ref<PlayFabResult>(), "connected");
            }
        }
        return;
    }

    // Gameplay envelope.
    int32_t source_peer = 0;
    int32_t channel = 0;
    MultiplayerPeer::TransferMode transfer_mode = MultiplayerPeer::TRANSFER_MODE_RELIABLE;
    PackedByteArray payload;
    if (!unwrap_gameplay_payload(buffer, size, &source_peer, &channel, &transfer_mode, &payload)) {
        return;
    }
    peer->enqueue_inbound(source_peer, channel, transfer_mode, payload);
}

void PlayFabParty::_process_network_descriptor_changed(const Party::PartyStateChange *p_change) {
    const auto *change = static_cast<const Party::PartyNetworkDescriptorChangedStateChange *>(p_change);
    Ref<PlayFabPartyNetwork> network = _find_network_by_native(change->network);
    if (!network.is_valid()) {
        return;
    }
    const String descriptor = _capture_finalized_descriptor(change->network);
    if (!descriptor.is_empty()) {
        network->set_descriptor(descriptor);
        const String network_id = _capture_network_identifier(change->network);
        if (!network_id.is_empty()) {
            network->set_network_id(network_id);
        }
        _emit_network_state(network, NETWORK_CHANGE_DESCRIPTOR_UPDATED, 0, Ref<PlayFabResult>(), "descriptor updated");
    }
}

void PlayFabParty::_process_leave_network_completed(const Party::PartyStateChange *p_change) {
    const auto *change = static_cast<const Party::PartyLeaveNetworkCompletedStateChange *>(p_change);
    PendingOperation *operation = static_cast<PendingOperation *>(change->asyncIdentifier);
    Ref<PlayFabResult> result;
    // Treat any non-Succeeded result with a zero errorDetail as a success.
    // PartyLeaveNetwork can surface result codes like CanceledByPlatform or
    // benign-but-not-Succeeded statuses where errorDetail == 0 and
    // GetErrorMessage returns the literal string "operation succeeded".
    // Without this guard we'd report `party_resource_not_ready:
    // PartyLeaveNetwork: operation succeeded` even though the leave itself
    // completed without an actionable error.
    if (change->result != Party::PartyStateChangeResult::Succeeded && change->errorDetail != 0) {
        result = _party_error_result(change->errorDetail, PARTY_RESOURCE_NOT_READY, "PartyLeaveNetwork");
    } else {
        result = PlayFabResult::ok_result();
    }
    Ref<PlayFabPartyNetwork> network;
    if (operation != nullptr) {
        network = operation->network;
    }
    if (!network.is_valid()) {
        network = _find_network_by_native(change->network);
    }
    if (network.is_valid()) {
        network->set_state_value(NETWORK_STATE_DISCONNECTED);
        _emit_network_state(network, NETWORK_CHANGE_STATE, 0, result, "disconnected");
    }
    if (operation != nullptr) {
        _complete_pending(operation, result);
    }
}

void PlayFabParty::_process_network_destroyed(const Party::PartyStateChange *p_change) {
    const auto *change = static_cast<const Party::PartyNetworkDestroyedStateChange *>(p_change);
    Ref<PlayFabPartyNetwork> network = _find_network_by_native(change->network);
    if (!network.is_valid()) {
        return;
    }
    Ref<PlayFabResult> result;
    if (change->reason != Party::PartyDestroyedReason::Requested) {
        result = _party_error_result(change->errorDetail, PARTY_RESOURCE_NOT_READY, "PartyNetworkDestroyed");
    }
    // Remember the destruction reason on the wrapper so any join-chain
    // *Completed delivered after this point (potentially in the same Party
    // DoWork batch) can surface the real failure via
    // _abort_join_op_if_network_dead instead of the generic
    // "Network destroyed during join." string.
    network->m_destroyed_result = result;
    network->set_state_value(NETWORK_STATE_DISCONNECTED);
    _emit_network_state(network, NETWORK_CHANGE_DESTROYED, 0, result, "network destroyed");
    Ref<PlayFabPartyPeer> peer = network->get_local_peer();
    if (peer.is_valid()) {
        peer->set_connection_status(MultiplayerPeer::CONNECTION_DISCONNECTED);
        peer->set_unique_id(0);
    }
    // Drain any handshake operation that is in flight against this native
    // network. PENDING_JOIN_HANDSHAKE has no Party-side completion event of its
    // own (it waits for a HANDSHAKE_REPLY message from the host), so without
    // this drain a network that dies between CreateEndpoint and the host's
    // reply would leave join_network_async() awaiting forever. Other join-chain
    // kinds (CONNECT_NETWORK, AUTHENTICATE, CREATE_CHAT_CONTROL,
    // CONNECT_CHAT_CONTROL, CREATE_ENDPOINT) MUST NOT be drained here because
    // Party still owns the PendingOperation* via asyncIdentifier and will
    // deliver the matching *Completed state change later (same or subsequent
    // batch). Freeing the op here would dangle that pointer.
    while (PendingOperation *handshake_op = _find_pending(PENDING_JOIN_HANDSHAKE, change->network)) {
        Ref<PlayFabResult> handshake_failure = result.is_valid()
                ? result
                : PlayFabResult::error_result(E_FAIL, PARTY_RESOURCE_NOT_READY, "Network destroyed during handshake.");
        _complete_pending(handshake_op, handshake_failure);
    }
    network->detach_native();
    _untrack_network(network);
}

void PlayFabParty::_process_create_chat_control_completed(const Party::PartyStateChange *p_change) {
    const auto *change = static_cast<const Party::PartyCreateChatControlCompletedStateChange *>(p_change);
    PendingOperation *operation = static_cast<PendingOperation *>(change->asyncIdentifier);
    if (operation == nullptr) {
        return;
    }
    if (_abort_join_op_if_network_dead(operation)) {
        // Newly-created local chat control is not attached to a wrapper here;
        // Party will surface its destruction independently.
        return;
    }
    if (change->result != Party::PartyStateChangeResult::Succeeded) {
        Ref<PlayFabResult> result = _party_error_result(change->errorDetail, PARTY_CHAT_CONTROL_CREATE_FAILED, "PartyCreateChatControl");
        if (operation->network.is_valid()) {
            _emit_network_state(operation->network, NETWORK_CHANGE_ERROR, 0, result, "Create chat control failed.");
        }
        _complete_pending(operation, result);
        return;
    }

    operation->native_chat_control = change->localChatControl;
    if (operation->network.is_valid()) {
        operation->network->m_native_local_chat_control = change->localChatControl;
        Ref<PlayFabPartyChatControl> wrapper;
        wrapper.instantiate();
        wrapper->attach(this, change->localChatControl, true);
        bool voice_enabled = operation->config.is_valid() ? operation->config->is_voice_chat_enabled() : true;
        bool text_enabled = operation->config.is_valid() ? operation->config->is_text_chat_enabled() : true;
        bool transcription_enabled = operation->config.is_valid() ? operation->config->is_transcription_enabled() : false;
        Dictionary local_entity_key = entity_key_for_chat_control(change->localChatControl);
        String local_entity_id = local_entity_key.get("id", String());
        wrapper->set_snapshot(local_entity_id, operation->user, voice_enabled, text_enabled, transcription_enabled, true);
        operation->network->m_local_chat_control = wrapper;
        if (m_chat.is_valid()) {
            m_chat->track(wrapper);
        }
    }

    PartyError err = operation->native_network->ConnectChatControl(change->localChatControl, operation);
    if (PARTY_FAILED(err)) {
        Ref<PlayFabResult> result = _party_error_result(err, PARTY_CHAT_CONTROL_CREATE_FAILED, "PartyNetwork::ConnectChatControl");
        if (operation->network.is_valid()) {
            _emit_network_state(operation->network, NETWORK_CHANGE_ERROR, 0, result, "Connect chat control dispatch failed.");
        }
        _complete_pending(operation, result);
        return;
    }
    operation->kind = PENDING_CONNECT_CHAT_CONTROL;
}

void PlayFabParty::_process_connect_chat_control_completed(const Party::PartyStateChange *p_change) {
    const auto *change = static_cast<const Party::PartyConnectChatControlCompletedStateChange *>(p_change);
    PendingOperation *operation = static_cast<PendingOperation *>(change->asyncIdentifier);
    if (operation == nullptr) {
        return;
    }
    if (_abort_join_op_if_network_dead(operation)) {
        return;
    }
    if (change->result != Party::PartyStateChangeResult::Succeeded) {
        Ref<PlayFabResult> result = _party_error_result(change->errorDetail, PARTY_CHAT_CONTROL_CREATE_FAILED, "PartyNetwork::ConnectChatControl");
        if (operation->network.is_valid()) {
            _emit_network_state(operation->network, NETWORK_CHANGE_ERROR, 0, result, "Connect chat control failed.");
        }
        _complete_pending(operation, result);
        return;
    }

    HRESULT hr = _start_create_endpoint_step(operation);
    if (FAILED(hr)) {
        Ref<PlayFabResult> result = PlayFabResult::error_result(hr, PARTY_TRANSPORT_CREATE_FAILED, "Failed to dispatch CreateEndpoint after chat control connection.");
        if (operation->network.is_valid()) {
            _emit_network_state(operation->network, NETWORK_CHANGE_ERROR, 0, result, "Create endpoint dispatch failed.");
        }
        _complete_pending(operation, result);
    }
}

void PlayFabParty::_process_chat_control_created(const Party::PartyStateChange *p_change) {
    const auto *change = static_cast<const Party::PartyChatControlCreatedStateChange *>(p_change);
    if (change->chatControl == nullptr) {
        return;
    }
    if (m_chat.is_null()) {
        return;
    }
    // Local chat controls are tracked from CreateChatControlCompleted; skip
    // them here.
    for (const Ref<PlayFabPartyNetwork> &network : m_networks) {
        if (network.is_valid() && network->get_native_local_chat_control() == change->chatControl) {
            return;
        }
    }
    // Remote chat control: find which tracked network owns the matching peer
    // by entity-key, attach a wrapper, and surface it through the per-peer
    // chat_control_added signal so GDScript can subscribe to messages and
    // transcriptions on a per-peer basis.
    Dictionary entity_key = entity_key_for_chat_control(change->chatControl);
    int32_t matched_peer_id = 0;
    Ref<PlayFabPartyPeer> matched_peer;
    for (const Ref<PlayFabPartyNetwork> &network : m_networks) {
        if (!network.is_valid()) {
            continue;
        }
        Ref<PlayFabPartyPeer> peer = network->get_local_peer();
        if (!peer.is_valid()) {
            continue;
        }
        int32_t peer_id = peer->find_peer_by_entity_key(entity_key);
        if (peer_id == 0) {
            continue;
        }
        if (peer->find_peer_by_chat_control(change->chatControl) != 0) {
            // Already mapped via handshake or a prior event.
            return;
        }
        matched_peer_id = peer_id;
        matched_peer = peer;
        break;
    }
    Ref<PlayFabPartyChatControl> wrapper;
    wrapper.instantiate();
    wrapper->attach(this, change->chatControl, false);
    PartyString id_str = nullptr;
    change->chatControl->GetEntityId(&id_str);
    String id = id_str != nullptr ? String::utf8(id_str) : String();
    wrapper->set_snapshot(id, Ref<PlayFabUser>(), true, true, false, false);
    m_chat->track(wrapper);
    if (matched_peer.is_valid()) {
        matched_peer->update_peer_chat_control(matched_peer_id, wrapper);
        matched_peer->emit_chat_control_added(matched_peer_id, wrapper);
    } else {
        // Peer not registered yet (handshake hasn't completed for this
        // chat control's owner). Queue and retry from
        // _attach_orphan_chat_controls() after the next register_peer.
        // Without this, a PartyChatControlCreated that lands in the same
        // DoWork pass BEFORE the handshake reply/request is processed is
        // silently dropped — chat_control_added never fires, the sample
        // never calls set_peer_chat_permissions_async, and both sides
        // reject inbound text/voice for lack of receive permission.
        m_orphan_chat_controls.push_back(wrapper);
    }
}

void PlayFabParty::_attach_orphan_chat_controls() {
    for (auto it = m_orphan_chat_controls.begin(); it != m_orphan_chat_controls.end(); ) {
        Ref<PlayFabPartyChatControl> wrapper = *it;
        if (!wrapper.is_valid()) {
            it = m_orphan_chat_controls.erase(it);
            continue;
        }
        Party::PartyChatControl *native = wrapper->get_native_handle();
        if (native == nullptr) {
            it = m_orphan_chat_controls.erase(it);
            continue;
        }
        Dictionary entity_key = entity_key_for_chat_control(native);
        bool attached = false;
        for (const Ref<PlayFabPartyNetwork> &network : m_networks) {
            if (!network.is_valid()) {
                continue;
            }
            Ref<PlayFabPartyPeer> peer = network->get_local_peer();
            if (!peer.is_valid()) {
                continue;
            }
            int32_t peer_id = peer->find_peer_by_entity_key(entity_key);
            if (peer_id == 0) {
                continue;
            }
            if (peer->find_peer_by_chat_control(native) == 0) {
                peer->update_peer_chat_control(peer_id, wrapper);
                peer->emit_chat_control_added(peer_id, wrapper);
            }
            attached = true;
            break;
        }
        if (attached) {
            it = m_orphan_chat_controls.erase(it);
        } else {
            ++it;
        }
    }
}

void PlayFabParty::_process_chat_control_destroyed(const Party::PartyStateChange *p_change) {
    const auto *change = static_cast<const Party::PartyChatControlDestroyedStateChange *>(p_change);
    if (change->chatControl == nullptr) {
        return;
    }
    if (m_chat.is_null()) {
        return;
    }
    // Clear any per-peer chat-control mapping for the destroyed control and
    // surface the removal through the peer's chat_control_removed signal.
    for (const Ref<PlayFabPartyNetwork> &network : m_networks) {
        if (!network.is_valid()) {
            continue;
        }
        Ref<PlayFabPartyPeer> peer = network->get_local_peer();
        if (!peer.is_valid()) {
            continue;
        }
        int32_t peer_id = peer->find_peer_by_chat_control(change->chatControl);
        if (peer_id != 0) {
            peer->emit_chat_control_removed(peer_id);
            peer->update_peer_chat_control(peer_id, Ref<PlayFabPartyChatControl>());
        }
    }
    for (Ref<PlayFabPartyChatControl> wrapper : m_chat->get_chat_controls()) {
        if (wrapper.is_valid() && wrapper->get_native_handle() == change->chatControl) {
            m_chat->untrack(wrapper);
            _emit_chat_state(wrapper, CHAT_CHANGE_DESTROYED, Ref<PlayFabResult>(), "chat control destroyed");
            break;
        }
    }
    // Drop any orphan wrapper still pointing at the destroyed control so we
    // don't leak the Ref or re-emit chat_control_added later for a dead handle.
    for (auto it = m_orphan_chat_controls.begin(); it != m_orphan_chat_controls.end(); ) {
        if (it->is_valid() && (*it)->get_native_handle() == change->chatControl) {
            it = m_orphan_chat_controls.erase(it);
        } else {
            ++it;
        }
    }
}

void PlayFabParty::_process_chat_text_received(const Party::PartyStateChange *p_change) {
    const auto *change = static_cast<const Party::PartyChatTextReceivedStateChange *>(p_change);
    if (m_chat.is_null() || change->senderChatControl == nullptr) {
        return;
    }
    Ref<PlayFabPartyChatMessage> message;
    message.instantiate();
    Array targets;
    Ref<PlayFabPartyChatControl> sender_wrapper;
    for (Ref<PlayFabPartyChatControl> wrapper : m_chat->get_chat_controls()) {
        if (wrapper.is_valid() && wrapper->get_native_handle() == change->senderChatControl) {
            sender_wrapper = wrapper;
            break;
        }
    }
    String text = String::utf8(change->chatText != nullptr ? change->chatText : "");
    String language = String::utf8(change->languageCode != nullptr ? change->languageCode : "");
    String translated = text;
    if (change->translationCount > 0 && change->translations != nullptr && change->translations[0].translation != nullptr) {
        translated = String::utf8(change->translations[0].translation);
    }
    message->set_values(sender_wrapper, Dictionary(), targets, text, language, translated, false, 0, Dictionary());
    if (sender_wrapper.is_valid()) {
        sender_wrapper->emit_signal("message_received", message);
    }
    // Mirror onto the network-level peer signal so titles that wired
    // PlayFabPartyPeer.text_message_received (the documented + tested
    // public API) actually receive incoming chat. A chat control is
    // only ever associated with one tracked network, so break after the
    // first match to avoid unnecessary scans and any chance of double
    // emission if that invariant ever changes.
    for (const Ref<PlayFabPartyNetwork> &network : m_networks) {
        if (!network.is_valid()) {
            continue;
        }
        Ref<PlayFabPartyPeer> peer = network->get_local_peer();
        if (!peer.is_valid()) {
            continue;
        }
        int32_t peer_id = peer->find_peer_by_chat_control(change->senderChatControl);
        if (peer_id != 0) {
            peer->emit_text_message(peer_id, message);
            break;
        }
    }
}

void PlayFabParty::_process_voice_chat_transcription_received(const Party::PartyStateChange *p_change) {
    const auto *change = static_cast<const Party::PartyVoiceChatTranscriptionReceivedStateChange *>(p_change);
    if (m_chat.is_null() || change->senderChatControl == nullptr) {
        return;
    }
    Ref<PlayFabPartyChatControl> sender_wrapper;
    for (Ref<PlayFabPartyChatControl> wrapper : m_chat->get_chat_controls()) {
        if (wrapper.is_valid() && wrapper->get_native_handle() == change->senderChatControl) {
            sender_wrapper = wrapper;
            break;
        }
    }
    Ref<PlayFabPartyChatMessage> message;
    message.instantiate();
    Array targets;
    String text = String::utf8(change->transcription != nullptr ? change->transcription : "");
    String language = String::utf8(change->languageCode != nullptr ? change->languageCode : "");
    message->set_values(sender_wrapper, Dictionary(), targets, text, language, text, true, 0, Dictionary());
    if (sender_wrapper.is_valid()) {
        sender_wrapper->emit_signal("transcription_received", message);
    }
    // Mirror onto the network-level peer signal so titles that wired
    // PlayFabPartyPeer.transcription_received (the documented + tested
    // public API) actually receive transcribed audio. A chat control is
    // only ever associated with one tracked network, so break after the
    // first match to avoid unnecessary scans and any chance of double
    // emission if that invariant ever changes.
    for (const Ref<PlayFabPartyNetwork> &network : m_networks) {
        if (!network.is_valid()) {
            continue;
        }
        Ref<PlayFabPartyPeer> peer = network->get_local_peer();
        if (!peer.is_valid()) {
            continue;
        }
        int32_t peer_id = peer->find_peer_by_chat_control(change->senderChatControl);
        if (peer_id != 0) {
            peer->emit_transcription(peer_id, message);
            break;
        }
    }
}

// ---------------------------------------------------------------------------
// PlayFabParty (chat helpers)

Signal PlayFabParty::_send_text_via_chat_control(Party::PartyLocalChatControl *p_local_chat_control, const std::vector<Party::PartyChatControl *> &p_targets, const String &p_message, const Ref<PlayFabPartyTextMessageConfig> &p_config) {
    if (p_local_chat_control == nullptr) {
        return _make_error_signal(E_NOT_VALID_STATE, PARTY_RESOURCE_NOT_READY,
                "PlayFab.party._send_text_via_chat_control() requires a connected local chat control.");
    }
    if (p_message.is_empty()) {
        return _make_error_signal(E_INVALIDARG, PARTY_INVALID_OPTIONS,
                "PlayFab.party._send_text_via_chat_control() requires a non-empty message.");
    }
    if (p_targets.empty()) {
        return _make_ok_signal();
    }
    const CharString text_utf8 = p_message.utf8();
    PartyError err = p_local_chat_control->SendText(
            static_cast<uint32_t>(p_targets.size()),
            const_cast<Party::PartyChatControlArray>(p_targets.data()),
            text_utf8.get_data(),
            0,
            nullptr);
    if (PARTY_FAILED(err)) {
        Ref<PlayFabResult> result = _party_error_result(err, PARTY_CHAT_PERMISSION_FAILED, "PartyLocalChatControl::SendText");
        return _make_error_signal(E_FAIL, PARTY_CHAT_PERMISSION_FAILED, result.is_valid() ? result->get_message() : String("PartyLocalChatControl::SendText failed."));
    }
    return _make_ok_signal();
}

Signal PlayFabParty::_set_chat_permissions(Party::PartyLocalChatControl *p_local_chat_control, Party::PartyChatControl *p_target, int64_t p_permissions, bool *r_succeeded) {
    if (r_succeeded != nullptr) {
        *r_succeeded = false;
    }
    if (p_local_chat_control == nullptr || p_target == nullptr) {
        return _make_error_signal(E_NOT_VALID_STATE, PARTY_CHAT_PERMISSION_FAILED,
                "PlayFab.party._set_chat_permissions() requires both a local chat control and a target.");
    }
    Party::PartyChatPermissionOptions native = static_cast<Party::PartyChatPermissionOptions>(_translate_chat_permissions_to_native(p_permissions));
    PartyError err = p_local_chat_control->SetPermissions(p_target, native);
    if (PARTY_FAILED(err)) {
        Ref<PlayFabResult> result = _party_error_result(err, PARTY_CHAT_PERMISSION_FAILED, "PartyLocalChatControl::SetPermissions");
        return _make_error_signal(E_FAIL, PARTY_CHAT_PERMISSION_FAILED, result.is_valid() ? result->get_message() : String("PartyLocalChatControl::SetPermissions failed."));
    }
    if (r_succeeded != nullptr) {
        *r_succeeded = true;
    }
    return _make_ok_signal();
}

Signal PlayFabParty::_set_incoming_audio_muted(Party::PartyLocalChatControl *p_local_chat_control, Party::PartyChatControl *p_target, bool p_muted, bool *r_succeeded) {
    if (r_succeeded != nullptr) {
        *r_succeeded = false;
    }
    if (p_local_chat_control == nullptr || p_target == nullptr) {
        return _make_error_signal(E_NOT_VALID_STATE, PARTY_CHAT_PERMISSION_FAILED,
                "PlayFab.party._set_incoming_audio_muted() requires both a local chat control and a target.");
    }
    PartyError err = p_local_chat_control->SetIncomingAudioMuted(p_target, p_muted ? PartyBool(1) : PartyBool(0));
    if (PARTY_FAILED(err)) {
        Ref<PlayFabResult> result = _party_error_result(err, PARTY_CHAT_PERMISSION_FAILED, "PartyLocalChatControl::SetIncomingAudioMuted");
        return _make_error_signal(E_FAIL, PARTY_CHAT_PERMISSION_FAILED, result.is_valid() ? result->get_message() : String("PartyLocalChatControl::SetIncomingAudioMuted failed."));
    }
    if (r_succeeded != nullptr) {
        *r_succeeded = true;
    }
    return _make_ok_signal();
}

Signal PlayFabParty::_destroy_chat_control(const Ref<PlayFabPartyChatControl> &p_chat_control) {
    if (!p_chat_control.is_valid() || p_chat_control->get_native_handle() == nullptr || !p_chat_control->is_local()) {
        return _make_ok_signal();
    }
    Party::PartyLocalDevice *local_device = nullptr;
    PartyError err = Party::PartyManager::GetSingleton().GetLocalDevice(&local_device);
    if (PARTY_FAILED(err) || local_device == nullptr) {
        return _make_error_signal(E_FAIL, PARTY_RESOURCE_NOT_READY, "PartyManager::GetLocalDevice failed.");
    }
    Party::PartyLocalChatControl *local_chat = static_cast<Party::PartyLocalChatControl *>(p_chat_control->get_native_handle());
    PendingOperation *operation = _create_pending(PENDING_DESTROY_CHAT_CONTROL);
    operation->native_chat_control = local_chat;
    err = local_device->DestroyChatControl(local_chat, operation);
    if (PARTY_FAILED(err)) {
        Ref<PlayFabResult> result = _party_error_result(err, PARTY_RESOURCE_NOT_READY, "PartyLocalDevice::DestroyChatControl");
        _complete_pending(operation, result);
        return _make_error_signal(E_FAIL, PARTY_RESOURCE_NOT_READY, result.is_valid() ? result->get_message() : String("PartyLocalDevice::DestroyChatControl failed."));
    }
    return operation->pending_signal->get_completed_signal();
}

// ---------------------------------------------------------------------------
// PlayFabParty (network step starters)

HRESULT PlayFabParty::_start_create_endpoint_step(PendingOperation *p_operation) {
    if (p_operation == nullptr || p_operation->native_network == nullptr || p_operation->native_user == nullptr) {
        return E_INVALIDARG;
    }
    Party::PartyLocalEndpoint *endpoint = nullptr;
    PartyError err = p_operation->native_network->CreateEndpoint(
            p_operation->native_user,
            0,
            nullptr,
            nullptr,
            p_operation,
            &endpoint);
    if (PARTY_FAILED(err)) {
        return E_FAIL;
    }
    p_operation->kind = PENDING_CREATE_ENDPOINT;
    return S_OK;
}

HRESULT PlayFabParty::_start_create_chat_control_step(PendingOperation *p_operation) {
    if (p_operation == nullptr || p_operation->native_user == nullptr) {
        return E_INVALIDARG;
    }
    Party::PartyLocalDevice *local_device = nullptr;
    PartyError err = Party::PartyManager::GetSingleton().GetLocalDevice(&local_device);
    if (PARTY_FAILED(err) || local_device == nullptr) {
        return E_FAIL;
    }
    Party::PartyLocalChatControl *chat_control = nullptr;
    err = local_device->CreateChatControl(p_operation->native_user, "en-US", p_operation, &chat_control);
    if (PARTY_FAILED(err)) {
        return E_FAIL;
    }
    p_operation->kind = PENDING_CREATE_CHAT_CONTROL;
    return S_OK;
}

HRESULT PlayFabParty::_start_handshake_step(PendingOperation *p_operation) {
    if (p_operation == nullptr || p_operation->network.is_null() || p_operation->user.is_null()) {
        return E_INVALIDARG;
    }
    Party::PartyLocalEndpoint *local = p_operation->network->get_native_local_endpoint();
    if (local == nullptr) {
        return E_INVALIDARG;
    }
    Party::PartyNetwork *native_net = p_operation->network->get_native_handle();
    if (native_net == nullptr) {
        return E_INVALIDARG;
    }

    // Generate and persist the per-handshake nonce so we can validate replies.
    p_operation->handshake_nonce = random_handshake_nonce();
    p_operation->kind = PENDING_JOIN_HANDSHAKE;

    uint32_t endpoint_count = 0;
    Party::PartyEndpointArray endpoints = nullptr;
    PartyError err = native_net->GetEndpoints(&endpoint_count, &endpoints);
    if (PARTY_FAILED(err)) {
        return E_FAIL;
    }
    // Send to any already-visible remote endpoints. If none exist yet, the
    // handshake is deferred and re-attempted from _process_endpoint_created
    // when a remote endpoint becomes visible. Per-endpoint send failures are
    // not treated as terminal because the host endpoint may not be among the
    // currently visible remotes; Party's NetworkDestroyed surfaces real
    // unrecoverable network errors.
    for (uint32_t i = 0; i < endpoint_count; ++i) {
        if (endpoints[i] != nullptr && endpoints[i] != static_cast<Party::PartyEndpoint *>(local)) {
            _send_handshake_request_to(p_operation, endpoints[i]);
        }
    }
    return S_OK;
}

HRESULT PlayFabParty::_send_handshake_request_to(PendingOperation *p_operation, Party::PartyEndpoint *p_target) {
    if (p_operation == nullptr || p_operation->network.is_null() || p_operation->user.is_null() || p_target == nullptr) {
        return E_INVALIDARG;
    }
    Party::PartyLocalEndpoint *local = p_operation->network->get_native_local_endpoint();
    if (local == nullptr) {
        return E_INVALIDARG;
    }

    Dictionary entity_key = p_operation->user->get_entity_key();
    String entity_id = entity_key.get("id", String());
    String entity_type = entity_key.get("type", String("title_player_account"));
    PackedByteArray request = build_handshake_request(p_operation->handshake_nonce, entity_id, entity_type);

    Party::PartyDataBuffer buffer = {};
    buffer.buffer = request.ptr();
    buffer.bufferByteCount = static_cast<uint32_t>(request.size());

    Party::PartyEndpoint *targets[1] = {p_target};
    PartyError err = local->SendMessage(
            1,
            targets,
            static_cast<Party::PartySendMessageOptions>(
                    static_cast<uint32_t>(Party::PartySendMessageOptions::GuaranteedDelivery) |
                    static_cast<uint32_t>(Party::PartySendMessageOptions::SequentialDelivery)),
            nullptr,
            1,
            &buffer,
            nullptr);
    if (PARTY_FAILED(err)) {
        return E_FAIL;
    }
    return S_OK;
}

void PlayFabParty::_send_handshake_assignment(PlayFabPartyPeer *p_peer, Party::PartyEndpoint *p_target_endpoint, uint32_t p_nonce, int32_t p_assigned_id) {
    if (p_peer == nullptr || p_target_endpoint == nullptr) {
        return;
    }
    Ref<PlayFabPartyNetwork> network = p_peer->get_network();
    if (!network.is_valid() || network->get_native_local_endpoint() == nullptr) {
        return;
    }
    PackedByteArray reply = build_handshake_reply(p_nonce, p_assigned_id);
    Party::PartyDataBuffer buffer = {};
    buffer.buffer = reply.ptr();
    buffer.bufferByteCount = static_cast<uint32_t>(reply.size());
    Party::PartyEndpoint *targets[1] = {p_target_endpoint};
    network->get_native_local_endpoint()->SendMessage(
            1,
            targets,
            static_cast<Party::PartySendMessageOptions>(
                    static_cast<uint32_t>(Party::PartySendMessageOptions::GuaranteedDelivery) |
                    static_cast<uint32_t>(Party::PartySendMessageOptions::SequentialDelivery)),
            nullptr,
            1,
            &buffer,
            nullptr);
}

void PlayFabParty::_resolve_handshake_assignment(PlayFabPartyPeer *p_peer, Party::PartyEndpoint *p_sender_endpoint, int32_t p_assigned_id, PendingOperation *p_operation) {
    if (p_peer == nullptr || p_operation == nullptr) {
        return;
    }
    p_peer->set_unique_id(p_assigned_id);
    p_peer->set_connection_status(MultiplayerPeer::CONNECTION_CONNECTED);
    Ref<PlayFabPartyNetwork> network = p_operation->network;
    if (network.is_valid()) {
        network->set_state_value(NETWORK_STATE_CONNECTED);
    }
    // Insert the host into m_peer_records BEFORE resolving the await so the
    // moment _attach_network runs and fires any outbound rpc(...), _put_packet
    // can find the host endpoint and route the packet. Without this, the very
    // first RPC after attach (the autoload's automatic
    //   rpc("handshake_message", "ready")
    // line) silently dropped with ERR_UNAVAILABLE because m_peer_records was
    // still empty. Populate the host's entity_key from the endpoint so a
    // later PartyChatControlCreated for the host's chat control can find
    // peer 1 by entity_key and fire chat_control_added; an empty key would
    // cause that lookup to fail and break text + voice in both directions.
    bool inserted = false;
    if (p_sender_endpoint != nullptr && p_peer->find_peer_by_endpoint(p_sender_endpoint) == 0) {
        Dictionary host_entity_key = entity_key_for_endpoint(p_sender_endpoint);
        inserted = p_peer->insert_peer_record(HOST_PEER_ID, p_sender_endpoint, host_entity_key);
    }
    // Resolve the await. The GDScript coroutine resumes synchronously, so
    // _attach_network runs here: it assigns multiplayer.multiplayer_peer,
    // wires signal handlers, and may fire an initial RPC. All of that
    // depends on m_peer_records being populated (done above), but the
    // inherited MultiplayerPeer "peer_connected" signal MUST NOT fire yet
    // — it has to land after multiplayer.multiplayer_peer is assigned so
    // Godot's MultiplayerAPI captures it and adds the peer to its connected
    // set. _complete_pending invalidates p_operation, so network was captured.
    _complete_pending(p_operation, PlayFabResult::ok_result(network));
    if (inserted) {
        // _attach_network has assigned multiplayer.multiplayer_peer and
        // wired peer.chat_control_added; now safe to emit peer_connected
        // (MultiplayerAPI picks it up so subsequent rpc(...) calls to peer
        // 1 are routed) and drain any chat controls that arrived before
        // the peer was registered.
        p_peer->emit_peer_connected(HOST_PEER_ID);
        _attach_orphan_chat_controls();
    }
    if (network.is_valid()) {
        // Mirror the host's NETWORK_CHANGE_PEER_JOINED emit (see line 2205)
        // so client-side listeners see the host as a joined peer.
        _emit_network_state(network, NETWORK_CHANGE_PEER_JOINED, HOST_PEER_ID, Ref<PlayFabResult>(), "handshake reply");
        _emit_network_state(network, NETWORK_CHANGE_STATE, p_assigned_id, Ref<PlayFabResult>(), "connected");
    }
}

PlayFabParty::PendingOperation *PlayFabParty::_find_handshake_pending(const Ref<PlayFabPartyNetwork> &p_network) {
    if (!p_network.is_valid()) {
        return nullptr;
    }
    Party::PartyNetwork *native = p_network->get_native_handle();
    return _find_pending_join(native);
}

// ---------------------------------------------------------------------------
// PlayFabParty (descriptor / translation / errors / state emit)

String PlayFabParty::_capture_finalized_descriptor(Party::PartyNetwork *p_network) const {
    if (p_network == nullptr) {
        return String();
    }
    Party::PartyNetworkDescriptor descriptor = {};
    PartyError err = p_network->GetNetworkDescriptor(&descriptor);
    if (PARTY_FAILED(err)) {
        return String();
    }
    char buffer[Party::c_maxSerializedNetworkDescriptorStringLength + 1] = {};
    err = Party::PartyManager::SerializeNetworkDescriptor(&descriptor, buffer);
    if (PARTY_FAILED(err)) {
        return String();
    }
    return String::utf8(buffer);
}

String PlayFabParty::_capture_network_identifier(Party::PartyNetwork *p_network) const {
    if (p_network == nullptr) {
        return String();
    }
    Party::PartyNetworkDescriptor descriptor = {};
    PartyError err = p_network->GetNetworkDescriptor(&descriptor);
    if (PARTY_FAILED(err)) {
        return String();
    }
    return String::utf8(descriptor.networkIdentifier);
}

int64_t PlayFabParty::_translate_chat_permissions_to_native(int64_t p_permissions) const {
    int64_t native = 0;
    if (p_permissions & CHAT_PERMISSION_SEND_AUDIO) {
        native |= static_cast<int64_t>(Party::PartyChatPermissionOptions::SendAudio);
    }
    if (p_permissions & CHAT_PERMISSION_RECEIVE_AUDIO) {
        native |= static_cast<int64_t>(Party::PartyChatPermissionOptions::ReceiveAudio);
    }
    if (p_permissions & CHAT_PERMISSION_RECEIVE_TEXT) {
        native |= static_cast<int64_t>(Party::PartyChatPermissionOptions::ReceiveText);
    }
    return native;
}

int64_t PlayFabParty::_translate_chat_permissions_from_native(int32_t p_native) const {
    int64_t result = 0;
    if (p_native & static_cast<int32_t>(Party::PartyChatPermissionOptions::SendAudio)) {
        result |= CHAT_PERMISSION_SEND_AUDIO;
    }
    if (p_native & static_cast<int32_t>(Party::PartyChatPermissionOptions::ReceiveAudio)) {
        result |= CHAT_PERMISSION_RECEIVE_AUDIO;
    }
    if (p_native & static_cast<int32_t>(Party::PartyChatPermissionOptions::ReceiveText)) {
        result |= CHAT_PERMISSION_RECEIVE_TEXT;
    }
    return result;
}

Ref<PlayFabResult> PlayFabParty::_party_error_result(uint32_t p_party_error, const String &p_code, const String &p_action) const {
    String message = p_action;
    String detail = _party_error_message(p_party_error);
    if (!detail.is_empty()) {
        message += String(": ") + detail;
    }
    return PlayFabResult::error_result(E_FAIL, p_code, message);
}

String PlayFabParty::_party_error_message(uint32_t p_party_error) {
    PartyString text = nullptr;
    PartyError err = Party::PartyManager::GetErrorMessage(p_party_error, &text);
    if (PARTY_FAILED(err) || text == nullptr) {
        return String("Unknown PlayFab Party error.");
    }
    return String::utf8(text);
}

void PlayFabParty::_emit_network_state(const Ref<PlayFabPartyNetwork> &p_network, int64_t p_kind, int64_t p_peer_id, const Ref<PlayFabResult> &p_result, const String &p_reason) {
    if (!p_network.is_valid()) {
        return;
    }
    Ref<PlayFabPartyNetworkStateChange> change;
    change.instantiate();
    Ref<PlayFabUser> user = p_network->get_local_user();
    change->set_values(p_kind, p_network, p_result, user, p_peer_id, p_network->get_state(), p_reason);
    p_network->emit_signal("state_changed", change);
}

void PlayFabParty::_emit_chat_state(const Ref<PlayFabPartyChatControl> &p_chat_control, int64_t p_kind, const Ref<PlayFabResult> &p_result, const String &p_reason) {
    if (!p_chat_control.is_valid()) {
        return;
    }
    Ref<PlayFabPartyChatStateChange> change;
    change.instantiate();
    change->set_values(p_kind, p_chat_control, p_result, p_reason);
    p_chat_control->emit_signal("state_changed", change);
}

// ---------------------------------------------------------------------------
// PlayFabParty (binding)

void PlayFabParty::_bind_methods() {
    ClassDB::bind_method(D_METHOD("is_initialized"), &PlayFabParty::is_initialized);
    ClassDB::bind_method(D_METHOD("initialize_async", "config", "local_udp_port"), &PlayFabParty::initialize_async, DEFVAL(Ref<PlayFabPartyConfig>()), DEFVAL(-1));
    ClassDB::bind_method(D_METHOD("shutdown_async"), &PlayFabParty::shutdown_async);
    ClassDB::bind_method(D_METHOD("create_and_join_network_async", "user", "config"), &PlayFabParty::create_and_join_network_async, DEFVAL(Ref<PlayFabPartyConfig>()));
    ClassDB::bind_method(D_METHOD("join_network_async", "user", "descriptor", "config"), &PlayFabParty::join_network_async, DEFVAL(Ref<PlayFabPartyConfig>()));
    ClassDB::bind_method(D_METHOD("leave_network_async", "network"), &PlayFabParty::leave_network_async);
    ClassDB::bind_method(D_METHOD("get_chat"), &PlayFabParty::get_chat);
    ClassDB::bind_method(D_METHOD("get_networks"), &PlayFabParty::get_networks);

    ADD_SIGNAL(MethodInfo("party_error", PropertyInfo(Variant::OBJECT, "result", PROPERTY_HINT_RESOURCE_TYPE, "PlayFabResult")));

    BIND_ENUM_CONSTANT(DIRECT_PEER_CONNECTIVITY_NONE);
    BIND_ENUM_CONSTANT(DIRECT_PEER_CONNECTIVITY_SAME_PLATFORM_TYPE);
    BIND_ENUM_CONSTANT(DIRECT_PEER_CONNECTIVITY_DIFFERENT_PLATFORM_TYPE);
    BIND_ENUM_CONSTANT(DIRECT_PEER_CONNECTIVITY_ANY_PLATFORM_TYPE);
    BIND_ENUM_CONSTANT(DIRECT_PEER_CONNECTIVITY_SAME_ENTITY_LOGIN_PROVIDER);
    BIND_ENUM_CONSTANT(DIRECT_PEER_CONNECTIVITY_DIFFERENT_ENTITY_LOGIN_PROVIDER);
    BIND_ENUM_CONSTANT(DIRECT_PEER_CONNECTIVITY_ANY_ENTITY_LOGIN_PROVIDER);
    BIND_ENUM_CONSTANT(DIRECT_PEER_CONNECTIVITY_ANY);
    BIND_ENUM_CONSTANT(DIRECT_PEER_CONNECTIVITY_ONLY_SERVERS);

    BIND_ENUM_CONSTANT(NETWORK_STATE_CREATING);
    BIND_ENUM_CONSTANT(NETWORK_STATE_CONNECTING);
    BIND_ENUM_CONSTANT(NETWORK_STATE_AUTHENTICATING);
    BIND_ENUM_CONSTANT(NETWORK_STATE_CONNECTED);
    BIND_ENUM_CONSTANT(NETWORK_STATE_DISCONNECTING);
    BIND_ENUM_CONSTANT(NETWORK_STATE_DISCONNECTED);
    BIND_ENUM_CONSTANT(NETWORK_STATE_FAILED);

    BIND_ENUM_CONSTANT(CHAT_PERMISSION_NONE);
    BIND_ENUM_CONSTANT(CHAT_PERMISSION_SEND_AUDIO);
    BIND_ENUM_CONSTANT(CHAT_PERMISSION_RECEIVE_AUDIO);
    BIND_ENUM_CONSTANT(CHAT_PERMISSION_RECEIVE_TEXT);

    BIND_ENUM_CONSTANT(NETWORK_CHANGE_STATE);
    BIND_ENUM_CONSTANT(NETWORK_CHANGE_PEER_JOINED);
    BIND_ENUM_CONSTANT(NETWORK_CHANGE_PEER_LEFT);
    BIND_ENUM_CONSTANT(NETWORK_CHANGE_DESCRIPTOR_UPDATED);
    BIND_ENUM_CONSTANT(NETWORK_CHANGE_DESTROYED);
    BIND_ENUM_CONSTANT(NETWORK_CHANGE_ERROR);

    BIND_ENUM_CONSTANT(CHAT_CHANGE_CREATED);
    BIND_ENUM_CONSTANT(CHAT_CHANGE_DESTROYED);
    BIND_ENUM_CONSTANT(CHAT_CHANGE_PERMISSIONS_CHANGED);
    BIND_ENUM_CONSTANT(CHAT_CHANGE_MUTED_CHANGED);
}

} // namespace godot