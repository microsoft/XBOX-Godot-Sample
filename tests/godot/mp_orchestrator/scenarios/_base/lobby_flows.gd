extends "res://scenarios/_base/mp_scenario_utils.gd"

func run_lobby_create_public_smoke(orch) -> Dictionary:
	var gate: Variant = requires_live_write(orch)
	if gate != null: return gate
	var signed: Variant = await _sign_in_roles(orch, ["host"])
	if _is_failure(signed): return signed
	var token: String = _unique_token(orch, "lobby-public")
	var lobby: Variant = await _create_lobby(orch, "host", "main", _public_lobby_config(4, { "string_key1": token }, {}, _role_member_properties("host")))
	if _is_failure(lobby): return lobby
	var err: Variant = assert_true(not String(lobby.get("lobby_id", "")).is_empty(), "lobby_id should be populated", { "lobby": lobby })
	if err != null: return err
	err = assert_true(not String(lobby.get("connection_string", "")).is_empty(), "connection string should be populated", { "lobby": lobby })
	if err != null: return err
	err = assert_eq(int(lobby.get("member_count", 0)), 1, "host should be sole member")
	if err != null: return err
	return ok({ "lobby_id": lobby.get("lobby_id", "") })


func run_lobby_create_private_smoke(orch) -> Dictionary:
	var gate: Variant = requires_live_write(orch)
	if gate != null: return gate
	var signed: Variant = await _sign_in_roles(orch, ["host"])
	if _is_failure(signed): return signed
	var lobby: Variant = await _create_lobby(orch, "host", "main", _private_lobby_config(4, {}, {}, _role_member_properties("host")))
	if _is_failure(lobby): return lobby
	var err: Variant = assert_eq(int(lobby.get("member_count", 0)), 1, "private lobby should contain only host")
	if err != null: return err
	return ok({ "lobby_id": lobby.get("lobby_id", "") })


func run_lobby_create_with_initial_lobby_properties(orch) -> Dictionary:
	var gate: Variant = requires_live_write(orch)
	if gate != null: return gate
	var signed: Variant = await _sign_in_roles(orch, ["host"])
	if _is_failure(signed): return signed
	var token: String = _unique_token(orch, "initial-lobby-props")
	var props: Dictionary = { "scenario": token, "phase": "create" }
	var lobby: Variant = await _create_lobby(orch, "host", "main", _public_lobby_config(4, {}, props, _role_member_properties("host")))
	if _is_failure(lobby): return lobby
	for key in props.keys():
		var err: Variant = assert_eq(String(lobby.get("properties", {}).get(key, "")), String(props[key]), "initial lobby property should round trip")
		if err != null: return err
	return ok({ "properties": props })


func run_lobby_create_with_initial_member_properties(orch) -> Dictionary:
	var gate: Variant = requires_live_write(orch)
	if gate != null: return gate
	var signed: Variant = await _sign_in_roles(orch, ["host"])
	if _is_failure(signed): return signed
	var token: String = _unique_token(orch, "initial-member-props")
	var member_props: Dictionary = _role_member_properties("host", { "scenario": token })
	var lobby: Variant = await _create_lobby(orch, "host", "main", _public_lobby_config(4, {}, {}, member_props))
	if _is_failure(lobby): return lobby
	var member: Dictionary = _member_for_role(lobby, "host")
	var err: Variant = assert_eq(String(member.get("properties", {}).get("scenario", "")), token, "initial member property should round trip")
	if err != null: return err
	return ok({ "member": member })


func run_lobby_create_with_initial_search_properties(orch) -> Dictionary:
	var gate: Variant = requires_live_write(orch)
	if gate != null: return gate
	var signed: Variant = await _sign_in_roles(orch, ["host", "observer"])
	if _is_failure(signed): return signed
	var token: String = _unique_token(orch, "initial-search-props")
	var lobby: Variant = await _create_lobby(orch, "host", "main", _public_lobby_config(4, { "string_key1": token }, {}, _role_member_properties("host")))
	if _is_failure(lobby): return lobby
	var search: Variant = await _search_lobbies(orch, "observer", _eq_filter("string_key1", token))
	if _is_failure(search): return search
	var err: Variant = assert_true(_search_contains_lobby_id(search, String(lobby.get("lobby_id", ""))), "search should find lobby by initial search property", { "search": search, "lobby": lobby })
	if err != null: return err
	return ok({ "filter": _eq_filter("string_key1", token) })


func run_lobby_join_by_connection_string(orch) -> Dictionary:
	var gate: Variant = requires_live_write(orch)
	if gate != null: return gate
	var setup: Variant = await _create_join_lobby(orch, ["host", "guest"])
	if _is_failure(setup): return setup
	return ok({ "connection_string": setup.get("connection_string", "") })


func run_lobby_join_three_clients(orch) -> Dictionary:
	var gate: Variant = requires_live_write(orch)
	if gate != null: return gate
	var setup: Variant = await _create_join_lobby(orch, ["host", "guest", "guest2"], "main", 0, 4)
	if _is_failure(setup): return setup
	return ok()


func run_lobby_search_public_by_string_key(orch) -> Dictionary:
	var gate: Variant = requires_live_write(orch)
	if gate != null: return gate
	var signed: Variant = await _sign_in_roles(orch, ["host", "observer"])
	if _is_failure(signed): return signed
	var token: String = _unique_token(orch, "search-public")
	var lobby: Variant = await _create_lobby(orch, "host", "main", _public_lobby_config(4, { "string_key1": token }, {}, _role_member_properties("host")))
	if _is_failure(lobby): return lobby
	var search: Variant = await _search_lobbies(orch, "observer", _eq_filter("string_key1", token))
	if _is_failure(search): return search
	var err: Variant = assert_true(_search_contains_lobby_id(search, String(lobby.get("lobby_id", ""))), "public search should include created lobby", { "search": search, "lobby": lobby })
	if err != null: return err
	return ok({ "count": int(search.get("count", 0)) })


func run_lobby_search_no_results_isolation(orch) -> Dictionary:
	var gate: Variant = requires_live_write(orch)
	if gate != null: return gate
	var signed: Variant = await _sign_in_roles(orch, ["host", "observer"])
	if _is_failure(signed): return signed
	var token: String = _unique_token(orch, "search-isolation")
	var other: String = token + "-absent"
	var lobby: Variant = await _create_lobby(orch, "host", "main", _public_lobby_config(4, { "string_key1": token }, {}, _role_member_properties("host")))
	if _is_failure(lobby): return lobby
	var search: Variant = await _search_lobbies(orch, "observer", _eq_filter("string_key1", other))
	if _is_failure(search): return search
	var err: Variant = assert_eq(int(search.get("count", 0)), 0, "mismatched search filter should return no lobbies")
	if err != null: return err
	return ok({ "filter": _eq_filter("string_key1", other) })


func run_lobby_search_multiple_lobbies(orch) -> Dictionary:
	var gate: Variant = requires_live_write(orch)
	if gate != null: return gate
	var signed: Variant = await _sign_in_roles(orch, ["host", "observer"])
	if _is_failure(signed): return signed
	var token: String = _unique_token(orch, "search-multiple")
	var first: Variant = await _create_lobby(orch, "host", "a", _public_lobby_config(4, { "string_key1": token, "string_key2": "a" }, {}, _role_member_properties("host")))
	if _is_failure(first): return first
	var second: Variant = await _create_lobby(orch, "host", "b", _public_lobby_config(4, { "string_key1": token, "string_key2": "b" }, {}, _role_member_properties("host")))
	if _is_failure(second): return second
	var search: Variant = await _search_lobbies(orch, "observer", _eq_filter("string_key1", token), 10)
	if _is_failure(search): return search
	var missing: Array = []
	for lobby_id in [String(first.get("lobby_id", "")), String(second.get("lobby_id", ""))]:
		if not _search_contains_lobby_id(search, lobby_id):
			missing.append(lobby_id)
	var err: Variant = assert_true(missing.is_empty(), "multiple matching lobbies should all appear", { "missing": missing, "search": search })
	if err != null: return err
	return ok({ "count": int(search.get("count", 0)) })


func run_lobby_search_private_not_searchable(orch) -> Dictionary:
	var gate: Variant = requires_live_write(orch)
	if gate != null: return gate
	var signed: Variant = await _sign_in_roles(orch, ["host", "observer"])
	if _is_failure(signed): return signed
	var token: String = _unique_token(orch, "private-search")
	var lobby: Variant = await _create_lobby(orch, "host", "main", _private_lobby_config(4, { "string_key1": token }, {}, _role_member_properties("host")))
	if _is_failure(lobby): return lobby
	var search: Variant = await _search_lobbies(orch, "observer", _eq_filter("string_key1", token))
	if _is_failure(search): return search
	var err: Variant = assert_true(not _search_contains_lobby_id(search, String(lobby.get("lobby_id", ""))), "private lobby should not be searchable", { "search": search, "lobby": lobby })
	if err != null: return err
	return ok()


func run_lobby_properties_lobby_propagation(orch) -> Dictionary:
	var gate: Variant = requires_live_write(orch)
	if gate != null: return gate
	var setup: Variant = await _create_join_lobby(orch, ["host", "guest", "guest2"], "main", 0, 4)
	if _is_failure(setup): return setup
	var token: String = _unique_token(orch, "lobby-prop")
	var set_result: Variant = await _command_ok(orch, "host", "set_lobby_properties", { "handle": "main", "properties": { "round": token } }, COMMAND_TIMEOUT_MS)
	if _is_failure(set_result): return set_result
	for role in ["host", "guest", "guest2"]:
		var waited: Variant = await _wait_lobby_property(orch, role, "main", "round", token)
		if _is_failure(waited): return waited
	return ok({ "round": token })


func run_lobby_properties_member_propagation(orch) -> Dictionary:
	var gate: Variant = requires_live_write(orch)
	if gate != null: return gate
	var setup: Variant = await _create_join_lobby(orch, ["host", "guest", "guest2"], "main", 0, 4)
	if _is_failure(setup): return setup
	var token: String = _unique_token(orch, "member-prop")
	var set_result: Variant = await _command_ok(orch, "guest", "set_member_properties", { "handle": "main", "properties": { "ready": token } }, COMMAND_TIMEOUT_MS)
	if _is_failure(set_result): return set_result
	for role in ["host", "guest", "guest2"]:
		var waited: Variant = await _wait_member_property(orch, role, "main", "guest", "ready", token)
		if _is_failure(waited): return waited
	return ok({ "ready": token })


func run_lobby_leave_client(orch) -> Dictionary:
	var gate: Variant = requires_live_write(orch)
	if gate != null: return gate
	var setup: Variant = await _create_join_lobby(orch, ["host", "guest"])
	if _is_failure(setup): return setup
	var left: Variant = await _leave_lobby(orch, "guest", "main")
	if _is_failure(left): return left
	var host_lobby: Variant = await _wait_lobby_member_count(orch, "host", "main", 1)
	if _is_failure(host_lobby): return host_lobby
	return ok()


func run_lobby_leave_third_member(orch) -> Dictionary:
	var gate: Variant = requires_live_write(orch)
	if gate != null: return gate
	var setup: Variant = await _create_join_lobby(orch, ["host", "guest", "guest2"], "main", 0, 4)
	if _is_failure(setup): return setup
	var left: Variant = await _leave_lobby(orch, "guest2", "main")
	if _is_failure(left): return left
	for role in ["host", "guest"]:
		var lobby: Variant = await _wait_lobby_member_count(orch, role, "main", 2)
		if _is_failure(lobby): return lobby
		var err: Variant = assert_true(_member_for_role(lobby, "guest2").is_empty(), "guest2 should no longer appear in lobby", { "role": role, "lobby": lobby })
		if err != null: return err
	return ok()


func run_lobby_leave_rejoin_after_leave(orch) -> Dictionary:
	var gate: Variant = requires_live_write(orch)
	if gate != null: return gate
	var setup: Variant = await _create_join_lobby(orch, ["host", "guest"])
	if _is_failure(setup): return setup
	var connection_string: String = String(setup.get("connection_string", ""))
	var left: Variant = await _leave_lobby(orch, "guest", "main")
	if _is_failure(left): return left
	var rejoined: Variant = await _join_lobby(orch, "guest", "main", connection_string, _role_member_properties("guest"))
	if _is_failure(rejoined): return rejoined
	var host_lobby: Variant = await _wait_lobby_member_count(orch, "host", "main", 2)
	if _is_failure(host_lobby): return host_lobby
	return ok()


func run_lobby_leave_host_owner_migration(orch) -> Dictionary:
	var gate: Variant = requires_live_write(orch)
	if gate != null: return gate
	var setup: Variant = await _create_join_lobby(orch, ["host", "guest"])
	if _is_failure(setup): return setup
	var left: Variant = await _leave_lobby(orch, "host", "main")
	if _is_failure(left): return left
	var guest_lobby: Variant = await _wait_lobby_owner_role(orch, "guest", "main", "guest")
	if _is_failure(guest_lobby): return guest_lobby
	return ok({ "owner": guest_lobby.get("owner_entity_key", {}) })


func run_lobby_join_invalid_connection_string(orch) -> Dictionary:
	var gate: Variant = requires_live(orch)
	if gate != null: return gate
	var signed: Variant = await _sign_in_roles(orch, ["guest"])
	if _is_failure(signed): return signed
	var err: Variant = await _expect_command_error(orch, "guest", "join_lobby", { "as": "bad", "connection_string": "not-a-valid-connection-string" }, [])
	if _is_failure(err): return err
	return ok()


func run_lobby_join_empty_connection_string(orch) -> Dictionary:
	var gate: Variant = requires_live(orch)
	if gate != null: return gate
	var signed: Variant = await _sign_in_roles(orch, ["guest"])
	if _is_failure(signed): return signed
	var err: Variant = await _expect_command_error(orch, "guest", "join_lobby", { "as": "empty", "connection_string": "" }, ["invalid_connection_string"])
	if _is_failure(err): return err
	return ok()


func run_lobby_create_invalid_max_players_zero(orch) -> Dictionary:
	var gate: Variant = requires_live_write(orch)
	if gate != null: return gate
	var signed: Variant = await _sign_in_roles(orch, ["host"])
	if _is_failure(signed): return signed
	var err: Variant = await _expect_command_error(orch, "host", "create_lobby", { "as": "bad", "config": _public_lobby_config(0, {}, {}, _role_member_properties("host")) }, [])
	if _is_failure(err): return err
	return ok()


func run_lobby_properties_set_unjoined_lobby(orch) -> Dictionary:
	var gate: Variant = requires_live_write(orch)
	if gate != null: return gate
	var setup: Variant = await _create_join_lobby(orch, ["host", "guest"])
	if _is_failure(setup): return setup
	var left: Variant = await _leave_lobby(orch, "guest", "main")
	if _is_failure(left): return left
	var err: Variant = await _expect_command_error(orch, "guest", "set_lobby_properties", { "handle": "main", "properties": { "late": "no" } }, ["unknown_handle"])
	if _is_failure(err): return err
	return ok()


func run_lobby_properties_member_set_unjoined_member(orch) -> Dictionary:
	var gate: Variant = requires_live(orch)
	if gate != null: return gate
	var signed: Variant = await _sign_in_roles(orch, ["guest"])
	if _is_failure(signed): return signed
	var err: Variant = await _expect_command_error(orch, "guest", "set_member_properties", { "handle": "missing", "properties": { "ready": "no" } }, ["unknown_handle"])
	if _is_failure(err): return err
	return ok()


func run_lobby_create_unsigned_in_user(orch) -> Dictionary:
	var err: Variant = await _expect_command_error(orch, "host", "create_lobby", { "as": "bad", "config": _public_lobby_config(4) }, ["not_signed_in"])
	if _is_failure(err): return err
	return ok()


func run_lobby_search_invalid_filter_string(orch) -> Dictionary:
	var gate: Variant = requires_live(orch)
	if gate != null: return gate
	var signed: Variant = await _sign_in_roles(orch, ["observer"])
	if _is_failure(signed): return signed
	var err: Variant = await _expect_command_error(orch, "observer", "search_lobbies", { "filter": "string_key1 === 'bad'" }, [])
	if _is_failure(err): return err
	return ok()


func run_lobby_state_create_join_leave_full_cycle(orch) -> Dictionary:
	var gate: Variant = requires_live_write(orch)
	if gate != null: return gate
	var setup: Variant = await _create_join_lobby(orch, ["host", "guest"])
	if _is_failure(setup): return setup
	var left: Variant = await _leave_lobby(orch, "guest", "main")
	if _is_failure(left): return left
	var removed: Variant = await _wait_event(_client(orch, "host"), "lobby.member_removed", { "member.properties.role": "guest" }, LOBBY_WAIT_MS)
	if _is_failure(removed): return removed
	var host_lobby: Variant = await _wait_lobby_member_count(orch, "host", "main", 1)
	if _is_failure(host_lobby): return host_lobby
	return ok()


func run_lobby_state_owner_migration_event_ordering(orch) -> Dictionary:
	var gate: Variant = requires_live_write(orch)
	if gate != null: return gate
	var setup: Variant = await _create_join_lobby(orch, ["host", "guest"])
	if _is_failure(setup): return setup
	var left: Variant = await _leave_lobby(orch, "host", "main")
	if _is_failure(left): return left
	var removed: Variant = await _wait_event(_client(orch, "guest"), "lobby.member_removed", { "member.properties.role": "host" }, LOBBY_WAIT_MS)
	if _is_failure(removed): return removed
	var owner: Variant = await _wait_event(_client(orch, "guest"), "lobby.owner_changed", {}, LOBBY_WAIT_MS)
	if _is_failure(owner): return owner
	var guest_lobby: Variant = await _wait_lobby_owner_role(orch, "guest", "main", "guest")
	if _is_failure(guest_lobby): return guest_lobby
	# _wait_event returns the waiter result shape { ok, event, timed_out }
	# (see _wait_event in mp_scenario_utils.gd); ts_ms lives on the inner
	# "event" dict, not the top-level waiter result.
	return ok({
		"removed_ts": int(removed.get("event", {}).get("ts_ms", 0)),
		"owner_ts": int(owner.get("event", {}).get("ts_ms", 0)),
	})


func run_lobby_chaos_host_kill_owner_migration(orch) -> Dictionary:
	var gate: Variant = requires_live_write(orch)
	if gate != null: return gate
	var setup: Variant = await _create_join_lobby(orch, ["host", "guest"])
	if _is_failure(setup): return setup
	_client(orch, "host").disconnect_client("scenario_host_kill")
	var guest_lobby: Variant = await _wait_lobby_owner_role(orch, "guest", "main", "guest", 120_000)
	if _is_failure(guest_lobby): return guest_lobby
	return ok()


func run_lobby_chaos_client_kill_member_removed(orch) -> Dictionary:
	var gate: Variant = requires_live_write(orch)
	if gate != null: return gate
	var setup: Variant = await _create_join_lobby(orch, ["host", "guest"])
	if _is_failure(setup): return setup
	_client(orch, "guest").disconnect_client("scenario_guest_kill")
	var host_lobby: Variant = await _wait_lobby_member_count(orch, "host", "main", 1, 120_000)
	if _is_failure(host_lobby): return host_lobby
	return ok()


func run_lobby_tracking_multiple_lobbies_per_host(orch) -> Dictionary:
	var gate: Variant = requires_live_write(orch)
	if gate != null: return gate
	var signed: Variant = await _sign_in_roles(orch, ["host"])
	if _is_failure(signed): return signed
	var a: Variant = await _create_lobby(orch, "host", "a", _public_lobby_config(2, { "string_key1": _unique_token(orch, "track-a") }, {}, _role_member_properties("host")))
	if _is_failure(a): return a
	var b: Variant = await _create_lobby(orch, "host", "b", _public_lobby_config(2, { "string_key1": _unique_token(orch, "track-b") }, {}, _role_member_properties("host")))
	if _is_failure(b): return b
	var err: Variant = assert_true(String(a.get("lobby_id", "")) != String(b.get("lobby_id", "")), "tracked lobbies should have distinct ids", { "a": a, "b": b })
	if err != null: return err
	var left: Variant = await _leave_lobby(orch, "host", "a")
	if _is_failure(left): return left
	var snap_b: Variant = await _lobby_snapshot(orch, "host", "b")
	if _is_failure(snap_b): return snap_b
	err = assert_eq(String(snap_b.get("lobby_id", "")), String(b.get("lobby_id", "")), "remaining handle should still address lobby b")
	if err != null: return err
	return ok()
