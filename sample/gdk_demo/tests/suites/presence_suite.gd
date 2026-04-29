extends RefCounted

func run(context) -> void:
	context.log_section("GDK Presence API")

	var gdk = context.get_gdk()
	if gdk == null:
		context.log_fail("GDK root singleton missing, skipping presence API group")
		return

	context.reset_runtime()

	var presence = gdk.get_presence()
	context.assert_not_null(presence, "GDK.presence returns service object")
	if presence == null:
		return

	for method_name in ["set_presence_async", "clear_presence_async", "get_presence_async", "get_cached_presence"]:
		context.assert_has_method(presence, method_name)

	for signal_name in ["presence_changed", "local_presence_set"]:
		context.assert_has_signal(presence, signal_name)

	var blank_record = context.instantiate_class("GDKPresenceRecord")
	context.assert_not_null(blank_record, "GDKPresenceRecord.new() returns wrapper")
	if blank_record != null:
		for method_name in ["get_xuid", "get_user_state", "get_user_state_name", "is_online", "get_title_records"]:
			context.assert_has_method(blank_record, method_name)

		context.assert_eq(blank_record.get_xuid(), "", "blank GDKPresenceRecord xuid defaults empty")
		context.assert_eq(blank_record.get_user_state(), context.get_class_constant("GDKPresenceRecord", "USER_STATE_UNKNOWN"), "blank GDKPresenceRecord user_state defaults to USER_STATE_UNKNOWN")
		context.assert_eq(blank_record.get_user_state_name(), "unknown", "blank GDKPresenceRecord user_state_name defaults to unknown")
		context.assert_eq(blank_record.is_online(), false, "blank GDKPresenceRecord is_online defaults false")
		context.assert_true(blank_record.get_title_records() is Array, "blank GDKPresenceRecord title_records returns Array")

	context.assert_true(presence.get_cached_presence("") == null, "get_cached_presence() returns null for missing xuid")

	var init_result = context.initialize_runtime()
	context.assert_not_null(init_result, "GDK.initialize() for presence behavior returns GDKResult")
	if init_result == null:
		return
	if not init_result.ok:
		context.log_skip("Presence runtime behavior", init_result.message)
		return

	var missing_xuids_op = presence.get_presence_async(PackedStringArray())
	context.assert_not_null(missing_xuids_op, "get_presence_async() returns GDKAsyncOp for empty XUID lists")
	if missing_xuids_op != null:
		context.assert_result_error(missing_xuids_op.get_result(), "missing_presence_xuids", "get_presence_async() rejects empty XUID lists")

	var invalid_xuid_op = presence.get_presence_async(PackedStringArray(["not-a-number"]))
	context.assert_not_null(invalid_xuid_op, "get_presence_async() returns GDKAsyncOp for invalid XUIDs")
	if invalid_xuid_op != null:
		context.assert_result_error(invalid_xuid_op.get_result(), "invalid_presence_xuid", "get_presence_async() rejects non-numeric XUID strings")

	var sign_in = context.ensure_primary_user()
	var sign_in_op = sign_in["op"]
	var sign_in_result = sign_in["result"]
	var user = sign_in["user"]
	if sign_in_op != null and sign_in_result == null:
		context.log_fail("Default-user flow for presence completes", "timed out waiting for a signed-in user")
		context.reset_runtime()
		return
	if user == null:
		if sign_in_result != null and not sign_in_result.ok:
			context.assert_true(sign_in_result.code.length() > 0, "failed presence sign-in exposes an error code")
			context.assert_true(sign_in_result.message.length() > 0, "failed presence sign-in exposes an error message")
			context.log_skip("Primary-user presence behavior", sign_in_result.message)
		else:
			context.log_skip("Primary-user presence behavior", "No signed-in user is available on this machine.")
		context.reset_runtime()
		return

	var invalid_state_op = presence.set_presence_async(user, "   ")
	context.assert_not_null(invalid_state_op, "set_presence_async() returns GDKAsyncOp for a signed-in user")
	if invalid_state_op != null:
		context.assert_result_error(invalid_state_op.get_result(), "invalid_presence_state", "set_presence_async() rejects blank presence states")

	var query_op = presence.get_presence_async(PackedStringArray([user.get_xuid()]))
	context.assert_not_null(query_op, "get_presence_async() returns GDKAsyncOp for the signed-in user's XUID")
	if query_op != null:
		var query_result = context.wait_for_op(query_op, 8000)
		if query_result == null:
			query_op.cancel()
			context.log_skip("get_presence_async()", "Timed out waiting for the presence query to finish.")
			context.reset_runtime()
			return

		if query_result.ok:
			context.assert_true(query_result.data is Array, "presence query returns Array data on success")
			if query_result.data is Array:
				var records: Array = query_result.data
				context.assert_eq(records.size(), 1, "presence query returns one record for one XUID")
				if records.size() == 1:
					context.assert_object_is(records[0], "GDKPresenceRecord", "presence query returns GDKPresenceRecord wrappers")
					if context.is_class_instance(records[0], "GDKPresenceRecord"):
						context.assert_eq(records[0].get_xuid(), user.get_xuid(), "presence query record matches the requested XUID")

			var cached_record = presence.get_cached_presence(user.get_xuid())
			context.assert_not_null(cached_record, "successful presence queries populate the cache")
			if cached_record != null:
				context.assert_eq(cached_record.get_xuid(), user.get_xuid(), "cached presence record matches the queried XUID")
		else:
			context.assert_true(query_result.code.length() > 0, "presence query failure exposes an error code")
			context.assert_true(query_result.message.length() > 0, "presence query failure exposes an error message")
			context.log_skip("Presence cache assertions", query_result.message)

	context.reset_runtime()
