extends "res://addons/godot_gdk_tests/playfab_test_base.gd"
## Wave 3 migration of the previous `tests/suites/services_suite.gd`.
##
## Covers the `PlayFab.game_saves` and `PlayFab.leaderboards` service
## contracts: method exposure, exported constants, and the
## not-initialized failure path on async + sync entry points.


func test_game_saves_contract() -> void:
	if pending_unless_playfab_available():
		return
	var playfab = get_playfab()

	var game_saves = playfab.get_game_saves()
	assert_object_is(game_saves, "PlayFabGameSaves", "PlayFab.get_game_saves() returns PlayFabGameSaves")
	if game_saves == null:
		return

	for method_name in [
		"add_user_with_ui_async",
		"upload_with_ui_async",
		"set_save_description_async",
		"reset_cloud_async",
		"get_folder",
		"get_folder_size",
		"get_remaining_quota",
		"is_connected_to_cloud",
	]:
		assert_has_method_named(game_saves, method_name)

	assert_eq(get_class_constant("PlayFabGameSaves", "ADD_USER_OPTION_NONE"), 0, "PlayFabGameSaves.ADD_USER_OPTION_NONE == 0")

	reset_playfab_runtime()
	var blank_user = instantiate_class("PlayFabUser")

	var add_user_signal = game_saves.add_user_with_ui_async(blank_user)
	await _assert_playfab_signal_result_error(
		add_user_signal, "not_initialized", "PlayFab.game_saves.add_user_with_ui_async() before initialize()")

	var folder_result = game_saves.get_folder(blank_user)
	assert_playfab_result_error(folder_result, "not_initialized", "PlayFab.game_saves.get_folder() before initialize()")


func test_leaderboards_contract() -> void:
	if pending_unless_playfab_available():
		return
	var playfab = get_playfab()

	var leaderboards = playfab.get_leaderboards()
	assert_object_is(leaderboards, "PlayFabLeaderboards", "PlayFab.get_leaderboards() returns PlayFabLeaderboards")
	if leaderboards == null:
		return

	for method_name in [
		"submit_score_async",
		"get_leaderboard_async",
		"get_leaderboard_around_user_async",
		"get_friend_leaderboard_async",
	]:
		assert_has_method_named(leaderboards, method_name)

	reset_playfab_runtime()
	var blank_user = instantiate_class("PlayFabUser")

	var submit_signal = leaderboards.submit_score_async(blank_user, "contract_suite", 42)
	await _assert_playfab_signal_result_error(
		submit_signal, "not_initialized", "PlayFab.leaderboards.submit_score_async() before initialize()")

	var query_signal = leaderboards.get_leaderboard_async(blank_user, "contract_suite")
	await _assert_playfab_signal_result_error(
		query_signal, "not_initialized", "PlayFab.leaderboards.get_leaderboard_async() before initialize()")


# Mirror of the previous `assert_signal_result_error`; routes through the
# PlayFab dual-pump `await_completion` and the PlayFabResult-labeled error
# assertion so failure messages point at the right type.
func _assert_playfab_signal_result_error(async_signal, expected_code: String, name: String) -> void:
	assert_eq(typeof(async_signal), TYPE_SIGNAL, "%s returns completion Signal" % name)
	if typeof(async_signal) != TYPE_SIGNAL:
		return
	assert_playfab_result_error(await await_completion(async_signal), expected_code, name)
