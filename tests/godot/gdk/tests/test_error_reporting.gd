extends "res://addons/godot_gdk_tests/gdk_test_base.gd"


func before_each() -> void:
	reset_runtime()


func after_each() -> void:
	reset_runtime()


func test_error_reporting_surface() -> void:
	if pending_unless_runtime_available():
		return

	var gdk = get_gdk()
	var error_reporting = gdk.get_error_reporting()
	assert_not_null(error_reporting, "GDK.get_error_reporting() returns service")
	if error_reporting == null:
		return

	assert_has_method_named(error_reporting, "configure_options")
	assert_has_method_named(error_reporting, "set_callback_enabled")
	assert_has_method_named(error_reporting, "is_callback_enabled")
	assert_has_signal_named(error_reporting, "error_reported")
	assert_eq(error_reporting.is_callback_enabled(), false, "callback forwarding starts disabled")
	assert_eq(error_reporting.ERROR_OPTIONS_OUTPUT_DEBUG_STRING_ON_ERROR, 1, "output debug string option matches XErrorOptions")
	assert_eq(error_reporting.ERROR_OPTIONS_DEBUG_BREAK_ON_ERROR, 2, "debug break option matches XErrorOptions")
	assert_eq(error_reporting.ERROR_OPTIONS_FAIL_FAST_ON_ERROR, 4, "fail-fast option matches XErrorOptions")


func test_error_reporting_validation_is_deterministic() -> void:
	if pending_unless_runtime_available():
		return

	var gdk = get_gdk()
	var error_reporting = gdk.get_error_reporting()
	assert_not_null(error_reporting, "GDK.error_reporting is available for validation checks")
	if error_reporting == null:
		return

	var invalid_option_result = error_reporting.configure_options(999, error_reporting.ERROR_OPTIONS_NONE)
	assert_result_error(invalid_option_result, "invalid_error_reporting_options", "configure_options() rejects unsupported option flags")

	var invalid_second_option_result = error_reporting.configure_options(error_reporting.ERROR_OPTIONS_NONE, 999)
	assert_result_error(invalid_second_option_result, "invalid_error_reporting_options", "configure_options() validates debugger_not_present_options independently")

	var pre_init_callback_result = error_reporting.set_callback_enabled(true)
	assert_result_error(pre_init_callback_result, "runtime_not_initialized", "set_callback_enabled() requires initialized runtime")


func test_error_reporting_runtime_configuration() -> void:
	if pending_unless_runtime_available():
		return

	var init_result = initialize_runtime()
	assert_not_null(init_result, "initialize() returns GDKResult")
	if init_result == null:
		return
	if not init_result.ok:
		pending("GDK.initialize() failed: %s" % init_result.message)
		return

	var gdk = get_gdk()
	var error_reporting = gdk.get_error_reporting()
	assert_not_null(error_reporting, "GDK.error_reporting available after initialize()")
	if error_reporting == null:
		return

	var configure_result = error_reporting.configure_options(
			error_reporting.ERROR_OPTIONS_DEBUG_BREAK_ON_ERROR,
			error_reporting.ERROR_OPTIONS_FAIL_FAST_ON_ERROR)
	assert_result_ok(configure_result, "configure_options() accepts documented option enums")

	var combined_options: int = error_reporting.ERROR_OPTIONS_DEBUG_BREAK_ON_ERROR | error_reporting.ERROR_OPTIONS_FAIL_FAST_ON_ERROR
	var combined_result = error_reporting.configure_options(combined_options, error_reporting.ERROR_OPTIONS_NONE)
	assert_result_ok(combined_result, "configure_options() accepts combined enum bitmask flags")

	var enable_result = error_reporting.set_callback_enabled(true)
	assert_result_ok(enable_result, "set_callback_enabled(true)")
	assert_eq(error_reporting.is_callback_enabled(), true, "callback forwarding toggles on")

	var disable_result = error_reporting.set_callback_enabled(false)
	assert_result_ok(disable_result, "set_callback_enabled(false)")
	assert_eq(error_reporting.is_callback_enabled(), false, "callback forwarding toggles off")


func test_error_reporting_live_register_unregister_shutdown_callback_context() -> void:
	if pending_unless_runtime_available():
		return
	if pending_unless_live():
		return

	var init_result = initialize_runtime()
	assert_not_null(init_result, "GDK.initialize() returns GDKResult for live XError callback-lifetime test")
	if init_result == null:
		return
	if not init_result.ok:
		pending("Live XError callback-lifetime test: %s" % init_result.message)
		return

	var gdk = get_gdk()
	var error_reporting = gdk.get_error_reporting()
	for iteration in 8:
		var enable_result = error_reporting.set_callback_enabled(true)
		assert_result_ok(enable_result, "set_callback_enabled(true) registers XError callback context on iteration %d" % iteration)
		var disable_result = error_reporting.set_callback_enabled(false)
		assert_result_ok(disable_result, "set_callback_enabled(false) unregisters XError callback context on iteration %d" % iteration)

	var final_enable_result = error_reporting.set_callback_enabled(true)
	assert_result_ok(final_enable_result, "set_callback_enabled(true) before shutdown")
	gdk.shutdown()
	assert_eq(gdk.is_initialized(), false, "GDK.shutdown() clears XError callback context without crashing")
