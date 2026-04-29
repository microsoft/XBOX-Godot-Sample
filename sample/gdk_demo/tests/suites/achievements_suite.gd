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

	var sign_in = context.ensure_primary_user()
	var sign_in_op = sign_in["op"]
	var sign_in_result = sign_in["result"]
	var user = sign_in["user"]
	if sign_in_op != null and sign_in_result == null:
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

	var query_op = achievements.query_player_achievements_async(user)
	context.assert_not_null(query_op, "query_player_achievements_async() returns GDKDispatchOp")
	if query_op != null:
		context.assert_true(query_op is GDKDispatchOp, "query_player_achievements_async() uses dispatch-backed op type")
		var query_result = context.wait_for_op(query_op, 8000)
		if query_result == null:
			query_op.cancel()
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
					context.assert_true(queried_achievements[0] is GDKAchievement, "achievement query returns GDKAchievement wrappers")

			var invalid_id_op = achievements.update_achievement_async(user, "", 25)
			context.assert_not_null(invalid_id_op, "update_achievement_async() returns GDKDispatchOp for a signed-in user")
			if invalid_id_op != null:
				context.assert_result_error(invalid_id_op.get_result(), "invalid_achievement_id", "update_achievement_async() rejects blank achievement ids")

			var invalid_progress_op = achievements.update_achievement_async(user, "test-achievement", 0)
			context.assert_not_null(invalid_progress_op, "update_achievement_async() validates achievement progress")
			if invalid_progress_op != null:
				context.assert_result_error(invalid_progress_op.get_result(), "invalid_achievement_progress", "update_achievement_async() rejects progress outside 1-100")
		else:
			context.assert_true(query_result.code.length() > 0, "achievement query failure exposes an error code")
			context.assert_true(query_result.message.length() > 0, "achievement query failure exposes an error message")
			context.log_skip("Achievement cache assertions", query_result.message)

	context.reset_runtime()
