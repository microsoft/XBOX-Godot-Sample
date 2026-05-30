extends "res://addons/godot_gdk_tests/playfab_test_base.gd"
## Wave 3 migration of the previous `tests/suites/core_suite.gd`.
##
## Covers PlayFab singleton/class registration and the root API contract:
## method/signal exposure, project-setting registration, and the
## `initialize()` failure path when `playfab/runtime/title_id` is blank.

const PLAYFAB_ROOT_METHODS := [
	"initialize",
	"shutdown",
	"is_available",
	"is_initialized",
	"dispatch",
	"get_users",
	"get_game_saves",
	"get_leaderboards",
	"get_multiplayer",
	"get_party",
	"get_accounts",
	"get_catalog",
	"get_cloud_script",
	"get_entity_data",
	"get_events",
	"get_experimentation",
	"get_friends",
	"get_groups",
	"get_inventory",
	"get_localization",
	"get_player_data",
	"get_statistics",
	"get_title_data",
	"get_title_id",
	"get_endpoint",
]

const PLAYFAB_ROOT_SIGNALS := ["initialized", "shutdown_completed"]
const REARM_TEST_TITLE_ID := "00000"

const REGISTERED_CLASSES := [
	"PlayFab",
	"PlayFabUsers",
	"PlayFabUser",
	"PlayFabGameSaves",
	"PlayFabLeaderboards",
	"PlayFabMultiplayer",
	"PlayFabMultiplayerConfig",
	"PlayFabLobbyConfig",
	"PlayFabLobbyJoinConfig",
	"PlayFabLobbySearchConfig",
	"PlayFabLobby",
	"PlayFabLobbyMember",
	"PlayFabLobbyInvite",
	"PlayFabLobbySummary",
	"PlayFabLobbySearchResult",
	"PlayFabLobbyStateChange",
	"PlayFabMatchmakingMember",
	"PlayFabMatchmakingTicketConfig",
	"PlayFabMatchTicket",
	"PlayFabMatchTicketStateChange",
	"PlayFabMultiplayerStateChange",
	"PlayFabParty",
	"PlayFabPartyConfig",
	"PlayFabPartyTextMessageConfig",
	"PlayFabPartyMember",
	"PlayFabPartyChatMessage",
	"PlayFabPartyChatStateChange",
	"PlayFabPartyChatControl",
	"PlayFabPartyChat",
	"PlayFabPartyNetworkStateChange",
	"PlayFabPartyNetwork",
	"PlayFabPartyPeer",
	"PlayFabAccounts",
	"PlayFabCatalog",
	"PlayFabCloudScript",
	"PlayFabEntityData",
	"PlayFabEvents",
	"PlayFabExperimentation",
	"PlayFabFriends",
	"PlayFabGroups",
	"PlayFabInventory",
	"PlayFabLocalization",
	"PlayFabPlayerData",
	"PlayFabStatistics",
	"PlayFabTitleData",
	"PlayFabResult",
]


func _restore_playfab_runtime_settings(original_title_id, original_endpoint) -> void:
	ProjectSettings.set_setting(PLAYFAB_TITLE_ID_SETTING, original_title_id)
	ProjectSettings.set_setting(PLAYFAB_ENDPOINT_SETTING, original_endpoint)


func _disconnect_playfab_lifecycle_handlers(playfab, initialized_handler: Callable, shutdown_handler: Callable) -> void:
	if playfab.initialized.is_connected(initialized_handler):
		playfab.initialized.disconnect(initialized_handler)
	if playfab.shutdown_completed.is_connected(shutdown_handler):
		playfab.shutdown_completed.disconnect(shutdown_handler)


func test_singleton_availability() -> void:
	assert_not_null(get_playfab(), "Engine.get_singleton('PlayFab')")


func test_class_registration() -> void:
	for registered_class in REGISTERED_CLASSES:
		assert_true(ClassDB.class_exists(registered_class), "%s registered in ClassDB" % registered_class)

	assert_true(ClassDB.is_parent_class("PlayFab", "Object"), "PlayFab extends Object")
	assert_true(ClassDB.is_parent_class("PlayFabUsers", "RefCounted"), "PlayFabUsers extends RefCounted")
	assert_true(ClassDB.is_parent_class("PlayFabUser", "RefCounted"), "PlayFabUser extends RefCounted")
	assert_true(ClassDB.is_parent_class("PlayFabGameSaves", "RefCounted"), "PlayFabGameSaves extends RefCounted")
	assert_true(ClassDB.is_parent_class("PlayFabLeaderboards", "RefCounted"), "PlayFabLeaderboards extends RefCounted")
	assert_true(ClassDB.is_parent_class("PlayFabMultiplayer", "RefCounted"), "PlayFabMultiplayer extends RefCounted")
	assert_true(ClassDB.is_parent_class("PlayFabParty", "RefCounted"), "PlayFabParty extends RefCounted")
	assert_true(ClassDB.is_parent_class("PlayFabPartyPeer", "MultiplayerPeerExtension"), "PlayFabPartyPeer extends MultiplayerPeerExtension")
	assert_true(ClassDB.is_parent_class("PlayFabLobby", "RefCounted"), "PlayFabLobby extends RefCounted")
	assert_true(ClassDB.is_parent_class("PlayFabMatchTicket", "RefCounted"), "PlayFabMatchTicket extends RefCounted")
	assert_true(ClassDB.is_parent_class("PlayFabAccounts", "RefCounted"), "PlayFabAccounts extends RefCounted")
	assert_true(ClassDB.is_parent_class("PlayFabCatalog", "RefCounted"), "PlayFabCatalog extends RefCounted")
	assert_true(ClassDB.is_parent_class("PlayFabCloudScript", "RefCounted"), "PlayFabCloudScript extends RefCounted")
	assert_true(ClassDB.is_parent_class("PlayFabEntityData", "RefCounted"), "PlayFabEntityData extends RefCounted")
	assert_true(ClassDB.is_parent_class("PlayFabEvents", "RefCounted"), "PlayFabEvents extends RefCounted")
	assert_true(ClassDB.is_parent_class("PlayFabExperimentation", "RefCounted"), "PlayFabExperimentation extends RefCounted")
	assert_true(ClassDB.is_parent_class("PlayFabFriends", "RefCounted"), "PlayFabFriends extends RefCounted")
	assert_true(ClassDB.is_parent_class("PlayFabGroups", "RefCounted"), "PlayFabGroups extends RefCounted")
	assert_true(ClassDB.is_parent_class("PlayFabInventory", "RefCounted"), "PlayFabInventory extends RefCounted")
	assert_true(ClassDB.is_parent_class("PlayFabLocalization", "RefCounted"), "PlayFabLocalization extends RefCounted")
	assert_true(ClassDB.is_parent_class("PlayFabPlayerData", "RefCounted"), "PlayFabPlayerData extends RefCounted")
	assert_true(ClassDB.is_parent_class("PlayFabStatistics", "RefCounted"), "PlayFabStatistics extends RefCounted")
	assert_true(ClassDB.is_parent_class("PlayFabTitleData", "RefCounted"), "PlayFabTitleData extends RefCounted")
	assert_true(ClassDB.is_parent_class("PlayFabResult", "RefCounted"), "PlayFabResult extends RefCounted")


func test_root_api_methods_and_signals() -> void:
	if pending_unless_playfab_available():
		return
	var playfab = get_playfab()

	for method_name in PLAYFAB_ROOT_METHODS:
		assert_has_method_named(playfab, method_name)

	for signal_name in PLAYFAB_ROOT_SIGNALS:
		assert_has_signal_named(playfab, signal_name)


func test_root_api_object_accessors() -> void:
	if pending_unless_playfab_available():
		return
	var playfab = get_playfab()

	reset_playfab_runtime()

	assert_object_is(playfab.get_users(), "PlayFabUsers", "PlayFab.users returns PlayFabUsers")
	assert_object_is(playfab.get_game_saves(), "PlayFabGameSaves", "PlayFab.game_saves returns PlayFabGameSaves")
	assert_object_is(playfab.get_leaderboards(), "PlayFabLeaderboards", "PlayFab.leaderboards returns PlayFabLeaderboards")
	assert_object_is(playfab.get_multiplayer(), "PlayFabMultiplayer", "PlayFab.multiplayer returns PlayFabMultiplayer")
	assert_object_is(playfab.get_party(), "PlayFabParty", "PlayFab.party returns PlayFabParty")
	assert_object_is(playfab.get_accounts(), "PlayFabAccounts", "PlayFab.accounts returns PlayFabAccounts")
	assert_object_is(playfab.get_catalog(), "PlayFabCatalog", "PlayFab.catalog returns PlayFabCatalog")
	assert_object_is(playfab.get_cloud_script(), "PlayFabCloudScript", "PlayFab.cloud_script returns PlayFabCloudScript")
	assert_object_is(playfab.get_entity_data(), "PlayFabEntityData", "PlayFab.entity_data returns PlayFabEntityData")
	assert_object_is(playfab.get_events(), "PlayFabEvents", "PlayFab.events returns PlayFabEvents")
	assert_object_is(playfab.get_experimentation(), "PlayFabExperimentation", "PlayFab.experimentation returns PlayFabExperimentation")
	assert_object_is(playfab.get_friends(), "PlayFabFriends", "PlayFab.friends returns PlayFabFriends")
	assert_object_is(playfab.get_groups(), "PlayFabGroups", "PlayFab.groups returns PlayFabGroups")
	assert_object_is(playfab.get_inventory(), "PlayFabInventory", "PlayFab.inventory returns PlayFabInventory")
	assert_object_is(playfab.get_localization(), "PlayFabLocalization", "PlayFab.localization returns PlayFabLocalization")
	assert_object_is(playfab.get_player_data(), "PlayFabPlayerData", "PlayFab.player_data returns PlayFabPlayerData")
	assert_object_is(playfab.get_statistics(), "PlayFabStatistics", "PlayFab.statistics returns PlayFabStatistics")
	assert_object_is(playfab.get_title_data(), "PlayFabTitleData", "PlayFab.title_data returns PlayFabTitleData")


func test_root_api_initial_state() -> void:
	if pending_unless_playfab_available():
		return
	var playfab = get_playfab()

	reset_playfab_runtime()

	assert_true(playfab.is_available() is bool, "PlayFab.is_available() returns bool")
	assert_eq(playfab.is_initialized(), false, "PlayFab.is_initialized() starts false")
	assert_eq(playfab.dispatch(), 0, "PlayFab.dispatch() is safe before init")
	assert_eq(playfab.get_title_id(), "", "PlayFab.get_title_id() is empty before init")
	assert_eq(playfab.get_endpoint(), "", "PlayFab.get_endpoint() is empty before init")


func test_project_settings_registration() -> void:
	assert_true(ProjectSettings.has_setting(PLAYFAB_TITLE_ID_SETTING), "playfab/runtime/title_id project setting registered")
	assert_eq(String(get_setting_default(PLAYFAB_TITLE_ID_SETTING)), "", "playfab/runtime/title_id default remains blank")
	assert_true(ProjectSettings.has_setting(PLAYFAB_ENDPOINT_SETTING), "playfab/runtime/endpoint project setting registered")
	assert_eq(String(get_setting_default(PLAYFAB_ENDPOINT_SETTING)), "", "playfab/runtime/endpoint default remains blank")
	assert_true(ProjectSettings.has_setting(PLAYFAB_EMBED_DISPATCH_SETTING), "playfab/runtime/embed_dispatch project setting registered")
	assert_eq(bool(get_setting_default(PLAYFAB_EMBED_DISPATCH_SETTING)), true, "playfab/runtime/embed_dispatch defaults to true")
	assert_true(ProjectSettings.has_setting(PLAYFAB_INITIALIZE_ON_STARTUP_SETTING), "playfab/runtime/initialize_on_startup project setting registered")
	assert_eq(bool(get_setting_default(PLAYFAB_INITIALIZE_ON_STARTUP_SETTING)), false, "playfab/runtime/initialize_on_startup defaults to false")


func test_initialize_rejects_blank_title_id() -> void:
	if pending_unless_playfab_available():
		return
	var playfab = get_playfab()

	reset_playfab_runtime()

	var initialized_events: Array = []
	var initialized_handler := func() -> void:
		initialized_events.append(true)

	playfab.initialized.connect(initialized_handler)

	var original_title_id = ProjectSettings.get_setting(PLAYFAB_TITLE_ID_SETTING, "")
	var original_endpoint = ProjectSettings.get_setting(PLAYFAB_ENDPOINT_SETTING, "")
	ProjectSettings.set_setting(PLAYFAB_TITLE_ID_SETTING, "")
	ProjectSettings.set_setting(PLAYFAB_ENDPOINT_SETTING, "")

	var init_result = playfab.initialize()

	ProjectSettings.set_setting(PLAYFAB_TITLE_ID_SETTING, original_title_id)
	ProjectSettings.set_setting(PLAYFAB_ENDPOINT_SETTING, original_endpoint)

	assert_playfab_result_error(init_result, "title_id_required", "PlayFab.initialize() rejects blank playfab/runtime/title_id")
	assert_eq(initialized_events.size(), 0, "PlayFab.initialized is not emitted for blank title id")

	if playfab.initialized.is_connected(initialized_handler):
		playfab.initialized.disconnect(initialized_handler)


func test_initialize_shutdown_rearms_runtime_lifecycle() -> void:
	if pending_unless_playfab_available():
		return
	var playfab = get_playfab()

	reset_playfab_runtime()

	var original_title_id = ProjectSettings.get_setting(PLAYFAB_TITLE_ID_SETTING, "")
	var original_endpoint = ProjectSettings.get_setting(PLAYFAB_ENDPOINT_SETTING, "")
	ProjectSettings.set_setting(PLAYFAB_TITLE_ID_SETTING, REARM_TEST_TITLE_ID)
	ProjectSettings.set_setting(PLAYFAB_ENDPOINT_SETTING, "")

	var initialized_events: Array = []
	var shutdown_events: Array = []
	var initialized_handler := func() -> void:
		initialized_events.append(true)
	var shutdown_handler := func() -> void:
		shutdown_events.append(true)

	playfab.initialized.connect(initialized_handler)
	playfab.shutdown_completed.connect(shutdown_handler)

	var lifecycle_failed := false
	for cycle_index in range(3):
		var init_result = playfab.initialize()
		if init_result == null:
			assert_not_null(init_result, "PlayFab.initialize() returns a result on cycle %d" % (cycle_index + 1))
			lifecycle_failed = true
			break
		if not init_result.is_ok():
			assert_playfab_result_ok(init_result, "PlayFab.initialize() re-arms after shutdown on cycle %d" % (cycle_index + 1))
			lifecycle_failed = true
			break

		assert_true(playfab.is_initialized(), "PlayFab is initialized on cycle %d" % (cycle_index + 1))
		if cycle_index < 2:
			playfab.shutdown()
			assert_false(playfab.is_initialized(), "PlayFab shuts down cleanly on cycle %d" % (cycle_index + 1))

	if not lifecycle_failed:
		assert_eq(initialized_events.size(), 3, "PlayFab.initialized emits for each re-arm initialize")
		assert_eq(shutdown_events.size(), 2, "PlayFab.shutdown_completed emits for the two explicit shutdowns before the third initialize")
		assert_true(playfab.is_initialized(), "PlayFab remains initialized after the third re-arm initialize")

	if playfab.is_initialized():
		playfab.shutdown()
	_restore_playfab_runtime_settings(original_title_id, original_endpoint)
	reset_playfab_runtime()
	_disconnect_playfab_lifecycle_handlers(playfab, initialized_handler, shutdown_handler)
