extends "res://addons/godot_gdk_tests/gdk_test_base.gd"


func before_each() -> void:
	reset_runtime()


func after_each() -> void:
	reset_runtime()


func test_privacy_surface_and_validation() -> void:
	if pending_unless_runtime_available():
		return

	var gdk = get_gdk()
	var privacy = gdk.get_privacy()
	assert_not_null(privacy, "GDK.privacy returns service object")
	if privacy == null:
		return

	for method_name in [
		"check_permission_async",
		"check_permission_for_anonymous_user_async",
		"batch_check_permission_async",
		"get_avoid_list_async",
		"get_mute_list_async",
	]:
		assert_has_method_named(privacy, method_name)

	var pre_init_signal = privacy.check_permission_async(null, "communicate_using_text", "1")
	await assert_signal_result_error(pre_init_signal, "runtime_unavailable", "check_permission_async() reports unavailable runtime before initialize")

	var init_result = initialize_runtime()
	assert_not_null(init_result, "GDK.initialize() for privacy validation returns GDKResult")
	if init_result == null:
		return
	if not init_result.ok:
		pending("Privacy runtime validation: %s" % init_result.message)
		return

	var invalid_user_signal = privacy.get_mute_list_async(null)
	await assert_signal_result_error(invalid_user_signal, "invalid_user", "get_mute_list_async() rejects null users after initialize")

	var sign_in = await ensure_primary_user()
	var sign_in_signal = sign_in["signal"]
	var sign_in_result = sign_in["result"]
	var user = sign_in["user"]
	if typeof(sign_in_signal) == TYPE_SIGNAL and sign_in_result == null:
		assert_true(false, "Default-user flow for privacy completes - timed out waiting for a signed-in user")
		return
	if user == null:
		if sign_in_result != null and not sign_in_result.ok:
			assert_true(sign_in_result.code.length() > 0, "failed privacy sign-in exposes an error code")
			assert_true(sign_in_result.message.length() > 0, "failed privacy sign-in exposes an error message")
			pending("Privacy signed-in validation: %s" % sign_in_result.message)
		else:
			pending("Privacy signed-in validation: No signed-in user is available on this machine.")
		return

	var invalid_permission_signal = privacy.check_permission_async(user, "not_a_permission", "1")
	await assert_signal_result_error(invalid_permission_signal, "invalid_permission", "check_permission_async() rejects unknown permissions")

	var invalid_xuid_signal = privacy.check_permission_async(user, "communicate_using_text", "not-a-number")
	await assert_signal_result_error(invalid_xuid_signal, "invalid_xuid", "check_permission_async() rejects non-numeric target XUIDs")

	var invalid_anonymous_signal = privacy.check_permission_for_anonymous_user_async(user, "communicate_using_text", "not_an_anonymous_type")
	await assert_signal_result_error(invalid_anonymous_signal, "invalid_anonymous_user_type", "check_permission_for_anonymous_user_async() rejects unknown anonymous user types")

	var empty_batch_signal = privacy.batch_check_permission_async(user, "communicate_using_text", PackedStringArray())
	await assert_signal_result_error(empty_batch_signal, "invalid_xuids", "batch_check_permission_async() rejects empty target XUID lists")
