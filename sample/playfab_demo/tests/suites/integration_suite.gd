extends RefCounted

const PLAYFAB_TITLE_ID_SETTING := "playfab/titleid"
const PLAYFAB_ENDPOINT_SETTING := "playfab/endpoint"


func run(context) -> void:
	_test_optional_live_sign_in(context)


func _test_optional_live_sign_in(context) -> void:
	context.log_section("PlayFab Configured Init + Sign-In Smoke")

	var playfab = context.get_playfab()
	if playfab == null:
		context.log_fail("PlayFab singleton missing, skipping live smoke group")
		return

	var configured_title_id := str(ProjectSettings.get_setting(PLAYFAB_TITLE_ID_SETTING, "")).strip_edges()
	if configured_title_id.is_empty():
		context.log_skip("PlayFab live init/sign-in smoke", "Set ProjectSettings['playfab/titleid'] to exercise the live PlayFab flow.")
		return

	var configured_endpoint := str(ProjectSettings.get_setting(PLAYFAB_ENDPOINT_SETTING, "")).strip_edges()
	context.reset_playfab_runtime()

	var initialized_events: Array = []
	var shutdown_events: Array = []
	var initialized_handler := func() -> void:
		initialized_events.append(true)
	var shutdown_handler := func() -> void:
		shutdown_events.append(true)
	playfab.initialized.connect(initialized_handler)
	playfab.shutdown_completed.connect(shutdown_handler)

	var init_result = playfab.initialize()
	context.assert_not_null(init_result, "PlayFab.initialize() returns PlayFabResult")
	if init_result == null:
		_disconnect_handlers(playfab, initialized_handler, shutdown_handler)
		return
	if not init_result.ok:
		context.log_skip("PlayFab.initialize() live smoke", init_result.message)
		_disconnect_handlers(playfab, initialized_handler, shutdown_handler)
		return

	context.assert_eq(initialized_events.size(), 1, "PlayFab.initialized emits once during live init")
	context.assert_eq(playfab.is_initialized(), true, "PlayFab runtime initializes when configured")
	context.assert_eq(playfab.get_title_id(), configured_title_id, "PlayFab.get_title_id() reflects configured title id")

	var expected_endpoint := configured_endpoint if not configured_endpoint.is_empty() else "https://%s.playfabapi.com" % configured_title_id
	context.assert_eq(playfab.get_endpoint(), expected_endpoint, "PlayFab.get_endpoint() resolves the configured endpoint")

	var gdk_outcome = await context.ensure_gdk_primary_user()
	var xbox_user = gdk_outcome.get("user")
	if xbox_user == null:
		context.log_skip("PlayFab sign-in smoke", str(gdk_outcome.get("skip_reason", "No signed-in Xbox user available.")))
		playfab.shutdown()
		context.assert_eq(shutdown_events.size(), 1, "PlayFab.shutdown_completed emits during live smoke cleanup")
		_disconnect_handlers(playfab, initialized_handler, shutdown_handler)
		return

	var sign_in_signal = playfab.sign_in_async(xbox_user)
	context.assert_true(typeof(sign_in_signal) == TYPE_SIGNAL, "PlayFab.sign_in_async() starts for a signed-in Xbox user")
	if typeof(sign_in_signal) != TYPE_SIGNAL:
		playfab.shutdown()
		context.assert_eq(shutdown_events.size(), 1, "PlayFab.shutdown_completed emits after failed live smoke start")
		_disconnect_handlers(playfab, initialized_handler, shutdown_handler)
		return

	var sign_in_result = await context.wait_for_signal(sign_in_signal)
	if sign_in_result == null:
		context.log_skip("PlayFab sign-in smoke", "Timed out waiting for the PlayFab sign-in request.")
		playfab.shutdown()
		context.assert_eq(shutdown_events.size(), 1, "PlayFab.shutdown_completed emits after live smoke timeout")
		_disconnect_handlers(playfab, initialized_handler, shutdown_handler)
		return
	if not sign_in_result.ok:
		context.log_skip("PlayFab sign-in smoke", sign_in_result.message)
		playfab.shutdown()
		context.assert_eq(shutdown_events.size(), 1, "PlayFab.shutdown_completed emits after live smoke skip")
		_disconnect_handlers(playfab, initialized_handler, shutdown_handler)
		return

	var playfab_user = sign_in_result.data
	context.assert_object_is(playfab_user, "PlayFabUser", "PlayFab sign-in returns PlayFabUser")
	if playfab_user != null:
		var local_id := int(playfab_user.local_id)
		context.assert_true(local_id > 0, "PlayFabUser.local_id is populated", str(local_id))

		var entity_key: Dictionary = playfab_user.entity_key
		context.assert_true(not str(entity_key.get("id", "")).is_empty(), "PlayFabUser.entity_key.id is populated")
		context.assert_true(not str(entity_key.get("type", "")).is_empty(), "PlayFabUser.entity_key.type is populated")
		context.assert_not_null(playfab.get_user_by_local_id(local_id), "PlayFab.get_user_by_local_id() returns the cached signed-in user")
		context.assert_true(playfab.get_users().get_users().size() >= 1, "PlayFab.users cache tracks the signed-in session")

		var last_error = playfab.get_last_error()
		if last_error != null:
			context.assert_eq(last_error.ok, true, "PlayFab.get_last_error() clears after successful sign-in")

	playfab.shutdown()
	context.assert_eq(playfab.is_initialized(), false, "PlayFab.shutdown() returns the runtime to the uninitialized state")
	context.assert_eq(shutdown_events.size(), 1, "PlayFab.shutdown_completed emits once after live smoke")
	_disconnect_handlers(playfab, initialized_handler, shutdown_handler)


func _disconnect_handlers(playfab, initialized_handler: Callable, shutdown_handler: Callable) -> void:
	if playfab.initialized.is_connected(initialized_handler):
		playfab.initialized.disconnect(initialized_handler)
	if playfab.shutdown_completed.is_connected(shutdown_handler):
		playfab.shutdown_completed.disconnect(shutdown_handler)
