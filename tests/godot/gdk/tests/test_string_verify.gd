extends "res://addons/godot_gdk_tests/gdk_test_base.gd"


func before_each() -> void:
	reset_runtime()


func after_each() -> void:
	reset_runtime()


func test_string_verify_surface_and_validation() -> void:
	if pending_unless_runtime_available():
		return

	var gdk = get_gdk()
	var string_verify = gdk.get_string_verify()
	assert_not_null(string_verify, "GDK.string_verify returns service object")
	if string_verify == null:
		return

	for method_name in [
		"verify_string_async",
		"verify_strings_async",
	]:
		assert_has_method_named(string_verify, method_name)

	var pre_init_signal = string_verify.verify_string_async(null, "hello")
	await assert_signal_result_error(pre_init_signal, "runtime_unavailable", "verify_string_async() reports unavailable runtime before initialize")

	var init_result = initialize_runtime()
	assert_not_null(init_result, "GDK.initialize() for string verification returns GDKResult")
	if init_result == null:
		return
	if not init_result.ok:
		pending("String verification runtime validation: %s" % init_result.message)
		return

	var invalid_user_signal = string_verify.verify_string_async(null, "hello")
	await assert_signal_result_error(invalid_user_signal, "invalid_user", "verify_string_async() rejects null users after initialize")

	var sign_in = await ensure_primary_user()
	var sign_in_signal = sign_in["signal"]
	var sign_in_result = sign_in["result"]
	var user = sign_in["user"]
	if typeof(sign_in_signal) == TYPE_SIGNAL and sign_in_result == null:
		assert_true(false, "Default-user flow for string verification completes - timed out waiting for a signed-in user")
		return
	if user == null:
		if sign_in_result != null and not sign_in_result.ok:
			assert_true(sign_in_result.code.length() > 0, "failed string verification sign-in exposes an error code")
			assert_true(sign_in_result.message.length() > 0, "failed string verification sign-in exposes an error message")
			pending("String verification signed-in validation: %s" % sign_in_result.message)
		else:
			pending("String verification signed-in validation: No signed-in user is available on this machine.")
		return

	var empty_batch_signal = string_verify.verify_strings_async(user, PackedStringArray())
	await assert_signal_result_error(empty_batch_signal, "invalid_strings", "verify_strings_async() rejects empty string lists")
