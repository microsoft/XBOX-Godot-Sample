extends RefCounted

const PLAYFAB_TITLE_ID_SETTING := "playfab/titleid"
const PLAYFAB_ENDPOINT_SETTING := "playfab/endpoint"
const PLAYFAB_EMBED_DISPATCH_SETTING := "playfab/runtime/embed_dispatch"


func run(context) -> void:
	_test_singleton_availability(context)
	_test_class_registration(context)
	_test_root_api(context)


func _test_singleton_availability(context) -> void:
	context.log_section("Singleton Availability")

	var playfab = context.get_playfab()
	context.assert_not_null(playfab, "Engine.get_singleton('PlayFab')")
	context.assert_not_null(context.get_gdk(), "Engine.get_singleton('GDK')")


func _test_class_registration(context) -> void:
	context.log_section("Class Registration")

	for registered_class in [
		"PlayFab",
		"PlayFabUsers",
		"PlayFabUser",
		"PlayFabGameSaves",
		"PlayFabLeaderboards",
		"PlayFabResult",
	]:
		context.assert_true(ClassDB.class_exists(registered_class), "%s registered in ClassDB" % registered_class)

	context.assert_true(ClassDB.is_parent_class("PlayFab", "Object"), "PlayFab extends Object")
	context.assert_true(ClassDB.is_parent_class("PlayFabUsers", "RefCounted"), "PlayFabUsers extends RefCounted")
	context.assert_true(ClassDB.is_parent_class("PlayFabUser", "RefCounted"), "PlayFabUser extends RefCounted")
	context.assert_true(ClassDB.is_parent_class("PlayFabGameSaves", "RefCounted"), "PlayFabGameSaves extends RefCounted")
	context.assert_true(ClassDB.is_parent_class("PlayFabLeaderboards", "RefCounted"), "PlayFabLeaderboards extends RefCounted")
	context.assert_true(ClassDB.is_parent_class("PlayFabResult", "RefCounted"), "PlayFabResult extends RefCounted")


func _test_root_api(context) -> void:
	context.log_section("PlayFab Root API")

	var playfab = context.get_playfab()
	if playfab == null:
		context.log_fail("PlayFab root singleton missing, skipping root API group")
		return

	context.reset_playfab_runtime()

	for method_name in [
		"initialize",
		"shutdown",
		"is_available",
		"is_initialized",
		"dispatch",
		"get_last_error",
		"get_users",
		"get_game_saves",
		"get_leaderboards",
		"sign_in_async",
		"get_user_by_local_id",
		"get_title_id",
		"get_endpoint",
	]:
		context.assert_has_method(playfab, method_name)

	for signal_name in ["initialized", "shutdown_completed", "runtime_error"]:
		context.assert_has_signal(playfab, signal_name)

	context.assert_object_is(playfab.get_users(), "PlayFabUsers", "PlayFab.users returns PlayFabUsers")
	context.assert_object_is(playfab.get_game_saves(), "PlayFabGameSaves", "PlayFab.game_saves returns PlayFabGameSaves")
	context.assert_object_is(playfab.get_leaderboards(), "PlayFabLeaderboards", "PlayFab.leaderboards returns PlayFabLeaderboards")
	context.assert_true(playfab.is_available() is bool, "PlayFab.is_available() returns bool")
	context.assert_eq(playfab.is_initialized(), false, "PlayFab.is_initialized() starts false")
	context.assert_eq(playfab.dispatch(), 0, "PlayFab.dispatch() is safe before init")
	context.assert_eq(playfab.get_title_id(), "", "PlayFab.get_title_id() is empty before init")
	context.assert_eq(playfab.get_endpoint(), "", "PlayFab.get_endpoint() is empty before init")

	context.assert_true(ProjectSettings.has_setting(PLAYFAB_TITLE_ID_SETTING), "playfab/titleid project setting registered")
	context.assert_eq(String(context.get_setting_default(PLAYFAB_TITLE_ID_SETTING)), "", "playfab/titleid default remains blank")
	context.assert_true(ProjectSettings.has_setting(PLAYFAB_ENDPOINT_SETTING), "playfab/endpoint project setting registered")
	context.assert_eq(String(context.get_setting_default(PLAYFAB_ENDPOINT_SETTING)), "", "playfab/endpoint default remains blank")
	context.assert_true(ProjectSettings.has_setting(PLAYFAB_EMBED_DISPATCH_SETTING), "playfab/runtime/embed_dispatch project setting registered")
	context.assert_eq(bool(context.get_setting_default(PLAYFAB_EMBED_DISPATCH_SETTING)), true, "playfab/runtime/embed_dispatch defaults to true")

	var last_error = playfab.get_last_error()
	context.assert_not_null(last_error, "PlayFab.get_last_error() returns PlayFabResult")
	if last_error != null:
		context.assert_eq(last_error.ok, true, "PlayFab.get_last_error() starts clear")

	var initialized_events: Array = []
	var runtime_errors: Array = []
	var initialized_handler := func() -> void:
		initialized_events.append(true)
	var runtime_error_handler := func(result) -> void:
		runtime_errors.append(result)

	playfab.initialized.connect(initialized_handler)
	playfab.runtime_error.connect(runtime_error_handler)

	var original_title_id = ProjectSettings.get_setting(PLAYFAB_TITLE_ID_SETTING, "")
	var original_endpoint = ProjectSettings.get_setting(PLAYFAB_ENDPOINT_SETTING, "")
	ProjectSettings.set_setting(PLAYFAB_TITLE_ID_SETTING, "")
	ProjectSettings.set_setting(PLAYFAB_ENDPOINT_SETTING, "")

	var init_result = playfab.initialize()

	ProjectSettings.set_setting(PLAYFAB_TITLE_ID_SETTING, original_title_id)
	ProjectSettings.set_setting(PLAYFAB_ENDPOINT_SETTING, original_endpoint)

	context.assert_result_error(init_result, "title_id_required", "PlayFab.initialize() rejects blank playfab/titleid")

	var current_last_error = playfab.get_last_error()
	context.assert_result_error(current_last_error, "title_id_required", "PlayFab.get_last_error() tracks blank title id failures")
	context.assert_eq(initialized_events.size(), 0, "PlayFab.initialized is not emitted for blank title id")
	context.assert_eq(runtime_errors.size(), 1, "PlayFab.runtime_error is emitted for blank title id")

	if playfab.initialized.is_connected(initialized_handler):
		playfab.initialized.disconnect(initialized_handler)
	if playfab.runtime_error.is_connected(runtime_error_handler):
		playfab.runtime_error.disconnect(runtime_error_handler)
