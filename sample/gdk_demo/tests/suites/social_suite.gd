extends RefCounted

func run(context) -> void:
	context.log_section("GDK Social API")

	var gdk = context.get_gdk()
	if gdk == null:
		context.log_fail("GDK root singleton missing, skipping social API group")
		return

	context.reset_runtime()

	var social = gdk.get_social()
	context.assert_not_null(social, "GDK.social returns service object")
	if social == null:
		return

	for method_name in [
		"start_social_graph",
		"stop_social_graph",
		"get_friends_async",
		"create_social_group",
		"create_social_group_from_xuids",
		"destroy_social_group",
		"get_group_users"
	]:
		context.assert_has_method(social, method_name)

	for signal_name in ["social_graph_changed", "social_group_updated", "social_user_changed"]:
		context.assert_has_signal(social, signal_name)

	var filter = context.instantiate_class("GDKSocialFilter")
	context.assert_not_null(filter, "GDKSocialFilter.new() returns wrapper")
	if filter != null:
		for method_name in ["get_presence_filter", "set_presence_filter", "get_relationship_filter", "set_relationship_filter"]:
			context.assert_has_method(filter, method_name)
		context.assert_eq(filter.get_presence_filter(), context.get_class_constant("GDKSocialFilter", "PRESENCE_FILTER_ALL"), "GDKSocialFilter presence_filter defaults to PRESENCE_FILTER_ALL")
		context.assert_eq(filter.get_relationship_filter(), context.get_class_constant("GDKSocialFilter", "RELATIONSHIP_FILTER_FRIENDS"), "GDKSocialFilter relationship_filter defaults to RELATIONSHIP_FILTER_FRIENDS")

	var group = context.instantiate_class("GDKSocialGroup")
	context.assert_not_null(group, "GDKSocialGroup.new() returns wrapper")
	if group != null:
		for method_name in ["get_local_user", "is_loaded", "get_group_type", "get_group_type_name", "get_presence_filter", "get_relationship_filter", "get_tracked_xuids"]:
			context.assert_has_method(group, method_name)
		context.assert_eq(group.is_loaded(), false, "blank GDKSocialGroup loaded defaults false")
		context.assert_eq(group.get_group_type(), context.get_class_constant("GDKSocialGroup", "GROUP_TYPE_FILTER"), "blank GDKSocialGroup group_type defaults to GROUP_TYPE_FILTER")
		context.assert_eq(group.get_group_type_name(), "filter", "blank GDKSocialGroup group_type_name defaults to filter")
		context.assert_eq(group.get_presence_filter(), context.get_class_constant("GDKSocialFilter", "PRESENCE_FILTER_ALL"), "blank GDKSocialGroup presence_filter defaults to PRESENCE_FILTER_ALL")
		context.assert_eq(group.get_relationship_filter(), context.get_class_constant("GDKSocialFilter", "RELATIONSHIP_FILTER_FRIENDS"), "blank GDKSocialGroup relationship_filter defaults to RELATIONSHIP_FILTER_FRIENDS")
		context.assert_true(group.get_tracked_xuids() is PackedStringArray, "blank GDKSocialGroup tracked_xuids returns PackedStringArray")

	var social_user = context.instantiate_class("GDKSocialUser")
	context.assert_not_null(social_user, "GDKSocialUser.new() returns wrapper")
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
			"get_preferred_color"
		]:
			context.assert_has_method(social_user, method_name)
		context.assert_eq(social_user.get_xuid(), "", "blank GDKSocialUser xuid defaults empty")
		context.assert_eq(social_user.is_friend(), false, "blank GDKSocialUser friend defaults false")
		context.assert_true(social_user.get_title_history() is Dictionary, "blank GDKSocialUser title_history returns Dictionary")
		context.assert_true(social_user.get_preferred_color() is Dictionary, "blank GDKSocialUser preferred_color returns Dictionary")

	var blank_user = context.instantiate_class("GDKUser")
	context.assert_true(social.create_social_group(blank_user) == null, "create_social_group() returns null when the graph cannot start")
	context.assert_true(social.create_social_group_from_xuids(blank_user, PackedStringArray(["1"])) == null, "create_social_group_from_xuids() returns null when the graph cannot start")
	context.assert_true(social.get_group_users(null) is Array, "get_group_users() returns Array")

	var init_result = context.initialize_runtime()
	context.assert_not_null(init_result, "GDK.initialize() for social behavior returns GDKResult")
	if init_result == null:
		return
	if not init_result.ok:
		context.log_skip("Social runtime behavior", init_result.message)
		return

	var sign_in = context.ensure_primary_user()
	var sign_in_op = sign_in["op"]
	var sign_in_result = sign_in["result"]
	var user = sign_in["user"]
	if sign_in_op != null and sign_in_result == null:
		context.log_fail("Default-user flow for social completes", "timed out waiting for a signed-in user")
		context.reset_runtime()
		return
	if user == null:
		if sign_in_result != null and not sign_in_result.ok:
			context.assert_true(sign_in_result.code.length() > 0, "failed social sign-in exposes an error code")
			context.assert_true(sign_in_result.message.length() > 0, "failed social sign-in exposes an error message")
			context.log_skip("Social runtime behavior", sign_in_result.message)
		else:
			context.log_skip("Social runtime behavior", "No signed-in user is available on this machine.")
		context.reset_runtime()
		return

	var runtime_errors: Array = []
	gdk.connect("runtime_error", func(result): runtime_errors.append(result))

	var start_result = social.start_social_graph(user)
	context.assert_not_null(start_result, "start_social_graph() returns GDKResult for a signed-in user")
	if start_result == null:
		context.disconnect_signal_handlers(gdk, ["runtime_error"])
		context.reset_runtime()
		return

	if start_result.ok:
		var friends_op = social.get_friends_async(user)
		context.assert_not_null(friends_op, "get_friends_async() returns GDKAsyncOp for a signed-in user")
		if friends_op != null:
			context.assert_object_is(friends_op, "GDKAsyncOp", "get_friends_async() uses async op surface")
			var friends_result = context.wait_for_op(friends_op, 8000)
			if friends_result == null:
				friends_op.cancel()
				context.log_skip("get_friends_async()", "Timed out waiting for the friends group to finish loading.")
				social.stop_social_graph(user)
				context.disconnect_signal_handlers(gdk, ["runtime_error"])
				context.reset_runtime()
				return

			if friends_result.ok:
				context.assert_object_is(friends_result.data, "GDKSocialGroup", "friends query returns a GDKSocialGroup on success")
				if context.is_class_instance(friends_result.data, "GDKSocialGroup"):
					var friends_group = friends_result.data
					context.assert_eq(friends_group.is_loaded(), true, "friends group reports loaded after a successful query")
					var local_user = friends_group.get_local_user()
					context.assert_not_null(local_user, "friends group keeps its local user reference")
					if local_user != null:
						context.assert_eq(local_user.get_local_id(), user.get_local_id(), "friends group local user matches the signed-in user")

					var group_users = social.get_group_users(friends_group)
					context.assert_true(group_users is Array, "get_group_users() returns Array for a loaded group")
					if group_users.size() > 0:
						context.assert_object_is(group_users[0], "GDKSocialUser", "loaded social groups return GDKSocialUser wrappers")

				var runtime_error_count = runtime_errors.size()
				var invalid_group = social.create_social_group_from_xuids(user, PackedStringArray())
				context.assert_true(invalid_group == null, "create_social_group_from_xuids() rejects empty XUID lists")

				var social_last_error = gdk.get_last_error()
				context.assert_not_null(social_last_error, "social validation failures update the root last error")
				if social_last_error != null:
					context.assert_eq(social_last_error.code, "missing_social_group_xuids", "empty social groups report missing_social_group_xuids")

				context.assert_eq(runtime_errors.size(), runtime_error_count + 1, "empty social groups emit runtime_error")
			else:
				context.assert_true(friends_result.code.length() > 0, "friends query failure exposes an error code")
				context.assert_true(friends_result.message.length() > 0, "friends query failure exposes an error message")
				context.log_skip("Loaded friends-group behavior", friends_result.message)

		social.stop_social_graph(user)
	else:
		context.assert_true(start_result.code.length() > 0, "social graph start failure exposes an error code")
		context.assert_true(start_result.message.length() > 0, "social graph start failure exposes an error message")
		context.log_skip("Social graph behavior", start_result.message)

	context.disconnect_signal_handlers(gdk, ["runtime_error"])
	context.reset_runtime()
