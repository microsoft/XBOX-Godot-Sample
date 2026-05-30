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


func test_relocate_keeps_config_referenced_logo_when_standard_logo_also_present() -> void:
	# Config references a custom logo filename (e.g. fhl_logo.png) for an attribute
	# while a standard GameConfigEditor-named logo (Square150x150Logo.png) also
	# happens to exist at the project root. Both files must be moved, and the
	# attribute rewrite must continue to point at the config-referenced custom
	# filename — not be overwritten by the standard-name entry.
	const _CUSTOM_LOGO := "fhl_logo.png"
	const _STANDARD_LOGO := "Square150x150Logo.png"
	# Back up the committed storelogos fixture so we can restore it after the test
	# overwrites it via relocate_logos_to_storelogos().
	var committed_storelogo_path: String = _project_path("storelogos").path_join(_STANDARD_LOGO)
	var committed_storelogo_bytes: PackedByteArray = PackedByteArray()
	if FileAccess.file_exists(committed_storelogo_path):
		var backup_file: FileAccess = FileAccess.open(committed_storelogo_path, FileAccess.READ)
		if backup_file != null:
			committed_storelogo_bytes = backup_file.get_buffer(int(backup_file.get_length()))
			backup_file.close()
	var config := ""
	config += '<?xml version="1.0" encoding="utf-8"?>\n'
	config += '<Game configVersion="1">\n'
	config += '  <ShellVisuals DefaultDisplayName="App"\n'
	config += '                Square150x150Logo="' + _CUSTOM_LOGO + '" />\n'
	config += '</Game>\n'
	_write_text(_project_path(_CONFIG_FILE), config)
	_write_text(_project_path(_CUSTOM_LOGO), "fake-custom-png")
	_write_text(_project_path(_STANDARD_LOGO), "fake-standard-png")

	var manager = GameConfigManagerScript.new(FakeToolchain.new())
	var moved: int = manager.relocate_logos_to_storelogos()

	assert_eq(moved, 2, "both root logos relocated")
	var rewritten: String = _read_text(_project_path(_CONFIG_FILE))
	assert_string_contains(rewritten, 'Square150x150Logo="storelogos\\' + _CUSTOM_LOGO + '"',
		"attribute keeps pointing at the config-referenced filename after the move")
	assert_eq(rewritten.find('Square150x150Logo="storelogos\\' + _STANDARD_LOGO + '"'), -1,
		"standard-name entry must not overwrite the config-referenced replacement")
	assert_true(FileAccess.file_exists(_project_path("storelogos").path_join(_CUSTOM_LOGO)),
		"custom logo moved into storelogos")
	assert_true(FileAccess.file_exists(_project_path("storelogos").path_join(_STANDARD_LOGO)),
		"standard logo moved into storelogos")
	# Cleanup the extra files this test created.
	for cleanup: String in [
		_project_path("storelogos").path_join(_CUSTOM_LOGO),
		_project_path(_CUSTOM_LOGO),
		_project_path(_STANDARD_LOGO),
	]:
		if FileAccess.file_exists(cleanup):
			DirAccess.remove_absolute(cleanup)
	# Restore the committed storelogos fixture if we shadowed it.
	if not committed_storelogo_bytes.is_empty():
		var restore_file: FileAccess = FileAccess.open(committed_storelogo_path, FileAccess.WRITE)
		if restore_file != null:
			restore_file.store_buffer(committed_storelogo_bytes)
			restore_file.close()
	elif FileAccess.file_exists(committed_storelogo_path):
		DirAccess.remove_absolute(committed_storelogo_path)


func test_rewrite_does_not_double_escape_xml_entities_in_logo_paths() -> void:
	# Logo filenames containing XML-significant characters (e.g. '&') appear in
	# MicrosoftGame.config as entity references ('&amp;'). The rewrite path
	# previously read the raw escaped text, re-escaped it during write-back, and
	# produced a doubly-escaped string ('&amp;amp;') that no longer refers to
	# the actual file. The reader now decodes entities so the round-trip stays
	# clean.
	const _ENTITY_LOGO := "Logo&Mark.png"
	const _ENTITY_LOGO_ESCAPED := "Logo&amp;Mark.png"
	var config := ""
	config += '<?xml version="1.0" encoding="utf-8"?>\n'
	config += '<Game configVersion="1">\n'
	config += '  <ShellVisuals DefaultDisplayName="App"\n'
	config += '                Square150x150Logo="' + _ENTITY_LOGO_ESCAPED + '" />\n'
	config += '</Game>\n'
	_write_text(_project_path(_CONFIG_FILE), config)
	_write_text(_project_path(_ENTITY_LOGO), "fake-png")

	var manager = GameConfigManagerScript.new(FakeToolchain.new())
	manager._rewrite_config_paths_to_storelogos()

	var rewritten: String = _read_text(_project_path(_CONFIG_FILE))
	assert_string_contains(rewritten, 'Square150x150Logo="storelogos\\' + _ENTITY_LOGO_ESCAPED + '"',
		"entity remains singly-escaped after rewrite")
	assert_eq(rewritten.find("&amp;amp;"), -1,
		"no double-escape introduced by the rewrite")
	# Cleanup.
	if FileAccess.file_exists(_project_path(_ENTITY_LOGO)):
		DirAccess.remove_absolute(_project_path(_ENTITY_LOGO))


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
