extends "res://addons/godot_gdk_tests/gdk_test_base.gd"
## Wave 3 GUT migration of `suites/social_suite.gd`. Behavior parity:
## same per-call assertion count as the pre-GUT harness; `log_skip` mapped to
## `pending(...)`; one-off `log_fail` early-returns preserved as
## `assert_true(false, ...)` so failures still fail the suite.

func before_each() -> void:
	reset_runtime()


func after_each() -> void:
	reset_runtime()


func test_social_full_flow() -> void:
	if pending_unless_runtime_available():
		return

	var gdk = get_gdk()
	var social = gdk.get_social()
	assert_not_null(social, "GDK.social returns service object")
	if social == null:
		return

	for method_name in [
		"start_social_graph",
		"stop_social_graph",
		"get_friends_async",
		"create_social_group",
		"create_social_group_from_xuids",
		"destroy_social_group",
		"get_group_users",
		"submit_reputation_feedback_async",
		"submit_batch_reputation_feedback_async",
	]:
		assert_has_method_named(social, method_name)

	for signal_name in ["social_graph_changed", "social_group_updated", "social_user_changed"]:
		assert_has_signal_named(social, signal_name)

	var filter = instantiate_class("GDKSocialFilter")
	assert_not_null(filter, "GDKSocialFilter.new() returns wrapper")
	if filter != null:
		for method_name in ["get_presence_filter", "set_presence_filter", "get_relationship_filter", "set_relationship_filter"]:
			assert_has_method_named(filter, method_name)
		assert_eq(filter.get_presence_filter(), get_class_constant("GDKSocialFilter", "PRESENCE_FILTER_ALL"), "GDKSocialFilter presence_filter defaults to PRESENCE_FILTER_ALL")
		assert_eq(filter.get_relationship_filter(), get_class_constant("GDKSocialFilter", "RELATIONSHIP_FILTER_FRIENDS"), "GDKSocialFilter relationship_filter defaults to RELATIONSHIP_FILTER_FRIENDS")

	var group = instantiate_class("GDKSocialGroup")
	assert_not_null(group, "GDKSocialGroup.new() returns wrapper")
	if group != null:
		for method_name in ["get_local_user", "is_loaded", "get_group_type", "get_group_type_name", "get_presence_filter", "get_relationship_filter", "get_tracked_xuids"]:
			assert_has_method_named(group, method_name)
		assert_eq(group.is_loaded(), false, "blank GDKSocialGroup loaded defaults false")
		assert_eq(group.get_group_type(), get_class_constant("GDKSocialGroup", "GROUP_TYPE_FILTER"), "blank GDKSocialGroup group_type defaults to GROUP_TYPE_FILTER")
		assert_eq(group.get_group_type_name(), "filter", "blank GDKSocialGroup group_type_name defaults to filter")
		assert_eq(group.get_presence_filter(), get_class_constant("GDKSocialFilter", "PRESENCE_FILTER_ALL"), "blank GDKSocialGroup presence_filter defaults to PRESENCE_FILTER_ALL")
		assert_eq(group.get_relationship_filter(), get_class_constant("GDKSocialFilter", "RELATIONSHIP_FILTER_FRIENDS"), "blank GDKSocialGroup relationship_filter defaults to RELATIONSHIP_FILTER_FRIENDS")
		assert_true(group.get_tracked_xuids() is PackedStringArray, "blank GDKSocialGroup tracked_xuids returns PackedStringArray")

	var social_user = instantiate_class("GDKSocialUser")
	assert_not_null(social_user, "GDKSocialUser.new() returns wrapper")
	if social_user != null:
		for method_name in [
			"get_xuid",
			"is_favorite",
			"is_friend",
			"is_following_user",
			"is_followed_by_caller",
			"get_display_name",
			"get_real_name",
			"get_display_picture_url",
			"uses_avatar",
			"get_gamerscore",
			"get_gamertag",
			"get_modern_gamertag",
			"get_modern_gamertag_suffix",
			"get_unique_modern_gamertag",
			"get_presence",
			"get_title_history",
			"get_preferred_color",
		]:
			assert_has_method_named(social_user, method_name)
		assert_eq(social_user.get_xuid(), "", "blank GDKSocialUser xuid defaults empty")
		assert_eq(social_user.is_friend(), false, "blank GDKSocialUser friend defaults false")
		assert_true(social_user.get_title_history() is Dictionary, "blank GDKSocialUser title_history returns Dictionary")
		assert_true(social_user.get_preferred_color() is Dictionary, "blank GDKSocialUser preferred_color returns Dictionary")

	var blank_user = instantiate_class("GDKUser")
	assert_true(social.create_social_group(blank_user) == null, "create_social_group() returns null when the graph cannot start")
	assert_true(social.create_social_group_from_xuids(blank_user, PackedStringArray(["1"])) == null, "create_social_group_from_xuids() returns null when the graph cannot start")
	assert_true(social.get_group_users(null) is Array, "get_group_users() returns Array")

	var init_result = initialize_runtime()
	assert_not_null(init_result, "GDK.initialize() for social behavior returns GDKResult")
	if init_result == null:
		return
	if not init_result.ok:
		pending("Social runtime behavior: %s" % init_result.message)
		return

	var sign_in = await ensure_primary_user()
	var sign_in_signal = sign_in["signal"]
	var sign_in_result = sign_in["result"]
	var user = sign_in["user"]
	if typeof(sign_in_signal) == TYPE_SIGNAL and sign_in_result == null:
		assert_true(false, "Default-user flow for social completes — timed out waiting for a signed-in user")
		return
	if user == null:
		if sign_in_result != null and not sign_in_result.ok:
			assert_true(sign_in_result.code.length() > 0, "failed social sign-in exposes an error code")
			assert_true(sign_in_result.message.length() > 0, "failed social sign-in exposes an error message")
			pending("Social runtime behavior: %s" % sign_in_result.message)
		else:
			pending("Social runtime behavior: No signed-in user is available on this machine.")
		return

	var runtime_errors: Array = []
	gdk.connect("runtime_error", func(result): runtime_errors.append(result))

	var invalid_feedback_xuid_signal = social.submit_reputation_feedback_async(user, "not-a-number", "fair_play_cheater")
	await assert_signal_result_error(invalid_feedback_xuid_signal, "invalid_xuid", "submit_reputation_feedback_async() rejects non-numeric XUID strings")

	var invalid_feedback_type_signal = social.submit_reputation_feedback_async(user, "1", "not_a_feedback_type")
	await assert_signal_result_error(invalid_feedback_type_signal, "invalid_feedback_type", "submit_reputation_feedback_async() rejects unknown feedback types")

	var empty_batch_signal = social.submit_batch_reputation_feedback_async(user, [])
	await assert_signal_result_error(empty_batch_signal, "invalid_feedback_items", "submit_batch_reputation_feedback_async() rejects empty batches")

	var invalid_batch_item_signal = social.submit_batch_reputation_feedback_async(user, ["not a dictionary"])
	await assert_signal_result_error(invalid_batch_item_signal, "invalid_feedback_item", "submit_batch_reputation_feedback_async() rejects non-dictionary items")

	var missing_batch_keys_signal = social.submit_batch_reputation_feedback_async(user, [{"target_xuid": "1"}])
	await assert_signal_result_error(missing_batch_keys_signal, "invalid_feedback_item", "submit_batch_reputation_feedback_async() rejects missing required keys")

	var start_result = social.start_social_graph(user)
	assert_not_null(start_result, "start_social_graph() returns GDKResult for a signed-in user")
	if start_result == null:
		disconnect_signal_handlers(gdk, ["runtime_error"])
		return

	if start_result.ok:
		var friends_signal = social.get_friends_async(user)
		assert_true(typeof(friends_signal) == TYPE_SIGNAL, "get_friends_async() returns completion Signal")
		if typeof(friends_signal) == TYPE_SIGNAL:
			var friends_result = await await_completion(friends_signal, 8000)
			if friends_result == null:
				pending("get_friends_async(): Timed out waiting for the friends group to finish loading.")
				social.stop_social_graph(user)
				disconnect_signal_handlers(gdk, ["runtime_error"])
				return

			if friends_result.ok:
				assert_object_is(friends_result.data, "GDKSocialGroup", "friends query returns a GDKSocialGroup on success")
				if is_class_instance(friends_result.data, "GDKSocialGroup"):
					var friends_group = friends_result.data
					assert_eq(friends_group.is_loaded(), true, "friends group reports loaded after a successful query")
					var local_user = friends_group.get_local_user()
					assert_not_null(local_user, "friends group keeps its local user reference")
					if local_user != null:
						assert_eq(local_user.get_local_id(), user.get_local_id(), "friends group local user matches the signed-in user")

					var group_users = social.get_group_users(friends_group)
					assert_true(group_users is Array, "get_group_users() returns Array for a loaded group")
					if group_users.size() > 0:
						assert_object_is(group_users[0], "GDKSocialUser", "loaded social groups return GDKSocialUser wrappers")

				var runtime_error_count = runtime_errors.size()
				var invalid_group = social.create_social_group_from_xuids(user, PackedStringArray())
				assert_true(invalid_group == null, "create_social_group_from_xuids() rejects empty XUID lists")

				var social_last_error = gdk.get_last_error()
				assert_not_null(social_last_error, "social validation failures update the root last error")
				if social_last_error != null:
					assert_eq(social_last_error.code, "missing_social_group_xuids", "empty social groups report missing_social_group_xuids")

				assert_eq(runtime_errors.size(), runtime_error_count + 1, "empty social groups emit runtime_error")
			else:
				assert_true(friends_result.code.length() > 0, "friends query failure exposes an error code")
				assert_true(friends_result.message.length() > 0, "friends query failure exposes an error message")
				pending("Loaded friends-group behavior: %s" % friends_result.message)

		social.stop_social_graph(user)
	else:
		assert_true(start_result.code.length() > 0, "social graph start failure exposes an error code")
		assert_true(start_result.message.length() > 0, "social graph start failure exposes an error message")
		pending("Social graph behavior: %s" % start_result.message)

	disconnect_signal_handlers(gdk, ["runtime_error"])
