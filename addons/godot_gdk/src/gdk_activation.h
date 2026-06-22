#ifndef GDK_ACTIVATION_H
#define GDK_ACTIVATION_H

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>

#include <cstdint>
#include <functional>
#include <vector>

#include <godot_cpp/classes/ref.hpp>
#include <godot_cpp/classes/ref_counted.hpp>
#include <godot_cpp/core/binder_common.hpp>
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/dictionary.hpp>
#include <godot_cpp/variant/string.hpp>

#include "gdk_edition.h"

#if GDK_EDITION_HAS_XGAME_ACTIVATION
#include <XGameActivation.h>
#else
// October 2025 GDK (251001) predates XGameActivation.h. Fall back to
// the older XGameProtocol + XGameInvite activation APIs it supersedes.
#include <XGameInvite.h>
#include <XGameProtocol.h>
#endif

namespace godot {

class GDK;
class GDKResult;
class GDKRuntime;

// GDKActivation
// -------------
// Game activation event service for PC GDK. Exposed as GDK.activation.
// Registers for game activation so titles can react to protocol launches,
// file-association launches, and pending/accepted invites. The public surface
// (signals, accept_pending_invite, ActivationType) is identical across GDK
// editions; only the native backend differs:
//
//   * April 2026 GDK (260400+): the unified XGameActivation* APIs
//     (XGameActivationRegisterForEvent / UnregisterForEvent /
//     AcceptPendingInvite), which deliver one typed XGameActivationInfo.
//   * October 2025 GDK (251001): XGameActivation.h does not exist, so
//     the equivalent older APIs are used instead -- XGameProtocol* for protocol
//     activation and XGameInvite* for pending/accepted invites (plus
//     XGameInviteAcceptPendingInvite). File activation has no October 2025
//     equivalent and simply does not fire on that edition.
//
// Coexistence note: GDKActivation is the sole owner of the native activation
// subscription(s). Other services that need activation payloads subscribe to
// the internal C++ listener list so strict GDK builds never see duplicate OS
// registrations.
class GDKActivation : public RefCounted {
    GDCLASS(GDKActivation, RefCounted);

    struct ActivationListener {
        uint64_t id = 0;
        std::function<void(const Dictionary &)> callback;
    };

    GDK *m_owner = nullptr;
    bool m_runtime_ready = false;
    bool m_activation_registered = false;
    uint64_t m_next_activation_listener_id = 1;
#if GDK_EDITION_HAS_XGAME_ACTIVATION
    XTaskQueueRegistrationToken m_activation_token = {};
#else
    XTaskQueueRegistrationToken m_protocol_token = {};
    XTaskQueueRegistrationToken m_pending_invite_token = {};
    XTaskQueueRegistrationToken m_accepted_invite_token = {};
#endif
    std::vector<ActivationListener> m_activation_listeners;

#if GDK_EDITION_HAS_XGAME_ACTIVATION
    static void CALLBACK _activation_callback(void *p_context, const XGameActivationInfo *p_activation_info);
#else
    static void CALLBACK _protocol_callback(void *p_context, const char *p_protocol_uri);
    static void CALLBACK _pending_invite_callback(void *p_context, const char *p_invite_uri);
    static void CALLBACK _accepted_invite_callback(void *p_context, const char *p_invite_uri);
#endif
    void notify_activation_listeners_internal(const Dictionary &p_info);
    void finish_activation_dispatch_internal(const Dictionary &p_info);

protected:
    static void _bind_methods();

public:
    // Mirrors XGameActivationType (stable since the April 2026 GDK); the values
    // are fixed here so the public enum is identical on editions that predate
    // XGameActivation.h. A static_assert in the .cpp guards against SDK drift.
    enum ActivationType {
        ACTIVATION_TYPE_PROTOCOL = 0,
        ACTIVATION_TYPE_FILE = 1,
        ACTIVATION_TYPE_PENDING_GAME_INVITE = 2,
        ACTIVATION_TYPE_ACCEPTED_GAME_INVITE = 3,
    };

    void set_owner(GDK *p_owner);

    Ref<GDKResult> on_runtime_initialized();
    void shutdown();

    // Accept a pending invite by URI. Wraps XGameActivationAcceptPendingInvite
    // (April 2026 GDK) or XGameInviteAcceptPendingInvite (October 2025 GDK).
    Ref<GDKResult> accept_pending_invite(const String &p_invite_uri);

    // Shared dispatch helpers: build the activation info Dictionary, emit the
    // type-specific signal, fan out to internal listeners, then emit "activated".
    // Both native backends funnel through these so behavior is identical.
    void dispatch_protocol_activation_internal(const String &p_uri);
    void dispatch_file_activation_internal(const String &p_file);
    void dispatch_pending_invite_internal(const String &p_invite_uri);
    void dispatch_accepted_invite_internal(const String &p_invite_uri);

#if GDK_EDITION_HAS_XGAME_ACTIVATION
    // Internal: dispatched from the unified XGameActivation callback.
    void handle_activation_internal(const XGameActivationInfo *p_activation_info);
#endif
    uint64_t add_activation_listener(std::function<void(const Dictionary &)> p_callback);
    void remove_activation_listener(uint64_t p_listener_id);

    static Dictionary make_invite_dictionary_internal(const String &p_uri, const String &p_activation_type);
};

} // namespace godot

VARIANT_ENUM_CAST(godot::GDKActivation::ActivationType);

#endif // GDK_ACTIVATION_H
