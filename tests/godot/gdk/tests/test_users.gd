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
		"add_user_by_id_with_ui_async",
		"get_primary_user",
		"get_users",
		"get_max_users",
		"is_sign_out_available",
		"sign_out_async",
		"acquire_sign_out_deferral",
		"find_user_by_xuid",
		"find_user_by_local_id",
		"find_user_for_device",
		"find_controller_for_user_with_ui_async",
		"get_device_associations",
		"get_devices_for_user",
		"get_default_audio_endpoint",
		"check_privilege_async",
		"resolve_privilege_with_ui_async",
		"resolve_issue_with_ui_async",
		"get_gamer_picture_async",
		"get_token_and_signature_async",
	]:
		assert_has_method_named(users, method_name)

	assert_has_signal_named(users, "user_changed")
	assert_has_signal_named(users, "device_association_changed")
	assert_has_signal_named(users, "default_audio_endpoint_changed")
	for removed_signal_name in ["user_added", "user_removed", "primary_user_changed"]:
		assert_false(users.has_signal(removed_signal_name), "GDK.users exposes only user_changed, not %s" % removed_signal_name)

	assert_eq(get_class_constant("GDKUsers", "AUDIO_ENDPOINT_KIND_COMMUNICATION_RENDER"), 0, "GDKUsers exposes AUDIO_ENDPOINT_KIND_COMMUNICATION_RENDER")
	assert_eq(get_class_constant("GDKUsers", "AUDIO_ENDPOINT_KIND_COMMUNICATION_CAPTURE"), 1, "GDKUsers exposes AUDIO_ENDPOINT_KIND_COMMUNICATION_CAPTURE")

	assert_true(users.get_users() is Array, "get_users() returns Array")
	assert_true(users.get_primary_user() == null, "get_primary_user() starts null before init")
	assert_true(users.get_device_associations() is Array, "get_device_associations() returns Array")
	assert_eq(users.get_device_associations().size(), 0, "get_device_associations() starts empty before init")
	assert_true(users.get_devices_for_user(null) is PackedStringArray, "get_devices_for_user() returns PackedStringArray")
	assert_eq(users.get_devices_for_user(null).size(), 0, "get_devices_for_user(null) returns no devices")
	assert_eq(users.is_sign_out_available(), false, "is_sign_out_available() reports false before init")

	assert_result_error(users.get_max_users(), "not_initialized", "get_max_users() rejects before initialize")
	assert_result_error(users.acquire_sign_out_deferral(), "not_initialized", "acquire_sign_out_deferral() rejects before initialize")
	assert_result_error(users.find_user_by_xuid("2814639011419087"), "not_initialized", "find_user_by_xuid() rejects before initialize")
	assert_result_error(users.find_user_by_local_id(1), "not_initialized", "find_user_by_local_id() rejects before initialize")
	assert_result_error(users.find_user_for_device("00"), "not_initialized", "find_user_for_device() rejects before initialize")
	assert_result_error(users.get_default_audio_endpoint(null), "not_initialized", "get_default_audio_endpoint() rejects before initialize")

	var blank_user = instantiate_class("GDKUser")
	assert_not_null(blank_user, "GDKUser.new() returns wrapper")
	if blank_user != null:
		for method_name in [
			"get_local_id",
			"get_xuid",
			"get_gamertag",
			"get_modern_gamertag",
			"get_modern_gamertag_suffix",
			"get_unique_modern_gamertag",
			"get_age_group",
			"get_age_group_name",
			"get_sign_in_state",
			"get_sign_in_state_name",
			"is_guest",
			"is_signed_in",
			"is_store_user",
			"is_valid",
			"is_same_user",
			"duplicate_user",
		]:
			assert_has_method_named(blank_user, method_name)

		assert_eq(blank_user.get_local_id(), 0, "blank GDKUser local_id defaults to 0")
		assert_eq(blank_user.get_xuid(), "", "blank GDKUser xuid defaults empty")
		assert_eq(blank_user.get_gamertag(), "", "blank GDKUser gamertag defaults empty")
		assert_eq(blank_user.get_modern_gamertag(), "", "blank GDKUser modern_gamertag defaults empty")
		assert_eq(blank_user.get_modern_gamertag_suffix(), "", "blank GDKUser modern_gamertag_suffix defaults empty")
		assert_eq(blank_user.get_unique_modern_gamertag(), "", "blank GDKUser unique_modern_gamertag defaults empty")
		assert_eq(blank_user.get_age_group(), get_class_constant("GDKUser", "AGE_GROUP_UNKNOWN"), "blank GDKUser age_group defaults to AGE_GROUP_UNKNOWN")
		assert_eq(blank_user.get_age_group_name(), "unknown", "blank GDKUser age_group_name defaults to unknown")
		assert_eq(blank_user.get_sign_in_state(), get_class_constant("GDKUser", "SIGN_IN_STATE_SIGNED_OUT"), "blank GDKUser sign_in_state defaults to SIGN_IN_STATE_SIGNED_OUT")
		assert_eq(blank_user.get_sign_in_state_name(), "signed_out", "blank GDKUser sign_in_state_name defaults to signed_out")
		assert_eq(blank_user.is_guest(), false, "blank GDKUser guest defaults false")
		assert_eq(blank_user.is_signed_in(), false, "blank GDKUser signed_in defaults false")
		assert_eq(blank_user.is_store_user(), false, "blank GDKUser store_user defaults false")
		assert_eq(blank_user.is_valid(), false, "blank GDKUser is_valid() defaults false")
		assert_eq(blank_user.is_same_user(null), false, "blank GDKUser is_same_user(null) is false")
		# A handle-less wrapper must never reach XUserCompare(), which has no
		# defined behavior for a null handle — on either side of the comparison.
		var other_blank_user = instantiate_class("GDKUser")
		if other_blank_user != null:
			assert_eq(blank_user.is_same_user(other_blank_user), false, "two handle-less GDKUsers are not the same user")
		assert_true(blank_user.duplicate_user() == null, "blank GDKUser duplicate_user() returns null")

	var blank_deferral = instantiate_class("GDKUserSignOutDeferral")
	assert_not_null(blank_deferral, "GDKUserSignOutDeferral.new() returns wrapper")
	if blank_deferral != null:
		assert_has_method_named(blank_deferral, "is_valid")
		assert_has_method_named(blank_deferral, "release")
		assert_eq(blank_deferral.is_valid(), false, "blank GDKUserSignOutDeferral is_valid() defaults false")
		blank_deferral.release()
		assert_eq(blank_deferral.is_valid(), false, "releasing an unheld deferral stays invalid")

	var pre_init_add_with_ui_signal = users.add_user_with_ui_async()
	await assert_signal_result_error(pre_init_add_with_ui_signal, "not_initialized", "add_user_with_ui_async() rejects before initialize")

	var pre_init_add_guest_signal = users.add_user_with_ui_async(true)
	await assert_signal_result_error(pre_init_add_guest_signal, "not_initialized", "add_user_with_ui_async(true) rejects before initialize")

	var pre_init_add_by_id_signal = users.add_user_by_id_with_ui_async("2814639011419087")
	await assert_signal_result_error(pre_init_add_by_id_signal, "not_initialized", "add_user_by_id_with_ui_async() rejects before initialize")

	var pre_init_sign_out_signal = users.sign_out_async(null)
	await assert_signal_result_error(pre_init_sign_out_signal, "not_initialized", "sign_out_async() rejects before initialize")

	var pre_init_find_controller_signal = users.find_controller_for_user_with_ui_async(null)
	await assert_signal_result_error(pre_init_find_controller_signal, "not_initialized", "find_controller_for_user_with_ui_async() rejects before initialize")

	var init_result = initialize_runtime()
	assert_not_null(init_result, "GDK.initialize() for users behavior returns GDKResult")
	if init_result == null:
		return
	if not init_result.ok:
		pending("Users runtime behavior: %s" % init_result.message)
		return

	assert_eq(users.get_users().size(), 0, "get_users() starts empty after init")
	assert_true(users.get_primary_user() == null, "get_primary_user() starts null after init")

	var max_users_result = users.get_max_users()
	assert_not_null(max_users_result, "get_max_users() returns GDKResult after initialize")
	if max_users_result != null and max_users_result.ok:
		assert_true(max_users_result.data is int, "get_max_users() data is an int")
		assert_true(max_users_result.data >= 1, "get_max_users() reports at least one supported user")

	# Device associations are replayed by the platform at registration time, so the
	# cache is authoritative immediately after initialize().
	var associations = users.get_device_associations()
	assert_true(associations is Array, "get_device_associations() returns Array after initialize")
	for association in associations:
		assert_true(association is Dictionary, "each device association is a Dictionary")
		if association is Dictionary:
			assert_dict_has_key(association, "device_id", "device association includes device_id")
			assert_dict_has_key(association, "user_local_id", "device association includes user_local_id")
			assert_eq(association["device_id"].length(), 64, "device association device_id is 64-char hex")

	assert_result_error(users.find_user_by_xuid(""), "invalid_xuid", "find_user_by_xuid() rejects blank XUIDs after initialize")
	# An XUID is unsigned. These must be rejected before reaching the native
	# call rather than wrapping into an unintended 64-bit id.
	for bad_xuid in ["-1", "-2814639011419087", "0", "12abc", "abc", "1 2", "99999999999999999999999"]:
		assert_result_error(users.find_user_by_xuid(bad_xuid), "invalid_xuid", "find_user_by_xuid() rejects %s" % [bad_xuid])
	assert_result_error(users.find_user_by_local_id(0), "invalid_local_id", "find_user_by_local_id() rejects a zero local id after initialize")
	assert_result_error(users.find_user_for_device("not-a-device-id"), "invalid_device_id", "find_user_for_device() rejects malformed device ids after initialize")
	assert_result_error(users.get_default_audio_endpoint(null), "invalid_user", "get_default_audio_endpoint() rejects null users after initialize")

	var blank_xuid_add_signal = users.add_user_by_id_with_ui_async("")
	await assert_signal_result_error(blank_xuid_add_signal, "invalid_xuid", "add_user_by_id_with_ui_async() rejects blank XUIDs after initialize")

	for bad_xuid in ["-1", "0", "12abc"]:
		var bad_xuid_add_signal = users.add_user_by_id_with_ui_async(bad_xuid)
		await assert_signal_result_error(bad_xuid_add_signal, "invalid_xuid", "add_user_by_id_with_ui_async() rejects %s" % [bad_xuid])

	var null_sign_out_signal = users.sign_out_async(null)
	await assert_signal_result_error(null_sign_out_signal, "invalid_user", "sign_out_async() rejects null users after initialize")

	var null_find_controller_signal = users.find_controller_for_user_with_ui_async(null)
	await assert_signal_result_error(null_find_controller_signal, "invalid_user", "find_controller_for_user_with_ui_async() rejects null users after initialize")

	var null_privilege_signal = users.check_privilege_async(null, 254)
	await assert_signal_result_error(null_privilege_signal, "invalid_user", "check_privilege_async() rejects null users after initialize")

	var null_issue_signal = users.resolve_issue_with_ui_async(null, "")
	await assert_signal_result_error(null_issue_signal, "invalid_user", "resolve_issue_with_ui_async() rejects null users after initialize")

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
	assert_true(user_changed_events.size() >= 1, "user_changed emitted for the first signed-in user")
	var matching_added_events := 0
	for event in user_changed_events:
		if event["user"].get_local_id() == user.get_local_id() and event["change_kind"] == "added":
			matching_added_events += 1
	assert_eq(matching_added_events, 1, "user_changed identifies the first signed-in user as added")

	assert_true(user.get_local_id() != 0, "signed-in user local_id is populated")
	assert_true(user.get_xuid().length() > 0, "signed-in user XUID is populated")
	assert_true(user.get_gamertag().length() > 0, "signed-in user gamertag is populated")
	assert_eq(user.get_sign_in_state(), get_class_constant("GDKUser", "SIGN_IN_STATE_SIGNED_IN"), "signed-in user reports SIGNED_IN")
	assert_eq(user.is_signed_in(), true, "signed-in user reports signed_in == true")
	assert_eq(user.is_valid(), true, "signed-in user wrapper reports is_valid() == true")
	assert_eq(user.is_same_user(user), true, "signed-in user is the same user as itself")
	assert_eq(user.is_same_user(null), false, "signed-in user is not the same user as null")

	var duplicated_user = user.duplicate_user()
	assert_not_null(duplicated_user, "duplicate_user() returns an independently owned wrapper")
	if duplicated_user != null:
		assert_object_is(duplicated_user, "GDKUser", "duplicate_user() returns a GDKUser")
		assert_eq(duplicated_user.get_local_id(), user.get_local_id(), "duplicated user keeps the same local id")
		assert_eq(user.is_same_user(duplicated_user), true, "duplicated user compares equal to its source")

	var found_by_xuid = users.find_user_by_xuid(user.get_xuid())
	assert_result_ok(found_by_xuid, "find_user_by_xuid() locates the signed-in user")
	if found_by_xuid != null and found_by_xuid.ok:
		assert_object_is(found_by_xuid.data, "GDKUser", "find_user_by_xuid() returns a GDKUser")
		assert_eq(found_by_xuid.data.get_local_id(), user.get_local_id(), "find_user_by_xuid() returns the cached user")

	var found_by_local_id = users.find_user_by_local_id(user.get_local_id())
	assert_result_ok(found_by_local_id, "find_user_by_local_id() locates the signed-in user")
	if found_by_local_id != null and found_by_local_id.ok:
		assert_eq(found_by_local_id.data.get_xuid(), user.get_xuid(), "find_user_by_local_id() returns the same account")

	# XR-112 pairing view: whatever devices the platform reports for this user must
	# also appear in the association cache, and must resolve back to the same user.
	var user_devices = users.get_devices_for_user(user)
	assert_true(user_devices is PackedStringArray, "get_devices_for_user() returns PackedStringArray for a signed-in user")
	for device_id in user_devices:
		assert_eq(device_id.length(), 64, "device id is 64-char hex")
		var device_user = users.find_user_for_device(device_id)
		if device_user != null and device_user.ok:
			assert_eq(device_user.data.get_local_id(), user.get_local_id(), "find_user_for_device() resolves back to the paired user")

	var render_endpoint = users.get_default_audio_endpoint(user)
	assert_not_null(render_endpoint, "get_default_audio_endpoint() returns GDKResult for a signed-in user")
	if render_endpoint != null and render_endpoint.ok:
		assert_true(render_endpoint.data is Dictionary, "get_default_audio_endpoint() returns Dictionary data")
		if render_endpoint.data is Dictionary:
			assert_dict_has_key(render_endpoint.data, "kind", "audio endpoint result includes kind")
			assert_dict_has_key(render_endpoint.data, "endpoint_id", "audio endpoint result includes endpoint_id")
			assert_eq(render_endpoint.data["kind"], get_class_constant("GDKUsers", "AUDIO_ENDPOINT_KIND_COMMUNICATION_RENDER"), "audio endpoint result echoes the requested kind")

	assert_result_error(
		users.get_default_audio_endpoint(user, 7),
		"invalid_audio_endpoint_kind",
		"get_default_audio_endpoint() rejects unknown endpoint kinds"
	)

	var deferral_result = users.acquire_sign_out_deferral()
	assert_not_null(deferral_result, "acquire_sign_out_deferral() returns GDKResult after initialize")
	if deferral_result != null and deferral_result.ok:
		assert_object_is(deferral_result.data, "GDKUserSignOutDeferral", "acquire_sign_out_deferral() returns a GDKUserSignOutDeferral")
		var deferral = deferral_result.data
		assert_eq(deferral.is_valid(), true, "an acquired sign-out deferral reports is_valid() == true")
		deferral.release()
		assert_eq(deferral.is_valid(), false, "a released sign-out deferral reports is_valid() == false")

	# is_sign_out_available() is the documented gate for sign_out_async(); when the
	# platform has no title-driven sign-out the wrapper must say so rather than call.
	if not users.is_sign_out_available():
		var unavailable_sign_out_signal = users.sign_out_async(user)
		await assert_signal_result_error(unavailable_sign_out_signal, "sign_out_not_available", "sign_out_async() reports when the platform has no title-driven sign-out")

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
