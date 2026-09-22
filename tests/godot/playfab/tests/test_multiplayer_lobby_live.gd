extends "res://addons/godot_gdk_tests/playfab_test_base.gd"
## Live regression coverage for the PlayFab Multiplayer lobby dispatcher.
##
## These tests reproduce the bug class fixed in commits 5fecbc5 ("addon(playfab):
## fix use-after-free + null member in lobby completion handlers") and 4778144
## ("addon(playfab): emit DISCONNECTED (not MEMBER_REMOVED) on LeaveLobbyCompleted").
## The bugs only manifest when the live PFLobby SDK actually dispatches state
## changes for a real lobby, so non-live contract tests cannot exercise them.
##
## Gated by `requires_live_write()` because these tests create and mutate
## lobbies in the configured PlayFab sandbox title. Lobby cleanup
## is best effort — leave_async() runs in the happy path; the shutdown test
## deliberately skips leave to exercise the shutdown-during-active-lobby race.
## Stale lobbies in a sandbox title age out via the PFLobby SDK TTL.
##
## Required configuration (matches existing live PlayFab tests):
##   - LIVE_TESTS=1 and LIVE_WRITE_TESTS=1 in env
##   - playfab/runtime/title_id set
##   - playfab/tests/custom_id (or PLAYFAB_CUSTOM_ID env) set so the run signs
##     in deterministically.

const _DEFAULT_OP_TIMEOUT_MSEC := 60000
const _STATE_PUMP_FRAMES := 30
const _E_INVALIDARG_HRESULT := 0x80070057
const _ARRANGED_CAPACITY_ERROR_MESSAGE := "PlayFabLobbyJoinConfig.max_member_count must be between 2 and 128."
const _ARRANGED_ACCESS_ERROR_MESSAGE := "PlayFabLobbyJoinConfig.access_policy must be ACCESS_POLICY_PUBLIC, ACCESS_POLICY_FRIENDS, or ACCESS_POLICY_PRIVATE."
const _ARRANGED_MIGRATION_ERROR_MESSAGE := "PlayFabLobbyJoinConfig.owner_migration_policy must be OWNER_MIGRATION_AUTOMATIC, OWNER_MIGRATION_MANUAL, or OWNER_MIGRATION_NONE."


func after_each() -> void:
	var playfab: Object = get_playfab()
	if playfab == null or not playfab.is_initialized():
		return
	var multiplayer: Object = playfab.get_multiplayer()
	if multiplayer == null or not multiplayer.is_initialized():
		return

	var lobbies: Array = multiplayer.get_lobbies()
	for lobby in lobbies:
		if lobby == null:
			continue
		var leave_result = await await_completion(
			lobby.leave_async(), _DEFAULT_OP_TIMEOUT_MSEC)
		if leave_result == null:
			push_warning("Timed out while cleaning up a live-test lobby.")
		elif not leave_result.ok:
			push_warning("Failed to clean up a live-test lobby: %s" % leave_result.message)
	if not lobbies.is_empty():
		await advance_process_frames(_STATE_PUMP_FRAMES)


func test_lobby_member_props_and_leave_state_signals() -> void:
	var session = await _begin_multiplayer_session()
	var playfab_user = session.get("playfab_user")
	if playfab_user == null:
		return

	var multiplayer: Object = session["multiplayer"]

	var lobby_config = instantiate_class("PlayFabLobbyConfig")
	assert_object_is(lobby_config, "PlayFabLobbyConfig", "PlayFabLobbyConfig instantiable for live create")
	if lobby_config == null:
		return

	lobby_config.max_players = 4
	lobby_config.access_policy = get_class_constant("PlayFabLobbyConfig", "ACCESS_POLICY_PRIVATE")
	lobby_config.member_properties = {"role": "owner"}

	var service_changes: Array = []
	var on_service_change = func(change): service_changes.append(change)
	multiplayer.state_changed.connect(on_service_change)

	var create_result = await await_completion(multiplayer.create_lobby_async(playfab_user, lobby_config), _DEFAULT_OP_TIMEOUT_MSEC)
	if create_result == null:
		multiplayer.state_changed.disconnect(on_service_change)
		fail_test("PlayFab.multiplayer.create_lobby_async timed out.")
		return
	if not create_result.ok:
		multiplayer.state_changed.disconnect(on_service_change)
		pending("PlayFab.multiplayer.create_lobby_async failed: %s" % create_result.message)
		return

	var lobby: Object = create_result.data
	assert_object_is(lobby, "PlayFabLobby", "create_lobby_async returns PlayFabLobby")
	assert_true(lobby != null and lobby.is_owner(playfab_user), "PlayFabLobby.is_owner(playfab_user) reports true for creating user")
	if lobby == null:
		multiplayer.state_changed.disconnect(on_service_change)
		return

	var lobby_changes: Array = []
	var on_lobby_change = func(change): lobby_changes.append(change)
	lobby.state_changed.connect(on_lobby_change)

	# Step 1: set_member_properties_async should emit MEMBER_UPDATED with a
	# non-null member. The pre-fix PostUpdateCompleted handler read
	# operation->user AFTER _complete_pending_operation deleted the op (UAF)
	# AND _set_member_properties_async never populated operation->user, so the
	# emitted change carried member=null.
	var props_result = await await_completion(lobby.set_member_properties_async({"ready": "true"}), _DEFAULT_OP_TIMEOUT_MSEC)
	assert_true(props_result != null and props_result.ok,
			"lobby.set_member_properties_async succeeds (%s)" % (props_result.message if props_result != null else "null"))
	await advance_process_frames(_STATE_PUMP_FRAMES)

	_assert_no_null_member_payload(lobby_changes, "after set_member_properties_async")
	_assert_kind_emitted_with_member(lobby_changes, get_class_constant("PlayFabLobby", "MEMBER_UPDATED"), playfab_user, "set_member_properties_async")

	# Step 2: leave_async. Pre-fix LeaveLobbyCompleted emitted MEMBER_REMOVED a
	# second time (duplicating the SDK's per-member MemberRemoved for the local
	# user) and with change.member=null because refresh_snapshot had already
	# dropped the leaving user. Post-fix it emits DISCONNECTED instead.
	var prior_lobby_change_count := lobby_changes.size()
	var leave_result = await await_completion(lobby.leave_async(), _DEFAULT_OP_TIMEOUT_MSEC)
	assert_true(leave_result != null and leave_result.ok,
			"lobby.leave_async succeeds (%s)" % (leave_result.message if leave_result != null else "null"))
	await advance_process_frames(_STATE_PUMP_FRAMES)

	var leave_phase: Array = lobby_changes.slice(prior_lobby_change_count)
	_assert_no_null_member_payload(leave_phase, "during leave_async")
	_assert_kind_emitted(leave_phase, get_class_constant("PlayFabLobby", "DISCONNECTED"), "leave_async emitted DISCONNECTED")
	_assert_member_removed_count_at_most_one(leave_phase, playfab_user, "leave_async")

	if lobby.is_connected("state_changed", on_lobby_change):
		lobby.state_changed.disconnect(on_lobby_change)
	if multiplayer.is_connected("state_changed", on_service_change):
		multiplayer.state_changed.disconnect(on_service_change)


func test_lobby_local_member_properties_converge_after_live_write() -> void:
	var session = await _begin_multiplayer_session()
	var playfab_user = session.get("playfab_user")
	if playfab_user == null:
		return

	var multiplayer: Object = session["multiplayer"]

	var lobby_config = instantiate_class("PlayFabLobbyConfig")
	if lobby_config == null:
		return
	lobby_config.max_players = 2
	lobby_config.access_policy = get_class_constant("PlayFabLobbyConfig", "ACCESS_POLICY_PRIVATE")
	lobby_config.member_properties = {"ready": "false", "role": "owner"}

	var create_result = await await_completion(multiplayer.create_lobby_async(playfab_user, lobby_config), _DEFAULT_OP_TIMEOUT_MSEC)
	if create_result == null:
		fail("PlayFab.multiplayer.create_lobby_async timed out before local member property convergence check.")
		return
	if not create_result.ok:
		pending("PlayFab.multiplayer.create_lobby_async failed before local member property convergence check: %s" % create_result.message)
		return

	var lobby: Object = create_result.data
	if lobby == null:
		return

	# Subscribe BEFORE the local-self write so the MEMBER_UPDATED change the
	# dispatcher emits for the patched local snapshot lands in our buffer.
	# Copilot review (PR #30) flagged that the offline contract test exercised
	# the helper directly; this section asserts the production
	# PostUpdateCompleted dispatcher path drives the same merge by inspecting
	# the change.member it emits.
	var lobby_changes: Array = []
	var on_lobby_change = func(change): lobby_changes.append(change)
	lobby.state_changed.connect(on_lobby_change)

	var props_result = await await_completion(lobby.set_member_properties_async({"ready": "true", "team": "blue", "role": null}), _DEFAULT_OP_TIMEOUT_MSEC)
	assert_true(props_result != null and props_result.ok,
			"lobby.set_member_properties_async succeeds for local member snapshot convergence (%s)" % (props_result.message if props_result != null else "null"))
	if props_result == null or not props_result.ok:
		if lobby.is_connected("state_changed", on_lobby_change):
			lobby.state_changed.disconnect(on_lobby_change)
		return

	await advance_process_frames(_STATE_PUMP_FRAMES)

	var local_props: Variant = _get_local_member_properties(lobby)
	assert_eq(typeof(local_props), TYPE_DICTIONARY, "local member properties are available immediately after set_member_properties_async")
	if typeof(local_props) == TYPE_DICTIONARY:
		assert_eq(String(local_props.get("ready", "")), "true", "local ready property converged after live write")
		assert_eq(String(local_props.get("team", "")), "blue", "local team property converged after live write")
		assert_false(local_props.has("role"), "local role property deletion converged after live write")

	# Dispatcher-path coverage: the MEMBER_UPDATED change the addon emits for
	# the local self after the write completes must carry change.member with the
	# patched properties. If a future refactor severs the
	# PostUpdateCompleted → _apply_local_member_property_update wiring the
	# convergence helpers above could still pass via SDK snapshot, but this
	# block would fail because the dispatched change.member would not reflect
	# the local-side merge.
	_assert_member_updated_carries_local_properties(lobby_changes, playfab_user,
			{"ready": "true", "team": "blue"}, ["role"],
			"set_member_properties_async dispatcher")

	if lobby.is_connected("state_changed", on_lobby_change):
		lobby.state_changed.disconnect(on_lobby_change)


func test_lobby_shutdown_without_leave_does_not_emit_null_member() -> void:
	var session = await _begin_multiplayer_session()
	var playfab_user = session.get("playfab_user")
	if playfab_user == null:
		return

	var playfab: Object = session["playfab"]
	var multiplayer: Object = session["multiplayer"]

	var lobby_config = instantiate_class("PlayFabLobbyConfig")
	if lobby_config == null:
		return
	lobby_config.max_players = 4
	lobby_config.access_policy = get_class_constant("PlayFabLobbyConfig", "ACCESS_POLICY_PRIVATE")

	var create_result = await await_completion(multiplayer.create_lobby_async(playfab_user, lobby_config), _DEFAULT_OP_TIMEOUT_MSEC)
	if create_result == null:
		fail_test("PlayFab.multiplayer.create_lobby_async timed out.")
		return
	if not create_result.ok:
		pending("PlayFab.multiplayer.create_lobby_async failed: %s" % create_result.message)
		return

	var lobby: Object = create_result.data
	if lobby == null:
		return

	var lobby_changes: Array = []
	var on_lobby_change = func(change): lobby_changes.append(change)
	lobby.state_changed.connect(on_lobby_change)

	# Shutdown WITHOUT leaving — same path as
	# sample/tutorial_integrated/addons/godot_playfab/runtime/playfab_bootstrap.gd
	# _exit_tree() when the player closes the window mid-lobby. The pre-fix
	# LeaveLobbyCompleted handler emitted MEMBER_REMOVED with change.member=null
	# here, which crashed sample listeners on `change.member.user_id`.
	playfab.shutdown()
	await advance_process_frames(_STATE_PUMP_FRAMES)

	_assert_no_null_member_payload(lobby_changes, "during playfab.shutdown() with active lobby")
	_assert_kind_emitted(lobby_changes, get_class_constant("PlayFabLobby", "DISCONNECTED"), "shutdown emits DISCONNECTED for active lobby")

	# The lobby reference is now detached; no further teardown needed.


func test_failed_lobby_join_does_not_leave_tracked_wrapper() -> void:
	var session = await _begin_multiplayer_session()
	var playfab_user = session.get("playfab_user")
	if playfab_user == null:
		return

	var multiplayer: Object = session["multiplayer"]

	var lobby_config = instantiate_class("PlayFabLobbyConfig")
	if lobby_config == null:
		return
	lobby_config.max_players = 2
	lobby_config.access_policy = get_class_constant("PlayFabLobbyConfig", "ACCESS_POLICY_PRIVATE")

	var create_result = await await_completion(multiplayer.create_lobby_async(playfab_user, lobby_config), _DEFAULT_OP_TIMEOUT_MSEC)
	if create_result == null:
		fail_test("PlayFab.multiplayer.create_lobby_async timed out.")
		return
	if not create_result.ok:
		pending("PlayFab.multiplayer.create_lobby_async failed: %s" % create_result.message)
		return

	var lobby: Object = create_result.data
	if lobby == null:
		return

	var stale_connection_string := str(lobby.get_connection_string())
	var leave_result = await await_completion(lobby.leave_async(), _DEFAULT_OP_TIMEOUT_MSEC)
	if leave_result == null:
		fail_test("PlayFabLobby.leave_async timed out before stale-join regression check.")
		return
	if not leave_result.ok:
		pending("PlayFabLobby.leave_async failed before stale-join regression check: %s" % leave_result.message)
		return
	await advance_process_frames(_STATE_PUMP_FRAMES)

	var before_count: int = multiplayer.get_lobbies().size()
	var join_config = instantiate_class("PlayFabLobbyJoinConfig")
	var join_result = await await_completion(multiplayer.join_lobby_async(playfab_user, stale_connection_string, join_config), _DEFAULT_OP_TIMEOUT_MSEC)
	if join_result == null:
		fail_test("PlayFab.multiplayer.join_lobby_async(stale_connection_string) timed out; cannot assert failure cleanup.")
		return
	if join_result.ok:
		var joined_lobby = join_result.data
		if joined_lobby != null:
			await await_completion(joined_lobby.leave_async(), _DEFAULT_OP_TIMEOUT_MSEC)
		pending("PlayFab service accepted a stale lobby connection string; failure-cleanup path was not exercised.")
		return

	await advance_process_frames(_STATE_PUMP_FRAMES)
	assert_eq(multiplayer.get_lobbies().size(), before_count,
			"failed join completion does not leave its PlayFabLobby wrapper tracked")


func test_multiplayer_live_validation_error_branches() -> void:
	var session = await _begin_multiplayer_session()
	var playfab_user = session.get("playfab_user")
	if playfab_user == null:
		return

	var multiplayer: Object = session["multiplayer"]

	var join_config = instantiate_class("PlayFabLobbyJoinConfig")
	await _assert_signal_error(
		multiplayer.join_arranged_lobby_async(playfab_user, "  ", join_config),
		"invalid_arranged_lobby_connection_string",
		"PlayFab.multiplayer.join_arranged_lobby_async() rejects blank arranged connection string")

	var lobby_config = instantiate_class("PlayFabLobbyConfig")
	if lobby_config != null:
		lobby_config.lobby_properties = {"bad_value": 12}
		await _assert_signal_error(
			multiplayer.create_lobby_async(playfab_user, lobby_config),
			"invalid_properties",
			"PlayFab.multiplayer.create_lobby_async() rejects non-string lobby property values")

	# Arranged-lobby initialization is rejected, never clamped. Each case uses a
	# fresh config so a rejected field cannot mask the next one.
	var rejected_lobby_count: int = multiplayer.get_lobbies().size()
	for case in [
		{
			"field": "max_member_count",
			"value": 0,
			"name": "capacity below PFLobbyMaxMemberCountLowerLimit",
			"message": _ARRANGED_CAPACITY_ERROR_MESSAGE,
		},
		{
			"field": "max_member_count",
			"value": 1,
			"name": "capacity just below PFLobbyMaxMemberCountLowerLimit",
			"message": _ARRANGED_CAPACITY_ERROR_MESSAGE,
		},
		{
			"field": "max_member_count",
			"value": -1,
			"name": "negative capacity",
			"message": _ARRANGED_CAPACITY_ERROR_MESSAGE,
		},
		{
			"field": "max_member_count",
			"value": 129,
			"name": "capacity above PFLobbyMaxMemberCountUpperLimit",
			"message": _ARRANGED_CAPACITY_ERROR_MESSAGE,
		},
		{
			"field": "max_member_count",
			"value": 2147483648,
			"name": "capacity above the signed 32-bit range",
			"message": _ARRANGED_CAPACITY_ERROR_MESSAGE,
		},
		{
			"field": "max_member_count",
			"value": 4294967304,
			"name": "capacity that narrows to valid uint32 value 8",
			"message": _ARRANGED_CAPACITY_ERROR_MESSAGE,
		},
		{
			"field": "max_member_count",
			"value": 9223372036854775807,
			"name": "capacity at the signed 64-bit ceiling",
			"message": _ARRANGED_CAPACITY_ERROR_MESSAGE,
		},
		{
			"field": "access_policy",
			"value": 99,
			"name": "unrecognized access policy",
			"message": _ARRANGED_ACCESS_ERROR_MESSAGE,
		},
		{
			"field": "owner_migration_policy",
			"value": 3,
			"name": "server owner-migration policy",
			"message": _ARRANGED_MIGRATION_ERROR_MESSAGE,
		},
	]:
		var invalid_config = instantiate_class("PlayFabLobbyJoinConfig")
		if invalid_config == null:
			return
		invalid_config.set(case["field"], case["value"])
		await _assert_signal_error_exact(
			multiplayer.join_arranged_lobby_async(playfab_user, "arranged-connection-string", invalid_config),
			"invalid_arranged_lobby_config",
			String(case["message"]),
			"PlayFab.multiplayer.join_arranged_lobby_async() rejects %s" % case["name"])
		assert_eq(
			multiplayer.get_lobbies().size(),
			rejected_lobby_count,
			"rejected arranged join does not add a tracked lobby for %s" % case["name"])

	# Member-property validation remains earlier than arranged scalar validation.
	var invalid_properties_config = instantiate_class("PlayFabLobbyJoinConfig")
	if invalid_properties_config == null:
		return
	invalid_properties_config.member_properties = {"bad_value": 12}
	invalid_properties_config.max_member_count = 0
	invalid_properties_config.access_policy = 99
	invalid_properties_config.owner_migration_policy = 3
	await _assert_signal_error(
		multiplayer.join_arranged_lobby_async(playfab_user, "arranged-connection-string", invalid_properties_config),
		"invalid_properties",
		"PlayFab.multiplayer.join_arranged_lobby_async() preserves member-property validation precedence")
	assert_eq(
		multiplayer.get_lobbies().size(),
		rejected_lobby_count,
		"bad arranged member properties do not add a tracked lobby")


# ── Live setup helpers ────────────────────────────────────────────────────

func _begin_multiplayer_session() -> Dictionary:
	var outcome = await begin_playfab_live_session(
		"Live PlayFab Multiplayer lobby",
		true,
		false,
		true,
		true,
		_DEFAULT_OP_TIMEOUT_MSEC)
	if outcome.get("playfab_user") == null:
		return {}

	var playfab: Object = outcome.get("playfab")
	if playfab == null:
		fail("PlayFab live session returned no runtime.")
		return {}

	var multiplayer: Object = outcome.get("multiplayer")
	if multiplayer == null:
		multiplayer = playfab.get_multiplayer()
	if multiplayer == null:
		fail("PlayFab.get_multiplayer() returned null in live session.")
		return {}

	if not multiplayer.is_initialized():
		var mp_init = await await_completion(
			multiplayer.initialize_async(), _DEFAULT_OP_TIMEOUT_MSEC)
		if mp_init == null:
			fail_test("PlayFab.multiplayer.initialize_async timed out.")
			return {}
		if not mp_init.ok:
			fail("PlayFab.multiplayer.initialize_async failed: %s" % mp_init.message)
			return {}

	outcome["multiplayer"] = multiplayer
	return outcome


# ── Lobby-change assertions ───────────────────────────────────────────────

func _assert_no_null_member_payload(changes: Array, phase: String) -> void:
	var member_added = get_class_constant("PlayFabLobby", "MEMBER_ADDED")
	var member_updated = get_class_constant("PlayFabLobby", "MEMBER_UPDATED")
	var member_removed = get_class_constant("PlayFabLobby", "MEMBER_REMOVED")
	var offenders: Array = []
	for change in changes:
		var kind: int = change.get_kind()
		var member = change.get_member()
		var result = change.get_result()
		var result_ok: bool = result == null or result.ok
		if not result_ok:
			continue
		if kind == member_added or kind == member_updated or kind == member_removed:
			if member == null:
				offenders.append(str(kind))
	assert_eq(offenders.size(), 0,
			"member-scoped state changes carry non-null change.member %s (offenders kinds=%s)" % [phase, str(offenders)])


func _assert_kind_emitted(changes: Array, expected_kind: int, name: String) -> void:
	for change in changes:
		if change.get_kind() == expected_kind:
			assert_true(true, name)
			return
	assert_true(false, "%s (no PlayFabLobbyStateChange.kind=%d in %d recorded changes)" % [name, expected_kind, changes.size()])


func _assert_kind_emitted_with_member(changes: Array, expected_kind: int, playfab_user, op_label: String) -> void:
	var expected_id := str(playfab_user.entity_key.get("id", ""))
	for change in changes:
		if change.get_kind() != expected_kind:
			continue
		var member = change.get_member()
		if member == null:
			continue
		var member_id := str(member.entity_key.get("id", ""))
		if member_id == expected_id:
			assert_true(true, "%s emitted kind=%d with change.member matching local user" % [op_label, expected_kind])
			return
	assert_true(false,
			"%s did not emit kind=%d with non-null change.member matching local user (recorded %d changes)" % [op_label, expected_kind, changes.size()])


func _get_local_member_properties(lobby: Object) -> Variant:
	if lobby == null:
		return null
	for member in lobby.get_members():
		if member != null and bool(member.is_local_member()):
			return member.get_properties()
	return null


func _assert_signal_error(async_signal, expected_code: String, name: String) -> void:
	assert_eq(typeof(async_signal), TYPE_SIGNAL, "%s returns completion Signal" % name)
	if typeof(async_signal) != TYPE_SIGNAL:
		return
	assert_playfab_result_error(await await_completion(async_signal, _DEFAULT_OP_TIMEOUT_MSEC), expected_code, name)


func _assert_signal_error_exact(
		async_signal,
		expected_code: String,
		expected_message: String,
		name: String) -> void:
	assert_eq(typeof(async_signal), TYPE_SIGNAL, "%s returns completion Signal" % name)
	if typeof(async_signal) != TYPE_SIGNAL:
		return
	var result = await await_completion(async_signal, _DEFAULT_OP_TIMEOUT_MSEC)
	assert_playfab_result_error(result, expected_code, name)
	if result == null:
		return
	assert_eq(String(result.message), expected_message, "%s exact error message" % name)
	assert_eq(
		int(result.hresult) & 0xFFFFFFFF,
		_E_INVALIDARG_HRESULT,
		"%s HRESULT is E_INVALIDARG" % name)


func _assert_member_removed_count_at_most_one(changes: Array, playfab_user, op_label: String) -> void:
	var member_removed = get_class_constant("PlayFabLobby", "MEMBER_REMOVED")
	var expected_id := str(playfab_user.entity_key.get("id", ""))
	var count := 0
	for change in changes:
		if change.get_kind() != member_removed:
			continue
		var member = change.get_member()
		if member == null:
			continue
		var member_id := str(member.entity_key.get("id", ""))
		if member_id == expected_id:
			count += 1
	assert_true(count <= 1,
			"%s emitted MEMBER_REMOVED for the local user at most once (saw %d). >1 would indicate the LeaveLobbyCompleted duplicate-signal regression." % [op_label, count])


func _assert_member_updated_carries_local_properties(changes: Array, playfab_user, expected_present: Dictionary, expected_absent: Array, op_label: String) -> void:
	# Look for a MEMBER_UPDATED for the local user whose change.member snapshot
	# reflects the patched properties (Copilot review on PR #30: the live
	# dispatcher path must carry the local-side merge through to
	# change.member.properties, not just into the lobby's member list).
	var member_updated = get_class_constant("PlayFabLobby", "MEMBER_UPDATED")
	var expected_id := str(playfab_user.entity_key.get("id", ""))
	for i in range(changes.size() - 1, -1, -1):
		var change = changes[i]
		if change.get_kind() != member_updated:
			continue
		var member = change.get_member()
		if member == null:
			continue
		var member_id := str(member.entity_key.get("id", ""))
		if member_id != expected_id:
			continue
		var props: Variant = member.get_properties()
		if typeof(props) != TYPE_DICTIONARY:
			continue
		var matches := true
		for k in expected_present.keys():
			if String(props.get(k, "")) != String(expected_present[k]):
				matches = false
				break
		if not matches:
			continue
		for k in expected_absent:
			if props.has(k):
				matches = false
				break
		if not matches:
			continue
		assert_true(true,
				"%s emitted MEMBER_UPDATED whose change.member.properties carries the patched local-self snapshot" % op_label)
		return
	assert_true(false,
			"%s did not emit MEMBER_UPDATED carrying the patched local-self snapshot (recorded %d changes; expected present=%s absent=%s)" %
					[op_label, changes.size(), str(expected_present), str(expected_absent)])
