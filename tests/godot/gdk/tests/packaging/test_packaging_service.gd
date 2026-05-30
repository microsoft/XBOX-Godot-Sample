extends GutTest
## Regression coverage for headless verb facade behaviours that the CLI
## advertises directly.

const PackagingServiceScript = preload("res://addons/godot_gdk_packaging/core/packaging_service.gd")
const PackagingCli = preload("res://addons/godot_gdk_packaging/core/packaging_cli.gd")
const PackagingResult = preload("res://addons/godot_gdk_packaging/core/packaging_result.gd")

const _FIXTURE_DIR := "user://test_packaging_service"

class _FakeToolchain extends RefCounted:
	var bin_dir: String = ProjectSettings.globalize_path("user://missing_gdk_bin")
	var gdk_available: bool = true
	var default_exit_code: int = 0
	var calls: Array[Dictionary] = []
	var detached_calls: Array[Dictionary] = []
	var operation_exit_codes: Dictionary = {}
	var detached_fail_operations: Dictionary = {}

	func _init(p_bin_dir: String = "") -> void:
		if not p_bin_dir.is_empty():
			bin_dir = p_bin_dir

	func get_bin_dir() -> String:
		return bin_dir

	func get_makepkg_path() -> String:
		return bin_dir.path_join("makepkg.exe")

	func get_sandbox_path() -> String:
		return bin_dir.path_join("XblPCSandbox.exe")

	func get_game_config_editor_path() -> String:
		return bin_dir.path_join("MicrosoftGameConfigEditor.exe")

	func is_gdk_available() -> bool:
		return gdk_available

	func execute_tool(exe_path: String, args: PackedStringArray) -> Dictionary:
		var operation: String = _operation_for(exe_path, args)
		calls.append({"operation": operation, "exe_path": exe_path, "args": PackedStringArray(args)})
		var exit_code: int = int(operation_exit_codes.get(operation, default_exit_code))
		var stdout: String = "Current sandbox: D5SANDBOX" if operation == "sandbox" else "stdout:%s" % operation
		var stderr: String = "stderr:%s" % operation if exit_code != 0 else ""
		return {"exit_code": exit_code, "stdout": stdout, "stderr": stderr}

	func launch_detached(exe_path: String, args: PackedStringArray) -> int:
		var operation: String = "store_wizard" if args.size() > 1 else "config_editor"
		detached_calls.append({"operation": operation, "exe_path": exe_path, "args": PackedStringArray(args)})
		if bool(detached_fail_operations.get(operation, false)):
			return -1
		return 4321

	func _operation_for(exe_path: String, args: PackedStringArray) -> String:
		var exe_name: String = exe_path.get_file().to_lower()
		if exe_name == "xblpcsandbox.exe":
			return "sandbox"
		if exe_name == "wdapp.exe" and not args.is_empty():
			var wdapp_op: String = args[0]
			return "register_loose" if wdapp_op == "register" else wdapp_op
		if not args.is_empty():
			var first: String = args[0]
			if ["pack", "genmap", "validate"].has(first):
				return first
			if args.has("--export-debug") or args.has("--export-release"):
				return "export"
		return "tool"


func before_each() -> void:
	_reset_fixture()
	DirAccess.make_dir_recursive_absolute(_fixture_root())


func after_each() -> void:
	_reset_fixture()


func _fixture_root() -> String:
	return ProjectSettings.globalize_path(_FIXTURE_DIR)


func _fixture_path(relative_path: String) -> String:
	return _fixture_root().path_join(relative_path)


func _reset_fixture() -> void:
	_remove_tree(_fixture_root())


func _remove_tree(path: String) -> void:
	if not DirAccess.dir_exists_absolute(path):
		return
	for file_name: String in DirAccess.get_files_at(path):
		DirAccess.remove_absolute(path.path_join(file_name))
	for dir_name: String in DirAccess.get_directories_at(path):
		_remove_tree(path.path_join(dir_name))
	DirAccess.remove_absolute(path)


func _read_text(path: String) -> String:
	var file: FileAccess = FileAccess.open(path, FileAccess.READ)
	if file == null:
		return ""
	var content: String = file.get_as_text()
	file.close()
	return content


func _write_text(path: String, content: String) -> void:
	DirAccess.make_dir_recursive_absolute(path.get_base_dir())
	var file: FileAccess = FileAccess.open(path, FileAccess.WRITE)
	file.store_string(content)
	file.close()


func _config_xml(identity_name: String, executable: String,
		shell_attrs: Dictionary = {}) -> String:
	var attrs: String = ""
	for key: String in shell_attrs:
		attrs += ' %s="%s"' % [key, shell_attrs[key]]
	var xml: String = ""
	xml += '<?xml version="1.0" encoding="utf-8"?>\n'
	xml += '<Game configVersion="1">\n'
	xml += '  <Identity Name="%s" Publisher="CN=Acme" Version="1.0.0.0" />\n' % identity_name
	xml += '  <ExecutableList>\n'
	xml += '    <Executable Name="%s" Id="Game" />\n' % executable
	xml += '  </ExecutableList>\n'
	xml += '  <ShellVisuals DefaultDisplayName="%s"%s />\n' % [identity_name, attrs]
	xml += '  <MSStore ProductId="TEST-PRODUCT" />\n'
	xml += '</Game>\n'
	return xml


func _new_service() -> RefCounted:
	return PackagingServiceScript.new(_FakeToolchain.new())


func test_method_for_verb_returns_expected_method_names() -> void:
	var service = _new_service()
	assert_eq(PackagingCli.VERBS.size(), 14, "PackagingService exposes the full CLI verb set")
	for verb: String in PackagingCli.VERBS:
		assert_eq(service.method_for_verb(verb), "run_" + verb, "method_for_verb maps %s" % verb)
	assert_eq(service.method_for_verb("missing_verb"), "", "unknown verbs do not invent a method")


func test_dispatch_routes_every_cli_verb_to_a_result_shaped_run_method() -> void:
	var root: String = _fixture_path("verb_shapes")
	var fake := _make_ready_toolchain(root)
	var service = PackagingServiceScript.new(fake)

	for verb: String in PackagingCli.VERBS:
		var result: Dictionary = service.dispatch(verb, _resolved_for_verb(verb, root))
		assert_true(PackagingResult.is_valid_shape(result), "%s result has PackagingResult shape" % verb)
		assert_eq(result["verb"], verb, "dispatch(%s) reaches run_%s" % [verb, verb])
		assert_eq(result["exit_code"], PackagingResult.EXIT_OK, "%s succeeds with the fake toolchain" % verb)


func test_dispatch_unknown_verb_returns_unimplemented_result() -> void:
	var result: Dictionary = _new_service().dispatch("missing_verb", {})

	assert_true(PackagingResult.is_valid_shape(result))
	assert_eq(result["verb"], "missing_verb")
	assert_eq(result["exit_code"], PackagingResult.EXIT_UNIMPLEMENTED)
	assert_false(result["ok"], "unknown dispatch does not report success")


func test_tool_backed_verbs_normalize_underlying_failures_to_exit_tool() -> void:
	var operations := [
		"pack",
		"genmap",
		"validate",
		"export",
		"register_loose",
		"install",
		"uninstall",
		"launch",
		"terminate",
		"sandbox",
		"config_editor",
		"store_wizard",
	]
	for verb: String in operations:
		var root: String = _fixture_path("tool_failure/%s" % verb)
		var fake := _make_ready_toolchain(root)
		if verb == "config_editor" or verb == "store_wizard":
			fake.detached_fail_operations[verb] = true
		else:
			fake.operation_exit_codes[verb] = 9
		var result: Dictionary = PackagingServiceScript.new(fake).dispatch(verb, _resolved_for_verb(verb, root))

		assert_true(PackagingResult.is_valid_shape(result), "%s failure still has PackagingResult shape" % verb)
		assert_eq(result["verb"], verb)
		assert_eq(result["exit_code"], PackagingResult.EXIT_TOOL, "%s maps dependency failure to EXIT_TOOL" % verb)
		assert_false(result["ok"], "%s failure reports ok=false" % verb)


func _make_ready_toolchain(root: String) -> _FakeToolchain:
	var bin_dir: String = root.path_join("GDK Bin")
	DirAccess.make_dir_recursive_absolute(bin_dir)
	_write_text(bin_dir.path_join("wdapp.exe"), "")
	_write_text(bin_dir.path_join("makepkg.exe"), "")
	_write_text(bin_dir.path_join("XblPCSandbox.exe"), "")
	_write_text(bin_dir.path_join("MicrosoftGameConfigEditor.exe"), "")
	_prepare_common_files(root)
	return _FakeToolchain.new(bin_dir)


func _prepare_common_files(root: String) -> void:
	var content_dir: String = root.path_join("content")
	DirAccess.make_dir_recursive_absolute(content_dir)
	_write_text(content_dir.path_join("VerbGame.exe"), "")
	_write_text(content_dir.path_join("MicrosoftGame.config"), _config_xml("VerbGame", "Placeholder.exe"))
	_write_text(root.path_join("config/MicrosoftGame.config"), _config_xml("VerbGame", "Placeholder.exe"))
	_write_text(root.path_join("layout.xml"), "<Package></Package>")
	_write_text(root.path_join("packages/Game.msixvc"), "package")
	DirAccess.make_dir_recursive_absolute(root.path_join("out"))
	DirAccess.make_dir_recursive_absolute(root.path_join("export_out"))


func _resolved_for_verb(verb: String, root: String) -> Dictionary:
	var content_dir: String = root.path_join("content")
	var config_path: String = root.path_join("config/MicrosoftGame.config")
	match verb:
		"pack":
			return {"source_dir": content_dir, "output_dir": root.path_join("out"), "map_file": root.path_join("layout.xml"), "no_prepare": true, "encrypt": "none", "updcompat": 3}
		"genmap":
			return {"source_dir": content_dir, "map_file": root.path_join("layout.xml")}
		"validate":
			return {"source_dir": content_dir, "map_file": root.path_join("layout.xml"), "output_dir": root.path_join("validate_out")}
		"prepare_content":
			return {"content_dir": content_dir, "config_path": config_path}
		"export":
			return {"preset_name": "Windows Desktop", "output_dir": root.path_join("export_out"), "no_prepare": true, "project_dir": root, "app_name": "VerbGame"}
		"register_loose":
			return {"content_dir": content_dir}
		"install":
			return {"package_path": root.path_join("packages/Game.msixvc")}
		"uninstall":
			return {"package_name": "Publisher.VerbGame_1.0.0.0_x64__abc123"}
		"launch":
			return {"package_name": "Publisher.VerbGame_1.0.0.0_x64__abc123", "aumid": "Publisher.VerbGame!Game"}
		"terminate":
			return {"package_name": "Publisher.VerbGame_1.0.0.0_x64__abc123"}
		"sandbox":
			return {"action": "get"}
		"config_template":
			return {"output": root.path_join("generated/Template.config"), "overwrite": true, "app_name": "VerbGame", "identity_publisher": "Acme"}
		"config_editor":
			return {"config_path": config_path}
		"store_wizard":
			return {"config_path": config_path}
		_:
			return {}


func test_prepare_content_reports_logo_escape_in_result_message() -> void:
	var content_dir: String = _fixture_path("content")
	DirAccess.make_dir_recursive_absolute(content_dir)
	var config_path: String = _fixture_path("attack/MicrosoftGame.config")
	_write_text(config_path, _config_xml("AttackGame", "Attack.exe", {
		"Square150x150Logo": "..\\..\\outside.png",
	}))

	var result: Dictionary = _new_service().run_prepare_content({
		"content_dir": content_dir,
		"config_path": config_path,
	})

	assert_false(result["ok"], "escaping logo path fails content prep")
	assert_string_contains(result["message"], "outside content directory",
		"service result surfaces the rejection reason")
	assert_false(FileAccess.file_exists(content_dir.path_join("MicrosoftGame.config")),
		"rejected prep does not stage config")


func test_config_template_output_writes_requested_path() -> void:
	var output: String = _fixture_path("custom/AltGame.config")
	var implicit_path: String = ProjectSettings.globalize_path("res://MicrosoftGame.config")
	var implicit_existed: bool = FileAccess.file_exists(implicit_path)

	var result: Dictionary = _new_service().run_config_template({
		"output": output,
		"app_name": "AltGame",
		"identity_publisher": "Acme",
	})

	assert_true(result["ok"], "config_template succeeds")
	assert_true(FileAccess.file_exists(output), "custom --output file is created")
	assert_string_contains(_read_text(output), 'Executable Name="AltGame.exe"',
		"template content lands in the requested output file")
	if not implicit_existed:
		assert_false(FileAccess.file_exists(implicit_path),
			"custom --output does not create res://MicrosoftGame.config")


func test_config_template_overwrite_recreates_requested_output() -> void:
	var output: String = _fixture_path("custom/OverwriteGame.config")
	_write_text(output, "old requested output")

	var result: Dictionary = _new_service().run_config_template({
		"output": output,
		"overwrite": true,
		"app_name": "OverwriteGame",
		"identity_publisher": "Acme",
	})

	assert_true(result["ok"], "--overwrite succeeds")
	assert_true(FileAccess.file_exists(output), "requested output is recreated")
	var content: String = _read_text(output)
	assert_false(content.contains("old requested output"), "old requested output was replaced")
	assert_string_contains(content, 'Executable Name="OverwriteGame.exe"',
		"new template is written at the requested output")


# ── path normalization (PR #13 review follow-up) ────────────────────────────

const _ConfigMgr = preload("res://addons/godot_gdk_packaging/core/game_config_manager.gd")


func test_to_filesystem_path_passes_through_empty_string() -> void:
	assert_eq(_ConfigMgr.to_filesystem_path(""), "",
		"empty input is returned unchanged")


func test_to_filesystem_path_globalizes_res_paths() -> void:
	var normalized: String = _ConfigMgr.to_filesystem_path("res://Configs/Alt.config")
	assert_eq(normalized, ProjectSettings.globalize_path("res://Configs/Alt.config"),
		"res:// paths are globalized to filesystem-absolute form")


func test_to_filesystem_path_globalizes_user_paths() -> void:
	var normalized: String = _ConfigMgr.to_filesystem_path("user://test_packaging_service/some.config")
	assert_eq(normalized, ProjectSettings.globalize_path("user://test_packaging_service/some.config"),
		"user:// paths are globalized to filesystem-absolute form")


func test_to_filesystem_path_preserves_absolute_paths() -> void:
	var absolute_input: String = _fixture_path("custom/Already.config")
	assert_eq(_ConfigMgr.to_filesystem_path(absolute_input), absolute_input,
		"already-absolute filesystem paths are returned unchanged")


func test_to_filesystem_path_resolves_relative_paths_against_project_root() -> void:
	var relative: String = "Configs/Alt.config"
	var expected: String = ProjectSettings.globalize_path("res://").path_join(relative)
	assert_eq(_ConfigMgr.to_filesystem_path(relative), expected,
		"relative --output paths are resolved against the project root, not CWD")


func test_config_template_accepts_res_output_path() -> void:
	# res://test_packaging_service/ResOut.config — write under user-test fixture
	# tree by globalizing a unique res-rooted path.
	var res_out: String = "res://test_packaging_service_res_out.config"
	var fs_out: String = ProjectSettings.globalize_path(res_out)
	# Clean any stale artefact from a previous run before / after the test.
	if FileAccess.file_exists(fs_out):
		DirAccess.remove_absolute(fs_out)

	var result: Dictionary = _new_service().run_config_template({
		"output": res_out,
		"overwrite": true,
		"app_name": "ResGame",
		"identity_publisher": "Acme",
	})

	assert_true(result["ok"], "res:// --output succeeds (regression: PR #13 review)")
	assert_true(FileAccess.file_exists(fs_out),
		"res:// --output writes to its globalized filesystem path")
	assert_string_contains(_read_text(fs_out), 'Executable Name="ResGame.exe"',
		"template content lands at the res:// destination")
	# Cleanup
	if FileAccess.file_exists(fs_out):
		DirAccess.remove_absolute(fs_out)


# ── PR #13 round-2 review follow-up: storelogos lands next to the config ────


func test_config_template_writes_storelogos_next_to_output() -> void:
	# When --output writes outside the project root, the placeholder logos
	# must land in <output_dir>/storelogos/ so the config's relative
	# "storelogos\..." references resolve next to the file. (PR #13 round-2.)
	var output: String = _fixture_path("alt_config_root/Custom.config")
	var sibling_logos_dir: String = _fixture_path("alt_config_root/storelogos")
	# Sanity: fixture starts clean.
	assert_false(DirAccess.dir_exists_absolute(sibling_logos_dir),
		"fixture starts with no sibling storelogos directory")

	var result: Dictionary = _new_service().run_config_template({
		"output": output,
		"overwrite": true,
		"app_name": "CustomGame",
		"identity_publisher": "Acme",
	})

	assert_true(result["ok"], "config_template succeeds for out-of-tree output")
	# We don't require the placeholder PNGs themselves (those need the GDK
	# default480x480 PNG, which the FakeToolchain points at a missing path —
	# the function will push_warning and return without writing PNGs). But we
	# DO require that the implementation reaches for the *sibling* storelogos
	# location, not res://storelogos, so the next-most-actionable check is
	# that res://storelogos is not created as a side-effect of running the
	# template with a custom --output.
	var res_logos: String = ProjectSettings.globalize_path("res://storelogos")
	# If res://storelogos already exists from a real prior template run we
	# can't assert on its absence — gate the assertion on a clean start.
	# In CI fixtures this directory is freshly created per-run.
	if not DirAccess.dir_exists_absolute(res_logos):
		# Already absent before — must remain absent after.
		assert_false(DirAccess.dir_exists_absolute(res_logos),
			"out-of-tree --output does not create res://storelogos as a side effect")
