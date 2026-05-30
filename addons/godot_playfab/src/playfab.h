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
class PlayFabMultiplayer;
class PlayFabParty;
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
    Ref<PlayFabMultiplayer> m_multiplayer;
    Ref<PlayFabParty> m_party;
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
    bool m_shutdown_deferred_until_services_complete = false;
    bool m_shutdown_in_progress = false;
    bool m_destroying = false;

    bool _has_deferred_service_shutdown() const;
    void _finish_shutdown_after_services(bool p_emit_signal);

protected:
    static void _bind_methods();

public:
    static PlayFab *get_singleton();

    PlayFab();
    ~PlayFab();

    Ref<PlayFabResult> initialize();
    void shutdown();
    void finish_deferred_shutdown_if_ready();
    bool is_available() const;
    bool is_initialized() const;
    int64_t dispatch();
    Ref<PlayFabUsers> get_users() const;
    Ref<PlayFabGameSaves> get_game_saves() const;
    Ref<PlayFabLeaderboards> get_leaderboards() const;
    Ref<PlayFabMultiplayer> get_multiplayer() const;
    Ref<PlayFabParty> get_party() const;
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
    String get_title_id() const;
    String get_endpoint() const;

    PlayFabRuntime *get_runtime() const;
};

} // namespace godot

#endif // GODOT_PLAYFAB_H
