extends "res://addons/godot_gdk_tests/gameinput_test_base.gd"
## Wave 4 — `GameInputMapper` extensions.
##
## Covers behaviour the existing `test_gameinput_mapper.gd` does not:
##   * Multiple `GameInputMapper` instances coexist with independent
##     `action_map`, `target_kind_mask`, and `target_device_id` settings —
##     state never leaks across instances.
##   * `action_map` can be hot-swapped on a live mapper. The setter accepts a
##     new typed `GameInputActionMap` reference and returns it via the getter
##     immediately; subsequent swaps (back to null, then to a third map) are
##     also stable.
##   * `target_device_id == -1` means "primary device of the kind mask" — it is
##     the documented default value and stays a `int` after explicit set.
##     A negative non-`-1` value is also accepted (the mapper soft-fails to no
##     active device when nothing matches).


func _new_mapper() -> Node:
	if not ClassDB.class_exists("GameInputMapper"):
		return null
	return ClassDB.instantiate("GameInputMapper") as Node


func _new_action_map_with(action_name: StringName, source_value: int) -> Resource:
	if not ClassDB.class_exists("GameInputActionMap") \
			or not ClassDB.class_exists("GameInputBinding"):
		return null
	var binding = ClassDB.instantiate("GameInputBinding")
	binding.set("action", action_name)
	binding.set("source", source_value)
	var action_map = ClassDB.instantiate("GameInputActionMap")
	action_map.set("bindings", [binding])
	return action_map


func test_multiple_mappers_independent_state() -> void:
	var a := _new_mapper()
	var b := _new_mapper()
	var c := _new_mapper()
	if a == null or b == null or c == null:
		if a != null: a.free()
		if b != null: b.free()
		if c != null: c.free()
		pending("GameInputMapper missing")
		return

	# Three independent kind masks via Godot Input flag values. We keep the
	# mapper's documented default (KIND_GAMEPAD == 1) on `a` and pick the
	# other documented bits for `b` (KEYBOARD) and `c` (MOUSE).
	var kind_gamepad: int = ClassDB.class_get_integer_constant("GameInputMapper", "KIND_GAMEPAD")
	var kind_keyboard: int = ClassDB.class_get_integer_constant("GameInputMapper", "KIND_KEYBOARD")
	var kind_mouse: int = ClassDB.class_get_integer_constant("GameInputMapper", "KIND_MOUSE")

	assert_eq(a.get("target_kind_mask"), kind_gamepad,
			"mapper a default target_kind_mask == KIND_GAMEPAD")

	b.set("target_kind_mask", kind_keyboard)
	c.set("target_kind_mask", kind_mouse)
	a.set("target_device_id", 11)
	b.set("target_device_id", 22)
	c.set("target_device_id", 33)

	# Each mapper retains the values it was given — no shared static state.
	assert_eq(a.get("target_kind_mask"), kind_gamepad,
			"mapper a kind mask preserved after b/c writes")
	assert_eq(b.get("target_kind_mask"), kind_keyboard,
			"mapper b kind mask preserved")
	assert_eq(c.get("target_kind_mask"), kind_mouse,
			"mapper c kind mask preserved")
	assert_eq(a.get("target_device_id"), 11, "mapper a device id preserved")
	assert_eq(b.get("target_device_id"), 22, "mapper b device id preserved")
	assert_eq(c.get("target_device_id"), 33, "mapper c device id preserved")

	# Independent action_map references.
	var map_a := _new_action_map_with(&"wave4_jump_a", 2)
	var map_b := _new_action_map_with(&"wave4_jump_b", 3)
	a.set("action_map", map_a)
	b.set("action_map", map_b)
	c.set("action_map", null)

	assert_true(a.get("action_map") == map_a,
			"mapper a action_map identity preserved")
	assert_true(b.get("action_map") == map_b,
			"mapper b action_map identity preserved")
	assert_true(c.get("action_map") == null,
			"mapper c keeps null action_map")

	a.free()
	b.free()
	c.free()


func test_action_map_hot_swap() -> void:
	var mapper := _new_mapper()
	if mapper == null:
		pending("GameInputMapper missing")
		return

	var first := _new_action_map_with(&"wave4_swap_first", 2)
	var second := _new_action_map_with(&"wave4_swap_second", 3)
	if first == null or second == null:
		mapper.free()
		pending("GameInputActionMap / GameInputBinding missing")
		return

	# Initial assignment.
	mapper.set("action_map", first)
	assert_true(mapper.get("action_map") == first,
			"action_map setter accepts initial map")

	# Hot-swap to a different map. The getter must return the new identity
	# immediately — there is no copy-on-set behaviour.
	mapper.set("action_map", second)
	assert_true(mapper.get("action_map") == second,
			"action_map setter accepts hot-swap to second map")
	assert_true(mapper.get("action_map") != first,
			"action_map getter no longer returns the first map after swap")

	# Clear and re-assign the original — exercises null-then-typed transitions.
	mapper.set("action_map", null)
	assert_true(mapper.get("action_map") == null,
			"action_map setter accepts null clear")
	mapper.set("action_map", first)
	assert_true(mapper.get("action_map") == first,
			"action_map setter accepts re-assignment back to original map")

	# Adding the mapper to a holder and removing it should remain safe across
	# the swaps — the mapper must drop any per-frame state cleanly.
	var holder := Node.new()
	holder.add_child(mapper)
	holder.remove_child(mapper)
	holder.free()
	mapper.free()
	assert_true(true, "Hot-swap + add/remove cycle did not crash")


func test_target_device_id_any_device_semantics() -> void:
	var mapper := _new_mapper()
	if mapper == null:
		pending("GameInputMapper missing")
		return

	# -1 is the documented "primary device of the configured kind mask"
	# default. Verify it is exposed as the initial value, can be re-applied
	# after a different value, and stays a strict `int` (not a float).
	var initial = mapper.get("target_device_id")
	assert_true(typeof(initial) == TYPE_INT,
			"target_device_id is exposed as int")
	assert_eq(initial, -1,
			"target_device_id default == -1 (primary of kind mask)")

	mapper.set("target_device_id", 42)
	assert_eq(mapper.get("target_device_id"), 42,
			"explicit positive id round-trips")

	mapper.set("target_device_id", -1)
	assert_eq(mapper.get("target_device_id"), -1,
			"resetting to -1 restores 'any device' semantics")

	# Negative values other than -1 are still accepted (the mapper just won't
	# match any real device); important to assert no crash so app code that
	# stores stale ids cannot brick startup.
	mapper.set("target_device_id", -7)
	assert_eq(mapper.get("target_device_id"), -7,
			"negative id other than -1 still round-trips (soft-fail at runtime)")

	mapper.free()
	assert_true(true, "Mapper free() after id churn is safe")
