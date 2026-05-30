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

const _RUNTIME_ADDON_NAME := "_copy_refresh_probe"
const _RUNTIME_DLL := "ProbeRuntime.dll"
const _RUNTIME_CONTENT_DIR := "user://packaging_runtime_copy"


func before_each() -> void:
	_cleanup_runtime_copy_fixture()


func after_each() -> void:
	_cleanup_runtime_copy_fixture()


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


func _cleanup_runtime_copy_fixture() -> void:
	var project_dir: String = ProjectSettings.globalize_path("res://")
	_remove_dir_recursive(project_dir.path_join("addons").path_join(_RUNTIME_ADDON_NAME))
	_remove_dir_recursive(ProjectSettings.globalize_path(_RUNTIME_CONTENT_DIR))


func _remove_dir_recursive(path: String) -> void:
	if not DirAccess.dir_exists_absolute(path):
		return
	for filename: String in DirAccess.get_files_at(path):
		DirAccess.remove_absolute(path.path_join(filename))
	for dirname: String in DirAccess.get_directories_at(path):
		_remove_dir_recursive(path.path_join(dirname))
	DirAccess.remove_absolute(path)
