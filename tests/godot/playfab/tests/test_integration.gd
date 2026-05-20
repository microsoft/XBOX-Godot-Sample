extends "res://addons/godot_gdk_tests/playfab_test_base.gd"
## Wave 3 migration of the previous `tests/suites/integration_suite.gd`.
##
## Live PlayFab init + custom-ID sign-in smoke. Gated behind `LIVE_TESTS=1`,
## a configured `playfab/runtime/title_id`, and `playfab/tests/custom_id` (or the
## `PLAYFAB_CUSTOM_ID` environment variable). The flow is read-only.


func test_optional_live_sign_in() -> void:
	if pending_unless_live():
		return
	if pending_unless_playfab_available():
		return

	var playfab = get_playfab()

	var configured_title_id := str(ProjectSettings.get_setting(PLAYFAB_TITLE_ID_SETTING, "")).strip_edges()
	if configured_title_id.is_empty():
		pending("Set ProjectSettings['playfab/runtime/title_id'] to exercise the live PlayFab flow.")
		return

	var configured_endpoint := str(ProjectSettings.get_setting(PLAYFAB_ENDPOINT_SETTING, "")).strip_edges()
	reset_playfab_runtime()

	var initialized_events: Array = []
	var shutdown_events: Array = []
	var initialized_handler := func() -> void:
		initialized_events.append(true)
	var shutdown_handler := func() -> void:
		shutdown_events.append(true)
	playfab.initialized.connect(initialized_handler)
	playfab.shutdown_completed.connect(shutdown_handler)

	var init_result = playfab.initialize()
	assert_not_null(init_result, "PlayFab.initialize() returns PlayFabResult")
	if init_result == null:
		_disconnect_handlers(playfab, initialized_handler, shutdown_handler)
		return
	if not init_result.ok:
		pending("PlayFab.initialize() live smoke skipped: %s" % init_result.message)
		_disconnect_handlers(playfab, initialized_handler, shutdown_handler)
		return

	assert_eq(initialized_events.size(), 1, "PlayFab.initialized emits once during live init")
	assert_eq(playfab.is_initialized(), true, "PlayFab runtime initializes when configured")
	assert_eq(playfab.get_title_id(), configured_title_id, "PlayFab.get_title_id() reflects configured title id")

	var expected_endpoint := configured_endpoint if not configured_endpoint.is_empty() else "https://%s.playfabapi.com" % configured_title_id
	assert_eq(playfab.get_endpoint(), expected_endpoint, "PlayFab.get_endpoint() resolves the configured endpoint")

	var custom_id_session = await sign_in_with_configured_custom_id(playfab, "PlayFab custom-ID sign-in smoke")
	var playfab_user = custom_id_session.get("playfab_user")
	if playfab_user == null:
		playfab.shutdown()
		assert_eq(shutdown_events.size(), 1, "PlayFab.shutdown_completed emits during live smoke cleanup")
		_disconnect_handlers(playfab, initialized_handler, shutdown_handler)
		return

	assert_object_is(playfab_user, "PlayFabUser", "PlayFab sign-in returns PlayFabUser")
	if playfab_user != null:
		var local_id := int(playfab_user.local_id)
		assert_eq(local_id, 0, "Custom-ID PlayFabUser.local_id remains 0")
		assert_eq(str(playfab_user.custom_id), str(custom_id_session["custom_id"]), "PlayFabUser.custom_id reflects the configured custom ID")
		assert_false(playfab_user.has_local_user_handle(), "Custom-ID PlayFabUser has no local user handle")

		var entity_key: Dictionary = playfab_user.entity_key
		assert_true(not str(entity_key.get("id", "")).is_empty(), "PlayFabUser.entity_key.id is populated")
		assert_true(not str(entity_key.get("type", "")).is_empty(), "PlayFabUser.entity_key.type is populated")
		assert_eq(playfab.get_users().get_user_by_local_id(local_id), null, "PlayFab.users.get_user_by_local_id(0) does not return custom-ID sessions")
		assert_not_null(playfab.get_users().get_user_by_custom_id(playfab_user.custom_id), "PlayFab.users.get_user_by_custom_id() returns the cached signed-in user")
		assert_true(playfab.get_users().get_users().size() >= 1, "PlayFab.users cache tracks the signed-in session")

	playfab.shutdown()
	assert_eq(playfab.is_initialized(), false, "PlayFab.shutdown() returns the runtime to the uninitialized state")
	assert_eq(shutdown_events.size(), 1, "PlayFab.shutdown_completed emits once after live smoke")
	_disconnect_handlers(playfab, initialized_handler, shutdown_handler)


func _disconnect_handlers(playfab, initialized_handler: Callable, shutdown_handler: Callable) -> void:
	if playfab.initialized.is_connected(initialized_handler):
		playfab.initialized.disconnect(initialized_handler)
	if playfab.shutdown_completed.is_connected(shutdown_handler):
		playfab.shutdown_completed.disconnect(shutdown_handler)
