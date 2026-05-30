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

#include <XGameActivation.h>

namespace godot {

class GDK;
class GDKResult;
class GDKRuntime;

// GDKActivation
// -------------
// Game activation event service for PC GDK. Exposed as GDK.activation.
// Wraps XGameActivation* registration so titles can react to protocol
// launches, file-association launches, and pending/accepted invites.
//
// PC GDK availability matrix (XGameActivation.h / xgameruntime.lib):
//   XGameActivationRegisterForEvent     -- YES, _GAMING_DESKTOP
//   XGameActivationUnregisterForEvent   -- YES, _GAMING_DESKTOP
//   XGameActivationAcceptPendingInvite  -- YES, _GAMING_DESKTOP
//
// XGameProtocol.h is intentionally NOT wrapped: both
// XGameProtocolRegisterForActivation / XGameProtocolUnregisterForActivation
// are explicitly __declspec(deprecated) in the SDK and are superseded by
// XGameActivation*. Use GDK.activation.protocol_activated for the modern
// protocol-activation event.
//
// Coexistence note: GDKActivation is the sole owner of the native
// XGameActivationRegisterForEvent subscription. Other services that need
// activation payloads subscribe to the internal C++ listener list so strict
// GDK builds never see duplicate OS registrations.
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
    XTaskQueueRegistrationToken m_activation_token = {};
    std::vector<ActivationListener> m_activation_listeners;

    static void CALLBACK _activation_callback(void *p_context, const XGameActivationInfo *p_activation_info);
    void notify_activation_listeners_internal(const Dictionary &p_info);

protected:
    static void _bind_methods();

public:
    // Maps to XGameActivationType.
    enum ActivationType {
        ACTIVATION_TYPE_PROTOCOL = static_cast<uint32_t>(XGameActivationType::Protocol),
        ACTIVATION_TYPE_FILE = static_cast<uint32_t>(XGameActivationType::File),
        ACTIVATION_TYPE_PENDING_GAME_INVITE = static_cast<uint32_t>(XGameActivationType::PendingGameInvite),
        ACTIVATION_TYPE_ACCEPTED_GAME_INVITE = static_cast<uint32_t>(XGameActivationType::AcceptedGameInvite),
    };

    void set_owner(GDK *p_owner);

    Ref<GDKResult> on_runtime_initialized();
    void shutdown();

    // Accept a pending invite by URI. Wraps XGameActivationAcceptPendingInvite.
    Ref<GDKResult> accept_pending_invite(const String &p_invite_uri);

    // Internal: dispatched from the activation callback.
    void handle_activation_internal(const XGameActivationInfo *p_activation_info);
    uint64_t add_activation_listener(std::function<void(const Dictionary &)> p_callback);
    void remove_activation_listener(uint64_t p_listener_id);

    static Dictionary make_invite_dictionary_internal(const String &p_uri, const String &p_activation_type);
};

} // namespace godot

VARIANT_ENUM_CAST(godot::GDKActivation::ActivationType);

#endif // GDK_ACTIVATION_H
