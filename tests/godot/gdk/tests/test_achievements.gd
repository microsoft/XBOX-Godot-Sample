extends "res://addons/godot_gdk_tests/gdk_test_base.gd"
## Wave 3 GUT migration of `suites/achievements_suite.gd`. Behavior parity:
## same per-call assertion count as the pre-GUT harness; `log_skip` mapped to
## `pending(...)`; one-off `log_pass` direct calls preserved as
## `assert_true(true, ...)` so GUT's `Asserts:` count tracks the pre-GUT total.

func before_each() -> void:
	reset_runtime()


func after_each() -> void:
	reset_runtime()


func test_achievements_full_flow() -> void:
	if pending_unless_runtime_available():
		return

	var gdk = get_gdk()
	var achievements = gdk.get_achievements()
	assert_not_null(achievements, "GDK.achievements returns service object")
	if achievements == null:
		return

	for method_name in ["query_player_achievements_async", "update_achievement_async", "get_cached_achievements"]:
		assert_has_method_named(achievements, method_name)

	for signal_name in ["achievement_unlocked", "achievements_updated"]:
		assert_has_signal_named(achievements, signal_name)

	assert_true(achievements.get_cached_achievements(null) is Array, "get_cached_achievements() returns Array")

	var init_result = initialize_runtime()
	assert_not_null(init_result, "GDK.initialize() for achievements behavior returns GDKResult")
	if init_result == null:
		return
	if not init_result.ok:
		pending("Achievements runtime behavior: %s" % init_result.message)
		return

	var sign_in = await ensure_primary_user()
	var sign_in_signal = sign_in["signal"]
	var sign_in_result = sign_in["result"]
	var user = sign_in["user"]
	if typeof(sign_in_signal) == TYPE_SIGNAL and sign_in_result == null:
		assert_true(false, "Default-user flow for achievements completes — timed out waiting for a signed-in user")
		return
	if user == null:
		if sign_in_result != null and not sign_in_result.ok:
			assert_true(sign_in_result.code.length() > 0, "failed achievements sign-in exposes an error code")
			assert_true(sign_in_result.message.length() > 0, "failed achievements sign-in exposes an error message")
			pending("Achievements runtime behavior: %s" % sign_in_result.message)
		else:
			pending("Achievements runtime behavior: No signed-in user is available on this machine.")
		return

	assert_true(achievements.get_cached_achievements(user) is Array, "get_cached_achievements(user) returns Array before the first query")

	var query_signal = achievements.query_player_achievements_async(user)
	assert_true(typeof(query_signal) == TYPE_SIGNAL, "query_player_achievements_async() returns completion Signal")
	if typeof(query_signal) == TYPE_SIGNAL:
		var query_result = await await_completion(query_signal, 8000)
		if query_result == null:
			pending("query_player_achievements_async(): Timed out waiting for Achievements Manager to finish the query.")
			return

		if query_result.ok:
			assert_true(query_result.data is Array, "achievement query returns Array data on success")
			if query_result.data is Array:
				var queried_achievements: Array = query_result.data
				var cached_achievements = achievements.get_cached_achievements(user)
				assert_true(cached_achievements is Array, "get_cached_achievements(user) returns Array after a query")
				assert_eq(cached_achievements.size(), queried_achievements.size(), "achievement cache size matches the query result size")
				if queried_achievements.size() > 0:
					assert_object_is(queried_achievements[0], "GDKAchievement", "achievement query returns GDKAchievement wrappers")

			var invalid_id_signal = achievements.update_achievement_async(user, "", 25)
			await assert_signal_result_error(invalid_id_signal, "invalid_achievement_id", "update_achievement_async() rejects blank achievement ids")

			var invalid_progress_signal = achievements.update_achievement_async(user, "test-achievement", 0)
			await assert_signal_result_error(invalid_progress_signal, "invalid_achievement_progress", "update_achievement_async() rejects progress outside 1-100")
		else:
			assert_true(query_result.code.length() > 0, "achievement query failure exposes an error code")
			assert_true(query_result.message.length() > 0, "achievement query failure exposes an error message")
			pending("Achievement cache assertions: %s" % query_result.message)
