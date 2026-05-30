extends "res://addons/godot_gdk_tests/playfab_test_base.gd"
## PlayFab Multiplayer lobby/matchmaking public contract coverage.
##
## These tests are intentionally non-live: they validate registration,
## object shape, constants, and immediate failure paths without creating
## service-side lobbies or matchmaking tickets.


func test_multiplayer_service_contract() -> void:
	if pending_unless_playfab_available():
		return

	var playfab = get_playfab()
	reset_playfab_runtime()

	var multiplayer = playfab.get_multiplayer()
	assert_object_is(multiplayer, "PlayFabMultiplayer", "PlayFab.get_multiplayer() returns PlayFabMultiplayer")
	if multiplayer == null:
		return

	for method_name in [
		"is_initialized",
		"initialize_async",
		"shutdown_async",
		"create_lobby_async",
		"join_lobby_async",
		"join_arranged_lobby_async",
		"find_lobbies_async",
		"create_match_ticket_async",
		"get_lobbies",
		"get_lobby",
		"get_match_tickets",
	]:
		assert_has_method_named(multiplayer, method_name)
	for lobby_method_name in ["set_lobby_properties_async", "set_member_properties_async", "leave_lobby_async"]:
		assert_false(multiplayer.has_method(lobby_method_name), "PlayFabMultiplayer does not expose %s; use PlayFabLobby methods" % lobby_method_name)
	for ticket_method_name in ["cancel_match_ticket_async", "get_match_ticket_async"]:
		assert_false(multiplayer.has_method(ticket_method_name), "PlayFabMultiplayer does not expose %s; use PlayFabMatchTicket methods" % ticket_method_name)

	for signal_name in ["state_changed", "invite_received", "multiplayer_error"]:
		assert_has_signal_named(multiplayer, signal_name)

	# All PlayFabLobby state-change kinds are part of the public contract — the
	# values are referenced from sample/tutorial_app/autoload/lobby.gd and from
	# the doc_classes XML. Regressions in either direction (renames or value
	# shifts) silently break listener match statements.
	assert_eq(get_class_constant("PlayFabLobby", "MEMBER_ADDED"), 1, "PlayFabLobby.MEMBER_ADDED constant is stable")
	assert_eq(get_class_constant("PlayFabLobby", "MEMBER_REMOVED"), 2, "PlayFabLobby.MEMBER_REMOVED constant is stable")
	assert_eq(get_class_constant("PlayFabLobby", "MEMBER_UPDATED"), 3, "PlayFabLobby.MEMBER_UPDATED constant is stable")
	assert_eq(get_class_constant("PlayFabLobby", "PROPERTIES_UPDATED"), 4, "PlayFabLobby.PROPERTIES_UPDATED constant is stable")
	assert_eq(get_class_constant("PlayFabLobby", "OWNER_CHANGED"), 5, "PlayFabLobby.OWNER_CHANGED constant is stable")
	assert_eq(get_class_constant("PlayFabLobby", "DISCONNECTED"), 6, "PlayFabLobby.DISCONNECTED constant is stable")
	assert_eq(get_class_constant("PlayFabMatchTicket", "CREATED"), 100, "PlayFabMatchTicket.CREATED constant is stable")
	assert_eq(get_class_constant("PlayFabMatchTicket", "STATUS_CHANGED"), 101, "PlayFabMatchTicket.STATUS_CHANGED constant is stable")
	assert_eq(get_class_constant("PlayFabMatchTicket", "COMPLETED"), 102, "PlayFabMatchTicket.COMPLETED constant is stable")
	assert_eq(get_class_constant("PlayFabMatchTicket", "CANCELLED"), 103, "PlayFabMatchTicket.CANCELLED constant is stable")
	assert_eq(get_class_constant("PlayFabMatchTicket", "FAILED"), 104, "PlayFabMatchTicket.FAILED constant is stable")

	# State-change payload shape is part of the public contract too — listeners
	# read change.kind / change.lobby / change.member / change.result directly.
	var lobby_change = instantiate_class("PlayFabLobbyStateChange")
	if lobby_change != null:
		for getter in ["get_kind", "get_lobby", "get_result", "get_member", "get_invite", "get_user", "get_properties"]:
			assert_has_method_named(lobby_change, getter)
	var ticket_change = instantiate_class("PlayFabMatchTicketStateChange")
	if ticket_change != null:
		for getter in ["get_kind", "get_ticket", "get_result", "get_status", "get_match_id", "get_arranged_lobby_connection_string"]:
			assert_has_method_named(ticket_change, getter)
	var service_change = instantiate_class("PlayFabMultiplayerStateChange")
	if service_change != null:
		for getter in ["get_kind", "get_lobby", "get_ticket", "get_result", "get_properties"]:
			assert_has_method_named(service_change, getter)

	# PlayFabLobby exposes state_changed so sample listeners can attach to a
	# per-lobby firehose; removing that signal would silently break member
	# event delivery to anything that wires up via lobby.state_changed.
	var detached_lobby = instantiate_class("PlayFabLobby")
	if detached_lobby != null:
		assert_has_signal_named(detached_lobby, "state_changed")
		assert_has_method_named(detached_lobby, "is_owner")
		assert_false(detached_lobby.is_owner(null), "Detached PlayFabLobby.is_owner(null) returns false")
	assert_false(multiplayer.is_initialized(), "PlayFab.multiplayer starts uninitialized")
	assert_eq(multiplayer.get_lobbies().size(), 0, "PlayFab.multiplayer starts with no tracked lobbies")
	assert_eq(multiplayer.get_match_tickets().size(), 0, "PlayFab.multiplayer starts with no tracked tickets")


func test_multiplayer_config_and_wrapper_contract() -> void:
	var lobby_config = instantiate_class("PlayFabLobbyConfig")
	assert_object_is(lobby_config, "PlayFabLobbyConfig", "PlayFabLobbyConfig can be instantiated")
	if lobby_config != null:
		assert_eq(lobby_config.max_players, 8, "PlayFabLobbyConfig.max_players default")
		assert_eq(lobby_config.access_policy, get_class_constant("PlayFabLobbyConfig", "ACCESS_POLICY_PRIVATE"), "PlayFabLobbyConfig.access_policy default")
		assert_eq(lobby_config.owner_migration_policy, get_class_constant("PlayFabLobbyConfig", "OWNER_MIGRATION_AUTOMATIC"), "PlayFabLobbyConfig.owner_migration_policy default")
		assert_false(lobby_config.restrict_invites_to_lobby_owner, "PlayFabLobbyConfig.restrict_invites_to_lobby_owner default")
		lobby_config.max_players = 4
		lobby_config.access_policy = get_class_constant("PlayFabLobbyConfig", "ACCESS_POLICY_PUBLIC")
		lobby_config.owner_migration_policy = get_class_constant("PlayFabLobbyConfig", "OWNER_MIGRATION_MANUAL")
		lobby_config.restrict_invites_to_lobby_owner = true
		lobby_config.search_properties = {"string_key1": "contract"}
		lobby_config.lobby_properties = {"map": "arena"}
		lobby_config.member_properties = {"display_name": "tester"}
		assert_eq(lobby_config.max_players, 4, "PlayFabLobbyConfig.max_players setter")
		assert_eq(lobby_config.owner_migration_policy, get_class_constant("PlayFabLobbyConfig", "OWNER_MIGRATION_MANUAL"), "PlayFabLobbyConfig.owner_migration_policy setter")
		assert_true(lobby_config.restrict_invites_to_lobby_owner, "PlayFabLobbyConfig.restrict_invites_to_lobby_owner setter")
		assert_eq(lobby_config.search_properties.get("string_key1"), "contract", "PlayFabLobbyConfig.search_properties setter")

	var join_config = instantiate_class("PlayFabLobbyJoinConfig")
	assert_object_is(join_config, "PlayFabLobbyJoinConfig", "PlayFabLobbyJoinConfig can be instantiated")
	if join_config != null:
		join_config.member_properties = {"display_name": "joiner"}
		assert_eq(join_config.member_properties.get("display_name"), "joiner", "PlayFabLobbyJoinConfig.member_properties setter")

	var search_config = instantiate_class("PlayFabLobbySearchConfig")
	assert_object_is(search_config, "PlayFabLobbySearchConfig", "PlayFabLobbySearchConfig can be instantiated")
	if search_config != null:
		search_config.filter = "string_key1 eq 'contract'"
		search_config.order_by = "memberCount asc"
		search_config.max_results = 5
		assert_eq(search_config.filter, "string_key1 eq 'contract'", "PlayFabLobbySearchConfig.filter setter")
		assert_eq(search_config.max_results, 5, "PlayFabLobbySearchConfig.max_results setter")

	var matchmaking_member = instantiate_class("PlayFabMatchmakingMember")
	assert_object_is(matchmaking_member, "PlayFabMatchmakingMember", "PlayFabMatchmakingMember can be instantiated")
	if matchmaking_member != null:
		matchmaking_member.attributes = {"skill": 12, "region": "westus"}
		assert_eq(int(matchmaking_member.attributes.get("skill", 0)), 12, "PlayFabMatchmakingMember.attributes setter")

	var ticket_config = instantiate_class("PlayFabMatchmakingTicketConfig")
	assert_object_is(ticket_config, "PlayFabMatchmakingTicketConfig", "PlayFabMatchmakingTicketConfig can be instantiated")
	if ticket_config != null:
		ticket_config.queue_name = "default"
		ticket_config.timeout_seconds = 90
		ticket_config.members = [matchmaking_member]
		assert_eq(ticket_config.queue_name, "default", "PlayFabMatchmakingTicketConfig.queue_name setter")
		assert_eq(ticket_config.timeout_seconds, 90, "PlayFabMatchmakingTicketConfig.timeout_seconds setter")
		assert_eq(ticket_config.members.size(), 1, "PlayFabMatchmakingTicketConfig.members setter")

	for wrapper_class in [
		"PlayFabLobbyMember",
		"PlayFabLobbyInvite",
		"PlayFabLobbySummary",
		"PlayFabLobbySearchResult",
		"PlayFabLobbyStateChange",
		"PlayFabMatchTicketStateChange",
		"PlayFabMultiplayerStateChange",
	]:
		assert_object_is(instantiate_class(wrapper_class), wrapper_class, "%s can be instantiated" % wrapper_class)


func test_multiplayer_not_initialized_failures() -> void:
	if pending_unless_playfab_available():
		return

	var playfab = get_playfab()
	reset_playfab_runtime()
	var multiplayer = playfab.get_multiplayer()
	var blank_user = instantiate_class("PlayFabUser")
	var lobby_config = instantiate_class("PlayFabLobbyConfig")
	var join_config = instantiate_class("PlayFabLobbyJoinConfig")
	var search_config = instantiate_class("PlayFabLobbySearchConfig")
	var ticket_config = instantiate_class("PlayFabMatchmakingTicketConfig")

	await _assert_signal_error(multiplayer.initialize_async(), "not_initialized", "PlayFab.multiplayer.initialize_async() before PlayFab.initialize()")
	await _assert_signal_error(multiplayer.create_lobby_async(blank_user, lobby_config), "not_initialized", "PlayFab.multiplayer.create_lobby_async() before multiplayer init")
	await _assert_signal_error(multiplayer.join_lobby_async(blank_user, "connection-string", join_config), "not_initialized", "PlayFab.multiplayer.join_lobby_async() before multiplayer init")
	await _assert_signal_error(multiplayer.join_arranged_lobby_async(blank_user, "arranged-connection-string", join_config), "not_initialized", "PlayFab.multiplayer.join_arranged_lobby_async() before multiplayer init")
	await _assert_signal_error(multiplayer.find_lobbies_async(blank_user, search_config), "not_initialized", "PlayFab.multiplayer.find_lobbies_async() before multiplayer init")
	await _assert_signal_error(multiplayer.create_match_ticket_async(blank_user, ticket_config), "not_initialized", "PlayFab.multiplayer.create_match_ticket_async() before multiplayer init")

	var detached_lobby = instantiate_class("PlayFabLobby")
	if detached_lobby != null:
		assert_has_method_named(detached_lobby, "set_properties_async")
		assert_has_method_named(detached_lobby, "set_member_properties_async")
		assert_has_method_named(detached_lobby, "leave_async")
		await _assert_signal_error(detached_lobby.set_properties_async({"map": "arena"}), "invalid_lobby", "Detached PlayFabLobby.set_properties_async() reports invalid_lobby")
		await _assert_signal_error(detached_lobby.set_member_properties_async({"ready": "true"}), "invalid_lobby", "Detached PlayFabLobby.set_member_properties_async() reports invalid_lobby")
		await _assert_signal_error(detached_lobby.leave_async(), "invalid_lobby", "Detached PlayFabLobby.leave_async() reports invalid_lobby")

	var detached_ticket = instantiate_class("PlayFabMatchTicket")
	if detached_ticket != null:
		assert_has_method_named(detached_ticket, "refresh_async")
		assert_has_method_named(detached_ticket, "cancel_async")
		await _assert_signal_error(detached_ticket.refresh_async(), "invalid_match_ticket", "Detached PlayFabMatchTicket.refresh_async() reports invalid_match_ticket")
		await _assert_signal_error(detached_ticket.cancel_async(), "invalid_match_ticket", "Detached PlayFabMatchTicket.cancel_async() reports invalid_match_ticket")


func test_multiplayer_shutdown_async_explicit_await_uninitialized() -> void:
	if pending_unless_playfab_available():
		return

	var playfab = get_playfab()
	reset_playfab_runtime()
	var multiplayer = playfab.get_multiplayer()
	if multiplayer == null:
		return

	assert_playfab_result_ok(await await_completion(multiplayer.shutdown_async()), "await PlayFab.multiplayer.shutdown_async() while uninitialized")
	assert_false(multiplayer.is_initialized(), "PlayFab.multiplayer remains uninitialized after explicit awaited shutdown")


func test_multiplayer_initialize_reports_already_initialized() -> void:
	if pending_unless_playfab_available():
		return

	var playfab = get_playfab()
	reset_playfab_runtime()
	var multiplayer = playfab.get_multiplayer()
	if multiplayer == null:
		return

	var original_title_id = ProjectSettings.get_setting(PLAYFAB_TITLE_ID_SETTING, "")
	var original_endpoint = ProjectSettings.get_setting(PLAYFAB_ENDPOINT_SETTING, "")
	ProjectSettings.set_setting(PLAYFAB_TITLE_ID_SETTING, "00000")
	ProjectSettings.set_setting(PLAYFAB_ENDPOINT_SETTING, "")

	var init_result = playfab.initialize()
	if init_result == null or not init_result.ok:
		pending("PlayFab.multiplayer already_initialized branch skipped: PlayFab.initialize() failed: %s" % (init_result.message if init_result != null else "null"))
		ProjectSettings.set_setting(PLAYFAB_TITLE_ID_SETTING, original_title_id)
		ProjectSettings.set_setting(PLAYFAB_ENDPOINT_SETTING, original_endpoint)
		reset_playfab_runtime()
		return

	var first_mp_init = await await_completion(multiplayer.initialize_async())
	if first_mp_init == null or not first_mp_init.ok:
		pending("PlayFab.multiplayer already_initialized branch skipped: initialize_async() failed: %s" % (first_mp_init.message if first_mp_init != null else "null"))
		playfab.shutdown()
		ProjectSettings.set_setting(PLAYFAB_TITLE_ID_SETTING, original_title_id)
		ProjectSettings.set_setting(PLAYFAB_ENDPOINT_SETTING, original_endpoint)
		reset_playfab_runtime()
		return

	await _assert_signal_error(multiplayer.initialize_async(), "already_initialized", "PlayFab.multiplayer.initialize_async() second call")
	playfab.shutdown()
	ProjectSettings.set_setting(PLAYFAB_TITLE_ID_SETTING, original_title_id)
	ProjectSettings.set_setting(PLAYFAB_ENDPOINT_SETTING, original_endpoint)
	reset_playfab_runtime()


func test_multiplayer_shutdown_cancels_reentrant_pending_operations() -> void:
	if pending_unless_playfab_available():
		return

	var playfab = get_playfab()
	reset_playfab_runtime()
	var multiplayer = playfab.get_multiplayer()
	if multiplayer == null:
		return
	if not multiplayer.has_method("_test_enqueue_shutdown_pending"):
		pending("PlayFab Multiplayer shutdown re-entry test requires debug test hooks.")
		return

	var completion_state := {
		"first": false,
		"reentrant": false,
	}
	var first_signal = multiplayer._test_enqueue_shutdown_pending()
	first_signal.connect(func(_result):
		completion_state["first"] = true
		var reentrant_signal = multiplayer._test_enqueue_shutdown_pending()
		reentrant_signal.connect(func(_reentrant_result):
			completion_state["reentrant"] = true
		)
	)

	assert_playfab_result_ok(await await_completion(multiplayer.shutdown_async()), "PlayFab.multiplayer.shutdown_async() with re-entrant pending completion")
	assert_true(completion_state["first"], "Initial pending operation completed during shutdown")
	assert_true(completion_state["reentrant"], "Re-entrant pending operation completed during the same shutdown")
	assert_eq(multiplayer._test_pending_operation_count(), 0, "Shutdown drains all PlayFab Multiplayer pending operations")


func _assert_signal_error(async_signal, expected_code: String, name: String) -> void:
	assert_eq(typeof(async_signal), TYPE_SIGNAL, "%s returns completion Signal" % name)
	if typeof(async_signal) != TYPE_SIGNAL:
		return
	assert_playfab_result_error(await await_completion(async_signal), expected_code, name)
