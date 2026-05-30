extends "res://addons/godot_gdk_tests/gameinput_test_base.gd"
## Audit C3 backfill for mapper cache, EXIT_TREE, active-count, axis, and
## missing-action debounce paths not covered by the stuck-action lane.

var _actions_to_cleanup: Array[StringName] = []


func after_each() -> void:
	for action in _actions_to_cleanup:
		Input.action_release(action)
		if InputMap.has_action(action):
			InputMap.erase_action(action)
	_actions_to_cleanup.clear()
	var gi = get_gameinput()
	if gi != null:
		gi.shutdown()


func _ensure_action(action: StringName) -> void:
	if not InputMap.has_action(action):
		InputMap.add_action(action)
		_actions_to_cleanup.append(action)
	Input.action_release(action)


func _new_binding(action: StringName, source_value: int = 2, is_axis: bool = false):
	if not ClassDB.class_exists("GameInputBinding"):
		return null
	var binding = ClassDB.instantiate("GameInputBinding")
	binding.set("action", action)
	binding.set("source", source_value)
	binding.set("is_axis", is_axis)
	return binding


func _new_action_map(bindings: Array):
	if not ClassDB.class_exists("GameInputActionMap"):
		return null
	var action_map = ClassDB.instantiate("GameInputActionMap")
	action_map.set("bindings", bindings)
	return action_map


func _new_action_map_with(action: StringName, source_value: int = 2, is_axis: bool = false):
	var binding = _new_binding(action, source_value, is_axis)
	if binding == null:
		return null
	return _new_action_map([binding])


func _new_mapper_basic():
	if not ClassDB.class_exists("GameInputMapper"):
		return null
	return ClassDB.instantiate("GameInputMapper")


func _new_mapper_with_test_hooks():
	var mapper = _new_mapper_basic()
	if mapper == null:
		return null
	if not mapper.has_method("_test_mark_binding_pressed") \
			or not mapper.has_method("_test_prime_native_handles_cache") \
			or not mapper.has_method("_test_get_native_handles_cache_count"):
		mapper.free()
		return null
	return mapper


func _seed_mapper_press(mapper: Node, action: StringName) -> void:
	mapper.call("_test_mark_binding_pressed", 0)
	assert_true(Input.is_action_pressed(action), "test hook seeds a held action")
	assert_eq(mapper.call("get_active_binding_count"), 1,
			"get_active_binding_count() reports one held binding")


func _device_constant(name: String) -> int:
	return ClassDB.class_get_integer_constant("GameInput", name)


func _source_constant(name: String) -> int:
	return ClassDB.class_get_integer_constant("GameInputDevice", name)


func test_native_handles_cache_reuses_identical_binding_index() -> void:
	var action_a := &"test_gi_cache_a"
	var action_b := &"test_gi_cache_b"
	_ensure_action(action_a)
	_ensure_action(action_b)

	var mapper = _new_mapper_with_test_hooks()
	var action_map = _new_action_map([
		_new_binding(action_a),
		_new_binding(action_b),
	])
	if mapper == null or action_map == null:
		if mapper != null: mapper.free()
		pending("GameInputMapper cache test hooks or action-map classes missing")
		return

	mapper.set("action_map", action_map)
	assert_eq(mapper.call("_test_get_native_handles_cache_count"), 0,
			"native-handles cache starts empty")
	mapper.call("_test_prime_native_handles_cache", 0, false)
	assert_eq(mapper.call("_test_get_native_handles_cache_count"), 1,
			"first lookup fills one cache entry")
	mapper.call("_test_prime_native_handles_cache", 0, true)
	assert_eq(mapper.call("_test_get_native_handles_cache_count"), 1,
			"repeated identical-binding lookup reuses the cached entry")
	mapper.call("_test_prime_native_handles_cache", 1, true)
	assert_eq(mapper.call("_test_get_native_handles_cache_count"), 2,
			"a different binding index gets its own cache entry")
	mapper.free()


func test_native_handles_cache_clears_on_map_swap_and_next_frame() -> void:
	var first_action := &"test_gi_cache_swap_first"
	var second_action := &"test_gi_cache_swap_second"
	_ensure_action(first_action)
	_ensure_action(second_action)

	var mapper = _new_mapper_with_test_hooks()
	var first = _new_action_map_with(first_action)
	var second = _new_action_map_with(second_action)
	if mapper == null or first == null or second == null:
		if mapper != null: mapper.free()
		pending("GameInputMapper cache test hooks or action-map classes missing")
		return

	mapper.set("action_map", first)
	mapper.call("_test_prime_native_handles_cache", 0, true)
	assert_eq(mapper.call("_test_get_native_handles_cache_count"), 1,
			"cache primed before map swap")
	mapper.set("action_map", second)
	assert_eq(mapper.call("_test_get_native_handles_cache_count"), 0,
			"set_action_map() clears native-handles cache")

	mapper.call("_test_prime_native_handles_cache", 0, true)
	assert_eq(mapper.call("_test_get_native_handles_cache_count"), 1,
			"cache primed before next-frame invalidation")
	var holder := Node.new()
	get_tree().root.add_child(holder)
	holder.add_child(mapper)
	await get_tree().process_frame
	assert_eq(mapper.call("_test_get_native_handles_cache_count"), 0,
			"mapper process clears stale native-handles cache entries on a new frame")
	holder.remove_child(mapper)
	mapper.free()
	holder.free()


func test_exit_tree_after_processing_releases_held_action() -> void:
	var held := &"test_gi_exit_tree_after_frame"
	_ensure_action(held)

	var mapper = _new_mapper_with_test_hooks()
	var action_map = _new_action_map_with(held)
	if mapper == null or action_map == null:
		if mapper != null: mapper.free()
		pending("GameInputMapper test hooks or action-map classes missing")
		return

	mapper.set("action_map", action_map)
	var holder := Node.new()
	get_tree().root.add_child(holder)
	holder.add_child(mapper)
	_seed_mapper_press(mapper, held)
	await get_tree().process_frame
	holder.remove_child(mapper)
	assert_false(Input.is_action_pressed(held),
			"NOTIFICATION_EXIT_TREE releases held actions after at least one processed frame")
	assert_eq(mapper.call("get_active_binding_count"), 0,
			"EXIT_TREE clears the active binding cache")
	mapper.free()
	holder.free()


func test_active_binding_count_zero_for_inert_mapper() -> void:
	var mapper = _new_mapper_basic()
	if mapper == null:
		pending("GameInputMapper missing")
		return
	assert_eq(mapper.call("get_active_binding_count"), 0,
			"new mapper has no active bindings")
	var holder := Node.new()
	get_tree().root.add_child(holder)
	holder.add_child(mapper)
	await get_tree().process_frame
	assert_eq(mapper.call("get_active_binding_count"), 0,
			"mapper without action_map/device remains at zero active bindings")
	holder.remove_child(mapper)
	mapper.free()
	holder.free()


func test_live_axis_binding_path_stays_bounded_below_threshold() -> void:
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
		pending("No live GameInput gamepad connected for axis binding coverage")
		return

	var action := &"test_gi_axis_bound"
	_ensure_action(action)
	var binding = _new_binding(action, _source_constant("SRC_AXIS_LEFT_X"), true)
	if binding == null:
		pending("GameInputBinding missing")
		return
	binding.set("axis_threshold", 1.0)
	binding.set("deadzone", 0.0)
	var action_map = _new_action_map([binding])
	var mapper = _new_mapper_basic()
	if mapper == null or action_map == null:
		if mapper != null: mapper.free()
		pending("GameInputMapper or action-map classes missing")
		return

	mapper.set("action_map", action_map)
	mapper.set("target_device_id", device.get_device_id())
	var holder := Node.new()
	get_tree().root.add_child(holder)
	holder.add_child(mapper)
	await get_tree().process_frame
	await get_tree().process_frame
	assert_false(Input.is_action_pressed(action),
			"axis binding with threshold 1.0 does not synthesize a pressed action")
	assert_eq(mapper.call("get_active_binding_count"), 0,
			"axis path leaves no active binding below threshold")
	holder.remove_child(mapper)
	mapper.free()
	holder.free()


func test_live_missing_action_warning_is_debounced() -> void:
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
		pending("No live GameInput gamepad connected for missing-action debounce coverage")
		return

	var missing_action := &"test_gi_missing_action_debounce"
	if InputMap.has_action(missing_action):
		InputMap.erase_action(missing_action)
	var action_map = _new_action_map_with(missing_action)
	var mapper = _new_mapper_basic()
	if mapper == null or action_map == null:
		if mapper != null: mapper.free()
		pending("GameInputMapper or action-map classes missing")
		return

	mapper.set("action_map", action_map)
	mapper.set("target_device_id", device.get_device_id())
	var holder := Node.new()
	get_tree().root.add_child(holder)
	holder.add_child(mapper)
	await get_tree().process_frame
	await get_tree().process_frame
	await get_tree().process_frame
	assert_push_warning_count(1,
			"missing InputMap action warning is emitted once across repeated frames")
	holder.remove_child(mapper)
	mapper.free()
	holder.free()
