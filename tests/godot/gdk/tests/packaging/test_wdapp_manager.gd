extends GutTest

const WdappManager = preload("res://addons/godot_gdk_packaging/core/wdapp_manager.gd")
const TEMP_ROOT := "res://.tmp_wdapp_manager_tests"

class FakeToolchain:
	extends RefCounted

	var bin_dir: String

	func _init(p_bin_dir: String) -> void:
		bin_dir = p_bin_dir

	func get_bin_dir() -> String:
		return bin_dir

	func execute_tool(_tool_path: String, _args: PackedStringArray) -> Dictionary:
		return {"exit_code": 99, "stdout": "wdapp failed", "stderr": "simulated failure"}


var _taskkill_images: Array[String] = []


func before_each() -> void:
	_taskkill_images.clear()


func after_each() -> void:
	_remove_recursive(ProjectSettings.globalize_path(TEMP_ROOT))


func test_taskkill_uses_microsoft_game_config_executable_and_ignores_extra_exes() -> void:
	var root: String = _make_temp_root("configured_executable")
	var bin_dir: String = root.path_join("gdk bin")
	var build_dir: String = root.path_join("build dir")
	DirAccess.make_dir_recursive_absolute(bin_dir)
	DirAccess.make_dir_recursive_absolute(build_dir)
	_write_file(bin_dir.path_join("wdapp.exe"), "")
	_write_config(build_dir.path_join("MicrosoftGame.config"), "Configured Game.exe")
	_write_file(build_dir.path_join("code.exe"), "attacker-controlled extra exe")
	_write_file(build_dir.path_join("Configured Game.exe"), "configured game exe")

	var manager: RefCounted = WdappManager.new(FakeToolchain.new(bin_dir), Callable(self, "_capture_taskkill"))
	var result: Dictionary = manager.terminate_app("Publisher.Game_1.0.0.0_x64__abc123", build_dir)

	assert_eq(result.get("exit_code", -1), 0)
	assert_eq(result.get("terminated_with", ""), "taskkill")
	assert_eq(_taskkill_images.size(), 1)
	assert_eq(_taskkill_images[0], "Configured Game.exe")


func test_taskkill_is_skipped_when_configured_executable_is_missing() -> void:
	var root: String = _make_temp_root("missing_configured_executable")
	var bin_dir: String = root.path_join("gdk bin")
	var build_dir: String = root.path_join("build dir")
	DirAccess.make_dir_recursive_absolute(bin_dir)
	DirAccess.make_dir_recursive_absolute(build_dir)
	_write_file(bin_dir.path_join("wdapp.exe"), "")
	_write_config(build_dir.path_join("MicrosoftGame.config"), "Configured Game.exe")
	_write_file(build_dir.path_join("code.exe"), "attacker-controlled extra exe")

	var manager: RefCounted = WdappManager.new(FakeToolchain.new(bin_dir), Callable(self, "_capture_taskkill"))
	var result: Dictionary = manager.terminate_app("Publisher.Game_1.0.0.0_x64__abc123", build_dir)

	assert_eq(result.get("exit_code", -1), 99)
	assert_eq(result.get("terminated_with", ""), "wdapp")
	assert_eq(result.get("taskkill_skipped_reason", ""), "no configured executable available")
	assert_eq(_taskkill_images.size(), 0)


func test_taskkill_rejects_non_bare_config_executable_names() -> void:
	var index := 0
	for invalid_name: String in ["*.exe", "subdir/Game.exe", "subdir\\Game.exe", "Game", "Bad:Game.exe", "&quot;Game.exe&quot;"]:
		var root: String = _make_temp_root("invalid_%d" % index)
		index += 1
		var build_dir: String = root.path_join("build dir")
		DirAccess.make_dir_recursive_absolute(build_dir)
		DirAccess.make_dir_recursive_absolute(build_dir.path_join("subdir"))
		_write_config(build_dir.path_join("MicrosoftGame.config"), invalid_name)
		_write_file(build_dir.path_join("Game.exe"), "extra bare exe")
		_write_file(build_dir.path_join("subdir").path_join("Game.exe"), "nested exe")

		assert_eq(WdappManager._resolve_taskkill_executable(build_dir), "", "rejected %s" % invalid_name)


func _capture_taskkill(exe_name: String) -> Dictionary:
	_taskkill_images.append(exe_name)
	return {"exit_code": 0, "stdout": exe_name, "stderr": "", "terminated_with": "taskkill"}


func _write_config(path: String, executable_name: String) -> void:
	_write_file(path, """
<?xml version="1.0" encoding="utf-8"?>
<Game>
  <ExecutableList>
    <Executable Name="%s" Id="Game" />
  </ExecutableList>
</Game>
""" % executable_name)


func _make_temp_root(name: String) -> String:
	var root: String = ProjectSettings.globalize_path(TEMP_ROOT.path_join(name))
	_remove_recursive(root)
	DirAccess.make_dir_recursive_absolute(root)
	return root


func _write_file(path: String, contents: String) -> void:
	DirAccess.make_dir_recursive_absolute(path.get_base_dir())
	var file := FileAccess.open(path, FileAccess.WRITE)
	assert_not_null(file, "opened %s" % path)
	if file == null:
		return
	file.store_string(contents)
	file.close()


func _remove_recursive(path: String) -> void:
	if FileAccess.file_exists(path):
		DirAccess.remove_absolute(path)
		return
	if not DirAccess.dir_exists_absolute(path):
		return
	var dir := DirAccess.open(path)
	if dir == null:
		return
	dir.list_dir_begin()
	var entry: String = dir.get_next()
	while entry != "":
		if entry != "." and entry != "..":
			var child: String = path.path_join(entry)
			if dir.current_is_dir():
				_remove_recursive(child)
			else:
				DirAccess.remove_absolute(child)
		entry = dir.get_next()
	dir.list_dir_end()
	DirAccess.remove_absolute(path)
