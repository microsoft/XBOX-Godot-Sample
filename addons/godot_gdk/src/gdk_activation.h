#ifndef GDK_ACTIVATION_H
#define GDK_ACTIVATION_H

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>

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
// Coexistence note: GDKMultiplayerActivity also calls
// XGameActivationRegisterForEvent (for invite-typed events only). The PC
// GDK supports multiple subscribers on the same registration API, so
// GDKActivation maintains its own independent registration token.
class GDKActivation : public RefCounted {
    GDCLASS(GDKActivation, RefCounted);

    GDK *m_owner = nullptr;
    bool m_runtime_ready = false;
    bool m_activation_registered = false;
    XTaskQueueRegistrationToken m_activation_token = {};

    static void CALLBACK _activation_callback(void *p_context, const XGameActivationInfo *p_activation_info);

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
};

} // namespace godot

VARIANT_ENUM_CAST(godot::GDKActivation::ActivationType);

#endif // GDK_ACTIVATION_H
