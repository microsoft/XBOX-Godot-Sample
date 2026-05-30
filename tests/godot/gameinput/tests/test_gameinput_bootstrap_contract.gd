extends "res://addons/godot_gdk_tests/gameinput_test_base.gd"
## Audit C3 backfill for GameInputBootstrap project-setting side effects and
## initialization ownership semantics.

const BootstrapScript = preload("res://addons/godot_gameinput/runtime/gameinput_bootstrap.gd")

const SETTING_INITIALIZE_ON_STARTUP := "game_input/runtime/initialize_on_startup"
const SETTING_AUTO_POLL := "game_input/runtime/auto_poll"
const SETTING_DEFAULT_ACTION_MAP := "game_input/mapper/default_action_map"

var _saved_settings: Dictionary = {}
var _files_to_cleanup: Array[String] = []


func after_each() -> void:
	for key in _saved_settings.keys():
		ProjectSettings.set_setting(key, _saved_settings[key])
	_saved_settings.clear()
	for path in _files_to_cleanup:
		if FileAccess.file_exists(path):
			DirAccess.remove_absolute(ProjectSettings.globalize_path(path))
	_files_to_cleanup.clear()
	var gi = get_gameinput()
	if gi != null:
		gi.shutdown()


func _set_bootstrap_setting(key: String, value: Variant) -> void:
	if not _saved_settings.has(key):
		_saved_settings[key] = ProjectSettings.get_setting(key, null)
	ProjectSettings.set_setting(key, value)


func _set_base_settings() -> void:
	_set_bootstrap_setting(SETTING_INITIALIZE_ON_STARTUP, false)
	_set_bootstrap_setting(SETTING_AUTO_POLL, false)
	_set_bootstrap_setting(SETTING_DEFAULT_ACTION_MAP, "")


func _spawn_bootstrap() -> Node:
	var bootstrap: Node = BootstrapScript.new()
	get_tree().root.add_child(bootstrap)
	await get_tree().process_frame
	return bootstrap


func _free_bootstrap(bootstrap: Node) -> void:
	if bootstrap == null:
		return
	if bootstrap.get_parent() != null:
		bootstrap.get_parent().remove_child(bootstrap)
	bootstrap.free()


func _new_action_map_resource():
	if not ClassDB.class_exists("GameInputActionMap") or not ClassDB.class_exists("GameInputBinding"):
		return null
	var binding = ClassDB.instantiate("GameInputBinding")
	binding.set("action", &"bootstrap_contract_action")
	binding.set("source", 2)
	var action_map = ClassDB.instantiate("GameInputActionMap")
	action_map.add_binding(binding)
	return action_map


func _save_resource(resource: Resource, file_name: String) -> String:
	var path := "user://%s" % file_name
	var err := ResourceSaver.save(resource, path)
	assert_eq(err, OK, "ResourceSaver.save(%s) succeeds" % file_name)
	_files_to_cleanup.append(path)
	return path


func test_auto_poll_setting_controls_process_mode() -> void:
	_set_base_settings()
	_set_bootstrap_setting(SETTING_AUTO_POLL, false)
	var bootstrap := await _spawn_bootstrap()
	assert_eq(bootstrap.get("_auto_poll"), false, "auto_poll=false is stored on bootstrap")
	assert_false(bootstrap.is_processing(), "auto_poll=false disables processing")
	_free_bootstrap(bootstrap)

	_set_bootstrap_setting(SETTING_AUTO_POLL, true)
	bootstrap = await _spawn_bootstrap()
	assert_eq(bootstrap.get("_auto_poll"), true, "auto_poll=true is stored on bootstrap")
	assert_true(bootstrap.is_processing(), "auto_poll=true enables processing")
	_free_bootstrap(bootstrap)


func test_default_action_map_valid_path_spawns_default_mapper() -> void:
	_set_base_settings()
	var action_map = _new_action_map_resource()
	if action_map == null:
		pending("GameInputActionMap / GameInputBinding missing")
		return
	var path := _save_resource(action_map, "gameinput_bootstrap_valid_map.tres")
	_set_bootstrap_setting(SETTING_DEFAULT_ACTION_MAP, path)

	var bootstrap := await _spawn_bootstrap()
	var mapper := bootstrap.get_node_or_null("DefaultMapper")
	assert_not_null(mapper, "valid default_action_map spawns DefaultMapper child")
	if mapper != null:
		assert_true(mapper.is_class("GameInputMapper"), "DefaultMapper is a GameInputMapper")
		var loaded_map = mapper.get("action_map")
		assert_not_null(loaded_map, "DefaultMapper action_map is assigned")
		if loaded_map != null:
			assert_true(loaded_map.is_class("GameInputActionMap"),
					"DefaultMapper action_map has GameInputActionMap class")
			assert_eq(loaded_map.get_binding_count(), 1,
					"DefaultMapper action_map preserves saved binding count")
	_free_bootstrap(bootstrap)


func test_default_action_map_invalid_path_soft_fails() -> void:
	_set_base_settings()
	_set_bootstrap_setting(SETTING_DEFAULT_ACTION_MAP, "user://missing_gameinput_bootstrap_map.tres")

	var bootstrap := await _spawn_bootstrap()
	assert_true(bootstrap.get_node_or_null("DefaultMapper") == null,
			"invalid default_action_map path does not spawn a mapper")
	assert_push_warning("does not exist",
			"invalid default_action_map path emits a warning")
	_free_bootstrap(bootstrap)


func test_default_action_map_wrong_type_soft_fails() -> void:
	_set_base_settings()
	var wrong_resource := Resource.new()
	var path := _save_resource(wrong_resource, "gameinput_bootstrap_wrong_type.tres")
	_set_bootstrap_setting(SETTING_DEFAULT_ACTION_MAP, path)

	var bootstrap := await _spawn_bootstrap()
	assert_true(bootstrap.get_node_or_null("DefaultMapper") == null,
			"wrong-type default_action_map path does not spawn a mapper")
	assert_push_warning("not a GameInputActionMap",
			"wrong-type default_action_map emits a warning")
	_free_bootstrap(bootstrap)


func test_initialize_on_startup_owns_shutdown_when_bootstrap_initializes() -> void:
	if pending_unless_runtime_available():
		return

	var gi = get_gameinput()
	gi.shutdown()
	_set_base_settings()
	_set_bootstrap_setting(SETTING_INITIALIZE_ON_STARTUP, true)

	var bootstrap := await _spawn_bootstrap()
	if not gi.is_initialized():
		_free_bootstrap(bootstrap)
		pending("GameInput.initialize() returned false from bootstrap")
		return
	assert_eq(bootstrap.get("_initialized_here"), true,
			"bootstrap records ownership when it initializes GameInput")
	bootstrap.notification(NOTIFICATION_PREDELETE)
	assert_eq(gi.is_initialized(), false,
			"bootstrap-owned runtime shuts down on PREDELETE")
	assert_eq(bootstrap.get("_initialized_here"), false,
			"bootstrap clears ownership after shutdown")
	_free_bootstrap(bootstrap)


func test_initialize_on_startup_does_not_shutdown_preinitialized_runtime() -> void:
	if pending_unless_runtime_available():
		return

	var gi = get_gameinput()
	gi.shutdown()
	var started: bool = gi.initialize()
	if not started:
		pending("GameInput.initialize() returned false before bootstrap")
		return

	_set_base_settings()
	_set_bootstrap_setting(SETTING_INITIALIZE_ON_STARTUP, true)
	var bootstrap := await _spawn_bootstrap()
	assert_eq(bootstrap.get("_initialized_here"), false,
			"bootstrap does not claim ownership of a preinitialized runtime")
	bootstrap.notification(NOTIFICATION_PREDELETE)
	assert_eq(gi.is_initialized(), true,
			"bootstrap PREDELETE leaves caller-owned runtime initialized")
	_free_bootstrap(bootstrap)


func test_should_skip_bootstrap_is_false_in_normal_gut_runner() -> void:
	var bootstrap: Node = BootstrapScript.new()
	assert_eq(bootstrap.call("_should_skip_bootstrap"), false,
			"normal GUT invocation does not trigger the gd-script-check skip path")
	bootstrap.free()
