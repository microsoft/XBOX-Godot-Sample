#ifndef GODOT_PLAYFAB_GAMESAVES_H
#define GODOT_PLAYFAB_GAMESAVES_H

#include <godot_cpp/classes/ref.hpp>
#include <godot_cpp/classes/ref_counted.hpp>
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/string.hpp>

namespace godot {

class PlayFab;
class PlayFabPendingSignal;
class PlayFabResult;
class PlayFabRuntime;
class PlayFabUser;

class PlayFabGameSaves : public RefCounted {
    GDCLASS(PlayFabGameSaves, RefCounted);

public:
    enum AddUserOption : int64_t {
        ADD_USER_OPTION_NONE = 0,
        ADD_USER_OPTION_ROLLBACK_TO_LAST_KNOWN_GOOD = 1,
        ADD_USER_OPTION_ROLLBACK_TO_LAST_CONFLICT = 2,
    };

private:
    PlayFab *m_owner = nullptr;

    PlayFabRuntime *_get_runtime() const;

protected:
    static void _bind_methods();

public:
    void set_owner(PlayFab *p_owner);

    Signal add_user_with_ui_async(const Ref<PlayFabUser> &p_user, int64_t p_options = 0);
    Signal upload_with_ui_async(const Ref<PlayFabUser> &p_user, bool p_release_device_as_active = false);
    Signal set_save_description_async(const Ref<PlayFabUser> &p_user, const String &p_short_save_description);
    Signal reset_cloud_async(const Ref<PlayFabUser> &p_user);

    Ref<PlayFabResult> get_folder(const Ref<PlayFabUser> &p_user) const;
    Ref<PlayFabResult> get_folder_size(const Ref<PlayFabUser> &p_user) const;
    Ref<PlayFabResult> get_remaining_quota(const Ref<PlayFabUser> &p_user) const;
    Ref<PlayFabResult> is_connected_to_cloud(const Ref<PlayFabUser> &p_user) const;
};

} // namespace godot

#endif // GODOT_PLAYFAB_GAMESAVES_H
