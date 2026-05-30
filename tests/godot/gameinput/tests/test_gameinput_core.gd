extends "res://addons/godot_gdk_tests/gameinput_test_base.gd"
## Wave 3 GUT-style port of `tests/suites/gameinput_core_suite.gd`.
##
## Verifies the GameInput singleton, class registration, soft-fail behavior,
## and initialize/shutdown idempotence. Tolerates the no-hardware case —
## initialize() may legitimately fail in CI environments without a usable
## GameInput device tree; we still assert that the API returns safe defaults
## and never crashes.


func test_singleton_registered() -> void:
	assert_true(Engine.has_singleton("GameInput"),
			"Engine.has_singleton('GameInput')")
	var gi = get_gameinput()
	assert_not_null(gi, "Engine.get_singleton('GameInput') returns object")


func test_class_registration() -> void:
	for cls in [
		"GameInput",
		"GameInputDevice",
		"GameInputReading",
		"GameInputBinding",
		"GameInputActionMap",
		"GameInputMapper",
	]:
		assert_true(ClassDB.class_exists(cls), "%s registered in ClassDB" % cls)

	assert_true(ClassDB.is_parent_class("GameInput", "Object"),
			"GameInput extends Object")
	assert_true(ClassDB.is_parent_class("GameInputDevice", "RefCounted"),
			"GameInputDevice extends RefCounted")
	assert_true(ClassDB.is_parent_class("GameInputReading", "RefCounted"),
			"GameInputReading extends RefCounted")
	assert_true(ClassDB.is_parent_class("GameInputBinding", "Resource"),
			"GameInputBinding extends Resource")
	assert_true(ClassDB.is_parent_class("GameInputActionMap", "Resource"),
			"GameInputActionMap extends Resource")
	assert_true(ClassDB.is_parent_class("GameInputMapper", "Node"),
			"GameInputMapper extends Node")


func test_soft_fail_before_init() -> void:
	if pending_unless_runtime_available():
		return

	var gi = get_gameinput()
	gi.shutdown()
	assert_eq(gi.is_initialized(), false, "is_initialized() == false before initialize()")

	var devices = gi.get_devices()
	assert_true(devices is Array, "get_devices() returns Array before init")
	assert_eq(devices.size(), 0, "get_devices() returns empty Array before init")

	var primary = gi.get_primary_device()
	assert_true(primary == null, "get_primary_device() returns null before init")

	var reading = gi.get_current_reading(null)
	assert_true(reading == null, "get_current_reading(null) returns null before init")

	assert_eq(gi.set_vibration(null, 1.0, 1.0), false,
			"set_vibration(null) returns false before init")

	gi.poll()
	assert_true(true, "poll() is safe before initialize()")


func test_initialize_shutdown_idempotence() -> void:
	if pending_unless_runtime_available():
		return

	var gi = get_gameinput()
	gi.shutdown()
	var first: bool = gi.initialize()
	if not first:
		pending("GameInput.initialize() returned false (no GameInput on host); safe-default API still verified above")
		gi.shutdown()
		return

	assert_eq(gi.is_initialized(), true, "is_initialized() true after initialize()")

	var second: bool = gi.initialize()
	assert_eq(second, true, "initialize() is idempotent (second call returns true)")
	assert_eq(gi.is_initialized(), true, "still initialized after second initialize()")

	gi.shutdown()
	assert_eq(gi.is_initialized(), false, "is_initialized() false after shutdown()")

	gi.shutdown()
	assert_eq(gi.is_initialized(), false, "shutdown() is idempotent")


func test_inert_after_shutdown() -> void:
	if pending_unless_runtime_available():
		return

	var gi = get_gameinput()
	gi.shutdown()

	var devices = gi.get_devices()
	assert_true(devices is Array and devices.size() == 0,
			"get_devices() returns empty Array after shutdown()")
	gi.poll()
	assert_true(true, "poll() is safe after shutdown()")
