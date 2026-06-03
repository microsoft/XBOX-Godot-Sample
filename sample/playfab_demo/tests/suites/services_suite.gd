extends RefCounted


func run(context) -> void:
	_test_game_saves_contract(context)
	_test_leaderboards_contract(context)


func _test_game_saves_contract(context) -> void:
	context.log_section("PlayFab Game Saves API")

	var playfab = context.get_playfab()
	if playfab == null:
		context.log_fail("PlayFab singleton missing, skipping Game Saves API group")
		return

	var game_saves = playfab.get_game_saves()
	context.assert_object_is(game_saves, "PlayFabGameSaves", "PlayFab.get_game_saves() returns PlayFabGameSaves")
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
		context.assert_has_method(game_saves, method_name)

	context.assert_eq(ClassDB.class_get_integer_constant("PlayFabGameSaves", "ADD_USER_OPTION_NONE"), 0, "PlayFabGameSaves.ADD_USER_OPTION_NONE == 0")

	context.reset_playfab_runtime()
	var blank_user = context.instantiate_class("PlayFabUser")
	var add_user_signal = game_saves.add_user_with_ui_async(blank_user)
	await context.assert_signal_result_error(add_user_signal, "not_initialized", "PlayFab.game_saves.add_user_with_ui_async() before initialize()")

	var folder_result = game_saves.get_folder(blank_user)
	context.assert_result_error(folder_result, "not_initialized", "PlayFab.game_saves.get_folder() before initialize()")


func _test_leaderboards_contract(context) -> void:
	context.log_section("PlayFab Leaderboards API")

	var playfab = context.get_playfab()
	if playfab == null:
		context.log_fail("PlayFab singleton missing, skipping leaderboards API group")
		return

	var leaderboards = playfab.get_leaderboards()
	context.assert_object_is(leaderboards, "PlayFabLeaderboards", "PlayFab.get_leaderboards() returns PlayFabLeaderboards")
	if leaderboards == null:
		return

	for method_name in [
		"submit_score_async",
		"get_leaderboard_async",
		"get_leaderboard_around_user_async",
		"get_friend_leaderboard_async",
	]:
		context.assert_has_method(leaderboards, method_name)

	context.reset_playfab_runtime()
	var blank_user = context.instantiate_class("PlayFabUser")

	var submit_signal = leaderboards.submit_score_async(blank_user, "contract_suite", 42)
	await context.assert_signal_result_error(submit_signal, "not_initialized", "PlayFab.leaderboards.submit_score_async() before initialize()")

	var query_signal = leaderboards.get_leaderboard_async(blank_user, "contract_suite")
	await context.assert_signal_result_error(query_signal, "not_initialized", "PlayFab.leaderboards.get_leaderboard_async() before initialize()")
