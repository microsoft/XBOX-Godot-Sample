#ifndef GDK_USER_H
#define GDK_USER_H

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>

#include <vector>

#include <godot_cpp/classes/ref.hpp>
#include <godot_cpp/classes/ref_counted.hpp>
#include <godot_cpp/core/binder_common.hpp>
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/array.hpp>
#include <godot_cpp/variant/dictionary.hpp>
#include <godot_cpp/variant/packed_byte_array.hpp>
#include <godot_cpp/variant/packed_string_array.hpp>
#include <godot_cpp/variant/string.hpp>
#include <godot_cpp/variant/variant.hpp>

#include <XUser.h>

namespace godot {

class GDK;
class GDKPendingSignal;
class GDKResult;
class GDKRuntime;

// GDKUserSignOutDeferral
// ----------------------
// RefCounted owner of an XUserSignOutDeferralHandle, returned by
// GDKUsers::acquire_sign_out_deferral(). Holding the deferral asks the platform
// to delay completing a pending sign-out (XUserChangeEvent::SigningOut) so the
// title can flush per-user state such as a save. The handle is closed by
// XUserCloseSignOutDeferralHandle on release() or destruction.
//
// This mirrors GDKDisplayTimeoutDeferral: the handle outlives nothing in the
// runtime, so release() is always safe -- including after GDK.shutdown().
class GDKUserSignOutDeferral : public RefCounted {
    GDCLASS(GDKUserSignOutDeferral, RefCounted);

    XUserSignOutDeferralHandle m_handle = nullptr;

protected:
    static void _bind_methods();

public:
    GDKUserSignOutDeferral() = default;
    ~GDKUserSignOutDeferral();

    bool is_valid() const;
    void release();

    // Internal: takes ownership of the handle. Called only by GDKUsers.
    void set_handle_internal(XUserSignOutDeferralHandle p_handle);
};

class GDKUser : public RefCounted {
    GDCLASS(GDKUser, RefCounted);

public:
    enum AgeGroup {
        AGE_GROUP_UNKNOWN = 0,
        AGE_GROUP_CHILD,
        AGE_GROUP_TEEN,
        AGE_GROUP_ADULT,
    };

    enum SignInState {
        SIGN_IN_STATE_SIGNED_OUT = 0,
        SIGN_IN_STATE_SIGNING_OUT,
        SIGN_IN_STATE_SIGNED_IN,
    };

private:
    XUserHandle m_user_handle = nullptr;
    XUserLocalId m_local_id = {};
    String m_xuid;
    String m_gamertag;
    String m_modern_gamertag;
    String m_modern_gamertag_suffix;
    String m_unique_modern_gamertag;
    AgeGroup m_age_group = AGE_GROUP_UNKNOWN;
    SignInState m_sign_in_state = SIGN_IN_STATE_SIGNED_OUT;
    bool m_is_guest = false;
    bool m_is_signed_in = false;
    bool m_is_store_user = false;

    HRESULT _populate_from_handle(XUserHandle p_user_handle);

protected:
    static void _bind_methods();

public:
    GDKUser();
    ~GDKUser();

    int64_t get_local_id() const;
    String get_xuid() const;
    String get_gamertag() const;
    String get_modern_gamertag() const;
    String get_modern_gamertag_suffix() const;
    String get_unique_modern_gamertag() const;
    AgeGroup get_age_group() const;
    String get_age_group_name() const;
    SignInState get_sign_in_state() const;
    String get_sign_in_state_name() const;
    bool is_guest() const;
    bool is_signed_in() const;
    bool is_store_user() const;
    bool is_valid() const;
    bool is_same_user(const Ref<GDKUser> &p_other) const;
    Ref<GDKUser> duplicate_user() const;

    HRESULT adopt_handle(XUserHandle p_user_handle);
    HRESULT refresh();
    bool matches_local_id(XUserLocalId p_user_local_id) const;
    XUserLocalId get_native_local_id() const;
    XUserHandle get_handle() const;
    void clear();
};

class GDKUsers : public RefCounted {
    GDCLASS(GDKUsers, RefCounted);

public:
    // Maps to XUserDefaultAudioEndpointKind.
    enum AudioEndpointKind {
        AUDIO_ENDPOINT_KIND_COMMUNICATION_RENDER = static_cast<uint32_t>(XUserDefaultAudioEndpointKind::CommunicationRender),
        AUDIO_ENDPOINT_KIND_COMMUNICATION_CAPTURE = static_cast<uint32_t>(XUserDefaultAudioEndpointKind::CommunicationCapture),
    };

private:
    // One entry per device the platform has reported through
    // XUserRegisterForDeviceAssociationChanged. The registration replays every
    // current association when it is installed, so this cache is the
    // authoritative user<->controller pairing view required by XR-112.
    struct DeviceAssociation {
        APP_LOCAL_DEVICE_ID device_id = {};
        XUserLocalId user_local_id = {};
    };

    GDK *m_owner = nullptr;
    std::vector<Ref<GDKUser>> m_users;
    std::vector<DeviceAssociation> m_device_associations;
    Ref<GDKUser> m_primary_user;
    bool m_runtime_ready = false;
    bool m_change_event_registered = false;
    bool m_device_association_registered = false;
    bool m_audio_endpoint_registered = false;
    XTaskQueueRegistrationToken m_change_token = {};
    XTaskQueueRegistrationToken m_device_association_token = {};
    XTaskQueueRegistrationToken m_audio_endpoint_token = {};

    static void CALLBACK _user_change_callback(void *p_context, XUserLocalId p_user_local_id, XUserChangeEvent p_event);
    static void CALLBACK _device_association_changed_callback(void *p_context, const XUserDeviceAssociationChange *p_change);
    static void CALLBACK _default_audio_endpoint_changed_callback(
            void *p_context,
            XUserLocalId p_user_local_id,
            XUserDefaultAudioEndpointKind p_kind,
            const wchar_t *p_endpoint_id_utf16);

    GDKRuntime *_get_runtime() const;
    Signal _start_add_user_async(XUserAddOptions p_options, const String &p_action);
    bool _add_or_update_user(const Ref<GDKUser> &p_user);
    Ref<GDKUser> _find_user_by_local_id(XUserLocalId p_user_local_id) const;
    void _remove_user_by_local_id(XUserLocalId p_user_local_id);
    Ref<GDKResult> _wrap_found_handle(XUserHandle p_user_handle);
    void _update_device_association(const APP_LOCAL_DEVICE_ID &p_device_id, XUserLocalId p_user_local_id);

protected:
    static void _bind_methods();

public:
    void set_owner(GDK *p_owner);

    Ref<GDKResult> on_runtime_initialized();
    void shutdown();

    Signal add_default_user_async();
    Signal add_user_with_ui_async(bool p_allow_guests = false);
    Signal add_user_by_id_with_ui_async(const String &p_xuid);
    Ref<GDKUser> get_primary_user() const;
    Array get_users() const;
    Ref<GDKResult> get_max_users() const;
    bool is_sign_out_available() const;
    Signal sign_out_async(const Ref<GDKUser> &p_user);
    Ref<GDKResult> acquire_sign_out_deferral() const;
    Ref<GDKResult> find_user_by_xuid(const String &p_xuid);
    Ref<GDKResult> find_user_by_local_id(int64_t p_local_id);
    Ref<GDKResult> find_user_for_device(const String &p_device_id);
    Signal find_controller_for_user_with_ui_async(const Ref<GDKUser> &p_user);
    Array get_device_associations() const;
    PackedStringArray get_devices_for_user(const Ref<GDKUser> &p_user) const;
    Ref<GDKResult> get_default_audio_endpoint(const Ref<GDKUser> &p_user, int64_t p_kind = AUDIO_ENDPOINT_KIND_COMMUNICATION_RENDER) const;
    Signal check_privilege_async(const Ref<GDKUser> &p_user, int64_t p_privilege);
    Signal resolve_privilege_with_ui_async(const Ref<GDKUser> &p_user, int64_t p_privilege);
    Signal resolve_issue_with_ui_async(const Ref<GDKUser> &p_user, const String &p_url = String());
    Signal get_gamer_picture_async(const Ref<GDKUser> &p_user, const String &p_size = "medium");
    Signal get_token_and_signature_async(
            const Ref<GDKUser> &p_user,
            const String &p_method,
            const String &p_url,
            const Dictionary &p_headers = Dictionary(),
            const PackedByteArray &p_body = PackedByteArray(),
            bool p_force_refresh = false);

    void on_user_change(XUserLocalId p_user_local_id, XUserChangeEvent p_event);
    void on_device_association_changed(const XUserDeviceAssociationChange &p_change);
    void on_default_audio_endpoint_changed(XUserLocalId p_user_local_id, XUserDefaultAudioEndpointKind p_kind, const String &p_endpoint_id);
    void complete_add_user(XUserHandle p_user_handle, const Ref<GDKPendingSignal> &p_pending_signal);
    void reconcile_signed_out_user(const Ref<GDKUser> &p_user);
};

} // namespace godot

VARIANT_ENUM_CAST(godot::GDKUser::AgeGroup);
VARIANT_ENUM_CAST(godot::GDKUser::SignInState);
VARIANT_ENUM_CAST(godot::GDKUsers::AudioEndpointKind);

#endif // GDK_USER_H
