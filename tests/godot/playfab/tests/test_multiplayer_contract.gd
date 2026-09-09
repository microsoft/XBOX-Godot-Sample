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
	# values are referenced from sample/tutorial_integrated/autoload/lobby.gd and from
	# the doc_classes XML. Regressions in either direction (renames or value
	# shifts) silently break listener match statements.
	assert_eq(get_class_constant("PlayFabLobby", "MEMBER_ADDED"), 1, "PlayFabLobby.MEMBER_ADDED constant is stable")
	assert_eq(get_class_constant("PlayFabLobby", "MEMBER_REMOVED"), 2, "PlayFabLobby.MEMBER_REMOVED constant is stable")
	assert_eq(get_class_constant("PlayFabLobby", "MEMBER_UPDATED"), 3, "PlayFabLobby.MEMBER_UPDATED constant is stable")
	assert_eq(get_class_constant("PlayFabLobby", "PROPERTIES_UPDATED"), 4, "PlayFabLobby.PROPERTIES_UPDATED constant is stable")
	assert_eq(get_class_constant("PlayFabLobby", "OWNER_CHANGED"), 5, "PlayFabLobby.OWNER_CHANGED constant is stable")
	assert_eq(get_class_constant("PlayFabLobby", "DISCONNECTED"), 6, "PlayFabLobby.DISCONNECTED constant is stable")
	assert_eq(get_class_constant("PlayFabLobby", "MEMBER_CONNECTION_CHANGED"), 7, "PlayFabLobby.MEMBER_CONNECTION_CHANGED constant is stable")
	assert_eq(get_class_constant("PlayFabLobby", "SEARCH_PROPERTIES_UPDATED"), 8, "PlayFabLobby.SEARCH_PROPERTIES_UPDATED constant is stable")
	assert_eq(get_class_constant("PlayFabLobby", "CONFIGURATION_UPDATED"), 9, "PlayFabLobby.CONFIGURATION_UPDATED constant is stable")
	assert_eq(get_class_constant("PlayFabLobby", "DISCONNECTING"), 10, "PlayFabLobby.DISCONNECTING constant is stable")
	assert_eq(get_class_constant("PlayFabLobby", "UPDATE_COMPLETED"), 11, "PlayFabLobby.UPDATE_COMPLETED constant is stable")

	# Membership-lock, departure-reason and connection-status constants mirror
	# the native PFLobby enums; titles branch on them directly.
	assert_eq(get_class_constant("PlayFabLobby", "MEMBERSHIP_LOCK_UNLOCKED"), 0, "PlayFabLobby.MEMBERSHIP_LOCK_UNLOCKED matches PFLobbyMembershipLock::Unlocked")
	assert_eq(get_class_constant("PlayFabLobby", "MEMBERSHIP_LOCK_LOCKED"), 1, "PlayFabLobby.MEMBERSHIP_LOCK_LOCKED matches PFLobbyMembershipLock::Locked")
	assert_eq(get_class_constant("PlayFabLobby", "MEMBER_REMOVED_LOCAL_USER_LEFT_LOBBY"), 0, "PlayFabLobby.MEMBER_REMOVED_LOCAL_USER_LEFT_LOBBY matches PFLobbyMemberRemovedReason")
	assert_eq(get_class_constant("PlayFabLobby", "MEMBER_REMOVED_LOCAL_USER_FORCIBLY_REMOVED"), 1, "PlayFabLobby.MEMBER_REMOVED_LOCAL_USER_FORCIBLY_REMOVED matches PFLobbyMemberRemovedReason")
	assert_eq(get_class_constant("PlayFabLobby", "MEMBER_REMOVED_REMOTE_USER_LEFT_LOBBY"), 2, "PlayFabLobby.MEMBER_REMOVED_REMOTE_USER_LEFT_LOBBY matches PFLobbyMemberRemovedReason")
	assert_eq(get_class_constant("PlayFabLobby", "DISCONNECTING_NO_LOCAL_MEMBERS"), 0, "PlayFabLobby.DISCONNECTING_NO_LOCAL_MEMBERS matches PFLobbyDisconnectingReason")
	assert_eq(get_class_constant("PlayFabLobby", "DISCONNECTING_LOBBY_DELETED"), 1, "PlayFabLobby.DISCONNECTING_LOBBY_DELETED matches PFLobbyDisconnectingReason")
	assert_eq(get_class_constant("PlayFabLobby", "DISCONNECTING_CONNECTION_INTERRUPTION"), 2, "PlayFabLobby.DISCONNECTING_CONNECTION_INTERRUPTION matches PFLobbyDisconnectingReason")
	assert_eq(get_class_constant("PlayFabLobby", "DISCONNECTING_LOBBY_SERVER_LEFT"), 3, "PlayFabLobby.DISCONNECTING_LOBBY_SERVER_LEFT matches PFLobbyDisconnectingReason")
	assert_eq(get_class_constant("PlayFabLobbyMember", "CONNECTION_STATUS_NOT_CONNECTED"), 0, "PlayFabLobbyMember.CONNECTION_STATUS_NOT_CONNECTED matches PFLobbyMemberConnectionStatus")
	assert_eq(get_class_constant("PlayFabLobbyMember", "CONNECTION_STATUS_CONNECTED"), 1, "PlayFabLobbyMember.CONNECTION_STATUS_CONNECTED matches PFLobbyMemberConnectionStatus")

	# PlayFabLobbyUpdateConfig deliberately reuses PlayFabLobbyConfig's values
	# so the same constant works for creation and for updates.
	assert_eq(get_class_constant("PlayFabLobbyUpdateConfig", "ACCESS_POLICY_PUBLIC"), get_class_constant("PlayFabLobbyConfig", "ACCESS_POLICY_PUBLIC"), "PlayFabLobbyUpdateConfig.ACCESS_POLICY_PUBLIC matches PlayFabLobbyConfig")
	assert_eq(get_class_constant("PlayFabLobbyUpdateConfig", "ACCESS_POLICY_FRIENDS"), get_class_constant("PlayFabLobbyConfig", "ACCESS_POLICY_FRIENDS"), "PlayFabLobbyUpdateConfig.ACCESS_POLICY_FRIENDS matches PlayFabLobbyConfig")
	assert_eq(get_class_constant("PlayFabLobbyUpdateConfig", "ACCESS_POLICY_PRIVATE"), get_class_constant("PlayFabLobbyConfig", "ACCESS_POLICY_PRIVATE"), "PlayFabLobbyUpdateConfig.ACCESS_POLICY_PRIVATE matches PlayFabLobbyConfig")
	assert_eq(get_class_constant("PlayFabLobbyUpdateConfig", "MEMBERSHIP_LOCK_LOCKED"), get_class_constant("PlayFabLobby", "MEMBERSHIP_LOCK_LOCKED"), "PlayFabLobbyUpdateConfig.MEMBERSHIP_LOCK_LOCKED matches PlayFabLobby")
	assert_eq(get_class_constant("PlayFabMatchTicket", "CREATED"), 100, "PlayFabMatchTicket.CREATED constant is stable")
	assert_eq(get_class_constant("PlayFabMatchTicket", "STATUS_CHANGED"), 101, "PlayFabMatchTicket.STATUS_CHANGED constant is stable")
	assert_eq(get_class_constant("PlayFabMatchTicket", "COMPLETED"), 102, "PlayFabMatchTicket.COMPLETED constant is stable")
	assert_eq(get_class_constant("PlayFabMatchTicket", "CANCELLED"), 103, "PlayFabMatchTicket.CANCELLED constant is stable")
	assert_eq(get_class_constant("PlayFabMatchTicket", "FAILED"), 104, "PlayFabMatchTicket.FAILED constant is stable")

	# State-change payload shape is part of the public contract too — listeners
	# read change.kind / change.lobby / change.member / change.result directly.
	var lobby_change = instantiate_class("PlayFabLobbyStateChange")
	if lobby_change != null:
		for getter in ["get_kind", "get_lobby", "get_result", "get_member", "get_invite", "get_user", "get_properties", "get_reason"]:
			assert_has_method_named(lobby_change, getter)
		assert_eq(lobby_change.get_reason(), get_class_constant("PlayFabLobbyStateChange", "REASON_NONE"), "PlayFabLobbyStateChange.reason defaults to REASON_NONE")
	var lobby_member = instantiate_class("PlayFabLobbyMember")
	if lobby_member != null:
		assert_has_method_named(lobby_member, "get_connection_status")
		assert_eq(lobby_member.connection_status, get_class_constant("PlayFabLobbyMember", "CONNECTION_STATUS_NOT_CONNECTED"), "PlayFabLobbyMember.connection_status defaults to CONNECTION_STATUS_NOT_CONNECTED")
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


func test_lobby_local_member_property_snapshot_converges_after_local_write() -> void:
	var lobby = instantiate_class("PlayFabLobby")
	assert_object_is(lobby, "PlayFabLobby", "PlayFabLobby can be instantiated")
	if lobby == null:
		return
	if not lobby.has_method("_test_seed_local_member") or not lobby.has_method("_test_apply_local_member_property_update"):
		pending("PlayFabLobby local-member snapshot convergence coverage requires GODOT_PLAYFAB_TEST_HOOKS")
		return

	lobby._test_seed_local_member({"id": "local-player", "type": "title_player_account"}, {"ready": "false", "role": "captain"})
	lobby._test_apply_local_member_property_update({"ready": "true", "team": "blue", "role": null})

	var local_member = _find_local_lobby_member(lobby)
	assert_not_null(local_member, "Local member remains present after local property write")
	if local_member == null:
		return
	var properties: Dictionary = local_member.properties
	assert_eq(properties.get("ready"), "true", "Local self ready property is patched eagerly")
	assert_eq(properties.get("team"), "blue", "Local self team property is patched eagerly")
	assert_false(properties.has("role"), "Local self property deletions are patched eagerly")


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

	var update_config = instantiate_class("PlayFabLobbyUpdateConfig")
	assert_object_is(update_config, "PlayFabLobbyUpdateConfig", "PlayFabLobbyUpdateConfig can be instantiated")
	if update_config != null:
		# Presence tracking is the whole point of this config: a field is only
		# sent when it was explicitly set, so `false` and enum value 0 stay
		# sendable instead of being indistinguishable from "unset".
		assert_true(update_config.is_empty(), "A fresh PlayFabLobbyUpdateConfig is empty")
		for field_name in ["membership_lock", "access_policy", "max_member_count", "restrict_invites_to_lobby_owner", "new_owner_entity_key", "search_properties", "lobby_properties"]:
			assert_false(update_config.call("has_%s" % field_name), "PlayFabLobbyUpdateConfig.has_%s() is false before assignment" % field_name)

		update_config.restrict_invites_to_lobby_owner = false
		assert_true(update_config.has_restrict_invites_to_lobby_owner(), "Assigning false still marks restrict_invites_to_lobby_owner present")
		assert_false(update_config.is_empty(), "PlayFabLobbyUpdateConfig is no longer empty once a field is set")
		update_config.clear_restrict_invites_to_lobby_owner()
		assert_false(update_config.has_restrict_invites_to_lobby_owner(), "clear_restrict_invites_to_lobby_owner() drops the field")
		assert_true(update_config.is_empty(), "Clearing the only assigned field returns the config to empty")

		update_config.membership_lock = get_class_constant("PlayFabLobbyUpdateConfig", "MEMBERSHIP_LOCK_LOCKED")
		update_config.access_policy = get_class_constant("PlayFabLobbyUpdateConfig", "ACCESS_POLICY_PUBLIC")
		update_config.max_member_count = 6
		update_config.new_owner_entity_key = {"id": "owner-id", "type": "title_player_account"}
		update_config.search_properties = {"string_key1": "updated"}
		update_config.lobby_properties = {"map": "arena"}
		assert_eq(update_config.membership_lock, get_class_constant("PlayFabLobbyUpdateConfig", "MEMBERSHIP_LOCK_LOCKED"), "PlayFabLobbyUpdateConfig.membership_lock setter")
		assert_eq(update_config.max_member_count, 6, "PlayFabLobbyUpdateConfig.max_member_count setter")
		assert_eq(update_config.new_owner_entity_key.get("id"), "owner-id", "PlayFabLobbyUpdateConfig.new_owner_entity_key setter")
		assert_eq(update_config.search_properties.get("string_key1"), "updated", "PlayFabLobbyUpdateConfig.search_properties setter")
		assert_eq(update_config.lobby_properties.get("map"), "arena", "PlayFabLobbyUpdateConfig.lobby_properties setter")
		for field_name in ["membership_lock", "access_policy", "max_member_count", "new_owner_entity_key", "search_properties", "lobby_properties"]:
			assert_true(update_config.call("has_%s" % field_name), "PlayFabLobbyUpdateConfig.has_%s() is true after assignment" % field_name)

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
		"PlayFabLobbyUpdateConfig",
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
		for update_method_name in ["post_update_async", "set_search_properties_async", "set_membership_lock_async"]:
			assert_has_method_named(detached_lobby, update_method_name)
		await _assert_signal_error(detached_lobby.post_update_async(instantiate_class("PlayFabLobbyUpdateConfig")), "invalid_lobby", "Detached PlayFabLobby.post_update_async() reports invalid_lobby")
		await _assert_signal_error(detached_lobby.set_search_properties_async({"string_key1": "detached"}), "invalid_lobby", "Detached PlayFabLobby.set_search_properties_async() reports invalid_lobby")
		await _assert_signal_error(detached_lobby.set_membership_lock_async(get_class_constant("PlayFabLobby", "MEMBERSHIP_LOCK_LOCKED")), "invalid_lobby", "Detached PlayFabLobby.set_membership_lock_async() reports invalid_lobby")
		# Reading cached configuration must be safe on a lobby that never
		# reached the service, so listeners can render state unconditionally.
		for getter in ["get_access_policy", "get_owner_migration_policy", "get_membership_lock", "get_restrict_invites_to_lobby_owner", "get_disconnecting_reason"]:
			assert_has_method_named(detached_lobby, getter)
		assert_eq(detached_lobby.get_disconnecting_reason(), get_class_constant("PlayFabLobbyStateChange", "REASON_NONE"), "Detached PlayFabLobby reports no disconnecting reason")

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


func _find_local_lobby_member(lobby: Object) -> Object:
	for member in lobby.members:
		if member != null and member.is_local:
			return member
	return null


func _assert_signal_error(async_signal, expected_code: String, name: String) -> void:
	assert_eq(typeof(async_signal), TYPE_SIGNAL, "%s returns completion Signal" % name)
	if typeof(async_signal) != TYPE_SIGNAL:
		return
	assert_playfab_result_error(await await_completion(async_signal), expected_code, name)
