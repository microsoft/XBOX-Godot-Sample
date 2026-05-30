extends "res://addons/godot_gdk_tests/gdk_test_base.gd"


func before_each() -> void:
	reset_runtime()


func after_each() -> void:
	reset_runtime()


func test_stats_surface_and_validation() -> void:
	if pending_unless_runtime_available():
		return

	var gdk = get_gdk()
	var stats = gdk.get_stats()
	assert_not_null(stats, "GDK.stats returns service object")
	if stats == null:
		return

	for method_name in [
		"query_user_stats_async",
		"query_users_stats_async",
		"set_stat_integer",
		"set_stat_number",
		"flush_stats_async",
		"track_stats",
		"stop_tracking_stats",
		"get_cached_stats",
	]:
		assert_has_method_named(stats, method_name)

	for signal_name in ["stats_updated", "stat_changed", "stats_flushed"]:
		assert_has_signal_named(stats, signal_name)

	assert_true(stats.get_cached_stats(null) is Dictionary, "get_cached_stats(null) returns an empty Dictionary")

	var pre_init_set_result = stats.set_stat_number(null, "score", 1.0)
	assert_result_error(pre_init_set_result, "runtime_unavailable", "set_stat_number() reports unavailable runtime before initialize")

	var pre_init_query_signal = stats.query_user_stats_async(null, PackedStringArray(["score"]))
	await assert_signal_result_error(pre_init_query_signal, "runtime_unavailable", "query_user_stats_async() reports unavailable runtime before initialize")

	var init_result = initialize_runtime()
	assert_not_null(init_result, "GDK.initialize() for stats validation returns GDKResult")
	if init_result == null:
		return
	if not init_result.ok:
		pending("Stats runtime validation: %s" % init_result.message)
		return

	var invalid_set_result = stats.set_stat_integer(null, "score", 1)
	assert_result_error(invalid_set_result, "invalid_user", "set_stat_integer() rejects null users after initialize")

	var blank_stat_result = stats.set_stat_number(null, "   ", 1.0)
	assert_result_error(blank_stat_result, "invalid_user", "set_stat_number() validates the user before stat names")

	var invalid_query_signal = stats.query_users_stats_async(null, PackedStringArray(), PackedStringArray(["score"]))
	await assert_signal_result_error(invalid_query_signal, "invalid_user", "query_users_stats_async() rejects null users after initialize")

	var sign_in = await ensure_primary_user()
	var sign_in_signal = sign_in["signal"]
	var sign_in_result = sign_in["result"]
	var user = sign_in["user"]
	if typeof(sign_in_signal) == TYPE_SIGNAL and sign_in_result == null:
		assert_true(false, "Default-user flow for stats completes - timed out waiting for a signed-in user")
		return
	if user == null:
		if sign_in_result != null and not sign_in_result.ok:
			assert_true(sign_in_result.code.length() > 0, "failed stats sign-in exposes an error code")
			assert_true(sign_in_result.message.length() > 0, "failed stats sign-in exposes an error message")
			pending("Stats signed-in validation: %s" % sign_in_result.message)
		else:
			pending("Stats signed-in validation: No signed-in user is available on this machine.")
		return

	var blank_name_result = stats.set_stat_number(user, "   ", 1.0)
	assert_result_error(blank_name_result, "invalid_stat_name", "set_stat_number() rejects blank statistic names")

	var empty_query_signal = stats.query_user_stats_async(user, PackedStringArray())
	await assert_signal_result_error(empty_query_signal, "invalid_stat_names", "query_user_stats_async() rejects empty statistic name lists")

	var invalid_xuid_signal = stats.query_users_stats_async(user, PackedStringArray(["not-a-number"]), PackedStringArray(["score"]))
	await assert_signal_result_error(invalid_xuid_signal, "invalid_xuid", "query_users_stats_async() rejects non-numeric XUID strings")

	var empty_track_result = stats.track_stats(user, PackedStringArray())
	assert_result_error(empty_track_result, "invalid_stat_names", "track_stats() rejects empty statistic name lists")

	var no_staged_signal = stats.flush_stats_async(user)
	await assert_signal_result_error(no_staged_signal, "no_staged_stats", "flush_stats_async() rejects empty staged-stat batches")


func test_set_stat_integer_live_stages_value() -> void:
	if pending_unless_runtime_available():
		return
	if not requires_live():
		return

	var init_result = initialize_runtime()
	assert_not_null(init_result, "GDK.initialize() for set_stat_integer live coverage returns GDKResult")
	if init_result == null:
		return
	if not init_result.ok:
		pending("set_stat_integer live coverage: %s" % init_result.message)
		return

	var gdk = get_gdk()
	var stats = gdk.get_stats()
	assert_not_null(stats, "GDK.stats is available for set_stat_integer live coverage")
	if stats == null:
		return

	var sign_in = await ensure_primary_user()
	var sign_in_signal = sign_in["signal"]
	var sign_in_result = sign_in["result"]
	var user = sign_in["user"]
	if typeof(sign_in_signal) == TYPE_SIGNAL and sign_in_result == null:
		assert_true(false, "Default-user flow for set_stat_integer live coverage completes")
		return
	if user == null:
		if sign_in_result != null and not sign_in_result.ok:
			assert_true(sign_in_result.code.length() > 0, "failed stats live sign-in exposes an error code")
			assert_true(sign_in_result.message.length() > 0, "failed stats live sign-in exposes an error message")
			pending("set_stat_integer live coverage: %s" % sign_in_result.message)
		else:
			pending("set_stat_integer live coverage: No signed-in user is available on this machine.")
		return

	var stat_name = with_unique_id("gdk_b3_integer_stat")
	var set_result = stats.set_stat_integer(user, stat_name, 42)
	assert_result_ok(set_result, "set_stat_integer() stages an integer value for a signed-in user")
	if set_result == null or not set_result.ok:
		return
	assert_true(set_result.data is Dictionary, "set_stat_integer() returns staged stat metadata")
	if set_result.data is Dictionary:
		assert_eq(set_result.data["name"], stat_name, "set_stat_integer() result echoes stat name")
		assert_eq(int(set_result.data["value"]), 42, "set_stat_integer() result echoes integer value")

func test_stats_live_track_stop_shutdown_callback_context() -> void:
	if pending_unless_runtime_available():
		return
	if pending_unless_live():
		return

	var init_result = initialize_runtime()
	assert_not_null(init_result, "GDK.initialize() returns GDKResult for live stats callback-lifetime test")
	if init_result == null:
		return
	if not init_result.ok:
		pending("Live stats callback-lifetime test: %s" % init_result.message)
		return

	var gdk = get_gdk()
	var stats = gdk.get_stats()
	var sign_in = await ensure_primary_user(10000)
	var sign_in_result = sign_in["result"]
	var user = sign_in["user"]
	if user == null:
		if sign_in_result != null and not sign_in_result.ok:
			pending("Live stats callback-lifetime sign-in: %s" % sign_in_result.message)
		else:
			pending("Live stats callback-lifetime test: no signed-in Xbox user is available.")
		return

	var stat_names := PackedStringArray(["high_score"])
	var registered_once := false
	for iteration in 4:
		var track_result = stats.track_stats(user, stat_names)
		if track_result == null or not track_result.ok:
			pending("Live stats track_stats() could not subscribe: %s" % (track_result.message if track_result != null else "missing result"))
			return
		registered_once = true
		for _frame_index in 3:
			gdk.dispatch()
			await get_tree().process_frame
		var stop_result = stats.stop_tracking_stats(user, PackedStringArray())
		assert_result_ok(stop_result, "stop_tracking_stats() unregisters live callback context on iteration %d" % iteration)

	var final_track_result = stats.track_stats(user, stat_names)
	if final_track_result != null and final_track_result.ok:
		gdk.shutdown()
		assert_eq(gdk.is_initialized(), false, "GDK.shutdown() unregisters live stats callback context")
	else:
		pending("Live stats final track_stats() could not subscribe before shutdown: %s" % (final_track_result.message if final_track_result != null else "missing result"))
		return

	assert_true(registered_once, "live stats callback context registered and unregistered without crashing")


func test_stats_live_write_tracked_callback_context() -> void:
	if pending_unless_runtime_available():
		return
	if pending_unless_live_write():
		return

	var init_result = initialize_runtime()
	if init_result == null or not init_result.ok:
		pending("Live-write stats callback-lifetime test: %s" % (init_result.message if init_result != null else "initialize returned null"))
		return

	var gdk = get_gdk()
	var stats = gdk.get_stats()
	var sign_in = await ensure_primary_user(10000)
	var user = sign_in["user"]
	if user == null:
		pending("Live-write stats callback-lifetime test: no signed-in Xbox user is available.")
		return

	var stat_names := PackedStringArray(["high_score"])
	var track_result = stats.track_stats(user, stat_names)
	if track_result == null or not track_result.ok:
		pending("Live-write stats track_stats() could not subscribe: %s" % (track_result.message if track_result != null else "missing result"))
		return

	var staged_result = stats.set_stat_integer(user, "high_score", int(Time.get_unix_time_from_system()) % 100000)
	if staged_result == null or not staged_result.ok:
		stats.stop_tracking_stats(user, PackedStringArray())
		pending("Live-write stats set_stat_integer() could not stage a write: %s" % (staged_result.message if staged_result != null else "missing result"))
		return

	var flush_signal = stats.flush_stats_async(user)
	var flush_result = await await_completion(flush_signal, 15000)
	if flush_result == null or not flush_result.ok:
		stats.stop_tracking_stats(user, PackedStringArray())
		pending("Live-write stats flush could not complete: %s" % (flush_result.message if flush_result != null else "timed out"))
		return

	for _frame_index in 5:
		gdk.dispatch()
		await get_tree().process_frame

	var stop_result = stats.stop_tracking_stats(user, PackedStringArray())
	assert_result_ok(stop_result, "stop_tracking_stats() unregisters after live write callback exercise")
	gdk.shutdown()
	assert_eq(gdk.is_initialized(), false, "GDK.shutdown() after live write callback exercise does not crash")
