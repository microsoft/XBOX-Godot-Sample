extends RefCounted

func run(context) -> void:
	context.log_section("GDK Achievements API")

	var gdk = context.get_gdk()
	if gdk == null:
		context.log_fail("GDK root singleton missing, skipping achievements API group")
		return

	context.reset_runtime()

	var achievements = gdk.get_achievements()
	context.assert_not_null(achievements, "GDK.achievements returns service object")
	if achievements == null:
		return

	for method_name in ["query_player_achievements_async", "update_achievement_async", "get_cached_achievements"]:
		context.assert_has_method(achievements, method_name)

	for signal_name in ["achievement_unlocked", "achievements_updated"]:
		context.assert_has_signal(achievements, signal_name)

	context.assert_true(achievements.get_cached_achievements(null) is Array, "get_cached_achievements() returns Array")

	var init_result = context.initialize_runtime()
	context.assert_not_null(init_result, "GDK.initialize() for achievements behavior returns GDKResult")
	if init_result == null:
		return
	if not init_result.ok:
		context.log_skip("Achievements runtime behavior", init_result.message)
		return

	var sign_in = await context.ensure_primary_user()
	var sign_in_signal = sign_in["signal"]
	var sign_in_result = sign_in["result"]
	var user = sign_in["user"]
	if typeof(sign_in_signal) == TYPE_SIGNAL and sign_in_result == null:
		context.log_fail("Default-user flow for achievements completes", "timed out waiting for a signed-in user")
		context.reset_runtime()
		return
	if user == null:
		if sign_in_result != null and not sign_in_result.ok:
			context.assert_true(sign_in_result.code.length() > 0, "failed achievements sign-in exposes an error code")
			context.assert_true(sign_in_result.message.length() > 0, "failed achievements sign-in exposes an error message")
			context.log_skip("Achievements runtime behavior", sign_in_result.message)
		else:
			context.log_skip("Achievements runtime behavior", "No signed-in user is available on this machine.")
		context.reset_runtime()
		return

	context.assert_true(achievements.get_cached_achievements(user) is Array, "get_cached_achievements(user) returns Array before the first query")

	var query_signal = achievements.query_player_achievements_async(user)
	context.assert_true(typeof(query_signal) == TYPE_SIGNAL, "query_player_achievements_async() returns completion Signal")
	if typeof(query_signal) == TYPE_SIGNAL:
		var query_result = await context.wait_for_signal(query_signal, 8000)
		if query_result == null:
			context.log_skip("query_player_achievements_async()", "Timed out waiting for Achievements Manager to finish the query.")
			context.reset_runtime()
			return

		if query_result.ok:
			context.assert_true(query_result.data is Array, "achievement query returns Array data on success")
			if query_result.data is Array:
				var queried_achievements: Array = query_result.data
				var cached_achievements = achievements.get_cached_achievements(user)
				context.assert_true(cached_achievements is Array, "get_cached_achievements(user) returns Array after a query")
				context.assert_eq(cached_achievements.size(), queried_achievements.size(), "achievement cache size matches the query result size")
				if queried_achievements.size() > 0:
					context.assert_object_is(queried_achievements[0], "GDKAchievement", "achievement query returns GDKAchievement wrappers")

			var invalid_id_signal = achievements.update_achievement_async(user, "", 25)
			await context.assert_signal_result_error(invalid_id_signal, "invalid_achievement_id", "update_achievement_async() rejects blank achievement ids")

			var invalid_progress_signal = achievements.update_achievement_async(user, "test-achievement", 0)
			await context.assert_signal_result_error(invalid_progress_signal, "invalid_achievement_progress", "update_achievement_async() rejects progress outside 1-100")
		else:
			context.assert_true(query_result.code.length() > 0, "achievement query failure exposes an error code")
			context.assert_true(query_result.message.length() > 0, "achievement query failure exposes an error message")
			context.log_skip("Achievement cache assertions", query_result.message)

	context.reset_runtime()
