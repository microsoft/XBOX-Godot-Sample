extends "res://addons/godot_gdk_tests/gdk_test_base.gd"


func before_each() -> void:
	reset_runtime()


func after_each() -> void:
	reset_runtime()


func test_game_save_surface_and_validation_paths() -> void:
	if pending_unless_runtime_available():
		return

	var gdk = get_gdk()
	var game_save = gdk.get_game_save()
	assert_not_null(game_save, "GDK.game_save returns service object")
	if game_save == null:
		return

	for method_name in [
		"get_folder_async",
		"get_remaining_quota",
	]:
		assert_has_method_named(game_save, method_name)

	var pre_init_quota = game_save.get_remaining_quota(null)
	assert_result_error(pre_init_quota, "not_initialized", "get_remaining_quota() rejects calls before GDK.initialize()")

	var pre_init_folder = game_save.get_folder_async(null)
	await assert_signal_result_error(pre_init_folder, "not_initialized", "get_folder_async() rejects calls before GDK.initialize()")

	var init_result = initialize_runtime()
	assert_not_null(init_result, "GDK.initialize() for game save behavior returns GDKResult")
	if init_result == null:
		return
	if not init_result.ok:
		pending("Game save runtime behavior: %s" % init_result.message)
		return

	var invalid_quota = game_save.get_remaining_quota(null)
	assert_result_error(invalid_quota, "invalid_user", "get_remaining_quota() rejects null users after initialize")

	var invalid_folder = game_save.get_folder_async(null)
	await assert_signal_result_error(invalid_folder, "invalid_user", "get_folder_async() rejects null users after initialize")


func test_game_save_folder_and_quota_when_user_available() -> void:
	if pending_unless_runtime_available():
		return

	var init_result = initialize_runtime()
	if init_result == null or not init_result.ok:
		pending("Game save runtime unavailable")
		return

	var gdk = get_gdk()
	var game_save = gdk.get_game_save()
	if game_save == null:
		return

	var sign_in = await ensure_primary_user()
	var user = sign_in["user"]
	if user == null:
		pending("Game save folder/quota coverage: no signed-in Xbox user is available on this machine.")
		return

	var quota_result = game_save.get_remaining_quota(user)
	assert_not_null(quota_result, "get_remaining_quota() returns GDKResult for a signed-in user")
	if quota_result == null:
		return
	if quota_result.ok:
		assert_dict_has_key(quota_result.data, "bytes", "get_remaining_quota() success data exposes bytes")
	else:
		assert_true(quota_result.code.length() > 0, "get_remaining_quota() failure exposes an error code")
		assert_true(quota_result.message.length() > 0, "get_remaining_quota() failure exposes an error message")

	var folder_result = await await_completion(game_save.get_folder_async(user))
	assert_not_null(folder_result, "get_folder_async() completes with a GDKResult for a signed-in user")
	if folder_result == null:
		return
	if folder_result.ok:
		assert_dict_has_key(folder_result.data, "path", "get_folder_async() success data exposes path")
	else:
		assert_true(folder_result.code.length() > 0, "get_folder_async() failure exposes an error code")
		assert_true(folder_result.message.length() > 0, "get_folder_async() failure exposes an error message")
