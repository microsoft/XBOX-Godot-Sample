extends "res://addons/godot_gdk_tests/gameinput_test_base.gd"
## Wave 4 — `GameInputReading` defaults + property/method shape.
##
## A `GameInputReading` constructed via `ClassDB.instantiate()` (i.e. without
## ever being populated by `GameInput.get_current_reading()`) must:
##   * Report all buttons as not-down for every defined button enum value.
##   * Report `was_button_pressed()` / `was_button_released()` as false (the
##     reading has no "previous" state).
##   * Report `0.0` for every defined axis enum value.
##   * Report `0` for `get_buttons_mask()` and `get_timestamp()`.
##   * Expose the documented method shape (`is_button_down`, `was_button_pressed`,
##     `was_button_released`, `get_axis`, `get_buttons_mask`, `get_timestamp`).
##
## The native class derives only from `RefCounted` and exposes no signals; the
## existence of every documented method is asserted via `assert_has_method_named`.


const _BUTTON_NAMES := [
	"BUTTON_NONE",
	"BUTTON_MENU",
	"BUTTON_VIEW",
	"BUTTON_A",
	"BUTTON_B",
	"BUTTON_X",
	"BUTTON_Y",
	"BUTTON_DPAD_UP",
	"BUTTON_DPAD_DOWN",
	"BUTTON_DPAD_LEFT",
	"BUTTON_DPAD_RIGHT",
	"BUTTON_LEFT_SHOULDER",
	"BUTTON_RIGHT_SHOULDER",
	"BUTTON_LEFT_THUMB",
	"BUTTON_RIGHT_THUMB",
]

const _AXIS_NAMES := [
	"AXIS_LEFT_X",
	"AXIS_LEFT_Y",
	"AXIS_RIGHT_X",
	"AXIS_RIGHT_Y",
	"AXIS_LEFT_TRIGGER",
	"AXIS_RIGHT_TRIGGER",
]


func after_each() -> void:
	var gi = get_gameinput()
	if gi != null:
		gi.shutdown()


func _device_constant(name: String) -> int:
	return ClassDB.class_get_integer_constant("GameInput", name)


func _gameinput_device_constant(name: String) -> int:
	return ClassDB.class_get_integer_constant("GameInputDevice", name)


func _new_reading() -> Object:
	if not ClassDB.class_exists("GameInputReading"):
		return null
	return ClassDB.instantiate("GameInputReading")


func test_reading_class_registered() -> void:
	assert_true(ClassDB.class_exists("GameInputReading"),
			"GameInputReading registered in ClassDB")
	assert_true(ClassDB.is_parent_class("GameInputReading", "RefCounted"),
			"GameInputReading extends RefCounted")


func test_reading_method_shape() -> void:
	var reading = _new_reading()
	if reading == null:
		pending("GameInputReading missing")
		return

	assert_has_method_named(reading, "is_button_down")
	assert_has_method_named(reading, "was_button_pressed")
	assert_has_method_named(reading, "was_button_released")
	assert_has_method_named(reading, "get_axis")
	assert_has_method_named(reading, "get_buttons_mask")
	assert_has_method_named(reading, "get_timestamp")


func test_reading_default_buttons_not_down() -> void:
	var reading = _new_reading()
	if reading == null:
		pending("GameInputReading missing")
		return

	for button_name in _BUTTON_NAMES:
		var value: int = ClassDB.class_get_integer_constant("GameInputDevice", button_name)
		assert_eq(reading.is_button_down(value), false,
				"is_button_down(%s) == false on default reading" % button_name)
		assert_eq(reading.was_button_pressed(value), false,
				"was_button_pressed(%s) == false on default reading" % button_name)
		assert_eq(reading.was_button_released(value), false,
				"was_button_released(%s) == false on default reading" % button_name)


func test_reading_default_axes_zero() -> void:
	var reading = _new_reading()
	if reading == null:
		pending("GameInputReading missing")
		return

	for axis_name in _AXIS_NAMES:
		var value: int = ClassDB.class_get_integer_constant("GameInputDevice", axis_name)
		assert_eq_approx(reading.get_axis(value), 0.0,
				"get_axis(%s) ≈ 0.0 on default reading" % axis_name)


func test_reading_default_buttons_mask_and_timestamp() -> void:
	var reading = _new_reading()
	if reading == null:
		pending("GameInputReading missing")
		return

	assert_eq(reading.get_buttons_mask(), 0,
			"get_buttons_mask() == 0 on default reading")
	assert_eq(reading.get_timestamp(), 0,
			"get_timestamp() == 0 on default reading (timestamp not surfaced in v1)")


func test_reading_unknown_button_axis_safe() -> void:
	var reading = _new_reading()
	if reading == null:
		pending("GameInputReading missing")
		return

	# Negative + out-of-range values must not crash and must return safe defaults
	# (false for buttons, 0.0 for axes). This guards GDScript callers that pass
	# an unguarded enum value through.
	assert_eq(reading.is_button_down(-1), false,
			"is_button_down(-1) returns false")
	assert_eq(reading.is_button_down(99999), false,
			"is_button_down(99999) returns false")
	assert_eq_approx(reading.get_axis(-1), 0.0,
			"get_axis(-1) ≈ 0.0")
	assert_eq_approx(reading.get_axis(99999), 0.0,
			"get_axis(99999) ≈ 0.0")


func test_live_second_reading_exercises_previous_state_branch() -> void:
	if pending_unless_live():
		return
	if pending_unless_runtime_available():
		return

	var gi = get_gameinput()
	gi.shutdown()
	var started: bool = gi.initialize()
	if not started:
		pending("GameInput.initialize() returned false (no GameInput on host)")
		return

	gi.poll()
	await get_tree().process_frame
	gi.poll()
	var device = gi.get_primary_device(_device_constant("DEVICE_GAMEPAD"))
	if device == null:
		pending("No live GameInput gamepad connected for previous-state reading coverage")
		return

	var first_reading = gi.get_current_reading(device)
	if first_reading == null:
		pending("Live GameInput device did not produce an initial reading")
		return

	await get_tree().process_frame
	gi.poll()
	var second_reading = gi.get_current_reading(device)
	assert_not_null(second_reading, "second live reading is available")
	if second_reading == null:
		return

	var button_a := _gameinput_device_constant("BUTTON_A")
	assert_true(second_reading.was_button_pressed(button_a) is bool,
			"was_button_pressed() returns bool after a previous reading exists")
	assert_true(second_reading.was_button_released(button_a) is bool,
			"was_button_released() returns bool after a previous reading exists")
	assert_true(second_reading.get_buttons_mask() is int,
			"second reading exposes buttons mask while previous state is populated")
