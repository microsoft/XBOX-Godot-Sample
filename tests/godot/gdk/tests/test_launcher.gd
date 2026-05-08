extends "res://addons/godot_gdk_tests/gdk_test_base.gd"


func before_each() -> void:
	reset_runtime()


func after_each() -> void:
	reset_runtime()


func test_launcher_surface_and_validation_paths() -> void:
	if pending_unless_runtime_available():
		return

	var gdk = get_gdk()
	var launcher = gdk.get_launcher()
	assert_not_null(launcher, "GDK.launcher returns service object")
	if launcher == null:
		return

	for method_name in [
		"launch_uri",
	]:
		assert_has_method_named(launcher, method_name)

	var pre_init_result = launcher.launch_uri("ms-settings:")
	assert_result_error(pre_init_result, "not_initialized", "launch_uri() rejects calls before GDK.initialize()")

	var init_result = initialize_runtime()
	assert_not_null(init_result, "GDK.initialize() for launcher behavior returns GDKResult")
	if init_result == null:
		return
	if not init_result.ok:
		pending("Launcher runtime behavior: %s" % init_result.message)
		return

	assert_result_error(launcher.launch_uri(""), "invalid_uri", "launch_uri('') rejects blank URI")
	assert_result_error(launcher.launch_uri("missing-scheme"), "invalid_uri", "launch_uri() requires absolute URI scheme")
	assert_result_error(launcher.launch_uri("file:///tmp/test.txt"), "unsupported_launcher_destination", "launch_uri() rejects unsupported file destination")
	assert_result_error(launcher.launch_uri("javascript:alert('x')"), "unsupported_launcher_destination", "launch_uri() rejects disallowed javascript scheme")
	assert_result_error(launcher.launch_uri("ms-help:"), "unsupported_launcher_destination", "launch_uri() rejects unsupported ms-* destination")

	var blank_user = instantiate_class("GDKUser")
	assert_result_error(launcher.launch_uri("ms-settings:", blank_user), "invalid_user", "launch_uri() rejects unsigned user handles")
