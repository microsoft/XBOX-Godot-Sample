extends RefCounted

func run(context) -> void:
	context.log_section("GDK Users API")

	var gdk = context.get_gdk()
	if gdk == null:
		context.log_fail("GDK root singleton missing, skipping users API group")
		return

	context.reset_runtime()

	var users = gdk.get_users()
	context.assert_not_null(users, "GDK.users returns service object")
	if users == null:
		return

	for method_name in [
		"add_default_user_async",
		"add_user_with_ui_async",
		"get_primary_user",
		"get_users",
		"check_privilege_async",
		"resolve_privilege_with_ui_async",
		"resolve_issue_with_ui_async",
		"get_gamer_picture_async",
		"get_token_and_signature_async"
	]:
		context.assert_has_method(users, method_name)

	for signal_name in ["user_added", "user_removed", "user_changed", "primary_user_changed"]:
		context.assert_has_signal(users, signal_name)

	context.assert_true(users.get_users() is Array, "get_users() returns Array")
	context.assert_true(users.get_primary_user() == null, "get_primary_user() starts null before init")

	var blank_user = GDKUser.new()
	context.assert_not_null(blank_user, "GDKUser.new() returns wrapper")
	if blank_user != null:
		for method_name in [
			"get_local_id",
			"get_xuid",
			"get_gamertag",
			"get_age_group",
			"get_age_group_name",
			"get_sign_in_state",
			"get_sign_in_state_name",
			"is_guest",
			"is_signed_in",
			"is_store_user"
		]:
			context.assert_has_method(blank_user, method_name)

		context.assert_eq(blank_user.get_local_id(), 0, "blank GDKUser local_id defaults to 0")
		context.assert_eq(blank_user.get_xuid(), "", "blank GDKUser xuid defaults empty")
		context.assert_eq(blank_user.get_gamertag(), "", "blank GDKUser gamertag defaults empty")
		context.assert_eq(blank_user.get_age_group(), GDKUser.AGE_GROUP_UNKNOWN, "blank GDKUser age_group defaults to AGE_GROUP_UNKNOWN")
		context.assert_eq(blank_user.get_age_group_name(), "unknown", "blank GDKUser age_group_name defaults to unknown")
		context.assert_eq(blank_user.get_sign_in_state(), GDKUser.SIGN_IN_STATE_SIGNED_OUT, "blank GDKUser sign_in_state defaults to SIGN_IN_STATE_SIGNED_OUT")
		context.assert_eq(blank_user.get_sign_in_state_name(), "signed_out", "blank GDKUser sign_in_state_name defaults to signed_out")
		context.assert_eq(blank_user.is_guest(), false, "blank GDKUser guest defaults false")
		context.assert_eq(blank_user.is_signed_in(), false, "blank GDKUser signed_in defaults false")
		context.assert_eq(blank_user.is_store_user(), false, "blank GDKUser store_user defaults false")

	var init_result = context.initialize_runtime()
	context.assert_not_null(init_result, "GDK.initialize() for users behavior returns GDKResult")
	if init_result == null:
		return
	if not init_result.ok:
		context.log_skip("Users runtime behavior", init_result.message)
		return

	context.assert_eq(users.get_users().size(), 0, "get_users() starts empty after init")
	context.assert_true(users.get_primary_user() == null, "get_primary_user() starts null after init")

	var user_added_events: Array = []
	var user_removed_events: Array = []
	var user_changed_events: Array = []
	var primary_user_changed_events: Array = []
	users.connect("user_added", func(user): user_added_events.append(user))
	users.connect("user_removed", func(local_id): user_removed_events.append(local_id))
	users.connect("user_changed", func(user): user_changed_events.append(user))
	users.connect("primary_user_changed", func(user): primary_user_changed_events.append(user))

	var sign_in = context.ensure_primary_user()
	var add_op = sign_in["op"]
	var add_result = sign_in["result"]
	var user = sign_in["user"]
	if add_op != null:
		context.assert_true(add_op is GDKAsyncOp, "add_default_user_async() uses XAsync-backed op type")
	if add_op != null and add_result == null:
		context.log_fail("add_default_user_async() completes", "timed out waiting for the default user flow")
		context.disconnect_signal_handlers(users, ["user_added", "user_removed", "user_changed", "primary_user_changed"])
		context.reset_runtime()
		return

	if add_result != null and not add_result.ok:
		context.assert_true(add_result.code.length() > 0, "failed default-user add exposes an error code")
		context.assert_true(add_result.message.length() > 0, "failed default-user add exposes an error message")
		context.assert_true(user == null, "failed default-user add leaves primary user unavailable")
		context.assert_eq(users.get_users().size(), 0, "failed default-user add keeps the user cache empty")

		var failed_add_last_error = gdk.get_last_error()
		context.assert_not_null(failed_add_last_error, "default-user add failure updates the root last error")
		if failed_add_last_error != null:
			context.assert_eq(failed_add_last_error.code, add_result.code, "root last error matches the failed default-user add")

		context.log_skip("Signed-in user behavior", add_result.message)
		context.disconnect_signal_handlers(users, ["user_added", "user_removed", "user_changed", "primary_user_changed"])
		context.reset_runtime()
		return

	if user == null:
		context.log_skip("Signed-in user behavior", "No default user is available on this machine.")
		context.disconnect_signal_handlers(users, ["user_added", "user_removed", "user_changed", "primary_user_changed"])
		context.reset_runtime()
		return

	context.assert_true(user is GDKUser, "default-user flow returns a GDKUser")
	if add_result != null and add_result.data != null and add_result.data is GDKUser:
		context.assert_eq(add_result.data.get_local_id(), user.get_local_id(), "default-user result data matches the cached primary user")

	var primary_user = users.get_primary_user()
	context.assert_not_null(primary_user, "get_primary_user() returns the signed-in user")
	if primary_user != null:
		context.assert_eq(primary_user.get_local_id(), user.get_local_id(), "primary user matches the signed-in user")

	context.assert_true(users.get_users().size() >= 1, "signed-in user is cached in the users service")
	context.assert_eq(user_added_events.size(), 1, "user_added emitted for the first signed-in user")
	context.assert_eq(primary_user_changed_events.size(), 1, "primary_user_changed emitted for the first signed-in user")
	context.assert_eq(user_changed_events.size(), 0, "user_changed is not emitted during the initial user add")
	context.assert_eq(user_removed_events.size(), 0, "user_removed is not emitted during the initial user add")

	context.assert_true(user.get_local_id() != 0, "signed-in user local_id is populated")
	context.assert_true(user.get_xuid().length() > 0, "signed-in user XUID is populated")
	context.assert_true(user.get_gamertag().length() > 0, "signed-in user gamertag is populated")
	context.assert_eq(user.get_sign_in_state(), GDKUser.SIGN_IN_STATE_SIGNED_IN, "signed-in user reports SIGNED_IN")
	context.assert_eq(user.is_signed_in(), true, "signed-in user reports signed_in == true")

	var privilege_op = users.check_privilege_async(user, 254)
	context.assert_not_null(privilege_op, "check_privilege_async() returns GDKAsyncOp for a signed-in user")
	if privilege_op != null:
		context.assert_true(privilege_op is GDKAsyncOp, "check_privilege_async() uses GDKAsyncOp")
		context.assert_true(privilege_op.is_done(), "check_privilege_async() completes immediately")
		var privilege_result = privilege_op.get_result()
		context.assert_not_null(privilege_result, "check_privilege_async() yields a result")
		if privilege_result != null:
			if privilege_result.ok:
				context.assert_true(privilege_result.data is Dictionary, "check_privilege_async() returns Dictionary data on success")
				if privilege_result.data is Dictionary:
					var privilege_data: Dictionary = privilege_result.data
					context.assert_eq(privilege_data["privilege"], 254, "privilege result echoes the requested privilege")
					context.assert_dict_has_key(privilege_data, "has_privilege", "privilege result includes has_privilege")
					context.assert_dict_has_key(privilege_data, "deny_reason", "privilege result includes deny_reason")
					context.assert_dict_has_key(privilege_data, "needs_user_issue_resolution", "privilege result includes issue-resolution flag")
			else:
				context.assert_true(privilege_result.code.length() > 0, "privilege failure exposes an error code")
				context.assert_true(privilege_result.message.length() > 0, "privilege failure exposes an error message")

	var invalid_picture_op = users.get_gamer_picture_async(user, "giant")
	context.assert_not_null(invalid_picture_op, "get_gamer_picture_async() returns GDKAsyncOp for a signed-in user")
	if invalid_picture_op != null:
		context.assert_result_error(invalid_picture_op.get_result(), "invalid_gamer_picture_size", "get_gamer_picture_async() rejects invalid sizes")

	var invalid_token_op = users.get_token_and_signature_async(user, "GET", " ")
	context.assert_not_null(invalid_token_op, "get_token_and_signature_async() returns GDKAsyncOp for a signed-in user")
	if invalid_token_op != null:
		context.assert_result_error(invalid_token_op.get_result(), "invalid_request_url", "get_token_and_signature_async() rejects blank URLs")

	context.disconnect_signal_handlers(users, ["user_added", "user_removed", "user_changed", "primary_user_changed"])
	context.reset_runtime()
	context.assert_true(users.get_primary_user() == null, "shutdown clears the cached primary user")
	context.assert_eq(users.get_users().size(), 0, "shutdown clears the cached users list")
