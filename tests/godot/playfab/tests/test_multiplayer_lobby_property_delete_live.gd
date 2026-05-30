extends "res://addons/godot_gdk_tests/playfab_test_base.gd"
## Live-write regression for PlayFabLobby null-valued property deletion.

const _DEFAULT_OP_TIMEOUT_MSEC := 60000
const _PROPERTY_CONVERGENCE_TIMEOUT_MSEC := 10000
const _STATE_PUMP_FRAMES := 30


func test_lobby_property_null_deletes_key() -> void:
	if not requires_live_write():
		return
	if pending_unless_playfab_available():
		return

	var playfab: Object = get_playfab()
	reset_playfab_runtime()

	var configured_title_id: String = get_active_playfab_title_id().strip_edges()
	if configured_title_id.is_empty():
		pending("Set ProjectSettings['playfab/runtime/title_id'] or PLAYFAB_TITLE_ID to exercise live PlayFab Multiplayer.")
		return

	var init_result = playfab.initialize()
	if init_result == null or not init_result.ok:
		pending("PlayFab.initialize() live setup skipped: %s" % (init_result.message if init_result != null else "null result"))
		return

	var sign_in = await sign_in_with_configured_custom_id(playfab, "PlayFab lobby null-delete regression", _DEFAULT_OP_TIMEOUT_MSEC)
	var playfab_user = sign_in.get("playfab_user")
	if playfab_user == null:
		_finish_session(playfab, null)
		return

	var multiplayer: Object = playfab.get_multiplayer()
	if multiplayer == null:
		pending("PlayFab.get_multiplayer() returned null in live null-delete regression.")
		_finish_session(playfab, null)
		return

	var mp_init = await await_completion(multiplayer.initialize_async(), _DEFAULT_OP_TIMEOUT_MSEC)
	if mp_init == null:
		fail_test("PlayFab.multiplayer.initialize_async timed out for null-delete regression.")
		_finish_session(playfab, null)
		return
	if not mp_init.ok:
		pending("PlayFab.multiplayer.initialize_async failed for null-delete regression: %s" % mp_init.message)
		_finish_session(playfab, null)
		return

	var lobby_config = instantiate_class("PlayFabLobbyConfig")
	if lobby_config == null:
		_finish_session(playfab, null)
		return
	lobby_config.max_players = 2
	lobby_config.access_policy = get_class_constant("PlayFabLobbyConfig", "ACCESS_POLICY_PRIVATE")

	var create_result = await await_completion(multiplayer.create_lobby_async(playfab_user, lobby_config), _DEFAULT_OP_TIMEOUT_MSEC)
	if create_result == null:
		fail_test("PlayFab.multiplayer.create_lobby_async timed out for null-delete regression.")
		_finish_session(playfab, null)
		return
	if not create_result.ok:
		pending("PlayFab.multiplayer.create_lobby_async failed for null-delete regression: %s" % create_result.message)
		_finish_session(playfab, null)
		return

	var lobby: Object = create_result.data
	assert_object_is(lobby, "PlayFabLobby", "create_lobby_async returns PlayFabLobby for null-delete regression")
	if lobby == null:
		_finish_session(playfab, null)
		return

	var write_result = await await_completion(lobby.set_properties_async({"a": "1", "b": "2"}), _DEFAULT_OP_TIMEOUT_MSEC)
	assert_true(write_result != null and write_result.ok,
			"lobby.set_properties_async writes baseline properties (%s)" % (write_result.message if write_result != null else "null"))

	var delete_result = null
	if write_result != null and write_result.ok:
		delete_result = await await_completion(lobby.set_properties_async({"a": null}), _DEFAULT_OP_TIMEOUT_MSEC)
		assert_true(delete_result != null and delete_result.ok,
				"lobby.set_properties_async treats null as delete (%s)" % (delete_result.message if delete_result != null else "null"))

	if delete_result != null and delete_result.ok:
		var props: Dictionary = await _wait_for_lobby_properties(lobby, {"a": null, "b": "2"})
		assert_false(props.has("a"), "null-valued lobby property update removes key a")
		assert_eq(String(props.get("b", "")), "2", "null-valued update leaves unrelated key b unchanged")
		assert_eq(props.size(), 1, "snapshot contains only the undeleted lobby property")

	var leave_result = await await_completion(lobby.leave_async(), _DEFAULT_OP_TIMEOUT_MSEC)
	assert_true(leave_result != null and leave_result.ok,
			"lobby.leave_async cleanup succeeds after null-delete regression (%s)" % (leave_result.message if leave_result != null else "null"))
	await advance_process_frames(_STATE_PUMP_FRAMES)
	_finish_session(playfab, null)


func _finish_session(playfab: Object, _ignored) -> void:
	if playfab != null:
		playfab.shutdown()


func _wait_for_lobby_properties(lobby: Object, expected: Dictionary, timeout_msec: int = _PROPERTY_CONVERGENCE_TIMEOUT_MSEC) -> Dictionary:
	var playfab: Object = get_playfab()
	var started_msec: int = Time.get_ticks_msec()
	while Time.get_ticks_msec() - started_msec < timeout_msec:
		if playfab != null:
			playfab.dispatch()
		var props: Dictionary = lobby.get_properties()
		if _lobby_properties_match(props, expected):
			return props
		await advance_process_frames(1)
	return lobby.get_properties()


func _lobby_properties_match(current: Dictionary, expected: Dictionary) -> bool:
	for key in expected.keys():
		var want: Variant = expected[key]
		if want == null:
			if current.has(key) and current[key] != null:
				return false
		elif String(current.get(key, "")) != String(want):
			return false
	return true
