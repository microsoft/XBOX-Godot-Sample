#ifndef XBOX_SOCIAL_H
#define XBOX_SOCIAL_H

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
#include <godot_cpp/variant/packed_string_array.hpp>
#include <godot_cpp/variant/string.hpp>

#include <XUser.h>
#include <xsapi-c/services_c.h>

#include "xbox_presence.h"
#include "xbox_pending_signal.h"

namespace godot {

class Xbox;
class XboxResult;
class XboxRuntime;
class XboxUser;
class XboxServices;

class XboxSocialFilter : public RefCounted {
    GDCLASS(XboxSocialFilter, RefCounted);

public:
    enum PresenceFilter {
        PRESENCE_FILTER_UNKNOWN = 0,
        PRESENCE_FILTER_TITLE_ONLINE,
        PRESENCE_FILTER_TITLE_OFFLINE,
        PRESENCE_FILTER_TITLE_ONLINE_OUTSIDE_TITLE,
        PRESENCE_FILTER_ALL_ONLINE,
        PRESENCE_FILTER_ALL_OFFLINE,
        PRESENCE_FILTER_ALL_TITLE,
        PRESENCE_FILTER_ALL,
    };

    enum RelationshipFilter {
        RELATIONSHIP_FILTER_UNKNOWN = 0,
        RELATIONSHIP_FILTER_FRIENDS,
        RELATIONSHIP_FILTER_FAVORITE,
    };

private:
    PresenceFilter m_presence_filter = PRESENCE_FILTER_ALL;
    RelationshipFilter m_relationship_filter = RELATIONSHIP_FILTER_FRIENDS;

protected:
    static void _bind_methods();

public:
    PresenceFilter get_presence_filter() const;
    void set_presence_filter(PresenceFilter p_presence_filter);
    RelationshipFilter get_relationship_filter() const;
    void set_relationship_filter(RelationshipFilter p_relationship_filter);
};

class XboxSocialGroup : public RefCounted {
    GDCLASS(XboxSocialGroup, RefCounted);

public:
    enum GroupType {
        GROUP_TYPE_FILTER = 0,
        GROUP_TYPE_USER_LIST,
    };

private:
    Ref<XboxUser> m_local_user;
    XblSocialManagerUserGroupHandle m_group_handle = nullptr;
    bool m_loaded = false;
    GroupType m_group_type = GROUP_TYPE_FILTER;
    XboxSocialFilter::PresenceFilter m_presence_filter = XboxSocialFilter::PRESENCE_FILTER_ALL;
    XboxSocialFilter::RelationshipFilter m_relationship_filter = XboxSocialFilter::RELATIONSHIP_FILTER_FRIENDS;
    PackedStringArray m_tracked_xuids;

protected:
    static void _bind_methods();

public:
    Ref<XboxUser> get_local_user() const;
    bool is_loaded() const;
    GroupType get_group_type() const;
    String get_group_type_name() const;
    XboxSocialFilter::PresenceFilter get_presence_filter() const;
    XboxSocialFilter::RelationshipFilter get_relationship_filter() const;
    PackedStringArray get_tracked_xuids() const;

    void attach(const Ref<XboxUser> &p_local_user, XblSocialManagerUserGroupHandle p_group_handle);
    void set_group_type(GroupType p_group_type);
    void set_filters(XboxSocialFilter::PresenceFilter p_presence_filter, XboxSocialFilter::RelationshipFilter p_relationship_filter);
    void set_tracked_xuids(const PackedStringArray &p_tracked_xuids);
    void set_loaded(bool p_loaded);
    bool matches_handle(XblSocialManagerUserGroupHandle p_group_handle) const;
    XblSocialManagerUserGroupHandle get_handle() const;
    void invalidate();
};

class XboxSocialUser : public RefCounted {
    GDCLASS(XboxSocialUser, RefCounted);

    String m_xuid;
    bool m_is_favorite = false;
    bool m_is_friend = false;
    bool m_is_following_user = false;
    bool m_is_followed_by_caller = false;
    String m_display_name;
    String m_real_name;
    String m_display_picture_url;
    bool m_use_avatar = false;
    String m_gamerscore;
    String m_gamertag;
    String m_modern_gamertag;
    String m_modern_gamertag_suffix;
    String m_unique_modern_gamertag;
    Ref<XboxPresenceRecord> m_presence;
    Dictionary m_title_history;
    Dictionary m_preferred_color;

protected:
    static void _bind_methods();

public:
    String get_xuid() const;
    bool is_favorite() const;
    bool is_friend() const;
    bool is_following_user() const;
    bool is_followed_by_caller() const;
    String get_display_name() const;
    String get_real_name() const;
    String get_display_picture_url() const;
    bool uses_avatar() const;
    String get_gamerscore() const;
    String get_gamertag() const;
    String get_modern_gamertag() const;
    String get_modern_gamertag_suffix() const;
    String get_unique_modern_gamertag() const;
    Ref<XboxPresenceRecord> get_presence() const;
    Dictionary get_title_history() const;
    Dictionary get_preferred_color() const;

    void populate_from_native(const XblSocialManagerUser &p_social_user);
};

class XboxSocial : public RefCounted {
    GDCLASS(XboxSocial, RefCounted);

    struct LocalUserState {
        Ref<XboxUser> user;
        XUserLocalId local_id = {};
        bool graph_started = false;
        bool graph_ready = false;
        Ref<XboxSocialGroup> friends_group;
    };

    struct PendingFriendsOp {
        XUserLocalId local_id = {};
        XblSocialManagerUserGroupHandle group_handle = nullptr;
        Ref<XboxPendingSignal> request;
    };

    Xbox *m_owner = nullptr;
    bool m_runtime_ready = false;
    std::vector<LocalUserState> m_local_user_states;
    std::vector<Ref<XboxSocialGroup>> m_groups;
    std::vector<PendingFriendsOp> m_pending_friend_ops;
    std::vector<Ref<XboxSocialUser>> m_cached_users;

    XboxRuntime *_get_runtime() const;
    XboxServices *_get_xbox_services() const;
    XboxPresence *_get_presence_service() const;
    Signal _make_completed_signal(const Ref<XboxResult> &p_result) const;
    Signal _make_error_signal(HRESULT p_hresult, const String &p_code, const String &p_message) const;
    Ref<XboxResult> _ensure_ready_user(const Ref<XboxUser> &p_user) const;
    Ref<XboxResult> _ensure_local_user_state(const Ref<XboxUser> &p_user, LocalUserState **r_state, bool p_auto_start);
    LocalUserState *_find_local_user_state(XUserLocalId p_local_id);
    Ref<XboxSocialGroup> _find_group_by_handle(XblSocialManagerUserGroupHandle p_group_handle) const;
    Ref<XboxSocialUser> _find_cached_user(const String &p_xuid) const;
    Ref<XboxSocialUser> _cache_social_user(const XblSocialManagerUser &p_social_user, bool p_emit_social_signal, bool p_emit_presence_signal);
    Ref<XboxSocialGroup> _create_social_group_internal(const Ref<XboxUser> &p_user, const Ref<XboxSocialFilter> &p_filter, Ref<XboxResult> *r_error);
    Ref<XboxSocialGroup> _create_social_group_from_xuids_internal(const Ref<XboxUser> &p_user, const PackedStringArray &p_xuids, Ref<XboxResult> *r_error);
    Array _get_group_users_internal(const Ref<XboxSocialGroup> &p_group, bool p_emit_social_signal, bool p_emit_presence_signal, Ref<XboxResult> *r_error = nullptr);
    Ref<XboxResult> _refresh_group_metadata(const Ref<XboxSocialGroup> &p_group);
    void _complete_pending_friend_ops(XblSocialManagerUserGroupHandle p_group_handle);
    void _fail_pending_friend_ops(XUserLocalId p_local_id, const Ref<XboxResult> &p_result);
    void _fail_pending_friend_ops_for_group(XblSocialManagerUserGroupHandle p_group_handle, const Ref<XboxResult> &p_result);
    void _cancel_pending_friend_signal(XboxPendingSignal *p_request);
    void _destroy_group_internal(const Ref<XboxSocialGroup> &p_group, bool p_remove_from_collection);
    void _erase_local_user_state(XUserLocalId p_local_id);
    void _emit_filter_group_updates(XUserLocalId p_local_id);
    void _emit_social_graph_changed(XUserLocalId p_local_id);

protected:
    static void _bind_methods();

public:
    void set_owner(Xbox *p_owner);

    Ref<XboxResult> on_runtime_initialized();
    void shutdown();
    int dispatch();

    Ref<XboxResult> start_social_graph(const Ref<XboxUser> &p_user);
    void stop_social_graph(const Ref<XboxUser> &p_user);
    Signal get_friends_async(const Ref<XboxUser> &p_user);
    Ref<XboxResult> create_social_group(const Ref<XboxUser> &p_user, const Ref<XboxSocialFilter> &p_filter = Ref<XboxSocialFilter>());
    Ref<XboxResult> create_social_group_from_xuids(const Ref<XboxUser> &p_user, const PackedStringArray &p_xuids);
    Ref<XboxResult> update_social_user_group(const Ref<XboxSocialGroup> &p_group, const PackedStringArray &p_xuids);
    Ref<XboxResult> set_rich_presence_polling(const Ref<XboxUser> &p_user, bool p_enabled);
    void destroy_social_group(const Ref<XboxSocialGroup> &p_group);
    Ref<XboxResult> get_group_users(const Ref<XboxSocialGroup> &p_group);
    Signal submit_reputation_feedback_async(const Ref<XboxUser> &p_user, const String &p_target_xuid, const String &p_feedback_type, const String &p_reason = String(), const String &p_evidence_id = String());
    Signal submit_batch_reputation_feedback_async(const Ref<XboxUser> &p_user, const Array &p_feedback_items);

    void on_user_removed(const Ref<XboxUser> &p_user);
};

} // namespace godot

VARIANT_ENUM_CAST(godot::XboxSocialFilter::PresenceFilter);
VARIANT_ENUM_CAST(godot::XboxSocialFilter::RelationshipFilter);
VARIANT_ENUM_CAST(godot::XboxSocialGroup::GroupType);

#endif // XBOX_SOCIAL_H
