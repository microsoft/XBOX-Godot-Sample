extends GutTest

const TEMP_ROOT := "res://.tmp_gdkpkg_forwarder_tests"


func after_each() -> void:
	_remove_recursive(ProjectSettings.globalize_path(TEMP_ROOT))


func test_posix_forwarder_uses_arrays_without_eval() -> void:
	var source: String = _read_text(ProjectSettings.globalize_path("res://addons/godot_gdk_packaging/gdkpkg.sh"))

	assert_false(source.contains("eval"), "gdkpkg.sh must not reintroduce eval-based env lookup or argv forwarding")
	assert_string_contains(source, "forward_args+=(\"$1\")")
	assert_string_contains(source, "\"${forward_args[@]}\"")


func test_windows_forwarder_discovery_order_is_env_sample_cwd_then_path() -> void:
	var source: String = _read_text(ProjectSettings.globalize_path("res://addons/godot_gdk_packaging/gdkpkg.cmd"))
	var env_index: int = source.find("if defined GODOT_CONSOLE")
	var sample_index: int = source.find("%SCRIPT_DIR%..\\..\\sample")
	var cwd_index: int = source.find("\"%CD%\") do")
	var path_index: int = source.find("where %%C")

	assert_gt(env_index, -1, "env var discovery is present")
	assert_gt(sample_index, env_index, "repo sample probe runs after env vars")
	assert_gt(cwd_index, sample_index, "CWD Godot probe runs after sample probe")
	assert_gt(path_index, cwd_index, "PATH lookup runs last")
	assert_string_contains(source, "set \"PROJECT_PATH=%CD%\"", "forwarder defaults --path to the caller CWD")


func test_windows_forwarder_prefers_env_over_path_and_preserves_special_args() -> void:
	if OS.get_name() != "Windows":
		pending("gdkpkg.cmd discovery is Windows-specific")
		return

	var root: String = _make_temp_root("env over path")
	var forwarder_path: String = root.path_join("gdkpkg.cmd")
	var env_fake_path: String = root.path_join("Env Godot Console.cmd")
	var path_dir: String = root.path_join("path bin")
	var path_fake_path: String = path_dir.path_join("godot.cmd")
	var runner_path: String = root.path_join("run env forwarder.cmd")
	var env_capture_path: String = root.path_join("env captured args.txt")
	var path_capture_path: String = root.path_join("path captured args.txt")
	var project_path: String = root.path_join("Project With Space")
	var source_dir: String = root.path_join("Source Dir With Space")
	var output_dir: String = root.path_join("Output Dir With Space")
	DirAccess.make_dir_recursive_absolute(project_path)
	DirAccess.make_dir_recursive_absolute(source_dir)
	DirAccess.make_dir_recursive_absolute(output_dir)
	DirAccess.make_dir_recursive_absolute(path_dir)
	_copy_file(ProjectSettings.globalize_path("res://addons/godot_gdk_packaging/gdkpkg.cmd"), forwarder_path)
	_write_file(env_fake_path, _fake_godot_script(env_capture_path))
	_write_file(path_fake_path, _fake_godot_script(path_capture_path))
	_write_file(runner_path, _env_runner_script(forwarder_path, env_fake_path, path_dir, project_path, source_dir, output_dir))

	var output: Array = []
	var exit_code: int = OS.execute("cmd.exe", PackedStringArray(["/D", "/V:OFF", "/C", runner_path]), output, true, false)

	assert_eq(exit_code, 0, "forwarder output: %s" % "\n".join(output))
	assert_true(FileAccess.file_exists(env_capture_path), "GODOT_CONSOLE candidate was executed")
	assert_false(FileAccess.file_exists(path_capture_path), "PATH candidate was not used while env var was set")
	assert_eq(_read_lines(env_capture_path), [
		"--headless",
		"--path",
		project_path,
		"-s",
		"res://addons/godot_gdk_packaging/run.gd",
		"--",
		"pack",
		"--source-dir",
		source_dir,
		"--output-dir",
		output_dir,
		"--encrypt-key",
		"key with spaces = symbols",
		"--no-prepare",
	])


func test_windows_forwarder_preserves_args_with_spaces_end_to_end() -> void:
	if OS.get_name() != "Windows":
		pending("gdkpkg.cmd tokenization is Windows-specific")
		return

	var root: String = _make_temp_root("with space")
	var forwarder_path: String = root.path_join("gdkpkg.cmd")
	var fake_godot_path: String = root.path_join("Fake Godot Console.cmd")
	var runner_path: String = root.path_join("run forwarder.cmd")
	var captured_args_path: String = root.path_join("captured args.txt")
	var project_path: String = root.path_join("Project With Space")
	var source_dir: String = root.path_join("Source Dir With Space")
	var output_dir: String = root.path_join("Output Dir With Space")
	DirAccess.make_dir_recursive_absolute(project_path)
	DirAccess.make_dir_recursive_absolute(source_dir)
	DirAccess.make_dir_recursive_absolute(output_dir)
	_copy_file(ProjectSettings.globalize_path("res://addons/godot_gdk_packaging/gdkpkg.cmd"), forwarder_path)
	_write_file(fake_godot_path, _fake_godot_script(captured_args_path))
	_write_file(runner_path, _runner_script(forwarder_path, fake_godot_path, project_path, source_dir, output_dir))

	var output: Array = []
	var exit_code: int = OS.execute("cmd.exe", PackedStringArray(["/D", "/C", runner_path]), output, true, false)

	assert_eq(exit_code, 0, "forwarder output: %s" % "\n".join(output))
	assert_true(FileAccess.file_exists(captured_args_path), "fake Godot captured the forwarded argv")
	assert_eq(_read_lines(captured_args_path), [
		"--headless",
		"--path",
		project_path,
		"-s",
		"res://addons/godot_gdk_packaging/run.gd",
		"--",
		"pack",
		"--source-dir",
		source_dir,
		"--output-dir",
		output_dir,
		"--no-prepare",
	])


func _fake_godot_script(captured_args_path: String) -> String:
	return """@echo off
break > "%s"
:loop
if "%%~1"=="" exit /b 0
>>"%s" echo(%%~1
shift
goto loop
""" % [captured_args_path, captured_args_path]


func _runner_script(forwarder_path: String, fake_godot_path: String, project_path: String, source_dir: String, output_dir: String) -> String:
	return """@echo off
call "%s" --godot "%s" --path "%s" pack --source-dir "%s" --output-dir "%s" --no-prepare
exit /b %%ERRORLEVEL%%
""" % [forwarder_path, fake_godot_path, project_path, source_dir, output_dir]


func _env_runner_script(forwarder_path: String, env_fake_path: String, path_dir: String, project_path: String, source_dir: String, output_dir: String) -> String:
	return """@echo off
setlocal EnableExtensions DisableDelayedExpansion
set "GODOT_CONSOLE=%s"
set "PATH=%s;%%PATH%%"
call "%s" --path "%s" pack --source-dir "%s" --output-dir "%s" --encrypt-key "key with spaces = symbols" --no-prepare
exit /b %%ERRORLEVEL%%
""" % [env_fake_path, path_dir, forwarder_path, project_path, source_dir, output_dir]


func _make_temp_root(name: String) -> String:
	var root: String = ProjectSettings.globalize_path(TEMP_ROOT.path_join(name))
	_remove_recursive(root)
	DirAccess.make_dir_recursive_absolute(root)
	return root


func _copy_file(source_path: String, destination_path: String) -> void:
	var source := FileAccess.open(source_path, FileAccess.READ)
	assert_not_null(source, "opened %s" % source_path)
	if source == null:
		return
	var destination := FileAccess.open(destination_path, FileAccess.WRITE)
	assert_not_null(destination, "opened %s" % destination_path)
	if destination == null:
		return
	destination.store_buffer(source.get_buffer(source.get_length()))
	source.close()
	destination.close()


func _write_file(path: String, contents: String) -> void:
	DirAccess.make_dir_recursive_absolute(path.get_base_dir())
	var file := FileAccess.open(path, FileAccess.WRITE)
	assert_not_null(file, "opened %s" % path)
	if file == null:
		return
	file.store_string(contents)
	file.close()


func _read_lines(path: String) -> Array:
	var text: String = _read_text(path).replace("\r\n", "\n").strip_edges()
	if text.is_empty():
		return []
	return Array(text.split("\n", false))


func _read_text(path: String) -> String:
	var file := FileAccess.open(path, FileAccess.READ)
	assert_not_null(file, "opened %s" % path)
	if file == null:
		return ""
	var text: String = file.get_as_text()
	file.close()
	return text


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
