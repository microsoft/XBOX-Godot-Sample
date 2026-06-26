extends GutTest
## Logo reconciliation coverage for MicrosoftGame.config packaging helpers, plus
## an encryption-key safety pin. The addon keeps every logo at the project root
## (where GameConfigEditor writes picked tiles) and regenerates the derived size
## variants in place from the 480x480 master — it does not move logos into a
## storelogos/ subfolder or rewrite the config's logo paths (issue #62).

const GameConfigManagerScript = preload("res://addons/godot_gdk_editortools/core/game_config_manager.gd")
const EditorToolsResult = preload("res://addons/godot_gdk_editortools/core/editortools_result.gd")
const EditorToolsServiceScript = preload("res://addons/godot_gdk_editortools/core/editortools_service.gd")

const _CONFIG_FILE := "MicrosoftGame.config"
const _MASTER := "Square480x480Logo.png"
const _LOGO_FILES := [
	"Square480x480Logo.png",
	"Square150x150Logo.png",
	"Square44x44Logo.png",
	"StoreLogo.png",
	"SplashScreenImage.png",
	"MyNewTile.png",
	"custom150.png",
]


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


func test_pack_encrypt_key_without_key_returns_config_error() -> void:
	var service = EditorToolsServiceScript.new(FakeToolchain.new())
	var result: Dictionary = service.run_pack({
		"source_dir": "C:\\does-not-need-to-exist",
		"output_dir": "C:\\does-not-need-to-exist",
		"encrypt": "key",
		"encrypt_key": "",
	})

	assert_false(bool(result.get("ok", true)), "pack fails before producing an unencrypted package")
	assert_eq(int(result.get("exit_code", -1)), EditorToolsResult.EXIT_CONFIG, "missing key is a config error")
	assert_string_contains(str(result.get("message", "")), "--encrypt=key requires --encrypt-key", "error explains missing key")
	assert_push_error("--encrypt=key requires --encrypt-key", "push_error emitted for editor callers")


func test_reconcile_regenerates_stale_variants_in_place_without_moving_or_rewriting() -> void:
	# Simulates issue #62: GameConfigEditor wrote a freshly-picked 480x480 master
	# (a custom filename) to the project root and pointed Square480x480Logo at it,
	# but left the derived size variants stale. reconcile_logos_after_edit() must
	# regenerate the four derived variants in place from that master, at the paths
	# the config declares — WITHOUT moving anything into storelogos/ or rewriting
	# the config's logo paths (which is what produced duplicates before).
	const _PICKED := "MyNewTile.png"
	var config := ""
	config += '<?xml version="1.0" encoding="utf-8"?>\n'
	config += '<Game configVersion="1">\n'
	config += '  <ShellVisuals DefaultDisplayName="App"\n'
	config += '                StoreLogo="StoreLogo.png"\n'
	config += '                Square44x44Logo="Square44x44Logo.png"\n'
	config += '                Square150x150Logo="Square150x150Logo.png"\n'
	config += '                Square480x480Logo="' + _PICKED + '"\n'
	config += '                SplashScreenImage="SplashScreenImage.png" />\n'
	config += '</Game>\n'
	_write_text(_project_path(_CONFIG_FILE), config)

	# The freshly-picked master image GameConfigEditor dropped at the root.
	_write_master_png(_project_path(_PICKED))
	# Stale 1x1 derived variants left over from a previous tile.
	for stale: String in ["StoreLogo.png", "Square44x44Logo.png", "Square150x150Logo.png", "SplashScreenImage.png"]:
		_write_png(_project_path(stale), 1, 1)

	var manager = GameConfigManagerScript.new(FakeToolchain.new())
	var summary: Dictionary = manager.reconcile_logos_after_edit()

	assert_eq(int(summary.get("synced", 0)), 4, "four derived variants regenerated (the 480 master is the source)")

	# Each derived variant exists at its declared root path with the right size.
	assert_eq(_png_size(_project_path("StoreLogo.png")), Vector2i(100, 100), "StoreLogo regenerated at 100x100")
	assert_eq(_png_size(_project_path("Square44x44Logo.png")), Vector2i(44, 44), "44 variant regenerated")
	assert_eq(_png_size(_project_path("Square150x150Logo.png")), Vector2i(150, 150), "150 variant regenerated")
	assert_eq(_png_size(_project_path("SplashScreenImage.png")), Vector2i(1920, 1080), "splash regenerated")

	# The picked master is untouched (not moved, not resized).
	assert_true(FileAccess.file_exists(_project_path(_PICKED)), "picked master still at root")
	assert_eq(_png_size(_project_path(_PICKED)), Vector2i(480, 480), "picked master unchanged")

	# Nothing is relocated into a storelogos/ subfolder and the config is
	# left as-is — this is what produced duplicates before.
	assert_false(FileAccess.file_exists(_project_path("storelogos").path_join(_PICKED)),
		"picked master is not relocated into storelogos/")
	var after_config: String = _read_text(_project_path(_CONFIG_FILE))
	assert_string_contains(after_config, 'Square480x480Logo="' + _PICKED + '"', "config still references the picked master, unrewritten")


func test_sync_writes_variant_to_config_declared_custom_path() -> void:
	# When the config declares a non-standard filename for a variant, sync must
	# regenerate the file at THAT path (so packaging staging finds a fresh file),
	# not at a hardcoded standard name.
	const _CUSTOM := "custom150.png"
	var config := ""
	config += '<?xml version="1.0" encoding="utf-8"?>\n'
	config += '<Game configVersion="1">\n'
	config += '  <ShellVisuals DefaultDisplayName="App"\n'
	config += '                Square480x480Logo="Square480x480Logo.png"\n'
	config += '                Square150x150Logo="' + _CUSTOM + '" />\n'
	config += '</Game>\n'
	_write_text(_project_path(_CONFIG_FILE), config)
	_write_master_png(_project_path(_MASTER))

	var manager = GameConfigManagerScript.new(FakeToolchain.new())
	var synced: int = manager.sync_store_logos()

	assert_eq(synced, 4, "all four derived variants regenerated")
	assert_true(FileAccess.file_exists(_project_path(_CUSTOM)), "custom-named variant written at its declared path")
	assert_eq(_png_size(_project_path(_CUSTOM)), Vector2i(150, 150), "custom variant has the 150 size")


func test_sync_no_op_without_a_master() -> void:
	# A config whose Square480x480Logo points at a missing file must not crash and
	# must report zero regenerations.
	var config := ""
	config += '<?xml version="1.0" encoding="utf-8"?>\n'
	config += '<Game configVersion="1">\n'
	config += '  <ShellVisuals DefaultDisplayName="App"\n'
	config += '                Square480x480Logo="Square480x480Logo.png" />\n'
	config += '</Game>\n'
	_write_text(_project_path(_CONFIG_FILE), config)

	var manager = GameConfigManagerScript.new(FakeToolchain.new())
	assert_eq(manager.sync_store_logos(), 0, "no master on disk -> nothing regenerated")


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


func _write_png(path: String, width: int, height: int) -> void:
	var img: Image = Image.create(width, height, false, Image.FORMAT_RGBA8)
	img.fill(Color(0.1, 0.6, 0.1))
	assert_eq(img.save_png(path), OK, "fixture PNG written: %s" % path)


func _write_master_png(path: String) -> void:
	_write_png(path, 480, 480)


func _png_size(path: String) -> Vector2i:
	var img: Image = Image.new()
	if img.load(path) != OK:
		return Vector2i(-1, -1)
	return Vector2i(img.get_width(), img.get_height())


func _cleanup_project_artifacts() -> void:
	if FileAccess.file_exists(_project_path(_CONFIG_FILE)):
		DirAccess.remove_absolute(_project_path(_CONFIG_FILE))
	for name: String in _LOGO_FILES:
		var p: String = _project_path(name)
		if FileAccess.file_exists(p):
			DirAccess.remove_absolute(p)
	var storelogos_dir: String = _project_path("storelogos")
	if DirAccess.dir_exists_absolute(storelogos_dir):
		if DirAccess.get_files_at(storelogos_dir).is_empty() and DirAccess.get_directories_at(storelogos_dir).is_empty():
			DirAccess.remove_absolute(storelogos_dir)
