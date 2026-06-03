extends RefCounted
## Verifies the GameInput singleton, class registration, soft-fail behavior, and
## initialize/shutdown idempotence. Tolerates the no-hardware case — initialize()
## may legitimately fail in CI environments without a usable GameInput device tree;
## we still assert that the API returns safe defaults and never crashes.


func run(context) -> void:
	_test_singleton_registered(context)
	_test_class_registration(context)
	_test_soft_fail_before_init(context)
	_test_initialize_shutdown_idempotence(context)
	_test_inert_after_shutdown(context)


func _test_singleton_registered(context) -> void:
	context.log_section("Singleton & Class Registration")

	context.assert_true(Engine.has_singleton("GameInput"),
			"Engine.has_singleton('GameInput')")
	var gi = context.get_gameinput()
	context.assert_not_null(gi, "Engine.get_singleton('GameInput') returns object")


func _test_class_registration(context) -> void:
	for cls in [
		"GameInput",
		"GameInputDevice",
		"GameInputReading",
		"GameInputBinding",
		"GameInputActionMap",
		"GameInputMapper",
	]:
		context.assert_true(ClassDB.class_exists(cls), "%s registered in ClassDB" % cls)

	context.assert_true(ClassDB.is_parent_class("GameInput", "Object"),
			"GameInput extends Object")
	context.assert_true(ClassDB.is_parent_class("GameInputDevice", "RefCounted"),
			"GameInputDevice extends RefCounted")
	context.assert_true(ClassDB.is_parent_class("GameInputReading", "RefCounted"),
			"GameInputReading extends RefCounted")
	context.assert_true(ClassDB.is_parent_class("GameInputBinding", "Resource"),
			"GameInputBinding extends Resource")
	context.assert_true(ClassDB.is_parent_class("GameInputActionMap", "Resource"),
			"GameInputActionMap extends Resource")
	context.assert_true(ClassDB.is_parent_class("GameInputMapper", "Node"),
			"GameInputMapper extends Node")


func _test_soft_fail_before_init(context) -> void:
	context.log_section("Soft-Fail Before initialize()")

	var gi = context.get_gameinput()
	if gi == null:
		context.log_skip("Soft-fail tests skipped — GameInput singleton missing")
		return

	gi.shutdown()
	context.assert_eq(gi.is_initialized(), false, "is_initialized() == false before initialize()")

	var devices = gi.get_devices()
	context.assert_true(devices is Array, "get_devices() returns Array before init")
	context.assert_eq(devices.size(), 0, "get_devices() returns empty Array before init")

	var primary = gi.get_primary_device()
	context.assert_true(primary == null, "get_primary_device() returns null before init")

	var reading = gi.get_current_reading(null)
	context.assert_true(reading == null, "get_current_reading(null) returns null before init")

	gi.poll()
	context.log_pass("poll() is safe before initialize()")


func _test_initialize_shutdown_idempotence(context) -> void:
	context.log_section("initialize() / shutdown() Idempotence")

	var gi = context.get_gameinput()
	if gi == null:
		return

	gi.shutdown()
	var first: bool = gi.initialize()
	if not first:
		context.log_skip("GameInput.initialize() returned false (no GameInput on host)",
				"safe-default API still verified above")
		gi.shutdown()
		return

	context.assert_eq(gi.is_initialized(), true, "is_initialized() true after initialize()")

	var second: bool = gi.initialize()
	context.assert_eq(second, true, "initialize() is idempotent (second call returns true)")
	context.assert_eq(gi.is_initialized(), true, "still initialized after second initialize()")

	gi.shutdown()
	context.assert_eq(gi.is_initialized(), false, "is_initialized() false after shutdown()")

	gi.shutdown()
	context.assert_eq(gi.is_initialized(), false, "shutdown() is idempotent")


func _test_inert_after_shutdown(context) -> void:
	var gi = context.get_gameinput()
	if gi == null:
		return

	gi.shutdown()

	var devices = gi.get_devices()
	context.assert_true(devices is Array and devices.size() == 0,
			"get_devices() returns empty Array after shutdown()")
	gi.poll()
	context.log_pass("poll() is safe after shutdown()")
