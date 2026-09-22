extends "res://scenarios/_base/mp_scenario_utils.gd"

func run_match_ticket_create_and_cancel(orch) -> Dictionary:
	var gate: Variant = requires_live_write(orch)
	if gate != null: return gate
	var queue_v: Variant = _configured_queue(orch)
	if _is_failure(queue_v) or _is_skip(queue_v): return queue_v
	var signed: Variant = await _sign_in_roles(orch, ["host"])
	if _is_failure(signed): return signed
	var ticket: Variant = await _create_match_ticket(orch, "host", "ticket", String(queue_v), { "scenario_token": _unique_token(orch, "cancel") })
	if _is_failure(ticket): return ticket
	var cancelled: Variant = await _command_ok(orch, "host", "cancel_match_ticket", { "handle": "ticket" }, COMMAND_TIMEOUT_MS)
	if _is_failure(cancelled): return cancelled
	var err: Variant = assert_true(bool(cancelled.get("ticket", {}).get("is_cancelled", false)) or String(cancelled.get("ticket", {}).get("status_name", "")) == "cancelled", "ticket should cancel", { "ticket": cancelled })
	if err != null: return err
	return ok()


func run_match_ticket_two_player_match_complete(orch) -> Dictionary:
	var gate: Variant = requires_live_write(orch)
	if gate != null: return gate
	var match: Variant = await _create_two_player_match(orch)
	if _is_failure(match) or _is_skip(match): return match
	return ok({ "match_id": match.get("host_ticket", {}).get("match_id", "") })


func run_match_ticket_completion_metadata_present(orch) -> Dictionary:
	var gate: Variant = requires_live_write(orch)
	if gate != null: return gate
	var match: Variant = await _create_two_player_match(orch)
	if _is_failure(match) or _is_skip(match): return match
	for key in ["host_ticket", "guest_ticket"]:
		var ticket: Dictionary = match.get(key, {})
		var err: Variant = assert_true(not String(ticket.get("match_id", "")).is_empty(), "match_id should be present", { "ticket": ticket })
		if err != null: return err
		err = assert_true(not String(ticket.get("arranged_lobby_connection_string", "")).is_empty(), "arranged lobby connection string should be present", { "ticket": ticket })
		if err != null: return err
		err = assert_true(int(ticket.get("member_count", 0)) >= 1, "ticket member metadata should be present", { "ticket": ticket })
		if err != null: return err
	return ok()


func run_match_ticket_invalid_queue_name(orch) -> Dictionary:
	var gate: Variant = requires_live_write(orch)
	if gate != null: return gate
	var signed: Variant = await _sign_in_roles(orch, ["host"])
	if _is_failure(signed): return signed
	var err: Variant = await _expect_command_error(orch, "host", "create_match_ticket", { "as": "bad", "queue_name": "__missing_queue__", "timeout_seconds": 10 }, [])
	if _is_failure(err): return err
	return ok()


func run_match_ticket_create_without_init(orch) -> Dictionary:
	var gate: Variant = requires_live(orch)
	if gate != null: return gate
	var queue_v: Variant = _configured_queue(orch)
	if _is_failure(queue_v) or _is_skip(queue_v): return queue_v
	var signed: Variant = await _sign_in_roles(orch, ["host"], { "host": { "initialize_multiplayer": false } })
	if _is_failure(signed): return signed
	var err: Variant = await _expect_command_error(orch, "host", "create_match_ticket", { "as": "bad", "queue_name": String(queue_v), "timeout_seconds": 10 }, ["multiplayer_not_initialized"])
	if _is_failure(err): return err
	return ok()


func run_match_state_full_match_event_sequence(orch) -> Dictionary:
	var gate: Variant = requires_live_write(orch)
	if gate != null: return gate
	var match: Variant = await _create_two_player_match(orch)
	if _is_failure(match) or _is_skip(match): return match
	return ok({ "match_id": match.get("host_ticket", {}).get("match_id", "") })


func run_match_integration_arranged_lobby_join(orch) -> Dictionary:
	# No overrides: proves an unset config still initializes the lobby with the
	# legacy defaults rather than sending zeroes.
	return await _run_arranged_lobby_initialization(orch, {}, {
		"max_member_count": 8,
		"access_policy": 2,
		"owner_migration_policy": 0,
	})


func run_match_integration_arranged_lobby_configuration(orch) -> Dictionary:
	# Explicit values, two of which coincide with the defaults: proves explicit
	# assignment is honored and is not mistaken for "unset".
	var requested: Dictionary = {
		"max_member_count": 4,
		"access_policy": 2,
		"owner_migration_policy": 0,
	}
	return await _run_arranged_lobby_initialization(orch, requested, requested)


func run_match_integration_arranged_lobby_policy_overrides(orch) -> Dictionary:
	# Every field differs from its default, including access_policy 0, which the
	# presence flag must not confuse with "unset".
	var requested: Dictionary = {
		"max_member_count": 2,
		"access_policy": 0,
		"owner_migration_policy": 1,
	}
	return await _run_arranged_lobby_initialization(orch, requested, requested)


func run_match_integration_arranged_lobby_capacity_above_eight(orch) -> Dictionary:
	var requested: Dictionary = {
		"max_member_count": 16,
		"access_policy": 2,
		"owner_migration_policy": 0,
	}
	return await _run_arranged_lobby_initialization(orch, requested, requested)


func run_match_integration_arranged_lobby_null_config(orch) -> Dictionary:
	var gate: Variant = requires_live_write(orch)
	if gate != null: return gate
	var match: Variant = await _create_two_player_match(orch)
	if _is_failure(match) or _is_skip(match): return match
	var connection_string_by_role: Dictionary = match.get("connection_string_by_role", {})
	var expected: Dictionary = {
		"max_member_count": 8,
		"access_policy": 2,
		"owner_migration_policy": 0,
	}
	var err: Variant = null
	var arranged_lobby_id: String = ""
	for role in ["host", "guest"]:
		var lobby: Variant = await _join_arranged_lobby(
			orch,
			role,
			"arranged",
			String(connection_string_by_role.get(role, "")),
			{},
			{},
			true)
		if _is_failure(lobby): return lobby
		for field in expected:
			err = assert_eq(
				int(lobby.get(field, -1)),
				int(expected[field]),
				"%s null-config arranged lobby %s should use its default" % [role, field])
			if err != null: return err
		var lobby_id: String = String(lobby.get("lobby_id", ""))
		err = assert_true(not lobby_id.is_empty(), "%s null-config arranged join should report a lobby id" % role, { "lobby": lobby })
		if err != null: return err
		if arranged_lobby_id.is_empty():
			arranged_lobby_id = lobby_id
		else:
			err = assert_eq(lobby_id, arranged_lobby_id, "both null-config joins should enter the same arranged lobby")
			if err != null: return err
	for role in ["host", "guest"]:
		var joined: Variant = await _wait_lobby_member_count(orch, role, "arranged", 2)
		if _is_failure(joined): return joined
		err = assert_eq(int(joined.get("member_count", 0)), 2, "null-config arranged lobby should contain the two matched players")
		if err != null: return err
	return ok(expected)


func _run_arranged_lobby_initialization(orch, overrides: Dictionary, expected: Dictionary) -> Dictionary:
	var gate: Variant = requires_live_write(orch)
	if gate != null: return gate
	var match: Variant = await _create_two_player_match(orch)
	if _is_failure(match) or _is_skip(match): return match
	var connection_string_by_role: Dictionary = match.get("connection_string_by_role", {})
	var arranged_lobby_id: String = ""
	var member_properties_by_role: Dictionary = {}
	for role in ["host", "guest"]:
		var member_properties: Dictionary = _role_member_properties(role)
		member_properties_by_role[role] = member_properties
		var lobby: Variant = await _join_arranged_lobby(
			orch,
			role,
			"arranged",
			String(connection_string_by_role.get(role, "")),
			member_properties,
			overrides)
		if _is_failure(lobby): return lobby
		# Assert on the snapshot this join returned, before any update, so the
		# check cannot be satisfied by a value some later call happened to set.
		for field in ["max_member_count", "access_policy", "owner_migration_policy"]:
			var actual: int = int(lobby.get(field, -1))
			var expect: int = int(expected[field])
			var check: Variant = assert_true(actual == expect,
				"%s arranged lobby %s should be %d but was %d" % [role, field, expect, actual],
				{ "role": role, "requested": overrides, "lobby": lobby })
			if check != null: return check
		var lobby_id: String = String(lobby.get("lobby_id", ""))
		var lobby_id_err: Variant = assert_true(not lobby_id.is_empty(), "%s arranged join should report a lobby id" % role, { "lobby": lobby })
		if lobby_id_err != null: return lobby_id_err
		if arranged_lobby_id.is_empty():
			arranged_lobby_id = lobby_id
		else:
			lobby_id_err = assert_eq(lobby_id, arranged_lobby_id, "both clients should join the same arranged lobby")
			if lobby_id_err != null: return lobby_id_err
		var local_member: Dictionary = _member_for_role(lobby, role)
		var local_member_err: Variant = assert_true(not local_member.is_empty(), "%s first snapshot should include its local member properties" % role, { "lobby": lobby })
		if local_member_err != null: return local_member_err
		for key in member_properties:
			local_member_err = assert_eq(
				String(local_member.get("properties", {}).get(key, "")),
				String(member_properties[key]),
				"%s first snapshot should preserve local member property %s" % [role, key])
			if local_member_err != null: return local_member_err
	for role in ["host", "guest"]:
		var joined: Variant = await _wait_lobby_member_count(orch, role, "arranged", 2)
		if _is_failure(joined): return joined
		var member_count_err: Variant = assert_eq(
			int(joined.get("member_count", 0)),
			2,
			"%s arranged lobby should contain the two matched players, independent of capacity" % role)
		if member_count_err != null: return member_count_err
	for observer_role in ["host", "guest"]:
		var remote_role: String = "guest" if observer_role == "host" else "host"
		var remote_properties: Dictionary = member_properties_by_role[remote_role]
		for key in remote_properties:
			var converged: Variant = await _wait_member_property(
				orch,
				observer_role,
				"arranged",
				remote_role,
				String(key),
				String(remote_properties[key]))
			if _is_failure(converged): return converged
	return ok(expected)


func run_match_integration_arranged_lobby_cleanup(orch) -> Dictionary:
	var joined: Variant = await run_match_integration_arranged_lobby_join(orch)
	if _is_failure(joined) or _is_skip(joined): return joined
	var left_guest: Variant = await _leave_lobby(orch, "guest", "arranged")
	if _is_failure(left_guest): return left_guest
	var left_host: Variant = await _leave_lobby(orch, "host", "arranged")
	if _is_failure(left_host): return left_host
	var err: Variant = await _expect_command_error(orch, "host", "get_lobby_snapshot", { "handle": "arranged" }, ["unknown_handle"])
	if _is_failure(err): return err
	return ok()


func run_match_integration_arranged_lobby_property_round_trip(orch) -> Dictionary:
	var joined: Variant = await run_match_integration_arranged_lobby_join(orch)
	if _is_failure(joined) or _is_skip(joined): return joined
	var token: String = _unique_token(orch, "arranged-prop")
	var set_result: Variant = await _command_ok(orch, "host", "set_lobby_properties", { "handle": "arranged", "properties": { "arranged_round": token } }, COMMAND_TIMEOUT_MS)
	if _is_failure(set_result): return set_result
	var guest_lobby: Variant = await _wait_lobby_property(orch, "guest", "arranged", "arranged_round", token)
	if _is_failure(guest_lobby): return guest_lobby
	return ok({ "arranged_round": token })
