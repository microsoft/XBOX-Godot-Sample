extends "res://addons/godot_gdk_tests/gameinput_test_base.gd"
## Regression coverage for mapper-held action release paths.
##
## These tests use internal mapper hooks to seed the same per-binding cache that a
## real GameInput press would populate, keeping the stuck-action sweep runnable on
## CI hosts without controller hardware.

const IMPOSSIBLE_DEVICE_ID := 9223372036854775807

var _actions_to_cleanup: Array[StringName] = []


func after_each() -> void:
	for action in _actions_to_cleanup:
		Input.action_release(action)
		if InputMap.has_action(action):
			InputMap.erase_action(action)
	_actions_to_cleanup.clear()


func _ensure_action(action: StringName) -> void:
	if not InputMap.has_action(action):
		InputMap.add_action(action)
		_actions_to_cleanup.append(action)
	Input.action_release(action)


func _new_binding(action: StringName, source_value: int = 2):
	if not ClassDB.class_exists("GameInputBinding"):
		return null
	var binding = ClassDB.instantiate("GameInputBinding")
	binding.set("action", action)
	binding.set("source", source_value)
	return binding


func _new_action_map_with(action: StringName, source_value: int = 2):
	if not ClassDB.class_exists("GameInputActionMap"):
		return null
	var binding = _new_binding(action, source_value)
	if binding == null:
		return null
	var action_map = ClassDB.instantiate("GameInputActionMap")
	action_map.set("bindings", [binding])
	return action_map


func _new_mapper():
	if not ClassDB.class_exists("GameInputMapper"):
		return null
	var mapper = ClassDB.instantiate("GameInputMapper")
	if not mapper.has_method("_test_mark_binding_pressed"):
		mapper.free()
		return null
	return mapper


func _seed_mapper_press(mapper: Node, action: StringName) -> void:
	mapper.call("_test_mark_binding_pressed", 0)
	assert_true(Input.is_action_pressed(action), "test hook seeds a held action")
	assert_eq(mapper.call("get_active_binding_count"), 1,
			"test hook seeds one active binding")


func test_action_map_swap_releases_previous_held_action() -> void:
	var old_action := &"test_gi_stuck_swap_old"
	var new_action := &"test_gi_stuck_swap_new"
	_ensure_action(old_action)
	_ensure_action(new_action)

	var mapper := _new_mapper()
	var first := _new_action_map_with(old_action)
	var second := _new_action_map_with(new_action)
	if mapper == null or first == null or second == null:
		if mapper != null: mapper.free()
		pending("GameInputMapper test hooks or action-map classes missing")
		return

	mapper.set("action_map", first)
	_seed_mapper_press(mapper, old_action)

	mapper.set("action_map", second)
	assert_false(Input.is_action_pressed(old_action),
			"set_action_map() releases actions held by the previous map")
	assert_eq(mapper.call("get_active_binding_count"), 0,
			"set_action_map() clears previous pressed-state cache")
	mapper.free()


func test_active_map_mutations_release_held_actions_and_invalidate_cache() -> void:
	var cases := [
		{
			"name": "add_binding",
			"held": &"test_gi_stuck_add_held",
			"replacement": &"test_gi_stuck_add_new",
		},
		{
			"name": "set_bindings",
			"held": &"test_gi_stuck_set_held",
			"replacement": &"test_gi_stuck_set_new",
		},
		{
			"name": "clear",
			"held": &"test_gi_stuck_clear_held",
			"replacement": &"test_gi_stuck_clear_new",
		},
	]

	for case in cases:
		var held: StringName = case["held"]
		var replacement: StringName = case["replacement"]
		_ensure_action(held)
		_ensure_action(replacement)

		var mapper := _new_mapper()
		var action_map := _new_action_map_with(held)
		if mapper == null or action_map == null:
			if mapper != null: mapper.free()
			pending("GameInputMapper test hooks or action-map classes missing")
			return

		mapper.set("action_map", action_map)
		_seed_mapper_press(mapper, held)
		mapper.call("_test_prime_native_handles_cache", 0, true)
		assert_eq(mapper.call("_test_get_native_handles_cache_count"), 1,
				"%s primes native event cache" % case["name"])

		match case["name"]:
			"add_binding":
				action_map.add_binding(_new_binding(replacement))
			"set_bindings":
				action_map.set("bindings", [_new_binding(replacement)])
			"clear":
				action_map.clear()

		assert_false(Input.is_action_pressed(held),
				"%s releases the action held by the active map" % case["name"])
		assert_eq(mapper.call("get_active_binding_count"), 0,
				"%s clears previous pressed-state cache" % case["name"])
		assert_eq(mapper.call("_test_get_native_handles_cache_count"), 0,
				"%s invalidates native event cache" % case["name"])
		mapper.free()


func test_binding_property_mutation_releases_held_action() -> void:
	var held := &"test_gi_stuck_binding_mutation_held"
	var replacement := &"test_gi_stuck_binding_mutation_new"
	_ensure_action(held)
	_ensure_action(replacement)

	var mapper := _new_mapper()
	var action_map := _new_action_map_with(held)
	if mapper == null or action_map == null:
		if mapper != null: mapper.free()
		pending("GameInputMapper test hooks or action-map classes missing")
		return

	var binding = action_map.get_binding(0)
	mapper.set("action_map", action_map)
	_seed_mapper_press(mapper, held)

	binding.set("action", replacement)
	assert_false(Input.is_action_pressed(held),
			"mutating a binding on the active map releases the old held action")
	assert_eq(mapper.call("get_active_binding_count"), 0,
			"binding mutation clears previous pressed-state cache")
	mapper.free()


func test_uninitialized_runtime_early_return_releases_held_action() -> void:
	if pending_unless_runtime_available():
		return

	var held := &"test_gi_stuck_runtime_unavailable"
	_ensure_action(held)

	var mapper := _new_mapper()
	var action_map := _new_action_map_with(held)
	if mapper == null or action_map == null:
		if mapper != null: mapper.free()
		pending("GameInputMapper test hooks or action-map classes missing")
		return

	var gi = get_gameinput()
	gi.shutdown()
	mapper.set("action_map", action_map)
	_seed_mapper_press(mapper, held)

	var holder := Node.new()
	get_tree().root.add_child(holder)
	holder.add_child(mapper)
	await get_tree().process_frame

	assert_false(Input.is_action_pressed(held),
			"process early-return when GameInput is stopped releases held actions")
	assert_eq(mapper.call("get_active_binding_count"), 0,
			"process early-return clears previous pressed-state cache")
	holder.remove_child(mapper)
	mapper.free()
	holder.free()


func test_missing_target_device_releases_held_action() -> void:
	if pending_unless_runtime_available():
		return

	var gi = get_gameinput()
	gi.shutdown()
	var started: bool = gi.initialize()
	if not started:
		pending("GameInput.initialize() returned false (no GameInput on host)")
		return

	var held := &"test_gi_stuck_missing_device"
	_ensure_action(held)

	var mapper := _new_mapper()
	var action_map := _new_action_map_with(held)
	if mapper == null or action_map == null:
		if mapper != null: mapper.free()
		gi.shutdown()
		pending("GameInputMapper test hooks or action-map classes missing")
		return

	mapper.set("action_map", action_map)
	mapper.set("target_device_id", IMPOSSIBLE_DEVICE_ID)
	_seed_mapper_press(mapper, held)

	var holder := Node.new()
	get_tree().root.add_child(holder)
	holder.add_child(mapper)
	await get_tree().process_frame

	assert_false(Input.is_action_pressed(held),
			"missing/disconnected target device releases held actions")
	assert_eq(mapper.call("get_active_binding_count"), 0,
			"missing/disconnected target device clears pressed-state cache")
	holder.remove_child(mapper)
	mapper.free()
	holder.free()
	gi.shutdown()


func test_non_gamepad_target_without_reading_releases_held_action() -> void:
	if pending_unless_runtime_available():
		return

	var gi = get_gameinput()
	gi.shutdown()
	var started: bool = gi.initialize()
	if not started:
		pending("GameInput.initialize() returned false (no GameInput on host)")
		return

	var keyboard_kind: int = ClassDB.class_get_integer_constant("GameInput", "DEVICE_KEYBOARD")
	var devices: Array = gi.get_devices(keyboard_kind)
	if devices.is_empty():
		gi.shutdown()
		pending("No GameInput keyboard device available to exercise null-reading path")
		return

	var held := &"test_gi_stuck_null_reading"
	_ensure_action(held)

	var mapper := _new_mapper()
	var action_map := _new_action_map_with(held)
	if mapper == null or action_map == null:
		if mapper != null: mapper.free()
		gi.shutdown()
		pending("GameInputMapper test hooks or action-map classes missing")
		return

	mapper.set("action_map", action_map)
	mapper.set("target_device_id", devices[0].get_device_id())
	_seed_mapper_press(mapper, held)

	var holder := Node.new()
	get_tree().root.add_child(holder)
	holder.add_child(mapper)
	await get_tree().process_frame

	assert_false(Input.is_action_pressed(held),
			"target with no gamepad reading releases held actions")
	assert_eq(mapper.call("get_active_binding_count"), 0,
			"null-reading path clears pressed-state cache")
	holder.remove_child(mapper)
	mapper.free()
	holder.free()
	gi.shutdown()
