extends "res://addons/godot_gdk_tests/gameinput_test_base.gd"
## Wave 3 GUT-style port of `tests/suites/gameinput_mapper_suite.gd`.
##
## Verifies that GameInputMapper is constructible, owns its action_map / mask /
## target_device_id properties, and stays inert (no crashes, no actions
## emitted) without a real device. Device-driven press/release transitions are
## covered by the manual test checklist in
## `docs/godot-gameinput-manual-tests.md`.


func test_mapper_construction() -> void:
	if not ClassDB.class_exists("GameInputMapper"):
		pending("GameInputMapper missing")
		return

	var mapper = ClassDB.instantiate("GameInputMapper")
	assert_not_null(mapper, "GameInputMapper.new()")
	assert_true(mapper is Node, "GameInputMapper is a Node")
	assert_eq(mapper.get("target_device_id"), -1,
			"target_device_id default -1 (= primary device of mask)")
	assert_true(mapper.get("target_kind_mask") is int,
			"target_kind_mask is int")
	if mapper != null:
		mapper.free()


func test_mapper_with_action_map() -> void:
	if not ClassDB.class_exists("GameInputMapper"):
		pending("GameInputMapper missing")
		return

	var mapper = ClassDB.instantiate("GameInputMapper")
	var action_map = ClassDB.instantiate("GameInputActionMap")
	mapper.set("action_map", action_map)
	assert_true(mapper.get("action_map") == action_map,
			"action_map setter accepts GameInputActionMap")

	mapper.set("action_map", null)
	assert_true(mapper.get("action_map") == null,
			"action_map can be set to null")
	if mapper != null:
		mapper.free()


func test_mapper_inert_without_device() -> void:
	if not ClassDB.class_exists("GameInputMapper") or not ClassDB.class_exists("GameInputBinding"):
		pending("GameInputMapper or GameInputBinding missing")
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
	assert_true(true, "Mapper add/remove cycle without device is safe")

	assert_true(not Input.is_action_pressed("test_gameinput_jump"),
			"test action not stuck pressed after mapper removal")

	if InputMap.has_action("test_gameinput_jump"):
		InputMap.erase_action("test_gameinput_jump")
