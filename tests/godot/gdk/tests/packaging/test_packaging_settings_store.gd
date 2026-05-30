extends GutTest

const PackagingSettingsStoreScript = preload("res://addons/godot_gdk_packaging/core/packaging_settings_store.gd")

const _FIXTURE_DIR := "user://test_packaging_settings_store"


func before_each() -> void:
	_reset_fixture()
	DirAccess.make_dir_recursive_absolute(_fixture_root())


func after_each() -> void:
	_reset_fixture()


func test_default_state_is_deep_copied_between_callers() -> void:
	var store = PackagingSettingsStoreScript.new()
	var first: Dictionary = store.get_default_state()
	var second: Dictionary = store.get_default_state()

	first["packaging"]["source_dir"] = "mutated"
	first["sandbox"]["sandbox_id"] = "XDKS.1"

	assert_eq(second["packaging"]["source_dir"], "", "packaging defaults are copied per call")
	assert_eq(second["sandbox"]["sandbox_id"], "", "nested sandbox defaults are not shared")
	assert_true(second["packaging"]["auto_genmap"], "boolean defaults remain intact")


func test_load_missing_file_returns_full_default_shape() -> void:
	var state: Dictionary = PackagingSettingsStoreScript.new().load_state(_fixture_path("missing.cfg"))

	assert_true(state.has_all(["packaging", "sandbox", "export"]))
	assert_eq(state["packaging"]["source_dir"], "")
	assert_eq(state["packaging"]["updcompat_option"], 0)
	assert_eq(state["sandbox"]["test_account"], "")
	assert_eq(state["export"]["clean_build"], false)


func test_save_and_load_round_trips_known_sections_and_types() -> void:
	var path: String = _fixture_path("settings.cfg")
	var store = PackagingSettingsStoreScript.new()
	var state: Dictionary = store.get_default_state()
	state["packaging"]["source_dir"] = "C:\\Build Content"
	state["packaging"]["map_file"] = "C:\\Maps\\layout.xml"
	state["packaging"]["encrypt_option"] = 2
	state["packaging"]["encrypt_key"] = "C:\\Keys\\license.ekb"
	state["sandbox"]["sandbox_id"] = "XDKS.1"
	state["sandbox"]["test_account"] = "user@example.test"
	state["export"]["preset_name"] = "Windows Desktop"
	state["export"]["clean_build"] = true

	var err: Error = store.save_state(path, state)
	var loaded: Dictionary = store.load_state(path)

	assert_eq(err, OK)
	assert_eq(loaded["packaging"]["source_dir"], "C:\\Build Content")
	assert_eq(loaded["packaging"]["map_file"], "C:\\Maps\\layout.xml")
	assert_eq(loaded["packaging"]["encrypt_option"], 2)
	assert_eq(loaded["packaging"]["encrypt_key"], "C:\\Keys\\license.ekb")
	assert_eq(loaded["sandbox"]["sandbox_id"], "XDKS.1")
	assert_eq(loaded["sandbox"]["test_account"], "user@example.test")
	assert_eq(loaded["export"]["preset_name"], "Windows Desktop")
	assert_true(loaded["export"]["clean_build"], "export clean_build type survives ConfigFile")


func test_load_ignores_unknown_keys_and_fills_omitted_defaults() -> void:
	var path: String = _fixture_path("partial.cfg")
	var cfg := ConfigFile.new()
	cfg.set_value("packaging", "source_dir", "C:\\Only Source")
	cfg.set_value("packaging", "unknown_key", "ignored")
	cfg.set_value("unknown_section", "value", 123)
	assert_eq(cfg.save(path), OK)

	var loaded: Dictionary = PackagingSettingsStoreScript.new().load_state(path)

	assert_eq(loaded["packaging"]["source_dir"], "C:\\Only Source")
	assert_false(loaded["packaging"].has("unknown_key"), "unknown keys do not leak into dock state")
	assert_false(loaded.has("unknown_section"), "unknown sections do not leak into dock state")
	assert_true(loaded["packaging"]["auto_genmap"], "omitted keys use defaults")
	assert_eq(loaded["sandbox"]["sandbox_id"], "", "omitted sections use defaults")
	assert_eq(loaded["export"]["preset_name"], "", "export defaults still present")


func _fixture_root() -> String:
	return ProjectSettings.globalize_path(_FIXTURE_DIR)


func _fixture_path(relative_path: String) -> String:
	return _fixture_root().path_join(relative_path)


func _reset_fixture() -> void:
	_remove_tree(_fixture_root())


func _remove_tree(path: String) -> void:
	if FileAccess.file_exists(path):
		DirAccess.remove_absolute(path)
		return
	if not DirAccess.dir_exists_absolute(path):
		return
	for file_name: String in DirAccess.get_files_at(path):
		DirAccess.remove_absolute(path.path_join(file_name))
	for dir_name: String in DirAccess.get_directories_at(path):
		_remove_tree(path.path_join(dir_name))
	DirAccess.remove_absolute(path)
