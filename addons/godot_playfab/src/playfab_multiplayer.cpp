#include "playfab_multiplayer.h"

#include <algorithm>
#include <atomic>
#include <string>

#include <godot_cpp/classes/json.hpp>

#include "playfab.h"
#include "playfab_pending_signal.h"
#include "playfab_result.h"
#include "playfab_runtime.h"
#include "playfab_user.h"

namespace godot {

namespace {

String pf_string(const char *p_value) {
    return p_value != nullptr ? String::utf8(p_value) : String();
}

Dictionary entity_key_to_dictionary(const PFEntityKey *p_entity_key) {
    Dictionary entity_key;
    entity_key["id"] = p_entity_key != nullptr && p_entity_key->id != nullptr ? String::utf8(p_entity_key->id) : String();
    entity_key["type"] = p_entity_key != nullptr && p_entity_key->type != nullptr ? String::utf8(p_entity_key->type) : String();
    return entity_key;
}

bool entity_key_equals_dictionary(const PFEntityKey *p_entity_key, const Dictionary &p_dictionary) {
    if (p_entity_key == nullptr) {
        return false;
    }
    return String(p_dictionary.get("id", String())) == pf_string(p_entity_key->id) &&
            String(p_dictionary.get("type", String())) == pf_string(p_entity_key->type);
}

bool entity_key_equals_user(const PFEntityKey *p_entity_key, const Ref<PlayFabUser> &p_user) {
    return p_user.is_valid() && entity_key_equals_dictionary(p_entity_key, p_user->get_entity_key());
}

Dictionary copy_dictionary(const Dictionary &p_dictionary) {
    Dictionary copy;
    Array keys = p_dictionary.keys();
    for (int64_t i = 0; i < keys.size(); ++i) {
        const Variant key = keys[i];
        copy[key] = p_dictionary[key];
    }
    return copy;
}

Dictionary normalize_property_dictionary(const Dictionary &p_properties) {
    Dictionary normalized;
    Array keys = p_properties.keys();
    for (int64_t i = 0; i < keys.size(); ++i) {
        const Variant key_variant = keys[i];
        const Variant value_variant = p_properties[key_variant];
        if (value_variant.get_type() == Variant::NIL) {
            continue;
        }
        normalized[String(key_variant)] = String(value_variant);
    }
    return normalized;
}

void apply_property_update(Dictionary &r_properties, const Dictionary &p_update) {
    Array keys = p_update.keys();
    for (int64_t i = 0; i < keys.size(); ++i) {
        const Variant key_variant = keys[i];
        const Variant value_variant = p_update[key_variant];
        const String key = String(key_variant);
        if (value_variant.get_type() == Variant::NIL) {
            r_properties.erase(key);
        } else {
            r_properties[key] = String(value_variant);
        }
    }
}

String multiplayer_error_message(HRESULT p_hresult, const String &p_fallback) {
    const char *message = PFMultiplayerGetErrorMessage(p_hresult);
    if (message != nullptr && message[0] != '\0') {
        return String::utf8(message);
    }
    return p_fallback;
}

Ref<PlayFabResult> multiplayer_hresult_error(HRESULT p_hresult, const String &p_action, const String &p_code, const Variant &p_data = Variant()) {
    return PlayFabResult::error_result(
            p_hresult,
            p_code.is_empty() ? PlayFabResult::format_hresult(p_hresult) : p_code,
            p_action + String(" ") + multiplayer_error_message(p_hresult, PlayFabResult::format_hresult(p_hresult)),
            p_data);
}

struct MultiplayerQueueTerminateContext {
    std::atomic<bool> terminated{false};
};

void CALLBACK multiplayer_queue_terminated(void *p_context) {
    auto *ctx = static_cast<MultiplayerQueueTerminateContext *>(p_context);
    if (ctx != nullptr) {
        ctx->terminated.store(true, std::memory_order_release);
    }
}

Signal detached_error_signal(HRESULT p_hresult, const String &p_code, const String &p_message) {
    Ref<PlayFabPendingSignal> pending_signal;
    pending_signal.instantiate();
    pending_signal->complete_deferred(PlayFabResult::error_result(p_hresult, p_code, p_message));
    return pending_signal->get_completed_signal();
}

class StringPairList {
    std::vector<std::string> m_key_strings;
    std::vector<std::string> m_value_strings;
    std::vector<bool> m_value_is_null;
    std::vector<const char *> m_key_ptrs;
    std::vector<const char *> m_value_ptrs;

public:
    // Builds parallel key/value char* arrays for the PlayFab Multiplayer SDK.
    //
    // A NIL value entry is preserved as a nullptr pointer in `values()`, which
    // the SDK interprets as "delete this property" (see PFLobby.h:
    // PFLobbyDataUpdate / PFLobbyMemberDataUpdate — "To delete a value,
    // provide nullptr as its new value"). String / StringName values are
    // copied into char* storage.
    bool assign(const Dictionary &p_dictionary, String *r_error_message) {
        m_key_strings.clear();
        m_value_strings.clear();
        m_value_is_null.clear();
        m_key_ptrs.clear();
        m_value_ptrs.clear();

        Array keys = p_dictionary.keys();
        for (int64_t i = 0; i < keys.size(); ++i) {
            const Variant key_variant = keys[i];
            if (key_variant.get_type() != Variant::STRING && key_variant.get_type() != Variant::STRING_NAME) {
                if (r_error_message != nullptr) {
                    *r_error_message = "PlayFab Multiplayer property dictionaries require String keys.";
                }
                return false;
            }

            const Variant value_variant = p_dictionary[key_variant];
            const Variant::Type value_type = value_variant.get_type();
            const bool value_is_null = value_type == Variant::NIL;
            if (!value_is_null && value_type != Variant::STRING && value_type != Variant::STRING_NAME) {
                if (r_error_message != nullptr) {
                    *r_error_message = "PlayFab Multiplayer property dictionaries require String values (or null to delete the entry).";
                }
                return false;
            }

            m_key_strings.push_back(String(key_variant).utf8().get_data());
            m_value_strings.push_back(value_is_null ? std::string() : std::string(String(value_variant).utf8().get_data()));
            m_value_is_null.push_back(value_is_null);
        }

        for (const std::string &key : m_key_strings) {
            m_key_ptrs.push_back(key.c_str());
        }
        for (size_t i = 0; i < m_value_strings.size(); ++i) {
            m_value_ptrs.push_back(m_value_is_null[i] ? nullptr : m_value_strings[i].c_str());
        }
        return true;
    }

    uint32_t count() const {
        return static_cast<uint32_t>(m_key_ptrs.size());
    }

    const char *const *keys() const {
        return m_key_ptrs.empty() ? nullptr : m_key_ptrs.data();
    }

    const char *const *values() const {
        return m_value_ptrs.empty() ? nullptr : m_value_ptrs.data();
    }
};

Dictionary get_property_dictionary(
        PFLobbyHandle p_lobby,
        HRESULT (*p_keys_fn)(PFLobbyHandle, uint32_t *, const char *const **),
        HRESULT (*p_value_fn)(PFLobbyHandle, const char *, const char **)) {
    Dictionary properties;
    if (p_lobby == nullptr || p_keys_fn == nullptr || p_value_fn == nullptr) {
        return properties;
    }

    uint32_t property_count = 0;
    const char *const *keys = nullptr;
    if (FAILED(p_keys_fn(p_lobby, &property_count, &keys)) || keys == nullptr) {
        return properties;
    }

    for (uint32_t i = 0; i < property_count; ++i) {
        const char *key = keys[i];
        if (key == nullptr) {
            continue;
        }

        const char *value = nullptr;
        if (SUCCEEDED(p_value_fn(p_lobby, key, &value)) && value != nullptr) {
            properties[String::utf8(key)] = String::utf8(value);
        }
    }

    return properties;
}

Dictionary get_member_property_dictionary(PFLobbyHandle p_lobby, const PFEntityKey *p_member) {
    Dictionary properties;
    if (p_lobby == nullptr || p_member == nullptr) {
        return properties;
    }

    uint32_t property_count = 0;
    const char *const *keys = nullptr;
    if (FAILED(PFLobbyGetMemberPropertyKeys(p_lobby, p_member, &property_count, &keys)) || keys == nullptr) {
        return properties;
    }

    for (uint32_t i = 0; i < property_count; ++i) {
        const char *key = keys[i];
        if (key == nullptr) {
            continue;
        }

        const char *value = nullptr;
        if (SUCCEEDED(PFLobbyGetMemberProperty(p_lobby, p_member, key, &value)) && value != nullptr) {
            properties[String::utf8(key)] = String::utf8(value);
        }
    }

    return properties;
}

bool validate_user_entity_handle(const Ref<PlayFabUser> &p_user, String *r_error_message) {
    if (!p_user.is_valid() || p_user->get_entity_handle() == nullptr) {
        if (r_error_message != nullptr) {
            *r_error_message = "PlayFab Multiplayer operations require a signed-in PlayFabUser with a valid entity handle.";
        }
        return false;
    }
    return true;
}

PFLobbyAccessPolicy to_lobby_access_policy(int64_t p_value) {
    switch (p_value) {
        case PlayFabLobbyConfig::ACCESS_POLICY_PUBLIC:
            return PFLobbyAccessPolicy::Public;
        case PlayFabLobbyConfig::ACCESS_POLICY_FRIENDS:
            return PFLobbyAccessPolicy::Friends;
        case PlayFabLobbyConfig::ACCESS_POLICY_PRIVATE:
        default:
            return PFLobbyAccessPolicy::Private;
    }
}

PFLobbyOwnerMigrationPolicy to_lobby_owner_migration_policy(int64_t p_value) {
    switch (p_value) {
        case PlayFabLobbyConfig::OWNER_MIGRATION_MANUAL:
            return PFLobbyOwnerMigrationPolicy::Manual;
        case PlayFabLobbyConfig::OWNER_MIGRATION_NONE:
            return PFLobbyOwnerMigrationPolicy::None;
        case PlayFabLobbyConfig::OWNER_MIGRATION_AUTOMATIC:
        default:
            return PFLobbyOwnerMigrationPolicy::Automatic;
    }
}

String ticket_status_to_string(PFMatchmakingTicketStatus p_status) {
    switch (p_status) {
        case PFMatchmakingTicketStatus::Creating:
            return "creating";
        case PFMatchmakingTicketStatus::Joining:
            return "joining";
        case PFMatchmakingTicketStatus::WaitingForPlayers:
            return "waiting_for_players";
        case PFMatchmakingTicketStatus::WaitingForMatch:
            return "waiting_for_match";
        case PFMatchmakingTicketStatus::Matched:
            return "matched";
        case PFMatchmakingTicketStatus::Canceled:
            return "cancelled";
        case PFMatchmakingTicketStatus::Failed:
            return "failed";
    }
    return "unknown";
}

} // namespace

struct PlayFabMultiplayer::PendingOperation {
    int64_t kind = 0;
    Ref<PlayFabPendingSignal> pending_signal;
    Ref<PlayFabLobby> lobby;
    Ref<PlayFabMatchTicket> ticket;
    Ref<PlayFabUser> user;
    Dictionary local_member_property_update;
    bool has_local_member_property_update = false;
    bool replace_local_member_properties = false;
};

void PlayFabMultiplayerConfig::_bind_methods() {}

void PlayFabLobbyConfig::_bind_methods() {
    ClassDB::bind_method(D_METHOD("get_max_players"), &PlayFabLobbyConfig::get_max_players);
    ClassDB::bind_method(D_METHOD("set_max_players", "max_players"), &PlayFabLobbyConfig::set_max_players);
    ClassDB::bind_method(D_METHOD("get_access_policy"), &PlayFabLobbyConfig::get_access_policy);
    ClassDB::bind_method(D_METHOD("set_access_policy", "access_policy"), &PlayFabLobbyConfig::set_access_policy);
    ClassDB::bind_method(D_METHOD("get_owner_migration_policy"), &PlayFabLobbyConfig::get_owner_migration_policy);
    ClassDB::bind_method(D_METHOD("set_owner_migration_policy", "owner_migration_policy"), &PlayFabLobbyConfig::set_owner_migration_policy);
    ClassDB::bind_method(D_METHOD("get_search_properties"), &PlayFabLobbyConfig::get_search_properties);
    ClassDB::bind_method(D_METHOD("set_search_properties", "search_properties"), &PlayFabLobbyConfig::set_search_properties);
    ClassDB::bind_method(D_METHOD("get_lobby_properties"), &PlayFabLobbyConfig::get_lobby_properties);
    ClassDB::bind_method(D_METHOD("set_lobby_properties", "lobby_properties"), &PlayFabLobbyConfig::set_lobby_properties);
    ClassDB::bind_method(D_METHOD("get_member_properties"), &PlayFabLobbyConfig::get_member_properties);
    ClassDB::bind_method(D_METHOD("set_member_properties", "member_properties"), &PlayFabLobbyConfig::set_member_properties);
    ClassDB::bind_method(D_METHOD("get_restrict_invites_to_lobby_owner"), &PlayFabLobbyConfig::get_restrict_invites_to_lobby_owner);
    ClassDB::bind_method(D_METHOD("set_restrict_invites_to_lobby_owner", "restrict"), &PlayFabLobbyConfig::set_restrict_invites_to_lobby_owner);

    ADD_PROPERTY(PropertyInfo(Variant::INT, "max_players"), "set_max_players", "get_max_players");
    ADD_PROPERTY(PropertyInfo(Variant::INT, "access_policy"), "set_access_policy", "get_access_policy");
    ADD_PROPERTY(PropertyInfo(Variant::INT, "owner_migration_policy"), "set_owner_migration_policy", "get_owner_migration_policy");
    ADD_PROPERTY(PropertyInfo(Variant::DICTIONARY, "search_properties"), "set_search_properties", "get_search_properties");
    ADD_PROPERTY(PropertyInfo(Variant::DICTIONARY, "lobby_properties"), "set_lobby_properties", "get_lobby_properties");
    ADD_PROPERTY(PropertyInfo(Variant::DICTIONARY, "member_properties"), "set_member_properties", "get_member_properties");
    ADD_PROPERTY(PropertyInfo(Variant::BOOL, "restrict_invites_to_lobby_owner"), "set_restrict_invites_to_lobby_owner", "get_restrict_invites_to_lobby_owner");

    BIND_CONSTANT(ACCESS_POLICY_PUBLIC);
    BIND_CONSTANT(ACCESS_POLICY_FRIENDS);
    BIND_CONSTANT(ACCESS_POLICY_PRIVATE);
    BIND_CONSTANT(OWNER_MIGRATION_AUTOMATIC);
    BIND_CONSTANT(OWNER_MIGRATION_MANUAL);
    BIND_CONSTANT(OWNER_MIGRATION_NONE);
}

int64_t PlayFabLobbyConfig::get_max_players() const { return m_max_players; }
void PlayFabLobbyConfig::set_max_players(int64_t p_max_players) { m_max_players = p_max_players; }
int64_t PlayFabLobbyConfig::get_access_policy() const { return m_access_policy; }
void PlayFabLobbyConfig::set_access_policy(int64_t p_access_policy) { m_access_policy = p_access_policy; }
int64_t PlayFabLobbyConfig::get_owner_migration_policy() const { return m_owner_migration_policy; }
void PlayFabLobbyConfig::set_owner_migration_policy(int64_t p_owner_migration_policy) { m_owner_migration_policy = p_owner_migration_policy; }
Dictionary PlayFabLobbyConfig::get_search_properties() const { return m_search_properties; }
void PlayFabLobbyConfig::set_search_properties(const Dictionary &p_properties) { m_search_properties = p_properties; }
Dictionary PlayFabLobbyConfig::get_lobby_properties() const { return m_lobby_properties; }
void PlayFabLobbyConfig::set_lobby_properties(const Dictionary &p_properties) { m_lobby_properties = p_properties; }
Dictionary PlayFabLobbyConfig::get_member_properties() const { return m_member_properties; }
void PlayFabLobbyConfig::set_member_properties(const Dictionary &p_properties) { m_member_properties = p_properties; }
bool PlayFabLobbyConfig::get_restrict_invites_to_lobby_owner() const { return m_restrict_invites_to_lobby_owner; }
void PlayFabLobbyConfig::set_restrict_invites_to_lobby_owner(bool p_restrict) { m_restrict_invites_to_lobby_owner = p_restrict; }

void PlayFabLobbyJoinConfig::_bind_methods() {
    ClassDB::bind_method(D_METHOD("get_member_properties"), &PlayFabLobbyJoinConfig::get_member_properties);
    ClassDB::bind_method(D_METHOD("set_member_properties", "member_properties"), &PlayFabLobbyJoinConfig::set_member_properties);
    ADD_PROPERTY(PropertyInfo(Variant::DICTIONARY, "member_properties"), "set_member_properties", "get_member_properties");
}

Dictionary PlayFabLobbyJoinConfig::get_member_properties() const { return m_member_properties; }
void PlayFabLobbyJoinConfig::set_member_properties(const Dictionary &p_properties) { m_member_properties = p_properties; }

void PlayFabLobbySearchConfig::_bind_methods() {
    ClassDB::bind_method(D_METHOD("get_filter"), &PlayFabLobbySearchConfig::get_filter);
    ClassDB::bind_method(D_METHOD("set_filter", "filter"), &PlayFabLobbySearchConfig::set_filter);
    ClassDB::bind_method(D_METHOD("get_order_by"), &PlayFabLobbySearchConfig::get_order_by);
    ClassDB::bind_method(D_METHOD("set_order_by", "order_by"), &PlayFabLobbySearchConfig::set_order_by);
    ClassDB::bind_method(D_METHOD("get_max_results"), &PlayFabLobbySearchConfig::get_max_results);
    ClassDB::bind_method(D_METHOD("set_max_results", "max_results"), &PlayFabLobbySearchConfig::set_max_results);
    ADD_PROPERTY(PropertyInfo(Variant::STRING, "filter"), "set_filter", "get_filter");
    ADD_PROPERTY(PropertyInfo(Variant::STRING, "order_by"), "set_order_by", "get_order_by");
    ADD_PROPERTY(PropertyInfo(Variant::INT, "max_results"), "set_max_results", "get_max_results");
}

String PlayFabLobbySearchConfig::get_filter() const { return m_filter; }
void PlayFabLobbySearchConfig::set_filter(const String &p_filter) { m_filter = p_filter; }
String PlayFabLobbySearchConfig::get_order_by() const { return m_order_by; }
void PlayFabLobbySearchConfig::set_order_by(const String &p_order_by) { m_order_by = p_order_by; }
int64_t PlayFabLobbySearchConfig::get_max_results() const { return m_max_results; }
void PlayFabLobbySearchConfig::set_max_results(int64_t p_max_results) { m_max_results = p_max_results; }

void PlayFabMatchmakingMember::_bind_methods() {
    ClassDB::bind_method(D_METHOD("get_user"), &PlayFabMatchmakingMember::get_user);
    ClassDB::bind_method(D_METHOD("set_user", "user"), &PlayFabMatchmakingMember::set_user);
    ClassDB::bind_method(D_METHOD("get_attributes"), &PlayFabMatchmakingMember::get_attributes);
    ClassDB::bind_method(D_METHOD("set_attributes", "attributes"), &PlayFabMatchmakingMember::set_attributes);
    ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "user", PROPERTY_HINT_NONE, "", PROPERTY_USAGE_DEFAULT, "PlayFabUser"), "set_user", "get_user");
    ADD_PROPERTY(PropertyInfo(Variant::DICTIONARY, "attributes"), "set_attributes", "get_attributes");
}

Ref<PlayFabUser> PlayFabMatchmakingMember::get_user() const { return m_user; }
void PlayFabMatchmakingMember::set_user(const Ref<PlayFabUser> &p_user) { m_user = p_user; }
Dictionary PlayFabMatchmakingMember::get_attributes() const { return m_attributes; }
void PlayFabMatchmakingMember::set_attributes(const Dictionary &p_attributes) { m_attributes = p_attributes; }

void PlayFabMatchmakingTicketConfig::_bind_methods() {
    ClassDB::bind_method(D_METHOD("get_queue_name"), &PlayFabMatchmakingTicketConfig::get_queue_name);
    ClassDB::bind_method(D_METHOD("set_queue_name", "queue_name"), &PlayFabMatchmakingTicketConfig::set_queue_name);
    ClassDB::bind_method(D_METHOD("get_timeout_seconds"), &PlayFabMatchmakingTicketConfig::get_timeout_seconds);
    ClassDB::bind_method(D_METHOD("set_timeout_seconds", "timeout_seconds"), &PlayFabMatchmakingTicketConfig::set_timeout_seconds);
    ClassDB::bind_method(D_METHOD("get_members"), &PlayFabMatchmakingTicketConfig::get_members);
    ClassDB::bind_method(D_METHOD("set_members", "members"), &PlayFabMatchmakingTicketConfig::set_members);
    ADD_PROPERTY(PropertyInfo(Variant::STRING, "queue_name"), "set_queue_name", "get_queue_name");
    ADD_PROPERTY(PropertyInfo(Variant::INT, "timeout_seconds"), "set_timeout_seconds", "get_timeout_seconds");
    ADD_PROPERTY(PropertyInfo(Variant::ARRAY, "members"), "set_members", "get_members");
}

String PlayFabMatchmakingTicketConfig::get_queue_name() const { return m_queue_name; }
void PlayFabMatchmakingTicketConfig::set_queue_name(const String &p_queue_name) { m_queue_name = p_queue_name; }
int64_t PlayFabMatchmakingTicketConfig::get_timeout_seconds() const { return m_timeout_seconds; }
void PlayFabMatchmakingTicketConfig::set_timeout_seconds(int64_t p_timeout_seconds) { m_timeout_seconds = p_timeout_seconds; }
Array PlayFabMatchmakingTicketConfig::get_members() const { return m_members; }
void PlayFabMatchmakingTicketConfig::set_members(const Array &p_members) { m_members = p_members; }

void PlayFabLobbyMember::_bind_methods() {
    ClassDB::bind_method(D_METHOD("get_user_id"), &PlayFabLobbyMember::get_user_id);
    ClassDB::bind_method(D_METHOD("get_entity_key"), &PlayFabLobbyMember::get_entity_key);
    ClassDB::bind_method(D_METHOD("get_properties"), &PlayFabLobbyMember::get_properties);
    ClassDB::bind_method(D_METHOD("is_local_member"), &PlayFabLobbyMember::is_local_member);
    ADD_PROPERTY(PropertyInfo(Variant::STRING, "user_id"), "", "get_user_id");
    ADD_PROPERTY(PropertyInfo(Variant::DICTIONARY, "entity_key"), "", "get_entity_key");
    ADD_PROPERTY(PropertyInfo(Variant::DICTIONARY, "properties"), "", "get_properties");
    ADD_PROPERTY(PropertyInfo(Variant::BOOL, "is_local"), "", "is_local_member");
}

void PlayFabLobbyMember::set_snapshot(const String &p_user_id, const Dictionary &p_entity_key, const Dictionary &p_properties, bool p_is_local) {
    m_user_id = p_user_id;
    m_entity_key = p_entity_key;
    m_properties = p_properties;
    m_is_local = p_is_local;
}
String PlayFabLobbyMember::get_user_id() const { return m_user_id; }
Dictionary PlayFabLobbyMember::get_entity_key() const { return m_entity_key; }
Dictionary PlayFabLobbyMember::get_properties() const { return m_properties; }
bool PlayFabLobbyMember::is_local_member() const { return m_is_local; }

void PlayFabLobbyInvite::_bind_methods() {
    ClassDB::bind_method(D_METHOD("get_lobby_id"), &PlayFabLobbyInvite::get_lobby_id);
    ClassDB::bind_method(D_METHOD("get_connection_string"), &PlayFabLobbyInvite::get_connection_string);
    ClassDB::bind_method(D_METHOD("get_sender_user_id"), &PlayFabLobbyInvite::get_sender_user_id);
    ClassDB::bind_method(D_METHOD("get_sender_entity_key"), &PlayFabLobbyInvite::get_sender_entity_key);
    ClassDB::bind_method(D_METHOD("get_invite_uri"), &PlayFabLobbyInvite::get_invite_uri);
    ClassDB::bind_method(D_METHOD("get_properties"), &PlayFabLobbyInvite::get_properties);
    ADD_PROPERTY(PropertyInfo(Variant::STRING, "lobby_id"), "", "get_lobby_id");
    ADD_PROPERTY(PropertyInfo(Variant::STRING, "connection_string"), "", "get_connection_string");
    ADD_PROPERTY(PropertyInfo(Variant::STRING, "sender_user_id"), "", "get_sender_user_id");
    ADD_PROPERTY(PropertyInfo(Variant::DICTIONARY, "sender_entity_key"), "", "get_sender_entity_key");
    ADD_PROPERTY(PropertyInfo(Variant::STRING, "invite_uri"), "", "get_invite_uri");
    ADD_PROPERTY(PropertyInfo(Variant::DICTIONARY, "properties"), "", "get_properties");
}

void PlayFabLobbyInvite::set_snapshot(const String &p_lobby_id, const String &p_connection_string, const Dictionary &p_sender_entity_key) {
    m_lobby_id = p_lobby_id;
    m_connection_string = p_connection_string;
    m_sender_entity_key = p_sender_entity_key;
    m_sender_user_id = String(p_sender_entity_key.get("id", String()));
}
String PlayFabLobbyInvite::get_lobby_id() const { return m_lobby_id; }
String PlayFabLobbyInvite::get_connection_string() const { return m_connection_string; }
String PlayFabLobbyInvite::get_sender_user_id() const { return m_sender_user_id; }
Dictionary PlayFabLobbyInvite::get_sender_entity_key() const { return m_sender_entity_key; }
String PlayFabLobbyInvite::get_invite_uri() const { return m_invite_uri; }
Dictionary PlayFabLobbyInvite::get_properties() const { return m_properties; }

void PlayFabLobbySummary::_bind_methods() {
    ClassDB::bind_method(D_METHOD("get_lobby_id"), &PlayFabLobbySummary::get_lobby_id);
    ClassDB::bind_method(D_METHOD("get_connection_string"), &PlayFabLobbySummary::get_connection_string);
    ClassDB::bind_method(D_METHOD("get_owner_entity_key"), &PlayFabLobbySummary::get_owner_entity_key);
    ClassDB::bind_method(D_METHOD("get_max_member_count"), &PlayFabLobbySummary::get_max_member_count);
    ClassDB::bind_method(D_METHOD("get_member_count"), &PlayFabLobbySummary::get_member_count);
    ClassDB::bind_method(D_METHOD("get_search_properties"), &PlayFabLobbySummary::get_search_properties);
    ClassDB::bind_method(D_METHOD("get_lobby_properties"), &PlayFabLobbySummary::get_lobby_properties);
    ADD_PROPERTY(PropertyInfo(Variant::STRING, "lobby_id"), "", "get_lobby_id");
    ADD_PROPERTY(PropertyInfo(Variant::STRING, "connection_string"), "", "get_connection_string");
    ADD_PROPERTY(PropertyInfo(Variant::DICTIONARY, "owner_entity_key"), "", "get_owner_entity_key");
    ADD_PROPERTY(PropertyInfo(Variant::INT, "max_member_count"), "", "get_max_member_count");
    ADD_PROPERTY(PropertyInfo(Variant::INT, "member_count"), "", "get_member_count");
    ADD_PROPERTY(PropertyInfo(Variant::DICTIONARY, "search_properties"), "", "get_search_properties");
    ADD_PROPERTY(PropertyInfo(Variant::DICTIONARY, "lobby_properties"), "", "get_lobby_properties");
}

void PlayFabLobbySummary::set_snapshot(
        const String &p_lobby_id,
        const String &p_connection_string,
        const Dictionary &p_owner_entity_key,
        int64_t p_max_member_count,
        int64_t p_member_count,
        const Dictionary &p_search_properties,
        const Dictionary &p_lobby_properties) {
    m_lobby_id = p_lobby_id;
    m_connection_string = p_connection_string;
    m_owner_entity_key = p_owner_entity_key;
    m_max_member_count = p_max_member_count;
    m_member_count = p_member_count;
    m_search_properties = p_search_properties;
    m_lobby_properties = p_lobby_properties;
}
String PlayFabLobbySummary::get_lobby_id() const { return m_lobby_id; }
String PlayFabLobbySummary::get_connection_string() const { return m_connection_string; }
Dictionary PlayFabLobbySummary::get_owner_entity_key() const { return m_owner_entity_key; }
int64_t PlayFabLobbySummary::get_max_member_count() const { return m_max_member_count; }
int64_t PlayFabLobbySummary::get_member_count() const { return m_member_count; }
Dictionary PlayFabLobbySummary::get_search_properties() const { return m_search_properties; }
Dictionary PlayFabLobbySummary::get_lobby_properties() const { return m_lobby_properties; }

void PlayFabLobbySearchResult::_bind_methods() {
    ClassDB::bind_method(D_METHOD("get_lobbies"), &PlayFabLobbySearchResult::get_lobbies);
    ClassDB::bind_method(D_METHOD("get_continuation_token"), &PlayFabLobbySearchResult::get_continuation_token);
    ADD_PROPERTY(PropertyInfo(Variant::ARRAY, "lobbies"), "", "get_lobbies");
    ADD_PROPERTY(PropertyInfo(Variant::STRING, "continuation_token"), "", "get_continuation_token");
}
void PlayFabLobbySearchResult::set_lobbies(const Array &p_lobbies) { m_lobbies = p_lobbies; }
Array PlayFabLobbySearchResult::get_lobbies() const { return m_lobbies; }
String PlayFabLobbySearchResult::get_continuation_token() const { return m_continuation_token; }

void PlayFabLobbyStateChange::_bind_methods() {
    ClassDB::bind_method(D_METHOD("get_kind"), &PlayFabLobbyStateChange::get_kind);
    ClassDB::bind_method(D_METHOD("get_lobby"), &PlayFabLobbyStateChange::get_lobby);
    ClassDB::bind_method(D_METHOD("get_result"), &PlayFabLobbyStateChange::get_result);
    ClassDB::bind_method(D_METHOD("get_member"), &PlayFabLobbyStateChange::get_member);
    ClassDB::bind_method(D_METHOD("get_invite"), &PlayFabLobbyStateChange::get_invite);
    ClassDB::bind_method(D_METHOD("get_user"), &PlayFabLobbyStateChange::get_user);
    ClassDB::bind_method(D_METHOD("get_properties"), &PlayFabLobbyStateChange::get_properties);
    ADD_PROPERTY(PropertyInfo(Variant::INT, "kind"), "", "get_kind");
    ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "lobby", PROPERTY_HINT_NONE, "", PROPERTY_USAGE_DEFAULT, "PlayFabLobby"), "", "get_lobby");
    ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "result", PROPERTY_HINT_NONE, "", PROPERTY_USAGE_DEFAULT, "PlayFabResult"), "", "get_result");
    ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "member", PROPERTY_HINT_NONE, "", PROPERTY_USAGE_DEFAULT, "PlayFabLobbyMember"), "", "get_member");
    ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "invite", PROPERTY_HINT_NONE, "", PROPERTY_USAGE_DEFAULT, "PlayFabLobbyInvite"), "", "get_invite");
    ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "user", PROPERTY_HINT_NONE, "", PROPERTY_USAGE_DEFAULT, "PlayFabUser"), "", "get_user");
    ADD_PROPERTY(PropertyInfo(Variant::DICTIONARY, "properties"), "", "get_properties");
}

void PlayFabLobbyStateChange::set_values(int64_t p_kind, const Ref<RefCounted> &p_lobby, const Ref<PlayFabResult> &p_result) {
    m_kind = p_kind;
    m_lobby = p_lobby;
    m_result = p_result;
}
void PlayFabLobbyStateChange::set_member(const Ref<PlayFabLobbyMember> &p_member) { m_member = p_member; }
void PlayFabLobbyStateChange::set_invite(const Ref<PlayFabLobbyInvite> &p_invite) { m_invite = p_invite; }
void PlayFabLobbyStateChange::set_user(const Ref<PlayFabUser> &p_user) { m_user = p_user; }
void PlayFabLobbyStateChange::set_properties(const Dictionary &p_properties) { m_properties = p_properties; }
int64_t PlayFabLobbyStateChange::get_kind() const { return m_kind; }
Ref<RefCounted> PlayFabLobbyStateChange::get_lobby() const { return m_lobby; }
Ref<PlayFabResult> PlayFabLobbyStateChange::get_result() const { return m_result; }
Ref<PlayFabLobbyMember> PlayFabLobbyStateChange::get_member() const { return m_member; }
Ref<PlayFabLobbyInvite> PlayFabLobbyStateChange::get_invite() const { return m_invite; }
Ref<PlayFabUser> PlayFabLobbyStateChange::get_user() const { return m_user; }
Dictionary PlayFabLobbyStateChange::get_properties() const { return m_properties; }

void PlayFabMatchTicketStateChange::_bind_methods() {
    ClassDB::bind_method(D_METHOD("get_kind"), &PlayFabMatchTicketStateChange::get_kind);
    ClassDB::bind_method(D_METHOD("get_ticket"), &PlayFabMatchTicketStateChange::get_ticket);
    ClassDB::bind_method(D_METHOD("get_result"), &PlayFabMatchTicketStateChange::get_result);
    ClassDB::bind_method(D_METHOD("get_status"), &PlayFabMatchTicketStateChange::get_status);
    ClassDB::bind_method(D_METHOD("get_match_id"), &PlayFabMatchTicketStateChange::get_match_id);
    ClassDB::bind_method(D_METHOD("get_arranged_lobby_connection_string"), &PlayFabMatchTicketStateChange::get_arranged_lobby_connection_string);
    ADD_PROPERTY(PropertyInfo(Variant::INT, "kind"), "", "get_kind");
    ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "ticket", PROPERTY_HINT_NONE, "", PROPERTY_USAGE_DEFAULT, "PlayFabMatchTicket"), "", "get_ticket");
    ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "result", PROPERTY_HINT_NONE, "", PROPERTY_USAGE_DEFAULT, "PlayFabResult"), "", "get_result");
    ADD_PROPERTY(PropertyInfo(Variant::INT, "status"), "", "get_status");
    ADD_PROPERTY(PropertyInfo(Variant::STRING, "match_id"), "", "get_match_id");
    ADD_PROPERTY(PropertyInfo(Variant::STRING, "arranged_lobby_connection_string"), "", "get_arranged_lobby_connection_string");
}
void PlayFabMatchTicketStateChange::set_values(
        int64_t p_kind,
        const Ref<RefCounted> &p_ticket,
        const Ref<PlayFabResult> &p_result,
        int64_t p_status,
        const String &p_match_id,
        const String &p_arranged_lobby_connection_string) {
    m_kind = p_kind;
    m_ticket = p_ticket;
    m_result = p_result;
    m_status = p_status;
    m_match_id = p_match_id;
    m_arranged_lobby_connection_string = p_arranged_lobby_connection_string;
}
int64_t PlayFabMatchTicketStateChange::get_kind() const { return m_kind; }
Ref<RefCounted> PlayFabMatchTicketStateChange::get_ticket() const { return m_ticket; }
Ref<PlayFabResult> PlayFabMatchTicketStateChange::get_result() const { return m_result; }
int64_t PlayFabMatchTicketStateChange::get_status() const { return m_status; }
String PlayFabMatchTicketStateChange::get_match_id() const { return m_match_id; }
String PlayFabMatchTicketStateChange::get_arranged_lobby_connection_string() const { return m_arranged_lobby_connection_string; }

void PlayFabMultiplayerStateChange::_bind_methods() {
    ClassDB::bind_method(D_METHOD("get_kind"), &PlayFabMultiplayerStateChange::get_kind);
    ClassDB::bind_method(D_METHOD("get_lobby"), &PlayFabMultiplayerStateChange::get_lobby);
    ClassDB::bind_method(D_METHOD("get_ticket"), &PlayFabMultiplayerStateChange::get_ticket);
    ClassDB::bind_method(D_METHOD("get_result"), &PlayFabMultiplayerStateChange::get_result);
    ClassDB::bind_method(D_METHOD("get_properties"), &PlayFabMultiplayerStateChange::get_properties);
    ADD_PROPERTY(PropertyInfo(Variant::INT, "kind"), "", "get_kind");
    ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "lobby", PROPERTY_HINT_NONE, "", PROPERTY_USAGE_DEFAULT, "PlayFabLobby"), "", "get_lobby");
    ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "ticket", PROPERTY_HINT_NONE, "", PROPERTY_USAGE_DEFAULT, "PlayFabMatchTicket"), "", "get_ticket");
    ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "result", PROPERTY_HINT_NONE, "", PROPERTY_USAGE_DEFAULT, "PlayFabResult"), "", "get_result");
    ADD_PROPERTY(PropertyInfo(Variant::DICTIONARY, "properties"), "", "get_properties");
}
void PlayFabMultiplayerStateChange::set_values(int64_t p_kind, const Ref<RefCounted> &p_lobby, const Ref<RefCounted> &p_ticket, const Ref<PlayFabResult> &p_result) {
    m_kind = p_kind;
    m_lobby = p_lobby;
    m_ticket = p_ticket;
    m_result = p_result;
}
void PlayFabMultiplayerStateChange::set_properties(const Dictionary &p_properties) { m_properties = p_properties; }
int64_t PlayFabMultiplayerStateChange::get_kind() const { return m_kind; }
Ref<RefCounted> PlayFabMultiplayerStateChange::get_lobby() const { return m_lobby; }
Ref<RefCounted> PlayFabMultiplayerStateChange::get_ticket() const { return m_ticket; }
Ref<PlayFabResult> PlayFabMultiplayerStateChange::get_result() const { return m_result; }
Dictionary PlayFabMultiplayerStateChange::get_properties() const { return m_properties; }

void PlayFabLobby::_bind_methods() {
    ClassDB::bind_method(D_METHOD("get_lobby_id"), &PlayFabLobby::get_lobby_id);
    ClassDB::bind_method(D_METHOD("get_connection_string"), &PlayFabLobby::get_connection_string);
    ClassDB::bind_method(D_METHOD("get_owner_entity_key"), &PlayFabLobby::get_owner_entity_key);
    ClassDB::bind_method(D_METHOD("get_max_member_count"), &PlayFabLobby::get_max_member_count);
    ClassDB::bind_method(D_METHOD("get_member_count"), &PlayFabLobby::get_member_count);
    ClassDB::bind_method(D_METHOD("get_members"), &PlayFabLobby::get_members);
    ClassDB::bind_method(D_METHOD("get_properties"), &PlayFabLobby::get_properties);
    ClassDB::bind_method(D_METHOD("get_search_properties"), &PlayFabLobby::get_search_properties);
    ClassDB::bind_method(D_METHOD("is_owner", "user"), &PlayFabLobby::is_owner);
    ClassDB::bind_method(D_METHOD("set_properties_async", "properties"), &PlayFabLobby::set_properties_async);
    ClassDB::bind_method(D_METHOD("set_member_properties_async", "properties"), &PlayFabLobby::set_member_properties_async);
    ClassDB::bind_method(D_METHOD("leave_async"), &PlayFabLobby::leave_async);
#ifdef GODOT_PLAYFAB_TEST_HOOKS
    ClassDB::bind_method(D_METHOD("_test_seed_local_member", "entity_key", "properties"), &PlayFabLobby::_test_seed_local_member);
    ClassDB::bind_method(D_METHOD("_test_apply_local_member_property_update", "properties"), &PlayFabLobby::_test_apply_local_member_property_update);
#endif
    ADD_PROPERTY(PropertyInfo(Variant::STRING, "lobby_id"), "", "get_lobby_id");
    ADD_PROPERTY(PropertyInfo(Variant::STRING, "connection_string"), "", "get_connection_string");
    ADD_PROPERTY(PropertyInfo(Variant::DICTIONARY, "owner_entity_key"), "", "get_owner_entity_key");
    ADD_PROPERTY(PropertyInfo(Variant::INT, "max_member_count"), "", "get_max_member_count");
    ADD_PROPERTY(PropertyInfo(Variant::INT, "member_count"), "", "get_member_count");
    ADD_PROPERTY(PropertyInfo(Variant::ARRAY, "members"), "", "get_members");
    ADD_PROPERTY(PropertyInfo(Variant::DICTIONARY, "properties"), "", "get_properties");
    ADD_PROPERTY(PropertyInfo(Variant::DICTIONARY, "search_properties"), "", "get_search_properties");
    ADD_SIGNAL(MethodInfo("state_changed", PropertyInfo(Variant::OBJECT, "change", PROPERTY_HINT_NONE, "", PROPERTY_USAGE_DEFAULT, "PlayFabLobbyStateChange")));

    BIND_CONSTANT(MEMBER_ADDED);
    BIND_CONSTANT(MEMBER_REMOVED);
    BIND_CONSTANT(MEMBER_UPDATED);
    BIND_CONSTANT(PROPERTIES_UPDATED);
    BIND_CONSTANT(OWNER_CHANGED);
    BIND_CONSTANT(DISCONNECTED);
}

void PlayFabLobby::set_owner(PlayFabMultiplayer *p_owner) { m_owner = p_owner; }
void PlayFabLobby::adopt_handle(PFLobbyHandle p_lobby_handle, const Ref<PlayFabUser> &p_local_user) {
    m_lobby_handle = p_lobby_handle;
    m_local_user = p_local_user;
}
PFLobbyHandle PlayFabLobby::get_native_handle() const { return m_lobby_handle; }
Ref<PlayFabUser> PlayFabLobby::get_local_user() const { return m_local_user; }
void PlayFabLobby::mark_disconnected() { m_disconnected = true; m_lobby_handle = nullptr; }
bool PlayFabLobby::is_disconnected() const { return m_disconnected; }

bool PlayFabLobby::_is_local_entity_key(const Dictionary &p_entity_key) const {
    if (!m_local_user.is_valid()) {
        return false;
    }

    const Dictionary local_key = m_local_user->get_entity_key();
    const String local_id = String(local_key.get("id", String()));
    const String member_id = String(p_entity_key.get("id", String()));
    if (local_id.is_empty() || member_id != local_id) {
        return false;
    }

    const String local_type = String(local_key.get("type", String()));
    const String member_type = String(p_entity_key.get("type", String()));
    return local_type.is_empty() || member_type.is_empty() || member_type == local_type;
}

Ref<PlayFabLobbyMember> PlayFabLobby::_find_local_member() const {
    for (int i = 0; i < m_members.size(); ++i) {
        Ref<PlayFabLobbyMember> member = m_members[i];
        if (!member.is_valid()) {
            continue;
        }
        if (member->is_local_member() || _is_local_entity_key(member->get_entity_key())) {
            return member;
        }
    }
    return Ref<PlayFabLobbyMember>();
}

void PlayFabLobby::_apply_local_member_properties_to_snapshot(bool p_allow_synthetic_create) {
    if (!m_local_member_properties_known) {
        return;
    }

    Ref<PlayFabLobbyMember> local_member = _find_local_member();
    if (local_member.is_valid()) {
        const Dictionary entity_key = local_member->get_entity_key();
        local_member->set_snapshot(
                String(entity_key.get("id", String())),
                entity_key,
                copy_dictionary(m_local_member_properties),
                true);
        return;
    }

    // Update path (apply_local_member_property_update): when the SDK snapshot
    // no longer contains the local user — for example during the local
    // MemberRemoved state change that precedes LeaveLobbyCompleted, or after
    // _refresh_snapshot drops a leaving user — do NOT synthesize a stale
    // local member. The leave/remove flow expects the refreshed snapshot to
    // be authoritative.
    if (!p_allow_synthetic_create) {
        return;
    }

    if (!m_local_user.is_valid()) {
        return;
    }

    const Dictionary entity_key = m_local_user->get_entity_key();
    Ref<PlayFabLobbyMember> member;
    member.instantiate();
    member->set_snapshot(
            String(entity_key.get("id", String())),
            entity_key,
            copy_dictionary(m_local_member_properties),
            true);
    m_members.push_back(member);
    m_member_count = std::max<int64_t>(m_member_count, static_cast<int64_t>(m_members.size()));
}

void PlayFabLobby::replace_local_member_properties(const Dictionary &p_properties) {
    m_local_member_properties = normalize_property_dictionary(p_properties);
    m_local_member_properties_known = true;
    // Create/join seeding: the SDK has not necessarily emitted a MemberAdded
    // callback for the local user yet, so allow the synthetic-create
    // fallback to seed the snapshot.
    _apply_local_member_properties_to_snapshot(true);
}

void PlayFabLobby::apply_local_member_property_update(const Dictionary &p_update) {
    if (!m_local_member_properties_known) {
        Ref<PlayFabLobbyMember> local_member = _find_local_member();
        m_local_member_properties = local_member.is_valid() ? copy_dictionary(local_member->get_properties()) : Dictionary();
        m_local_member_properties_known = true;
    }

    apply_property_update(m_local_member_properties, p_update);
    // Update path: do NOT synthesize a stale local member if the SDK
    // snapshot has already dropped the local user (mid-leave/remove).
    _apply_local_member_properties_to_snapshot(false);
}

#ifdef GODOT_PLAYFAB_TEST_HOOKS
void PlayFabLobby::_test_seed_local_member(const Dictionary &p_entity_key, const Dictionary &p_properties) {
    Ref<PlayFabLobbyMember> member;
    member.instantiate();
    member->set_snapshot(
            String(p_entity_key.get("id", String())),
            p_entity_key,
            normalize_property_dictionary(p_properties),
            true);
    m_members.clear();
    m_members.push_back(member);
    m_member_count = 1;
    replace_local_member_properties(p_properties);
}

void PlayFabLobby::_test_apply_local_member_property_update(const Dictionary &p_update) {
    apply_local_member_property_update(p_update);
}
#endif

HRESULT PlayFabLobby::refresh_snapshot() {
    if (m_lobby_handle == nullptr) {
        return E_HANDLE;
    }

    const char *lobby_id = nullptr;
    if (SUCCEEDED(PFLobbyGetLobbyId(m_lobby_handle, &lobby_id))) {
        m_lobby_id = pf_string(lobby_id);
    }

    const char *connection_string = nullptr;
    if (SUCCEEDED(PFLobbyGetConnectionString(m_lobby_handle, &connection_string))) {
        m_connection_string = pf_string(connection_string);
    }

    uint32_t max_member_count = 0;
    if (SUCCEEDED(PFLobbyGetMaxMemberCount(m_lobby_handle, &max_member_count))) {
        m_max_member_count = static_cast<int64_t>(max_member_count);
    }

    const PFEntityKey *owner = nullptr;
    if (SUCCEEDED(PFLobbyGetOwner(m_lobby_handle, &owner))) {
        m_owner_entity_key = entity_key_to_dictionary(owner);
    }

    m_properties = get_property_dictionary(m_lobby_handle, PFLobbyGetLobbyPropertyKeys, PFLobbyGetLobbyProperty);
    m_search_properties = get_property_dictionary(m_lobby_handle, PFLobbyGetSearchPropertyKeys, PFLobbyGetSearchProperty);

    uint32_t member_count = 0;
    const PFEntityKey *members = nullptr;
    if (SUCCEEDED(PFLobbyGetMembers(m_lobby_handle, &member_count, &members))) {
        m_member_count = static_cast<int64_t>(member_count);
        Array member_wrappers;
        for (uint32_t i = 0; i < member_count; ++i) {
            const PFEntityKey *member_key = &members[i];
            Ref<PlayFabLobbyMember> member;
            member.instantiate();
            const Dictionary entity_key = entity_key_to_dictionary(member_key);
            const bool is_local = entity_key_equals_user(member_key, m_local_user) || _is_local_entity_key(entity_key);
            Dictionary member_properties = get_member_property_dictionary(m_lobby_handle, member_key);
            if (is_local) {
                if (m_local_member_properties_known) {
                    member_properties = copy_dictionary(m_local_member_properties);
                } else {
                    m_local_member_properties = copy_dictionary(member_properties);
                    m_local_member_properties_known = true;
                }
            }
            member->set_snapshot(
                    String(entity_key.get("id", String())),
                    entity_key,
                    member_properties,
                    is_local);
            member_wrappers.push_back(member);
        }
        m_members = member_wrappers;
        // refresh_snapshot is authoritative: if the SDK has dropped the local
        // user (mid-leave / MemberRemoved), do NOT synthesize a stale local
        // member here. Callers that want to seed the local snapshot from a
        // pending operation (CreateAndJoinLobbyCompleted /
        // JoinLobbyCompleted / JoinArrangedLobbyCompleted) follow this call
        // with replace_local_member_properties(), which is allowed to
        // synthesize because the local user has just joined.
        _apply_local_member_properties_to_snapshot(false);
    }

    return S_OK;
}

String PlayFabLobby::get_lobby_id() const { return m_lobby_id; }
String PlayFabLobby::get_connection_string() const { return m_connection_string; }
Dictionary PlayFabLobby::get_owner_entity_key() const { return m_owner_entity_key; }
int64_t PlayFabLobby::get_max_member_count() const { return m_max_member_count; }
int64_t PlayFabLobby::get_member_count() const { return m_member_count; }
Array PlayFabLobby::get_members() const { return m_members; }
Dictionary PlayFabLobby::get_properties() const { return m_properties; }
Dictionary PlayFabLobby::get_search_properties() const { return m_search_properties; }
Ref<PlayFabLobbyMember> PlayFabLobby::find_member(const Dictionary &p_entity_key) const {
    const String id = p_entity_key.get("id", String());
    const String type = p_entity_key.get("type", String());
    if (id.is_empty() && type.is_empty()) {
        return Ref<PlayFabLobbyMember>();
    }
    for (int i = 0; i < m_members.size(); ++i) {
        Ref<PlayFabLobbyMember> member = m_members[i];
        if (!member.is_valid()) {
            continue;
        }
        const Dictionary mk = member->get_entity_key();
        const String member_id = String(mk.get("id", String()));
        const String member_type = String(mk.get("type", String()));
        if (member_id == id && (member_type == type || member_type.is_empty() || type.is_empty())) {
            return member;
        }
    }
    return Ref<PlayFabLobbyMember>();
}
bool PlayFabLobby::is_owner(const Ref<PlayFabUser> &p_user) const {
    return p_user.is_valid() && String(m_owner_entity_key.get("id", String())) == String(p_user->get_entity_key().get("id", String())) &&
            String(m_owner_entity_key.get("type", String())) == String(p_user->get_entity_key().get("type", String()));
}
Signal PlayFabLobby::set_properties_async(const Dictionary &p_properties) {
    if (m_owner == nullptr) {
        return detached_error_signal(E_INVALIDARG, "invalid_lobby", "set_properties_async requires a PlayFabLobby created or joined through PlayFab.multiplayer.");
    }
    return m_owner->_set_lobby_properties_async(Ref<PlayFabLobby>(this), p_properties);
}
Signal PlayFabLobby::set_member_properties_async(const Dictionary &p_properties) {
    if (m_owner == nullptr) {
        return detached_error_signal(E_INVALIDARG, "invalid_lobby", "set_member_properties_async requires a PlayFabLobby created or joined through PlayFab.multiplayer.");
    }
    return m_owner->_set_member_properties_async(Ref<PlayFabLobby>(this), p_properties);
}
Signal PlayFabLobby::leave_async() {
    if (m_owner == nullptr) {
        return detached_error_signal(E_INVALIDARG, "invalid_lobby", "leave_async requires a PlayFabLobby created or joined through PlayFab.multiplayer.");
    }
    return m_owner->_leave_lobby_async(Ref<PlayFabLobby>(this));
}

void PlayFabMatchTicket::_bind_methods() {
    ClassDB::bind_method(D_METHOD("get_ticket_id"), &PlayFabMatchTicket::get_ticket_id);
    ClassDB::bind_method(D_METHOD("get_queue_name"), &PlayFabMatchTicket::get_queue_name);
    ClassDB::bind_method(D_METHOD("get_status"), &PlayFabMatchTicket::get_status);
    ClassDB::bind_method(D_METHOD("get_members"), &PlayFabMatchTicket::get_members);
    ClassDB::bind_method(D_METHOD("get_match_id"), &PlayFabMatchTicket::get_match_id);
    ClassDB::bind_method(D_METHOD("get_arranged_lobby_connection_string"), &PlayFabMatchTicket::get_arranged_lobby_connection_string);
    ClassDB::bind_method(D_METHOD("get_properties"), &PlayFabMatchTicket::get_properties);
    ClassDB::bind_method(D_METHOD("is_complete"), &PlayFabMatchTicket::is_complete);
    ClassDB::bind_method(D_METHOD("is_cancelled"), &PlayFabMatchTicket::is_cancelled);
    ClassDB::bind_method(D_METHOD("refresh_async"), &PlayFabMatchTicket::refresh_async);
    ClassDB::bind_method(D_METHOD("cancel_async"), &PlayFabMatchTicket::cancel_async);
    ADD_PROPERTY(PropertyInfo(Variant::STRING, "ticket_id"), "", "get_ticket_id");
    ADD_PROPERTY(PropertyInfo(Variant::STRING, "queue_name"), "", "get_queue_name");
    ADD_PROPERTY(PropertyInfo(Variant::INT, "status"), "", "get_status");
    ADD_PROPERTY(PropertyInfo(Variant::ARRAY, "members"), "", "get_members");
    ADD_PROPERTY(PropertyInfo(Variant::STRING, "match_id"), "", "get_match_id");
    ADD_PROPERTY(PropertyInfo(Variant::STRING, "arranged_lobby_connection_string"), "", "get_arranged_lobby_connection_string");
    ADD_PROPERTY(PropertyInfo(Variant::DICTIONARY, "properties"), "", "get_properties");
    ADD_SIGNAL(MethodInfo("state_changed", PropertyInfo(Variant::OBJECT, "change", PROPERTY_HINT_NONE, "", PROPERTY_USAGE_DEFAULT, "PlayFabMatchTicketStateChange")));

    BIND_CONSTANT(CREATED);
    BIND_CONSTANT(STATUS_CHANGED);
    BIND_CONSTANT(COMPLETED);
    BIND_CONSTANT(CANCELLED);
    BIND_CONSTANT(FAILED);
}

void PlayFabMatchTicket::set_owner(PlayFabMultiplayer *p_owner) { m_owner = p_owner; }
void PlayFabMatchTicket::adopt_handle(PFMatchmakingTicketHandle p_ticket_handle, const String &p_queue_name, const Array &p_members) {
    m_ticket_handle = p_ticket_handle;
    m_queue_name = p_queue_name;
    m_members = p_members;
}
PFMatchmakingTicketHandle PlayFabMatchTicket::get_native_handle() const { return m_ticket_handle; }
void PlayFabMatchTicket::mark_destroyed() {
    m_destroyed = true;
    m_ticket_handle = nullptr;
}
bool PlayFabMatchTicket::is_destroyed() const { return m_destroyed; }

HRESULT PlayFabMatchTicket::refresh_snapshot() {
    if (m_ticket_handle == nullptr) {
        return E_HANDLE;
    }

    const char *ticket_id = nullptr;
    if (SUCCEEDED(PFMatchmakingTicketGetTicketId(m_ticket_handle, &ticket_id))) {
        m_ticket_id = pf_string(ticket_id);
    }

    PFMatchmakingTicketStatus status = PFMatchmakingTicketStatus::Creating;
    if (SUCCEEDED(PFMatchmakingTicketGetStatus(m_ticket_handle, &status))) {
        m_status = static_cast<int64_t>(status);
        m_properties["status_name"] = ticket_status_to_string(status);
    }

    const PFMatchmakingMatchDetails *match = nullptr;
    if (SUCCEEDED(PFMatchmakingTicketGetMatch(m_ticket_handle, &match)) && match != nullptr) {
        set_match_details(pf_string(match->matchId), pf_string(match->lobbyArrangementString));
    }

    return S_OK;
}

void PlayFabMatchTicket::set_match_details(const String &p_match_id, const String &p_arranged_lobby_connection_string) {
    m_match_id = p_match_id;
    m_arranged_lobby_connection_string = p_arranged_lobby_connection_string;
}

String PlayFabMatchTicket::get_ticket_id() const { return m_ticket_id; }
String PlayFabMatchTicket::get_queue_name() const { return m_queue_name; }
int64_t PlayFabMatchTicket::get_status() const { return m_status; }
Array PlayFabMatchTicket::get_members() const { return m_members; }
String PlayFabMatchTicket::get_match_id() const { return m_match_id; }
String PlayFabMatchTicket::get_arranged_lobby_connection_string() const { return m_arranged_lobby_connection_string; }
Dictionary PlayFabMatchTicket::get_properties() const { return m_properties; }
bool PlayFabMatchTicket::is_complete() const {
    return m_status == static_cast<int64_t>(PFMatchmakingTicketStatus::Matched) ||
            m_status == static_cast<int64_t>(PFMatchmakingTicketStatus::Canceled) ||
            m_status == static_cast<int64_t>(PFMatchmakingTicketStatus::Failed);
}
bool PlayFabMatchTicket::is_cancelled() const {
    return m_status == static_cast<int64_t>(PFMatchmakingTicketStatus::Canceled);
}
Signal PlayFabMatchTicket::refresh_async() {
    if (m_owner == nullptr) {
        return detached_error_signal(E_INVALIDARG, "invalid_match_ticket", "refresh_async requires a PlayFabMatchTicket created through PlayFab.multiplayer.");
    }
    return m_owner->_refresh_match_ticket_async(Ref<PlayFabMatchTicket>(this));
}
Signal PlayFabMatchTicket::cancel_async() {
    if (m_owner == nullptr) {
        return detached_error_signal(E_INVALIDARG, "invalid_match_ticket", "cancel_async requires a PlayFabMatchTicket created through PlayFab.multiplayer.");
    }
    return m_owner->_cancel_match_ticket_async(Ref<PlayFabMatchTicket>(this));
}

PlayFabMultiplayer::~PlayFabMultiplayer() {
    shutdown();
}

void PlayFabMultiplayer::_bind_methods() {
    ClassDB::bind_method(D_METHOD("is_initialized"), &PlayFabMultiplayer::is_initialized);
    ClassDB::bind_method(D_METHOD("initialize_async", "config"), &PlayFabMultiplayer::initialize_async, DEFVAL(Ref<PlayFabMultiplayerConfig>()));
    ClassDB::bind_method(D_METHOD("shutdown_async"), &PlayFabMultiplayer::shutdown_async);
    ClassDB::bind_method(D_METHOD("create_lobby_async", "user", "config"), &PlayFabMultiplayer::create_lobby_async, DEFVAL(Ref<PlayFabLobbyConfig>()));
    ClassDB::bind_method(D_METHOD("join_lobby_async", "user", "connection_string", "config"), &PlayFabMultiplayer::join_lobby_async, DEFVAL(Ref<PlayFabLobbyJoinConfig>()));
    ClassDB::bind_method(D_METHOD("join_arranged_lobby_async", "user", "connection_string", "config"), &PlayFabMultiplayer::join_arranged_lobby_async, DEFVAL(Ref<PlayFabLobbyJoinConfig>()));
    ClassDB::bind_method(D_METHOD("find_lobbies_async", "user", "search"), &PlayFabMultiplayer::find_lobbies_async, DEFVAL(Ref<PlayFabLobbySearchConfig>()));
    ClassDB::bind_method(D_METHOD("create_match_ticket_async", "user", "config"), &PlayFabMultiplayer::create_match_ticket_async);
    ClassDB::bind_method(D_METHOD("get_lobbies"), &PlayFabMultiplayer::get_lobbies);
    ClassDB::bind_method(D_METHOD("get_lobby", "lobby_id"), &PlayFabMultiplayer::get_lobby);
    ClassDB::bind_method(D_METHOD("get_match_tickets"), &PlayFabMultiplayer::get_match_tickets);
#ifdef GODOT_PLAYFAB_TEST_HOOKS
    ClassDB::bind_method(D_METHOD("_test_enqueue_shutdown_pending"), &PlayFabMultiplayer::_test_enqueue_shutdown_pending);
    ClassDB::bind_method(D_METHOD("_test_pending_operation_count"), &PlayFabMultiplayer::_test_pending_operation_count);
#endif

    ADD_SIGNAL(MethodInfo("state_changed", PropertyInfo(Variant::OBJECT, "change", PROPERTY_HINT_NONE, "", PROPERTY_USAGE_DEFAULT, "PlayFabMultiplayerStateChange")));
    ADD_SIGNAL(MethodInfo("invite_received", PropertyInfo(Variant::OBJECT, "invite", PROPERTY_HINT_NONE, "", PROPERTY_USAGE_DEFAULT, "PlayFabLobbyInvite")));
    ADD_SIGNAL(MethodInfo("multiplayer_error", PropertyInfo(Variant::OBJECT, "result", PROPERTY_HINT_NONE, "", PROPERTY_USAGE_DEFAULT, "PlayFabResult")));
}

void PlayFabMultiplayer::set_owner(PlayFab *p_owner) { m_owner = p_owner; }
bool PlayFabMultiplayer::is_initialized() const { return m_initialized; }
bool PlayFabMultiplayer::has_deferred_shutdown() const { return m_shutdown_deferred_until_dispatch_complete; }

PlayFabRuntime *PlayFabMultiplayer::_get_runtime() const {
    return m_owner != nullptr ? m_owner->get_runtime() : nullptr;
}

Ref<PlayFabPendingSignal> PlayFabMultiplayer::_make_pending_signal() {
    PlayFabRuntime *runtime = _get_runtime();
    if (runtime != nullptr) {
        return runtime->make_pending_signal();
    }
    Ref<PlayFabPendingSignal> pending_signal;
    pending_signal.instantiate();
    return pending_signal;
}

Signal PlayFabMultiplayer::_make_error_signal(HRESULT p_hresult, const String &p_code, const String &p_message, const Variant &p_data) {
    PlayFabRuntime *runtime = _get_runtime();
    if (runtime != nullptr) {
        return runtime->make_error_signal(p_hresult, p_code, p_message, p_data);
    }

    Ref<PlayFabPendingSignal> pending_signal;
    pending_signal.instantiate();
    pending_signal->complete_deferred(PlayFabResult::error_result(p_hresult, p_code, p_message, p_data));
    return pending_signal->get_completed_signal();
}

PlayFabMultiplayer::PendingOperation *PlayFabMultiplayer::_create_pending_operation(int64_t p_kind, const Ref<PlayFabPendingSignal> &p_pending_signal) {
    PendingOperation *operation = new PendingOperation;
    operation->kind = p_kind;
    operation->pending_signal = p_pending_signal;
    m_pending_operations.push_back(operation);
    return operation;
}

void PlayFabMultiplayer::_cancel_active_pending_operations(const String &p_cancel_message) {
    while (!m_pending_operations.empty()) {
        std::vector<PendingOperation *> pending_operations;
        pending_operations.swap(m_pending_operations);
        for (PendingOperation *operation : pending_operations) {
            _defer_pending_delete(operation);
        }
        for (PendingOperation *operation : pending_operations) {
            if (operation != nullptr && operation->pending_signal.is_valid()) {
                operation->pending_signal->complete(PlayFabResult::cancelled(p_cancel_message));
            }
        }
    }
}

void PlayFabMultiplayer::_defer_pending_delete(PendingOperation *p_operation) {
    if (p_operation == nullptr) {
        return;
    }
    m_pending_operations.erase(std::remove(m_pending_operations.begin(), m_pending_operations.end(), p_operation), m_pending_operations.end());
    if (std::find(m_pending_operations_deferred_delete.begin(), m_pending_operations_deferred_delete.end(), p_operation) == m_pending_operations_deferred_delete.end()) {
        m_pending_operations_deferred_delete.push_back(p_operation);
    }
}

void PlayFabMultiplayer::_delete_deferred_pending_operations() {
    std::vector<PendingOperation *> active_operations;
    active_operations.swap(m_pending_operations);
    for (PendingOperation *operation : active_operations) {
        _defer_pending_delete(operation);
    }

    for (PendingOperation *operation : m_pending_operations_deferred_delete) {
        delete operation;
    }
    m_pending_operations_deferred_delete.clear();
}

void PlayFabMultiplayer::_complete_shutdown_pending_signals() {
    std::vector<Ref<PlayFabPendingSignal>> pending_signals;
    pending_signals.swap(m_shutdown_pending_signals);
    for (const Ref<PlayFabPendingSignal> &pending_signal : pending_signals) {
        if (pending_signal.is_valid()) {
            pending_signal->complete(PlayFabResult::ok_result());
        }
    }
}

void PlayFabMultiplayer::_complete_pending_operation(PendingOperation *p_operation, const Ref<PlayFabResult> &p_result) {
    if (p_operation == nullptr) {
        return;
    }

    m_pending_operations.erase(std::remove(m_pending_operations.begin(), m_pending_operations.end(), p_operation), m_pending_operations.end());

    if (p_operation->pending_signal.is_valid()) {
        Ref<PlayFabResult> final_result = p_result;
        if (p_operation->pending_signal->was_cancel_requested()) {
            final_result = PlayFabResult::cancelled("PlayFab Multiplayer operation cancelled.");
        }
        p_operation->pending_signal->complete(final_result);
    }

    // Deferring after complete() (rather than both before and after) keeps the
    // operation alive across the complete() call without double-tracking it.
    // The check is re-read here in case complete() re-entrantly flipped
    // m_shutting_down (e.g. an awaiter calling PlayFab.shutdown()).
    if (m_shutting_down) {
        _defer_pending_delete(p_operation);
    } else {
        delete p_operation;
    }
}

void PlayFabMultiplayer::_release_pending_operation(PendingOperation *p_operation) {
    m_pending_operations.erase(std::remove(m_pending_operations.begin(), m_pending_operations.end(), p_operation), m_pending_operations.end());
    if (m_shutting_down) {
        _defer_pending_delete(p_operation);
        return;
    }
    delete p_operation;
}

PlayFabMultiplayer::PendingOperation *PlayFabMultiplayer::_find_pending_ticket_operation(const Ref<PlayFabMatchTicket> &p_ticket, int64_t p_kind) const {
    if (p_ticket.is_null()) {
        return nullptr;
    }
    for (PendingOperation *operation : m_pending_operations) {
        if (operation != nullptr && operation->kind == p_kind && operation->ticket == p_ticket) {
            return operation;
        }
    }
    return nullptr;
}

Ref<PlayFabLobby> PlayFabMultiplayer::_find_lobby(PFLobbyHandle p_lobby_handle) const {
    for (const Ref<PlayFabLobby> &lobby : m_lobbies) {
        if (lobby.is_valid() && lobby->get_native_handle() == p_lobby_handle) {
            return lobby;
        }
    }
    return Ref<PlayFabLobby>();
}

Ref<PlayFabMatchTicket> PlayFabMultiplayer::_find_ticket(PFMatchmakingTicketHandle p_ticket_handle) const {
    for (const Ref<PlayFabMatchTicket> &ticket : m_tickets) {
        if (ticket.is_valid() && ticket->get_native_handle() == p_ticket_handle) {
            return ticket;
        }
    }
    return Ref<PlayFabMatchTicket>();
}

void PlayFabMultiplayer::_track_lobby(const Ref<PlayFabLobby> &p_lobby) {
    if (!p_lobby.is_valid() || !p_lobby->get_native_handle()) {
        return;
    }
    if (_find_lobby(p_lobby->get_native_handle()).is_null()) {
        m_lobbies.push_back(p_lobby);
    }
}

void PlayFabMultiplayer::_untrack_lobby(const Ref<PlayFabLobby> &p_lobby) {
    if (!p_lobby.is_valid()) {
        return;
    }
    m_lobbies.erase(std::remove(m_lobbies.begin(), m_lobbies.end(), p_lobby), m_lobbies.end());
}

void PlayFabMultiplayer::_track_ticket(const Ref<PlayFabMatchTicket> &p_ticket) {
    if (!p_ticket.is_valid() || !p_ticket->get_native_handle()) {
        return;
    }
    if (_find_ticket(p_ticket->get_native_handle()).is_null()) {
        m_tickets.push_back(p_ticket);
    }
}

void PlayFabMultiplayer::_complete_match_ticket_create_if_ready(const Ref<PlayFabMatchTicket> &p_ticket) {
    PendingOperation *operation = _find_pending_ticket_operation(p_ticket, PlayFabMatchTicket::CREATED);
    if (operation == nullptr || p_ticket.is_null()) {
        return;
    }

    if (!p_ticket->get_ticket_id().is_empty()) {
        Ref<PlayFabResult> result = PlayFabResult::ok_result(p_ticket);
        _complete_pending_operation(operation, result);
        _emit_ticket_change(PlayFabMatchTicket::CREATED, p_ticket, result, p_ticket->get_status(), p_ticket->get_match_id(), p_ticket->get_arranged_lobby_connection_string());
        return;
    }

    if (p_ticket->is_complete()) {
        Ref<PlayFabResult> result = PlayFabResult::error_result(E_FAIL, "match_ticket_create_failed", "The PlayFab matchmaking ticket completed before a ticket_id was assigned.", p_ticket->get_properties());
        _complete_pending_operation(operation, result);
    }
}

void PlayFabMultiplayer::_terminate_multiplayer_queue() {
    if (m_multiplayer_queue == nullptr) {
        return;
    }

    // Heap-allocate the terminate context so we can safely give up the wait
    // without risking a use-after-free if the SDK callback fires later. The
    // context is freed by us if termination completes within the deadline,
    // otherwise it is intentionally leaked to let the eventual callback land
    // on valid memory.
    auto *ctx = new MultiplayerQueueTerminateContext();
    const HRESULT terminate_hr = XTaskQueueTerminate(m_multiplayer_queue, false, ctx, multiplayer_queue_terminated);
    if (SUCCEEDED(terminate_hr)) {
        constexpr int kMaxDispatchIterations = 500; // ~5 seconds at 10ms per dispatch
        int iterations = 0;
        while (!ctx->terminated.load(std::memory_order_acquire) && iterations < kMaxDispatchIterations) {
            XTaskQueueDispatch(m_multiplayer_queue, XTaskQueuePort::Completion, 10);
            ++iterations;
        }
        if (ctx->terminated.load(std::memory_order_acquire)) {
            delete ctx;
        } else {
            WARN_PRINT("PlayFabMultiplayer: XTaskQueueTerminate did not complete within 5s; leaking terminate context to avoid UAF if the SDK callback fires later.");
            // ctx intentionally leaked.
        }
    } else {
        WARN_PRINT(vformat("PlayFabMultiplayer: XTaskQueueTerminate failed during shutdown/reset: %s", PlayFabResult::format_hresult(terminate_hr)));
        delete ctx;
    }
    XTaskQueueCloseHandle(m_multiplayer_queue);
    m_multiplayer_queue = nullptr;
}

void PlayFabMultiplayer::_reset_after_state_change_finish_failure(const Ref<PlayFabResult> &p_result) {
    Ref<PlayFabResult> result = p_result;
    if (result.is_null()) {
        result = PlayFabResult::error_result(E_FAIL, "state_changes_finish_failed", "PlayFab Multiplayer failed to finish state change processing.");
    }

    m_initialized = false;
    ++m_dispatch_generation;

    if (m_handle != nullptr) {
        PFMultiplayerUninitialize(m_handle);
        m_handle = nullptr;
    }
    _terminate_multiplayer_queue();

    for (const Ref<PlayFabLobby> &lobby : m_lobbies) {
        if (lobby.is_valid()) {
            lobby->mark_disconnected();
        }
    }
    m_lobbies.clear();

    for (const Ref<PlayFabMatchTicket> &ticket : m_tickets) {
        if (ticket.is_valid()) {
            ticket->mark_destroyed();
        }
    }
    m_tickets.clear();

    std::vector<PendingOperation *> pending_operations;
    pending_operations.swap(m_pending_operations);

    ERR_PRINT(vformat(
            "PlayFabMultiplayer: FinishStateChanges failed with %s; Multiplayer was reset. Call PlayFab.multiplayer.initialize_async() before using it again.",
            PlayFabResult::format_hresult(static_cast<HRESULT>(result->get_hresult()))));
    emit_signal("multiplayer_error", result);

    for (PendingOperation *operation : pending_operations) {
        if (operation != nullptr && operation->pending_signal.is_valid()) {
            operation->pending_signal->complete(result);
        }
        delete operation;
    }
}

Signal PlayFabMultiplayer::initialize_async(const Ref<PlayFabMultiplayerConfig> &p_config) {
    (void)p_config;
    if (m_shutting_down) {
        return _make_error_signal(E_ABORT, "shutting_down", "PlayFab Multiplayer cannot initialize while shutdown is in progress.");
    }
    PlayFabRuntime *runtime = _get_runtime();
    if (runtime == nullptr || !runtime->is_initialized()) {
        return _make_error_signal(E_FAIL, "not_initialized", "PlayFab must be initialized before PlayFab Multiplayer.");
    }
    if (m_initialized) {
        return _make_error_signal(E_FAIL, "already_initialized", "PlayFab Multiplayer is already initialized.");
    }

    Ref<PlayFabPendingSignal> pending_signal = _make_pending_signal();

    HRESULT hr = XTaskQueueCreate(XTaskQueueDispatchMode::ThreadPool, XTaskQueueDispatchMode::Manual, &m_multiplayer_queue);
    if (FAILED(hr)) {
        Ref<PlayFabResult> result = PlayFabResult::hresult_error(hr, "Failed to create the PlayFab Multiplayer task queue.", "multiplayer_queue_create_failed");
        pending_signal->complete_deferred(result);
        return pending_signal->get_completed_signal();
    }

    const CharString title_id_utf8 = runtime->get_title_id().utf8();
    MultiplayerInitializationConfiguration init_config = {};
    init_config.titleId = title_id_utf8.get_data();
    init_config.multiplayerTaskQueue = m_multiplayer_queue;

    hr = PFMultiplayerInitialize(&init_config, &m_handle);
    if (FAILED(hr)) {
        XTaskQueueCloseHandle(m_multiplayer_queue);
        m_multiplayer_queue = nullptr;
        Ref<PlayFabResult> result = multiplayer_hresult_error(hr, "Failed to initialize PlayFab Multiplayer.", "multiplayer_initialize_failed");
        pending_signal->complete_deferred(result);
        return pending_signal->get_completed_signal();
    }

    m_initialized = true;
    m_shutting_down = false;
    ++m_dispatch_generation;
    pending_signal->complete_deferred(PlayFabResult::ok_result());
    return pending_signal->get_completed_signal();
}

Signal PlayFabMultiplayer::shutdown_async() {
    Ref<PlayFabPendingSignal> pending_signal = _make_pending_signal();
    shutdown();
    if (m_shutdown_deferred_until_dispatch_complete) {
        m_shutdown_pending_signals.push_back(pending_signal);
    } else {
        pending_signal->complete_deferred(PlayFabResult::ok_result());
    }
    return pending_signal->get_completed_signal();
}

void PlayFabMultiplayer::shutdown() {
    if (m_processing_state_changes) {
        m_shutdown_deferred_until_dispatch_complete = true;
        if (!m_shutting_down) {
            m_shutting_down = true;
            _cancel_active_pending_operations("PlayFab Multiplayer is shutting down.");
        }
        return;
    }
    if (m_shutting_down && !m_shutdown_deferred_until_dispatch_complete) {
        return;
    }
    if (!m_initialized && m_handle == nullptr && m_pending_operations.empty() && m_pending_operations_deferred_delete.empty() && m_lobbies.empty() && m_tickets.empty()) {
        return;
    }

    m_shutdown_deferred_until_dispatch_complete = false;
    m_shutting_down = true;

    _cancel_active_pending_operations("PlayFab Multiplayer is shutting down.");

    std::vector<Ref<PlayFabMatchTicket>> tickets_to_destroy = m_tickets;
    for (const Ref<PlayFabMatchTicket> &ticket : tickets_to_destroy) {
        if (ticket.is_valid() && ticket->get_native_handle() != nullptr && m_handle != nullptr) {
            PFMultiplayerDestroyMatchmakingTicket(m_handle, ticket->get_native_handle());
            ticket->mark_destroyed();
        }
    }

    std::vector<PFLobbyHandle> leave_requested;
    auto request_lobby_leaves = [&]() {
        std::vector<Ref<PlayFabLobby>> lobbies_snapshot = m_lobbies;
        for (const Ref<PlayFabLobby> &lobby : lobbies_snapshot) {
            if (!lobby.is_valid() || lobby->is_disconnected() || lobby->get_native_handle() == nullptr) {
                continue;
            }
            PFLobbyHandle lobby_handle = lobby->get_native_handle();
            if (std::find(leave_requested.begin(), leave_requested.end(), lobby_handle) != leave_requested.end()) {
                continue;
            }
            PFEntityHandle local_user_handle = lobby->get_local_user().is_valid() ? lobby->get_local_user()->get_entity_handle() : nullptr;
            HRESULT leave_hr = PFLobbyLeaveWithEntityHandle(lobby_handle, local_user_handle, nullptr);
            if (SUCCEEDED(leave_hr)) {
                leave_requested.push_back(lobby_handle);
            } else {
                lobby->mark_disconnected();
            }
        }
    };

    for (int attempt = 0; attempt < 200; ++attempt) {
        request_lobby_leaves();
        bool all_lobbies_disconnected = true;
        std::vector<Ref<PlayFabLobby>> lobbies_snapshot = m_lobbies;
        for (const Ref<PlayFabLobby> &lobby : lobbies_snapshot) {
            if (lobby.is_valid() && !lobby->is_disconnected() && lobby->get_native_handle() != nullptr) {
                all_lobbies_disconnected = false;
                break;
            }
        }
        if (all_lobbies_disconnected && m_pending_operations.empty()) {
            break;
        }
        dispatch();
        _cancel_active_pending_operations("PlayFab Multiplayer is shutting down.");
        Sleep(10);
    }

    _cancel_active_pending_operations("PlayFab Multiplayer is shutting down.");

    if (m_handle != nullptr) {
        PFMultiplayerUninitialize(m_handle);
        m_handle = nullptr;
    }

    _terminate_multiplayer_queue();

    std::vector<Ref<PlayFabLobby>> lobbies_to_mark = m_lobbies;
    for (const Ref<PlayFabLobby> &lobby : lobbies_to_mark) {
        if (lobby.is_valid()) {
            lobby->mark_disconnected();
        }
    }
    m_lobbies.clear();
    m_tickets.clear();

    _delete_deferred_pending_operations();

    m_initialized = false;
    m_processing_state_changes = false;
    m_shutting_down = false;
    ++m_dispatch_generation;
    _complete_shutdown_pending_signals();
    if (m_owner != nullptr) {
        m_owner->finish_deferred_shutdown_if_ready();
    }
}

int PlayFabMultiplayer::dispatch() {
    if (!m_initialized || m_handle == nullptr || m_processing_state_changes) {
        return 0;
    }

    int dispatched = 0;
    if (m_multiplayer_queue != nullptr) {
        while (XTaskQueueDispatch(m_multiplayer_queue, XTaskQueuePort::Completion, 0)) {
            ++dispatched;
        }
    }

    const uint64_t dispatch_generation = m_dispatch_generation;
    m_processing_state_changes = true;
    dispatched += _dispatch_lobby_state_changes();
    if (!m_shutdown_deferred_until_dispatch_complete && m_dispatch_generation == dispatch_generation && m_initialized && m_handle != nullptr) {
        dispatched += _dispatch_matchmaking_state_changes();
    }
    m_processing_state_changes = false;
    if (m_shutdown_deferred_until_dispatch_complete) {
        shutdown();
    }
    return dispatched;
}

Signal PlayFabMultiplayer::create_lobby_async(const Ref<PlayFabUser> &p_user, const Ref<PlayFabLobbyConfig> &p_config) {
    if (m_shutting_down) {
        return _make_error_signal(E_ABORT, "shutting_down", "PlayFab Multiplayer operations cannot start while shutdown is in progress.");
    }
    if (!m_initialized || m_handle == nullptr) {
        return _make_error_signal(E_FAIL, "not_initialized", "PlayFab Multiplayer is not initialized. Call PlayFab.multiplayer.initialize_async() first.");
    }

    String error_message;
    if (!validate_user_entity_handle(p_user, &error_message)) {
        return _make_error_signal(E_INVALIDARG, "invalid_user", error_message);
    }

    Ref<PlayFabLobbyConfig> config = p_config;
    if (config.is_null()) {
        config.instantiate();
    }

    StringPairList search_properties;
    StringPairList lobby_properties;
    StringPairList member_properties;
    if (!search_properties.assign(config->get_search_properties(), &error_message) ||
            !lobby_properties.assign(config->get_lobby_properties(), &error_message) ||
            !member_properties.assign(config->get_member_properties(), &error_message)) {
        return _make_error_signal(E_INVALIDARG, "invalid_properties", error_message);
    }

    PFLobbyCreateConfiguration create_config = {};
    create_config.maxMemberCount = static_cast<uint32_t>(std::max<int64_t>(2, std::min<int64_t>(128, config->get_max_players())));
    create_config.accessPolicy = to_lobby_access_policy(config->get_access_policy());
    create_config.ownerMigrationPolicy = to_lobby_owner_migration_policy(config->get_owner_migration_policy());
    create_config.searchPropertyCount = search_properties.count();
    create_config.searchPropertyKeys = search_properties.keys();
    create_config.searchPropertyValues = search_properties.values();
    create_config.lobbyPropertyCount = lobby_properties.count();
    create_config.lobbyPropertyKeys = lobby_properties.keys();
    create_config.lobbyPropertyValues = lobby_properties.values();
    create_config.restrictInvitesToLobbyOwner = config->get_restrict_invites_to_lobby_owner();

    PFLobbyJoinConfiguration join_config = {};
    join_config.memberPropertyCount = member_properties.count();
    join_config.memberPropertyKeys = member_properties.keys();
    join_config.memberPropertyValues = member_properties.values();

    Ref<PlayFabPendingSignal> pending_signal = _make_pending_signal();
    PendingOperation *operation = _create_pending_operation(PlayFabLobby::PROPERTIES_UPDATED, pending_signal);

    Ref<PlayFabLobby> lobby;
    lobby.instantiate();
    lobby->set_owner(this);
    operation->lobby = lobby;
    operation->user = p_user;
    operation->local_member_property_update = config->get_member_properties();
    operation->has_local_member_property_update = true;
    operation->replace_local_member_properties = true;

    PFLobbyHandle lobby_handle = nullptr;
    HRESULT hr = PFMultiplayerCreateAndJoinLobbyWithEntityHandle(
            m_handle,
            p_user->get_entity_handle(),
            &create_config,
            &join_config,
            operation,
            &lobby_handle);
    if (FAILED(hr)) {
        _release_pending_operation(operation);
        Ref<PlayFabResult> result = multiplayer_hresult_error(hr, "Failed to start creating the PlayFab lobby.", "lobby_create_start_failed");
        pending_signal->complete_deferred(result);
        return pending_signal->get_completed_signal();
    }

    if (lobby_handle != nullptr) {
        lobby->adopt_handle(lobby_handle, p_user);
        _track_lobby(lobby);
    }

    return pending_signal->get_completed_signal();
}

Signal PlayFabMultiplayer::join_lobby_async(const Ref<PlayFabUser> &p_user, const String &p_connection_string, const Ref<PlayFabLobbyJoinConfig> &p_config) {
    if (m_shutting_down) {
        return _make_error_signal(E_ABORT, "shutting_down", "PlayFab Multiplayer operations cannot start while shutdown is in progress.");
    }
    if (!m_initialized || m_handle == nullptr) {
        return _make_error_signal(E_FAIL, "not_initialized", "PlayFab Multiplayer is not initialized. Call PlayFab.multiplayer.initialize_async() first.");
    }
    if (p_connection_string.strip_edges().is_empty()) {
        return _make_error_signal(E_INVALIDARG, "invalid_connection_string", "Joining a PlayFab lobby requires a non-empty connection string.");
    }

    String error_message;
    if (!validate_user_entity_handle(p_user, &error_message)) {
        return _make_error_signal(E_INVALIDARG, "invalid_user", error_message);
    }

    Ref<PlayFabLobbyJoinConfig> config = p_config;
    if (config.is_null()) {
        config.instantiate();
    }

    StringPairList member_properties;
    if (!member_properties.assign(config->get_member_properties(), &error_message)) {
        return _make_error_signal(E_INVALIDARG, "invalid_properties", error_message);
    }

    PFLobbyJoinConfiguration join_config = {};
    join_config.memberPropertyCount = member_properties.count();
    join_config.memberPropertyKeys = member_properties.keys();
    join_config.memberPropertyValues = member_properties.values();

    Ref<PlayFabPendingSignal> pending_signal = _make_pending_signal();
    PendingOperation *operation = _create_pending_operation(PlayFabLobby::PROPERTIES_UPDATED, pending_signal);

    Ref<PlayFabLobby> lobby;
    lobby.instantiate();
    lobby->set_owner(this);
    operation->lobby = lobby;
    operation->user = p_user;
    operation->local_member_property_update = config->get_member_properties();
    operation->has_local_member_property_update = true;
    operation->replace_local_member_properties = true;

    const CharString connection_string_utf8 = p_connection_string.strip_edges().utf8();
    PFLobbyHandle lobby_handle = nullptr;
    HRESULT hr = PFMultiplayerJoinLobbyWithEntityHandle(
            m_handle,
            p_user->get_entity_handle(),
            connection_string_utf8.get_data(),
            &join_config,
            operation,
            &lobby_handle);
    if (FAILED(hr)) {
        _release_pending_operation(operation);
        Ref<PlayFabResult> result = multiplayer_hresult_error(hr, "Failed to start joining the PlayFab lobby.", "lobby_join_start_failed");
        pending_signal->complete_deferred(result);
        return pending_signal->get_completed_signal();
    }

    if (lobby_handle != nullptr) {
        lobby->adopt_handle(lobby_handle, p_user);
        _track_lobby(lobby);
    }

    return pending_signal->get_completed_signal();
}

Signal PlayFabMultiplayer::join_arranged_lobby_async(const Ref<PlayFabUser> &p_user, const String &p_connection_string, const Ref<PlayFabLobbyJoinConfig> &p_config) {
    if (m_shutting_down) {
        return _make_error_signal(E_ABORT, "shutting_down", "PlayFab Multiplayer operations cannot start while shutdown is in progress.");
    }
    if (!m_initialized || m_handle == nullptr) {
        return _make_error_signal(E_FAIL, "not_initialized", "PlayFab Multiplayer is not initialized. Call PlayFab.multiplayer.initialize_async() first.");
    }
    if (p_connection_string.strip_edges().is_empty()) {
        return _make_error_signal(E_INVALIDARG, "invalid_arranged_lobby_connection_string", "Joining an arranged PlayFab lobby requires a non-empty arranged lobby connection string.");
    }

    String error_message;
    if (!validate_user_entity_handle(p_user, &error_message)) {
        return _make_error_signal(E_INVALIDARG, "invalid_user", error_message);
    }

    Ref<PlayFabLobbyJoinConfig> config = p_config;
    if (config.is_null()) {
        config.instantiate();
    }

    StringPairList member_properties;
    if (!member_properties.assign(config->get_member_properties(), &error_message)) {
        return _make_error_signal(E_INVALIDARG, "invalid_properties", error_message);
    }

    PFLobbyArrangedJoinConfiguration join_config = {};
    join_config.maxMemberCount = 8;
    join_config.ownerMigrationPolicy = PFLobbyOwnerMigrationPolicy::Automatic;
    join_config.accessPolicy = PFLobbyAccessPolicy::Private;
    join_config.memberPropertyCount = member_properties.count();
    join_config.memberPropertyKeys = member_properties.keys();
    join_config.memberPropertyValues = member_properties.values();
    join_config.restrictInvitesToLobbyOwner = false;

    Ref<PlayFabPendingSignal> pending_signal = _make_pending_signal();
    PendingOperation *operation = _create_pending_operation(PlayFabLobby::PROPERTIES_UPDATED, pending_signal);

    Ref<PlayFabLobby> lobby;
    lobby.instantiate();
    lobby->set_owner(this);
    operation->lobby = lobby;
    operation->user = p_user;
    operation->local_member_property_update = config->get_member_properties();
    operation->has_local_member_property_update = true;
    operation->replace_local_member_properties = true;

    const CharString connection_string_utf8 = p_connection_string.strip_edges().utf8();
    PFLobbyHandle lobby_handle = nullptr;
    HRESULT hr = PFMultiplayerJoinArrangedLobbyWithEntityHandle(
            m_handle,
            p_user->get_entity_handle(),
            connection_string_utf8.get_data(),
            &join_config,
            operation,
            &lobby_handle);
    if (FAILED(hr)) {
        _release_pending_operation(operation);
        Ref<PlayFabResult> result = multiplayer_hresult_error(hr, "Failed to start joining the arranged PlayFab lobby.", "arranged_lobby_join_start_failed");
        pending_signal->complete_deferred(result);
        return pending_signal->get_completed_signal();
    }

    if (lobby_handle != nullptr) {
        lobby->adopt_handle(lobby_handle, p_user);
        _track_lobby(lobby);
    }

    return pending_signal->get_completed_signal();
}

Signal PlayFabMultiplayer::find_lobbies_async(const Ref<PlayFabUser> &p_user, const Ref<PlayFabLobbySearchConfig> &p_search) {
    if (m_shutting_down) {
        return _make_error_signal(E_ABORT, "shutting_down", "PlayFab Multiplayer operations cannot start while shutdown is in progress.");
    }
    if (!m_initialized || m_handle == nullptr) {
        return _make_error_signal(E_FAIL, "not_initialized", "PlayFab Multiplayer is not initialized. Call PlayFab.multiplayer.initialize_async() first.");
    }

    String error_message;
    if (!validate_user_entity_handle(p_user, &error_message)) {
        return _make_error_signal(E_INVALIDARG, "invalid_user", error_message);
    }

    Ref<PlayFabLobbySearchConfig> search = p_search;
    if (search.is_null()) {
        search.instantiate();
    }

    const int64_t requested_count = search->get_max_results();
    if (requested_count < 1 || requested_count > PFLobbyClientRequestedSearchResultCountUpperLimit) {
        return _make_error_signal(E_INVALIDARG, "invalid_search", "PlayFab lobby searches require max_results from 1 to 50.");
    }

    const CharString filter_utf8 = search->get_filter().strip_edges().utf8();
    const CharString sort_utf8 = search->get_order_by().strip_edges().utf8();
    uint32_t max_results = static_cast<uint32_t>(requested_count);
    PFLobbySearchConfiguration search_config = {};
    search_config.filterString = search->get_filter().strip_edges().is_empty() ? nullptr : filter_utf8.get_data();
    search_config.sortString = search->get_order_by().strip_edges().is_empty() ? nullptr : sort_utf8.get_data();
    search_config.clientSearchResultCount = &max_results;

    Ref<PlayFabPendingSignal> pending_signal = _make_pending_signal();
    PendingOperation *operation = _create_pending_operation(0, pending_signal);

    HRESULT hr = PFMultiplayerFindLobbiesWithEntityHandle(
            m_handle,
            p_user->get_entity_handle(),
            &search_config,
            operation);
    if (FAILED(hr)) {
        _release_pending_operation(operation);
        Ref<PlayFabResult> result = multiplayer_hresult_error(hr, "Failed to start finding PlayFab lobbies.", "lobby_search_start_failed");
        pending_signal->complete_deferred(result);
    }

    return pending_signal->get_completed_signal();
}

Signal PlayFabMultiplayer::_set_lobby_properties_async(const Ref<PlayFabLobby> &p_lobby, const Dictionary &p_properties) {
    if (m_shutting_down) {
        return _make_error_signal(E_ABORT, "shutting_down", "PlayFab Multiplayer operations cannot start while shutdown is in progress.");
    }
    if (!m_initialized || m_handle == nullptr) {
        return _make_error_signal(E_FAIL, "not_initialized", "PlayFab Multiplayer is not initialized. Call PlayFab.multiplayer.initialize_async() first.");
    }
    if (!p_lobby.is_valid() || p_lobby->get_native_handle() == nullptr || !p_lobby->get_local_user().is_valid()) {
        return _make_error_signal(E_INVALIDARG, "invalid_lobby", "PlayFabLobby.set_properties_async requires a tracked PlayFabLobby with a local user.");
    }

    String error_message;
    StringPairList lobby_properties;
    if (!lobby_properties.assign(p_properties, &error_message)) {
        return _make_error_signal(E_INVALIDARG, "invalid_properties", error_message);
    }

    PFLobbyDataUpdate lobby_update = {};
    lobby_update.lobbyPropertyCount = lobby_properties.count();
    lobby_update.lobbyPropertyKeys = lobby_properties.keys();
    lobby_update.lobbyPropertyValues = lobby_properties.values();

    Ref<PlayFabPendingSignal> pending_signal = _make_pending_signal();
    PendingOperation *operation = _create_pending_operation(PlayFabLobby::PROPERTIES_UPDATED, pending_signal);
    operation->lobby = p_lobby;

    HRESULT hr = PFLobbyPostUpdateWithEntityHandle(
            p_lobby->get_native_handle(),
            p_lobby->get_local_user()->get_entity_handle(),
            &lobby_update,
            nullptr,
            operation);
    if (FAILED(hr)) {
        _release_pending_operation(operation);
        Ref<PlayFabResult> result = multiplayer_hresult_error(hr, "Failed to start updating PlayFab lobby properties.", "lobby_update_start_failed");
        pending_signal->complete_deferred(result);
    }

    return pending_signal->get_completed_signal();
}

Signal PlayFabMultiplayer::_set_member_properties_async(const Ref<PlayFabLobby> &p_lobby, const Dictionary &p_properties) {
    if (m_shutting_down) {
        return _make_error_signal(E_ABORT, "shutting_down", "PlayFab Multiplayer operations cannot start while shutdown is in progress.");
    }
    if (!m_initialized || m_handle == nullptr) {
        return _make_error_signal(E_FAIL, "not_initialized", "PlayFab Multiplayer is not initialized. Call PlayFab.multiplayer.initialize_async() first.");
    }
    if (!p_lobby.is_valid() || p_lobby->get_native_handle() == nullptr || !p_lobby->get_local_user().is_valid()) {
        return _make_error_signal(E_INVALIDARG, "invalid_lobby", "PlayFabLobby.set_member_properties_async requires a tracked PlayFabLobby with a local user.");
    }

    String error_message;
    StringPairList member_properties;
    if (!member_properties.assign(p_properties, &error_message)) {
        return _make_error_signal(E_INVALIDARG, "invalid_properties", error_message);
    }

    PFLobbyMemberDataUpdate member_update = {};
    member_update.memberPropertyCount = member_properties.count();
    member_update.memberPropertyKeys = member_properties.keys();
    member_update.memberPropertyValues = member_properties.values();

    Ref<PlayFabPendingSignal> pending_signal = _make_pending_signal();
    PendingOperation *operation = _create_pending_operation(PlayFabLobby::MEMBER_UPDATED, pending_signal);
    operation->lobby = p_lobby;
    operation->local_member_property_update = p_properties;
    operation->has_local_member_property_update = true;

    HRESULT hr = PFLobbyPostUpdateWithEntityHandle(
            p_lobby->get_native_handle(),
            p_lobby->get_local_user()->get_entity_handle(),
            nullptr,
            &member_update,
            operation);
    if (FAILED(hr)) {
        _release_pending_operation(operation);
        Ref<PlayFabResult> result = multiplayer_hresult_error(hr, "Failed to start updating PlayFab lobby member properties.", "member_update_start_failed");
        pending_signal->complete_deferred(result);
    }

    return pending_signal->get_completed_signal();
}

Signal PlayFabMultiplayer::_leave_lobby_async(const Ref<PlayFabLobby> &p_lobby) {
    if (m_shutting_down) {
        return _make_error_signal(E_ABORT, "shutting_down", "PlayFab Multiplayer operations cannot start while shutdown is in progress.");
    }
    if (!m_initialized || m_handle == nullptr) {
        return _make_error_signal(E_FAIL, "not_initialized", "PlayFab Multiplayer is not initialized. Call PlayFab.multiplayer.initialize_async() first.");
    }
    if (!p_lobby.is_valid() || p_lobby->get_native_handle() == nullptr) {
        return _make_error_signal(E_INVALIDARG, "invalid_lobby", "PlayFabLobby.leave_async requires a tracked PlayFabLobby.");
    }

    Ref<PlayFabPendingSignal> pending_signal = _make_pending_signal();
    PendingOperation *operation = _create_pending_operation(PlayFabLobby::MEMBER_REMOVED, pending_signal);
    operation->lobby = p_lobby;

    PFEntityHandle local_user_handle = p_lobby->get_local_user().is_valid() ? p_lobby->get_local_user()->get_entity_handle() : nullptr;
    HRESULT hr = PFLobbyLeaveWithEntityHandle(p_lobby->get_native_handle(), local_user_handle, operation);
    if (FAILED(hr)) {
        _release_pending_operation(operation);
        Ref<PlayFabResult> result = multiplayer_hresult_error(hr, "Failed to start leaving the PlayFab lobby.", "lobby_leave_start_failed");
        pending_signal->complete_deferred(result);
    }

    return pending_signal->get_completed_signal();
}

Signal PlayFabMultiplayer::create_match_ticket_async(const Ref<PlayFabUser> &p_user, const Ref<PlayFabMatchmakingTicketConfig> &p_config) {
    if (m_shutting_down) {
        return _make_error_signal(E_ABORT, "shutting_down", "PlayFab Multiplayer operations cannot start while shutdown is in progress.");
    }
    if (!m_initialized || m_handle == nullptr) {
        return _make_error_signal(E_FAIL, "not_initialized", "PlayFab Multiplayer is not initialized. Call PlayFab.multiplayer.initialize_async() first.");
    }
    if (p_config.is_null()) {
        return _make_error_signal(E_INVALIDARG, "invalid_match_ticket_config", "create_match_ticket_async requires a PlayFabMatchmakingTicketConfig.");
    }
    if (p_config->get_queue_name().strip_edges().is_empty()) {
        return _make_error_signal(E_INVALIDARG, "invalid_match_ticket_config", "Matchmaking ticket configs require a non-empty queue_name.");
    }
    if (p_config->get_timeout_seconds() <= 0) {
        return _make_error_signal(E_INVALIDARG, "invalid_match_ticket_config", "Matchmaking ticket configs require timeout_seconds greater than zero.");
    }

    String error_message;
    if (!validate_user_entity_handle(p_user, &error_message)) {
        return _make_error_signal(E_INVALIDARG, "invalid_user", error_message);
    }

    Array members = p_config->get_members();
    bool requester_in_members = false;
    for (int64_t i = 0; i < members.size(); ++i) {
        if (members[i].get_type() != Variant::OBJECT) {
            continue;
        }

        Object *object = members[i].operator Object *();
        if (auto *matchmaking_member = Object::cast_to<PlayFabMatchmakingMember>(object)) {
            if (matchmaking_member->get_user() == p_user) {
                requester_in_members = true;
                break;
            }
        } else if (auto *playfab_user = Object::cast_to<PlayFabUser>(object)) {
            if (p_user.ptr() == playfab_user) {
                requester_in_members = true;
                break;
            }
        }
    }
    if (!requester_in_members) {
        members.push_back(p_user);
    }

    std::vector<PFEntityHandle> local_users;
    std::vector<std::string> attribute_strings;
    std::vector<const char *> attribute_ptrs;
    Array user_members;
    for (int64_t i = 0; i < members.size(); ++i) {
        Ref<PlayFabUser> member_user;
        Dictionary attributes;

        if (members[i].get_type() == Variant::OBJECT) {
            Object *object = members[i].operator Object *();
            if (auto *matchmaking_member = Object::cast_to<PlayFabMatchmakingMember>(object)) {
                member_user = matchmaking_member->get_user();
                attributes = matchmaking_member->get_attributes();
            } else if (auto *playfab_user = Object::cast_to<PlayFabUser>(object)) {
                member_user = Ref<PlayFabUser>(playfab_user);
            }
        }

        if (!validate_user_entity_handle(member_user, &error_message)) {
            return _make_error_signal(E_INVALIDARG, "invalid_match_ticket_member", error_message);
        }

        local_users.push_back(member_user->get_entity_handle());
        attribute_strings.push_back(attributes.is_empty() ? std::string() : std::string(JSON::stringify(attributes).utf8().get_data()));
        user_members.push_back(member_user);
    }
    for (const std::string &attributes : attribute_strings) {
        attribute_ptrs.push_back(attributes.c_str());
    }

    const CharString queue_name_utf8 = p_config->get_queue_name().strip_edges().utf8();
    PFMatchmakingTicketConfiguration ticket_config = {};
    ticket_config.queueName = queue_name_utf8.get_data();
    ticket_config.timeoutInSeconds = static_cast<uint32_t>(p_config->get_timeout_seconds());

    PFMatchmakingTicketHandle ticket_handle = nullptr;
    HRESULT hr = PFMultiplayerCreateMatchmakingTicketWithEntityHandles(
            m_handle,
            static_cast<uint32_t>(local_users.size()),
            local_users.data(),
            attribute_ptrs.data(),
            &ticket_config,
            nullptr,
            &ticket_handle);
    if (FAILED(hr)) {
        Ref<PlayFabResult> result = multiplayer_hresult_error(hr, "Failed to start creating the PlayFab matchmaking ticket.", "match_ticket_create_start_failed");
        Ref<PlayFabPendingSignal> pending_signal = _make_pending_signal();
        pending_signal->complete_deferred(result);
        return pending_signal->get_completed_signal();
    }

    Ref<PlayFabMatchTicket> ticket;
    ticket.instantiate();
    ticket->set_owner(this);
    ticket->adopt_handle(ticket_handle, p_config->get_queue_name().strip_edges(), user_members);
    ticket->refresh_snapshot();
    _track_ticket(ticket);

    Ref<PlayFabPendingSignal> pending_signal = _make_pending_signal();
    PendingOperation *operation = _create_pending_operation(PlayFabMatchTicket::CREATED, pending_signal);
    operation->ticket = ticket;
    if (!ticket->get_ticket_id().is_empty()) {
        _release_pending_operation(operation);
        Ref<PlayFabResult> result = PlayFabResult::ok_result(ticket);
        pending_signal->complete_deferred(result);
        _emit_ticket_change(PlayFabMatchTicket::CREATED, ticket, result, ticket->get_status(), ticket->get_match_id(), ticket->get_arranged_lobby_connection_string());
    }
    return pending_signal->get_completed_signal();
}

Signal PlayFabMultiplayer::_cancel_match_ticket_async(const Ref<PlayFabMatchTicket> &p_ticket) {
    if (m_shutting_down) {
        return _make_error_signal(E_ABORT, "shutting_down", "PlayFab Multiplayer operations cannot start while shutdown is in progress.");
    }
    if (!m_initialized || m_handle == nullptr) {
        return _make_error_signal(E_FAIL, "not_initialized", "PlayFab Multiplayer is not initialized. Call PlayFab.multiplayer.initialize_async() first.");
    }
    if (!p_ticket.is_valid() || p_ticket->get_native_handle() == nullptr) {
        return _make_error_signal(E_INVALIDARG, "invalid_match_ticket", "PlayFabMatchTicket.cancel_async requires a tracked PlayFabMatchTicket.");
    }

    Ref<PlayFabPendingSignal> pending_signal = _make_pending_signal();
    HRESULT hr = PFMatchmakingTicketCancel(p_ticket->get_native_handle());
    if (FAILED(hr)) {
        Ref<PlayFabResult> result = multiplayer_hresult_error(hr, "Failed to start cancelling the PlayFab matchmaking ticket.", "match_ticket_cancel_start_failed");
        pending_signal->complete_deferred(result);
    } else {
        PendingOperation *operation = _create_pending_operation(PlayFabMatchTicket::CANCELLED, pending_signal);
        operation->ticket = p_ticket;
    }
    return pending_signal->get_completed_signal();
}

Signal PlayFabMultiplayer::_refresh_match_ticket_async(const Ref<PlayFabMatchTicket> &p_ticket) {
    if (m_shutting_down) {
        return _make_error_signal(E_ABORT, "shutting_down", "PlayFab Multiplayer operations cannot start while shutdown is in progress.");
    }
    if (!m_initialized || m_handle == nullptr) {
        return _make_error_signal(E_FAIL, "not_initialized", "PlayFab Multiplayer is not initialized. Call PlayFab.multiplayer.initialize_async() first.");
    }
    if (!p_ticket.is_valid() || p_ticket->get_native_handle() == nullptr) {
        return _make_error_signal(E_INVALIDARG, "invalid_match_ticket", "PlayFabMatchTicket.refresh_async requires a tracked PlayFabMatchTicket.");
    }

    Ref<PlayFabPendingSignal> pending_signal = _make_pending_signal();
    HRESULT hr = p_ticket->refresh_snapshot();
    if (FAILED(hr)) {
        Ref<PlayFabResult> result = multiplayer_hresult_error(hr, "Failed to refresh the PlayFab matchmaking ticket.", "match_ticket_refresh_failed");
        pending_signal->complete_deferred(result);
    } else {
        pending_signal->complete_deferred(PlayFabResult::ok_result(p_ticket));
    }
    return pending_signal->get_completed_signal();
}

Array PlayFabMultiplayer::get_lobbies() const {
    Array lobbies;
    for (const Ref<PlayFabLobby> &lobby : m_lobbies) {
        if (lobby.is_valid() && !lobby->is_disconnected()) {
            lobbies.push_back(lobby);
        }
    }
    return lobbies;
}

Ref<PlayFabLobby> PlayFabMultiplayer::get_lobby(const String &p_lobby_id) const {
    for (const Ref<PlayFabLobby> &lobby : m_lobbies) {
        if (lobby.is_valid() && lobby->get_lobby_id() == p_lobby_id) {
            return lobby;
        }
    }
    return Ref<PlayFabLobby>();
}

Array PlayFabMultiplayer::get_match_tickets() const {
    Array tickets;
    for (const Ref<PlayFabMatchTicket> &ticket : m_tickets) {
        if (ticket.is_valid() && !ticket->is_destroyed() && !ticket->get_ticket_id().is_empty()) {
            tickets.push_back(ticket);
        }
    }
    return tickets;
}

#ifdef GODOT_PLAYFAB_TEST_HOOKS
Signal PlayFabMultiplayer::_test_enqueue_shutdown_pending() {
    Ref<PlayFabPendingSignal> pending_signal = _make_pending_signal();
    _create_pending_operation(0, pending_signal);
    return pending_signal->get_completed_signal();
}

int64_t PlayFabMultiplayer::_test_pending_operation_count() const {
    return static_cast<int64_t>(m_pending_operations.size() + m_pending_operations_deferred_delete.size());
}
#endif

void PlayFabMultiplayer::_emit_lobby_change(
        int64_t p_kind,
        const Ref<PlayFabLobby> &p_lobby,
        const Ref<PlayFabResult> &p_result,
        const Ref<PlayFabLobbyMember> &p_member,
        const Dictionary &p_properties) {
    // Defense in depth: member-scoped kinds with a successful result must
    // carry a non-null member, or the listener will null-deref on
    // `change.member.<anything>`. Surface this loud-fail in dev rather than
    // silently emitting a malformed payload — the duplicate-emit bug in
    // LeaveLobbyCompleted and the use-after-free bugs in JoinLobbyCompleted /
    // PostUpdateCompleted both manifested as null-member emits that the
    // sample crashed on. A failed result legitimately has a null member
    // (e.g. JoinLobbyCompleted with FAILED result was never a member event).
    if (p_member.is_null() && p_result.is_valid() && p_result->is_ok() &&
            (p_kind == PlayFabLobby::MEMBER_ADDED || p_kind == PlayFabLobby::MEMBER_UPDATED || p_kind == PlayFabLobby::MEMBER_REMOVED)) {
        WARN_PRINT(vformat("PlayFabMultiplayer: emitting member-scoped lobby state change (kind=%d) with null member; listeners expecting change.member will fail. This is a regression in the dispatcher.", p_kind));
    }

    Ref<PlayFabLobbyStateChange> lobby_change;
    lobby_change.instantiate();
    Ref<RefCounted> lobby_ref = p_lobby;
    lobby_change->set_values(p_kind, lobby_ref, p_result);
    if (p_member.is_valid()) {
        lobby_change->set_member(p_member);
    }
    if (!p_properties.is_empty()) {
        lobby_change->set_properties(p_properties);
    }
    if (p_lobby.is_valid()) {
        p_lobby->emit_signal("state_changed", lobby_change);
    }

    Ref<PlayFabMultiplayerStateChange> service_change;
    service_change.instantiate();
    service_change->set_values(p_kind, lobby_ref, Ref<RefCounted>(), p_result);
    if (!p_properties.is_empty()) {
        service_change->set_properties(p_properties);
    }
    emit_signal("state_changed", service_change);
}

void PlayFabMultiplayer::_emit_ticket_change(
        int64_t p_kind,
        const Ref<PlayFabMatchTicket> &p_ticket,
        const Ref<PlayFabResult> &p_result,
        int64_t p_status,
        const String &p_match_id,
        const String &p_arranged_lobby_connection_string) {
    Ref<PlayFabMatchTicketStateChange> ticket_change;
    ticket_change.instantiate();
    Ref<RefCounted> ticket_ref = p_ticket;
    ticket_change->set_values(p_kind, ticket_ref, p_result, p_status, p_match_id, p_arranged_lobby_connection_string);
    if (p_ticket.is_valid()) {
        p_ticket->emit_signal("state_changed", ticket_change);
    }

    Ref<PlayFabMultiplayerStateChange> service_change;
    service_change.instantiate();
    service_change->set_values(p_kind, Ref<RefCounted>(), ticket_ref, p_result);
    emit_signal("state_changed", service_change);
}

int PlayFabMultiplayer::_dispatch_lobby_state_changes() {
    uint32_t state_change_count = 0;
    const PFLobbyStateChange *const *state_changes = nullptr;
    HRESULT hr = PFMultiplayerStartProcessingLobbyStateChanges(m_handle, &state_change_count, &state_changes);
    if (FAILED(hr)) {
        Ref<PlayFabResult> result = multiplayer_hresult_error(hr, "Failed to process PlayFab lobby state changes.", "lobby_state_processing_failed");
        if (_get_runtime() != nullptr) {
        }
        emit_signal("multiplayer_error", result);
        return 0;
    }

    for (uint32_t i = 0; i < state_change_count; ++i) {
        const PFLobbyStateChange *state_change = state_changes[i];
        if (state_change == nullptr) {
            continue;
        }

        switch (state_change->stateChangeType) {
            case PFLobbyStateChangeType::CreateAndJoinLobbyCompleted: {
                const auto *change = static_cast<const PFLobbyCreateAndJoinLobbyCompletedStateChange *>(state_change);
                const bool succeeded = SUCCEEDED(change->result);
                PendingOperation *operation = static_cast<PendingOperation *>(change->asyncContext);
                Ref<PlayFabLobby> lobby = operation != nullptr ? operation->lobby : _find_lobby(change->lobby);
                if (lobby.is_valid() && lobby->get_native_handle() == nullptr && change->lobby != nullptr) {
                    lobby->adopt_handle(change->lobby, operation != nullptr ? operation->user : Ref<PlayFabUser>());
                    _track_lobby(lobby);
                }
                if (lobby.is_valid() && succeeded) {
                    lobby->refresh_snapshot();
                    if (operation != nullptr && operation->has_local_member_property_update && operation->replace_local_member_properties) {
                        lobby->replace_local_member_properties(operation->local_member_property_update);
                    }
                }
                Ref<PlayFabResult> result = succeeded ? PlayFabResult::ok_result(lobby) : multiplayer_hresult_error(change->result, "PlayFab lobby creation failed.", "lobby_create_failed");
                if (!succeeded && lobby.is_valid()) {
                    _untrack_lobby(lobby);
                    lobby->mark_disconnected();
                }
                _complete_pending_operation(operation, result);
                _emit_lobby_change(PlayFabLobby::PROPERTIES_UPDATED, lobby, result);
            } break;
            case PFLobbyStateChangeType::JoinLobbyCompleted: {
                const auto *change = static_cast<const PFLobbyJoinLobbyCompletedStateChange *>(state_change);
                const bool succeeded = SUCCEEDED(change->result);
                PendingOperation *operation = static_cast<PendingOperation *>(change->asyncContext);
                Ref<PlayFabLobby> lobby = operation != nullptr ? operation->lobby : _find_lobby(change->lobby);
                if (lobby.is_valid() && lobby->get_native_handle() == nullptr && change->lobby != nullptr) {
                    lobby->adopt_handle(change->lobby, operation != nullptr ? operation->user : Ref<PlayFabUser>());
                    _track_lobby(lobby);
                }
                if (lobby.is_valid() && succeeded) {
                    lobby->refresh_snapshot();
                    if (operation != nullptr && operation->has_local_member_property_update && operation->replace_local_member_properties) {
                        lobby->replace_local_member_properties(operation->local_member_property_update);
                    }
                }
                Ref<PlayFabResult> result = succeeded ? PlayFabResult::ok_result(lobby) : multiplayer_hresult_error(change->result, "PlayFab lobby join failed.", "lobby_join_failed");
                Ref<PlayFabLobbyMember> joined_member;
                if (succeeded && lobby.is_valid() && operation != nullptr && operation->user.is_valid()) {
                    joined_member = lobby->find_member(operation->user->get_entity_key());
                }
                if (!succeeded && lobby.is_valid()) {
                    _untrack_lobby(lobby);
                    lobby->mark_disconnected();
                }
                _complete_pending_operation(operation, result);
                _emit_lobby_change(PlayFabLobby::MEMBER_ADDED, lobby, result, joined_member);
            } break;
            case PFLobbyStateChangeType::JoinArrangedLobbyCompleted: {
                const auto *change = static_cast<const PFLobbyJoinArrangedLobbyCompletedStateChange *>(state_change);
                const bool succeeded = SUCCEEDED(change->result);
                PendingOperation *operation = static_cast<PendingOperation *>(change->asyncContext);
                Ref<PlayFabLobby> lobby = operation != nullptr ? operation->lobby : _find_lobby(change->lobby);
                if (lobby.is_valid() && lobby->get_native_handle() == nullptr && change->lobby != nullptr) {
                    lobby->adopt_handle(change->lobby, operation != nullptr ? operation->user : Ref<PlayFabUser>());
                    _track_lobby(lobby);
                }
                if (lobby.is_valid() && succeeded) {
                    lobby->refresh_snapshot();
                    if (operation != nullptr && operation->has_local_member_property_update && operation->replace_local_member_properties) {
                        lobby->replace_local_member_properties(operation->local_member_property_update);
                    }
                }
                Ref<PlayFabResult> result = succeeded ? PlayFabResult::ok_result(lobby) : multiplayer_hresult_error(change->result, "PlayFab arranged lobby join failed.", "arranged_lobby_join_failed");
                Ref<PlayFabLobbyMember> joined_member;
                if (succeeded && lobby.is_valid() && operation != nullptr && operation->user.is_valid()) {
                    joined_member = lobby->find_member(operation->user->get_entity_key());
                }
                if (!succeeded && lobby.is_valid()) {
                    _untrack_lobby(lobby);
                    lobby->mark_disconnected();
                }
                _complete_pending_operation(operation, result);
                _emit_lobby_change(PlayFabLobby::MEMBER_ADDED, lobby, result, joined_member);
            } break;
            case PFLobbyStateChangeType::FindLobbiesCompleted: {
                const auto *change = static_cast<const PFLobbyFindLobbiesCompletedStateChange *>(state_change);
                PendingOperation *operation = static_cast<PendingOperation *>(change->asyncContext);
                Array summaries;
                if (SUCCEEDED(change->result)) {
                    for (uint32_t result_index = 0; result_index < change->searchResultCount; ++result_index) {
                        const PFLobbySearchResult &search_result = change->searchResults[result_index];
                        Dictionary search_properties;
                        for (uint32_t property_index = 0; property_index < search_result.searchPropertyCount; ++property_index) {
                            if (search_result.searchPropertyKeys != nullptr && search_result.searchPropertyValues != nullptr &&
                                    search_result.searchPropertyKeys[property_index] != nullptr && search_result.searchPropertyValues[property_index] != nullptr) {
                                search_properties[String::utf8(search_result.searchPropertyKeys[property_index])] = String::utf8(search_result.searchPropertyValues[property_index]);
                            }
                        }
                        Ref<PlayFabLobbySummary> summary;
                        summary.instantiate();
                        summary->set_snapshot(
                                pf_string(search_result.lobbyId),
                                pf_string(search_result.connectionString),
                                entity_key_to_dictionary(search_result.ownerEntity),
                                static_cast<int64_t>(search_result.maxMemberCount),
                                static_cast<int64_t>(search_result.currentMemberCount),
                                search_properties,
                                Dictionary());
                        summaries.push_back(summary);
                    }
                }
                Ref<PlayFabLobbySearchResult> search_result;
                search_result.instantiate();
                search_result->set_lobbies(summaries);
                Ref<PlayFabResult> result = SUCCEEDED(change->result) ? PlayFabResult::ok_result(search_result) : multiplayer_hresult_error(change->result, "PlayFab lobby search failed.", "lobby_search_failed");
                _complete_pending_operation(operation, result);
            } break;
            case PFLobbyStateChangeType::PostUpdateCompleted: {
                const auto *change = static_cast<const PFLobbyPostUpdateCompletedStateChange *>(state_change);
                PendingOperation *operation = static_cast<PendingOperation *>(change->asyncContext);
                Ref<PlayFabLobby> lobby = operation != nullptr ? operation->lobby : _find_lobby(change->lobby);
                const bool succeeded = SUCCEEDED(change->result);
                const int64_t completed_kind = operation != nullptr ? operation->kind : PlayFabLobby::PROPERTIES_UPDATED;
                if (lobby.is_valid()) {
                    lobby->refresh_snapshot();
                    if (succeeded && completed_kind == PlayFabLobby::MEMBER_UPDATED && operation != nullptr && operation->has_local_member_property_update) {
                        lobby->apply_local_member_property_update(operation->local_member_property_update);
                    }
                }
                Ref<PlayFabResult> result = succeeded ? PlayFabResult::ok_result(lobby) : multiplayer_hresult_error(change->result, "PlayFab lobby update failed.", "lobby_update_failed");
                // Capture kind + look up the affected member BEFORE
                // _complete_pending_operation deletes the operation. For
                // local-self member updates, apply_local_member_property_update
                // eagerly patches the snapshot because the SDK does not surface
                // a LobbyMemberPropertyChanged callback for the writer.
                Ref<PlayFabLobbyMember> updated_member;
                if (completed_kind == PlayFabLobby::MEMBER_UPDATED && lobby.is_valid() && lobby->get_local_user().is_valid()) {
                    updated_member = lobby->find_member(lobby->get_local_user()->get_entity_key());
                }
                _complete_pending_operation(operation, result);
                _emit_lobby_change(completed_kind, lobby, result, updated_member);
            } break;
            case PFLobbyStateChangeType::LeaveLobbyCompleted: {
                const auto *change = static_cast<const PFLobbyLeaveLobbyCompletedStateChange *>(state_change);
                PendingOperation *operation = static_cast<PendingOperation *>(change->asyncContext);
                Ref<PlayFabLobby> lobby = operation != nullptr ? operation->lobby : _find_lobby(change->lobby);
                Ref<PlayFabResult> result = PlayFabResult::ok_result();
                if (lobby.is_valid()) {
                    lobby->mark_disconnected();
                    m_lobbies.erase(std::remove_if(m_lobbies.begin(), m_lobbies.end(), [&lobby](const Ref<PlayFabLobby> &tracked_lobby) {
                        return tracked_lobby == lobby;
                    }), m_lobbies.end());
                }
                _complete_pending_operation(operation, result);
                // Emit DISCONNECTED here, not MEMBER_REMOVED — the SDK already
                // fired a per-member MemberRemoved for every local user that
                // left as part of this op. Re-emitting MEMBER_REMOVED would
                // duplicate that signal and (after the MemberRemoved handler's
                // refresh_snapshot dropped the leaving user from m_members)
                // would also carry change.member = null, since the user is no
                // longer in the cached members list.
                _emit_lobby_change(PlayFabLobby::DISCONNECTED, lobby, result);
            } break;
            case PFLobbyStateChangeType::MemberAdded: {
                const auto *change = static_cast<const PFLobbyMemberAddedStateChange *>(state_change);
                Ref<PlayFabLobby> lobby = _find_lobby(change->lobby);
                if (lobby.is_valid()) {
                    lobby->refresh_snapshot();
                    Ref<PlayFabLobbyMember> added_member = lobby->find_member(entity_key_to_dictionary(&change->member));
                    _emit_lobby_change(PlayFabLobby::MEMBER_ADDED, lobby, PlayFabResult::ok_result(), added_member);
                }
            } break;
            case PFLobbyStateChangeType::MemberRemoved: {
                const auto *change = static_cast<const PFLobbyMemberRemovedStateChange *>(state_change);
                Ref<PlayFabLobby> lobby = _find_lobby(change->lobby);
                if (lobby.is_valid()) {
                    // Resolve the removed member from the pre-refresh snapshot —
                    // after refresh_snapshot() the SDK has already dropped them
                    // from the members list and their properties are emptied.
                    Ref<PlayFabLobbyMember> removed_member = lobby->find_member(entity_key_to_dictionary(&change->member));
                    lobby->refresh_snapshot();
                    _emit_lobby_change(PlayFabLobby::MEMBER_REMOVED, lobby, PlayFabResult::ok_result(), removed_member);
                }
            } break;
            case PFLobbyStateChangeType::Updated: {
                const auto *change = static_cast<const PFLobbyUpdatedStateChange *>(state_change);
                Ref<PlayFabLobby> lobby = _find_lobby(change->lobby);
                if (lobby.is_valid()) {
                    lobby->refresh_snapshot();
                    if (change->ownerUpdated) {
                        _emit_lobby_change(PlayFabLobby::OWNER_CHANGED, lobby, PlayFabResult::ok_result());
                    } else if (change->memberUpdateCount > 0) {
                        // Fire one MEMBER_UPDATED per affected member so a
                        // listener can attribute every update; member-update
                        // bundling is rare but legal in the PFLobby SDK.
                        for (uint32_t mi = 0; mi < change->memberUpdateCount; ++mi) {
                            Ref<PlayFabLobbyMember> updated_member = lobby->find_member(entity_key_to_dictionary(&change->memberUpdates[mi].member));
                            _emit_lobby_change(PlayFabLobby::MEMBER_UPDATED, lobby, PlayFabResult::ok_result(), updated_member);
                        }
                    } else {
                        // Build the changed-properties dictionary from the
                        // updated keys. A missing value (PFLobbyGetLobbyProperty
                        // returning null) maps to Variant() and signals the key
                        // was cleared.
                        Dictionary updated_lobby_props;
                        PFLobbyHandle handle = lobby->get_native_handle();
                        if (handle != nullptr) {
                            for (uint32_t pi = 0; pi < change->updatedLobbyPropertyCount; ++pi) {
                                const char *key = change->updatedLobbyPropertyKeys[pi];
                                if (key == nullptr) {
                                    continue;
                                }
                                const char *value = nullptr;
                                if (SUCCEEDED(PFLobbyGetLobbyProperty(handle, key, &value)) && value != nullptr) {
                                    updated_lobby_props[String::utf8(key)] = String::utf8(value);
                                } else {
                                    updated_lobby_props[String::utf8(key)] = Variant();
                                }
                            }
                        }
                        _emit_lobby_change(PlayFabLobby::PROPERTIES_UPDATED, lobby, PlayFabResult::ok_result(), Ref<PlayFabLobbyMember>(), updated_lobby_props);
                    }
                }
            } break;
            case PFLobbyStateChangeType::InviteReceived: {
                const auto *change = static_cast<const PFLobbyInviteReceivedStateChange *>(state_change);
                Ref<PlayFabLobbyInvite> invite;
                invite.instantiate();
                invite->set_snapshot(pf_string(change->lobbyId), pf_string(change->connectionString), entity_key_to_dictionary(&change->invitingEntity));
                emit_signal("invite_received", invite);
            } break;
            case PFLobbyStateChangeType::Disconnected: {
                const auto *change = static_cast<const PFLobbyDisconnectedStateChange *>(state_change);
                Ref<PlayFabLobby> lobby = _find_lobby(change->lobby);
                if (lobby.is_valid()) {
                    lobby->mark_disconnected();
                    _emit_lobby_change(PlayFabLobby::DISCONNECTED, lobby, PlayFabResult::ok_result());
                }
            } break;
            default:
                break;
        }
    }

    HRESULT finish_hr = PFMultiplayerFinishProcessingLobbyStateChanges(m_handle, state_change_count, state_changes);
    if (FAILED(finish_hr)) {
        Ref<PlayFabResult> result = multiplayer_hresult_error(finish_hr, "Failed to finish PlayFab lobby state changes.", "lobby_state_finish_failed");
        _reset_after_state_change_finish_failure(result);
    }
    return static_cast<int>(state_change_count);
}

int PlayFabMultiplayer::_dispatch_matchmaking_state_changes() {
    uint32_t state_change_count = 0;
    const PFMatchmakingStateChange *const *state_changes = nullptr;
    HRESULT hr = PFMultiplayerStartProcessingMatchmakingStateChanges(m_handle, &state_change_count, &state_changes);
    if (FAILED(hr)) {
        Ref<PlayFabResult> result = multiplayer_hresult_error(hr, "Failed to process PlayFab matchmaking state changes.", "matchmaking_state_processing_failed");
        if (_get_runtime() != nullptr) {
        }
        emit_signal("multiplayer_error", result);
        return 0;
    }

    std::vector<Ref<PlayFabMatchTicket>> terminal_tickets;
    for (uint32_t i = 0; i < state_change_count; ++i) {
        const PFMatchmakingStateChange *state_change = state_changes[i];
        if (state_change == nullptr) {
            continue;
        }

        switch (state_change->stateChangeType) {
            case PFMatchmakingStateChangeType::TicketStatusChanged: {
                const auto *change = static_cast<const PFMatchmakingTicketStatusChangedStateChange *>(state_change);
                Ref<PlayFabMatchTicket> ticket = _find_ticket(change->ticket);
                if (ticket.is_valid()) {
                    ticket->refresh_snapshot();
                    _complete_match_ticket_create_if_ready(ticket);
                    if (_find_pending_ticket_operation(ticket, PlayFabMatchTicket::CREATED) != nullptr && ticket->get_ticket_id().is_empty()) {
                        break;
                    }
                    int64_t kind = PlayFabMatchTicket::STATUS_CHANGED;
                    if (ticket->is_cancelled()) {
                        kind = PlayFabMatchTicket::CANCELLED;
                    } else if (ticket->get_status() == static_cast<int64_t>(PFMatchmakingTicketStatus::Failed)) {
                        kind = PlayFabMatchTicket::FAILED;
                    }
                    Ref<PlayFabResult> result = kind == PlayFabMatchTicket::FAILED ?
                            PlayFabResult::error_result(E_FAIL, "match_ticket_failed", "PlayFab matchmaking ticket failed.", ticket) :
                            PlayFabResult::ok_result(ticket);
                    if (kind == PlayFabMatchTicket::FAILED && _get_runtime() != nullptr) {
                    }
                    if (kind == PlayFabMatchTicket::CANCELLED || kind == PlayFabMatchTicket::FAILED) {
                        PendingOperation *cancel_operation = _find_pending_ticket_operation(ticket, PlayFabMatchTicket::CANCELLED);
                        if (cancel_operation != nullptr) {
                            _complete_pending_operation(cancel_operation, kind == PlayFabMatchTicket::CANCELLED ? PlayFabResult::ok_result() : result);
                        }
                        terminal_tickets.push_back(ticket);
                    }
                    _emit_ticket_change(kind, ticket, result, ticket->get_status(), ticket->get_match_id(), ticket->get_arranged_lobby_connection_string());
                }
            } break;
            case PFMatchmakingStateChangeType::TicketCompleted: {
                const auto *change = static_cast<const PFMatchmakingTicketCompletedStateChange *>(state_change);
                Ref<PlayFabMatchTicket> ticket = _find_ticket(change->ticket);
                if (ticket.is_valid()) {
                    ticket->refresh_snapshot();
                    _complete_match_ticket_create_if_ready(ticket);
                    Ref<PlayFabResult> result = SUCCEEDED(change->result) ? PlayFabResult::ok_result(ticket) : multiplayer_hresult_error(change->result, "PlayFab matchmaking ticket failed.", "match_ticket_completed_failed");
                    int64_t kind = PlayFabMatchTicket::COMPLETED;
                    if (ticket->is_cancelled()) {
                        kind = PlayFabMatchTicket::CANCELLED;
                    } else if (!result->is_ok() || ticket->get_status() == static_cast<int64_t>(PFMatchmakingTicketStatus::Failed)) {
                        kind = PlayFabMatchTicket::FAILED;
                    }
                    PendingOperation *cancel_operation = _find_pending_ticket_operation(ticket, PlayFabMatchTicket::CANCELLED);
                    if (cancel_operation != nullptr && (kind == PlayFabMatchTicket::CANCELLED || kind == PlayFabMatchTicket::FAILED)) {
                        _complete_pending_operation(cancel_operation, kind == PlayFabMatchTicket::CANCELLED ? PlayFabResult::ok_result() : result);
                    }
                    terminal_tickets.push_back(ticket);
                    _emit_ticket_change(kind, ticket, result, ticket->get_status(), ticket->get_match_id(), ticket->get_arranged_lobby_connection_string());
                }
            } break;
        }
    }

    HRESULT finish_hr = PFMultiplayerFinishProcessingMatchmakingStateChanges(m_handle, state_change_count, state_changes);
    if (FAILED(finish_hr)) {
        Ref<PlayFabResult> result = multiplayer_hresult_error(finish_hr, "Failed to finish PlayFab matchmaking state changes.", "matchmaking_state_finish_failed");
        _reset_after_state_change_finish_failure(result);
        return static_cast<int>(state_change_count);
    }

    for (const Ref<PlayFabMatchTicket> &ticket : terminal_tickets) {
        if (ticket.is_valid() && ticket->get_native_handle() != nullptr && m_handle != nullptr) {
            PFMultiplayerDestroyMatchmakingTicket(m_handle, ticket->get_native_handle());
            ticket->mark_destroyed();
        }
    }
    m_tickets.erase(std::remove_if(m_tickets.begin(), m_tickets.end(), [](const Ref<PlayFabMatchTicket> &t) {
        return !t.is_valid() || t->is_destroyed();
    }), m_tickets.end());
    return static_cast<int>(state_change_count);
}

} // namespace godot
