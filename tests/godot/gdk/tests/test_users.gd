extends "res://addons/godot_gdk_tests/gdk_test_base.gd"
## Wave 3 GUT migration of `suites/users_suite.gd`. Behavior parity:
## same per-call assertion count as the pre-GUT harness; `log_skip` mapped to
## `pending(...)`; one-off `log_fail` early-returns preserved as
## `assert_true(false, ...)` so failures still fail the suite.

func before_each() -> void:
	reset_runtime()


func after_each() -> void:
	reset_runtime()


func test_users_full_flow() -> void:
	if pending_unless_runtime_available():
		return

	var gdk = get_gdk()
	var users = gdk.get_users()
	assert_not_null(users, "GDK.users returns service object")
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
		"get_token_and_signature_async",
	]:
		assert_has_method_named(users, method_name)

	assert_has_signal_named(users, "user_changed")
	for removed_signal_name in ["user_added", "user_removed", "primary_user_changed"]:
		assert_false(users.has_signal(removed_signal_name), "GDK.users exposes only user_changed, not %s" % removed_signal_name)

	assert_true(users.get_users() is Array, "get_users() returns Array")
	assert_true(users.get_primary_user() == null, "get_primary_user() starts null before init")

	var blank_user = instantiate_class("GDKUser")
	assert_not_null(blank_user, "GDKUser.new() returns wrapper")
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
			"is_store_user",
		]:
			assert_has_method_named(blank_user, method_name)

		assert_eq(blank_user.get_local_id(), 0, "blank GDKUser local_id defaults to 0")
		assert_eq(blank_user.get_xuid(), "", "blank GDKUser xuid defaults empty")
		assert_eq(blank_user.get_gamertag(), "", "blank GDKUser gamertag defaults empty")
		assert_eq(blank_user.get_age_group(), get_class_constant("GDKUser", "AGE_GROUP_UNKNOWN"), "blank GDKUser age_group defaults to AGE_GROUP_UNKNOWN")
		assert_eq(blank_user.get_age_group_name(), "unknown", "blank GDKUser age_group_name defaults to unknown")
		assert_eq(blank_user.get_sign_in_state(), get_class_constant("GDKUser", "SIGN_IN_STATE_SIGNED_OUT"), "blank GDKUser sign_in_state defaults to SIGN_IN_STATE_SIGNED_OUT")
		assert_eq(blank_user.get_sign_in_state_name(), "signed_out", "blank GDKUser sign_in_state_name defaults to signed_out")
		assert_eq(blank_user.is_guest(), false, "blank GDKUser guest defaults false")
		assert_eq(blank_user.is_signed_in(), false, "blank GDKUser signed_in defaults false")
		assert_eq(blank_user.is_store_user(), false, "blank GDKUser store_user defaults false")

	var init_result = initialize_runtime()
	assert_not_null(init_result, "GDK.initialize() for users behavior returns GDKResult")
	if init_result == null:
		return
	if not init_result.ok:
		pending("Users runtime behavior: %s" % init_result.message)
		return

	assert_eq(users.get_users().size(), 0, "get_users() starts empty after init")
	assert_true(users.get_primary_user() == null, "get_primary_user() starts null after init")

	var user_changed_events: Array = []
	users.connect("user_changed", func(user_arg, change_kind_arg): user_changed_events.append({"user": user_arg, "change_kind": change_kind_arg}))

	var sign_in = await ensure_primary_user()
	var add_signal = sign_in["signal"]
	var add_result = sign_in["result"]
	var user = sign_in["user"]
	if not sign_in["had_existing_user"]:
		assert_true(typeof(add_signal) == TYPE_SIGNAL, "add_default_user_async() returns completion Signal")
	if typeof(add_signal) == TYPE_SIGNAL and add_result == null:
		assert_true(false, "add_default_user_async() completes — timed out waiting for the default user flow")
		disconnect_signal_handlers(users, ["user_changed"])
		return

	if add_result != null and not add_result.ok:
		assert_true(add_result.code.length() > 0, "failed default-user add exposes an error code")
		assert_true(add_result.message.length() > 0, "failed default-user add exposes an error message")
		assert_true(user == null, "failed default-user add leaves primary user unavailable")
		assert_eq(users.get_users().size(), 0, "failed default-user add keeps the user cache empty")

		pending("Signed-in user behavior: %s" % add_result.message)
		disconnect_signal_handlers(users, ["user_changed"])
		return

	if user == null:
		pending("Signed-in user behavior: No default user is available on this machine.")
		disconnect_signal_handlers(users, ["user_changed"])
		return

	assert_object_is(user, "GDKUser", "default-user flow returns a GDKUser")
	if add_result != null and is_class_instance(add_result.data, "GDKUser"):
		assert_eq(add_result.data.get_local_id(), user.get_local_id(), "default-user result data matches the cached primary user")

	var primary_user = users.get_primary_user()
	assert_not_null(primary_user, "get_primary_user() returns the signed-in user")
	if primary_user != null:
		assert_eq(primary_user.get_local_id(), user.get_local_id(), "primary user matches the signed-in user")

	assert_true(users.get_users().size() >= 1, "signed-in user is cached in the users service")
	assert_eq(user_changed_events.size(), 1, "user_changed emitted for the first signed-in user")
	if user_changed_events.size() > 0:
		assert_eq(user_changed_events[0]["user"].get_local_id(), user.get_local_id(), "user_changed identifies the signed-in user")
		assert_eq(user_changed_events[0]["change_kind"], "added", "user_changed reports added for a newly cached user")

	assert_true(user.get_local_id() != 0, "signed-in user local_id is populated")
	assert_true(user.get_xuid().length() > 0, "signed-in user XUID is populated")
	assert_true(user.get_gamertag().length() > 0, "signed-in user gamertag is populated")
	assert_eq(user.get_sign_in_state(), get_class_constant("GDKUser", "SIGN_IN_STATE_SIGNED_IN"), "signed-in user reports SIGNED_IN")
	assert_eq(user.is_signed_in(), true, "signed-in user reports signed_in == true")

	var privilege_signal = users.check_privilege_async(user, 254)
	assert_true(typeof(privilege_signal) == TYPE_SIGNAL, "check_privilege_async() returns completion Signal for a signed-in user")
	if typeof(privilege_signal) == TYPE_SIGNAL:
		var privilege_result = await await_completion(privilege_signal)
		assert_not_null(privilege_result, "check_privilege_async() yields a result")
		if privilege_result != null:
			if privilege_result.ok:
				assert_true(privilege_result.data is Dictionary, "check_privilege_async() returns Dictionary data on success")
				if privilege_result.data is Dictionary:
					var privilege_data: Dictionary = privilege_result.data
					assert_eq(privilege_data["privilege"], 254, "privilege result echoes the requested privilege")
					assert_dict_has_key(privilege_data, "has_privilege", "privilege result includes has_privilege")
					assert_dict_has_key(privilege_data, "deny_reason", "privilege result includes deny_reason")
					assert_dict_has_key(privilege_data, "needs_user_issue_resolution", "privilege result includes issue-resolution flag")
			else:
				assert_true(privilege_result.code.length() > 0, "privilege failure exposes an error code")
				assert_true(privilege_result.message.length() > 0, "privilege failure exposes an error message")

	var invalid_picture_signal = users.get_gamer_picture_async(user, "giant")
	await assert_signal_result_error(invalid_picture_signal, "invalid_gamer_picture_size", "get_gamer_picture_async() rejects invalid sizes")

	var invalid_token_signal = users.get_token_and_signature_async(user, "GET", " ")
	await assert_signal_result_error(invalid_token_signal, "invalid_request_url", "get_token_and_signature_async() rejects blank URLs")

	disconnect_signal_handlers(users, ["user_changed"])
	reset_runtime()
	assert_true(users.get_primary_user() == null, "shutdown clears the cached primary user")
	assert_eq(users.get_users().size(), 0, "shutdown clears the cached users list")
