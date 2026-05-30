#include "playfab.h"

#include "playfab_gamesaves.h"
#include "playfab_leaderboards.h"
#include "playfab_multiplayer.h"
#include "playfab_party.h"
#include "playfab_result.h"
#include "playfab_runtime.h"
#include "playfab_user.h"
#include "playfab_users.h"
#include "api/playfab_api_services.h"

namespace godot {

PlayFab *PlayFab::singleton = nullptr;

PlayFab *PlayFab::get_singleton() {
    return singleton;
}

PlayFab::PlayFab() {
    ERR_FAIL_COND(singleton != nullptr);
    singleton = this;

    m_runtime = new PlayFabRuntime();

    m_users.instantiate();
    m_users->set_owner(this);

    m_game_saves.instantiate();
    m_game_saves->set_owner(this);

    m_leaderboards.instantiate();
    m_leaderboards->set_owner(this);

    m_multiplayer.instantiate();
    m_multiplayer->set_owner(this);

    m_party.instantiate();
    m_party->set_owner(this);

    m_accounts.instantiate();
    m_accounts->set_owner(this);
    m_catalog.instantiate();
    m_catalog->set_owner(this);
    m_cloud_script.instantiate();
    m_cloud_script->set_owner(this);
    m_entity_data.instantiate();
    m_entity_data->set_owner(this);
    m_events.instantiate();
    m_events->set_owner(this);
    m_experimentation.instantiate();
    m_experimentation->set_owner(this);
    m_friends.instantiate();
    m_friends->set_owner(this);
    m_groups.instantiate();
    m_groups->set_owner(this);
    m_inventory.instantiate();
    m_inventory->set_owner(this);
    m_localization.instantiate();
    m_localization->set_owner(this);
    m_player_data.instantiate();
    m_player_data->set_owner(this);
    m_statistics.instantiate();
    m_statistics->set_owner(this);
    m_title_data.instantiate();
    m_title_data->set_owner(this);
}

PlayFab::~PlayFab() {
    m_destroying = true;
    shutdown();

    // If shutdown was deferred because Party/Multiplayer were mid-batch when the
    // singleton tore down, drain their state changes here BEFORE tearing down
    // the shared services (m_users entity tokens, m_runtime queues, XGameRuntime).
    // Without this drain, queued Party/Multiplayer callbacks may dereference
    // invalidated entity handles or runtime task-queue state during their own
    // destructor-driven cleanup. We cap the wait to avoid hanging the editor
    // tear-down on a wedged SDK callback.
    if (m_shutdown_deferred_until_services_complete) {
        constexpr int kMaxDrainAttempts = 200; // ~1 second total at 5ms apart
        int drain_attempt = 0;
        while (m_shutdown_deferred_until_services_complete && drain_attempt < kMaxDrainAttempts) {
            if (m_multiplayer.is_valid()) {
                m_multiplayer->dispatch();
            }
            if (m_party.is_valid()) {
                m_party->dispatch();
            }
            // Retry shutdown to let subsystems re-evaluate now that pending
            // state-change batches have had a chance to complete.
            if (m_party.is_valid()) {
                m_party->shutdown();
            }
            if (m_multiplayer.is_valid()) {
                m_multiplayer->shutdown();
            }
            finish_deferred_shutdown_if_ready();
            ++drain_attempt;
        }
        if (m_shutdown_deferred_until_services_complete) {
            WARN_PRINT("PlayFab destructor: Party/Multiplayer shutdown did not drain in time; proceeding with service teardown anyway.");
        }
    }

    if (m_party.is_valid()) {
        m_party->set_owner(nullptr);
    }
    if (m_multiplayer.is_valid()) {
        m_multiplayer->set_owner(nullptr);
    }
    m_shutdown_deferred_until_services_complete = false;
    if (m_runtime != nullptr && m_runtime->is_initialized()) {
        _finish_shutdown_after_services(false);
    }
    m_shutdown_in_progress = false;

    m_users.unref();
    m_game_saves.unref();
    m_leaderboards.unref();
    m_multiplayer.unref();
    m_party.unref();
    m_accounts.unref();
    m_catalog.unref();
    m_cloud_script.unref();
    m_entity_data.unref();
    m_events.unref();
    m_experimentation.unref();
    m_friends.unref();
    m_groups.unref();
    m_inventory.unref();
    m_localization.unref();
    m_player_data.unref();
    m_statistics.unref();
    m_title_data.unref();

    if (m_runtime != nullptr) {
        delete m_runtime;
        m_runtime = nullptr;
    }

    singleton = nullptr;
}

void PlayFab::_bind_methods() {
    ClassDB::bind_method(D_METHOD("initialize"), &PlayFab::initialize);
    ClassDB::bind_method(D_METHOD("shutdown"), &PlayFab::shutdown);
    ClassDB::bind_method(D_METHOD("is_available"), &PlayFab::is_available);
    ClassDB::bind_method(D_METHOD("is_initialized"), &PlayFab::is_initialized);
    ClassDB::bind_method(D_METHOD("dispatch"), &PlayFab::dispatch);
    ClassDB::bind_method(D_METHOD("get_users"), &PlayFab::get_users);
    ClassDB::bind_method(D_METHOD("get_game_saves"), &PlayFab::get_game_saves);
    ClassDB::bind_method(D_METHOD("get_leaderboards"), &PlayFab::get_leaderboards);
    ClassDB::bind_method(D_METHOD("get_multiplayer"), &PlayFab::get_multiplayer);
    ClassDB::bind_method(D_METHOD("get_party"), &PlayFab::get_party);
    ClassDB::bind_method(D_METHOD("get_accounts"), &PlayFab::get_accounts);
    ClassDB::bind_method(D_METHOD("get_catalog"), &PlayFab::get_catalog);
    ClassDB::bind_method(D_METHOD("get_cloud_script"), &PlayFab::get_cloud_script);
    ClassDB::bind_method(D_METHOD("get_entity_data"), &PlayFab::get_entity_data);
    ClassDB::bind_method(D_METHOD("get_events"), &PlayFab::get_events);
    ClassDB::bind_method(D_METHOD("get_experimentation"), &PlayFab::get_experimentation);
    ClassDB::bind_method(D_METHOD("get_friends"), &PlayFab::get_friends);
    ClassDB::bind_method(D_METHOD("get_groups"), &PlayFab::get_groups);
    ClassDB::bind_method(D_METHOD("get_inventory"), &PlayFab::get_inventory);
    ClassDB::bind_method(D_METHOD("get_localization"), &PlayFab::get_localization);
    ClassDB::bind_method(D_METHOD("get_player_data"), &PlayFab::get_player_data);
    ClassDB::bind_method(D_METHOD("get_statistics"), &PlayFab::get_statistics);
    ClassDB::bind_method(D_METHOD("get_title_data"), &PlayFab::get_title_data);
    ClassDB::bind_method(D_METHOD("get_title_id"), &PlayFab::get_title_id);
    ClassDB::bind_method(D_METHOD("get_endpoint"), &PlayFab::get_endpoint);

    ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "users", PROPERTY_HINT_NONE, "", PROPERTY_USAGE_SCRIPT_VARIABLE, "PlayFabUsers"), "", "get_users");
    ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "game_saves", PROPERTY_HINT_NONE, "", PROPERTY_USAGE_SCRIPT_VARIABLE, "PlayFabGameSaves"), "", "get_game_saves");
    ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "leaderboards", PROPERTY_HINT_NONE, "", PROPERTY_USAGE_SCRIPT_VARIABLE, "PlayFabLeaderboards"), "", "get_leaderboards");
    ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "multiplayer", PROPERTY_HINT_NONE, "", PROPERTY_USAGE_SCRIPT_VARIABLE, "PlayFabMultiplayer"), "", "get_multiplayer");
    ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "party", PROPERTY_HINT_NONE, "", PROPERTY_USAGE_SCRIPT_VARIABLE, "PlayFabParty"), "", "get_party");
    ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "accounts", PROPERTY_HINT_NONE, "", PROPERTY_USAGE_SCRIPT_VARIABLE, "PlayFabAccounts"), "", "get_accounts");
    ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "catalog", PROPERTY_HINT_NONE, "", PROPERTY_USAGE_SCRIPT_VARIABLE, "PlayFabCatalog"), "", "get_catalog");
    ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "cloud_script", PROPERTY_HINT_NONE, "", PROPERTY_USAGE_SCRIPT_VARIABLE, "PlayFabCloudScript"), "", "get_cloud_script");
    ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "entity_data", PROPERTY_HINT_NONE, "", PROPERTY_USAGE_SCRIPT_VARIABLE, "PlayFabEntityData"), "", "get_entity_data");
    ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "events", PROPERTY_HINT_NONE, "", PROPERTY_USAGE_SCRIPT_VARIABLE, "PlayFabEvents"), "", "get_events");
    ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "experimentation", PROPERTY_HINT_NONE, "", PROPERTY_USAGE_SCRIPT_VARIABLE, "PlayFabExperimentation"), "", "get_experimentation");
    ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "friends", PROPERTY_HINT_NONE, "", PROPERTY_USAGE_SCRIPT_VARIABLE, "PlayFabFriends"), "", "get_friends");
    ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "groups", PROPERTY_HINT_NONE, "", PROPERTY_USAGE_SCRIPT_VARIABLE, "PlayFabGroups"), "", "get_groups");
    ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "inventory", PROPERTY_HINT_NONE, "", PROPERTY_USAGE_SCRIPT_VARIABLE, "PlayFabInventory"), "", "get_inventory");
    ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "localization", PROPERTY_HINT_NONE, "", PROPERTY_USAGE_SCRIPT_VARIABLE, "PlayFabLocalization"), "", "get_localization");
    ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "player_data", PROPERTY_HINT_NONE, "", PROPERTY_USAGE_SCRIPT_VARIABLE, "PlayFabPlayerData"), "", "get_player_data");
    ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "statistics", PROPERTY_HINT_NONE, "", PROPERTY_USAGE_SCRIPT_VARIABLE, "PlayFabStatistics"), "", "get_statistics");
    ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "title_data", PROPERTY_HINT_NONE, "", PROPERTY_USAGE_SCRIPT_VARIABLE, "PlayFabTitleData"), "", "get_title_data");
    ADD_PROPERTY(PropertyInfo(Variant::STRING, "title_id"), "", "get_title_id");
    ADD_PROPERTY(PropertyInfo(Variant::STRING, "endpoint"), "", "get_endpoint");

    ADD_SIGNAL(MethodInfo("initialized"));
    ADD_SIGNAL(MethodInfo("shutdown_completed"));
}

Ref<PlayFabResult> PlayFab::initialize() {
    Ref<PlayFabResult> runtime_result = m_runtime->initialize();
    if (!runtime_result->is_ok()) {
        return runtime_result;
    }

    Ref<PlayFabResult> users_result = m_users->on_runtime_initialized();
    if (!users_result->is_ok()) {
        m_users->shutdown();
        m_runtime->shutdown();
        return users_result;
    }

    emit_signal("initialized");
    return PlayFabResult::ok_result();
}

bool PlayFab::_has_deferred_service_shutdown() const {
    return (m_party.is_valid() && m_party->has_deferred_shutdown()) ||
            (m_multiplayer.is_valid() && m_multiplayer->has_deferred_shutdown());
}

void PlayFab::_finish_shutdown_after_services(bool p_emit_signal) {
    if (m_users.is_valid()) {
        m_users->shutdown();
    }
    if (m_runtime != nullptr) {
        m_runtime->shutdown();
    }
    if (p_emit_signal && !m_destroying) {
        emit_signal("shutdown_completed");
    }
}

void PlayFab::finish_deferred_shutdown_if_ready() {
    if (!m_shutdown_deferred_until_services_complete) {
        return;
    }
    if (_has_deferred_service_shutdown()) {
        return;
    }
    m_shutdown_deferred_until_services_complete = false;
    m_shutdown_in_progress = false;
    _finish_shutdown_after_services(true);
}

void PlayFab::shutdown() {
    if (m_runtime == nullptr) {
        return;
    }
    if (m_shutdown_deferred_until_services_complete) {
        finish_deferred_shutdown_if_ready();
        return;
    }
    if (m_shutdown_in_progress) {
        return;
    }

    const bool was_initialized = m_runtime->is_initialized();
    if (!was_initialized) {
        m_runtime->shutdown();
        return;
    }

    m_shutdown_in_progress = true;

    if (m_party.is_valid()) {
        m_party->shutdown();
    }
    if (m_multiplayer.is_valid()) {
        m_multiplayer->shutdown();
    }

    if (_has_deferred_service_shutdown()) {
        m_shutdown_deferred_until_services_complete = true;
        return;
    }

    m_shutdown_in_progress = false;
    _finish_shutdown_after_services(true);
}

bool PlayFab::is_available() const {
    return m_runtime != nullptr && m_runtime->is_available();
}

bool PlayFab::is_initialized() const {
    return m_runtime != nullptr && m_runtime->is_initialized();
}

int64_t PlayFab::dispatch() {
    int64_t dispatched = m_runtime != nullptr ? static_cast<int64_t>(m_runtime->dispatch()) : 0;
    if (m_multiplayer.is_valid()) {
        dispatched += static_cast<int64_t>(m_multiplayer->dispatch());
    }
    if (m_party.is_valid()) {
        dispatched += static_cast<int64_t>(m_party->dispatch());
    }
    return dispatched;
}

Ref<PlayFabUsers> PlayFab::get_users() const {
    return m_users;
}

Ref<PlayFabGameSaves> PlayFab::get_game_saves() const {
    return m_game_saves;
}

Ref<PlayFabLeaderboards> PlayFab::get_leaderboards() const {
    return m_leaderboards;
}

Ref<PlayFabMultiplayer> PlayFab::get_multiplayer() const {
    return m_multiplayer;
}

Ref<PlayFabParty> PlayFab::get_party() const {
    return m_party;
}

Ref<PlayFabAccounts> PlayFab::get_accounts() const {
    return m_accounts;
}

Ref<PlayFabCatalog> PlayFab::get_catalog() const {
    return m_catalog;
}

Ref<PlayFabCloudScript> PlayFab::get_cloud_script() const {
    return m_cloud_script;
}

Ref<PlayFabEntityData> PlayFab::get_entity_data() const {
    return m_entity_data;
}

Ref<PlayFabEvents> PlayFab::get_events() const {
    return m_events;
}

Ref<PlayFabExperimentation> PlayFab::get_experimentation() const {
    return m_experimentation;
}

Ref<PlayFabFriends> PlayFab::get_friends() const {
    return m_friends;
}

Ref<PlayFabGroups> PlayFab::get_groups() const {
    return m_groups;
}

Ref<PlayFabInventory> PlayFab::get_inventory() const {
    return m_inventory;
}

Ref<PlayFabLocalization> PlayFab::get_localization() const {
    return m_localization;
}

Ref<PlayFabPlayerData> PlayFab::get_player_data() const {
    return m_player_data;
}

Ref<PlayFabStatistics> PlayFab::get_statistics() const {
    return m_statistics;
}

Ref<PlayFabTitleData> PlayFab::get_title_data() const {
    return m_title_data;
}

String PlayFab::get_title_id() const {
    return m_runtime != nullptr ? m_runtime->get_title_id() : String();
}

String PlayFab::get_endpoint() const {
    return m_runtime != nullptr ? m_runtime->get_endpoint() : String();
}

PlayFabRuntime *PlayFab::get_runtime() const {
    return m_runtime;
}

} // namespace godot
