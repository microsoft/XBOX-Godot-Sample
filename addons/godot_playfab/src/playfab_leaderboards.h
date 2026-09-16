#ifndef GODOT_PLAYFAB_LEADERBOARDS_H
#define GODOT_PLAYFAB_LEADERBOARDS_H

#include <godot_cpp/classes/ref.hpp>
#include <godot_cpp/classes/ref_counted.hpp>
#include <godot_cpp/core/binder_common.hpp>
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

public:
    enum FriendSources : int64_t {
        FRIEND_SOURCE_NONE = 0x00,
        FRIEND_SOURCE_STEAM = 0x01,
        FRIEND_SOURCE_FACEBOOK = 0x02,
        FRIEND_SOURCE_XBOX = 0x04,
        FRIEND_SOURCE_PSN = 0x08,
        FRIEND_SOURCE_ALL = 0x10,
    };

private:
    PlayFab *m_owner = nullptr;

    PlayFabRuntime *_get_runtime() const;
    static FriendSources _friend_sources_from_legacy_bool(bool p_include_xbox_friends);

#ifdef GODOT_PLAYFAB_TEST_HOOKS
    Ref<PlayFabResult> _test_friend_leaderboard_request(
            BitField<FriendSources> p_friend_sources,
            const String &p_xbox_token = String(),
            int64_t p_version = -1) const;
    int64_t _test_friend_token_context_sources(
            BitField<FriendSources> p_friend_sources) const;
#endif

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
    Signal get_friend_leaderboard_with_sources_async(
            const Ref<PlayFabUser> &p_user,
            const String &p_leaderboard_name,
            BitField<FriendSources> p_friend_sources,
            int64_t p_version = -1);
};

} // namespace godot

VARIANT_BITFIELD_CAST(godot::PlayFabLeaderboards::FriendSources);

#endif // GODOT_PLAYFAB_LEADERBOARDS_H
