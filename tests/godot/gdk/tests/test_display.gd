extends "res://addons/godot_gdk_tests/gdk_test_base.gd"
## Surface + validation coverage for `GDKDisplay` (`GDK.display`).
## Wraps `XDisplay.h`: HDR mode probe + display timeout deferrals.

func before_each() -> void:
	reset_runtime()


func after_each() -> void:
	reset_runtime()


func test_display_surface_and_validation_paths() -> void:
	if pending_unless_runtime_available():
		return

	var gdk = get_gdk()
	var display = gdk.get_display()
	assert_not_null(display, "GDK.display returns service object")
	if display == null:
		return

	for method_name in [
		"try_enable_hdr_mode",
		"acquire_timeout_deferral",
	]:
		assert_has_method_named(display, method_name)

	for constant_name in [
		"HDR_MODE_UNKNOWN",
		"HDR_MODE_ENABLED",
		"HDR_MODE_DISABLED",
		"HDR_MODE_PREFERENCE_PREFER_HDR",
		"HDR_MODE_PREFERENCE_PREFER_REFRESH_RATE",
	]:
		assert_true(
				ClassDB.class_has_integer_constant("GDKDisplay", constant_name),
				"GDKDisplay exposes %s" % constant_name)

	var pre_init_hdr = display.try_enable_hdr_mode()
	assert_result_error(pre_init_hdr, "not_initialized", "try_enable_hdr_mode() rejects calls before GDK.initialize()")

	var pre_init_deferral = display.acquire_timeout_deferral()
	assert_result_error(pre_init_deferral, "not_initialized", "acquire_timeout_deferral() rejects calls before GDK.initialize()")

	var init_result = initialize_runtime()
	assert_not_null(init_result, "GDK.initialize() for display behavior returns GDKResult")
	if init_result == null:
		return
	if not init_result.ok:
		pending("Display runtime behavior: %s" % init_result.message)
		return

	var invalid_pref = display.try_enable_hdr_mode(999)
	assert_result_error(invalid_pref, "invalid_preference", "try_enable_hdr_mode() rejects unknown preference values")

	var hdr_mode = display.try_enable_hdr_mode(get_class_constant("GDKDisplay", "HDR_MODE_PREFERENCE_PREFER_REFRESH_RATE"))
	assert_not_null(hdr_mode, "try_enable_hdr_mode() returns GDKResult after init")
	if hdr_mode != null and hdr_mode.ok:
		assert_true(hdr_mode.data is Dictionary, "try_enable_hdr_mode() success data is Dictionary")
		if hdr_mode.data is Dictionary:
			assert_true(hdr_mode.data.has("mode"), "try_enable_hdr_mode() data has mode")
			var mode_value: int = int(hdr_mode.data.get("mode", -1))
			var valid_modes := [
				get_class_constant("GDKDisplay", "HDR_MODE_UNKNOWN"),
				get_class_constant("GDKDisplay", "HDR_MODE_ENABLED"),
				get_class_constant("GDKDisplay", "HDR_MODE_DISABLED"),
			]
			assert_true(mode_value in valid_modes, "try_enable_hdr_mode() mode is one of HDR_MODE_*")
			if mode_value == get_class_constant("GDKDisplay", "HDR_MODE_ENABLED"):
				assert_true(hdr_mode.data.has("info"), "try_enable_hdr_mode() includes info dict when enabled")
				if hdr_mode.data.has("info"):
					var info: Dictionary = hdr_mode.data["info"]
					for info_key in ["min_tone_map_luminance", "max_tone_map_luminance", "max_full_frame_tone_map_luminance"]:
						assert_dict_has_key(info, info_key, "info has %s" % info_key)

	var deferral_result = display.acquire_timeout_deferral()
	assert_not_null(deferral_result, "acquire_timeout_deferral() returns GDKResult after init")
	if deferral_result != null and deferral_result.ok:
		var deferral = deferral_result.data
		assert_true(is_class_instance(deferral, "GDKDisplayTimeoutDeferral"), "acquire_timeout_deferral() data is GDKDisplayTimeoutDeferral")
		if is_class_instance(deferral, "GDKDisplayTimeoutDeferral"):
			assert_true(deferral.is_valid(), "GDKDisplayTimeoutDeferral is valid after acquire")
			deferral.release()
			assert_false(deferral.is_valid(), "GDKDisplayTimeoutDeferral is invalid after release()")
			deferral.release()
			assert_false(deferral.is_valid(), "release() is idempotent")
	elif deferral_result != null:
		assert_true(deferral_result.code.length() > 0, "failed acquire_timeout_deferral exposes an error code")
		assert_true(deferral_result.message.length() > 0, "failed acquire_timeout_deferral exposes an error message")
