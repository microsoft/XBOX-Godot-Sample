extends GutTest
## XML rewrite safety coverage for MicrosoftGame.config packaging helpers.

const GameConfigManagerScript = preload("res://addons/godot_gdk_packaging/core/game_config_manager.gd")
const PackagingResult = preload("res://addons/godot_gdk_packaging/core/packaging_result.gd")
const PackagingServiceScript = preload("res://addons/godot_gdk_packaging/core/packaging_service.gd")

const _CONFIG_FILE := "MicrosoftGame.config"
const _ROOT_LOGO := "Logo.png"


class FakeToolchain:
	extends RefCounted

	func is_gdk_available() -> bool:
		return false

	func get_makepkg_path() -> String:
		return ""

	func get_game_config_editor_path() -> String:
		return ""

	func get_sandbox_path() -> String:
		return ""

	func get_dev_account_path() -> String:
		return ""

	func get_gdk_version() -> String:
		return ""

	func get_bin_dir() -> String:
		return ""

	func execute_tool(_exe_path: String, _args: PackedStringArray) -> Dictionary:
		return {"exit_code": 0, "stdout": "", "stderr": ""}

	func launch_detached(_exe_path: String, _args: PackedStringArray) -> int:
		return -1


func before_each() -> void:
	_cleanup_project_artifacts()


func after_each() -> void:
	_cleanup_project_artifacts()


func test_relocate_logos_updates_only_logo_attributes() -> void:
	var config := ""
	config += '<?xml version="1.0" encoding="utf-8"?>\n'
	config += '<Game configVersion="1">\n'
	config += '  <ShellVisuals DefaultDisplayName="Logo.png"\n'
	config += '                PublisherDisplayName="Logo.png"\n'
	config += '                Square150x150Logo="Logo.png"\n'
	config += '                Description="assets\\Logo.png" />\n'
	config += '</Game>\n'
	_write_text(_project_path(_CONFIG_FILE), config)
	_write_text(_project_path(_ROOT_LOGO), "fake-png")

	var manager = GameConfigManagerScript.new(FakeToolchain.new())
	var moved: int = manager.relocate_logos_to_storelogos()

	assert_eq(moved, 1, "root logo moved once")
	var rewritten: String = _read_text(_project_path(_CONFIG_FILE))
	assert_string_contains(rewritten, 'Square150x150Logo="storelogos\\Logo.png"', "logo attr rewritten")
	assert_string_contains(rewritten, 'DefaultDisplayName="Logo.png"', "same value in non-logo attr preserved")
	assert_string_contains(rewritten, 'PublisherDisplayName="Logo.png"', "unrelated attr not rewritten")
	assert_string_contains(rewritten, 'Description="assets\\Logo.png"', "substring in unrelated attr preserved")
	assert_false(FileAccess.file_exists(_project_path(_ROOT_LOGO)), "root logo removed")
	assert_true(FileAccess.file_exists(_project_path("storelogos").path_join(_ROOT_LOGO)), "logo moved into storelogos")


func test_pack_encrypt_key_without_key_returns_config_error() -> void:
	var service = PackagingServiceScript.new(FakeToolchain.new())
	var result: Dictionary = service.run_pack({
		"source_dir": "C:\\does-not-need-to-exist",
		"output_dir": "C:\\does-not-need-to-exist",
		"encrypt": "key",
		"encrypt_key": "",
	})

	assert_false(bool(result.get("ok", true)), "pack fails before producing an unencrypted package")
	assert_eq(int(result.get("exit_code", -1)), PackagingResult.EXIT_CONFIG, "missing key is a config error")
	assert_string_contains(str(result.get("message", "")), "--encrypt=key requires --encrypt-key", "error explains missing key")
	assert_push_error("--encrypt=key requires --encrypt-key", "push_error emitted for editor callers")


func _project_path(relative_path: String) -> String:
	return ProjectSettings.globalize_path("res://").path_join(relative_path)


func _write_text(path: String, text: String) -> void:
	var file: FileAccess = FileAccess.open(path, FileAccess.WRITE)
	assert_not_null(file, "fixture file opened for writing: %s" % path)
	if file == null:
		return
	file.store_string(text)
	file.close()


func _read_text(path: String) -> String:
	var file: FileAccess = FileAccess.open(path, FileAccess.READ)
	if file == null:
		return ""
	var text: String = file.get_as_text()
	file.close()
	return text


func _cleanup_project_artifacts() -> void:
	for path: String in [
		_project_path(_CONFIG_FILE),
		_project_path(_ROOT_LOGO),
		_project_path("storelogos").path_join(_ROOT_LOGO),
	]:
		if FileAccess.file_exists(path):
			DirAccess.remove_absolute(path)
	var storelogos_dir: String = _project_path("storelogos")
	if DirAccess.dir_exists_absolute(storelogos_dir):
		if DirAccess.get_files_at(storelogos_dir).is_empty() and DirAccess.get_directories_at(storelogos_dir).is_empty():
			DirAccess.remove_absolute(storelogos_dir)
