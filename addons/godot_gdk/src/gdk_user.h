#ifndef GDK_USER_H
#define GDK_USER_H

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>

#include <vector>

#include <godot_cpp/classes/ref.hpp>
#include <godot_cpp/classes/ref_counted.hpp>
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/array.hpp>
#include <godot_cpp/variant/string.hpp>

#include <XUser.h>

namespace godot {

class GDK;
class GDKAsyncOp;
class GDKResult;
class GDKRuntime;

class GDKUser : public RefCounted {
    GDCLASS(GDKUser, RefCounted);

    XUserHandle m_user_handle = nullptr;
    XUserLocalId m_local_id = {};
    String m_xuid;
    String m_gamertag;
    bool m_is_guest = false;
    bool m_is_signed_in = false;

    HRESULT _populate_from_handle(XUserHandle p_user_handle);

protected:
    static void _bind_methods();

public:
    GDKUser();
    ~GDKUser();

    int64_t get_local_id() const;
    String get_xuid() const;
    String get_gamertag() const;
    bool is_guest() const;
    bool is_signed_in() const;

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
    Ref<GDKAsyncOp> _start_add_user_async(XUserAddOptions p_options, const String &p_action);
    bool _upsert_user(const Ref<GDKUser> &p_user);
    Ref<GDKUser> _find_user_by_local_id(XUserLocalId p_user_local_id) const;
    void _remove_user_by_local_id(XUserLocalId p_user_local_id);

protected:
    static void _bind_methods();

public:
    void set_owner(GDK *p_owner);

    Ref<GDKResult> on_runtime_initialized();
    void shutdown();

    Ref<GDKAsyncOp> add_default_user_async(bool p_allow_guests = false);
    Ref<GDKAsyncOp> add_user_with_ui_async();
    Ref<GDKUser> get_primary_user() const;
    Array get_users() const;

    void on_user_change(XUserLocalId p_user_local_id, XUserChangeEvent p_event);
    void complete_add_user(XUserHandle p_user_handle, const Ref<GDKAsyncOp> &p_op);
};

} // namespace godot

#endif // GDK_USER_H
