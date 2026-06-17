#include "gdk_game_chat.h"

#include <cstring>
#include <vector>

#include <godot_cpp/variant/char_string.hpp>
#include <godot_cpp/variant/dictionary.hpp>
#include <godot_cpp/variant/packed_int64_array.hpp>

#include <GameChat2.h>
// GameChat2's C++ class methods are defined (out-of-line, not inline) in
// GameChat2Impl.h, which forwards to the C API exported by GameChat2.lib.
// Include it in exactly one translation unit (this one) to provide those
// definitions; no other godot_gdk source includes the GameChat2 headers.
#include <GameChat2Impl.h>

#include "gdk.h"
#include "gdk_result.h"
#include "gdk_runtime.h"
#include "gdk_user.h"

namespace godot {

using namespace xbox::services::game_chat_2;

namespace {

// GameChat2 XUIDs are PCWSTR (UTF-16). godot::String stores char32_t; utf16()
// yields a Char16String whose data is layout-compatible with wchar_t on Windows.
String wide_to_string(PCWSTR p_value) {
    if (p_value == nullptr) {
        return String();
    }
    return String(reinterpret_cast<const char16_t *>(p_value));
}

} // namespace

void GDKGameChat::_bind_methods() {
    ClassDB::bind_method(
            D_METHOD("initialize", "max_users", "default_relationship"),
            &GDKGameChat::initialize,
            DEFVAL(16),
            DEFVAL(RELATIONSHIP_SEND_AND_RECEIVE_ALL));
    ClassDB::bind_method(D_METHOD("is_initialized"), &GDKGameChat::is_initialized);
    ClassDB::bind_method(D_METHOD("cleanup"), &GDKGameChat::cleanup);
    ClassDB::bind_method(D_METHOD("add_local_user", "user"), &GDKGameChat::add_local_user);
    ClassDB::bind_method(D_METHOD("add_remote_user", "xuid", "endpoint_id"), &GDKGameChat::add_remote_user);
    ClassDB::bind_method(D_METHOD("remove_user", "xuid"), &GDKGameChat::remove_user);
    ClassDB::bind_method(
            D_METHOD("set_communication_relationship", "local_xuid", "target_xuid", "relationship"),
            &GDKGameChat::set_communication_relationship);
    ClassDB::bind_method(D_METHOD("set_microphone_muted", "local_xuid", "muted"), &GDKGameChat::set_microphone_muted);
    ClassDB::bind_method(
            D_METHOD("set_remote_user_muted", "local_xuid", "target_xuid", "muted"),
            &GDKGameChat::set_remote_user_muted);
    ClassDB::bind_method(
            D_METHOD("set_audio_render_volume", "local_xuid", "target_xuid", "volume"),
            &GDKGameChat::set_audio_render_volume);
    ClassDB::bind_method(D_METHOD("send_text", "local_xuid", "text"), &GDKGameChat::send_text);
    ClassDB::bind_method(
            D_METHOD("synthesize_text_to_speech", "local_xuid", "text"),
            &GDKGameChat::synthesize_text_to_speech);
    ClassDB::bind_method(
            D_METHOD("process_incoming_data_frame", "source_endpoint_id", "bytes"),
            &GDKGameChat::process_incoming_data_frame);
    ClassDB::bind_method(D_METHOD("get_chat_users"), &GDKGameChat::get_chat_users);

    ADD_SIGNAL(MethodInfo(
            "outgoing_data_frame",
            PropertyInfo(Variant::PACKED_INT64_ARRAY, "target_endpoint_ids"),
            PropertyInfo(Variant::PACKED_BYTE_ARRAY, "bytes"),
            PropertyInfo(Variant::INT, "transport_requirement")));
    ADD_SIGNAL(MethodInfo(
            "text_chat_received",
            PropertyInfo(Variant::STRING, "sender_xuid"),
            PropertyInfo(Variant::STRING, "message")));
    ADD_SIGNAL(MethodInfo(
            "transcribed_chat_received",
            PropertyInfo(Variant::STRING, "speaker_xuid"),
            PropertyInfo(Variant::STRING, "message")));

    BIND_ENUM_CONSTANT(RELATIONSHIP_NONE);
    BIND_ENUM_CONSTANT(RELATIONSHIP_SEND_MICROPHONE_AUDIO);
    BIND_ENUM_CONSTANT(RELATIONSHIP_SEND_TEXT_TO_SPEECH_AUDIO);
    BIND_ENUM_CONSTANT(RELATIONSHIP_SEND_TEXT);
    BIND_ENUM_CONSTANT(RELATIONSHIP_RECEIVE_AUDIO);
    BIND_ENUM_CONSTANT(RELATIONSHIP_RECEIVE_TEXT);
    BIND_ENUM_CONSTANT(RELATIONSHIP_SEND_ALL);
    BIND_ENUM_CONSTANT(RELATIONSHIP_RECEIVE_ALL);
    BIND_ENUM_CONSTANT(RELATIONSHIP_SEND_AND_RECEIVE_ALL);

    BIND_ENUM_CONSTANT(TRANSPORT_GUARANTEED);
    BIND_ENUM_CONSTANT(TRANSPORT_BEST_EFFORT);

    BIND_ENUM_CONSTANT(CHAT_INDICATOR_SILENT);
    BIND_ENUM_CONSTANT(CHAT_INDICATOR_TALKING);
    BIND_ENUM_CONSTANT(CHAT_INDICATOR_LOCAL_MICROPHONE_MUTED);
    BIND_ENUM_CONSTANT(CHAT_INDICATOR_INCOMING_COMMUNICATIONS_MUTED);
    BIND_ENUM_CONSTANT(CHAT_INDICATOR_REPUTATION_RESTRICTED);
    BIND_ENUM_CONSTANT(CHAT_INDICATOR_PLATFORM_RESTRICTED);
    BIND_ENUM_CONSTANT(CHAT_INDICATOR_NO_CHAT_FOCUS);
    BIND_ENUM_CONSTANT(CHAT_INDICATOR_NO_MICROPHONE);
}

void GDKGameChat::set_owner(GDK *p_owner) {
    m_owner = p_owner;
}

GDKRuntime *GDKGameChat::_get_runtime() const {
    return m_owner == nullptr ? nullptr : m_owner->get_runtime();
}

void *GDKGameChat::_find_user(const String &p_xuid) const {
    if (!m_initialized || p_xuid.is_empty()) {
        return nullptr;
    }

    uint32_t count = 0;
    chat_user_array users = nullptr;
    chat_manager::singleton_instance().get_chat_users(&count, &users);
    for (uint32_t i = 0; i < count; ++i) {
        chat_user *user = users[i];
        if (user == nullptr) {
            continue;
        }
        if (wide_to_string(user->xbox_user_id()) == p_xuid) {
            return user;
        }
    }
    return nullptr;
}

Ref<GDKResult> GDKGameChat::on_runtime_initialized() {
    GDKRuntime *runtime = _get_runtime();
    if (runtime == nullptr || !runtime->is_initialized()) {
        return GDKResult::error_result(
                E_FAIL,
                "runtime_not_initialized",
                "Cannot ready the game chat service before the GDK runtime.");
    }

    m_runtime_ready = true;
    return GDKResult::ok_result();
}

void GDKGameChat::shutdown() {
    cleanup();
    m_runtime_ready = false;
}

int GDKGameChat::dispatch() {
    if (!m_initialized) {
        return 0;
    }

    GDKRuntime *runtime = _get_runtime();
    if (runtime != nullptr && runtime->is_shutting_down()) {
        return 0;
    }

    int handled = 0;
    handled += _pump_data_frames_count();
    handled += _pump_state_changes_count();
    return handled;
}

int GDKGameChat::_pump_data_frames_count() {
    // Extract every frame into local storage *before* emitting any Godot
    // signal. Signals run synchronously, so a handler is free to call back into
    // GameChat2 (e.g. process_incoming_data_frame, remove_user, cleanup) or
    // recurse through GDK.dispatch(). GameChat2 forbids re-entering the
    // start/finish_processing window, so we must close it first and emit after.
    struct OutgoingFrame {
        PackedInt64Array targets;
        PackedByteArray bytes;
        int transport = TRANSPORT_BEST_EFFORT;
    };
    std::vector<OutgoingFrame> outgoing;

    uint32_t count = 0;
    game_chat_data_frame_array frames = nullptr;
    chat_manager::singleton_instance().start_processing_data_frames(&count, &frames);

    for (uint32_t i = 0; i < count; ++i) {
        const game_chat_data_frame *frame = frames[i];
        if (frame == nullptr) {
            continue;
        }

        OutgoingFrame entry;
        entry.targets.resize(static_cast<int64_t>(frame->target_endpoint_identifier_count));
        for (uint32_t t = 0; t < frame->target_endpoint_identifier_count; ++t) {
            entry.targets.set(static_cast<int64_t>(t), static_cast<int64_t>(frame->target_endpoint_identifiers[t]));
        }

        entry.bytes.resize(static_cast<int64_t>(frame->packet_byte_count));
        if (frame->packet_byte_count > 0) {
            memcpy(entry.bytes.ptrw(), frame->packet_buffer, frame->packet_byte_count);
        }

        entry.transport = frame->transport_requirement == game_chat_data_transport_requirement::guaranteed
                ? TRANSPORT_GUARANTEED
                : TRANSPORT_BEST_EFFORT;
        outgoing.push_back(std::move(entry));
    }

    chat_manager::singleton_instance().finish_processing_data_frames(frames);

    for (const OutgoingFrame &entry : outgoing) {
        emit_signal("outgoing_data_frame", entry.targets, entry.bytes, entry.transport);
    }
    return static_cast<int>(count);
}

int GDKGameChat::_pump_state_changes_count() {
    // As with data frames, copy out every state change and close GameChat2's
    // processing window before emitting signals, so synchronous handlers cannot
    // re-enter the start/finish_processing_state_changes window.
    struct ChatMessage {
        const char *signal_name;
        String identity;
        String message;
    };
    std::vector<ChatMessage> messages;

    uint32_t count = 0;
    game_chat_state_change_array changes = nullptr;
    chat_manager::singleton_instance().start_processing_state_changes(&count, &changes);

    for (uint32_t i = 0; i < count; ++i) {
        const game_chat_state_change *base = changes[i];
        if (base == nullptr) {
            continue;
        }

        switch (base->state_change_type) {
            case game_chat_state_change_type::text_chat_received: {
                const auto *change = static_cast<const game_chat_text_chat_received_state_change *>(base);
                const String sender = change->sender == nullptr
                        ? String()
                        : wide_to_string(change->sender->xbox_user_id());
                messages.push_back({ "text_chat_received", sender, wide_to_string(change->message) });
                break;
            }
            case game_chat_state_change_type::transcribed_chat_received: {
                const auto *change = static_cast<const game_chat_transcribed_chat_received_state_change *>(base);
                const String speaker = change->speaker == nullptr
                        ? String()
                        : wide_to_string(change->speaker->xbox_user_id());
                messages.push_back({ "transcribed_chat_received", speaker, wide_to_string(change->message) });
                break;
            }
            default:
                break;
        }
    }

    chat_manager::singleton_instance().finish_processing_state_changes(changes);

    for (const ChatMessage &entry : messages) {
        emit_signal(entry.signal_name, entry.identity, entry.message);
    }
    return static_cast<int>(count);
}

Ref<GDKResult> GDKGameChat::initialize(int p_max_users, int p_default_relationship) {
    GDKRuntime *runtime = _get_runtime();
    if (runtime == nullptr || !runtime->is_initialized() || !m_runtime_ready) {
        return GDKResult::error_result(
                E_FAIL,
                "not_initialized",
                "GDK is not initialized. Call GDK.initialize() before GDK.game_chat.initialize().");
    }
    if (m_initialized) {
        return GDKResult::error_result(
                E_FAIL,
                "already_initialized",
                "Game chat is already initialized. Call cleanup() before re-initializing.");
    }
    if (p_max_users <= 0) {
        return GDKResult::error_result(
                E_INVALIDARG,
                "invalid_max_users",
                "max_users must be greater than zero.");
    }

    const auto default_relationship = static_cast<game_chat_communication_relationship_flags>(p_default_relationship);
    chat_manager::singleton_instance().initialize(
            static_cast<uint32_t>(p_max_users),
            1.0f,
            default_relationship);
    m_initialized = true;
    return GDKResult::ok_result();
}

bool GDKGameChat::is_initialized() const {
    return m_initialized;
}

void GDKGameChat::cleanup() {
    if (!m_initialized) {
        return;
    }
    chat_manager::singleton_instance().cleanup();
    m_initialized = false;
}

Ref<GDKResult> GDKGameChat::add_local_user(const Ref<GDKUser> &p_user) {
    if (!m_initialized) {
        return GDKResult::error_result(E_FAIL, "not_initialized", "Call GDK.game_chat.initialize() first.");
    }
    if (!p_user.is_valid() || p_user->get_handle() == nullptr || !p_user->is_signed_in()) {
        return GDKResult::error_result(E_INVALIDARG, "invalid_user", "A signed-in GDKUser is required to add a local chat user.");
    }

    const String xuid = p_user->get_xuid();
    if (xuid.is_empty()) {
        return GDKResult::error_result(E_INVALIDARG, "invalid_user", "The GDKUser does not have a valid Xbox User ID.");
    }

    const Char16String wide_xuid = xuid.utf16();
    chat_user *user = chat_manager::singleton_instance().add_local_user(
            reinterpret_cast<PCWSTR>(wide_xuid.get_data()));
    if (user == nullptr) {
        return GDKResult::error_result(E_FAIL, "add_user_failed", "Game chat could not add the local user.");
    }

    Dictionary data;
    data["xuid"] = xuid;
    return GDKResult::ok_result(data);
}

Ref<GDKResult> GDKGameChat::add_remote_user(const String &p_xuid, int64_t p_endpoint_id) {
    if (!m_initialized) {
        return GDKResult::error_result(E_FAIL, "not_initialized", "Call GDK.game_chat.initialize() first.");
    }
    if (p_xuid.is_empty()) {
        return GDKResult::error_result(E_INVALIDARG, "invalid_xuid", "A non-empty Xbox User ID is required.");
    }

    const Char16String wide_xuid = p_xuid.utf16();
    chat_user *user = chat_manager::singleton_instance().add_remote_user(
            reinterpret_cast<PCWSTR>(wide_xuid.get_data()),
            static_cast<uint64_t>(p_endpoint_id));
    if (user == nullptr) {
        return GDKResult::error_result(E_FAIL, "add_user_failed", "Game chat could not add the remote user.");
    }

    Dictionary data;
    data["xuid"] = p_xuid;
    data["endpoint_id"] = p_endpoint_id;
    return GDKResult::ok_result(data);
}

Ref<GDKResult> GDKGameChat::remove_user(const String &p_xuid) {
    if (!m_initialized) {
        return GDKResult::error_result(E_FAIL, "not_initialized", "Call GDK.game_chat.initialize() first.");
    }

    auto *user = static_cast<chat_user *>(_find_user(p_xuid));
    if (user == nullptr) {
        return GDKResult::error_result(E_INVALIDARG, "user_not_found", "No chat user matches the supplied Xbox User ID.");
    }

    chat_manager::singleton_instance().remove_user(user);
    return GDKResult::ok_result();
}

Ref<GDKResult> GDKGameChat::set_communication_relationship(const String &p_local_xuid, const String &p_target_xuid, int p_relationship) {
    if (!m_initialized) {
        return GDKResult::error_result(E_FAIL, "not_initialized", "Call GDK.game_chat.initialize() first.");
    }

    auto *local = static_cast<chat_user *>(_find_user(p_local_xuid));
    if (local == nullptr) {
        return GDKResult::error_result(E_INVALIDARG, "user_not_found", "No chat user matches the local Xbox User ID.");
    }
    chat_user::chat_user_local *local_state = local->local();
    if (local_state == nullptr) {
        return GDKResult::error_result(E_INVALIDARG, "not_local_user", "The local Xbox User ID does not refer to a local chat user.");
    }
    auto *target = static_cast<chat_user *>(_find_user(p_target_xuid));
    if (target == nullptr) {
        return GDKResult::error_result(E_INVALIDARG, "target_not_found", "No chat user matches the target Xbox User ID.");
    }

    local_state->set_communication_relationship(
            target,
            static_cast<game_chat_communication_relationship_flags>(p_relationship));
    return GDKResult::ok_result();
}

Ref<GDKResult> GDKGameChat::set_microphone_muted(const String &p_local_xuid, bool p_muted) {
    if (!m_initialized) {
        return GDKResult::error_result(E_FAIL, "not_initialized", "Call GDK.game_chat.initialize() first.");
    }

    auto *local = static_cast<chat_user *>(_find_user(p_local_xuid));
    if (local == nullptr) {
        return GDKResult::error_result(E_INVALIDARG, "user_not_found", "No chat user matches the local Xbox User ID.");
    }
    chat_user::chat_user_local *local_state = local->local();
    if (local_state == nullptr) {
        return GDKResult::error_result(E_INVALIDARG, "not_local_user", "The local Xbox User ID does not refer to a local chat user.");
    }

    local_state->set_microphone_muted(p_muted);
    return GDKResult::ok_result();
}

Ref<GDKResult> GDKGameChat::set_remote_user_muted(const String &p_local_xuid, const String &p_target_xuid, bool p_muted) {
    if (!m_initialized) {
        return GDKResult::error_result(E_FAIL, "not_initialized", "Call GDK.game_chat.initialize() first.");
    }

    auto *local = static_cast<chat_user *>(_find_user(p_local_xuid));
    if (local == nullptr) {
        return GDKResult::error_result(E_INVALIDARG, "user_not_found", "No chat user matches the local Xbox User ID.");
    }
    chat_user::chat_user_local *local_state = local->local();
    if (local_state == nullptr) {
        return GDKResult::error_result(E_INVALIDARG, "not_local_user", "The local Xbox User ID does not refer to a local chat user.");
    }
    auto *target = static_cast<chat_user *>(_find_user(p_target_xuid));
    if (target == nullptr) {
        return GDKResult::error_result(E_INVALIDARG, "target_not_found", "No chat user matches the target Xbox User ID.");
    }

    local_state->set_remote_user_muted(target, p_muted);
    return GDKResult::ok_result();
}

Ref<GDKResult> GDKGameChat::set_audio_render_volume(const String &p_local_xuid, const String &p_target_xuid, float p_volume) {
    if (!m_initialized) {
        return GDKResult::error_result(E_FAIL, "not_initialized", "Call GDK.game_chat.initialize() first.");
    }

    auto *local = static_cast<chat_user *>(_find_user(p_local_xuid));
    if (local == nullptr) {
        return GDKResult::error_result(E_INVALIDARG, "user_not_found", "No chat user matches the local Xbox User ID.");
    }
    chat_user::chat_user_local *local_state = local->local();
    if (local_state == nullptr) {
        return GDKResult::error_result(E_INVALIDARG, "not_local_user", "The local Xbox User ID does not refer to a local chat user.");
    }
    auto *target = static_cast<chat_user *>(_find_user(p_target_xuid));
    if (target == nullptr) {
        return GDKResult::error_result(E_INVALIDARG, "target_not_found", "No chat user matches the target Xbox User ID.");
    }

    local_state->set_audio_render_volume(target, p_volume);
    return GDKResult::ok_result();
}

Ref<GDKResult> GDKGameChat::send_text(const String &p_local_xuid, const String &p_text) {
    if (!m_initialized) {
        return GDKResult::error_result(E_FAIL, "not_initialized", "Call GDK.game_chat.initialize() first.");
    }
    if (p_text.is_empty() || p_text.length() > c_maxChatTextMessageLength) {
        return GDKResult::error_result(
                E_INVALIDARG,
                "invalid_text",
                "Chat text must contain between 1 and 1023 characters.");
    }

    auto *local = static_cast<chat_user *>(_find_user(p_local_xuid));
    if (local == nullptr) {
        return GDKResult::error_result(E_INVALIDARG, "user_not_found", "No chat user matches the local Xbox User ID.");
    }
    chat_user::chat_user_local *local_state = local->local();
    if (local_state == nullptr) {
        return GDKResult::error_result(E_INVALIDARG, "not_local_user", "The local Xbox User ID does not refer to a local chat user.");
    }

    const Char16String wide_text = p_text.utf16();
    local_state->send_chat_text(reinterpret_cast<PCWSTR>(wide_text.get_data()));
    return GDKResult::ok_result();
}

Ref<GDKResult> GDKGameChat::synthesize_text_to_speech(const String &p_local_xuid, const String &p_text) {
    if (!m_initialized) {
        return GDKResult::error_result(E_FAIL, "not_initialized", "Call GDK.game_chat.initialize() first.");
    }
    if (p_text.is_empty()) {
        return GDKResult::error_result(E_INVALIDARG, "invalid_text", "Text to synthesize must be non-empty.");
    }

    auto *local = static_cast<chat_user *>(_find_user(p_local_xuid));
    if (local == nullptr) {
        return GDKResult::error_result(E_INVALIDARG, "user_not_found", "No chat user matches the local Xbox User ID.");
    }
    chat_user::chat_user_local *local_state = local->local();
    if (local_state == nullptr) {
        return GDKResult::error_result(E_INVALIDARG, "not_local_user", "The local Xbox User ID does not refer to a local chat user.");
    }

    const Char16String wide_text = p_text.utf16();
    local_state->synthesize_text_to_speech(reinterpret_cast<PCWSTR>(wide_text.get_data()));
    return GDKResult::ok_result();
}

Ref<GDKResult> GDKGameChat::process_incoming_data_frame(int64_t p_source_endpoint_id, const PackedByteArray &p_bytes) {
    if (!m_initialized) {
        return GDKResult::error_result(E_FAIL, "not_initialized", "Call GDK.game_chat.initialize() first.");
    }
    if (p_bytes.is_empty()) {
        return GDKResult::error_result(E_INVALIDARG, "invalid_data", "The incoming data frame must contain at least one byte.");
    }

    chat_manager::singleton_instance().process_incoming_data(
            static_cast<uint64_t>(p_source_endpoint_id),
            static_cast<uint32_t>(p_bytes.size()),
            p_bytes.ptr());
    return GDKResult::ok_result();
}

Array GDKGameChat::get_chat_users() const {
    Array result;
    if (!m_initialized) {
        return result;
    }

    uint32_t count = 0;
    chat_user_array users = nullptr;
    chat_manager::singleton_instance().get_chat_users(&count, &users);
    for (uint32_t i = 0; i < count; ++i) {
        chat_user *user = users[i];
        if (user == nullptr) {
            continue;
        }
        Dictionary entry;
        entry["xuid"] = wide_to_string(user->xbox_user_id());
        entry["is_local"] = user->local() != nullptr;
        entry["chat_indicator"] = static_cast<int>(user->chat_indicator());
        result.push_back(entry);
    }
    return result;
}

} // namespace godot
