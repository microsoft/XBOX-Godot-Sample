extends "res://addons/godot_gdk_tests/gameinput_test_base.gd"
## Wave 3 GUT-style port of `tests/suites/gameinput_resource_suite.gd`.
##
## Verifies the inspector-friendly Resource surface for GameInput action maps:
## GameInputBinding round-trips via ResourceSaver/ResourceLoader and exports
## the expected typed properties; GameInputActionMap round-trips with its
## bindings array intact.


func test_binding_defaults_and_setters() -> void:
	if not ClassDB.class_exists("GameInputBinding"):
		pending("GameInputBinding missing")
		return

	var binding = ClassDB.instantiate("GameInputBinding")
	assert_not_null(binding, "GameInputBinding.new()")

	assert_eq(binding.get("is_axis"), false, "binding.is_axis default false")
	assert_eq(binding.get("axis_invert"), false, "binding.axis_invert default false")
	assert_eq_approx(binding.get("axis_threshold"), 0.5, "binding.axis_threshold default 0.5")
	assert_eq_approx(binding.get("deadzone"), 0.2, "binding.deadzone default 0.2")

	binding.set("action", &"jump")
	binding.set("source", 2)
	binding.set("is_axis", true)
	binding.set("axis_threshold", 0.75)
	binding.set("axis_invert", true)
	binding.set("deadzone", 0.1)

	assert_eq(binding.get("action"), &"jump", "binding.action setter")
	assert_eq(binding.get("source"), 2, "binding.source setter")
	assert_eq(binding.get("is_axis"), true, "binding.is_axis setter")
	assert_eq_approx(binding.get("axis_threshold"), 0.75, "binding.axis_threshold setter")
	assert_eq(binding.get("axis_invert"), true, "binding.axis_invert setter")
	assert_eq_approx(binding.get("deadzone"), 0.1, "binding.deadzone setter")


func test_binding_clamps_axis_threshold_and_deadzone() -> void:
	if not ClassDB.class_exists("GameInputBinding"):
		pending("GameInputBinding missing")
		return

	var binding = ClassDB.instantiate("GameInputBinding")
	binding.set("axis_threshold", -1.0)
	binding.set("deadzone", -0.25)
	assert_eq_approx(binding.get("axis_threshold"), 0.0,
			"axis_threshold clamps negative values to 0.0")
	assert_eq_approx(binding.get("deadzone"), 0.0,
			"deadzone clamps negative values to 0.0")

	binding.set("axis_threshold", 2.0)
	binding.set("deadzone", 1.5)
	assert_eq_approx(binding.get("axis_threshold"), 1.0,
			"axis_threshold clamps values above 1.0")
	assert_eq_approx(binding.get("deadzone"), 1.0,
			"deadzone clamps values above 1.0")


func test_binding_save_load_roundtrip() -> void:
	if not ClassDB.class_exists("GameInputBinding"):
		pending("GameInputBinding missing")
		return

	var binding = ClassDB.instantiate("GameInputBinding")
	binding.set("action", &"fire")
	binding.set("source", 5)
	binding.set("is_axis", true)
	binding.set("axis_threshold", 0.6)
	binding.set("axis_invert", true)
	binding.set("deadzone", 0.15)

	var path := "user://test_binding.tres"
	var save_err := ResourceSaver.save(binding, path)
	assert_eq(save_err, OK, "ResourceSaver.save(binding) succeeds")

	var loaded = ResourceLoader.load(path, "", ResourceLoader.CACHE_MODE_IGNORE)
	assert_not_null(loaded, "ResourceLoader.load(binding)")

	if loaded != null:
		assert_true(loaded is Resource, "loaded value is Resource")
		assert_true(loaded.is_class("GameInputBinding"),
				"loaded resource class == GameInputBinding")
		assert_eq(loaded.get("action"), &"fire", "round-trip preserves action")
		assert_eq(loaded.get("source"), 5, "round-trip preserves source")
		assert_eq(loaded.get("is_axis"), true, "round-trip preserves is_axis")
		assert_eq_approx(loaded.get("axis_threshold"), 0.6,
				"round-trip preserves axis_threshold")
		assert_eq(loaded.get("axis_invert"), true,
				"round-trip preserves axis_invert")
		assert_eq_approx(loaded.get("deadzone"), 0.15, "round-trip preserves deadzone")

	if FileAccess.file_exists(path):
		DirAccess.remove_absolute(ProjectSettings.globalize_path(path))


func test_action_map_typed_array() -> void:
	if not ClassDB.class_exists("GameInputActionMap"):
		pending("GameInputActionMap missing")
		return

	var action_map = ClassDB.instantiate("GameInputActionMap")
	assert_not_null(action_map, "GameInputActionMap.new()")

	var initial = action_map.get("bindings")
	assert_true(initial is Array, "action_map.bindings is Array")
	assert_eq(initial.size(), 0, "action_map.bindings empty by default")

	var b1 = ClassDB.instantiate("GameInputBinding")
	b1.set("action", &"move_left")
	var b2 = ClassDB.instantiate("GameInputBinding")
	b2.set("action", &"move_right")

	action_map.set("bindings", [b1, b2])
	var bindings = action_map.get("bindings")
	assert_eq(bindings.size(), 2, "action_map.bindings holds 2 entries")
	assert_eq(bindings[0].get("action"), &"move_left", "first binding action preserved")
	assert_eq(bindings[1].get("action"), &"move_right", "second binding action preserved")


func test_action_map_add_get_binding_and_clear_methods() -> void:
	if not ClassDB.class_exists("GameInputActionMap") or not ClassDB.class_exists("GameInputBinding"):
		pending("GameInputActionMap / GameInputBinding missing")
		return

	var action_map = ClassDB.instantiate("GameInputActionMap")
	assert_eq(action_map.get_binding_count(), 0, "new action map count starts at 0")
	assert_true(action_map.get_binding(0) == null,
			"get_binding(0) returns null for an empty map")
	assert_true(action_map.get_binding(-1) == null,
			"get_binding(-1) returns null for an empty map")

	var b1 = ClassDB.instantiate("GameInputBinding")
	b1.set("action", &"jump")
	var b2 = ClassDB.instantiate("GameInputBinding")
	b2.set("action", &"fire")
	action_map.add_binding(b1)
	action_map.add_binding(null)
	action_map.add_binding(b2)

	assert_eq(action_map.get_binding_count(), 2,
			"add_binding ignores null and counts real bindings")
	assert_eq(action_map.get_binding(0), b1, "get_binding(0) returns first binding")
	assert_eq(action_map.get_binding(1), b2, "get_binding(1) returns second binding")
	assert_true(action_map.get_binding(2) == null,
			"get_binding(out-of-range) returns null")

	action_map.clear()
	assert_eq(action_map.get_binding_count(), 0, "clear() removes all bindings")
	assert_true(action_map.get_binding(0) == null,
			"get_binding(0) returns null after clear()")


func test_action_map_save_load_roundtrip() -> void:
	if not ClassDB.class_exists("GameInputActionMap"):
		pending("GameInputActionMap missing")
		return

	var action_map = ClassDB.instantiate("GameInputActionMap")
	var b1 = ClassDB.instantiate("GameInputBinding")
	b1.set("action", &"jump")
	b1.set("source", 2)
	var b2 = ClassDB.instantiate("GameInputBinding")
	b2.set("action", &"fire")
	b2.set("source", 9)
	b2.set("is_axis", true)
	b2.set("axis_threshold", 0.4)
	action_map.set("bindings", [b1, b2])

	var path := "user://test_action_map.tres"
	var save_err := ResourceSaver.save(action_map, path)
	assert_eq(save_err, OK, "ResourceSaver.save(action_map) succeeds")

	var loaded = ResourceLoader.load(path, "", ResourceLoader.CACHE_MODE_IGNORE)
	assert_not_null(loaded, "ResourceLoader.load(action_map)")

	if loaded != null:
		assert_true(loaded.is_class("GameInputActionMap"),
				"loaded resource class == GameInputActionMap")
		var loaded_bindings = loaded.get("bindings")
		assert_eq(loaded_bindings.size(), 2,
				"round-trip preserves binding count")

		if loaded_bindings.size() >= 2:
			assert_eq(loaded_bindings[0].get("action"), &"jump",
					"round-trip preserves binding[0].action")
			assert_eq(loaded_bindings[0].get("source"), 2,
					"round-trip preserves binding[0].source")
			assert_eq(loaded_bindings[1].get("action"), &"fire",
					"round-trip preserves binding[1].action")
			assert_eq(loaded_bindings[1].get("source"), 9,
					"round-trip preserves binding[1].source")
			assert_eq(loaded_bindings[1].get("is_axis"), true,
					"round-trip preserves binding[1].is_axis")
			assert_eq_approx(loaded_bindings[1].get("axis_threshold"), 0.4,
					"round-trip preserves binding[1].axis_threshold")

	if FileAccess.file_exists(path):
		DirAccess.remove_absolute(ProjectSettings.globalize_path(path))
