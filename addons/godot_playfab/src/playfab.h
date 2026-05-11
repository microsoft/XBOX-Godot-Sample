#ifndef GODOT_PLAYFAB_H
#define GODOT_PLAYFAB_H

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>

#include <godot_cpp/classes/object.hpp>
#include <godot_cpp/classes/ref.hpp>
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/variant.hpp>
#include <godot_cpp/variant/string.hpp>

namespace godot {

class PlayFabGameSaves;
class PlayFabLeaderboards;
class PlayFabResult;
class PlayFabRuntime;
class PlayFabUser;
class PlayFabUsers;
class PlayFabAccounts;
class PlayFabCatalog;
class PlayFabCloudScript;
class PlayFabEntityData;
class PlayFabEvents;
class PlayFabExperimentation;
class PlayFabFriends;
class PlayFabGroups;
class PlayFabInventory;
class PlayFabLocalization;
class PlayFabPlayerData;
class PlayFabStatistics;
class PlayFabTitleData;

class PlayFab : public Object {
    GDCLASS(PlayFab, Object);

    static PlayFab *singleton;

    PlayFabRuntime *m_runtime = nullptr;
    Ref<PlayFabUsers> m_users;
    Ref<PlayFabGameSaves> m_game_saves;
    Ref<PlayFabLeaderboards> m_leaderboards;
    Ref<PlayFabAccounts> m_accounts;
    Ref<PlayFabCatalog> m_catalog;
    Ref<PlayFabCloudScript> m_cloud_script;
    Ref<PlayFabEntityData> m_entity_data;
    Ref<PlayFabEvents> m_events;
    Ref<PlayFabExperimentation> m_experimentation;
    Ref<PlayFabFriends> m_friends;
    Ref<PlayFabGroups> m_groups;
    Ref<PlayFabInventory> m_inventory;
    Ref<PlayFabLocalization> m_localization;
    Ref<PlayFabPlayerData> m_player_data;
    Ref<PlayFabStatistics> m_statistics;
    Ref<PlayFabTitleData> m_title_data;

protected:
    static void _bind_methods();

public:
    static PlayFab *get_singleton();

    PlayFab();
    ~PlayFab();

    Ref<PlayFabResult> initialize();
    void shutdown();
    bool is_available() const;
    bool is_initialized() const;
    int64_t dispatch();
    Ref<PlayFabResult> get_last_error() const;
    Ref<PlayFabUsers> get_users() const;
    Ref<PlayFabGameSaves> get_game_saves() const;
    Ref<PlayFabLeaderboards> get_leaderboards() const;
    Ref<PlayFabAccounts> get_accounts() const;
    Ref<PlayFabCatalog> get_catalog() const;
    Ref<PlayFabCloudScript> get_cloud_script() const;
    Ref<PlayFabEntityData> get_entity_data() const;
    Ref<PlayFabEvents> get_events() const;
    Ref<PlayFabExperimentation> get_experimentation() const;
    Ref<PlayFabFriends> get_friends() const;
    Ref<PlayFabGroups> get_groups() const;
    Ref<PlayFabInventory> get_inventory() const;
    Ref<PlayFabLocalization> get_localization() const;
    Ref<PlayFabPlayerData> get_player_data() const;
    Ref<PlayFabStatistics> get_statistics() const;
    Ref<PlayFabTitleData> get_title_data() const;
    Signal sign_in_with_xuser_async(const Variant &p_user, bool p_create_account = true);
    Signal sign_in_with_custom_id_async(const String &p_custom_id, bool p_create_account = true);
    Ref<PlayFabUser> get_user_by_local_id(int64_t p_local_id) const;
    Ref<PlayFabUser> get_user_by_custom_id(const String &p_custom_id) const;
    String get_title_id() const;
    String get_endpoint() const;

    PlayFabRuntime *get_runtime() const;
    void emit_runtime_error(const Ref<PlayFabResult> &p_result);
};

} // namespace godot

#endif // GODOT_PLAYFAB_H
