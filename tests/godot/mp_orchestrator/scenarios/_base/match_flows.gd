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
	var gate: Variant = requires_live_write(orch)
	if gate != null: return gate
	var match: Variant = await _create_two_player_match(orch)
	if _is_failure(match) or _is_skip(match): return match
	var connection_string: String = String(match.get("connection_string", ""))
	var err: Variant = assert_true(not connection_string.is_empty(), "arranged lobby connection string should be present", { "match": match })
	if err != null: return err
	for role in ["host", "guest"]:
		var lobby: Variant = await _join_arranged_lobby(orch, role, "arranged", connection_string, _role_member_properties(role))
		if _is_failure(lobby): return lobby
	for role in ["host", "guest"]:
		var lobby: Variant = await _wait_lobby_member_count(orch, role, "arranged", 2)
		if _is_failure(lobby): return lobby
	return ok()


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
