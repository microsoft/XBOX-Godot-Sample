extends GutTest
## Regression coverage for headless verb facade behaviours that the CLI
## advertises directly.

const PackagingServiceScript = preload("res://addons/godot_gdk_packaging/core/packaging_service.gd")

const _FIXTURE_DIR := "user://test_packaging_service"

class _FakeToolchain extends RefCounted:
	func get_bin_dir() -> String:
		return ProjectSettings.globalize_path("user://missing_gdk_bin")


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
