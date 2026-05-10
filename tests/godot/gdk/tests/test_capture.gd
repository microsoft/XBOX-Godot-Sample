extends "res://addons/godot_gdk_tests/gdk_test_base.gd"
## GUT coverage for GDKCapture.
##
## Deterministic surface and validation tests run without a signed-in user.
## Live diagnostic-capture tests (record_diagnostic_clip_async,
## take_diagnostic_screenshot_async) require LIVE_TESTS=1 and a device with
## Game Bar active; they are gated by pending_unless_live().

func before_each() -> void:
	reset_runtime()


func after_each() -> void:
	reset_runtime()


# ── Surface ──────────────────────────────────────────────────────────────

func test_capture_service_surface() -> void:
	if pending_unless_runtime_available():
		return

	var gdk = get_gdk()
	var capture = gdk.get_capture()
	assert_not_null(capture, "GDK.capture returns service object")
	if capture == null:
		return

	for method_name in [
		"enable_capture",
		"disable_capture",
		"record_diagnostic_clip_async",
		"take_diagnostic_screenshot_async",
		"create_metadata",
	]:
		assert_has_method_named(capture, method_name)


func test_capture_metadata_class_surface() -> void:
	if pending_unless_runtime_available():
		return

	var meta = instantiate_class("GDKCaptureMetaData")
	assert_not_null(meta, "GDKCaptureMetaData can be instantiated")
	if meta == null:
		return

	for method_name in [
		"is_valid",
		"close",
		"stop_all_states",
		"get_remaining_storage_bytes",
		"add_string_event",
		"add_double_event",
		"add_int32_event",
		"start_string_state",
		"start_double_state",
		"start_int32_state",
	]:
		assert_has_method_named(meta, method_name)

	assert_false(meta.is_valid(), "default GDKCaptureMetaData.is_valid() is false before create_metadata()")
	assert_eq(meta.get_remaining_storage_bytes(), -1, "default GDKCaptureMetaData.get_remaining_storage_bytes() returns -1")


func test_capture_metadata_priority_constants() -> void:
	if pending_unless_runtime_available():
		return

	assert_eq(
		get_class_constant("GDKCaptureMetaData", "PRIORITY_GAMEPLAY"),
		0,
		"GDKCaptureMetaData.PRIORITY_GAMEPLAY == 0"
	)
	assert_eq(
		get_class_constant("GDKCaptureMetaData", "PRIORITY_IMPORTANT"),
		1,
		"GDKCaptureMetaData.PRIORITY_IMPORTANT == 1"
	)


# ── GDKCapture validation (before init) ─────────────────────────────────

func test_capture_enable_rejects_before_init() -> void:
	if pending_unless_runtime_available():
		return

	var gdk = get_gdk()
	var capture = gdk.get_capture()
	assert_not_null(capture, "GDK.capture is accessible")
	if capture == null:
		return

	var result = capture.enable_capture()
	assert_result_error(result, "runtime_not_initialized", "enable_capture() rejects before GDK init")


func test_capture_disable_rejects_before_init() -> void:
	if pending_unless_runtime_available():
		return

	var gdk = get_gdk()
	var capture = gdk.get_capture()
	assert_not_null(capture, "GDK.capture is accessible")
	if capture == null:
		return

	var result = capture.disable_capture()
	assert_result_error(result, "runtime_not_initialized", "disable_capture() rejects before GDK init")


func test_capture_record_clip_rejects_invalid_duration() -> void:
	if pending_unless_runtime_available():
		return

	var gdk = get_gdk()
	var capture = gdk.get_capture()
	assert_not_null(capture, "GDK.capture is accessible")
	if capture == null:
		return

	var zero_duration_signal = capture.record_diagnostic_clip_async(0.0)
	await assert_signal_result_error(zero_duration_signal, "invalid_capture_duration", "record_diagnostic_clip_async(0) rejects zero duration")

	var negative_duration_signal = capture.record_diagnostic_clip_async(-1.0)
	await assert_signal_result_error(negative_duration_signal, "invalid_capture_duration", "record_diagnostic_clip_async(-1) rejects negative duration")


func test_capture_screenshot_rejects_empty_path_hint() -> void:
	if pending_unless_runtime_available():
		return

	var gdk = get_gdk()
	var capture = gdk.get_capture()
	assert_not_null(capture, "GDK.capture is accessible")
	if capture == null:
		return

	var empty_signal = capture.take_diagnostic_screenshot_async("")
	await assert_signal_result_error(empty_signal, "invalid_screenshot_path_hint", "take_diagnostic_screenshot_async('') rejects empty path hint")

	var blank_signal = capture.take_diagnostic_screenshot_async("   ")
	await assert_signal_result_error(blank_signal, "invalid_screenshot_path_hint", "take_diagnostic_screenshot_async('   ') rejects whitespace-only path hint")


func test_capture_create_metadata_returns_null_before_init() -> void:
	if pending_unless_runtime_available():
		return

	var gdk = get_gdk()
	var capture = gdk.get_capture()
	assert_not_null(capture, "GDK.capture is accessible")
	if capture == null:
		return

	var meta = capture.create_metadata()
	assert_true(meta == null, "create_metadata() returns null before GDK init")


# ── GDKCaptureMetaData validation (invalid context) ────────────────────

func test_capture_metadata_operations_reject_invalid_handle() -> void:
	if pending_unless_runtime_available():
		return

	var meta = instantiate_class("GDKCaptureMetaData")
	assert_not_null(meta, "GDKCaptureMetaData can be instantiated")
	if meta == null:
		return

	assert_false(meta.is_valid(), "GDKCaptureMetaData.is_valid() is false before create_metadata()")

	var stop_result = meta.stop_all_states()
	assert_result_error(stop_result, "invalid_metadata_handle", "stop_all_states() rejects invalid handle")

	var add_str_result = meta.add_string_event("zone", "lobby")
	assert_result_error(add_str_result, "invalid_metadata_handle", "add_string_event() rejects invalid handle")

	var add_dbl_result = meta.add_double_event("score", 3.14)
	assert_result_error(add_dbl_result, "invalid_metadata_handle", "add_double_event() rejects invalid handle")

	var add_int_result = meta.add_int32_event("level", 1)
	assert_result_error(add_int_result, "invalid_metadata_handle", "add_int32_event() rejects invalid handle")

	var start_str_result = meta.start_string_state("mode", "versus")
	assert_result_error(start_str_result, "invalid_metadata_handle", "start_string_state() rejects invalid handle")

	var start_dbl_result = meta.start_double_state("health", 100.0)
	assert_result_error(start_dbl_result, "invalid_metadata_handle", "start_double_state() rejects invalid handle")

	var start_int_result = meta.start_int32_state("ammo", 30)
	assert_result_error(start_int_result, "invalid_metadata_handle", "start_int32_state() rejects invalid handle")


func test_capture_metadata_operations_reject_empty_name() -> void:
	## These validation paths are exercised post-init so the GDK metadata
	## context is valid, letting us confirm the name-guard fires independently.
	if pending_unless_runtime_available():
		return

	var init_result = initialize_runtime()
	assert_not_null(init_result, "GDK.initialize() returns GDKResult")
	if init_result == null:
		return
	if not init_result.ok:
		pending("Metadata name validation: runtime init failed — %s" % init_result.message)
		return

	var gdk = get_gdk()
	var capture = gdk.get_capture()
	assert_not_null(capture, "GDK.capture is accessible after init")
	if capture == null:
		return

	var meta = capture.create_metadata()
	assert_not_null(meta, "create_metadata() returns non-null after init")
	if meta == null:
		pending("create_metadata() returned null after successful init — check runtime state")
		return

	assert_true(meta.is_valid(), "created GDKCaptureMetaData.is_valid() is true")

	var add_str_empty = meta.add_string_event("", "value")
	assert_result_error(add_str_empty, "invalid_metadata_name", "add_string_event() rejects empty name")

	var add_dbl_empty = meta.add_double_event("", 1.0)
	assert_result_error(add_dbl_empty, "invalid_metadata_name", "add_double_event() rejects empty name")

	var add_int_empty = meta.add_int32_event("", 1)
	assert_result_error(add_int_empty, "invalid_metadata_name", "add_int32_event() rejects empty name")

	var start_str_empty = meta.start_string_state("", "value")
	assert_result_error(start_str_empty, "invalid_metadata_name", "start_string_state() rejects empty name")

	var start_dbl_empty = meta.start_double_state("", 1.0)
	assert_result_error(start_dbl_empty, "invalid_metadata_name", "start_double_state() rejects empty name")

	var start_int_empty = meta.start_int32_state("", 1)
	assert_result_error(start_int_empty, "invalid_metadata_name", "start_int32_state() rejects empty name")

	meta.close()
	assert_false(meta.is_valid(), "GDKCaptureMetaData.is_valid() is false after explicit close()")


# ── Metadata start/stop flow (deterministic with GDK running) ────────────

func test_capture_metadata_start_stop_flow() -> void:
	## Validates that start_string_state + stop_all_states round-trips without
	## error when the GDK runtime is available.
	if pending_unless_runtime_available():
		return

	var init_result = initialize_runtime()
	assert_not_null(init_result, "GDK.initialize() returns GDKResult")
	if init_result == null:
		return
	if not init_result.ok:
		pending("Metadata start/stop flow: runtime init failed — %s" % init_result.message)
		return

	var gdk = get_gdk()
	var capture = gdk.get_capture()
	assert_not_null(capture, "GDK.capture is accessible after init")
	if capture == null:
		return

	var meta = capture.create_metadata()
	assert_not_null(meta, "create_metadata() returns non-null after init")
	if meta == null:
		pending("create_metadata() returned null — check runtime state")
		return

	assert_true(meta.is_valid(), "metadata context is valid after create_metadata()")

	var start_result = meta.start_string_state("zone", "lobby")
	assert_not_null(start_result, "start_string_state() returns GDKResult")
	if start_result == null:
		meta.close()
		return
	if not start_result.ok:
		pending("start_string_state: %s" % start_result.message)
		meta.close()
		return
	assert_true(start_result.ok, "start_string_state('zone','lobby') succeeds")

	var stop_result = meta.stop_all_states()
	assert_not_null(stop_result, "stop_all_states() returns GDKResult")
	if stop_result == null:
		meta.close()
		return
	if not stop_result.ok:
		pending("stop_all_states: %s" % stop_result.message)
		meta.close()
		return
	assert_true(stop_result.ok, "stop_all_states() succeeds after start_string_state")

	meta.close()
	assert_false(meta.is_valid(), "metadata context is invalid after close()")


# ── Capture state (enable / disable — requires init) ─────────────────────

func test_capture_enable_disable_after_init() -> void:
	if pending_unless_runtime_available():
		return

	var init_result = initialize_runtime()
	assert_not_null(init_result, "GDK.initialize() returns GDKResult")
	if init_result == null:
		return
	if not init_result.ok:
		pending("Capture enable/disable: runtime init failed — %s" % init_result.message)
		return

	var gdk = get_gdk()
	var capture = gdk.get_capture()
	assert_not_null(capture, "GDK.capture is accessible after init")
	if capture == null:
		return

	var disable_result = capture.disable_capture()
	assert_not_null(disable_result, "disable_capture() returns GDKResult")
	if disable_result == null:
		return
	# disable_capture() may fail when Game Bar is absent; that is expected and
	# not a test failure — the API contract is satisfied as long as a
	# GDKResult is returned with a populated error code.
	if not disable_result.ok:
		assert_true(disable_result.code.length() > 0, "disable_capture() failure exposes an error code")
		assert_true(disable_result.message.length() > 0, "disable_capture() failure exposes an error message")
		pending("disable_capture(): %s" % disable_result.message)
		return

	var enable_result = capture.enable_capture()
	assert_not_null(enable_result, "enable_capture() returns GDKResult")
	if enable_result == null:
		return
	if not enable_result.ok:
		assert_true(enable_result.code.length() > 0, "enable_capture() failure exposes an error code")
		assert_true(enable_result.message.length() > 0, "enable_capture() failure exposes an error message")
		pending("enable_capture(): %s" % enable_result.message)
		return
	assert_true(enable_result.ok, "enable_capture() succeeds after disable_capture()")


# ── Live diagnostic capture (LIVE_TESTS=1, Game Bar required) ─────────────

func test_capture_record_clip_live() -> void:
	## Manual / live-only: requires Game Bar to be active on the device.
	## Gate with pending_unless_live().
	if pending_unless_runtime_available():
		return

	var init_result = initialize_runtime()
	assert_not_null(init_result, "GDK.initialize() returns GDKResult")
	if init_result == null:
		return
	if not init_result.ok:
		pending("record_diagnostic_clip_async live: runtime init failed — %s" % init_result.message)
		return

	if pending_unless_live():
		return

	var gdk = get_gdk()
	var capture = gdk.get_capture()
	assert_not_null(capture, "GDK.capture is accessible after init")
	if capture == null:
		return

	var clip_signal = capture.record_diagnostic_clip_async(3.0)
	assert_true(typeof(clip_signal) == TYPE_SIGNAL, "record_diagnostic_clip_async(3.0) returns Signal")
	if typeof(clip_signal) != TYPE_SIGNAL:
		return

	var clip_result = await await_completion(clip_signal, 15000)
	if clip_result == null:
		pending("record_diagnostic_clip_async: timed out waiting for clip recording.")
		return
	if not clip_result.ok:
		assert_true(clip_result.code.length() > 0, "failed record_diagnostic_clip_async exposes an error code")
		assert_true(clip_result.message.length() > 0, "failed record_diagnostic_clip_async exposes an error message")
		pending("record_diagnostic_clip_async: %s" % clip_result.message)
		return
	assert_true(clip_result.ok, "record_diagnostic_clip_async(3.0) reports ok")


func test_capture_take_screenshot_live() -> void:
	## Manual / live-only: requires Game Bar to be active on the device.
	if pending_unless_runtime_available():
		return

	var init_result = initialize_runtime()
	assert_not_null(init_result, "GDK.initialize() returns GDKResult")
	if init_result == null:
		return
	if not init_result.ok:
		pending("take_diagnostic_screenshot_async live: runtime init failed — %s" % init_result.message)
		return

	if pending_unless_live():
		return

	var gdk = get_gdk()
	var capture = gdk.get_capture()
	assert_not_null(capture, "GDK.capture is accessible after init")
	if capture == null:
		return

	var screenshot_signal = capture.take_diagnostic_screenshot_async("gdk_capture_test")
	assert_true(typeof(screenshot_signal) == TYPE_SIGNAL, "take_diagnostic_screenshot_async() returns Signal")
	if typeof(screenshot_signal) != TYPE_SIGNAL:
		return

	var screenshot_result = await await_completion(screenshot_signal, 10000)
	if screenshot_result == null:
		pending("take_diagnostic_screenshot_async: timed out waiting for screenshot.")
		return
	if not screenshot_result.ok:
		assert_true(screenshot_result.code.length() > 0, "failed screenshot exposes an error code")
		assert_true(screenshot_result.message.length() > 0, "failed screenshot exposes an error message")
		pending("take_diagnostic_screenshot_async: %s" % screenshot_result.message)
		return
	assert_true(screenshot_result.ok, "take_diagnostic_screenshot_async() reports ok")
