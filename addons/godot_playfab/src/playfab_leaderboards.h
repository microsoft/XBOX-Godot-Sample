#ifndef GODOT_PLAYFAB_LEADERBOARDS_H
#define GODOT_PLAYFAB_LEADERBOARDS_H

#include <godot_cpp/classes/ref.hpp>
#include <godot_cpp/classes/ref_counted.hpp>
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/array.hpp>

namespace godot {

class PlayFab;
class PlayFabPendingSignal;
class PlayFabResult;
class PlayFabRuntime;
class PlayFabUser;

class PlayFabLeaderboards : public RefCounted {
    GDCLASS(PlayFabLeaderboards, RefCounted);

    PlayFab *m_owner = nullptr;

    PlayFabRuntime *_get_runtime() const;

protected:
    static void _bind_methods();

public:
    void set_owner(PlayFab *p_owner);

    Signal submit_score_async(
            const Ref<PlayFabUser> &p_user,
            const String &p_leaderboard_name,
            int64_t p_score,
            const Array &p_additional_scores = Array(),
            const String &p_metadata = String());
    Signal get_leaderboard_async(
            const Ref<PlayFabUser> &p_user,
            const String &p_leaderboard_name,
            int64_t p_start_position = 1,
            int64_t p_page_size = 10,
            int64_t p_version = -1);
    Signal get_leaderboard_around_user_async(
            const Ref<PlayFabUser> &p_user,
            const String &p_leaderboard_name,
            int64_t p_max_surrounding_entries = 10,
            int64_t p_version = -1);
    Signal get_friend_leaderboard_async(
            const Ref<PlayFabUser> &p_user,
            const String &p_leaderboard_name,
            bool p_include_xbox_friends = true,
            int64_t p_version = -1);
};

} // namespace godot

#endif // GODOT_PLAYFAB_LEADERBOARDS_H
