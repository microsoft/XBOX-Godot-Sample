#ifndef GODOT_PLAYFAB_MULTIPLAYER_H
#define GODOT_PLAYFAB_MULTIPLAYER_H

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>

#include <map>
#include <vector>

#include <godot_cpp/classes/ref.hpp>
#include <godot_cpp/classes/ref_counted.hpp>
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/array.hpp>
#include <godot_cpp/variant/dictionary.hpp>
#include <godot_cpp/variant/string.hpp>

#include <XTaskQueue.h>
#include <playfab/multiplayer/PFMultiplayer.h>
#include <playfab/multiplayer/PFLobby.h>
#include <playfab/multiplayer/PFMatchmaking.h>

namespace godot {

class PlayFab;
class PlayFabPendingSignal;
class PlayFabResult;
class PlayFabRuntime;
class PlayFabUser;

class PlayFabMultiplayerConfig : public RefCounted {
    GDCLASS(PlayFabMultiplayerConfig, RefCounted);

protected:
    static void _bind_methods();
};

class PlayFabLobbyConfig : public RefCounted {
    GDCLASS(PlayFabLobbyConfig, RefCounted);

    int64_t m_max_players = 8;
    int64_t m_access_policy = ACCESS_POLICY_PRIVATE;
    int64_t m_owner_migration_policy = OWNER_MIGRATION_AUTOMATIC;
    Dictionary m_search_properties;
    Dictionary m_lobby_properties;
    Dictionary m_member_properties;
    bool m_restrict_invites_to_lobby_owner = false;

protected:
    static void _bind_methods();

public:
    enum AccessPolicy : int64_t {
        ACCESS_POLICY_PUBLIC = 0,
        ACCESS_POLICY_FRIENDS = 1,
        ACCESS_POLICY_PRIVATE = 2,
    };

    enum OwnerMigrationPolicy : int64_t {
        OWNER_MIGRATION_AUTOMATIC = 0,
        OWNER_MIGRATION_MANUAL = 1,
        OWNER_MIGRATION_NONE = 2,
    };

    int64_t get_max_players() const;
    void set_max_players(int64_t p_max_players);
    int64_t get_access_policy() const;
    void set_access_policy(int64_t p_access_policy);
    int64_t get_owner_migration_policy() const;
    void set_owner_migration_policy(int64_t p_owner_migration_policy);
    Dictionary get_search_properties() const;
    void set_search_properties(const Dictionary &p_properties);
    Dictionary get_lobby_properties() const;
    void set_lobby_properties(const Dictionary &p_properties);
    Dictionary get_member_properties() const;
    void set_member_properties(const Dictionary &p_properties);
    bool get_restrict_invites_to_lobby_owner() const;
    void set_restrict_invites_to_lobby_owner(bool p_restrict);
};

class PlayFabLobbyJoinConfig : public RefCounted {
    GDCLASS(PlayFabLobbyJoinConfig, RefCounted);

    Dictionary m_member_properties;

protected:
    static void _bind_methods();

public:
    Dictionary get_member_properties() const;
    void set_member_properties(const Dictionary &p_properties);
};

class PlayFabLobbySearchConfig : public RefCounted {
    GDCLASS(PlayFabLobbySearchConfig, RefCounted);

    String m_filter;
    String m_order_by;
    int64_t m_max_results = 10;

protected:
    static void _bind_methods();

public:
    String get_filter() const;
    void set_filter(const String &p_filter);
    String get_order_by() const;
    void set_order_by(const String &p_order_by);
    int64_t get_max_results() const;
    void set_max_results(int64_t p_max_results);
};

class PlayFabMatchmakingMember : public RefCounted {
    GDCLASS(PlayFabMatchmakingMember, RefCounted);

    Ref<PlayFabUser> m_user;
    Dictionary m_attributes;

protected:
    static void _bind_methods();

public:
    Ref<PlayFabUser> get_user() const;
    void set_user(const Ref<PlayFabUser> &p_user);
    Dictionary get_attributes() const;
    void set_attributes(const Dictionary &p_attributes);
};

class PlayFabMatchmakingTicketConfig : public RefCounted {
    GDCLASS(PlayFabMatchmakingTicketConfig, RefCounted);

    String m_queue_name;
    int64_t m_timeout_seconds = 120;
    Array m_members;

protected:
    static void _bind_methods();

public:
    String get_queue_name() const;
    void set_queue_name(const String &p_queue_name);
    int64_t get_timeout_seconds() const;
    void set_timeout_seconds(int64_t p_timeout_seconds);
    Array get_members() const;
    void set_members(const Array &p_members);
};

class PlayFabLobbyMember : public RefCounted {
    GDCLASS(PlayFabLobbyMember, RefCounted);

    String m_user_id;
    Dictionary m_entity_key;
    Dictionary m_properties;
    bool m_is_local = false;

protected:
    static void _bind_methods();

public:
    void set_snapshot(const String &p_user_id, const Dictionary &p_entity_key, const Dictionary &p_properties, bool p_is_local);
    String get_user_id() const;
    Dictionary get_entity_key() const;
    Dictionary get_properties() const;
    bool is_local_member() const;
};

class PlayFabLobbyInvite : public RefCounted {
    GDCLASS(PlayFabLobbyInvite, RefCounted);

    String m_lobby_id;
    String m_connection_string;
    String m_sender_user_id;
    Dictionary m_sender_entity_key;
    String m_invite_uri;
    Dictionary m_properties;

protected:
    static void _bind_methods();

public:
    void set_snapshot(const String &p_lobby_id, const String &p_connection_string, const Dictionary &p_sender_entity_key);
    String get_lobby_id() const;
    String get_connection_string() const;
    String get_sender_user_id() const;
    Dictionary get_sender_entity_key() const;
    String get_invite_uri() const;
    Dictionary get_properties() const;
};

class PlayFabLobbySummary : public RefCounted {
    GDCLASS(PlayFabLobbySummary, RefCounted);

    String m_lobby_id;
    String m_connection_string;
    Dictionary m_owner_entity_key;
    int64_t m_max_member_count = 0;
    int64_t m_member_count = 0;
    Dictionary m_search_properties;
    Dictionary m_lobby_properties;

protected:
    static void _bind_methods();

public:
    void set_snapshot(
            const String &p_lobby_id,
            const String &p_connection_string,
            const Dictionary &p_owner_entity_key,
            int64_t p_max_member_count,
            int64_t p_member_count,
            const Dictionary &p_search_properties,
            const Dictionary &p_lobby_properties);
    String get_lobby_id() const;
    String get_connection_string() const;
    Dictionary get_owner_entity_key() const;
    int64_t get_max_member_count() const;
    int64_t get_member_count() const;
    Dictionary get_search_properties() const;
    Dictionary get_lobby_properties() const;
};

class PlayFabLobbySearchResult : public RefCounted {
    GDCLASS(PlayFabLobbySearchResult, RefCounted);

    Array m_lobbies;
    String m_continuation_token;

protected:
    static void _bind_methods();

public:
    void set_lobbies(const Array &p_lobbies);
    Array get_lobbies() const;
    String get_continuation_token() const;
};

class PlayFabLobbyStateChange : public RefCounted {
    GDCLASS(PlayFabLobbyStateChange, RefCounted);

    int64_t m_kind = 0;
    Ref<RefCounted> m_lobby;
    Ref<PlayFabResult> m_result;
    Ref<PlayFabLobbyMember> m_member;
    Ref<PlayFabLobbyInvite> m_invite;
    Ref<PlayFabUser> m_user;
    Dictionary m_properties;

protected:
    static void _bind_methods();

public:
    void set_values(int64_t p_kind, const Ref<RefCounted> &p_lobby, const Ref<PlayFabResult> &p_result = Ref<PlayFabResult>());
    void set_member(const Ref<PlayFabLobbyMember> &p_member);
    void set_invite(const Ref<PlayFabLobbyInvite> &p_invite);
    void set_user(const Ref<PlayFabUser> &p_user);
    void set_properties(const Dictionary &p_properties);
    int64_t get_kind() const;
    Ref<RefCounted> get_lobby() const;
    Ref<PlayFabResult> get_result() const;
    Ref<PlayFabLobbyMember> get_member() const;
    Ref<PlayFabLobbyInvite> get_invite() const;
    Ref<PlayFabUser> get_user() const;
    Dictionary get_properties() const;
};

class PlayFabMatchTicketStateChange : public RefCounted {
    GDCLASS(PlayFabMatchTicketStateChange, RefCounted);

    int64_t m_kind = 0;
    Ref<RefCounted> m_ticket;
    Ref<PlayFabResult> m_result;
    int64_t m_status = 0;
    String m_match_id;
    String m_arranged_lobby_connection_string;

protected:
    static void _bind_methods();

public:
    void set_values(
            int64_t p_kind,
            const Ref<RefCounted> &p_ticket,
            const Ref<PlayFabResult> &p_result,
            int64_t p_status,
            const String &p_match_id,
            const String &p_arranged_lobby_connection_string);
    int64_t get_kind() const;
    Ref<RefCounted> get_ticket() const;
    Ref<PlayFabResult> get_result() const;
    int64_t get_status() const;
    String get_match_id() const;
    String get_arranged_lobby_connection_string() const;
};

class PlayFabMultiplayerStateChange : public RefCounted {
    GDCLASS(PlayFabMultiplayerStateChange, RefCounted);

    int64_t m_kind = 0;
    Ref<RefCounted> m_lobby;
    Ref<RefCounted> m_ticket;
    Ref<PlayFabResult> m_result;
    Dictionary m_properties;

protected:
    static void _bind_methods();

public:
    void set_values(int64_t p_kind, const Ref<RefCounted> &p_lobby, const Ref<RefCounted> &p_ticket, const Ref<PlayFabResult> &p_result);
    void set_properties(const Dictionary &p_properties);
    int64_t get_kind() const;
    Ref<RefCounted> get_lobby() const;
    Ref<RefCounted> get_ticket() const;
    Ref<PlayFabResult> get_result() const;
    Dictionary get_properties() const;
};

class PlayFabMultiplayer;

class PlayFabLobby : public RefCounted {
    GDCLASS(PlayFabLobby, RefCounted);

    PlayFabMultiplayer *m_owner = nullptr;
    PFLobbyHandle m_lobby_handle = nullptr;
    Ref<PlayFabUser> m_local_user;
    String m_lobby_id;
    String m_connection_string;
    Dictionary m_owner_entity_key;
    int64_t m_max_member_count = 0;
    int64_t m_member_count = 0;
    Dictionary m_properties;
    Dictionary m_search_properties;
    Array m_members;
    bool m_disconnected = false;

protected:
    static void _bind_methods();

public:
    enum StateChangeKind : int64_t {
        MEMBER_ADDED = 1,
        MEMBER_REMOVED = 2,
        MEMBER_UPDATED = 3,
        PROPERTIES_UPDATED = 4,
        OWNER_CHANGED = 5,
        DISCONNECTED = 6,
    };

    void set_owner(PlayFabMultiplayer *p_owner);
    void adopt_handle(PFLobbyHandle p_lobby_handle, const Ref<PlayFabUser> &p_local_user);
    PFLobbyHandle get_native_handle() const;
    Ref<PlayFabUser> get_local_user() const;
    void mark_disconnected();
    bool is_disconnected() const;
    HRESULT refresh_snapshot();

    String get_lobby_id() const;
    String get_connection_string() const;
    Dictionary get_owner_entity_key() const;
    int64_t get_max_member_count() const;
    int64_t get_member_count() const;
    Array get_members() const;
    Dictionary get_properties() const;
    Dictionary get_search_properties() const;
    bool is_owner(const Ref<PlayFabUser> &p_user) const;
    Signal set_properties_async(const Dictionary &p_properties);
    Signal set_member_properties_async(const Dictionary &p_properties);
    Signal leave_async();
};

class PlayFabMatchTicket : public RefCounted {
    GDCLASS(PlayFabMatchTicket, RefCounted);

    PlayFabMultiplayer *m_owner = nullptr;
    PFMatchmakingTicketHandle m_ticket_handle = nullptr;
    String m_ticket_id;
    String m_queue_name;
    int64_t m_status = 0;
    Array m_members;
    String m_match_id;
    String m_arranged_lobby_connection_string;
    Dictionary m_properties;
    bool m_destroyed = false;

protected:
    static void _bind_methods();

public:
    enum StateChangeKind : int64_t {
        CREATED = 100,
        STATUS_CHANGED = 101,
        COMPLETED = 102,
        CANCELLED = 103,
        FAILED = 104,
    };

    void set_owner(PlayFabMultiplayer *p_owner);
    void adopt_handle(PFMatchmakingTicketHandle p_ticket_handle, const String &p_queue_name, const Array &p_members);
    PFMatchmakingTicketHandle get_native_handle() const;
    void mark_destroyed();
    bool is_destroyed() const;
    HRESULT refresh_snapshot();
    void set_match_details(const String &p_match_id, const String &p_arranged_lobby_connection_string);

    String get_ticket_id() const;
    String get_queue_name() const;
    int64_t get_status() const;
    Array get_members() const;
    String get_match_id() const;
    String get_arranged_lobby_connection_string() const;
    Dictionary get_properties() const;
    bool is_complete() const;
    bool is_cancelled() const;
    Signal refresh_async();
    Signal cancel_async();
};

class PlayFabMultiplayer : public RefCounted {
    GDCLASS(PlayFabMultiplayer, RefCounted);

private:
    friend class PlayFabLobby;
    friend class PlayFabMatchTicket;

    struct PendingOperation;

    PlayFab *m_owner = nullptr;
    PFMultiplayerHandle m_handle = nullptr;
    XTaskQueueHandle m_multiplayer_queue = nullptr;
    bool m_initialized = false;
    bool m_processing_state_changes = false;
    bool m_shutting_down = false;
    std::vector<Ref<PlayFabLobby>> m_lobbies;
    std::vector<Ref<PlayFabMatchTicket>> m_tickets;
    std::vector<PendingOperation *> m_pending_operations;

    PlayFabRuntime *_get_runtime() const;
    Ref<PlayFabPendingSignal> _make_pending_signal();
    Signal _make_error_signal(HRESULT p_hresult, const String &p_code, const String &p_message, const Variant &p_data = Variant());
    PendingOperation *_create_pending_operation(int64_t p_kind, const Ref<PlayFabPendingSignal> &p_pending_signal);
    void _complete_pending_operation(PendingOperation *p_operation, const Ref<PlayFabResult> &p_result);
    void _release_pending_operation(PendingOperation *p_operation);
    PendingOperation *_find_pending_ticket_operation(const Ref<PlayFabMatchTicket> &p_ticket, int64_t p_kind) const;
    Ref<PlayFabLobby> _find_lobby(PFLobbyHandle p_lobby_handle) const;
    Ref<PlayFabMatchTicket> _find_ticket(PFMatchmakingTicketHandle p_ticket_handle) const;
    void _track_lobby(const Ref<PlayFabLobby> &p_lobby);
    void _track_ticket(const Ref<PlayFabMatchTicket> &p_ticket);
    int _dispatch_lobby_state_changes();
    int _dispatch_matchmaking_state_changes();
    void _emit_lobby_change(int64_t p_kind, const Ref<PlayFabLobby> &p_lobby, const Ref<PlayFabResult> &p_result = Ref<PlayFabResult>());
    void _emit_ticket_change(
            int64_t p_kind,
            const Ref<PlayFabMatchTicket> &p_ticket,
            const Ref<PlayFabResult> &p_result,
            int64_t p_status,
            const String &p_match_id,
            const String &p_arranged_lobby_connection_string);
    Signal _set_lobby_properties_async(const Ref<PlayFabLobby> &p_lobby, const Dictionary &p_properties);
    Signal _set_member_properties_async(const Ref<PlayFabLobby> &p_lobby, const Dictionary &p_properties);
    Signal _leave_lobby_async(const Ref<PlayFabLobby> &p_lobby);
    Signal _refresh_match_ticket_async(const Ref<PlayFabMatchTicket> &p_ticket);
    Signal _cancel_match_ticket_async(const Ref<PlayFabMatchTicket> &p_ticket);

protected:
    static void _bind_methods();

public:
    ~PlayFabMultiplayer();

    void set_owner(PlayFab *p_owner);
    bool is_initialized() const;
    Signal initialize_async(const Ref<PlayFabMultiplayerConfig> &p_config = Ref<PlayFabMultiplayerConfig>());
    Signal shutdown_async();
    void shutdown();
    int dispatch();

    Signal create_lobby_async(const Ref<PlayFabUser> &p_user, const Ref<PlayFabLobbyConfig> &p_config = Ref<PlayFabLobbyConfig>());
    Signal join_lobby_async(const Ref<PlayFabUser> &p_user, const String &p_connection_string, const Ref<PlayFabLobbyJoinConfig> &p_config = Ref<PlayFabLobbyJoinConfig>());
    Signal join_arranged_lobby_async(const Ref<PlayFabUser> &p_user, const String &p_connection_string, const Ref<PlayFabLobbyJoinConfig> &p_config = Ref<PlayFabLobbyJoinConfig>());
    Signal find_lobbies_async(const Ref<PlayFabUser> &p_user, const Ref<PlayFabLobbySearchConfig> &p_search = Ref<PlayFabLobbySearchConfig>());

    Signal create_match_ticket_async(const Ref<PlayFabUser> &p_user, const Ref<PlayFabMatchmakingTicketConfig> &p_config);

    Array get_lobbies() const;
    Ref<PlayFabLobby> get_lobby(const String &p_lobby_id) const;
    Array get_match_tickets() const;
};

} // namespace godot

#endif // GODOT_PLAYFAB_MULTIPLAYER_H
