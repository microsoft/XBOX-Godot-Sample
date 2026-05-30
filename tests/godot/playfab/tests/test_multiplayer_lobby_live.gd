extends "res://addons/godot_gdk_tests/playfab_test_base.gd"
## Live regression coverage for the PlayFab Multiplayer lobby dispatcher.
##
## These tests reproduce the bug class fixed in commits 5fecbc5 ("addon(playfab):
## fix use-after-free + null member in lobby completion handlers") and 4778144
## ("addon(playfab): emit DISCONNECTED (not MEMBER_REMOVED) on LeaveLobbyCompleted").
## The bugs only manifest when the live PFLobby SDK actually dispatches state
## changes for a real lobby, so non-live contract tests cannot exercise them.
##
## Gated by `pending_unless_live()`; these are write tests in the sense that
## they create a lobby in the configured PlayFab sandbox title. Lobby cleanup
## is best effort — leave_async() runs in the happy path; the shutdown test
## deliberately skips leave to exercise the shutdown-during-active-lobby race.
## Stale lobbies in a sandbox title age out via the PFLobby SDK TTL.
##
## Required configuration (matches existing live PlayFab tests):
##   - LIVE_TESTS=1 in env
##   - playfab/runtime/title_id set
##   - playfab/tests/custom_id (or PLAYFAB_CUSTOM_ID env) set so the run signs
##     in deterministically.

const _DEFAULT_OP_TIMEOUT_MSEC := 60000
const _STATE_PUMP_FRAMES := 30


func test_lobby_member_props_and_leave_state_signals() -> void:
	var session = await _begin_multiplayer_session()
	var playfab_user = session.get("playfab_user")
	if playfab_user == null:
		return

	var playfab: Object = session["playfab"]
	var multiplayer: Object = session["multiplayer"]

	var lobby_config = instantiate_class("PlayFabLobbyConfig")
	assert_object_is(lobby_config, "PlayFabLobbyConfig", "PlayFabLobbyConfig instantiable for live create")
	if lobby_config == null:
		_finish_session(playfab, null)
		return

	lobby_config.max_players = 4
	lobby_config.access_policy = get_class_constant("PlayFabLobbyConfig", "ACCESS_POLICY_PRIVATE")
	lobby_config.member_properties = {"role": "owner"}

	var service_changes: Array = []
	var on_service_change = func(change): service_changes.append(change)
	multiplayer.state_changed.connect(on_service_change)

	var create_result = await await_completion(multiplayer.create_lobby_async(playfab_user, lobby_config), _DEFAULT_OP_TIMEOUT_MSEC)
	if create_result == null or not create_result.ok:
		multiplayer.state_changed.disconnect(on_service_change)
		pending("PlayFab.multiplayer.create_lobby_async failed: %s" % (create_result.message if create_result != null else "null result"))
		_finish_session(playfab, null)
		return

	var lobby: Object = create_result.data
	assert_object_is(lobby, "PlayFabLobby", "create_lobby_async returns PlayFabLobby")
	if lobby == null:
		multiplayer.state_changed.disconnect(on_service_change)
		_finish_session(playfab, null)
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

	_finish_session(playfab, null)


func test_lobby_shutdown_without_leave_does_not_emit_null_member() -> void:
	var session = await _begin_multiplayer_session()
	var playfab_user = session.get("playfab_user")
	if playfab_user == null:
		return

	var playfab: Object = session["playfab"]
	var multiplayer: Object = session["multiplayer"]

	var lobby_config = instantiate_class("PlayFabLobbyConfig")
	if lobby_config == null:
		_finish_session(playfab, null)
		return
	lobby_config.max_players = 4
	lobby_config.access_policy = get_class_constant("PlayFabLobbyConfig", "ACCESS_POLICY_PRIVATE")

	var create_result = await await_completion(multiplayer.create_lobby_async(playfab_user, lobby_config), _DEFAULT_OP_TIMEOUT_MSEC)
	if create_result == null or not create_result.ok:
		pending("PlayFab.multiplayer.create_lobby_async failed: %s" % (create_result.message if create_result != null else "null result"))
		_finish_session(playfab, null)
		return

	var lobby: Object = create_result.data
	if lobby == null:
		_finish_session(playfab, null)
		return

	var lobby_changes: Array = []
	var on_lobby_change = func(change): lobby_changes.append(change)
	lobby.state_changed.connect(on_lobby_change)

	# Shutdown WITHOUT leaving — same path as
	# sample/tutorial_app/addons/godot_playfab/runtime/playfab_bootstrap.gd
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

	var playfab: Object = session["playfab"]
	var multiplayer: Object = session["multiplayer"]

	var lobby_config = instantiate_class("PlayFabLobbyConfig")
	if lobby_config == null:
		_finish_session(playfab, null)
		return
	lobby_config.max_players = 2
	lobby_config.access_policy = get_class_constant("PlayFabLobbyConfig", "ACCESS_POLICY_PRIVATE")

	var create_result = await await_completion(multiplayer.create_lobby_async(playfab_user, lobby_config), _DEFAULT_OP_TIMEOUT_MSEC)
	if create_result == null or not create_result.ok:
		pending("PlayFab.multiplayer.create_lobby_async failed: %s" % (create_result.message if create_result != null else "null result"))
		_finish_session(playfab, null)
		return

	var lobby: Object = create_result.data
	if lobby == null:
		_finish_session(playfab, null)
		return

	var stale_connection_string := str(lobby.get_connection_string())
	var leave_result = await await_completion(lobby.leave_async(), _DEFAULT_OP_TIMEOUT_MSEC)
	if leave_result == null or not leave_result.ok:
		pending("PlayFabLobby.leave_async failed before stale-join regression check: %s" % (leave_result.message if leave_result != null else "null result"))
		_finish_session(playfab, null)
		return
	await advance_process_frames(_STATE_PUMP_FRAMES)

	var before_count := multiplayer.get_lobbies().size()
	var join_config = instantiate_class("PlayFabLobbyJoinConfig")
	var join_result = await await_completion(multiplayer.join_lobby_async(playfab_user, stale_connection_string, join_config), _DEFAULT_OP_TIMEOUT_MSEC)
	if join_result == null:
		pending("PlayFab.multiplayer.join_lobby_async(stale_connection_string) timed out; cannot assert failure cleanup.")
		_finish_session(playfab, null)
		return
	if join_result.ok:
		var joined_lobby = join_result.data
		if joined_lobby != null:
			await await_completion(joined_lobby.leave_async(), _DEFAULT_OP_TIMEOUT_MSEC)
		pending("PlayFab service accepted a stale lobby connection string; failure-cleanup path was not exercised.")
		_finish_session(playfab, null)
		return

	await advance_process_frames(_STATE_PUMP_FRAMES)
	assert_eq(multiplayer.get_lobbies().size(), before_count,
			"failed join completion does not leave its PlayFabLobby wrapper tracked")

	_finish_session(playfab, null)


# ── Live setup helpers ────────────────────────────────────────────────────

func _begin_multiplayer_session() -> Dictionary:
	var outcome := {
		"playfab_user": null,
		"playfab": null,
		"multiplayer": null,
	}

	if pending_unless_live():
		return outcome
	if pending_unless_playfab_available():
		return outcome

	var playfab: Object = get_playfab()
	outcome["playfab"] = playfab

	var configured_title_id := str(ProjectSettings.get_setting(PLAYFAB_TITLE_ID_SETTING, "")).strip_edges()
	if configured_title_id.is_empty():
		pending("Set ProjectSettings['playfab/runtime/title_id'] to exercise live PlayFab Multiplayer.")
		return outcome

	reset_playfab_runtime()
	var init_result = playfab.initialize()
	if init_result == null or not init_result.ok:
		pending("PlayFab.initialize() live setup skipped: %s" % (init_result.message if init_result != null else "null result"))
		return outcome

	var custom_id_session = await _sign_in_with_or_create_custom_id(playfab, "PlayFab multiplayer lobby live test")
	if custom_id_session.get("playfab_user") == null:
		return outcome

	var multiplayer: Object = playfab.get_multiplayer()
	if multiplayer == null:
		pending("PlayFab.get_multiplayer() returned null in live session.")
		return outcome

	var mp_init = await await_completion(multiplayer.initialize_async(), _DEFAULT_OP_TIMEOUT_MSEC)
	if mp_init == null or not mp_init.ok:
		pending("PlayFab.multiplayer.initialize_async failed: %s" % (mp_init.message if mp_init != null else "null result"))
		return outcome

	outcome["playfab_user"] = custom_id_session["playfab_user"]
	outcome["multiplayer"] = multiplayer
	return outcome


# Local sign-in helper. The shared sign_in_with_configured_custom_id() in the
# base passes create_account=false (other live suites depend on a
# preconfigured account from tools/configure_playfab_test_title.ps1). The
# multiplayer regression coverage is happy to create-on-demand because it
# only needs an entity-token-bearing user; pass create_account=true so the
# tests work against any sandbox title without separate pre-provisioning.
func _sign_in_with_or_create_custom_id(playfab: Object, label: String) -> Dictionary:
	var outcome := {"playfab_user": null}
	var custom_id := get_configured_playfab_custom_id()
	if custom_id.is_empty():
		pending("Set ProjectSettings['playfab/tests/custom_id'] or PLAYFAB_CUSTOM_ID to exercise %s." % label)
		return outcome
	var sign_in_signal = playfab.users.sign_in_with_custom_id_async(custom_id, true)
	if typeof(sign_in_signal) != TYPE_SIGNAL:
		pending("%s skipped: PlayFab.users.sign_in_with_custom_id_async() did not start." % label)
		return outcome
	var sign_in_result = await await_completion(sign_in_signal, _DEFAULT_OP_TIMEOUT_MSEC)
	if sign_in_result == null or not sign_in_result.ok:
		pending("%s skipped: %s" % [label, sign_in_result.message if sign_in_result != null else "timed out"])
		return outcome
	outcome["playfab_user"] = sign_in_result.data
	return outcome


func _finish_session(playfab: Object, _ignored) -> void:
	if playfab != null:
		playfab.shutdown()


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
