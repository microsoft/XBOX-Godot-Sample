#ifndef GODOT_PLAYFAB_USERS_H
#define GODOT_PLAYFAB_USERS_H

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>

#include <vector>

#include <godot_cpp/classes/ref.hpp>
#include <godot_cpp/classes/ref_counted.hpp>
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/array.hpp>
#include <godot_cpp/variant/variant.hpp>

#include <XUser.h>

namespace godot {

class PlayFab;
class PlayFabResult;
class PlayFabRuntime;
class PlayFabUser;

class PlayFabUsers : public RefCounted {
    GDCLASS(PlayFabUsers, RefCounted);

    PlayFab *m_owner = nullptr;
    std::vector<Ref<PlayFabUser>> m_users;

    PlayFabRuntime *_get_runtime() const;
    static bool _try_get_local_id_from_variant(const Variant &p_user_or_local_id, XUserLocalId *r_local_id, String *r_error = nullptr);
    bool _add_or_update_user(const Ref<PlayFabUser> &p_user);
    Ref<PlayFabUser> _find_user_by_local_id(XUserLocalId p_user_local_id) const;

protected:
    static void _bind_methods();

public:
    void set_owner(PlayFab *p_owner);

    Ref<PlayFabResult> on_runtime_initialized();
    void shutdown();

    Signal sign_in_async(const Variant &p_user_or_local_id, bool p_create_account = true);
    Ref<PlayFabUser> get_user_by_local_id(int64_t p_local_id) const;
    Ref<PlayFabUser> get_user(const Variant &p_user_or_local_id) const;
    Array get_users() const;
    bool add_or_update_user_session(const Ref<PlayFabUser> &p_user);

};

} // namespace godot

#endif // GODOT_PLAYFAB_USERS_H
