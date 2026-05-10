extends "res://addons/godot_gdk_tests/gdk_test_base.gd"
## Wave 3 GUT migration of `suites/presence_suite.gd`. Behavior parity:
## same per-call assertion count as the pre-GUT harness; `log_skip` mapped
## to `pending(...)`; one-off `log_fail` early-returns preserved as
## `assert_true(false, ...)` so failures still fail the suite.

func before_each() -> void:
	reset_runtime()


func after_each() -> void:
	reset_runtime()


func test_presence_full_flow() -> void:
	if pending_unless_runtime_available():
		return

	var gdk = get_gdk()
	var presence = gdk.get_presence()
	assert_not_null(presence, "GDK.presence returns service object")
	if presence == null:
		return

	for method_name in ["set_presence_async", "clear_presence_async", "get_presence_async", "get_presence_for_social_group_async", "track_presence", "stop_tracking_presence", "get_cached_presence"]:
		assert_has_method_named(presence, method_name)

	for signal_name in ["presence_changed", "local_presence_set", "device_presence_changed", "title_presence_changed"]:
		assert_has_signal_named(presence, signal_name)

	var blank_record = instantiate_class("GDKPresenceRecord")
	assert_not_null(blank_record, "GDKPresenceRecord.new() returns wrapper")
	if blank_record != null:
		for method_name in ["get_xuid", "get_user_state", "get_user_state_name", "is_online", "get_title_records"]:
			assert_has_method_named(blank_record, method_name)

		assert_eq(blank_record.get_xuid(), "", "blank GDKPresenceRecord xuid defaults empty")
		assert_eq(blank_record.get_user_state(), get_class_constant("GDKPresenceRecord", "USER_STATE_UNKNOWN"), "blank GDKPresenceRecord user_state defaults to USER_STATE_UNKNOWN")
		assert_eq(blank_record.get_user_state_name(), "unknown", "blank GDKPresenceRecord user_state_name defaults to unknown")
		assert_eq(blank_record.is_online(), false, "blank GDKPresenceRecord is_online defaults false")
		assert_true(blank_record.get_title_records() is Array, "blank GDKPresenceRecord title_records returns Array")

	assert_true(presence.get_cached_presence("") == null, "get_cached_presence() returns null for missing xuid")

	var init_result = initialize_runtime()
	assert_not_null(init_result, "GDK.initialize() for presence behavior returns GDKResult")
	if init_result == null:
		return
	if not init_result.ok:
		pending("Presence runtime behavior: %s" % init_result.message)
		return

	var missing_xuids_signal = presence.get_presence_async(PackedStringArray())
	await assert_signal_result_error(missing_xuids_signal, "missing_presence_xuids", "get_presence_async() rejects empty XUID lists")

	var invalid_xuid_signal = presence.get_presence_async(PackedStringArray(["not-a-number"]))
	await assert_signal_result_error(invalid_xuid_signal, "invalid_presence_xuid", "get_presence_async() rejects non-numeric XUID strings")

	var sign_in = await ensure_primary_user()
	var sign_in_signal = sign_in["signal"]
	var sign_in_result = sign_in["result"]
	var user = sign_in["user"]
	if typeof(sign_in_signal) == TYPE_SIGNAL and sign_in_result == null:
		assert_true(false, "Default-user flow for presence completes — timed out waiting for a signed-in user")
		return
	if user == null:
		if sign_in_result != null and not sign_in_result.ok:
			assert_true(sign_in_result.code.length() > 0, "failed presence sign-in exposes an error code")
			assert_true(sign_in_result.message.length() > 0, "failed presence sign-in exposes an error message")
			pending("Primary-user presence behavior: %s" % sign_in_result.message)
		else:
			pending("Primary-user presence behavior: No signed-in user is available on this machine.")
		return

	var invalid_state_signal = presence.set_presence_async(user, "   ")
	await assert_signal_result_error(invalid_state_signal, "invalid_presence_state", "set_presence_async() rejects blank presence states")

	var blank_group_signal = presence.get_presence_for_social_group_async(user, "   ")
	await assert_signal_result_error(blank_group_signal, "invalid_social_group", "get_presence_for_social_group_async() rejects blank social group names")

	var empty_track_result = presence.track_presence(user, PackedStringArray())
	assert_result_error(empty_track_result, "missing_presence_xuids", "track_presence() rejects empty XUID lists")

	var invalid_track_xuid_result = presence.track_presence(user, PackedStringArray(["not-a-number"]))
	assert_result_error(invalid_track_xuid_result, "invalid_presence_xuid", "track_presence() rejects non-numeric XUID strings")

	var invalid_track_title_result = presence.track_presence(user, PackedStringArray([user.get_xuid()]), PackedInt64Array([-1]))
	assert_result_error(invalid_track_title_result, "invalid_title_id", "track_presence() rejects invalid title IDs")

	var invalid_stop_xuid_result = presence.stop_tracking_presence(user, PackedStringArray(["not-a-number"]))
	assert_result_error(invalid_stop_xuid_result, "invalid_presence_xuid", "stop_tracking_presence() rejects non-numeric XUID strings")

	var query_signal = presence.get_presence_async(PackedStringArray([user.get_xuid()]))
	assert_true(typeof(query_signal) == TYPE_SIGNAL, "get_presence_async() returns completion Signal for the signed-in user's XUID")
	if typeof(query_signal) == TYPE_SIGNAL:
		var query_result = await await_completion(query_signal, 8000)
		if query_result == null:
			pending("get_presence_async(): Timed out waiting for the presence query to finish.")
			return

		if query_result.ok:
			assert_true(query_result.data is Array, "presence query returns Array data on success")
			if query_result.data is Array:
				var records: Array = query_result.data
				assert_eq(records.size(), 1, "presence query returns one record for one XUID")
				if records.size() == 1:
					assert_object_is(records[0], "GDKPresenceRecord", "presence query returns GDKPresenceRecord wrappers")
					if is_class_instance(records[0], "GDKPresenceRecord"):
						assert_eq(records[0].get_xuid(), user.get_xuid(), "presence query record matches the requested XUID")

			var cached_record = presence.get_cached_presence(user.get_xuid())
			assert_not_null(cached_record, "successful presence queries populate the cache")
			if cached_record != null:
				assert_eq(cached_record.get_xuid(), user.get_xuid(), "cached presence record matches the queried XUID")
		else:
			assert_true(query_result.code.length() > 0, "presence query failure exposes an error code")
			assert_true(query_result.message.length() > 0, "presence query failure exposes an error message")
			pending("Presence cache assertions: %s" % query_result.message)
