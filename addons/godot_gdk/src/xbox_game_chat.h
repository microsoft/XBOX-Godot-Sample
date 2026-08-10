#ifndef XBOX_GAME_CHAT_H
#define XBOX_GAME_CHAT_H

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>

#include <godot_cpp/classes/ref.hpp>
#include <godot_cpp/classes/ref_counted.hpp>
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/array.hpp>
#include <godot_cpp/variant/packed_byte_array.hpp>
#include <godot_cpp/variant/string.hpp>

namespace godot {

class Xbox;
class XboxResult;
class XboxRuntime;
class XboxUser;

// Wraps the Game Chat 2 (GameChat2.h) chat_manager singleton as a GDK service.
// Game Chat encodes its own opaque audio/text data frames; it does NOT pick or
// build a network transport. This wrapper exposes that data-frame surface --
// outgoing frames are emitted via the outgoing_data_frame signal and inbound
// frames are fed back in via process_incoming_data_frame() -- so titles can
// ferry frames over whatever transport they already have. Tests/sample use a
// single-process loopback.
class XboxGameChat : public RefCounted {
    GDCLASS(XboxGameChat, RefCounted);

    Xbox *m_owner = nullptr;
    bool m_runtime_ready = false;
    bool m_initialized = false;

    XboxRuntime *_get_runtime() const;
    // Returns the chat_user* matching p_xuid (cast to void* to keep GameChat2
    // types out of the header), or nullptr if no such user is present.
    void *_find_user(const String &p_xuid) const;
    // Drains queued outgoing data frames / inbound state changes, emitting the
    // matching signals. Each returns the number of items handled.
    int _pump_data_frames_count();
    int _pump_state_changes_count();

protected:
    static void _bind_methods();

public:
    enum CommunicationRelationship {
        RELATIONSHIP_NONE = 0x0,
        RELATIONSHIP_SEND_MICROPHONE_AUDIO = 0x1,
        RELATIONSHIP_SEND_TEXT_TO_SPEECH_AUDIO = 0x2,
        RELATIONSHIP_SEND_TEXT = 0x4,
        RELATIONSHIP_RECEIVE_AUDIO = 0x8,
        RELATIONSHIP_RECEIVE_TEXT = 0x10,
        RELATIONSHIP_SEND_ALL = 0x7,
        RELATIONSHIP_RECEIVE_ALL = 0x18,
        RELATIONSHIP_SEND_AND_RECEIVE_ALL = 0x1F,
    };

    enum TransportRequirement {
        TRANSPORT_GUARANTEED = 0,
        TRANSPORT_BEST_EFFORT = 1,
    };

    enum ChatIndicator {
        CHAT_INDICATOR_SILENT = 0,
        CHAT_INDICATOR_TALKING = 1,
        CHAT_INDICATOR_LOCAL_MICROPHONE_MUTED = 2,
        CHAT_INDICATOR_INCOMING_COMMUNICATIONS_MUTED = 3,
        CHAT_INDICATOR_REPUTATION_RESTRICTED = 4,
        CHAT_INDICATOR_PLATFORM_RESTRICTED = 5,
        CHAT_INDICATOR_NO_CHAT_FOCUS = 6,
        CHAT_INDICATOR_NO_MICROPHONE = 7,
    };

    void set_owner(Xbox *p_owner);

    Ref<XboxResult> on_runtime_initialized();
    void shutdown();
    int dispatch();

    Ref<XboxResult> initialize(int p_max_users = 16, int p_default_relationship = RELATIONSHIP_SEND_AND_RECEIVE_ALL);
    bool is_initialized() const;
    void cleanup();

    Ref<XboxResult> add_local_user(const Ref<XboxUser> &p_user);
    Ref<XboxResult> add_remote_user(const String &p_xuid, int64_t p_endpoint_id);
    Ref<XboxResult> remove_user(const String &p_xuid);

    void on_user_removed(const Ref<XboxUser> &p_user);

    Ref<XboxResult> set_communication_relationship(const String &p_local_xuid, const String &p_target_xuid, int p_relationship);
    Ref<XboxResult> set_microphone_muted(const String &p_local_xuid, bool p_muted);
    Ref<XboxResult> set_remote_user_muted(const String &p_local_xuid, const String &p_target_xuid, bool p_muted);
    Ref<XboxResult> set_audio_render_volume(const String &p_local_xuid, const String &p_target_xuid, float p_volume);

    Ref<XboxResult> send_text(const String &p_local_xuid, const String &p_text);
    Ref<XboxResult> synthesize_text_to_speech(const String &p_local_xuid, const String &p_text);

    Ref<XboxResult> process_incoming_data_frame(int64_t p_source_endpoint_id, const PackedByteArray &p_bytes);
    Array get_chat_users() const;
};

} // namespace godot

VARIANT_ENUM_CAST(godot::XboxGameChat::CommunicationRelationship);
VARIANT_ENUM_CAST(godot::XboxGameChat::TransportRequirement);
VARIANT_ENUM_CAST(godot::XboxGameChat::ChatIndicator);

#endif // XBOX_GAME_CHAT_H
