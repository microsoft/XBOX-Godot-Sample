extends GutTest

const PackagingResult = preload("res://addons/godot_gdk_packaging/core/packaging_result.gd")

const _FIXTURE_DIR := "user://test_headless_runner"


func before_each() -> void:
	_reset_fixture()
	DirAccess.make_dir_recursive_absolute(_fixture_root())


func after_each() -> void:
	_reset_fixture()


func test_no_json_suppresses_packaging_result_json_line() -> void:
	var project_dir: String = _build_fake_runner_project("no_json")
	var run := _run_child(project_dir, PackedStringArray([
		"sandbox",
		"--action", "set",
		"--sandbox-id", "0",
		"--no-json",
	]))

	assert_eq(run["exit_code"], 0, run["output"])
	assert_string_contains(run["output"], "[packaging] sandbox ok", "summary still prints")
	assert_eq(run["output"].find(PackagingResult.JSON_LINE_PREFIX), -1, "JSON marker is omitted under --no-json")


func test_runner_process_exit_code_matches_dispatched_result() -> void:
	var project_dir: String = _build_fake_runner_project("exit_code")
	var run := _run_child(project_dir, PackedStringArray([
		"sandbox",
		"--action", "set",
		"--sandbox-id", "7",
	]))
	var result: Dictionary = _result_from_output(run["output"])

	assert_eq(run["exit_code"], 7, "SceneTree.quit receives the verb exit_code")
	assert_eq(result.get("verb", ""), "sandbox")
	assert_eq(int(result.get("exit_code", -1)), 7)
	assert_false(result.get("ok", true), "non-zero fake verb result is reflected in JSON")


func test_runner_wires_parse_resolve_and_dispatch() -> void:
	var project_dir: String = _build_fake_runner_project("pipeline")
	var source_dir := "C:\\Content Dir With Spaces"
	var output_dir := "C:\\Output Dir With Spaces"
	var map_file := "C:\\Maps\\layout file.xml"
	var config_path := "C:\\Config Dir\\MicrosoftGame.config"
	var run := _run_child(project_dir, PackedStringArray([
		"pack",
		"--source-dir", source_dir,
		"--output-dir", output_dir,
		"--map-file", map_file,
		"--content-id", "D5-CONTENT",
		"--encrypt", "key",
		"--encrypt-key", "key with spaces.ekb",
		"--updcompat", "2",
		"--no-prepare",
		"--config", config_path,
	]))
	var result: Dictionary = _result_from_output(run["output"])
	var resolved: Dictionary = result.get("details", {}).get("resolved", {})

	assert_eq(run["exit_code"], 0, run["output"])
	assert_eq(result.get("verb", ""), "pack")
	assert_eq(resolved.get("resolved_marker", ""), "fake resolver saw parsed options")
	assert_eq(resolved.get("source_dir", ""), source_dir)
	assert_eq(resolved.get("output_dir", ""), output_dir)
	assert_eq(resolved.get("map_file", ""), map_file)
	assert_eq(resolved.get("content_id", ""), "D5-CONTENT")
	assert_eq(resolved.get("encrypt", ""), "key")
	assert_eq(resolved.get("encrypt_key", ""), "key with spaces.ekb")
	assert_eq(int(resolved.get("updcompat", -1)), 2)
	assert_true(bool(resolved.get("no_prepare", false)), "bool flag survives parser + resolver")
	assert_eq(resolved.get("config_path_override_arg", ""), config_path)
	assert_eq(resolved.get("settings_path_arg", ""), "res://.gdk_packaging.cfg")


func _build_fake_runner_project(name: String) -> String:
	var root: String = _fixture_path(name)
	var addon_dir: String = root.path_join("addons").path_join("godot_gdk_packaging")
	var core_dir: String = addon_dir.path_join("core")
	DirAccess.make_dir_recursive_absolute(core_dir)
	_write_text(root.path_join("project.godot"), "[application]\nconfig/name=\"Headless Runner Test\"\n")
	_copy_text("res://addons/godot_gdk_packaging/run.gd", addon_dir.path_join("run.gd"))
	_copy_text("res://addons/godot_gdk_packaging/core/packaging_cli.gd", core_dir.path_join("packaging_cli.gd"))
	_copy_text("res://addons/godot_gdk_packaging/core/packaging_result.gd", core_dir.path_join("packaging_result.gd"))
	_write_text(core_dir.path_join("packaging_config.gd"), _fake_packaging_config_source())
	_write_text(core_dir.path_join("packaging_service.gd"), _fake_packaging_service_source())
	return root


func _run_child(project_dir: String, user_args: PackedStringArray) -> Dictionary:
	var args := PackedStringArray([
		"--headless",
		"--path", project_dir,
		"-s", "res://addons/godot_gdk_packaging/run.gd",
		"--",
	])
	args.append_array(user_args)
	var output: Array = []
	var exit_code: int = OS.execute(OS.get_executable_path(), args, output, true, false)
	return {"exit_code": exit_code, "output": "\n".join(output)}


func _result_from_output(output: String) -> Dictionary:
	for line: String in output.split("\n"):
		var parsed: Dictionary = PackagingResult.from_json_line(line)
		if not parsed.is_empty():
			return parsed
	return {}


func _fake_packaging_config_source() -> String:
	return """
@tool
extends RefCounted
const PACKAGING_SETTINGS_PATH := "res://.gdk_packaging.cfg"
const _CLI_KEY_REMAP := {
	"source-dir": "source_dir",
	"map-file": "map_file",
	"output-dir": "output_dir",
	"content-id": "content_id",
	"product-id": "product_id",
	"encrypt-key": "encrypt_key",
	"no-prepare": "no_prepare",
	"sandbox-id": "sandbox_id",
	"config": "config_path",
}
static func resolve(cli_options: Dictionary, project_root: String = "", settings_path: String = PACKAGING_SETTINGS_PATH, config_path_override: String = "") -> Dictionary:
	var resolved: Dictionary = {"resolved_marker": "fake resolver saw parsed options"}
	for cli_key: String in cli_options:
		var resolved_key: String = _CLI_KEY_REMAP.get(cli_key, cli_key.replace("-", "_"))
		resolved[resolved_key] = cli_options[cli_key]
	resolved["project_root_arg"] = project_root
	resolved["settings_path_arg"] = settings_path
	resolved["config_path_override_arg"] = config_path_override
	return resolved
"""


func _fake_packaging_service_source() -> String:
	return """
@tool
extends RefCounted
const PackagingResult = preload("res://addons/godot_gdk_packaging/core/packaging_result.gd")
func dispatch(verb: String, resolved: Dictionary) -> Dictionary:
	var exit_code := 0
	if resolved.has("sandbox_id"):
		exit_code = int(resolved["sandbox_id"])
	return PackagingResult.make(verb, exit_code, "fake dispatched " + verb, {"resolved": resolved}, "fake stdout", "fake stderr" if exit_code != 0 else "", 0)
"""


func _copy_text(source_res_path: String, destination_path: String) -> void:
	var source_path: String = ProjectSettings.globalize_path(source_res_path)
	var file := FileAccess.open(source_path, FileAccess.READ)
	assert_not_null(file, "opened %s" % source_path)
	if file == null:
		return
	var text: String = file.get_as_text()
	file.close()
	_write_text(destination_path, text)


func _write_text(path: String, content: String) -> void:
	DirAccess.make_dir_recursive_absolute(path.get_base_dir())
	var file := FileAccess.open(path, FileAccess.WRITE)
	assert_not_null(file, "opened %s" % path)
	if file == null:
		return
	file.store_string(content)
	file.close()


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
