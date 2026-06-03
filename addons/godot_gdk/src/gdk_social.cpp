#include "gdk_social.h"

#include <algorithm>
#include <cerrno>
#include <cstdio>
#include <cstdlib>
#include <cstring>

#include "gdk.h"
#include "gdk_pending_signal.h"
#include "gdk_result.h"
#include "gdk_runtime.h"
#include "gdk_user.h"
#include "gdk_xbox_services.h"

namespace godot {

namespace {

String _utf8_or_empty(const char *p_value) {
    if (p_value == nullptr || p_value[0] == '\0') {
        return String();
    }

    return String::utf8(p_value);
}

bool _try_parse_xuid(const String &p_xuid, uint64_t *r_xuid) {
    if (r_xuid == nullptr) {
        return false;
    }

    const String normalized = p_xuid.strip_edges();
    if (normalized.is_empty()) {
        return false;
    }

    const CharString utf8 = normalized.utf8();
    char *end_ptr = nullptr;
    errno = 0;
    const unsigned long long parsed = std::strtoull(utf8.get_data(), &end_ptr, 10);
    if (errno != 0 || end_ptr == nullptr || *end_ptr != '\0') {
        return false;
    }

    *r_xuid = static_cast<uint64_t>(parsed);
    return true;
}

XblPresenceFilter _presence_filter_to_native(GDKSocialFilter::PresenceFilter p_filter) {
    switch (p_filter) {
        case GDKSocialFilter::PRESENCE_FILTER_TITLE_ONLINE:
            return XblPresenceFilter::TitleOnline;
        case GDKSocialFilter::PRESENCE_FILTER_TITLE_OFFLINE:
            return XblPresenceFilter::TitleOffline;
        case GDKSocialFilter::PRESENCE_FILTER_TITLE_ONLINE_OUTSIDE_TITLE:
            return XblPresenceFilter::TitleOnlineOutsideTitle;
        case GDKSocialFilter::PRESENCE_FILTER_ALL_ONLINE:
            return XblPresenceFilter::AllOnline;
        case GDKSocialFilter::PRESENCE_FILTER_ALL_OFFLINE:
            return XblPresenceFilter::AllOffline;
        case GDKSocialFilter::PRESENCE_FILTER_ALL_TITLE:
            return XblPresenceFilter::AllTitle;
        case GDKSocialFilter::PRESENCE_FILTER_ALL:
            return XblPresenceFilter::All;
        case GDKSocialFilter::PRESENCE_FILTER_UNKNOWN:
        default:
            return XblPresenceFilter::Unknown;
    }
}

GDKSocialFilter::PresenceFilter _presence_filter_from_native(XblPresenceFilter p_filter) {
    switch (p_filter) {
        case XblPresenceFilter::TitleOnline:
            return GDKSocialFilter::PRESENCE_FILTER_TITLE_ONLINE;
        case XblPresenceFilter::TitleOffline:
            return GDKSocialFilter::PRESENCE_FILTER_TITLE_OFFLINE;
        case XblPresenceFilter::TitleOnlineOutsideTitle:
            return GDKSocialFilter::PRESENCE_FILTER_TITLE_ONLINE_OUTSIDE_TITLE;
        case XblPresenceFilter::AllOnline:
            return GDKSocialFilter::PRESENCE_FILTER_ALL_ONLINE;
        case XblPresenceFilter::AllOffline:
            return GDKSocialFilter::PRESENCE_FILTER_ALL_OFFLINE;
        case XblPresenceFilter::AllTitle:
            return GDKSocialFilter::PRESENCE_FILTER_ALL_TITLE;
        case XblPresenceFilter::All:
            return GDKSocialFilter::PRESENCE_FILTER_ALL;
        case XblPresenceFilter::Unknown:
        default:
            return GDKSocialFilter::PRESENCE_FILTER_UNKNOWN;
    }
}

XblRelationshipFilter _relationship_filter_to_native(GDKSocialFilter::RelationshipFilter p_filter) {
    switch (p_filter) {
        case GDKSocialFilter::RELATIONSHIP_FILTER_FRIENDS:
            return XblRelationshipFilter::Friends;
        case GDKSocialFilter::RELATIONSHIP_FILTER_FAVORITE:
            return XblRelationshipFilter::Favorite;
        case GDKSocialFilter::RELATIONSHIP_FILTER_UNKNOWN:
        default:
            return XblRelationshipFilter::Unknown;
    }
}

GDKSocialFilter::RelationshipFilter _relationship_filter_from_native(XblRelationshipFilter p_filter) {
    switch (p_filter) {
        case XblRelationshipFilter::Friends:
            return GDKSocialFilter::RELATIONSHIP_FILTER_FRIENDS;
        case XblRelationshipFilter::Favorite:
            return GDKSocialFilter::RELATIONSHIP_FILTER_FAVORITE;
        case XblRelationshipFilter::Unknown:
        default:
            return GDKSocialFilter::RELATIONSHIP_FILTER_UNKNOWN;
    }
}

String _group_type_to_name(GDKSocialGroup::GroupType p_group_type) {
    switch (p_group_type) {
        case GDKSocialGroup::GROUP_TYPE_USER_LIST:
            return "user_list";
        case GDKSocialGroup::GROUP_TYPE_FILTER:
        default:
            return "filter";
    }
}

Dictionary _make_title_history_dictionary(const XblTitleHistory &p_title_history) {
    Dictionary history;
    history["has_user_played"] = p_title_history.hasUserPlayed;
    history["last_time_user_played"] = static_cast<int64_t>(p_title_history.lastTimeUserPlayed);
    history["last_time_user_played_text"] = _utf8_or_empty(p_title_history.lastTimeUserPlayedText);
    return history;
}

Dictionary _make_preferred_color_dictionary(const XblPreferredColor &p_preferred_color) {
    Dictionary preferred_color;
    preferred_color["primary"] = _utf8_or_empty(p_preferred_color.primaryColor);
    preferred_color["secondary"] = _utf8_or_empty(p_preferred_color.secondaryColor);
    preferred_color["tertiary"] = _utf8_or_empty(p_preferred_color.tertiaryColor);
    return preferred_color;
}

void _append_unique_local_id(std::vector<uint64_t> *r_values, XUserLocalId p_local_id) {
    if (r_values == nullptr) {
        return;
    }

    if (std::find(r_values->begin(), r_values->end(), p_local_id.value) == r_values->end()) {
        r_values->push_back(p_local_id.value);
    }
}

Ref<GDKResult> _make_social_error_result(HRESULT p_hresult, const String &p_code, const String &p_message) {
    return GDKResult::error_result(p_hresult, p_code, p_message);
}

} // namespace

void GDKSocialFilter::_bind_methods() {
    ClassDB::bind_method(D_METHOD("get_presence_filter"), &GDKSocialFilter::get_presence_filter);
    ClassDB::bind_method(D_METHOD("set_presence_filter", "presence_filter"), &GDKSocialFilter::set_presence_filter);
    ClassDB::bind_method(D_METHOD("get_relationship_filter"), &GDKSocialFilter::get_relationship_filter);
    ClassDB::bind_method(D_METHOD("set_relationship_filter", "relationship_filter"), &GDKSocialFilter::set_relationship_filter);

    BIND_ENUM_CONSTANT(PRESENCE_FILTER_UNKNOWN);
    BIND_ENUM_CONSTANT(PRESENCE_FILTER_TITLE_ONLINE);
    BIND_ENUM_CONSTANT(PRESENCE_FILTER_TITLE_OFFLINE);
    BIND_ENUM_CONSTANT(PRESENCE_FILTER_TITLE_ONLINE_OUTSIDE_TITLE);
    BIND_ENUM_CONSTANT(PRESENCE_FILTER_ALL_ONLINE);
    BIND_ENUM_CONSTANT(PRESENCE_FILTER_ALL_OFFLINE);
    BIND_ENUM_CONSTANT(PRESENCE_FILTER_ALL_TITLE);
    BIND_ENUM_CONSTANT(PRESENCE_FILTER_ALL);

    BIND_ENUM_CONSTANT(RELATIONSHIP_FILTER_UNKNOWN);
    BIND_ENUM_CONSTANT(RELATIONSHIP_FILTER_FRIENDS);
    BIND_ENUM_CONSTANT(RELATIONSHIP_FILTER_FAVORITE);

    ADD_PROPERTY(PropertyInfo(Variant::INT, "presence_filter", PROPERTY_HINT_ENUM, "Unknown,Title Online,Title Offline,Title Online Outside Title,All Online,All Offline,All Title,All"), "set_presence_filter", "get_presence_filter");
    ADD_PROPERTY(PropertyInfo(Variant::INT, "relationship_filter", PROPERTY_HINT_ENUM, "Unknown,Friends,Favorite"), "set_relationship_filter", "get_relationship_filter");
}

GDKSocialFilter::PresenceFilter GDKSocialFilter::get_presence_filter() const {
    return m_presence_filter;
}

void GDKSocialFilter::set_presence_filter(PresenceFilter p_presence_filter) {
    m_presence_filter = p_presence_filter;
}

GDKSocialFilter::RelationshipFilter GDKSocialFilter::get_relationship_filter() const {
    return m_relationship_filter;
}

void GDKSocialFilter::set_relationship_filter(RelationshipFilter p_relationship_filter) {
    m_relationship_filter = p_relationship_filter;
}

void GDKSocialGroup::_bind_methods() {
    ClassDB::bind_method(D_METHOD("get_local_user"), &GDKSocialGroup::get_local_user);
    ClassDB::bind_method(D_METHOD("is_loaded"), &GDKSocialGroup::is_loaded);
    ClassDB::bind_method(D_METHOD("get_group_type"), &GDKSocialGroup::get_group_type);
    ClassDB::bind_method(D_METHOD("get_group_type_name"), &GDKSocialGroup::get_group_type_name);
    ClassDB::bind_method(D_METHOD("get_presence_filter"), &GDKSocialGroup::get_presence_filter);
    ClassDB::bind_method(D_METHOD("get_relationship_filter"), &GDKSocialGroup::get_relationship_filter);
    ClassDB::bind_method(D_METHOD("get_tracked_xuids"), &GDKSocialGroup::get_tracked_xuids);

    BIND_ENUM_CONSTANT(GROUP_TYPE_FILTER);
    BIND_ENUM_CONSTANT(GROUP_TYPE_USER_LIST);

    ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "local_user", PROPERTY_HINT_NONE, "", PROPERTY_USAGE_DEFAULT, "GDKUser"), "", "get_local_user");
    ADD_PROPERTY(PropertyInfo(Variant::BOOL, "loaded"), "", "is_loaded");
    ADD_PROPERTY(PropertyInfo(Variant::INT, "group_type", PROPERTY_HINT_ENUM, "Filter,User List"), "", "get_group_type");
    ADD_PROPERTY(PropertyInfo(Variant::PACKED_STRING_ARRAY, "tracked_xuids"), "", "get_tracked_xuids");
}

Ref<GDKUser> GDKSocialGroup::get_local_user() const {
    return m_local_user;
}

bool GDKSocialGroup::is_loaded() const {
    return m_loaded;
}

GDKSocialGroup::GroupType GDKSocialGroup::get_group_type() const {
    return m_group_type;
}

String GDKSocialGroup::get_group_type_name() const {
    return _group_type_to_name(m_group_type);
}

GDKSocialFilter::PresenceFilter GDKSocialGroup::get_presence_filter() const {
    return m_presence_filter;
}

GDKSocialFilter::RelationshipFilter GDKSocialGroup::get_relationship_filter() const {
    return m_relationship_filter;
}

PackedStringArray GDKSocialGroup::get_tracked_xuids() const {
    return m_tracked_xuids;
}

void GDKSocialGroup::attach(const Ref<GDKUser> &p_local_user, XblSocialManagerUserGroupHandle p_group_handle) {
    m_local_user = p_local_user;
    m_group_handle = p_group_handle;
}

void GDKSocialGroup::set_group_type(GroupType p_group_type) {
    m_group_type = p_group_type;
}

void GDKSocialGroup::set_filters(GDKSocialFilter::PresenceFilter p_presence_filter, GDKSocialFilter::RelationshipFilter p_relationship_filter) {
    m_presence_filter = p_presence_filter;
    m_relationship_filter = p_relationship_filter;
}

void GDKSocialGroup::set_tracked_xuids(const PackedStringArray &p_tracked_xuids) {
    m_tracked_xuids = p_tracked_xuids;
}

void GDKSocialGroup::set_loaded(bool p_loaded) {
    m_loaded = p_loaded;
}

bool GDKSocialGroup::matches_handle(XblSocialManagerUserGroupHandle p_group_handle) const {
    return m_group_handle == p_group_handle;
}

XblSocialManagerUserGroupHandle GDKSocialGroup::get_handle() const {
    return m_group_handle;
}

void GDKSocialGroup::invalidate() {
    m_group_handle = nullptr;
    m_loaded = false;
}

void GDKSocialUser::_bind_methods() {
    ClassDB::bind_method(D_METHOD("get_xuid"), &GDKSocialUser::get_xuid);
    ClassDB::bind_method(D_METHOD("is_favorite"), &GDKSocialUser::is_favorite);
    ClassDB::bind_method(D_METHOD("is_friend"), &GDKSocialUser::is_friend);
    ClassDB::bind_method(D_METHOD("is_following_user"), &GDKSocialUser::is_following_user);
    ClassDB::bind_method(D_METHOD("is_followed_by_caller"), &GDKSocialUser::is_followed_by_caller);
    ClassDB::bind_method(D_METHOD("get_display_name"), &GDKSocialUser::get_display_name);
    ClassDB::bind_method(D_METHOD("get_real_name"), &GDKSocialUser::get_real_name);
    ClassDB::bind_method(D_METHOD("get_display_picture_url"), &GDKSocialUser::get_display_picture_url);
    ClassDB::bind_method(D_METHOD("uses_avatar"), &GDKSocialUser::uses_avatar);
    ClassDB::bind_method(D_METHOD("get_gamerscore"), &GDKSocialUser::get_gamerscore);
    ClassDB::bind_method(D_METHOD("get_gamertag"), &GDKSocialUser::get_gamertag);
    ClassDB::bind_method(D_METHOD("get_modern_gamertag"), &GDKSocialUser::get_modern_gamertag);
    ClassDB::bind_method(D_METHOD("get_modern_gamertag_suffix"), &GDKSocialUser::get_modern_gamertag_suffix);
    ClassDB::bind_method(D_METHOD("get_unique_modern_gamertag"), &GDKSocialUser::get_unique_modern_gamertag);
    ClassDB::bind_method(D_METHOD("get_presence"), &GDKSocialUser::get_presence);
    ClassDB::bind_method(D_METHOD("get_title_history"), &GDKSocialUser::get_title_history);
    ClassDB::bind_method(D_METHOD("get_preferred_color"), &GDKSocialUser::get_preferred_color);

    ADD_PROPERTY(PropertyInfo(Variant::STRING, "xuid"), "", "get_xuid");
    ADD_PROPERTY(PropertyInfo(Variant::BOOL, "favorite"), "", "is_favorite");
    ADD_PROPERTY(PropertyInfo(Variant::BOOL, "friend"), "", "is_friend");
    ADD_PROPERTY(PropertyInfo(Variant::STRING, "display_name"), "", "get_display_name");
    ADD_PROPERTY(PropertyInfo(Variant::STRING, "real_name"), "", "get_real_name");
    ADD_PROPERTY(PropertyInfo(Variant::STRING, "display_picture_url"), "", "get_display_picture_url");
    ADD_PROPERTY(PropertyInfo(Variant::STRING, "gamerscore"), "", "get_gamerscore");
    ADD_PROPERTY(PropertyInfo(Variant::STRING, "gamertag"), "", "get_gamertag");
    ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "presence", PROPERTY_HINT_NONE, "", PROPERTY_USAGE_DEFAULT, "GDKPresenceRecord"), "", "get_presence");
    ADD_PROPERTY(PropertyInfo(Variant::DICTIONARY, "title_history"), "", "get_title_history");
    ADD_PROPERTY(PropertyInfo(Variant::DICTIONARY, "preferred_color"), "", "get_preferred_color");
}

String GDKSocialUser::get_xuid() const {
    return m_xuid;
}

bool GDKSocialUser::is_favorite() const {
    return m_is_favorite;
}

bool GDKSocialUser::is_friend() const {
    return m_is_friend;
}

bool GDKSocialUser::is_following_user() const {
    return m_is_following_user;
}

bool GDKSocialUser::is_followed_by_caller() const {
    return m_is_followed_by_caller;
}

String GDKSocialUser::get_display_name() const {
    return m_display_name;
}

String GDKSocialUser::get_real_name() const {
    return m_real_name;
}

String GDKSocialUser::get_display_picture_url() const {
    return m_display_picture_url;
}

bool GDKSocialUser::uses_avatar() const {
    return m_use_avatar;
}

String GDKSocialUser::get_gamerscore() const {
    return m_gamerscore;
}

String GDKSocialUser::get_gamertag() const {
    return m_gamertag;
}

String GDKSocialUser::get_modern_gamertag() const {
    return m_modern_gamertag;
}

String GDKSocialUser::get_modern_gamertag_suffix() const {
    return m_modern_gamertag_suffix;
}

String GDKSocialUser::get_unique_modern_gamertag() const {
    return m_unique_modern_gamertag;
}

Ref<GDKPresenceRecord> GDKSocialUser::get_presence() const {
    return m_presence;
}

Dictionary GDKSocialUser::get_title_history() const {
    return m_title_history;
}

Dictionary GDKSocialUser::get_preferred_color() const {
    return m_preferred_color;
}

void GDKSocialUser::populate_from_native(const XblSocialManagerUser &p_social_user) {
    m_xuid = String::num_uint64(p_social_user.xboxUserId);
    m_is_favorite = p_social_user.isFavorite;
    m_is_friend = p_social_user.isFriend;
    m_is_following_user = p_social_user.isFollowingUser;
    m_is_followed_by_caller = p_social_user.isFollowedByCaller;
    m_display_name = _utf8_or_empty(p_social_user.displayName);
    m_real_name = _utf8_or_empty(p_social_user.realName);
    m_display_picture_url = _utf8_or_empty(p_social_user.displayPicUrlRaw);
    m_use_avatar = p_social_user.useAvatar;
    m_gamerscore = _utf8_or_empty(p_social_user.gamerscore);
    m_gamertag = _utf8_or_empty(p_social_user.gamertag);
    m_modern_gamertag = _utf8_or_empty(p_social_user.modernGamertag);
    m_modern_gamertag_suffix = _utf8_or_empty(p_social_user.modernGamertagSuffix);
    m_unique_modern_gamertag = _utf8_or_empty(p_social_user.uniqueModernGamertag);
    m_title_history = _make_title_history_dictionary(p_social_user.titleHistory);
    m_preferred_color = _make_preferred_color_dictionary(p_social_user.preferredColor);

    if (m_presence.is_null()) {
        m_presence.instantiate();
    }
    m_presence->populate_from_social_manager_record(p_social_user.xboxUserId, p_social_user.presenceRecord);
}

void GDKSocial::_bind_methods() {
    ClassDB::bind_method(D_METHOD("start_social_graph", "user"), &GDKSocial::start_social_graph);
    ClassDB::bind_method(D_METHOD("stop_social_graph", "user"), &GDKSocial::stop_social_graph);
    ClassDB::bind_method(D_METHOD("get_friends_async", "user"), &GDKSocial::get_friends_async);
    ClassDB::bind_method(D_METHOD("create_social_group", "user", "filter"), &GDKSocial::create_social_group, DEFVAL(Ref<GDKSocialFilter>()));
    ClassDB::bind_method(D_METHOD("create_social_group_from_xuids", "user", "xuids"), &GDKSocial::create_social_group_from_xuids);
    ClassDB::bind_method(D_METHOD("destroy_social_group", "group"), &GDKSocial::destroy_social_group);
    ClassDB::bind_method(D_METHOD("get_group_users", "group"), &GDKSocial::get_group_users);

    ADD_SIGNAL(MethodInfo("social_graph_changed", PropertyInfo(Variant::OBJECT, "user", PROPERTY_HINT_NONE, "", PROPERTY_USAGE_DEFAULT, "GDKUser")));
    ADD_SIGNAL(MethodInfo("social_group_updated", PropertyInfo(Variant::OBJECT, "group", PROPERTY_HINT_NONE, "", PROPERTY_USAGE_DEFAULT, "GDKSocialGroup")));
    ADD_SIGNAL(MethodInfo("social_user_changed",
            PropertyInfo(Variant::STRING, "xuid"),
            PropertyInfo(Variant::OBJECT, "social_user", PROPERTY_HINT_NONE, "", PROPERTY_USAGE_DEFAULT, "GDKSocialUser")));
}

void GDKSocial::set_owner(GDK *p_owner) {
    m_owner = p_owner;
}

Ref<GDKResult> GDKSocial::on_runtime_initialized() {
    GDKRuntime *runtime = _get_runtime();
    if (runtime == nullptr || !runtime->is_initialized()) {
        return GDKResult::error_result(E_FAIL, "runtime_not_initialized", "Cannot initialize the social service before the GDK runtime.");
    }

    m_runtime_ready = true;
    return GDKResult::ok_result();
}

void GDKSocial::shutdown() {
    m_runtime_ready = false;

    Ref<GDKResult> cancelled = GDKResult::cancelled("Social operation cancelled during shutdown.");
    for (LocalUserState &state : m_local_user_states) {
        _fail_pending_friend_ops(state.local_id, cancelled);
    }

    std::vector<Ref<GDKSocialGroup>> groups = m_groups;
    for (const Ref<GDKSocialGroup> &group : groups) {
        _destroy_group_internal(group, false);
    }
    m_groups.clear();

    for (LocalUserState &state : m_local_user_states) {
        if (state.graph_started && state.user.is_valid() && state.user->get_handle() != nullptr) {
            XblSocialManagerRemoveLocalUser(state.user->get_handle());
        }
    }

    m_pending_friend_ops.clear();
    m_local_user_states.clear();
    m_cached_users.clear();
}

int GDKSocial::dispatch() {
    if (!m_runtime_ready) {
        return 0;
    }

    GDKXboxServices *xbox_services = _get_xbox_services();
    if (xbox_services == nullptr || !xbox_services->is_initialized()) {
        return 0;
    }

    const XblSocialManagerEvent *events = nullptr;
    size_t event_count = 0;
    HRESULT hr = XblSocialManagerDoWork(&events, &event_count);
    if (FAILED(hr)) {
        if (m_owner != nullptr) {
            m_owner->emit_runtime_error(GDKResult::hresult_error(
                    hr,
                    "Failed to dispatch Social Manager state.",
                    "social_manager_dispatch_failed"));
        }
        return 0;
    }

    int handled_events = 0;
    std::vector<uint64_t> graph_changed_users;
    std::vector<uint64_t> filter_group_updates;

    for (size_t i = 0; i < event_count; ++i) {
        const XblSocialManagerEvent &event = events[i];
        XUserLocalId local_id = {};
        if (event.user != nullptr) {
            XUserGetLocalId(event.user, &local_id);
        }

        LocalUserState *state = _find_local_user_state(local_id);
        ++handled_events;

        if (FAILED(event.hr)) {
            Ref<GDKResult> result = GDKResult::hresult_error(event.hr, "A Social Manager event failed.", "social_event_failed");
            if (m_owner != nullptr) {
                m_owner->emit_runtime_error(result);
            }
            if (event.eventType == XblSocialManagerEventType::LocalUserAdded) {
                _fail_pending_friend_ops(local_id, result);
            }
            continue;
        }

        switch (event.eventType) {
            case XblSocialManagerEventType::LocalUserAdded: {
                if (state != nullptr) {
                    state->graph_ready = true;
                    _append_unique_local_id(&graph_changed_users, local_id);
                }
            } break;
            case XblSocialManagerEventType::SocialUserGroupLoaded:
            case XblSocialManagerEventType::SocialUserGroupUpdated: {
                Ref<GDKSocialGroup> group = _find_group_by_handle(event.groupAffected);
                if (!group.is_valid()) {
                    continue;
                }

                group->set_loaded(true);
                Ref<GDKResult> refresh_result = _refresh_group_metadata(group);
                if (!refresh_result->is_ok()) {
                    _fail_pending_friend_ops(local_id, refresh_result);
                    if (m_owner != nullptr) {
                        m_owner->emit_runtime_error(refresh_result);
                    }
                    continue;
                }

                _get_group_users_internal(group, false, false);
                emit_signal("social_group_updated", group);
                _complete_pending_friend_ops(group->get_handle());
            } break;
            case XblSocialManagerEventType::UsersAddedToSocialGraph:
            case XblSocialManagerEventType::UsersRemovedFromSocialGraph:
            case XblSocialManagerEventType::PresenceChanged:
            case XblSocialManagerEventType::ProfilesChanged:
            case XblSocialManagerEventType::SocialRelationshipsChanged: {
                for (uint32_t user_index = 0; user_index < XBL_SOCIAL_MANAGER_MAX_AFFECTED_USERS_PER_EVENT; ++user_index) {
                    XblSocialManagerUser *affected_user = event.usersAffected[user_index];
                    if (affected_user == nullptr) {
                        continue;
                    }

                    const bool emit_presence_signal = event.eventType == XblSocialManagerEventType::PresenceChanged;
                    _cache_social_user(*affected_user, true, emit_presence_signal);
                }

                if (event.eventType == XblSocialManagerEventType::UsersAddedToSocialGraph ||
                        event.eventType == XblSocialManagerEventType::UsersRemovedFromSocialGraph ||
                        event.eventType == XblSocialManagerEventType::SocialRelationshipsChanged) {
                    _append_unique_local_id(&graph_changed_users, local_id);
                }
                if (event.eventType == XblSocialManagerEventType::UsersAddedToSocialGraph ||
                        event.eventType == XblSocialManagerEventType::UsersRemovedFromSocialGraph ||
                        event.eventType == XblSocialManagerEventType::PresenceChanged ||
                        event.eventType == XblSocialManagerEventType::SocialRelationshipsChanged) {
                    _append_unique_local_id(&filter_group_updates, local_id);
                }
            } break;
            case XblSocialManagerEventType::UnknownEvent:
            default:
                break;
        }
    }

    for (uint64_t local_id_value : filter_group_updates) {
        XUserLocalId local_id = {};
        local_id.value = local_id_value;
        _emit_filter_group_updates(local_id);
    }
    for (uint64_t local_id_value : graph_changed_users) {
        XUserLocalId local_id = {};
        local_id.value = local_id_value;
        _emit_social_graph_changed(local_id);
    }

    return handled_events;
}

Ref<GDKResult> GDKSocial::start_social_graph(const Ref<GDKUser> &p_user) {
    GDKRuntime *runtime = _get_runtime();
    if (runtime == nullptr || !runtime->is_initialized()) {
        return _make_social_error_result(E_FAIL, "not_initialized", "GDK is not initialized. Call GDK.initialize() first.");
    }
    if (!p_user.is_valid() || p_user->get_handle() == nullptr) {
        return _make_social_error_result(E_INVALIDARG, "invalid_user", "A signed-in GDKUser is required for social graph operations.");
    }

    GDKXboxServices *xbox_services = _get_xbox_services();
    if (xbox_services == nullptr || !xbox_services->is_initialized()) {
        return _make_social_error_result(E_FAIL, "xbox_services_not_initialized", "Xbox services are unavailable. Ensure the title has a TitleId before using social.");
    }

    XUserLocalId local_id = {};
    local_id.value = static_cast<uint64_t>(p_user->get_local_id());
    LocalUserState *existing_state = _find_local_user_state(local_id);
    if (existing_state != nullptr && existing_state->graph_started) {
        return GDKResult::ok_result();
    }

    const bool created_state = existing_state == nullptr;
    if (existing_state == nullptr) {
        LocalUserState state;
        state.user = p_user;
        state.local_id = local_id;
        m_local_user_states.push_back(state);
        existing_state = &m_local_user_states.back();
    }

    HRESULT hr = XblSocialManagerAddLocalUser(
            p_user->get_handle(),
            XblSocialManagerExtraDetailLevel::All,
            runtime->get_task_queue());
    if (FAILED(hr)) {
        if (created_state) {
            _erase_local_user_state(local_id);
        }
        return GDKResult::hresult_error(hr, "Failed to start the social graph.", "social_graph_start_failed");
    }

    existing_state->user = p_user;
    existing_state->graph_started = true;
    existing_state->graph_ready = false;
    return GDKResult::ok_result();
}

void GDKSocial::stop_social_graph(const Ref<GDKUser> &p_user) {
    if (!p_user.is_valid()) {
        return;
    }

    XUserLocalId local_id = {};
    local_id.value = static_cast<uint64_t>(p_user->get_local_id());
    LocalUserState *state = _find_local_user_state(local_id);
    if (state == nullptr) {
        return;
    }

    _fail_pending_friend_ops(local_id, GDKResult::cancelled("Friends query cancelled because the social graph stopped."));

    std::vector<Ref<GDKSocialGroup>> groups = m_groups;
    for (const Ref<GDKSocialGroup> &group : groups) {
        if (!group.is_valid()) {
            continue;
        }

        Ref<GDKUser> local_user = group->get_local_user();
        if (local_user.is_valid() && local_user->get_local_id() == p_user->get_local_id()) {
            _destroy_group_internal(group, true);
        }
    }

    if (state->graph_started && p_user->get_handle() != nullptr) {
        XblSocialManagerRemoveLocalUser(p_user->get_handle());
    }

    _erase_local_user_state(local_id);
}

Signal GDKSocial::get_friends_async(const Ref<GDKUser> &p_user) {
    LocalUserState *state = nullptr;
    Ref<GDKResult> ensure_result = _ensure_local_user_state(p_user, &state, true);
    if (!ensure_result->is_ok()) {
        return _make_error_signal(
                static_cast<HRESULT>(ensure_result->get_hresult()),
                ensure_result->get_code(),
                ensure_result->get_message());
    }

    if (!state->friends_group.is_valid()) {
        Ref<GDKSocialFilter> filter;
        filter.instantiate();
        filter->set_presence_filter(GDKSocialFilter::PRESENCE_FILTER_ALL);
        filter->set_relationship_filter(GDKSocialFilter::RELATIONSHIP_FILTER_FRIENDS);
        state->friends_group = create_social_group(state->user, filter);
        if (!state->friends_group.is_valid()) {
            return _make_error_signal(E_FAIL, "friends_group_create_failed", "Failed to create the default friends social group.");
        }
    }

    if (state->friends_group->is_loaded()) {
        return _make_completed_signal(GDKResult::ok_result(state->friends_group));
    }

    GDKRuntime *runtime = _get_runtime();
    Ref<GDKPendingSignal> pending_signal = runtime != nullptr ? runtime->make_pending_signal() : Ref<GDKPendingSignal>();
    ERR_FAIL_COND_V(pending_signal.is_null(), Signal());

    PendingFriendsOp pending_op;
    pending_op.local_id = state->local_id;
    pending_op.group_handle = state->friends_group->get_handle();
    pending_op.request = pending_signal;
    m_pending_friend_ops.push_back(pending_op);

    GDKPendingSignal *pending_signal_ptr = pending_signal.ptr();
    pending_signal->set_cancel_handler([this, pending_signal_ptr]() {
        _cancel_pending_friend_signal(pending_signal_ptr);
    });

    return pending_signal->get_completed_signal();
}

Ref<GDKSocialGroup> GDKSocial::create_social_group(const Ref<GDKUser> &p_user, const Ref<GDKSocialFilter> &p_filter) {
    LocalUserState *state = nullptr;
    Ref<GDKResult> ensure_result = _ensure_local_user_state(p_user, &state, true);
    if (!ensure_result->is_ok()) {
        if (m_owner != nullptr) {
            m_owner->emit_runtime_error(ensure_result);
        }
        return Ref<GDKSocialGroup>();
    }

    const GDKSocialFilter::PresenceFilter presence_filter = p_filter.is_valid() ? p_filter->get_presence_filter() : GDKSocialFilter::PRESENCE_FILTER_ALL;
    const GDKSocialFilter::RelationshipFilter relationship_filter = p_filter.is_valid() ? p_filter->get_relationship_filter() : GDKSocialFilter::RELATIONSHIP_FILTER_FRIENDS;

    XblSocialManagerUserGroupHandle group_handle = nullptr;
    HRESULT hr = XblSocialManagerCreateSocialUserGroupFromFilters(
            p_user->get_handle(),
            _presence_filter_to_native(presence_filter),
            _relationship_filter_to_native(relationship_filter),
            &group_handle);
    if (FAILED(hr)) {
        if (m_owner != nullptr) {
            m_owner->emit_runtime_error(GDKResult::hresult_error(
                    hr,
                    "Failed to create a filter-based social group.",
                    "social_group_create_failed"));
        }
        return Ref<GDKSocialGroup>();
    }

    Ref<GDKSocialGroup> group;
    group.instantiate();
    group->attach(p_user, group_handle);
    group->set_group_type(GDKSocialGroup::GROUP_TYPE_FILTER);
    group->set_filters(presence_filter, relationship_filter);
    m_groups.push_back(group);
    return group;
}

Ref<GDKSocialGroup> GDKSocial::create_social_group_from_xuids(const Ref<GDKUser> &p_user, const PackedStringArray &p_xuids) {
    LocalUserState *state = nullptr;
    Ref<GDKResult> ensure_result = _ensure_local_user_state(p_user, &state, true);
    if (!ensure_result->is_ok()) {
        if (m_owner != nullptr) {
            m_owner->emit_runtime_error(ensure_result);
        }
        return Ref<GDKSocialGroup>();
    }

    if (p_xuids.is_empty()) {
        if (m_owner != nullptr) {
            m_owner->emit_runtime_error(GDKResult::error_result(E_INVALIDARG, "missing_social_group_xuids", "Social list groups require at least one XUID."));
        }
        return Ref<GDKSocialGroup>();
    }
    if (p_xuids.size() > XBL_SOCIAL_MANAGER_MAX_USERS_FROM_LIST) {
        if (m_owner != nullptr) {
            m_owner->emit_runtime_error(GDKResult::error_result(E_INVALIDARG, "too_many_social_group_xuids", "Social list groups cannot exceed the XSAPI maximum tracked user count."));
        }
        return Ref<GDKSocialGroup>();
    }

    std::vector<uint64_t> native_xuids;
    native_xuids.reserve(static_cast<size_t>(p_xuids.size()));
    for (int64_t i = 0; i < p_xuids.size(); ++i) {
        uint64_t xuid = 0;
        if (!_try_parse_xuid(p_xuids[i], &xuid)) {
            if (m_owner != nullptr) {
                m_owner->emit_runtime_error(GDKResult::error_result(E_INVALIDARG, "invalid_social_group_xuid", "Social list groups require numeric XUID strings."));
            }
            return Ref<GDKSocialGroup>();
        }
        native_xuids.push_back(xuid);
    }

    XblSocialManagerUserGroupHandle group_handle = nullptr;
    HRESULT hr = XblSocialManagerCreateSocialUserGroupFromList(
            p_user->get_handle(),
            native_xuids.data(),
            native_xuids.size(),
            &group_handle);
    if (FAILED(hr)) {
        if (m_owner != nullptr) {
            m_owner->emit_runtime_error(GDKResult::hresult_error(
                    hr,
                    "Failed to create a list-based social group.",
                    "social_group_list_create_failed"));
        }
        return Ref<GDKSocialGroup>();
    }

    Ref<GDKSocialGroup> group;
    group.instantiate();
    group->attach(p_user, group_handle);
    group->set_group_type(GDKSocialGroup::GROUP_TYPE_USER_LIST);
    group->set_tracked_xuids(p_xuids);
    m_groups.push_back(group);
    return group;
}

void GDKSocial::destroy_social_group(const Ref<GDKSocialGroup> &p_group) {
    _destroy_group_internal(p_group, true);
}

Array GDKSocial::get_group_users(const Ref<GDKSocialGroup> &p_group) {
    return _get_group_users_internal(p_group, false, false);
}

void GDKSocial::on_user_removed(const Ref<GDKUser> &p_user) {
    stop_social_graph(p_user);
}

GDKRuntime *GDKSocial::_get_runtime() const {
    return m_owner != nullptr ? m_owner->get_runtime() : nullptr;
}

GDKXboxServices *GDKSocial::_get_xbox_services() const {
    return m_owner != nullptr ? m_owner->get_xbox_services() : nullptr;
}

GDKPresence *GDKSocial::_get_presence_service() const {
    if (m_owner == nullptr) {
        return nullptr;
    }

    Ref<GDKPresence> presence = m_owner->get_presence();
    return presence.ptr();
}

Signal GDKSocial::_make_completed_signal(const Ref<GDKResult> &p_result) const {
    GDKRuntime *runtime = _get_runtime();
    Ref<GDKPendingSignal> pending_signal = runtime != nullptr ? runtime->make_pending_signal() : Ref<GDKPendingSignal>();
    if (pending_signal.is_null()) {
        pending_signal.instantiate();
    }
    pending_signal->complete_deferred(p_result);
    return pending_signal->get_completed_signal();
}

Signal GDKSocial::_make_error_signal(HRESULT p_hresult, const String &p_code, const String &p_message) const {
    GDKRuntime *runtime = _get_runtime();
    if (runtime != nullptr) {
        return runtime->make_error_signal(p_hresult, p_code, p_message);
    }

    Ref<GDKPendingSignal> pending_signal;
    pending_signal.instantiate();
    pending_signal->complete_deferred(GDKResult::error_result(p_hresult, p_code, p_message));
    return pending_signal->get_completed_signal();
}

Ref<GDKResult> GDKSocial::_ensure_local_user_state(const Ref<GDKUser> &p_user, LocalUserState **r_state, bool p_auto_start) {
    ERR_FAIL_COND_V(r_state == nullptr, GDKResult::error_result(E_POINTER, "internal_error", "Missing social local-user output."));

    *r_state = nullptr;

    GDKRuntime *runtime = _get_runtime();
    if (runtime == nullptr || !runtime->is_initialized()) {
        return GDKResult::error_result(E_FAIL, "not_initialized", "GDK is not initialized. Call GDK.initialize() first.");
    }
    if (!p_user.is_valid() || p_user->get_handle() == nullptr) {
        return GDKResult::error_result(E_INVALIDARG, "invalid_user", "A signed-in GDKUser is required for social graph operations.");
    }

    XUserLocalId local_id = {};
    local_id.value = static_cast<uint64_t>(p_user->get_local_id());
    LocalUserState *state = _find_local_user_state(local_id);
    if (state == nullptr || !state->graph_started) {
        if (!p_auto_start) {
            return GDKResult::error_result(E_FAIL, "social_graph_not_started", "Start the social graph before using social groups.");
        }

        Ref<GDKResult> start_result = start_social_graph(p_user);
        if (!start_result->is_ok()) {
            return start_result;
        }
        state = _find_local_user_state(local_id);
    }

    if (state == nullptr) {
        return GDKResult::error_result(E_FAIL, "social_graph_state_missing", "The social graph state could not be created.");
    }

    *r_state = state;
    return GDKResult::ok_result();
}

GDKSocial::LocalUserState *GDKSocial::_find_local_user_state(XUserLocalId p_local_id) {
    for (LocalUserState &state : m_local_user_states) {
        if (state.local_id.value == p_local_id.value) {
            return &state;
        }
    }

    return nullptr;
}

Ref<GDKSocialGroup> GDKSocial::_find_group_by_handle(XblSocialManagerUserGroupHandle p_group_handle) const {
    for (const Ref<GDKSocialGroup> &group : m_groups) {
        if (group.is_valid() && group->matches_handle(p_group_handle)) {
            return group;
        }
    }

    return Ref<GDKSocialGroup>();
}

Ref<GDKSocialUser> GDKSocial::_find_cached_user(const String &p_xuid) const {
    for (const Ref<GDKSocialUser> &user : m_cached_users) {
        if (user.is_valid() && user->get_xuid() == p_xuid) {
            return user;
        }
    }

    return Ref<GDKSocialUser>();
}

Ref<GDKSocialUser> GDKSocial::_cache_social_user(const XblSocialManagerUser &p_social_user, bool p_emit_social_signal, bool p_emit_presence_signal) {
    const String xuid = String::num_uint64(p_social_user.xboxUserId);
    Ref<GDKSocialUser> social_user = _find_cached_user(xuid);
    if (social_user.is_null()) {
        social_user.instantiate();
        m_cached_users.push_back(social_user);
    }

    social_user->populate_from_native(p_social_user);

    GDKPresence *presence = _get_presence_service();
    if (presence != nullptr && social_user->get_presence().is_valid()) {
        presence->cache_presence_record(social_user->get_presence(), p_emit_presence_signal);
    }

    if (p_emit_social_signal) {
        emit_signal("social_user_changed", xuid, social_user);
    }

    return social_user;
}

Array GDKSocial::_get_group_users_internal(const Ref<GDKSocialGroup> &p_group, bool p_emit_social_signal, bool p_emit_presence_signal) {
    Array users;
    if (!p_group.is_valid() || p_group->get_handle() == nullptr || !p_group->is_loaded()) {
        return users;
    }

    XblSocialManagerUserPtrArray native_users = nullptr;
    size_t native_user_count = 0;
    HRESULT hr = XblSocialManagerUserGroupGetUsers(p_group->get_handle(), &native_users, &native_user_count);
    if (FAILED(hr)) {
        if (m_owner != nullptr) {
            m_owner->emit_runtime_error(GDKResult::hresult_error(
                    hr,
                    "Failed to read the users for a social group.",
                    "social_group_users_failed"));
        }
        return users;
    }

    for (size_t i = 0; i < native_user_count; ++i) {
        if (native_users[i] == nullptr) {
            continue;
        }
        users.push_back(_cache_social_user(*native_users[i], p_emit_social_signal, p_emit_presence_signal));
    }

    return users;
}

Ref<GDKResult> GDKSocial::_refresh_group_metadata(const Ref<GDKSocialGroup> &p_group) {
    ERR_FAIL_COND_V(!p_group.is_valid(), GDKResult::error_result(E_INVALIDARG, "invalid_social_group", "A valid GDKSocialGroup is required."));
    if (p_group->get_handle() == nullptr) {
        return GDKResult::error_result(E_FAIL, "social_group_invalidated", "The social group is no longer valid.");
    }

    XblSocialUserGroupType native_group_type = XblSocialUserGroupType::FilterType;
    HRESULT hr = XblSocialManagerUserGroupGetType(p_group->get_handle(), &native_group_type);
    if (FAILED(hr)) {
        return GDKResult::hresult_error(hr, "Failed to read the social group type.", "social_group_type_failed");
    }

    p_group->set_group_type(native_group_type == XblSocialUserGroupType::UserListType ? GDKSocialGroup::GROUP_TYPE_USER_LIST : GDKSocialGroup::GROUP_TYPE_FILTER);

    if (native_group_type == XblSocialUserGroupType::FilterType) {
        XblPresenceFilter native_presence_filter = XblPresenceFilter::Unknown;
        XblRelationshipFilter native_relationship_filter = XblRelationshipFilter::Unknown;
        hr = XblSocialManagerUserGroupGetFilters(p_group->get_handle(), &native_presence_filter, &native_relationship_filter);
        if (FAILED(hr)) {
            return GDKResult::hresult_error(hr, "Failed to read the social group filters.", "social_group_filters_failed");
        }

        p_group->set_filters(_presence_filter_from_native(native_presence_filter), _relationship_filter_from_native(native_relationship_filter));
    }

    const uint64_t *tracked_users = nullptr;
    size_t tracked_user_count = 0;
    hr = XblSocialManagerUserGroupGetUsersTrackedByGroup(p_group->get_handle(), &tracked_users, &tracked_user_count);
    if (FAILED(hr)) {
        return GDKResult::hresult_error(hr, "Failed to read the tracked social group users.", "social_group_tracked_users_failed");
    }

    PackedStringArray tracked_xuids;
    for (size_t i = 0; i < tracked_user_count; ++i) {
        tracked_xuids.push_back(String::num_uint64(tracked_users[i]));
    }
    p_group->set_tracked_xuids(tracked_xuids);
    p_group->set_loaded(true);
    return GDKResult::ok_result();
}

void GDKSocial::_complete_pending_friend_ops(XblSocialManagerUserGroupHandle p_group_handle) {
    for (auto it = m_pending_friend_ops.begin(); it != m_pending_friend_ops.end();) {
        if (!it->request.is_valid()) {
            it = m_pending_friend_ops.erase(it);
            continue;
        }
        if (it->group_handle != p_group_handle) {
            ++it;
            continue;
        }

        Ref<GDKSocialGroup> group = _find_group_by_handle(p_group_handle);
        it->request->complete(GDKResult::ok_result(group));
        it = m_pending_friend_ops.erase(it);
    }
}

void GDKSocial::_fail_pending_friend_ops(XUserLocalId p_local_id, const Ref<GDKResult> &p_result) {
    for (auto it = m_pending_friend_ops.begin(); it != m_pending_friend_ops.end();) {
        if (!it->request.is_valid()) {
            it = m_pending_friend_ops.erase(it);
            continue;
        }
        if (p_local_id.value != 0 && it->local_id.value != p_local_id.value) {
            ++it;
            continue;
        }

        it->request->complete(p_result);
        it = m_pending_friend_ops.erase(it);
    }
}

void GDKSocial::_fail_pending_friend_ops_for_group(XblSocialManagerUserGroupHandle p_group_handle, const Ref<GDKResult> &p_result) {
    if (p_group_handle == nullptr) {
        return;
    }

    for (auto it = m_pending_friend_ops.begin(); it != m_pending_friend_ops.end();) {
        if (!it->request.is_valid()) {
            it = m_pending_friend_ops.erase(it);
            continue;
        }
        if (it->group_handle != p_group_handle) {
            ++it;
            continue;
        }

        it->request->complete(p_result);
        it = m_pending_friend_ops.erase(it);
    }
}

void GDKSocial::_cancel_pending_friend_signal(GDKPendingSignal *p_request) {
    if (p_request == nullptr) {
        return;
    }

    for (auto it = m_pending_friend_ops.begin(); it != m_pending_friend_ops.end(); ++it) {
        if (it->request.is_null() || it->request.ptr() != p_request) {
            continue;
        }

        Ref<GDKPendingSignal> pending_signal = it->request;
        m_pending_friend_ops.erase(it);
        if (pending_signal.is_valid()) {
            pending_signal->clear_cancel_handler();
            pending_signal->complete(GDKResult::cancelled("Friends query cancelled."));
        }
        return;
    }
}

void GDKSocial::_destroy_group_internal(const Ref<GDKSocialGroup> &p_group, bool p_remove_from_collection) {
    if (!p_group.is_valid()) {
        return;
    }

    XblSocialManagerUserGroupHandle group_handle = p_group->get_handle();
    if (group_handle != nullptr) {
        _fail_pending_friend_ops_for_group(group_handle, GDKResult::cancelled("Friends query cancelled because the social group was destroyed."));
        XblSocialManagerDestroySocialUserGroup(group_handle);
    }
    p_group->invalidate();

    if (p_remove_from_collection) {
        m_groups.erase(
                std::remove_if(
                        m_groups.begin(),
                        m_groups.end(),
                        [&p_group](const Ref<GDKSocialGroup> &group) {
                            return group.is_null() || group == p_group;
                        }),
                m_groups.end());
    }

    for (LocalUserState &state : m_local_user_states) {
        if (state.friends_group == p_group) {
            state.friends_group.unref();
        }
    }
}

void GDKSocial::_erase_local_user_state(XUserLocalId p_local_id) {
    m_local_user_states.erase(
            std::remove_if(
                    m_local_user_states.begin(),
                    m_local_user_states.end(),
                    [p_local_id](const LocalUserState &state) {
                        return state.local_id.value == p_local_id.value;
                    }),
            m_local_user_states.end());
}

void GDKSocial::_emit_filter_group_updates(XUserLocalId p_local_id) {
    std::vector<Ref<GDKSocialGroup>> groups = m_groups;
    for (const Ref<GDKSocialGroup> &group : groups) {
        if (!group.is_valid() || !group->is_loaded() || group->get_handle() == nullptr) {
            continue;
        }

        Ref<GDKUser> local_user = group->get_local_user();
        if (!local_user.is_valid() || local_user->get_local_id() != static_cast<int64_t>(p_local_id.value)) {
            continue;
        }
        if (group->get_group_type() != GDKSocialGroup::GROUP_TYPE_FILTER) {
            continue;
        }

        Ref<GDKResult> refresh_result = _refresh_group_metadata(group);
        if (!refresh_result->is_ok()) {
            if (m_owner != nullptr) {
                m_owner->emit_runtime_error(refresh_result);
            }
            continue;
        }

        emit_signal("social_group_updated", group);
    }
}

void GDKSocial::_emit_social_graph_changed(XUserLocalId p_local_id) {
    LocalUserState *state = _find_local_user_state(p_local_id);
    if (state != nullptr && state->user.is_valid()) {
        emit_signal("social_graph_changed", state->user);
    }
}

} // namespace godot
