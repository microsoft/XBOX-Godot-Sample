#include "register_types.h"

#include <gdextension_interface.h>
#include <godot_cpp/classes/engine.hpp>
#include <godot_cpp/classes/project_settings.hpp>
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/godot.hpp>

#include "playfab.h"
#include "playfab_gamesaves.h"
#include "playfab_leaderboards.h"
#include "playfab_multiplayer.h"
#include "playfab_party.h"
#include "playfab_pending_signal.h"
#include "playfab_result.h"
#include "playfab_user.h"
#include "playfab_users.h"
#include "api/playfab_api_services.h"

using namespace godot;

static PlayFab *playfab_singleton = nullptr;
static int playfab_extension_ref_count = 0;

namespace {

constexpr const char *PLAYFAB_RUNTIME_EMBED_DISPATCH_SETTING = "playfab/runtime/embed_dispatch";
constexpr bool PLAYFAB_RUNTIME_EMBED_DISPATCH_DEFAULT = true;
constexpr const char *PLAYFAB_TITLE_ID_SETTING = "playfab/titleid";
constexpr const char *PLAYFAB_TITLE_ID_DEFAULT = "";
constexpr const char *PLAYFAB_ENDPOINT_SETTING = "playfab/endpoint";
constexpr const char *PLAYFAB_ENDPOINT_DEFAULT = "";
constexpr const char *PLAYFAB_LEADERBOARD_SETTLE_MSEC_SETTING = "playfab/tests/leaderboard_settle_msec";
constexpr int PLAYFAB_LEADERBOARD_SETTLE_MSEC_DEFAULT = 30000;

void register_string_project_setting(ProjectSettings *p_project_settings, const char *p_name, const String &p_default_value) {
    if (!p_project_settings->has_setting(p_name)) {
        p_project_settings->set_setting(p_name, p_default_value);
    }

    p_project_settings->set_initial_value(p_name, p_default_value);
    p_project_settings->set_as_basic(p_name, true);

    Dictionary setting_info;
    setting_info["name"] = p_name;
    setting_info["type"] = Variant::STRING;
    setting_info["hint"] = PROPERTY_HINT_NONE;
    setting_info["hint_string"] = "";
    p_project_settings->add_property_info(setting_info);
}

void register_playfab_project_settings() {
    ProjectSettings *project_settings = ProjectSettings::get_singleton();
    if (project_settings == nullptr) {
        return;
    }

    if (!project_settings->has_setting(PLAYFAB_RUNTIME_EMBED_DISPATCH_SETTING)) {
        project_settings->set_setting(PLAYFAB_RUNTIME_EMBED_DISPATCH_SETTING, PLAYFAB_RUNTIME_EMBED_DISPATCH_DEFAULT);
    }

    project_settings->set_initial_value(PLAYFAB_RUNTIME_EMBED_DISPATCH_SETTING, PLAYFAB_RUNTIME_EMBED_DISPATCH_DEFAULT);
    project_settings->set_as_basic(PLAYFAB_RUNTIME_EMBED_DISPATCH_SETTING, true);

    Dictionary setting_info;
    setting_info["name"] = PLAYFAB_RUNTIME_EMBED_DISPATCH_SETTING;
    setting_info["type"] = Variant::BOOL;
    setting_info["hint"] = PROPERTY_HINT_NONE;
    setting_info["hint_string"] = "";
    project_settings->add_property_info(setting_info);

    register_string_project_setting(project_settings, PLAYFAB_TITLE_ID_SETTING, PLAYFAB_TITLE_ID_DEFAULT);
    register_string_project_setting(project_settings, PLAYFAB_ENDPOINT_SETTING, PLAYFAB_ENDPOINT_DEFAULT);

    {
        if (!project_settings->has_setting(PLAYFAB_LEADERBOARD_SETTLE_MSEC_SETTING)) {
            project_settings->set_setting(PLAYFAB_LEADERBOARD_SETTLE_MSEC_SETTING, PLAYFAB_LEADERBOARD_SETTLE_MSEC_DEFAULT);
        }
        project_settings->set_initial_value(PLAYFAB_LEADERBOARD_SETTLE_MSEC_SETTING, PLAYFAB_LEADERBOARD_SETTLE_MSEC_DEFAULT);
        project_settings->set_as_basic(PLAYFAB_LEADERBOARD_SETTLE_MSEC_SETTING, true);

        Dictionary settle_info;
        settle_info["name"] = PLAYFAB_LEADERBOARD_SETTLE_MSEC_SETTING;
        settle_info["type"] = Variant::INT;
        settle_info["hint"] = PROPERTY_HINT_RANGE;
        settle_info["hint_string"] = "0,600000,1,or_greater";
        project_settings->add_property_info(settle_info);
    }
}

bool is_embed_dispatch_enabled() {
    ProjectSettings *project_settings = ProjectSettings::get_singleton();
    if (project_settings == nullptr) {
        return PLAYFAB_RUNTIME_EMBED_DISPATCH_DEFAULT;
    }

    return static_cast<bool>(project_settings->get_setting(
            PLAYFAB_RUNTIME_EMBED_DISPATCH_SETTING,
            PLAYFAB_RUNTIME_EMBED_DISPATCH_DEFAULT));
}

#if GODOT_VERSION_MINOR >= 5
void playfab_frame_callback() {
    if (playfab_singleton == nullptr || !playfab_singleton->is_initialized() || !is_embed_dispatch_enabled()) {
        return;
    }

    playfab_singleton->dispatch();
}
#endif

} // namespace

void initialize_playfab_extension(ModuleInitializationLevel p_level) {
    if (p_level != MODULE_INITIALIZATION_LEVEL_SCENE) {
        return;
    }

    ++playfab_extension_ref_count;
    if (playfab_extension_ref_count > 1) {
        return;
    }

    ClassDB::register_abstract_class<PlayFab>();
    ClassDB::register_class<PlayFabResult>();
    ClassDB::register_internal_class<PlayFabPendingSignal>();
    ClassDB::register_class<PlayFabUser>();
    ClassDB::register_class<PlayFabUsers>();
    ClassDB::register_class<PlayFabGameSaves>();
    ClassDB::register_class<PlayFabLeaderboards>();
    ClassDB::register_class<PlayFabAccounts>();
    ClassDB::register_class<PlayFabCatalog>();
    ClassDB::register_class<PlayFabCloudScript>();
    ClassDB::register_class<PlayFabEntityData>();
    ClassDB::register_class<PlayFabEvents>();
    ClassDB::register_class<PlayFabExperimentation>();
    ClassDB::register_class<PlayFabFriends>();
    ClassDB::register_class<PlayFabGroups>();
    ClassDB::register_class<PlayFabInventory>();
    ClassDB::register_class<PlayFabLocalization>();
    ClassDB::register_class<PlayFabPlayerData>();
    ClassDB::register_class<PlayFabStatistics>();
    ClassDB::register_class<PlayFabTitleData>();
    ClassDB::register_class<PlayFabMultiplayerConfig>();
    ClassDB::register_class<PlayFabLobbyConfig>();
    ClassDB::register_class<PlayFabLobbyJoinConfig>();
    ClassDB::register_class<PlayFabLobbySearchConfig>();
    ClassDB::register_class<PlayFabLobbyMember>();
    ClassDB::register_class<PlayFabLobbyInvite>();
    ClassDB::register_class<PlayFabLobbySummary>();
    ClassDB::register_class<PlayFabLobbySearchResult>();
    ClassDB::register_class<PlayFabLobbyStateChange>();
    ClassDB::register_class<PlayFabLobby>();
    ClassDB::register_class<PlayFabMatchmakingMember>();
    ClassDB::register_class<PlayFabMatchmakingTicketConfig>();
    ClassDB::register_class<PlayFabMatchTicket>();
    ClassDB::register_class<PlayFabMatchTicketStateChange>();
    ClassDB::register_class<PlayFabMultiplayerStateChange>();
    ClassDB::register_class<PlayFabMultiplayer>();
    ClassDB::register_class<PlayFabPartyConfig>();
    ClassDB::register_class<PlayFabPartyTextMessageConfig>();
    ClassDB::register_class<PlayFabPartyMember>();
    ClassDB::register_class<PlayFabPartyChatMessage>();
    ClassDB::register_class<PlayFabPartyChatStateChange>();
    ClassDB::register_class<PlayFabPartyChatControl>();
    ClassDB::register_class<PlayFabPartyChat>();
    ClassDB::register_class<PlayFabPartyNetworkStateChange>();
    ClassDB::register_class<PlayFabPartyNetwork>();
    ClassDB::register_class<PlayFabPartyPeer>();
    ClassDB::register_class<PlayFabParty>();

    playfab_singleton = memnew(PlayFab);
    Engine::get_singleton()->register_singleton("PlayFab", PlayFab::get_singleton());
    register_playfab_project_settings();
}

void uninitialize_playfab_extension(ModuleInitializationLevel p_level) {
    if (p_level != MODULE_INITIALIZATION_LEVEL_SCENE || playfab_extension_ref_count == 0) {
        return;
    }

    --playfab_extension_ref_count;
    if (playfab_extension_ref_count > 0) {
        return;
    }

    Engine::get_singleton()->unregister_singleton("PlayFab");

    if (playfab_singleton != nullptr) {
        memdelete(playfab_singleton);
        playfab_singleton = nullptr;
    }
}

extern "C" {

GDExtensionBool GDE_EXPORT playfab_addon_init(
        GDExtensionInterfaceGetProcAddress p_get_proc_address,
        GDExtensionClassLibraryPtr p_library,
        GDExtensionInitialization *r_initialization) {
    GDExtensionBinding::InitObject init_obj(p_get_proc_address, p_library, r_initialization);

    init_obj.register_initializer(initialize_playfab_extension);
    init_obj.register_terminator(uninitialize_playfab_extension);
#if GODOT_VERSION_MINOR >= 5
    init_obj.register_frame_callback(playfab_frame_callback);
#endif
    init_obj.set_minimum_library_initialization_level(MODULE_INITIALIZATION_LEVEL_SCENE);

    return init_obj.init();
}

} // extern "C"
