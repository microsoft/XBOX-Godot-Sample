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


# ---------------------------------------------------------------------------
# Typed lobby updates (post_update_async and its single-field helpers)
# ---------------------------------------------------------------------------

## A batched post_update_async must complete with the neutral
## UPDATE_COMPLETED kind, never with PROPERTIES_UPDATED. Regression coverage
## for the completion kind being hard-coded to PROPERTIES_UPDATED, which made
## every typed update announce itself as a lobby-property change.
func run_lobby_update_post_update_typed_completion(orch) -> Dictionary:
	var gate: Variant = requires_live_write(orch)
	if gate != null: return gate
	var setup: Variant = await _create_join_lobby(orch, ["host", "guest"])
	if _is_failure(setup): return setup
	var token: String = _unique_token(orch, "batch-update")
	var posted: Variant = await _command_ok(orch, "host", "post_lobby_update", {
		"handle": "main",
		"update": {
			"lobby_properties": { "round": token },
			"membership_lock": 1,
		},
	}, COMMAND_TIMEOUT_MS)
	if _is_failure(posted): return posted

	var completed: Variant = await _wait_event(_client(orch, "host"), "lobby.update_completed", { "result.ok": true }, LOBBY_WAIT_MS)
	if _is_failure(completed): return completed
	var err: Variant = assert_eq(int(completed.get("event", {}).get("payload", {}).get("kind", -1)),
			11, "post_update_async completion should use UPDATE_COMPLETED (11)")
	if err != null: return err

	# Both batched fields must actually land, proving the neutral completion
	# kind did not come at the cost of applying the update.
	var prop: Variant = await _wait_lobby_property(orch, "guest", "main", "round", token)
	if _is_failure(prop): return prop
	var locked: Variant = await _wait_lobby_field(orch, "guest", "main", "membership_lock", 1)
	if _is_failure(locked): return locked
	return ok({ "round": token })


## set_membership_lock_async completes as CONFIGURATION_UPDATED, propagates to
## every member, and actually seals the lobby against new joins.
func run_lobby_update_membership_lock(orch) -> Dictionary:
	var gate: Variant = requires_live_write(orch)
	if gate != null: return gate
	var setup: Variant = await _create_join_lobby(orch, ["host", "guest"], "main", 0, 4)
	if _is_failure(setup): return setup
	var connection_string: String = String(setup.get("connection_string", ""))

	var locked_result: Variant = await _command_ok(orch, "host", "set_membership_lock", { "handle": "main", "membership_lock": 1 }, COMMAND_TIMEOUT_MS)
	if _is_failure(locked_result): return locked_result

	# `result.ok` is the discriminator that matters. PlayFab broadcasts
	# CONFIGURATION_UPDATED to every member regardless of who posted, and only
	# the posting client's *completion* carries a PlayFabResult. Without this
	# filter the wait would be satisfied by the broadcast and the scenario
	# would still pass if the completion kind regressed.
	var configured: Variant = await _wait_event(_client(orch, "host"), "lobby.configuration_updated", { "result.ok": true }, LOBBY_WAIT_MS)
	if _is_failure(configured): return configured
	var err: Variant = assert_eq(int(configured.get("event", {}).get("payload", {}).get("kind", -1)),
			9, "set_membership_lock_async completion should use CONFIGURATION_UPDATED (9)")
	if err != null: return err

	for role in ["host", "guest"]:
		var seen: Variant = await _wait_lobby_field(orch, role, "main", "membership_lock", 1)
		if _is_failure(seen): return seen

	# A locked lobby must reject new joins.
	var signed: Variant = await _sign_in_roles(orch, ["guest2"])
	if _is_failure(signed): return signed
	var join: Dictionary = await _command(orch, "guest2", "join_lobby", {
		"as": "main",
		"connection_string": connection_string,
		"member_properties": _role_member_properties("guest2"),
	}, COMMAND_TIMEOUT_MS)
	err = assert_true(not bool(join.get("ok", false)), "join should be rejected while the lobby is locked", { "join": join })
	if err != null: return err

	# Unlocking must be observable too, so the lock is not a one-way door.
	var unlocked_result: Variant = await _command_ok(orch, "host", "set_membership_lock", { "handle": "main", "membership_lock": 0 }, COMMAND_TIMEOUT_MS)
	if _is_failure(unlocked_result): return unlocked_result
	var unlocked: Variant = await _wait_lobby_field(orch, "guest", "main", "membership_lock", 0)
	if _is_failure(unlocked): return unlocked
	return ok()


## set_search_properties_async completes as SEARCH_PROPERTIES_UPDATED and the
## new values reach every member. Asserted against the member-visible snapshot
## rather than find_lobbies_async, which lags behind by search-index
## propagation and would make this scenario flaky.
func run_lobby_update_search_properties(orch) -> Dictionary:
	var gate: Variant = requires_live_write(orch)
	if gate != null: return gate
	var setup: Variant = await _create_join_lobby(orch, ["host", "guest"])
	if _is_failure(setup): return setup
	var token: String = _unique_token(orch, "search-update")
	var set_result: Variant = await _command_ok(orch, "host", "set_search_properties", {
		"handle": "main",
		"search_properties": { "string_key2": token },
	}, COMMAND_TIMEOUT_MS)
	if _is_failure(set_result): return set_result

	# Filtering on result.ok isolates the poster's completion from the
	# SEARCH_PROPERTIES_UPDATED broadcast every member receives; only the
	# completion carries a PlayFabResult.
	var updated: Variant = await _wait_event(_client(orch, "host"), "lobby.search_properties_updated", { "result.ok": true }, LOBBY_WAIT_MS)
	if _is_failure(updated): return updated
	var err: Variant = assert_eq(int(updated.get("event", {}).get("payload", {}).get("kind", -1)),
			8, "set_search_properties_async completion should use SEARCH_PROPERTIES_UPDATED (8)")
	if err != null: return err

	for role in ["host", "guest"]:
		var seen: Variant = await _wait_lobby_search_property(orch, role, "main", "string_key2", token)
		if _is_failure(seen): return seen
	return ok({ "string_key2": token })


## A deliberate leave must surface DISCONNECTING with
## DISCONNECTING_NO_LOCAL_MEMBERS (0) and a successful DISCONNECTED result.
## This is what proves the runtime can distinguish an intentional leave from
## an unexpected disconnect, which DISCONNECTED's result now depends on.
func run_lobby_disconnect_reason_on_leave(orch) -> Dictionary:
	var gate: Variant = requires_live_write(orch)
	if gate != null: return gate
	var setup: Variant = await _create_join_lobby(orch, ["host", "guest"])
	if _is_failure(setup): return setup
	# keep_events stops the client from detaching the state_changed
	# subscription when leave_async completes; DISCONNECTING/DISCONNECTED
	# arrive after that completion and would otherwise be dropped.
	var left: Variant = await _command_ok(orch, "guest", "leave_lobby", { "handle": "main", "keep_events": true }, COMMAND_TIMEOUT_MS)
	if _is_failure(left): return left

	var disconnecting: Variant = await _wait_event(_client(orch, "guest"), "lobby.disconnecting", {}, LOBBY_WAIT_MS)
	if _is_failure(disconnecting): return disconnecting
	var disconnecting_payload: Dictionary = disconnecting.get("event", {}).get("payload", {})
	var err: Variant = assert_eq(int(disconnecting_payload.get("reason", -1)), 0,
			"a deliberate leave should report DISCONNECTING_NO_LOCAL_MEMBERS (0)")
	if err != null: return err

	var disconnected: Variant = await _wait_event(_client(orch, "guest"), "lobby.disconnected", {}, LOBBY_WAIT_MS)
	if _is_failure(disconnected): return disconnected
	var disconnected_payload: Dictionary = disconnected.get("event", {}).get("payload", {})
	err = assert_eq(int(disconnected_payload.get("reason", -1)), 0,
			"DISCONNECTED should replay the cached DISCONNECTING reason")
	if err != null: return err
	err = assert_true(bool(disconnected_payload.get("result", {}).get("ok", false)),
			"DISCONNECTED after a deliberate leave should report a successful result", { "disconnected": disconnected_payload })
	if err != null: return err
	return ok()
