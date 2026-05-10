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
