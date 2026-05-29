#ifndef GODOT_PLAYFAB_PARTY_H
#define GODOT_PLAYFAB_PARTY_H

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>

#include <deque>
#include <map>
#include <vector>

#include <godot_cpp/classes/multiplayer_peer.hpp>
#include <godot_cpp/classes/multiplayer_peer_extension.hpp>
#include <godot_cpp/classes/ref.hpp>
#include <godot_cpp/classes/ref_counted.hpp>
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/array.hpp>
#include <godot_cpp/variant/dictionary.hpp>
#include <godot_cpp/variant/packed_byte_array.hpp>
#include <godot_cpp/variant/packed_int32_array.hpp>
#include <godot_cpp/variant/packed_string_array.hpp>
#include <godot_cpp/variant/string.hpp>

#include <XTaskQueue.h>
#include <playfab/core/PFEntity.h>

namespace Party {
class PartyChatControl;
class PartyEndpoint;
class PartyLocalChatControl;
class PartyLocalEndpoint;
class PartyLocalUser;
class PartyNetwork;
struct PartyStateChange;
}

namespace godot {

class PlayFab;
class PlayFabParty;
class PlayFabPartyChat;
class PlayFabPartyChatControl;
class PlayFabPartyChatMessage;
class PlayFabPartyChatStateChange;
class PlayFabPartyMember;
class PlayFabPartyNetwork;
class PlayFabPartyNetworkStateChange;
class PlayFabPartyPeer;
class PlayFabPendingSignal;
class PlayFabResult;
class PlayFabRuntime;
class PlayFabUser;

class PlayFabPartyConfig : public RefCounted {
    GDCLASS(PlayFabPartyConfig, RefCounted);

    int64_t m_max_players = 8;
    int64_t m_direct_peer_connectivity = 0;
    String m_invitation_id;
    bool m_enable_voice_chat = true;
    bool m_enable_text_chat = true;
    bool m_enable_transcription = false;
    bool m_enable_translation = false;
    String m_audio_input;
    String m_audio_output;
    Dictionary m_metadata;

protected:
    static void _bind_methods();

public:
    int64_t get_max_players() const;
    void set_max_players(int64_t p_max_players);
    int64_t get_direct_peer_connectivity() const;
    void set_direct_peer_connectivity(int64_t p_direct_peer_connectivity);
    String get_invitation_id() const;
    void set_invitation_id(const String &p_invitation_id);
    bool is_voice_chat_enabled() const;
    void set_voice_chat_enabled(bool p_enabled);
    bool is_text_chat_enabled() const;
    void set_text_chat_enabled(bool p_enabled);
    bool is_transcription_enabled() const;
    void set_transcription_enabled(bool p_enabled);
    bool is_translation_enabled() const;
    void set_translation_enabled(bool p_enabled);
    String get_audio_input() const;
    void set_audio_input(const String &p_audio_input);
    String get_audio_output() const;
    void set_audio_output(const String &p_audio_output);
    Dictionary get_metadata() const;
    void set_metadata(const Dictionary &p_metadata);
};

class PlayFabPartyTextMessageConfig : public RefCounted {
    GDCLASS(PlayFabPartyTextMessageConfig, RefCounted);

    String m_language_code;
    PackedStringArray m_translate_to_languages;
    Dictionary m_metadata;

protected:
    static void _bind_methods();

public:
    String get_language_code() const;
    void set_language_code(const String &p_language_code);
    PackedStringArray get_translate_to_languages() const;
    void set_translate_to_languages(const PackedStringArray &p_languages);
    Dictionary get_metadata() const;
    void set_metadata(const Dictionary &p_metadata);
};

class PlayFabPartyMember : public RefCounted {
    GDCLASS(PlayFabPartyMember, RefCounted);

    int64_t m_peer_id = 0;
    Dictionary m_entity_key;
    Ref<PlayFabUser> m_user;
    bool m_local = false;

protected:
    static void _bind_methods();

public:
    void set_snapshot(int64_t p_peer_id, const Dictionary &p_entity_key, const Ref<PlayFabUser> &p_user, bool p_local);

    int64_t get_peer_id() const;
    Dictionary get_entity_key() const;
    Ref<PlayFabUser> get_user() const;
    bool is_local_member() const;
};

class PlayFabPartyChatMessage : public RefCounted {
    GDCLASS(PlayFabPartyChatMessage, RefCounted);

    Ref<PlayFabPartyChatControl> m_sender;
    Dictionary m_sender_entity_key;
    Array m_targets;
    String m_text;
    String m_language_code;
    String m_translated_text;
    bool m_transcription = false;
    int64_t m_timestamp = 0;
    Dictionary m_metadata;

protected:
    static void _bind_methods();

public:
    void set_values(
            const Ref<PlayFabPartyChatControl> &p_sender,
            const Dictionary &p_sender_entity_key,
            const Array &p_targets,
            const String &p_text,
            const String &p_language_code,
            const String &p_translated_text,
            bool p_transcription,
            int64_t p_timestamp,
            const Dictionary &p_metadata);

    Ref<PlayFabPartyChatControl> get_sender() const;
    Dictionary get_sender_entity_key() const;
    Array get_targets() const;
    String get_text() const;
    String get_language_code() const;
    String get_translated_text() const;
    bool is_transcription() const;
    int64_t get_timestamp() const;
    Dictionary get_metadata() const;
};

class PlayFabPartyChatStateChange : public RefCounted {
    GDCLASS(PlayFabPartyChatStateChange, RefCounted);

    int64_t m_kind = 0;
    Ref<PlayFabPartyChatControl> m_chat_control;
    Ref<PlayFabResult> m_result;
    String m_reason;

protected:
    static void _bind_methods();

public:
    void set_values(
            int64_t p_kind,
            const Ref<PlayFabPartyChatControl> &p_chat_control,
            const Ref<PlayFabResult> &p_result,
            const String &p_reason);

    int64_t get_kind() const;
    Ref<PlayFabPartyChatControl> get_chat_control() const;
    Ref<PlayFabResult> get_result() const;
    String get_reason() const;
};

class PlayFabPartyChatControl : public RefCounted {
    GDCLASS(PlayFabPartyChatControl, RefCounted);

    friend class PlayFabParty;
    friend class PlayFabPartyNetwork;
    friend class PlayFabPartyPeer;

    PlayFabParty *m_owner = nullptr;
    Party::PartyChatControl *m_native_handle = nullptr;
    String m_id;
    Ref<PlayFabUser> m_user;
    bool m_voice_enabled = false;
    bool m_text_enabled = false;
    bool m_transcription_enabled = false;
    bool m_local = false;

protected:
    static void _bind_methods();

public:
    void attach(PlayFabParty *p_owner, Party::PartyChatControl *p_handle, bool p_local);
    void set_snapshot(
            const String &p_id,
            const Ref<PlayFabUser> &p_user,
            bool p_voice_enabled,
            bool p_text_enabled,
            bool p_transcription_enabled,
            bool p_local);
    Party::PartyChatControl *get_native_handle() const;

    String get_id() const;
    Ref<PlayFabUser> get_user() const;
    bool is_voice_enabled() const;
    bool is_text_enabled() const;
    bool is_transcription_enabled() const;
    bool is_local() const;

    Signal send_text_async(const Array &p_targets, const String &p_message, const Ref<PlayFabPartyTextMessageConfig> &p_config = Ref<PlayFabPartyTextMessageConfig>());
    Signal set_permissions_async(const Ref<PlayFabPartyChatControl> &p_target, int64_t p_permissions);
    Signal set_muted_async(const Ref<PlayFabPartyChatControl> &p_target, bool p_muted);
    Signal destroy_async();
};

class PlayFabPartyChat : public RefCounted {
    GDCLASS(PlayFabPartyChat, RefCounted);

    friend class PlayFabParty;

    Array m_chat_controls;

protected:
    static void _bind_methods();

public:
    void clear();
    void track(const Ref<PlayFabPartyChatControl> &p_chat_control);
    void untrack(const Ref<PlayFabPartyChatControl> &p_chat_control);

    Ref<PlayFabPartyChatControl> get_local_chat_control(const Ref<PlayFabUser> &p_user) const;
    Array get_chat_controls() const;
};

class PlayFabPartyNetworkStateChange : public RefCounted {
    GDCLASS(PlayFabPartyNetworkStateChange, RefCounted);

    int64_t m_kind = 0;
    Ref<PlayFabPartyNetwork> m_network;
    Ref<PlayFabResult> m_result;
    Ref<PlayFabUser> m_user;
    int64_t m_peer_id = 0;
    int64_t m_state = 0;
    String m_reason;

protected:
    static void _bind_methods();

public:
    void set_values(
            int64_t p_kind,
            const Ref<PlayFabPartyNetwork> &p_network,
            const Ref<PlayFabResult> &p_result,
            const Ref<PlayFabUser> &p_user,
            int64_t p_peer_id,
            int64_t p_state,
            const String &p_reason);

    int64_t get_kind() const;
    Ref<PlayFabPartyNetwork> get_network() const;
    Ref<PlayFabResult> get_result() const;
    Ref<PlayFabUser> get_user() const;
    int64_t get_peer_id() const;
    int64_t get_state() const;
    String get_reason() const;
};

class PlayFabPartyNetwork : public RefCounted {
    GDCLASS(PlayFabPartyNetwork, RefCounted);

    friend class PlayFabParty;
    friend class PlayFabPartyPeer;

    PlayFabParty *m_owner = nullptr;
    Party::PartyNetwork *m_native_network = nullptr;
    Party::PartyLocalUser *m_native_local_user = nullptr;
    Party::PartyLocalEndpoint *m_native_local_endpoint = nullptr;
    Party::PartyLocalChatControl *m_native_local_chat_control = nullptr;
    String m_network_id;
    String m_descriptor;
    int64_t m_state = 0;
    Ref<PlayFabUser> m_local_user;
    Ref<PlayFabPartyPeer> m_local_peer;
    Ref<PlayFabPartyChatControl> m_local_chat_control;
    bool m_host = false;
    // Remembered failure result from PartyNetworkDestroyed so that any
    // in-flight join-chain *Completed handler can surface the real reason
    // the network died instead of a generic "Network destroyed during join."
    // string. Cleared on attach_native().
    Ref<PlayFabResult> m_destroyed_result;

protected:
    static void _bind_methods();

public:
    void set_owner(PlayFabParty *p_owner);
    void set_snapshot(
            const String &p_network_id,
            const String &p_descriptor,
            int64_t p_state,
            const Ref<PlayFabUser> &p_local_user,
            const Ref<PlayFabPartyPeer> &p_local_peer,
            const Ref<PlayFabPartyChatControl> &p_local_chat_control,
            bool p_host);
    void set_state_value(int64_t p_state);
    void set_descriptor(const String &p_descriptor);
    void set_network_id(const String &p_network_id);

    void attach_native(
            Party::PartyNetwork *p_network,
            Party::PartyLocalUser *p_local_user,
            Party::PartyLocalEndpoint *p_local_endpoint,
            Party::PartyLocalChatControl *p_local_chat_control);
    void detach_native();

    Party::PartyNetwork *get_native_handle() const;
    Party::PartyLocalUser *get_native_local_user() const;
    Party::PartyLocalEndpoint *get_native_local_endpoint() const;
    Party::PartyLocalChatControl *get_native_local_chat_control() const;

    String get_network_id() const;
    String get_descriptor() const;
    int64_t get_state() const;
    Ref<PlayFabUser> get_local_user() const;
    Ref<PlayFabPartyPeer> get_local_peer() const;
    Ref<PlayFabPartyChatControl> get_local_chat_control() const;
    bool is_host_network() const;

    Signal leave_async();
};

class PlayFabPartyPeer : public MultiplayerPeerExtension {
    GDCLASS(PlayFabPartyPeer, MultiplayerPeerExtension);

    friend class PlayFabParty;
    friend class PlayFabPartyNetwork;

public:
    struct PeerRecord {
        Party::PartyEndpoint *endpoint = nullptr;
        Dictionary entity_key;
        Ref<PlayFabPartyChatControl> chat_control;
        bool muted = false;
        int64_t permissions = 0;
    };

    struct InboundPacket {
        int32_t source_peer = 0;
        int32_t channel = 0;
        MultiplayerPeer::TransferMode mode = MultiplayerPeer::TRANSFER_MODE_RELIABLE;
        PackedByteArray payload;
    };

private:
    Ref<PlayFabPartyNetwork> m_network;
    int32_t m_transfer_channel = 0;
    MultiplayerPeer::TransferMode m_transfer_mode = MultiplayerPeer::TRANSFER_MODE_RELIABLE;
    int32_t m_target_peer = MultiplayerPeer::TARGET_PEER_BROADCAST;
    bool m_refusing_new_connections = false;
    MultiplayerPeer::ConnectionStatus m_connection_status = MultiplayerPeer::CONNECTION_DISCONNECTED;
    int32_t m_unique_id = 0;
    int32_t m_next_assigned_peer_id = 2;
    PackedByteArray m_current_packet;
    int32_t m_current_packet_peer = 0;
    int32_t m_current_packet_channel = 0;
    MultiplayerPeer::TransferMode m_current_packet_mode = MultiplayerPeer::TRANSFER_MODE_RELIABLE;
    std::deque<InboundPacket> m_inbound;
    std::map<int32_t, PeerRecord> m_peer_records;

protected:
    static void _bind_methods();

public:
    void set_network(const Ref<PlayFabPartyNetwork> &p_network);
    void set_unique_id(int32_t p_unique_id);
    void set_connection_status(MultiplayerPeer::ConnectionStatus p_status);

    int32_t allocate_peer_id();
    bool register_peer(int32_t p_peer_id, Party::PartyEndpoint *p_endpoint, const Dictionary &p_entity_key);
    // Splits register_peer's "add to records" from "emit Godot signal" so a
    // caller can insert the record BEFORE the GDScript await resumes (so
    // _put_packet has a target the moment _attach_network fires its first
    // rpc) but emit peer_connected only AFTER multiplayer.multiplayer_peer
    // has been assigned (so Godot's MultiplayerAPI captures the signal and
    // adds the peer to its internal connected set). Required by the
    // handshake-resolve fix in _resolve_handshake_assignment.
    bool insert_peer_record(int32_t p_peer_id, Party::PartyEndpoint *p_endpoint, const Dictionary &p_entity_key);
    void emit_peer_connected(int32_t p_peer_id);
    void update_peer_endpoint(int32_t p_peer_id, Party::PartyEndpoint *p_endpoint);
    void update_peer_chat_control(int32_t p_peer_id, const Ref<PlayFabPartyChatControl> &p_chat_control);
    void unregister_peer(int32_t p_peer_id);
    void unregister_endpoint(Party::PartyEndpoint *p_endpoint);
    int32_t find_peer_by_endpoint(Party::PartyEndpoint *p_endpoint) const;
    int32_t find_peer_by_entity_key(const Dictionary &p_entity_key) const;
    int32_t find_peer_by_chat_control(Party::PartyChatControl *p_chat_control) const;
    Party::PartyEndpoint *get_peer_endpoint(int32_t p_peer_id) const;
    void enqueue_inbound(int32_t p_source_peer, int32_t p_channel, MultiplayerPeer::TransferMode p_mode, const PackedByteArray &p_payload);
    void emit_chat_control_added(int32_t p_peer_id, const Ref<PlayFabPartyChatControl> &p_chat_control);
    void emit_chat_control_removed(int32_t p_peer_id);
    void emit_text_message(int32_t p_peer_id, const Ref<PlayFabPartyChatMessage> &p_message);
    void emit_transcription(int32_t p_peer_id, const Ref<PlayFabPartyChatMessage> &p_message);
    void emit_chat_permissions_changed(int32_t p_peer_id, int64_t p_permissions);
    void emit_peer_muted_changed(int32_t p_peer_id, bool p_muted);
    void emit_network_error(const Ref<PlayFabResult> &p_result);

    Ref<PlayFabPartyNetwork> get_network() const;
    Ref<PlayFabUser> get_local_user() const;
    String get_descriptor() const;
    Dictionary get_peer_entity_key(int64_t p_peer_id) const;
    Ref<PlayFabPartyMember> get_peer_member(int64_t p_peer_id) const;
    Array get_peers() const;
    Ref<PlayFabPartyChatControl> get_local_chat_control() const;
    Ref<PlayFabPartyChatControl> get_peer_chat_control(int64_t p_peer_id) const;

    Signal send_text_async(const String &p_message, const PackedInt32Array &p_target_peer_ids = PackedInt32Array(), const Ref<PlayFabPartyTextMessageConfig> &p_config = Ref<PlayFabPartyTextMessageConfig>());
    Signal set_peer_chat_permissions_async(int64_t p_peer_id, int64_t p_permissions);
    Signal set_peer_muted_async(int64_t p_peer_id, bool p_muted);

    void close_with_reason(const String &p_reason = String());

    Error _get_packet(const uint8_t **r_buffer, int32_t *r_buffer_size) override;
    Error _put_packet(const uint8_t *p_buffer, int32_t p_buffer_size) override;
    int32_t _get_available_packet_count() const override;
    int32_t _get_max_packet_size() const override;
    PackedByteArray _get_packet_script() override;
    Error _put_packet_script(const PackedByteArray &p_buffer) override;
    int32_t _get_packet_channel() const override;
    MultiplayerPeer::TransferMode _get_packet_mode() const override;
    void _set_transfer_channel(int32_t p_channel) override;
    int32_t _get_transfer_channel() const override;
    void _set_transfer_mode(MultiplayerPeer::TransferMode p_mode) override;
    MultiplayerPeer::TransferMode _get_transfer_mode() const override;
    void _set_target_peer(int32_t p_peer) override;
    int32_t _get_packet_peer() const override;
    bool _is_server() const override;
    void _poll() override;
    void _close() override;
    void _disconnect_peer(int32_t p_peer, bool p_force) override;
    int32_t _get_unique_id() const override;
    void _set_refuse_new_connections(bool p_enable) override;
    bool _is_refusing_new_connections() const override;
    bool _is_server_relay_supported() const override;
    MultiplayerPeer::ConnectionStatus _get_connection_status() const override;
};

class PlayFabParty : public RefCounted {
    GDCLASS(PlayFabParty, RefCounted);

    friend class PlayFabPartyChatControl;
    friend class PlayFabPartyNetwork;
    friend class PlayFabPartyPeer;

public:
    enum DirectPeerConnectivity : int64_t {
        DIRECT_PEER_CONNECTIVITY_NONE = 0,
        DIRECT_PEER_CONNECTIVITY_SAME_PLATFORM_TYPE = 1,
        DIRECT_PEER_CONNECTIVITY_DIFFERENT_PLATFORM_TYPE = 2,
        DIRECT_PEER_CONNECTIVITY_ANY_PLATFORM_TYPE = 3,
        DIRECT_PEER_CONNECTIVITY_SAME_ENTITY_LOGIN_PROVIDER = 4,
        DIRECT_PEER_CONNECTIVITY_DIFFERENT_ENTITY_LOGIN_PROVIDER = 8,
        DIRECT_PEER_CONNECTIVITY_ANY_ENTITY_LOGIN_PROVIDER = 12,
        DIRECT_PEER_CONNECTIVITY_ANY = 15,
        DIRECT_PEER_CONNECTIVITY_ONLY_SERVERS = 16,
    };

    enum NetworkState : int64_t {
        NETWORK_STATE_CREATING = 0,
        NETWORK_STATE_CONNECTING = 1,
        NETWORK_STATE_AUTHENTICATING = 2,
        NETWORK_STATE_CONNECTED = 3,
        NETWORK_STATE_DISCONNECTING = 4,
        NETWORK_STATE_DISCONNECTED = 5,
        NETWORK_STATE_FAILED = 6,
    };

    enum ChatPermission : int64_t {
        CHAT_PERMISSION_NONE = 0,
        CHAT_PERMISSION_SEND_AUDIO = 1,
        CHAT_PERMISSION_RECEIVE_AUDIO = 2,
        CHAT_PERMISSION_RECEIVE_TEXT = 4,
    };

    enum NetworkStateChangeKind : int64_t {
        NETWORK_CHANGE_STATE = 1,
        NETWORK_CHANGE_PEER_JOINED = 2,
        NETWORK_CHANGE_PEER_LEFT = 3,
        NETWORK_CHANGE_DESCRIPTOR_UPDATED = 4,
        NETWORK_CHANGE_DESTROYED = 5,
        NETWORK_CHANGE_ERROR = 6,
    };

    enum ChatStateChangeKind : int64_t {
        CHAT_CHANGE_CREATED = 1,
        CHAT_CHANGE_DESTROYED = 2,
        CHAT_CHANGE_PERMISSIONS_CHANGED = 3,
        CHAT_CHANGE_MUTED_CHANGED = 4,
    };

    enum PendingOperationKind : int32_t {
        PENDING_NONE = 0,
        PENDING_CREATE_NETWORK = 1,
        PENDING_CONNECT_NETWORK = 2,
        PENDING_AUTHENTICATE = 3,
        PENDING_CREATE_ENDPOINT = 4,
        PENDING_CREATE_CHAT_CONTROL = 5,
        PENDING_CONNECT_CHAT_CONTROL = 6,
        PENDING_LEAVE_NETWORK = 7,
        PENDING_DESTROY_CHAT_CONTROL = 8,
        PENDING_JOIN_HANDSHAKE = 9,
    };

    struct PendingOperation {
        int32_t kind = PENDING_NONE;
        Ref<PlayFabPendingSignal> pending_signal;
        Ref<PlayFabPartyNetwork> network;
        Ref<PlayFabUser> user;
        Ref<PlayFabPartyConfig> config;
        bool host = false;
        String descriptor;
        Party::PartyLocalUser *native_user = nullptr;
        Party::PartyNetwork *native_network = nullptr;
        Party::PartyLocalEndpoint *native_endpoint = nullptr;
        Party::PartyLocalChatControl *native_chat_control = nullptr;
        String invitation_id;
        uint32_t handshake_nonce = 0;
    };

private:
    PlayFab *m_owner = nullptr;
    bool m_initialized = false;
    bool m_processing_state_changes = false;
    bool m_shutting_down = false;
    XTaskQueueHandle m_task_queue = nullptr;
    Ref<PlayFabPartyChat> m_chat;
    std::vector<Ref<PlayFabPartyNetwork>> m_networks;
    std::map<PFEntityHandle, Party::PartyLocalUser *> m_local_users;
    std::vector<PendingOperation *> m_pending_operations;
    // Chat controls that arrived via PartyChatControlCreated before the
    // matching peer was registered (handshake race). Drained by
    // _attach_orphan_chat_controls() after every register_peer.
    std::vector<Ref<PlayFabPartyChatControl>> m_orphan_chat_controls;

    PlayFabRuntime *_get_runtime() const;
    Ref<PlayFabPendingSignal> _make_pending_signal();
    Signal _make_error_signal(HRESULT p_hresult, const String &p_code, const String &p_message, const Variant &p_data = Variant());
    Signal _make_ok_signal(const Variant &p_data = Variant());
    Ref<PlayFabResult> _validate_user(const Ref<PlayFabUser> &p_user) const;
    static bool _validate_direct_peer_connectivity(int64_t p_options, String *r_error);

    HRESULT _ensure_initialized(int p_local_udp_port_override = -1);
    Party::PartyLocalUser *_get_or_create_local_user(const Ref<PlayFabUser> &p_user, String *r_error);
    void _release_local_user(PFEntityHandle p_handle);
    void _release_all_local_users();
    void _track_network(const Ref<PlayFabPartyNetwork> &p_network);
    void _untrack_network(const Ref<PlayFabPartyNetwork> &p_network);
    Ref<PlayFabPartyNetwork> _find_network_by_native(Party::PartyNetwork *p_native) const;

    PendingOperation *_create_pending(int32_t p_kind);
    void _release_pending(PendingOperation *p_operation);
    void _complete_pending(PendingOperation *p_operation, const Ref<PlayFabResult> &p_result);
    PendingOperation *_find_pending(int32_t p_kind, Party::PartyNetwork *p_native_network);
    PendingOperation *_find_pending_join(Party::PartyNetwork *p_native_network);
    // Returns true and completes p_operation with a NETWORK_DESTROYED failure
    // when the operation's target network has already been detached
    // (e.g. because PartyNetworkDestroyed was processed earlier in the same
    // batch). Callers should bail out when this returns true to avoid touching
    // the dead native network handle.
    bool _abort_join_op_if_network_dead(PendingOperation *p_operation);

    int _pump_state_changes();
    void _process_state_change(const Party::PartyStateChange *p_change);
    void _process_create_new_network_completed(const Party::PartyStateChange *p_change);
    void _process_connect_to_network_completed(const Party::PartyStateChange *p_change);
    void _process_authenticate_local_user_completed(const Party::PartyStateChange *p_change);
    void _process_create_endpoint_completed(const Party::PartyStateChange *p_change);
    void _process_endpoint_created(const Party::PartyStateChange *p_change);
    void _process_endpoint_destroyed(const Party::PartyStateChange *p_change);
    void _process_endpoint_message_received(const Party::PartyStateChange *p_change);
    void _process_network_descriptor_changed(const Party::PartyStateChange *p_change);
    void _process_leave_network_completed(const Party::PartyStateChange *p_change);
    void _process_network_destroyed(const Party::PartyStateChange *p_change);
    void _process_create_chat_control_completed(const Party::PartyStateChange *p_change);
    void _process_connect_chat_control_completed(const Party::PartyStateChange *p_change);
    void _process_chat_control_created(const Party::PartyStateChange *p_change);
    void _process_chat_control_destroyed(const Party::PartyStateChange *p_change);
    void _process_chat_text_received(const Party::PartyStateChange *p_change);
    void _process_voice_chat_transcription_received(const Party::PartyStateChange *p_change);
    // Drains m_orphan_chat_controls — chat controls that arrived before
    // the matching peer was registered. Called after every register_peer
    // so handshake-race-orphaned controls are surfaced as chat_control_added
    // once the peer mapping exists.
    void _attach_orphan_chat_controls();

    void _emit_network_state(const Ref<PlayFabPartyNetwork> &p_network, int64_t p_kind, int64_t p_peer_id, const Ref<PlayFabResult> &p_result, const String &p_reason);
    void _emit_chat_state(const Ref<PlayFabPartyChatControl> &p_chat_control, int64_t p_kind, const Ref<PlayFabResult> &p_result, const String &p_reason);

    Signal _send_text_via_chat_control(Party::PartyLocalChatControl *p_local_chat_control, const std::vector<Party::PartyChatControl *> &p_targets, const String &p_message, const Ref<PlayFabPartyTextMessageConfig> &p_config);
    Signal _set_chat_permissions(Party::PartyLocalChatControl *p_local_chat_control, Party::PartyChatControl *p_target, int64_t p_permissions, bool *r_succeeded = nullptr);
    Signal _set_incoming_audio_muted(Party::PartyLocalChatControl *p_local_chat_control, Party::PartyChatControl *p_target, bool p_muted, bool *r_succeeded = nullptr);
    Signal _destroy_chat_control(const Ref<PlayFabPartyChatControl> &p_chat_control);

    HRESULT _start_create_endpoint_step(PendingOperation *p_operation);
    HRESULT _start_create_chat_control_step(PendingOperation *p_operation);
    HRESULT _start_handshake_step(PendingOperation *p_operation);
    HRESULT _send_handshake_request_to(PendingOperation *p_operation, Party::PartyEndpoint *p_target);
    void _send_handshake_assignment(PlayFabPartyPeer *p_peer, Party::PartyEndpoint *p_target_endpoint, uint32_t p_nonce, int32_t p_assigned_id);
    void _resolve_handshake_assignment(PlayFabPartyPeer *p_peer, Party::PartyEndpoint *p_sender_endpoint, int32_t p_assigned_id, PendingOperation *p_operation);
    PendingOperation *_find_handshake_pending(const Ref<PlayFabPartyNetwork> &p_network);

    String _capture_finalized_descriptor(Party::PartyNetwork *p_network) const;
    String _capture_network_identifier(Party::PartyNetwork *p_network) const;

    int64_t _translate_chat_permissions_to_native(int64_t p_permissions) const;
    int64_t _translate_chat_permissions_from_native(int32_t p_native) const;

    Ref<PlayFabResult> _party_error_result(uint32_t p_party_error, const String &p_code, const String &p_action) const;
    static String _party_error_message(uint32_t p_party_error);

protected:
    static void _bind_methods();

public:
    PlayFabParty();
    ~PlayFabParty();

    void set_owner(PlayFab *p_owner);

    bool is_initialized() const;
    Signal initialize_async(const Ref<PlayFabPartyConfig> &p_config = Ref<PlayFabPartyConfig>(), int p_local_udp_port = -1);
    Signal shutdown_async();
    void shutdown();
    int dispatch();

    Signal create_and_join_network_async(const Ref<PlayFabUser> &p_user, const Ref<PlayFabPartyConfig> &p_config = Ref<PlayFabPartyConfig>());
    Signal join_network_async(const Ref<PlayFabUser> &p_user, const String &p_descriptor, const Ref<PlayFabPartyConfig> &p_config = Ref<PlayFabPartyConfig>());
    Signal leave_network_async(const Ref<PlayFabPartyNetwork> &p_network);

    Ref<PlayFabPartyChat> get_chat() const;
    Array get_networks() const;
};

} // namespace godot

VARIANT_ENUM_CAST(godot::PlayFabParty::DirectPeerConnectivity);
VARIANT_ENUM_CAST(godot::PlayFabParty::NetworkState);
VARIANT_ENUM_CAST(godot::PlayFabParty::ChatPermission);
VARIANT_ENUM_CAST(godot::PlayFabParty::NetworkStateChangeKind);
VARIANT_ENUM_CAST(godot::PlayFabParty::ChatStateChangeKind);

#endif // GODOT_PLAYFAB_PARTY_H
