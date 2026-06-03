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
#include <godot_cpp/variant/string.hpp>
#include <godot_cpp/variant/variant.hpp>

#include <XUser.h>

namespace godot {

class GDK;
class GDKPendingSignal;
class GDKResult;
class GDKRuntime;

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
    AgeGroup get_age_group() const;
    String get_age_group_name() const;
    SignInState get_sign_in_state() const;
    String get_sign_in_state_name() const;
    bool is_guest() const;
    bool is_signed_in() const;
    bool is_store_user() const;

    HRESULT adopt_handle(XUserHandle p_user_handle);
    HRESULT refresh();
    bool matches_local_id(XUserLocalId p_user_local_id) const;
    XUserHandle get_handle() const;
    void clear();
};

class GDKUsers : public RefCounted {
    GDCLASS(GDKUsers, RefCounted);

    GDK *m_owner = nullptr;
    std::vector<Ref<GDKUser>> m_users;
    Ref<GDKUser> m_primary_user;
    bool m_runtime_ready = false;
    bool m_change_event_registered = false;
    XTaskQueueRegistrationToken m_change_token = {};

    static void CALLBACK _user_change_callback(void *p_context, XUserLocalId p_user_local_id, XUserChangeEvent p_event);

    GDKRuntime *_get_runtime() const;
    Signal _start_add_user_async(XUserAddOptions p_options, const String &p_action);
    bool _add_or_update_user(const Ref<GDKUser> &p_user);
    Ref<GDKUser> _find_user_by_local_id(XUserLocalId p_user_local_id) const;
    void _remove_user_by_local_id(XUserLocalId p_user_local_id);

protected:
    static void _bind_methods();

public:
    void set_owner(GDK *p_owner);

    Ref<GDKResult> on_runtime_initialized();
    void shutdown();

    Signal add_default_user_async();
    Signal add_user_with_ui_async();
    Ref<GDKUser> get_primary_user() const;
    Array get_users() const;
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
    void complete_add_user(XUserHandle p_user_handle, const Ref<GDKPendingSignal> &p_pending_signal);
};

} // namespace godot

VARIANT_ENUM_CAST(godot::GDKUser::AgeGroup);
VARIANT_ENUM_CAST(godot::GDKUser::SignInState);

#endif // GDK_USER_H
