extends "res://addons/godot_gdk_tests/gdk_test_base.gd"


func before_each() -> void:
	reset_runtime()


func after_each() -> void:
	reset_runtime()


func test_leaderboards_surface_and_validation() -> void:
	if pending_unless_runtime_available():
		return

	var gdk = get_gdk()
	var leaderboards = gdk.get_leaderboards()
	assert_not_null(leaderboards, "GDK.leaderboards returns service object")
	if leaderboards == null:
		return

	for method_name in [
		"get_leaderboard_async",
		"get_leaderboard_around_user_async",
		"get_social_leaderboard_async",
		"get_next_page_async",
		"get_cached_leaderboard",
	]:
		assert_has_method_named(leaderboards, method_name)

	assert_has_signal_named(leaderboards, "leaderboard_updated")
	assert_eq(leaderboards.get_cached_leaderboard("missing"), null, "get_cached_leaderboard() returns null before any query")

	var blank_leaderboard = instantiate_class("GDKLeaderboard")
	assert_not_null(blank_leaderboard, "GDKLeaderboard.new() returns wrapper")
	if blank_leaderboard != null:
		for method_name in ["get_stat_name", "get_query_type", "get_total_row_count", "has_next", "get_columns", "get_rows"]:
			assert_has_method_named(blank_leaderboard, method_name)
		assert_eq(blank_leaderboard.get_stat_name(), "", "blank GDKLeaderboard stat name defaults empty")
		assert_eq(blank_leaderboard.get_query_type(), "", "blank GDKLeaderboard query type defaults empty")
		assert_eq(blank_leaderboard.get_total_row_count(), 0, "blank GDKLeaderboard total row count defaults zero")
		assert_eq(blank_leaderboard.has_next(), false, "blank GDKLeaderboard has_next defaults false")
		assert_true(blank_leaderboard.get_columns() is Array, "blank GDKLeaderboard columns returns Array")
		assert_true(blank_leaderboard.get_rows() is Array, "blank GDKLeaderboard rows returns Array")

	var blank_column = instantiate_class("GDKLeaderboardColumn")
	assert_not_null(blank_column, "GDKLeaderboardColumn.new() returns wrapper")
	if blank_column != null:
		assert_has_method_named(blank_column, "get_stat_name")
		assert_has_method_named(blank_column, "get_stat_type")
		assert_eq(blank_column.get_stat_name(), "", "blank GDKLeaderboardColumn stat name defaults empty")
		assert_eq(blank_column.get_stat_type(), "", "blank GDKLeaderboardColumn stat type defaults empty")

	var blank_row = instantiate_class("GDKLeaderboardRow")
	assert_not_null(blank_row, "GDKLeaderboardRow.new() returns wrapper")
	if blank_row != null:
		for method_name in [
			"get_gamertag",
			"get_modern_gamertag",
			"get_modern_gamertag_suffix",
			"get_unique_modern_gamertag",
			"get_xuid",
			"get_percentile",
			"get_rank",
			"get_global_rank",
			"get_column_values",
		]:
			assert_has_method_named(blank_row, method_name)
		assert_eq(blank_row.get_xuid(), "", "blank GDKLeaderboardRow xuid defaults empty")
		assert_eq(blank_row.get_rank(), 0, "blank GDKLeaderboardRow rank defaults zero")
		assert_true(blank_row.get_column_values() is PackedStringArray, "blank GDKLeaderboardRow column values returns PackedStringArray")

	var pre_init_signal = leaderboards.get_leaderboard_async(null, "score")
	await assert_signal_result_error(pre_init_signal, "runtime_unavailable", "get_leaderboard_async() reports unavailable runtime before initialize")

	var pre_init_around_user_signal = leaderboards.get_leaderboard_around_user_async(null, "score")
	await assert_signal_result_error(pre_init_around_user_signal, "runtime_unavailable", "get_leaderboard_around_user_async() reports unavailable runtime before initialize")

	var init_result = initialize_runtime()
	assert_not_null(init_result, "GDK.initialize() for leaderboard validation returns GDKResult")
	if init_result == null:
		return
	if not init_result.ok:
		pending("Leaderboards runtime validation: %s" % init_result.message)
		return

	var invalid_user_signal = leaderboards.get_social_leaderboard_async(null, "score")
	await assert_signal_result_error(invalid_user_signal, "invalid_user", "get_social_leaderboard_async() rejects null users after initialize")

	var around_invalid_user_signal = leaderboards.get_leaderboard_around_user_async(null, "score")
	await assert_signal_result_error(around_invalid_user_signal, "invalid_user", "get_leaderboard_around_user_async() rejects null users after initialize")

	var invalid_next_page_signal = leaderboards.get_next_page_async(null)
	await assert_signal_result_error(invalid_next_page_signal, "invalid_leaderboard", "get_next_page_async() rejects null leaderboards")

	var sign_in = await ensure_primary_user()
	var sign_in_signal = sign_in["signal"]
	var sign_in_result = sign_in["result"]
	var user = sign_in["user"]
	if typeof(sign_in_signal) == TYPE_SIGNAL and sign_in_result == null:
		assert_true(false, "Default-user flow for leaderboards completes - timed out waiting for a signed-in user")
		return
	if user == null:
		if sign_in_result != null and not sign_in_result.ok:
			assert_true(sign_in_result.code.length() > 0, "failed leaderboards sign-in exposes an error code")
			assert_true(sign_in_result.message.length() > 0, "failed leaderboards sign-in exposes an error message")
			pending("Leaderboards signed-in validation: %s" % sign_in_result.message)
		else:
			pending("Leaderboards signed-in validation: No signed-in user is available on this machine.")
		return

	var blank_stat_signal = leaderboards.get_leaderboard_async(user, "   ")
	await assert_signal_result_error(blank_stat_signal, "invalid_stat_name", "get_leaderboard_async() rejects blank stat names")

	var invalid_max_signal = leaderboards.get_leaderboard_async(user, "score", -1)
	await assert_signal_result_error(invalid_max_signal, "invalid_max_items", "get_leaderboard_async() rejects negative max_items")

	var blank_next_page_signal = leaderboards.get_next_page_async(blank_leaderboard)
	await assert_signal_result_error(blank_next_page_signal, "invalid_leaderboard", "get_next_page_async() rejects manually-created leaderboards")


func test_leaderboard_around_user_live_read() -> void:
	if pending_unless_runtime_available():
		return
	if not requires_live():
		return

	var init_result = initialize_runtime()
	assert_not_null(init_result, "GDK.initialize() for around-user leaderboard live read returns GDKResult")
	if init_result == null:
		return
	if not init_result.ok:
		pending("Around-user leaderboard live read: %s" % init_result.message)
		return

	var gdk = get_gdk()
	var leaderboards = gdk.get_leaderboards()
	assert_not_null(leaderboards, "GDK.leaderboards is available for live around-user query")
	if leaderboards == null:
		return

	var sign_in = await ensure_primary_user()
	var sign_in_signal = sign_in["signal"]
	var sign_in_result = sign_in["result"]
	var user = sign_in["user"]
	if typeof(sign_in_signal) == TYPE_SIGNAL and sign_in_result == null:
		assert_true(false, "Default-user flow for around-user leaderboard live read completes")
		return
	if user == null:
		if sign_in_result != null and not sign_in_result.ok:
			assert_true(sign_in_result.code.length() > 0, "failed around-user leaderboard sign-in exposes an error code")
			assert_true(sign_in_result.message.length() > 0, "failed around-user leaderboard sign-in exposes an error message")
			pending("Around-user leaderboard live read: %s" % sign_in_result.message)
		else:
			pending("Around-user leaderboard live read: No signed-in user is available on this machine.")
		return

	var around_signal = leaderboards.get_leaderboard_around_user_async(user, "score", 25)
	assert_eq(typeof(around_signal), TYPE_SIGNAL, "get_leaderboard_around_user_async() returns completion Signal")
	if typeof(around_signal) != TYPE_SIGNAL:
		return

	var around_result = await await_completion(around_signal, 15000)
	assert_not_null(around_result, "get_leaderboard_around_user_async() yields a result")
	if around_result == null:
		return
	if not around_result.ok:
		assert_true(around_result.code.length() > 0, "failed around-user leaderboard query exposes an error code")
		assert_true(around_result.message.length() > 0, "failed around-user leaderboard query exposes an error message")
		pending("get_leaderboard_around_user_async(): %s" % around_result.message)
		return

	assert_object_is(around_result.data, "GDKLeaderboard", "around-user leaderboard query returns a GDKLeaderboard")
	if is_class_instance(around_result.data, "GDKLeaderboard"):
		assert_eq(around_result.data.get_stat_name(), "score", "around-user leaderboard keeps the requested stat name")
		assert_eq(around_result.data.get_query_type(), "around_user", "around-user leaderboard records the around_user query type")
