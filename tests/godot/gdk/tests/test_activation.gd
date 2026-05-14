extends "res://addons/godot_gdk_tests/gdk_test_base.gd"
## Surface + validation coverage for `GDKActivation` (`GDK.activation`).
## Wraps `XGameActivation.h` (modern replacement for the deprecated
## `XGameProtocol.h` registration).

func before_each() -> void:
	reset_runtime()


func after_each() -> void:
	reset_runtime()


func test_activation_surface_and_validation_paths() -> void:
	if pending_unless_runtime_available():
		return

	var gdk = get_gdk()
	var activation = gdk.get_activation()
	assert_not_null(activation, "GDK.activation returns service object")
	if activation == null:
		return

	for method_name in ["accept_pending_invite"]:
		assert_has_method_named(activation, method_name)

	for signal_name in [
		"protocol_activated",
		"file_activated",
		"pending_invite_received",
		"invite_accepted",
		"activated",
	]:
		assert_has_signal_named(activation, signal_name)

	for constant_name in [
		"ACTIVATION_TYPE_PROTOCOL",
		"ACTIVATION_TYPE_FILE",
		"ACTIVATION_TYPE_PENDING_GAME_INVITE",
		"ACTIVATION_TYPE_ACCEPTED_GAME_INVITE",
	]:
		assert_true(
				ClassDB.class_has_integer_constant("GDKActivation", constant_name),
				"GDKActivation exposes %s" % constant_name)

	var pre_init_accept = activation.accept_pending_invite("ms-xbl-multiplayer:?inviteHandleId=123")
	assert_result_error(pre_init_accept, "not_initialized", "accept_pending_invite() rejects calls before GDK.initialize()")

	var init_result = initialize_runtime()
	assert_not_null(init_result, "GDK.initialize() for activation behavior returns GDKResult")
	if init_result == null:
		return
	if not init_result.ok:
		pending("Activation runtime behavior: %s" % init_result.message)
		return

	assert_result_error(activation.accept_pending_invite(""), "invalid_invite_uri", "accept_pending_invite('') rejects empty URI")
	assert_result_error(activation.accept_pending_invite("   "), "invalid_invite_uri", "accept_pending_invite() rejects whitespace-only URI")
