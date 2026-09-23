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
		"join_match_ticket_async",
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
	assert_eq(get_class_constant("PlayFabMatchTicket", "STATUS_CREATING"), 0, "PlayFabMatchTicket.STATUS_CREATING matches PFMatchmakingTicketStatus::Creating")
	assert_eq(get_class_constant("PlayFabMatchTicket", "STATUS_JOINING"), 1, "PlayFabMatchTicket.STATUS_JOINING matches PFMatchmakingTicketStatus::Joining")
	assert_eq(get_class_constant("PlayFabMatchTicket", "STATUS_WAITING_FOR_PLAYERS"), 2, "PlayFabMatchTicket.STATUS_WAITING_FOR_PLAYERS matches PFMatchmakingTicketStatus::WaitingForPlayers")
	assert_eq(get_class_constant("PlayFabMatchTicket", "STATUS_WAITING_FOR_MATCH"), 3, "PlayFabMatchTicket.STATUS_WAITING_FOR_MATCH matches PFMatchmakingTicketStatus::WaitingForMatch")
	assert_eq(get_class_constant("PlayFabMatchTicket", "STATUS_MATCHED"), 4, "PlayFabMatchTicket.STATUS_MATCHED matches PFMatchmakingTicketStatus::Matched")
	assert_eq(get_class_constant("PlayFabMatchTicket", "STATUS_CANCELLED"), 5, "PlayFabMatchTicket.STATUS_CANCELLED matches PFMatchmakingTicketStatus::Canceled")
	assert_eq(get_class_constant("PlayFabMatchTicket", "STATUS_FAILED"), 6, "PlayFabMatchTicket.STATUS_FAILED matches PFMatchmakingTicketStatus::Failed")
	var status_ticket = instantiate_class("PlayFabMatchTicket")
	if status_ticket != null:
		assert_eq(typeof(status_ticket.status), TYPE_INT, "PlayFabMatchTicket.status remains an int")

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
		assert_eq(ticket_config.members, [matchmaking_member], "PlayFabMatchmakingTicketConfig.members setter")
		assert_eq(ticket_config.members_to_match_with, [], "Assigning members leaves members_to_match_with unchanged")
		var remote_members: Array = [
			{"id": "remote-player", "type": "title_player_account"},
		]
		ticket_config.members_to_match_with = remote_members
		assert_eq(
			ticket_config.members_to_match_with,
			remote_members,
			"PlayFabMatchmakingTicketConfig.members_to_match_with setter")
		assert_eq(ticket_config.members, [matchmaking_member], "Assigning members_to_match_with leaves members unchanged")
		ticket_config.members = []
		assert_eq(ticket_config.members, [], "PlayFabMatchmakingTicketConfig.members can be cleared independently")
		assert_eq(ticket_config.members_to_match_with, remote_members, "Clearing members leaves members_to_match_with unchanged")

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


func test_matchmaking_members_to_match_with_validation() -> void:
	if pending_unless_playfab_available():
		return

	var config = instantiate_class("PlayFabMatchmakingTicketConfig")
	assert_object_is(config, "PlayFabMatchmakingTicketConfig", "PlayFabMatchmakingTicketConfig can validate remote premade members")
	if config == null:
		return
	for hook in ["_test_validate_members_to_match_with", "_test_prepare_local_members"]:
		if config.has_method(hook):
			continue
		if OS.is_debug_build():
			fail_test("%s is required in debug coverage builds" % hook)
		else:
			pending("%s coverage requires GODOT_PLAYFAB_TEST_HOOKS" % hook)
		return

	var local_keys: Array = [
		{"id": "local-player", "type": "title_player_account"},
	]
	config.members_to_match_with = []
	var empty_result = config._test_validate_members_to_match_with(local_keys)
	assert_playfab_result_ok(empty_result, "Empty members_to_match_with preserves ordinary matchmaking")
	if empty_result != null and empty_result.ok:
		assert_eq(int(empty_result.data.get("count", -1)), 0, "Empty members_to_match_with maps to count zero")
		assert_eq(empty_result.data.get("values", []), [], "Empty members_to_match_with copies back no entity keys")
		assert_true(bool(empty_result.data.get("pointer_is_null", false)), "Empty members_to_match_with publishes a null native pointer")

	var expected_remote_keys: Array = [
		{"id": "remote-a", "type": "title_player_account"},
		{"id": "remote-b", "type": "title_player_account"},
	]
	config.members_to_match_with = expected_remote_keys
	var valid_result = config._test_validate_members_to_match_with(local_keys)
	assert_playfab_result_ok(valid_result, "Distinct remote entity keys are accepted")
	if valid_result != null and valid_result.ok:
		assert_eq(int(valid_result.data.get("count", -1)), 2, "Valid remote entity keys preserve their count")
		assert_eq(valid_result.data.get("values", []), expected_remote_keys, "Valid remote entity keys preserve id and type values")
		assert_false(bool(valid_result.data.get("pointer_is_null", true)), "Non-empty members_to_match_with publishes a native pointer")

	config.members_to_match_with = [
		{"id": &"remote-string-name", "type": &"title_player_account"},
	]
	var string_name_result = config._test_validate_members_to_match_with(local_keys)
	assert_playfab_result_ok(string_name_result, "StringName remote entity-key values are accepted")
	if string_name_result != null and string_name_result.ok:
		assert_eq(
			string_name_result.data.get("values", []),
			[{"id": "remote-string-name", "type": "title_player_account"}],
			"StringName entity-key values are copied as strings")

	for case in [
		{
			"name": "non-Dictionary entry",
			"value": [42],
		},
		{
			"name": "missing id",
			"value": [{"type": "title_player_account"}],
		},
		{
			"name": "blank id",
			"value": [{"id": "  ", "type": "title_player_account"}],
		},
		{
			"name": "missing type",
			"value": [{"id": "remote-a"}],
		},
		{
			"name": "blank type",
			"value": [{"id": "remote-a", "type": "  "}],
		},
		{
			"name": "numeric id",
			"value": [{"id": 42, "type": "title_player_account"}],
		},
		{
			"name": "boolean type",
			"value": [{"id": "remote-a", "type": true}],
		},
		{
			"name": "Array id",
			"value": [{"id": ["remote-a"], "type": "title_player_account"}],
		},
		{
			"name": "null type",
			"value": [{"id": "remote-a", "type": null}],
		},
		{
			"name": "duplicate remote entity key",
			"value": [
				{"id": "remote-a", "type": "title_player_account"},
				{"id": "remote-a", "type": "title_player_account"},
			],
		},
		{
			"name": "overlap with a local member",
			"value": [{"id": "local-player", "type": "title_player_account"}],
		},
	]:
		config.members_to_match_with = case["value"]
		assert_playfab_result_error(
			config._test_validate_members_to_match_with(local_keys),
			"invalid_match_ticket_config",
			"members_to_match_with rejects %s" % case["name"])


func test_matchmaking_local_member_inputs_remain_read_only() -> void:
	if pending_unless_playfab_available():
		return

	var config = instantiate_class("PlayFabMatchmakingTicketConfig")
	if config == null:
		return
	if not config.has_method("_test_prepare_local_members"):
		if OS.is_debug_build():
			fail_test("_test_prepare_local_members is required in debug coverage builds")
		else:
			pending("Local member preparation coverage requires GODOT_PLAYFAB_TEST_HOOKS")
		return

	var requester_a = instantiate_class("PlayFabUser")
	var requester_b = instantiate_class("PlayFabUser")
	var shared_empty: Array = []
	for requester in [requester_a, requester_b]:
		var result = config._test_prepare_local_members(requester, shared_empty)
		assert_playfab_result_ok(result, "Omitted local_members prepares only the current requester")
		if result != null and result.ok:
			assert_eq(int(result.data.get("input_count", -1)), 0, "Omitted local_members input stays empty")
			assert_eq(int(result.data.get("output_count", -1)), 1, "Omitted local_members produces one local user per call")
		assert_eq(shared_empty.size(), 0, "Successive omitted local_members calls do not accumulate requesters")

	var explicit_member = instantiate_class("PlayFabMatchmakingMember")
	assert_object_is(explicit_member, "PlayFabMatchmakingMember", "Explicit local member can be instantiated")
	if explicit_member == null:
		return
	explicit_member.user = requester_a
	var explicit_members: Array[Object] = [explicit_member]
	var explicit_result = config._test_prepare_local_members(requester_b, explicit_members)
	assert_playfab_result_ok(explicit_result, "Explicit local member preparation accepts typed Arrays")
	if explicit_result != null and explicit_result.ok:
		assert_eq(int(explicit_result.data.get("input_count", -1)), 1, "Explicit input count is preserved")
		assert_eq(int(explicit_result.data.get("output_count", -1)), 2, "Missing requester is appended only to owned output")
	assert_eq(explicit_members.size(), 1, "Explicit typed local_members Array is not mutated")


func test_join_match_ticket_readiness_requires_service_acceptance() -> void:
	if pending_unless_playfab_available():
		return

	var multiplayer = get_playfab().get_multiplayer()
	if multiplayer == null:
		return
	if not multiplayer.has_method("_test_join_match_ticket_readiness"):
		if OS.is_debug_build():
			fail_test("_test_join_match_ticket_readiness is required in debug coverage builds")
		else:
			pending("Join-ticket readiness coverage requires GODOT_PLAYFAB_TEST_HOOKS")
		return

	assert_eq(
		int(multiplayer._test_join_match_ticket_readiness(get_class_constant("PlayFabMatchTicket", "STATUS_CREATING"))),
		0,
		"Creating does not complete a joined ticket")
	assert_eq(
		int(multiplayer._test_join_match_ticket_readiness(get_class_constant("PlayFabMatchTicket", "STATUS_JOINING"))),
		0,
		"Queuing the native join does not report success")
	for status_name in ["STATUS_WAITING_FOR_PLAYERS", "STATUS_WAITING_FOR_MATCH", "STATUS_MATCHED"]:
		assert_eq(
			int(multiplayer._test_join_match_ticket_readiness(get_class_constant("PlayFabMatchTicket", status_name))),
			1,
			"%s means the service accepted the joined ticket" % status_name)
	assert_eq(
		int(multiplayer._test_join_match_ticket_readiness(get_class_constant("PlayFabMatchTicket", "STATUS_CANCELLED"))),
		2,
		"Cancelled is a join error")
	assert_eq(
		int(multiplayer._test_join_match_ticket_readiness(get_class_constant("PlayFabMatchTicket", "STATUS_FAILED"))),
		3,
		"Failed is a join error")


func test_arranged_lobby_join_config_presence_contract() -> void:
	if pending_unless_playfab_available():
		return

	var join_config = instantiate_class("PlayFabLobbyJoinConfig")
	assert_object_is(join_config, "PlayFabLobbyJoinConfig", "PlayFabLobbyJoinConfig can be instantiated")
	if join_config == null:
		return

	var arranged_fields: Array[String] = [
		"max_member_count",
		"access_policy",
		"owner_migration_policy",
		"restrict_invites_to_lobby_owner",
	]
	var defaults: Dictionary = {
		"max_member_count": 8,
		"access_policy": PlayFabLobbyConfig.ACCESS_POLICY_PRIVATE,
		"owner_migration_policy": PlayFabLobbyConfig.OWNER_MIGRATION_AUTOMATIC,
		"restrict_invites_to_lobby_owner": false,
	}

	# Consumers detect this feature by reading property metadata, so a missing
	# property or a setter-only binding would leave them permanently disabled.
	var expected_property_types: Dictionary = {
		"member_properties": TYPE_DICTIONARY,
		"max_member_count": TYPE_INT,
		"access_policy": TYPE_INT,
		"owner_migration_policy": TYPE_INT,
		"restrict_invites_to_lobby_owner": TYPE_BOOL,
	}
	var property_types: Dictionary = {}
	for property in ClassDB.class_get_property_list("PlayFabLobbyJoinConfig"):
		var property_name: String = String(property.get("name", ""))
		if expected_property_types.has(property_name):
			property_types[property_name] = int(property.get("type", TYPE_NIL))
	for property_name in expected_property_types:
		assert_true(
			property_types.has(property_name),
			"PlayFabLobbyJoinConfig exposes %s in its property list" % property_name)
		assert_eq(
			int(property_types.get(property_name, TYPE_NIL)),
			int(expected_property_types[property_name]),
			"PlayFabLobbyJoinConfig.%s has the exact property type" % property_name)

	for field_name in arranged_fields:
		for operation in ["get", "set", "has", "clear"]:
			var method_name: String = "%s_%s" % [operation, field_name]
			assert_true(
				ClassDB.class_has_method("PlayFabLobbyJoinConfig", method_name),
				"PlayFabLobbyJoinConfig exposes %s()" % method_name)

	var expected_constants: Dictionary = {
		"ACCESS_POLICY_PUBLIC": 0,
		"ACCESS_POLICY_FRIENDS": 1,
		"ACCESS_POLICY_PRIVATE": 2,
		"OWNER_MIGRATION_AUTOMATIC": 0,
		"OWNER_MIGRATION_MANUAL": 1,
		"OWNER_MIGRATION_NONE": 2,
	}
	for constant_name in expected_constants:
		assert_eq(
			int(get_class_constant("PlayFabLobbyConfig", constant_name)),
			int(expected_constants[constant_name]),
			"PlayFabLobbyConfig.%s has the native value" % constant_name)

	# Unset means "send the documented default", not "omit the field".
	for field_name in arranged_fields:
		assert_eq(
			join_config.get(field_name),
			defaults[field_name],
			"PlayFabLobbyJoinConfig.%s has its documented default" % field_name)
		assert_false(
			bool(join_config.call("has_%s" % field_name)),
			"PlayFabLobbyJoinConfig.has_%s() is false on a fresh config" % field_name)

	var member_properties: Dictionary = {
		"display_name": "joiner",
		"role": "guest",
	}
	join_config.member_properties = member_properties
	assert_eq(
		join_config.member_properties,
		member_properties,
		"PlayFabLobbyJoinConfig.member_properties round trips")
	for field_name in arranged_fields:
		assert_false(
			bool(join_config.call("has_%s" % field_name)),
			"member_properties assignment leaves has_%s() false" % field_name)

	# Each case starts fresh and assigns exactly one field. The two policy cases
	# use enum value zero and the bool case uses false; both must mark presence.
	var assignment_cases: Array[Dictionary] = [
		{
			"field": "max_member_count",
			"value": 4,
			"label": "max_member_count",
		},
		{
			"field": "access_policy",
			"value": PlayFabLobbyConfig.ACCESS_POLICY_PUBLIC,
			"label": "ACCESS_POLICY_PUBLIC (zero)",
		},
		{
			"field": "owner_migration_policy",
			"value": PlayFabLobbyConfig.OWNER_MIGRATION_AUTOMATIC,
			"label": "OWNER_MIGRATION_AUTOMATIC (zero)",
		},
		{
			"field": "restrict_invites_to_lobby_owner",
			"value": false,
			"label": "restrict_invites_to_lobby_owner (false)",
		},
	]
	for assignment_case in assignment_cases:
		var assigned_config = instantiate_class("PlayFabLobbyJoinConfig")
		assert_object_is(
			assigned_config,
			"PlayFabLobbyJoinConfig",
			"Fresh config for %s assignment" % assignment_case["label"])
		if assigned_config == null:
			return
		var assigned_field: String = String(assignment_case["field"])
		assigned_config.set(assigned_field, assignment_case["value"])
		for field_name in arranged_fields:
			var is_assigned_field: bool = field_name == assigned_field
			var expected_value: Variant = (
				assignment_case["value"]
				if is_assigned_field
				else defaults[field_name])
			assert_eq(
				assigned_config.get(field_name),
				expected_value,
				"%s assignment leaves %s value correct" % [
					assignment_case["label"],
					field_name,
				])
			assert_eq(
				bool(assigned_config.call("has_%s" % field_name)),
				is_assigned_field,
				"%s assignment leaves has_%s() independent" % [
					assignment_case["label"],
					field_name,
				])

	for capacity in [2, 16, 128]:
		var capacity_config = instantiate_class("PlayFabLobbyJoinConfig")
		assert_object_is(
			capacity_config,
			"PlayFabLobbyJoinConfig",
			"Fresh config for max_member_count %d" % capacity)
		if capacity_config == null:
			return
		capacity_config.max_member_count = capacity
		assert_eq(
			int(capacity_config.max_member_count),
			capacity,
			"PlayFabLobbyJoinConfig.max_member_count round trips %d" % capacity)
		assert_true(
			capacity_config.has_max_member_count(),
			"max_member_count %d marks presence" % capacity)
		capacity_config.clear_max_member_count()
		assert_eq(
			int(capacity_config.max_member_count),
			8,
			"clear_max_member_count() restores 8 after %d" % capacity)
		assert_false(
			capacity_config.has_max_member_count(),
			"clear_max_member_count() clears presence after %d" % capacity)

	# Each clear case starts fresh, sets every field, then clears exactly one.
	# The other values and presence flags must remain unchanged.
	var explicit_values: Dictionary = {
		"max_member_count": 4,
		"access_policy": PlayFabLobbyConfig.ACCESS_POLICY_FRIENDS,
		"owner_migration_policy": PlayFabLobbyConfig.OWNER_MIGRATION_MANUAL,
		"restrict_invites_to_lobby_owner": true,
	}
	for cleared_field in arranged_fields:
		var cleared_config = instantiate_class("PlayFabLobbyJoinConfig")
		assert_object_is(
			cleared_config,
			"PlayFabLobbyJoinConfig",
			"Fresh config for clear_%s()" % cleared_field)
		if cleared_config == null:
			return
		for field_name in arranged_fields:
			cleared_config.set(field_name, explicit_values[field_name])
		cleared_config.call("clear_%s" % cleared_field)
		for field_name in arranged_fields:
			var is_cleared_field: bool = field_name == cleared_field
			var expected_value: Variant = (
				defaults[field_name]
				if is_cleared_field
				else explicit_values[field_name])
			assert_eq(
				cleared_config.get(field_name),
				expected_value,
				"clear_%s() leaves %s value correct" % [
					cleared_field,
					field_name,
				])
			assert_eq(
				bool(cleared_config.call("has_%s" % field_name)),
				not is_cleared_field,
				"clear_%s() leaves has_%s() independent" % [
					cleared_field,
					field_name,
				])


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
	await _assert_signal_error(multiplayer.join_match_ticket_async(blank_user, "", "", []), "not_initialized", "PlayFab.multiplayer.join_match_ticket_async() before multiplayer init")

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

	var blank_user = instantiate_class("PlayFabUser")
	await _assert_signal_error(
		multiplayer.join_match_ticket_async(blank_user, "", "queue"),
		"invalid_join_match_ticket",
		"PlayFab.multiplayer.join_match_ticket_async() rejects a blank ticket_id before user validation")
	await _assert_signal_error(
		multiplayer.join_match_ticket_async(blank_user, "ticket-id", ""),
		"invalid_join_match_ticket",
		"PlayFab.multiplayer.join_match_ticket_async() rejects a blank queue_name before user validation")
	await _assert_signal_error(
		multiplayer.join_match_ticket_async(blank_user, "ticket-id", "queue"),
		"invalid_user",
		"PlayFab.multiplayer.join_match_ticket_async() validates the requester after its string arguments")

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
