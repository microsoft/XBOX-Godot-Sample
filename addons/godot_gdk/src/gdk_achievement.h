#ifndef GDK_ACHIEVEMENT_H
#define GDK_ACHIEVEMENT_H

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
#include <xsapi-c/services_c.h>

namespace godot {

class GDK;
class GDKAsyncOp;
class GDKDispatchOp;
class GDKResult;
class GDKRuntime;
class GDKUser;
class GDKXboxServices;

class GDKAchievement : public RefCounted {
    GDCLASS(GDKAchievement, RefCounted);

    String m_id;
    String m_name;
    String m_service_configuration_id;
    String m_progress_state;
    int64_t m_progress_percent = 0;
    bool m_unlocked = false;
    bool m_secret = false;
    String m_locked_description;
    String m_unlocked_description;

protected:
    static void _bind_methods();

public:
    String get_id() const;
    String get_name() const;
    String get_service_configuration_id() const;
    String get_progress_state() const;
    int64_t get_progress_percent() const;
    bool is_unlocked() const;
    bool is_secret() const;
    String get_locked_description() const;
    String get_unlocked_description() const;

    bool matches_id(const String &p_id) const;
    void populate_from_native(const XblAchievement &p_native_achievement);
};

class GDKAchievements : public RefCounted {
    GDCLASS(GDKAchievements, RefCounted);

    struct UserState {
        Ref<GDKUser> user;
        uint64_t xbox_user_id = 0;
        bool manager_added = false;
        bool initialized = false;
        std::vector<Ref<GDKAchievement>> achievements;
    };

    struct PendingQueryOp {
        uint64_t xbox_user_id = 0;
        Ref<GDKDispatchOp> op;
    };

    struct PendingUpdateOp {
        uint64_t xbox_user_id = 0;
        String achievement_id;
        uint32_t percent_complete = 0;
        bool submitted = false;
        Ref<GDKDispatchOp> op;
    };

    GDK *m_owner = nullptr;
    bool m_runtime_ready = false;
    std::vector<UserState> m_user_states;
    std::vector<PendingQueryOp> m_pending_query_ops;
    std::vector<PendingUpdateOp> m_pending_update_ops;

    GDKRuntime *_get_runtime() const;
    GDKXboxServices *_get_xbox_services() const;
    Ref<GDKDispatchOp> _make_completed_dispatch_op(const Ref<GDKResult> &p_result) const;
    Ref<GDKDispatchOp> _make_error_dispatch_op(HRESULT p_hresult, const String &p_code, const String &p_message) const;
    UserState *_find_user_state_by_xuid(uint64_t p_xbox_user_id);
    UserState *_find_user_state_by_local_id(XUserLocalId p_local_id);
    Ref<GDKAchievement> _find_cached_achievement(const UserState &p_state, const String &p_achievement_id) const;
    Array _get_cached_achievements_array(const UserState &p_state) const;
    Ref<GDKResult> _ensure_user_state(const Ref<GDKUser> &p_user, UserState **r_state);
    Ref<GDKResult> _refresh_user_cache(UserState &p_state);
    Ref<GDKResult> _refresh_single_achievement(UserState &p_state, const String &p_achievement_id);
    Ref<GDKResult> _submit_update(PendingUpdateOp &p_pending_update);
    void _complete_pending_queries(UserState &p_state);
    void _fail_pending_queries(uint64_t p_xbox_user_id, const Ref<GDKResult> &p_result);
    void _complete_pending_updates(UserState &p_state, const String &p_achievement_id);
    void _fail_pending_updates(uint64_t p_xbox_user_id, const Ref<GDKResult> &p_result);
    void _cancel_pending_query_op(GDKDispatchOp *p_op);
    void _cancel_pending_update_op(GDKDispatchOp *p_op);
    void _submit_waiting_updates(UserState &p_state);
    void _erase_completed_updates();
    void _erase_user_state(uint64_t p_xbox_user_id);

protected:
    static void _bind_methods();

public:
    void set_owner(GDK *p_owner);

    Ref<GDKResult> on_runtime_initialized();
    void shutdown();
    int dispatch();

    Ref<GDKDispatchOp> query_player_achievements_async(const Ref<GDKUser> &p_user);
    Ref<GDKDispatchOp> update_achievement_async(const Ref<GDKUser> &p_user, const String &p_achievement_id, int64_t p_percent_complete);
    Array get_cached_achievements(const Ref<GDKUser> &p_user) const;

    void on_user_removed(const Ref<GDKUser> &p_user);
};

} // namespace godot

#endif // GDK_ACHIEVEMENT_H
