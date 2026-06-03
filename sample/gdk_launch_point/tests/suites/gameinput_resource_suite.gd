extends RefCounted
## Verifies the inspector-friendly Resource surface for GameInput action maps:
## GameInputBinding round-trips via ResourceSaver/ResourceLoader and exports the
## expected typed properties; GameInputActionMap round-trips with its bindings
## array intact.


func run(context) -> void:
	_test_binding_defaults_and_setters(context)
	_test_binding_save_load_roundtrip(context)
	_test_action_map_typed_array(context)
	_test_action_map_save_load_roundtrip(context)


func _test_binding_defaults_and_setters(context) -> void:
	context.log_section("GameInputBinding — Defaults & Setters")

	if not ClassDB.class_exists("GameInputBinding"):
		context.log_skip("GameInputBinding missing")
		return

	var binding = ClassDB.instantiate("GameInputBinding")
	context.assert_not_null(binding, "GameInputBinding.new()")

	context.assert_eq(binding.get("is_axis"), false, "binding.is_axis default false")
	context.assert_eq(binding.get("axis_invert"), false, "binding.axis_invert default false")
	context.assert_eq_approx(binding.get("axis_threshold"), 0.5, "binding.axis_threshold default 0.5")
	context.assert_eq_approx(binding.get("deadzone"), 0.2, "binding.deadzone default 0.2")

	binding.set("action", &"jump")
	binding.set("source", 2)
	binding.set("is_axis", true)
	binding.set("axis_threshold", 0.75)
	binding.set("axis_invert", true)
	binding.set("deadzone", 0.1)

	context.assert_eq(binding.get("action"), &"jump", "binding.action setter")
	context.assert_eq(binding.get("source"), 2, "binding.source setter")
	context.assert_eq(binding.get("is_axis"), true, "binding.is_axis setter")
	context.assert_eq_approx(binding.get("axis_threshold"), 0.75, "binding.axis_threshold setter")
	context.assert_eq(binding.get("axis_invert"), true, "binding.axis_invert setter")
	context.assert_eq_approx(binding.get("deadzone"), 0.1, "binding.deadzone setter")


func _test_binding_save_load_roundtrip(context) -> void:
	context.log_section("GameInputBinding — Save / Load Round-Trip")

	if not ClassDB.class_exists("GameInputBinding"):
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
	context.assert_eq(save_err, OK, "ResourceSaver.save(binding) succeeds")

	var loaded = ResourceLoader.load(path, "", ResourceLoader.CACHE_MODE_IGNORE)
	context.assert_not_null(loaded, "ResourceLoader.load(binding)")

	if loaded != null:
		context.assert_true(loaded is Resource, "loaded value is Resource")
		context.assert_true(loaded.is_class("GameInputBinding"),
				"loaded resource class == GameInputBinding")
		context.assert_eq(loaded.get("action"), &"fire", "round-trip preserves action")
		context.assert_eq(loaded.get("source"), 5, "round-trip preserves source")
		context.assert_eq(loaded.get("is_axis"), true, "round-trip preserves is_axis")
		context.assert_eq_approx(loaded.get("axis_threshold"), 0.6,
				"round-trip preserves axis_threshold")
		context.assert_eq(loaded.get("axis_invert"), true,
				"round-trip preserves axis_invert")
		context.assert_eq_approx(loaded.get("deadzone"), 0.15, "round-trip preserves deadzone")

	if FileAccess.file_exists(path):
		DirAccess.remove_absolute(ProjectSettings.globalize_path(path))


func _test_action_map_typed_array(context) -> void:
	context.log_section("GameInputActionMap — Typed Array")

	if not ClassDB.class_exists("GameInputActionMap"):
		context.log_skip("GameInputActionMap missing")
		return

	var action_map = ClassDB.instantiate("GameInputActionMap")
	context.assert_not_null(action_map, "GameInputActionMap.new()")

	var initial = action_map.get("bindings")
	context.assert_true(initial is Array, "action_map.bindings is Array")
	context.assert_eq(initial.size(), 0, "action_map.bindings empty by default")

	var b1 = ClassDB.instantiate("GameInputBinding")
	b1.set("action", &"move_left")
	var b2 = ClassDB.instantiate("GameInputBinding")
	b2.set("action", &"move_right")

	action_map.set("bindings", [b1, b2])
	var bindings = action_map.get("bindings")
	context.assert_eq(bindings.size(), 2, "action_map.bindings holds 2 entries")
	context.assert_eq(bindings[0].get("action"), &"move_left", "first binding action preserved")
	context.assert_eq(bindings[1].get("action"), &"move_right", "second binding action preserved")


func _test_action_map_save_load_roundtrip(context) -> void:
	context.log_section("GameInputActionMap — Save / Load Round-Trip")

	if not ClassDB.class_exists("GameInputActionMap"):
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
	context.assert_eq(save_err, OK, "ResourceSaver.save(action_map) succeeds")

	var loaded = ResourceLoader.load(path, "", ResourceLoader.CACHE_MODE_IGNORE)
	context.assert_not_null(loaded, "ResourceLoader.load(action_map)")

	if loaded != null:
		context.assert_true(loaded.is_class("GameInputActionMap"),
				"loaded resource class == GameInputActionMap")
		var loaded_bindings = loaded.get("bindings")
		context.assert_eq(loaded_bindings.size(), 2,
				"round-trip preserves binding count")

		if loaded_bindings.size() >= 2:
			context.assert_eq(loaded_bindings[0].get("action"), &"jump",
					"round-trip preserves binding[0].action")
			context.assert_eq(loaded_bindings[0].get("source"), 2,
					"round-trip preserves binding[0].source")
			context.assert_eq(loaded_bindings[1].get("action"), &"fire",
					"round-trip preserves binding[1].action")
			context.assert_eq(loaded_bindings[1].get("source"), 9,
					"round-trip preserves binding[1].source")
			context.assert_eq(loaded_bindings[1].get("is_axis"), true,
					"round-trip preserves binding[1].is_axis")
			context.assert_eq_approx(loaded_bindings[1].get("axis_threshold"), 0.4,
					"round-trip preserves binding[1].axis_threshold")

	if FileAccess.file_exists(path):
		DirAccess.remove_absolute(ProjectSettings.globalize_path(path))
