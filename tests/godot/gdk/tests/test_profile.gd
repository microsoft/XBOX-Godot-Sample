extends "res://addons/godot_gdk_tests/gdk_test_base.gd"


func before_each() -> void:
	reset_runtime()


func after_each() -> void:
	reset_runtime()


func test_profile_surface_and_validation() -> void:
	if pending_unless_runtime_available():
		return

	var gdk = get_gdk()
	var profile = gdk.get_profile()
	assert_not_null(profile, "GDK.profile returns service object")
	if profile == null:
		return

	for method_name in [
		"get_profile_async",
		"get_profiles_async",
		"get_profiles_for_social_group_async",
	]:
		assert_has_method_named(profile, method_name)

	var user_profile = GDKUserProfile.new()
	assert_not_null(user_profile, "GDKUserProfile can be instantiated")
	if user_profile != null:
		assert_eq(user_profile.xuid, "", "empty user profile starts with no XUID")
		assert_true(user_profile.has_method("get_unique_modern_gamertag"), "GDKUserProfile exposes modern gamertag data")

	var pre_init_signal = profile.get_profile_async(null, "1")
	await assert_signal_result_error(pre_init_signal, "runtime_unavailable", "get_profile_async() reports unavailable runtime before initialize")

	var init_result = initialize_runtime()
	assert_not_null(init_result, "GDK.initialize() for profile validation returns GDKResult")
	if init_result == null:
		return
	if not init_result.ok:
		pending("Profile runtime validation: %s" % init_result.message)
		return

	var invalid_user_signal = profile.get_profile_async(null, "1")
	await assert_signal_result_error(invalid_user_signal, "invalid_user", "get_profile_async() rejects null users after initialize")

	var sign_in = await ensure_primary_user()
	var sign_in_signal = sign_in["signal"]
	var sign_in_result = sign_in["result"]
	var user = sign_in["user"]
	if typeof(sign_in_signal) == TYPE_SIGNAL and sign_in_result == null:
		assert_true(false, "Default-user flow for profile completes - timed out waiting for a signed-in user")
		return
	if user == null:
		if sign_in_result != null and not sign_in_result.ok:
			assert_true(sign_in_result.code.length() > 0, "failed profile sign-in exposes an error code")
			assert_true(sign_in_result.message.length() > 0, "failed profile sign-in exposes an error message")
			pending("Profile signed-in validation: %s" % sign_in_result.message)
		else:
			pending("Profile signed-in validation: No signed-in user is available on this machine.")
		return

	var invalid_xuid_signal = profile.get_profile_async(user, "not-a-number")
	await assert_signal_result_error(invalid_xuid_signal, "invalid_xuid", "get_profile_async() rejects non-numeric XUIDs")

	var empty_batch_signal = profile.get_profiles_async(user, PackedStringArray())
	await assert_signal_result_error(empty_batch_signal, "invalid_xuids", "get_profiles_async() rejects empty XUID lists")

	var invalid_batch_signal = profile.get_profiles_async(user, PackedStringArray(["1", "not-a-number"]))
	await assert_signal_result_error(invalid_batch_signal, "invalid_xuid", "get_profiles_async() rejects non-numeric XUID entries")

	var empty_group_signal = profile.get_profiles_for_social_group_async(user, "")
	await assert_signal_result_error(empty_group_signal, "invalid_social_group", "get_profiles_for_social_group_async() rejects empty groups")
