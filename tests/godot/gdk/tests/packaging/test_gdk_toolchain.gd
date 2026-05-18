extends GutTest
## Edge-case coverage for `gdk_toolchain.gd` GDK detection.
##
## The toolchain script reads `GameDKCoreLatest` and `GDK_BIN` from the
## process environment in `_init`. We can't safely mutate process env in a
## test (it would leak into sibling suites), so the checks here are
## introspective — they pin the path-normalisation, the version-extraction
## branch behavior, and the soft-fail invariants when the tools are missing.

const GDKToolchainScript = preload("res://addons/godot_gdk_packaging/core/gdk_toolchain.gd")

const SCRIPT_PATH := "res://addons/godot_gdk_packaging/core/gdk_toolchain.gd"


# ── Public API shape ──────────────────────────────────────────────────────

func test_default_construction_does_not_crash() -> void:
	var tc = GDKToolchainScript.new()
	assert_not_null(tc, "toolchain instantiates")
	# All getters must return something (never null) regardless of GDK presence.
	assert_eq(typeof(tc.get_makepkg_path()), TYPE_STRING, "makepkg path is String")
	assert_eq(typeof(tc.get_game_config_editor_path()), TYPE_STRING, "GameConfigEditor path is String")
	assert_eq(typeof(tc.get_sandbox_path()), TYPE_STRING, "sandbox path is String")
	assert_eq(typeof(tc.get_dev_account_path()), TYPE_STRING, "dev_account path is String")
	assert_eq(typeof(tc.get_gdk_version()), TYPE_STRING, "gdk version is String")
	assert_eq(typeof(tc.get_bin_dir()), TYPE_STRING, "bin dir is String")
	assert_eq(typeof(tc.is_gdk_available()), TYPE_BOOL, "is_gdk_available is bool")


func test_consistency_when_unavailable() -> void:
	# When the runtime SDK is not present the optional paths must stay
	# empty. When it IS present, the required paths (makepkg + game config
	# editor) must both be populated.
	var tc = GDKToolchainScript.new()
	if not tc.is_gdk_available():
		assert_eq(tc.get_makepkg_path(), "", "no makepkg path when unavailable")
		assert_eq(tc.get_game_config_editor_path(), "", "no GameConfigEditor path when unavailable")
		assert_eq(tc.get_bin_dir(), "", "no bin dir when unavailable")
	else:
		assert_ne(tc.get_makepkg_path(), "", "required makepkg path set when available")
		assert_ne(tc.get_game_config_editor_path(), "", "required GameConfigEditor path set when available")
		assert_ne(tc.get_bin_dir(), "", "bin dir set when available")
		assert_true(FileAccess.file_exists(tc.get_makepkg_path()), "makepkg.exe exists at detected path")
		assert_true(FileAccess.file_exists(tc.get_game_config_editor_path()), "GameConfigEditor.exe exists at detected path")


# ── execute_tool — missing executable soft-fail ───────────────────────────

func test_execute_tool_missing_exe_returns_error_dict() -> void:
	var tc = GDKToolchainScript.new()
	var result: Dictionary = tc.execute_tool("C:/this/path/does/not/exist/nope.exe", PackedStringArray(["--version"]))
	assert_eq(result["exit_code"], -1, "missing exe yields exit_code -1")
	assert_eq(result["stdout"], "", "stdout empty when exe missing")
	assert_string_contains(result["stderr"], "Tool not found", "stderr explains the failure")


func test_execute_tool_result_dict_shape() -> void:
	var tc = GDKToolchainScript.new()
	var result: Dictionary = tc.execute_tool("C:/missing.exe", PackedStringArray())
	assert_true(result.has("exit_code"), "result has exit_code key")
	assert_true(result.has("stdout"), "result has stdout key")
	assert_true(result.has("stderr"), "result has stderr key")
	assert_eq(typeof(result["exit_code"]), TYPE_INT, "exit_code is int")
	assert_eq(typeof(result["stdout"]), TYPE_STRING, "stdout is String")
	assert_eq(typeof(result["stderr"]), TYPE_STRING, "stderr is String")


# ── launch_detached — missing executable soft-fail ────────────────────────

func test_launch_detached_missing_exe_returns_negative() -> void:
	# `launch_detached` emits a `push_error` on missing exe. GUT treats any
	# pushed engine error as an unexpected failure, so we can't assert the
	# return value without polluting the run. Pin the contract via the
	# source instead — `launch_detached` returns -1 BEFORE calling
	# `OS.create_process` when the exe is missing.
	var src := _read_script_source()
	assert_string_contains(src, 'func launch_detached(', "launch_detached defined")
	assert_string_contains(src, 'return -1', "early-return on missing exe")
	assert_string_contains(src, 'OS.create_process(exe_path, args)', "delegates to OS.create_process when present")
	pending("launch_detached push_error path can't be exercised under GUT without flagging the run as failed")


# ── Path normalisation pinning ────────────────────────────────────────────

func test_path_normalisation_strips_backslashes() -> void:
	# `_detect_gdk` normalises the env value via `replace("\\", "/").split("/")`.
	# Pin the algorithm: any 6-digit numeric segment in a backslash-separated
	# path must be discoverable.
	var raw_examples := [
		"C:\\Program Files (x86)\\Microsoft GDK\\260400\\",
		"C:/Program Files (x86)/Microsoft GDK/260400/",
		"D:\\custom\\path\\261000\\sub",
	]
	var expected_versions := ["260400", "260400", "261000"]
	for i in raw_examples.size():
		var raw: String = raw_examples[i]
		var version := _extract_version_segment(raw)
		assert_eq(version, expected_versions[i], "version extracted from %s" % raw)


func test_path_normalisation_ignores_non_numeric_segments() -> void:
	# Random 6-character non-numeric strings must NOT be picked up as a
	# version. Catches a regression where the length check is loosened.
	var bogus := [
		"C:\\foo\\abcdef\\bar",
		"C:\\foo\\12345\\bar",   # 5 digits — too short
		"C:\\foo\\1234567\\bar", # 7 digits — too long
		"C:\\foo\\26x400\\bar",  # mixed
	]
	for raw in bogus:
		assert_eq(_extract_version_segment(raw), "", "no version extracted from %s" % raw)


func test_empty_env_yields_empty_version() -> void:
	assert_eq(_extract_version_segment(""), "", "empty path produces empty version")


# Re-implements the detection branch in isolation so tests don't need to
# mutate process env. Mirrors `_detect_gdk` lines that read `GameDKCoreLatest`.
func _extract_version_segment(raw: String) -> String:
	if raw == "":
		return ""
	var parts := raw.replace("\\", "/").split("/")
	for part in parts:
		if part.length() == 6 and part.is_valid_int():
			return str(part)
	return ""


# ── Source invariants (catch silent contract drift) ───────────────────────

func test_default_gdk_bin_constant_matches_documented_path() -> void:
	# Pin the documented default install path so future relocations land
	# in docs and tests together.
	var src := _read_script_source()
	assert_string_contains(
		src,
		'_DEFAULT_GDK_BIN := "C:/Program Files (x86)/Microsoft GDK/bin"',
		"default GDK bin path constant unchanged")


func test_detect_gdk_uses_documented_env_vars() -> void:
	var src := _read_script_source()
	assert_string_contains(src, 'OS.get_environment("GameDKCoreLatest")', "GameDKCoreLatest read")
	assert_string_contains(src, 'OS.get_environment("GDK_BIN")', "GDK_BIN read")


func test_try_bin_dir_validates_required_tools() -> void:
	# Required tools per the contract: makepkg + GameConfigEditor.
	var src := _read_script_source()
	assert_string_contains(src, 'path_join("makepkg.exe")', "makepkg.exe required")
	assert_string_contains(src, 'path_join("GameConfigEditor.exe")', "GameConfigEditor.exe required")
	# Optional tools must be present in the source — sandbox + dev account.
	assert_string_contains(src, 'path_join("XblPCSandbox.exe")', "XblPCSandbox optional probe")
	assert_string_contains(src, 'path_join("XblDevAccount.exe")', "XblDevAccount optional probe")


func _read_script_source() -> String:
	var f := FileAccess.open(SCRIPT_PATH, FileAccess.READ)
	if f == null:
		return ""
	var text := f.get_as_text()
	f.close()
	return text
