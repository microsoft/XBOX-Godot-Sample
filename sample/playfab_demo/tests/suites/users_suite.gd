extends RefCounted


func run(context) -> void:
	_test_users_api(context)
	_test_user_wrapper_defaults(context)
	_test_not_initialized_sign_in(context)


func _test_users_api(context) -> void:
	context.log_section("PlayFab Users API")

	var playfab = context.get_playfab()
	if playfab == null:
		context.log_fail("PlayFab singleton missing, skipping users API group")
		return

	var users = playfab.get_users()
	context.assert_object_is(users, "PlayFabUsers", "PlayFab.get_users() returns PlayFabUsers")
	if users == null:
		return

	for method_name in ["sign_in_async", "get_user_by_local_id", "get_user", "get_users"]:
		context.assert_has_method(users, method_name)

	for signal_name in ["user_signed_in", "user_signed_out", "user_changed"]:
		context.assert_true(not users.has_signal(signal_name), "PlayFabUsers.%s is not exposed" % signal_name)

	var cached_users = users.get_users()
	context.assert_true(cached_users is Array, "PlayFabUsers.get_users() returns Array")
	context.assert_eq(cached_users.size(), 0, "PlayFabUsers cache starts empty")
	context.assert_eq(users.get_user_by_local_id(1), null, "get_user_by_local_id() returns null with no cached user")
	context.assert_eq(users.get_user(1), null, "get_user(int) returns null with no cached user")


func _test_user_wrapper_defaults(context) -> void:
	context.log_section("PlayFabUser Wrapper")

	var user = context.instantiate_class("PlayFabUser")
	context.assert_not_null(user, "PlayFabUser.new()")
	if user == null:
		return

	context.assert_eq(int(user.local_id), 0, "blank PlayFabUser.local_id defaults to 0")

	var entity_key: Dictionary = user.entity_key
	context.assert_eq(str(entity_key.get("id", "")), "", "blank PlayFabUser.entity_key.id defaults empty")
	context.assert_eq(str(entity_key.get("type", "")), "", "blank PlayFabUser.entity_key.type defaults empty")


func _test_not_initialized_sign_in(context) -> void:
	context.log_section("PlayFab Sign-In Validation")

	var playfab = context.get_playfab()
	if playfab == null:
		context.log_fail("PlayFab singleton missing, skipping sign-in validation")
		return

	context.reset_playfab_runtime()

	var root_signal = playfab.sign_in_async(1)
	await context.assert_signal_result_error(root_signal, "not_initialized", "PlayFab.sign_in_async() before initialize()")

	var users_signal = playfab.get_users().sign_in_async(1)
	await context.assert_signal_result_error(users_signal, "not_initialized", "PlayFab.users.sign_in_async() before initialize()")

	var last_error = playfab.get_last_error()
	context.assert_result_error(last_error, "not_initialized", "PlayFab.get_last_error() tracks not_initialized sign-in")
