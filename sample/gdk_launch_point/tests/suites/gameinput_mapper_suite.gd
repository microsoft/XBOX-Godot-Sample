extends RefCounted
## Verifies that GameInputMapper is constructible, owns its action_map / mask /
## target_device_id properties, and stays inert (no crashes, no actions emitted)
## without a real device. Device-driven press/release transitions are covered
## by the manual test checklist in docs/godot-gameinput-manual-tests.md.


func run(context) -> void:
	_test_mapper_construction(context)
	_test_mapper_with_action_map(context)
	_test_mapper_inert_without_device(context)


func _test_mapper_construction(context) -> void:
	context.log_section("GameInputMapper — Construction & Properties")

	if not ClassDB.class_exists("GameInputMapper"):
		context.log_skip("GameInputMapper missing")
		return

	var mapper = ClassDB.instantiate("GameInputMapper")
	context.assert_not_null(mapper, "GameInputMapper.new()")
	context.assert_true(mapper is Node, "GameInputMapper is a Node")
	context.assert_eq(mapper.get("target_device_id"), -1,
			"target_device_id default -1 (= primary device of mask)")
	context.assert_true(mapper.get("target_kind_mask") is int,
			"target_kind_mask is int")
	if mapper != null:
		mapper.free()


func _test_mapper_with_action_map(context) -> void:
	context.log_section("GameInputMapper — action_map binding")

	if not ClassDB.class_exists("GameInputMapper"):
		return

	var mapper = ClassDB.instantiate("GameInputMapper")
	var action_map = ClassDB.instantiate("GameInputActionMap")
	mapper.set("action_map", action_map)
	context.assert_true(mapper.get("action_map") == action_map,
			"action_map setter accepts GameInputActionMap")

	mapper.set("action_map", null)
	context.assert_true(mapper.get("action_map") == null,
			"action_map can be set to null")
	if mapper != null:
		mapper.free()


func _test_mapper_inert_without_device(context) -> void:
	context.log_section("GameInputMapper — Inert without a real device")

	if not ClassDB.class_exists("GameInputMapper") or not ClassDB.class_exists("GameInputBinding"):
		return

	if not InputMap.has_action("test_gameinput_jump"):
		InputMap.add_action("test_gameinput_jump")

	var binding = ClassDB.instantiate("GameInputBinding")
	binding.set("action", &"test_gameinput_jump")
	binding.set("source", 2)

	var action_map = ClassDB.instantiate("GameInputActionMap")
	action_map.set("bindings", [binding])

	var mapper = ClassDB.instantiate("GameInputMapper")
	mapper.set("action_map", action_map)

	var holder := Node.new()
	holder.add_child(mapper)
	holder.remove_child(mapper)
	mapper.free()
	holder.free()
	context.log_pass("Mapper add/remove cycle without device is safe")

	context.assert_true(not Input.is_action_pressed("test_gameinput_jump"),
			"test action not stuck pressed after mapper removal")

	if InputMap.has_action("test_gameinput_jump"):
		InputMap.erase_action("test_gameinput_jump")
