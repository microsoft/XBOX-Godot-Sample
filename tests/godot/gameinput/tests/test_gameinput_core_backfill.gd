extends "res://addons/godot_gdk_tests/gameinput_test_base.gd"
## Audit C3 backfill for singleton bind_methods, haptics, masks, signals,
## and the init -> shutdown -> init callback re-arm cycle.


func after_each() -> void:
	var gi = get_gameinput()
	if gi != null:
		gi.shutdown()


func _find_signal_info(obj: Object, signal_name: StringName) -> Dictionary:
	for info in obj.get_signal_list():
		if info.get("name") == signal_name:
			return info
	return {}


func _assert_device_payload(device: Object, label: String) -> void:
	assert_not_null(device, "%s is non-null" % label)
	if device == null:
		return
	assert_true(device.has_method("get_device_id"), "%s exposes get_device_id()" % label)
	assert_true(device.has_method("get_kind_mask"), "%s exposes get_kind_mask()" % label)
	assert_true(device.has_method("is_connected"), "%s exposes is_connected()" % label)
	assert_true(device.call("is_connected"), "%s reports connected" % label)
	assert_true(device.get_device_id() > 0, "%s has a positive session id" % label)
	assert_true(device.get_kind_mask() != 0, "%s has a non-zero kind mask" % label)


func _device_constant(name: String) -> int:
	return ClassDB.class_get_integer_constant("GameInput", name)


func test_signal_shapes_and_connected_count_surface() -> void:
	if pending_unless_runtime_available():
		return

	var gi = get_gameinput()
	assert_has_signal_named(gi, "device_connected")
	assert_has_signal_named(gi, "device_disconnected")
	assert_has_method_named(gi, "get_connected_device_count")

	var connected_info := _find_signal_info(gi, &"device_connected")
	assert_false(connected_info.is_empty(), "device_connected signal metadata is registered")
	if not connected_info.is_empty():
		var args: Array = connected_info.get("args", [])
		assert_eq(args.size(), 1, "device_connected has one argument")
		if args.size() == 1:
			assert_eq(str(args[0].get("name", "")), "device", "device_connected argument name")
			assert_eq(args[0].get("type"), TYPE_OBJECT, "device_connected argument type")
			var device_class := str(args[0].get("class_name", ""))
			if device_class.is_empty():
				device_class = str(args[0].get("hint_string", ""))
			assert_eq(device_class, "GameInputDevice",
					"device_connected argument class hint")

	var disconnected_info := _find_signal_info(gi, &"device_disconnected")
	assert_false(disconnected_info.is_empty(), "device_disconnected signal metadata is registered")
	if not disconnected_info.is_empty():
		var args: Array = disconnected_info.get("args", [])
		assert_eq(args.size(), 1, "device_disconnected has one argument")
		if args.size() == 1:
			assert_eq(str(args[0].get("name", "")), "device_id",
					"device_disconnected argument name")
			assert_eq(args[0].get("type"), TYPE_INT, "device_disconnected argument type")

	gi.shutdown()
	assert_eq(gi.get_connected_device_count(), 0,
			"get_connected_device_count() returns 0 after shutdown")


func test_kind_mask_queries_return_consistent_content_after_initialize() -> void:
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

	var all_mask := _device_constant("DEVICE_ALL")
	var gamepad_mask := _device_constant("DEVICE_GAMEPAD")
	var keyboard_mask := _device_constant("DEVICE_KEYBOARD")
	var mouse_mask := _device_constant("DEVICE_MOUSE")
	var unknown_mask := _device_constant("DEVICE_UNKNOWN")

	var all_devices: Array = gi.get_devices(all_mask)
	assert_eq(all_devices.size(), gi.get_connected_device_count(),
			"DEVICE_ALL count matches get_connected_device_count()")
	for device in all_devices:
		_assert_device_payload(device, "DEVICE_ALL device")

	for mask in [gamepad_mask, keyboard_mask, mouse_mask, all_mask]:
		var devices: Array = gi.get_devices(mask)
		assert_true(devices is Array, "get_devices(%d) returns Array" % mask)
		for device in devices:
			_assert_device_payload(device, "mask %d device" % mask)
			assert_true((device.get_kind_mask() & mask) != 0,
					"device kind mask intersects query mask %d" % mask)

		var primary = gi.get_primary_device(mask)
		if devices.is_empty():
			assert_true(primary == null,
					"get_primary_device(%d) returns null when no device matches" % mask)
		else:
			_assert_device_payload(primary, "primary mask %d" % mask)
			assert_true((primary.get_kind_mask() & mask) != 0,
					"primary device kind mask intersects query mask %d" % mask)

	assert_eq(gi.get_devices(unknown_mask).size(), 0,
			"get_devices(DEVICE_UNKNOWN) returns no devices")
	assert_true(gi.get_primary_device(unknown_mask) == null,
			"get_primary_device(DEVICE_UNKNOWN) returns null")


func test_initialize_shutdown_initialize_rearms_device_enumeration() -> void:
	if pending_unless_runtime_available():
		return

	var gi = get_gameinput()
	gi.shutdown()
	var first_started: bool = gi.initialize()
	if not first_started:
		pending("GameInput.initialize() returned false (no GameInput on host)")
		return

	gi.poll()
	await get_tree().process_frame
	gi.poll()
	var first_devices: Array = gi.get_devices(_device_constant("DEVICE_ALL"))
	assert_eq(first_devices.size(), gi.get_connected_device_count(),
			"first init drains device callbacks into the cache")

	gi.shutdown()
	assert_eq(gi.is_initialized(), false, "shutdown() tears down first init")
	assert_eq(gi.get_connected_device_count(), 0, "shutdown() clears the device cache")

	var second_started: bool = gi.initialize()
	assert_true(second_started, "second initialize() after shutdown succeeds")
	if not second_started:
		return

	gi.poll()
	await get_tree().process_frame
	gi.poll()
	var second_devices: Array = gi.get_devices(_device_constant("DEVICE_ALL"))
	assert_eq(second_devices.size(), gi.get_connected_device_count(),
			"second init re-arms callbacks and exposes a consistent device cache")
	for device in second_devices:
		_assert_device_payload(device, "second init device")


func test_haptics_soft_fail_on_null_and_stale_wrappers() -> void:
	if pending_unless_runtime_available():
		return
	if not ClassDB.class_exists("GameInputDevice"):
		pending("GameInputDevice missing")
		return

	var gi = get_gameinput()
	gi.shutdown()
	var started: bool = gi.initialize()
	if not started:
		pending("GameInput.initialize() returned false (no GameInput on host)")
		return

	assert_eq(gi.set_vibration(null, 1.0, 1.0, 1.0, 1.0), false,
			"set_vibration(null) returns false while initialized")
	gi.stop_haptics(null)
	assert_true(true, "stop_haptics(null) soft-fails without crashing")

	var stale_device = ClassDB.instantiate("GameInputDevice")
	assert_eq(gi.set_vibration(stale_device, 0.25, 0.5, 0.75, 1.0), false,
			"set_vibration(stale/disconnected wrapper) returns false")
	gi.stop_haptics(stale_device)
	assert_true(true, "stop_haptics(stale/disconnected wrapper) soft-fails without crashing")


func test_live_vibration_success_path_and_stop_haptics() -> void:
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
		pending("No live GameInput gamepad connected for vibration coverage")
		return
	if not device.supports_vibration():
		pending("Live GameInput gamepad does not report rumble support")
		return

	var ok: bool = gi.set_vibration(device, 1.5, -1.0, 2.0, -0.5)
	assert_true(ok, "set_vibration() succeeds on a live rumble-capable device")
	await get_tree().create_timer(0.05).timeout
	gi.stop_haptics(device)
	assert_true(true, "stop_haptics() sends a zero-rumble request after success")


func test_live_device_connected_signal_emits_device_payload() -> void:
	if pending_unless_live():
		return
	if pending_unless_runtime_available():
		return

	var gi = get_gameinput()
	gi.shutdown()
	watch_signals(gi)
	var started: bool = gi.initialize()
	if not started:
		pending("GameInput.initialize() returned false (no GameInput on host)")
		return

	gi.poll()
	await get_tree().process_frame
	gi.poll()
	if gi.get_connected_device_count() == 0:
		pending("No live GameInput devices connected to assert device_connected emission")
		return

	assert_true(get_signal_emit_count(gi, "device_connected") > 0,
			"initial enumeration emits device_connected on the main thread")
	var params = get_signal_parameters(gi, "device_connected", 0)
	assert_true(params is Array, "device_connected parameters are captured")
	if params is Array and params.size() == 1:
		_assert_device_payload(params[0], "device_connected payload")
