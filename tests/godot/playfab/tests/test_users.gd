extends "res://addons/godot_gdk_tests/playfab_test_base.gd"
## Wave 3 migration of the previous `tests/suites/users_suite.gd`.
##
## Covers `PlayFab.users` API exposure, `PlayFabUser` wrapper defaults, and
## the not-initialized sign-in failure paths on both the root and users
## service surfaces.


func test_users_api() -> void:
	if pending_unless_playfab_available():
		return
	var playfab = get_playfab()

	var users = playfab.get_users()
	assert_object_is(users, "PlayFabUsers", "PlayFab.get_users() returns PlayFabUsers")
	if users == null:
		return

	for method_name in ["sign_in_with_xuser_async", "sign_in_with_custom_id_async", "get_user_by_local_id", "get_user_by_custom_id", "get_user", "get_users"]:
		assert_has_method_named(users, method_name)

	for signal_name in ["user_signed_in", "user_signed_out", "user_changed"]:
		assert_true(not users.has_signal(signal_name), "PlayFabUsers.%s is not exposed" % signal_name)

	var cached_users = users.get_users()
	assert_true(cached_users is Array, "PlayFabUsers.get_users() returns Array")
	assert_eq(cached_users.size(), 0, "PlayFabUsers cache starts empty")
	assert_eq(users.get_user_by_local_id(0), null, "get_user_by_local_id(0) returns null with no cached user")
	assert_eq(users.get_user_by_local_id(1), null, "get_user_by_local_id() returns null with no cached user")
	assert_eq(users.get_user_by_custom_id("missing"), null, "get_user_by_custom_id() returns null with no cached user")
	assert_eq(users.get_user(1), null, "get_user(int) returns null with no cached user")


func test_user_wrapper_defaults() -> void:
	var user = instantiate_class("PlayFabUser")
	assert_not_null(user, "PlayFabUser.new()")
	if user == null:
		return

	assert_eq(int(user.local_id), 0, "blank PlayFabUser.local_id defaults to 0")
	assert_eq(str(user.custom_id), "", "blank PlayFabUser.custom_id defaults empty")
	assert_false(user.has_local_user_handle(), "blank PlayFabUser has no local user handle")

	var entity_key: Dictionary = user.entity_key
	assert_eq(str(entity_key.get("id", "")), "", "blank PlayFabUser.entity_key.id defaults empty")
	assert_eq(str(entity_key.get("type", "")), "", "blank PlayFabUser.entity_key.type defaults empty")


func test_not_initialized_sign_in() -> void:
	if pending_unless_playfab_available():
		return
	var playfab = get_playfab()

	reset_playfab_runtime()

	var users_signal = playfab.get_users().sign_in_with_xuser_async(null)
	await _assert_playfab_signal_result_error(
		users_signal, "not_initialized", "PlayFab.users.sign_in_with_xuser_async() before initialize()")


func test_custom_id_sign_in_validation() -> void:
	if pending_unless_playfab_available():
		return
	var playfab = get_playfab()

	reset_playfab_runtime()

	var empty_users_signal = playfab.get_users().sign_in_with_custom_id_async("  ")
	await _assert_playfab_signal_result_error(
		empty_users_signal, "invalid_custom_id", "PlayFab.users.sign_in_with_custom_id_async() rejects blank custom_id")

	var users_signal = playfab.get_users().sign_in_with_custom_id_async("gdkfleet-test-custom-id")
	await _assert_playfab_signal_result_error(
		users_signal, "not_initialized", "PlayFab.users.sign_in_with_custom_id_async() before initialize()")


# Mirror of the previous `assert_signal_result_error` but routes through the
# PlayFab dual-pump `await_completion` and the PlayFabResult-labeled error
# assertion so failure messages point at the right type.
func _assert_playfab_signal_result_error(async_signal, expected_code: String, name: String) -> void:
	assert_eq(typeof(async_signal), TYPE_SIGNAL, "%s returns completion Signal" % name)
	if typeof(async_signal) != TYPE_SIGNAL:
		return
	assert_playfab_result_error(await await_completion(async_signal), expected_code, name)
