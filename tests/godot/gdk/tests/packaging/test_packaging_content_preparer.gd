extends GutTest
## GUT coverage for the static helpers on `packaging_content_preparer.gd`
## (absorbed from the cancelled `cpp-packaging-helpers` workstream — the
## helpers are pure GDScript with no C++ surface, so this is the right
## test layer).
##
## Pinned behaviors:
##   * `patch_executable_name` rewrites every `<Executable Name="…">`, XML-
##     escapes replacement values, and treats dollar signs as literal text.
##   * `inject_vc14_dependency` merges into an existing DesktopRegistration/
##     DependencyList and skips duplicates by dependency Name.
##   * Both helpers passthrough on no-match (no `Executable Name=`,
##     no `</Game>` tag).

const PackagingContentPreparerScript = preload("res://addons/godot_gdk_packaging/core/packaging_content_preparer.gd")
const GameConfigManagerScript = preload("res://addons/godot_gdk_packaging/core/game_config_manager.gd")

const _FIXTURE_DIR := "user://test_packaging_content_preparer"

class _FakeToolchain extends RefCounted:
	func get_bin_dir() -> String:
		return ProjectSettings.globalize_path("user://missing_gdk_bin")


func before_each() -> void:
	_reset_fixture()
	DirAccess.make_dir_recursive_absolute(_fixture_root())
	_cleanup_runtime_copy_fixture()


func after_each() -> void:
	_reset_fixture()
	_cleanup_runtime_copy_fixture()


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


func _write_text(path: String, content: String) -> void:
	DirAccess.make_dir_recursive_absolute(path.get_base_dir())
	var file: FileAccess = FileAccess.open(path, FileAccess.WRITE)
	file.store_string(content)
	file.close()


func _read_text(path: String) -> String:
	var file: FileAccess = FileAccess.open(path, FileAccess.READ)
	if file == null:
		return ""
	var content: String = file.get_as_text()
	file.close()
	return content


func _config_xml(identity_name: String, executable: String, shell_attrs: Dictionary = {}) -> String:
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


func _new_preparer() -> RefCounted:
	return PackagingContentPreparerScript.new(GameConfigManagerScript.new(_FakeToolchain.new()))

const _RUNTIME_ADDON_NAME := "_copy_refresh_probe"
const _RUNTIME_DLL := "ProbeRuntime.dll"
const _RUNTIME_CONTENT_DIR := "user://packaging_runtime_copy"


# ── patch_executable_name ─────────────────────────────────────────────────

func test_patch_executable_replaces_existing_attribute() -> void:
	var input := '<Executable Name="OldName.exe" Id="Game" />'
	var patched: String = PackagingContentPreparerScript.patch_executable_name(input, "NewName.exe")
	assert_string_contains(patched, 'Executable Name="NewName.exe"', "new exe name written")
	assert_false(patched.contains("OldName.exe"), "old exe name removed")


func test_patch_executable_no_match_passthrough() -> void:
	var input := '<Executable Id="Game" />'
	var patched: String = PackagingContentPreparerScript.patch_executable_name(input, "Anything.exe")
	assert_eq(patched, input, "input returned unchanged when no Executable Name= attribute is present")


func test_patch_executable_replaces_all_attributes() -> void:
	var input := '<Executable Name="A.exe" /><Executable Name="B.exe" />'
	var patched: String = PackagingContentPreparerScript.patch_executable_name(input, "C.exe")
	assert_eq(_count_occurrences(patched, 'Name="C.exe"'), 2, "all executable names replaced")
	assert_false(patched.contains('Name="A.exe"'), "first old name removed")
	assert_false(patched.contains('Name="B.exe"'), "second old name removed")


func test_patch_executable_escapes_xml_special_chars() -> void:
	var input := '<Executable Name="OldName.exe" Id="Game" />'
	var unsafe := 'Game&Name<Special>"Quote".exe'
	var patched: String = PackagingContentPreparerScript.patch_executable_name(input, unsafe)
	assert_string_contains(
		patched,
		'Executable Name="Game&amp;Name&lt;Special&gt;&quot;Quote&quot;.exe"',
		"replacement is safe for XML attributes")
	assert_false(patched.contains(unsafe), "unsafe replacement was not inserted verbatim")


func test_patch_executable_preserves_dollar_characters() -> void:
	var input := '<Executable Name="OldName.exe" Id="Game" />'
	var patched: String = PackagingContentPreparerScript.patch_executable_name(input, "Cash$1Game$N.exe")
	assert_string_contains(patched, 'Executable Name="Cash$1Game$N.exe"', "dollar signs stay literal")


func test_patch_executable_handles_realistic_config_block() -> void:
	var input := ""
	input += '<?xml version="1.0" encoding="utf-8"?>\n'
	input += '<Game configVersion="1">\n'
	input += '  <ExecutableList>\n'
	input += '    <Executable Name="OldExe.exe" Id="Game" />\n'
	input += '  </ExecutableList>\n'
	input += '</Game>\n'
	var patched: String = PackagingContentPreparerScript.patch_executable_name(input, "MyGame.exe")
	assert_string_contains(patched, 'Executable Name="MyGame.exe"', "realistic config patched")
	assert_false(patched.contains("OldExe.exe"), "previous name gone")
	assert_string_contains(patched, "</Game>", "rest of XML preserved")


# ── inject_vc14_dependency ────────────────────────────────────────────────

func test_inject_vc14_inserts_block_before_close_game() -> void:
	var input := "<Game configVersion=\"1\">\n  <Identity Name=\"X\" />\n</Game>\n"
	var injected: String = PackagingContentPreparerScript.inject_vc14_dependency(input)
	assert_string_contains(injected, '<KnownDependency Name="VC14"/>', "VC14 dependency injected")
	assert_string_contains(injected, "<DesktopRegistration>", "DesktopRegistration wrapper added")
	assert_string_contains(injected, "</Game>", "</Game> still present")
	assert_lt(
		injected.find('<KnownDependency Name="VC14"/>'),
		injected.find("</Game>"),
		"dependency block sits BEFORE </Game>")


func test_inject_vc14_idempotent_when_dependency_name_already_present() -> void:
	var existing := ""
	existing += "<Game>\n"
	existing += "  <DesktopRegistration>\n"
	existing += "    <DependencyList>\n"
	existing += "      <PackageDependency Name=\"VC14\" MinVersion=\"old\" />\n"
	existing += "    </DependencyList>\n"
	existing += "  </DesktopRegistration>\n"
	existing += "</Game>"
	var injected: String = PackagingContentPreparerScript.inject_vc14_dependency(existing)
	assert_eq(injected, existing, "input with VC14 Name already present returned unchanged")
	assert_eq(_count_occurrences(injected, 'Name="VC14"'), 1, "no duplicate VC14 dependency added")


func test_inject_vc14_merges_into_existing_dependency_list() -> void:
	var input := ""
	input += "<Game>\n"
	input += "  <DesktopRegistration>\n"
	input += "    <DependencyList>\n"
	input += "      <PackageDependency Name=\"OtherRuntime\" MinVersion=\"1.0.0.0\" />\n"
	input += "    </DependencyList>\n"
	input += "  </DesktopRegistration>\n"
	input += "</Game>"
	var injected: String = PackagingContentPreparerScript.inject_vc14_dependency(input)
	assert_string_contains(injected, '<PackageDependency Name="OtherRuntime"', "existing dependency preserved")
	assert_string_contains(injected, '<KnownDependency Name="VC14"/>', "VC14 dependency merged")
	assert_eq(_count_occurrences(injected, "<DependencyList"), 1, "no duplicate DependencyList added")
	assert_eq(_count_occurrences(injected, "<DesktopRegistration"), 1, "no duplicate DesktopRegistration added")


func test_inject_vc14_passthrough_when_no_close_game_tag() -> void:
	var input := "<NotAGameConfig />"
	var injected: String = PackagingContentPreparerScript.inject_vc14_dependency(input)
	assert_eq(injected, input, "input without </Game> returned unchanged")


func test_inject_vc14_runs_again_after_dependency_stripped() -> void:
	var input := "<Game>\n</Game>\n"
	var first: String = PackagingContentPreparerScript.inject_vc14_dependency(input)
	var stripped := first.replace('<KnownDependency Name="VC14"/>', '<KnownDependency Name="OTHER"/>')
	var second: String = PackagingContentPreparerScript.inject_vc14_dependency(stripped)
	assert_string_contains(second, '<KnownDependency Name="VC14"/>', "VC14 re-injected after marker removed")
	assert_string_contains(second, '<KnownDependency Name="OTHER"/>', "unrelated dependency preserved")
	assert_eq(_count_occurrences(second, "<DependencyList"), 1, "VC14 merged into existing DependencyList")


# ── runtime DLL copying ───────────────────────────────────────────────────

func test_copy_addon_runtime_dlls_refreshes_stale_destination() -> void:
	var project_dir: String = ProjectSettings.globalize_path("res://")
	var addon_bin: String = project_dir.path_join("addons").path_join(_RUNTIME_ADDON_NAME).path_join("bin")
	var content_dir: String = ProjectSettings.globalize_path(_RUNTIME_CONTENT_DIR)
	DirAccess.make_dir_recursive_absolute(addon_bin)
	DirAccess.make_dir_recursive_absolute(content_dir)
	var src_path: String = addon_bin.path_join(_RUNTIME_DLL)
	var dest_path: String = content_dir.path_join(_RUNTIME_DLL)
	_write_text(src_path, "fresh-runtime")
	_write_text(dest_path, "stale-runtime")

	var preparer = PackagingContentPreparerScript.new(RefCounted.new())
	preparer._copy_addon_runtime_dlls(content_dir, Callable())

	assert_eq(_read_text(dest_path), "fresh-runtime", "stale destination DLL overwritten from source")


# ── pipeline (typical caller order) ───────────────────────────────────────

func test_pipeline_inject_then_patch_matches_caller_order() -> void:
	# `ensure_content_dir_ready` calls inject_vc14_dependency first, then
	# patch_executable_name. Document both transformations on a single
	# canonical config block.
	var input := ""
	input += '<?xml version="1.0" encoding="utf-8"?>\n'
	input += '<Game configVersion="1">\n'
	input += '  <ExecutableList>\n'
	input += '    <Executable Name="Placeholder.exe" Id="Game" />\n'
	input += '  </ExecutableList>\n'
	input += '</Game>\n'
	var step1: String = PackagingContentPreparerScript.inject_vc14_dependency(input)
	var step2: String = PackagingContentPreparerScript.patch_executable_name(step1, "RealGame.exe")
	assert_string_contains(step2, 'Executable Name="RealGame.exe"', "exe patched after inject")
	assert_string_contains(step2, '<KnownDependency Name="VC14"/>', "VC14 still present after patch")


func _count_occurrences(text: String, needle: String) -> int:
	if needle.is_empty():
		return 0
	return text.split(needle).size() - 1


func _cleanup_runtime_copy_fixture() -> void:
	var project_dir: String = ProjectSettings.globalize_path("res://")
	_remove_tree(project_dir.path_join("addons").path_join(_RUNTIME_ADDON_NAME))
	_remove_tree(ProjectSettings.globalize_path(_RUNTIME_CONTENT_DIR))


func test_ensure_content_dir_uses_config_override() -> void:
	var content_dir: String = _fixture_path("content")
	DirAccess.make_dir_recursive_absolute(content_dir)
	_write_text(content_dir.path_join("RealGame.exe"), "")
	var config_path: String = _fixture_path("override/MicrosoftGame.config")
	_write_text(config_path, _config_xml("OverrideGame", "Placeholder.exe"))

	var ok: bool = _new_preparer().ensure_content_dir_ready(content_dir, Callable(), config_path)
	assert_true(ok, "content prep succeeds with explicit config path")
	var staged_config: String = _read_text(content_dir.path_join("MicrosoftGame.config"))
	assert_string_contains(staged_config, 'Identity Name="OverrideGame"',
		"staged config comes from the --config target")
	assert_string_contains(staged_config, 'Executable Name="RealGame.exe"',
		"staged config is still patched to the exported executable")


func test_ensure_content_dir_rejects_logo_path_outside_content_dir() -> void:
	var content_dir: String = _fixture_path("content")
	DirAccess.make_dir_recursive_absolute(content_dir)
	var config_path: String = _fixture_path("attack/MicrosoftGame.config")
	_write_text(config_path, _config_xml("AttackGame", "Attack.exe", {
		"Square150x150Logo": "..\\..\\outside.png",
	}))
	var outside_path: String = content_dir.path_join("..\\..\\outside.png").simplify_path()
	var logs: Array[String] = []
	var logger := func(message: String) -> void:
		logs.append(message)

	var ok: bool = _new_preparer().ensure_content_dir_ready(content_dir, logger, config_path)
	assert_false(ok, "escaping logo destination is rejected")
	assert_false(FileAccess.file_exists(outside_path), "no file is written outside the content dir")
	assert_false(FileAccess.file_exists(content_dir.path_join("MicrosoftGame.config")),
		"content prep aborts before staging config")
	assert_string_contains("\n".join(logs), "outside content directory",
		"error explains the content-dir safety rule")
