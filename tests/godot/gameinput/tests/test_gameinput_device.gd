extends "res://addons/godot_gdk_tests/gameinput_test_base.gd"
## Wave 4 — `GameInputDevice` defaults + soft-fail on accessors.
##
## A `GameInputDevice` wrapper holds only a session-local id (a weak handle).
## Constructed bare via `ClassDB.instantiate()` it has id `0`; calling its
## accessors before `GameInput.initialize()` (or after `shutdown()`) must:
##   * never crash,
##   * return the documented sentinel values:
##       - `get_display_name()` → empty `String`
##       - `get_kind_mask()`    → `GameInput.DEVICE_UNKNOWN` (== 0)
##       - `is_connected()`     → `false`
##       - `supports_vibration()` / `supports_haptics()` → `false`
##       - `get_device_info()`   → empty `Dictionary`
##
## These match the soft-fail conventions documented in
## `.github/instructions/godot-gameinput.instructions.md` and the
## `device_*` shims in `gameinput_singleton.cpp`.


const _METHOD_NAMES := [
	"get_device_id",
	"get_display_name",
	"get_kind_mask",
	"is_connected",
	"supports_vibration",
	"supports_haptics",
	"get_device_info",
]


func _new_device() -> Object:
	if not ClassDB.class_exists("GameInputDevice"):
		return null
	return ClassDB.instantiate("GameInputDevice")


func test_device_class_registered() -> void:
	assert_true(ClassDB.class_exists("GameInputDevice"),
			"GameInputDevice registered in ClassDB")
	assert_true(ClassDB.is_parent_class("GameInputDevice", "RefCounted"),
			"GameInputDevice extends RefCounted")


func test_device_method_shape() -> void:
	var device = _new_device()
	if device == null:
		pending("GameInputDevice missing")
		return
	for method_name in _METHOD_NAMES:
		assert_has_method_named(device, method_name)


func test_device_defaults_before_init() -> void:
	# Make sure GameInput is not currently initialized so we exercise the
	# "before init" branch in the device shims explicitly.
	var gi = get_gameinput()
	if gi != null:
		gi.shutdown()

	var device = _new_device()
	if device == null:
		pending("GameInputDevice missing")
		return

	assert_eq(device.get_device_id(), 0, "default device id == 0")
	assert_eq(device.get_display_name(), "",
			"get_display_name() == \"\" before init")
	assert_eq(device.get_kind_mask(), 0,
			"get_kind_mask() == 0 (DEVICE_UNKNOWN) before init")
	assert_eq(device.call("is_connected"), false,
			"is_connected() == false before init")
	assert_eq(device.supports_vibration(), false,
			"supports_vibration() == false before init")
	assert_eq(device.supports_haptics(), false,
			"supports_haptics() == false before init")
	var info = device.get_device_info()
	assert_true(info is Dictionary, "get_device_info() returns Dictionary")
	assert_eq(info.size(), 0,
			"get_device_info() returns empty Dictionary before init")


func test_device_defaults_after_shutdown() -> void:
	if pending_unless_runtime_available():
		return

	var gi = get_gameinput()
	# Try to bring it up so we can prove the "after shutdown" branch is
	# distinct from the "never initialised" branch.
	gi.shutdown()
	var _started: bool = gi.initialize()
	gi.shutdown()
	assert_eq(gi.is_initialized(), false,
			"runtime not initialized after explicit shutdown()")

	var device = _new_device()
	if device == null:
		pending("GameInputDevice missing")
		return

	# All accessors must keep returning safe defaults after the runtime has
	# been torn down — even if the wrapper happened to hold a real id, the
	# device shims must soft-fail without dereferencing native pointers.
	assert_eq(device.get_display_name(), "",
			"get_display_name() == \"\" after shutdown()")
	assert_eq(device.get_kind_mask(), 0,
			"get_kind_mask() == 0 after shutdown()")
	assert_eq(device.call("is_connected"), false,
			"is_connected() == false after shutdown()")
	assert_eq(device.supports_vibration(), false,
			"supports_vibration() == false after shutdown()")
	assert_eq(device.supports_haptics(), false,
			"supports_haptics() == false after shutdown()")
	var info = device.get_device_info()
	assert_true(info is Dictionary and info.size() == 0,
			"get_device_info() returns empty Dictionary after shutdown()")


func test_device_static_button_axis_to_source() -> void:
	if not ClassDB.class_exists("GameInputDevice"):
		pending("GameInputDevice missing")
		return

	# The static helpers are part of the public surface; verify they round-trip
	# the documented enum constants without crashing on out-of-range input.
	var src_a: int = ClassDB.class_get_integer_constant("GameInputDevice", "SRC_BTN_A")
	var btn_a: int = ClassDB.class_get_integer_constant("GameInputDevice", "BUTTON_A")
	assert_eq(GameInputDevice.button_to_source(btn_a), src_a,
			"button_to_source(BUTTON_A) == SRC_BTN_A")

	var src_lx: int = ClassDB.class_get_integer_constant("GameInputDevice", "SRC_AXIS_LEFT_X")
	var axis_lx: int = ClassDB.class_get_integer_constant("GameInputDevice", "AXIS_LEFT_X")
	assert_eq(GameInputDevice.axis_to_source(axis_lx), src_lx,
			"axis_to_source(AXIS_LEFT_X) == SRC_AXIS_LEFT_X")

	# Out-of-range values must return the documented safe-default sources
	# (SRC_BTN_A and SRC_AXIS_LEFT_X) instead of crashing.
	assert_eq(GameInputDevice.button_to_source(99999), src_a,
			"button_to_source(out_of_range) returns SRC_BTN_A safe default")
	assert_eq(GameInputDevice.axis_to_source(99999), src_lx,
			"axis_to_source(out_of_range) returns SRC_AXIS_LEFT_X safe default")
