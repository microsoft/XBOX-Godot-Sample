extends "res://addons/godot_gdk_tests/gdk_test_base.gd"
## GUT coverage for the GDK export platform's tool-failure diagnostics
## (`addons/godot_gdk/editor/gdk_export_platform.gd`).
##
## Regression pin for issue #123 ("Export error code 47"): when a Register
## Loose / MSIXVC export step fails, the platform used to return `ERR_BUG`
## (Godot Error value 47), which the export dialog surfaced as an opaque
## "unexpected error code 47" while the real cause stayed buried in the
## console. It now captures both stdout and stderr and surfaces the real
## makepkg/wdapp HRESULT.
##
## The platform script `extends EditorExportPlatformExtension`, which cannot be
## instantiated outside the editor process, so the diagnostic logic lives in
## static functions we exercise directly, and the return-code regression is
## pinned by source inspection.

const ExportPlatform := preload("res://addons/godot_gdk/editor/gdk_export_platform.gd")
const SCRIPT_PATH := "res://addons/godot_gdk/editor/gdk_export_platform.gd"

# Real strings captured from GDK 260400 makepkg.exe on failure.
const GENMAP_STDERR := "'C:\\out\\layout.xml' map file was not created, error = 0x80070003"
const GENMAP_STDOUT := "Failed to traverse path 'C:\\out\\does_not_exist'."
const PACK_STDOUT := "Failed with error (0x80070002): The system cannot find the file specified.\nPackage was not created, error = 0x80070002. See the full output for additional information."
const PACK_STDERR := "Mapfile 'C:\\out\\nope_layout.xml' does not exist."


# ── HRESULT extraction ────────────────────────────────────────────────────

func test_extract_hresult_from_genmap_stderr() -> void:
	assert_eq(ExportPlatform._extract_hresult(GENMAP_STDERR), "0x80070003",
		"genmap stderr HRESULT extracted")


func test_extract_hresult_from_pack_stdout() -> void:
	assert_eq(ExportPlatform._extract_hresult(PACK_STDOUT), "0x80070002",
		"pack stdout HRESULT extracted")


func test_extract_hresult_ignores_success_zero() -> void:
	# 0x00000000 is a success token and must never be reported as the failure.
	assert_eq(ExportPlatform._extract_hresult("done, error = 0x00000000"), "",
		"success HRESULT (0x00000000) is not treated as a failure code")


func test_extract_hresult_prefers_failure_code_after_success() -> void:
	var text := "first 0x00000000 then error = 0x80070005"
	assert_eq(ExportPlatform._extract_hresult(text), "0x80070005",
		"a trailing failure HRESULT wins over a leading success token")


func test_extract_hresult_none_present() -> void:
	assert_eq(ExportPlatform._extract_hresult("everything is fine"), "",
		"plain text yields no HRESULT")


# ── HRESULT descriptions ──────────────────────────────────────────────────

func test_describe_known_hresults() -> void:
	assert_string_contains(ExportPlatform._describe_hresult("0x80070002"), "ERROR_FILE_NOT_FOUND")
	assert_string_contains(ExportPlatform._describe_hresult("0x80070003"), "ERROR_PATH_NOT_FOUND")
	assert_string_contains(ExportPlatform._describe_hresult("0x80070005"), "ERROR_ACCESS_DENIED")
	assert_string_contains(ExportPlatform._describe_hresult("0x80070057"), "E_INVALIDARG")


func test_describe_unknown_hresult_is_empty() -> void:
	assert_eq(ExportPlatform._describe_hresult("0x8000FFFF"), "",
		"unknown HRESULT returns no description")


# ── Failure summary ───────────────────────────────────────────────────────

func test_summary_surfaces_hresult_and_output() -> void:
	var result := {"exit_code": 3, "stdout": GENMAP_STDOUT, "stderr": GENMAP_STDERR}
	var summary: String = ExportPlatform._summarize_tool_failure("makepkg genmap", result)
	assert_string_contains(summary, "makepkg genmap failed")
	assert_string_contains(summary, "0x80070003")
	assert_string_contains(summary, "ERROR_PATH_NOT_FOUND")
	# Both stream contents must be echoed so the real cause is visible.
	assert_string_contains(summary, "Failed to traverse path")
	assert_string_contains(summary, "map file was not created")


func test_summary_handles_pack_split_streams() -> void:
	var result := {"exit_code": 3, "stdout": PACK_STDOUT, "stderr": PACK_STDERR}
	var summary: String = ExportPlatform._summarize_tool_failure("makepkg pack", result)
	assert_string_contains(summary, "0x80070002")
	assert_string_contains(summary, "Mapfile")
	assert_string_contains(summary, "The system cannot find the file specified")


func test_summary_without_output_reports_exit_code() -> void:
	var result := {"exit_code": 9, "stdout": "", "stderr": ""}
	var summary: String = ExportPlatform._summarize_tool_failure("wdapp register", result)
	assert_string_contains(summary, "wdapp register failed")
	assert_string_contains(summary, "9")


func test_summary_caps_long_output_to_tail() -> void:
	# 40 lines of stdout should be truncated to the tail in the dialog summary,
	# with the HRESULT (on the last line) and a truncation notice preserved.
	var many: Array[String] = []
	for i in range(39):
		many.append("noise line %d" % i)
	many.append("Package was not created, error = 0x80070002.")
	var result := {"exit_code": 3, "stdout": "\n".join(many), "stderr": ""}
	var summary: String = ExportPlatform._summarize_tool_failure("makepkg pack", result)

	# The HRESULT must survive truncation.
	assert_string_contains(summary, "0x80070002")
	# A truncation notice referencing the full line count is shown.
	assert_string_contains(summary, "last 12 of 40 lines")
	# Early noise lines are dropped; the final line is retained.
	assert_false(summary.contains("noise line 0"), "leading lines are truncated")
	assert_string_contains(summary, "Package was not created")
	# The embedded output is bounded (well under the 40 source lines).
	assert_true(summary.split("\n").size() <= 16, "summary stays compact")


# ── Return-code regression (issue #123) ───────────────────────────────────

func test_tool_steps_no_longer_return_err_bug() -> void:
	var src := FileAccess.get_file_as_string(SCRIPT_PATH)
	assert_ne(src, "", "export platform source is readable")
	assert_false(src.contains("return ERR_BUG"),
		"tool-failure paths must not return ERR_BUG (Godot error 47) — issue #123")
	assert_true(src.contains("return FAILED"),
		"tool-failure paths return FAILED so the dialog shows a real error")


# ── Lazy GDK detection regression (issue #127) ────────────────────────────
#
# Godot only began calling EditorExportPlatform::initialize() from
# add_export_platform() in 4.6, so on the supported Godot 4.5.x line the
# platform's _initialize() never fires. If detection only ran from
# _initialize(), _gdk_found stayed false and "XBOX on PC" refused to export on
# 4.5.x. Detection must therefore be triggered lazily from the export/
# validation entry points as well. Pinned by source inspection because
# EditorExportPlatformExtension can only be instantiated by the editor process.

# Returns the source of a top-level GDScript function `p_name`: from its
# `func p_name(` header up to (but not including) the next top-level `func`.
func _func_body(p_src: String, p_name: String) -> String:
	var header := "func %s(" % p_name
	var start := p_src.find(header)
	if start == -1:
		return ""
	var rest := p_src.substr(start + header.length())
	var next := rest.find("\nfunc ")
	if next == -1:
		return rest
	return rest.substr(0, next)


func test_detection_triggered_lazily_not_only_from_initialize() -> void:
	var src := FileAccess.get_file_as_string(SCRIPT_PATH)
	assert_ne(src, "", "export platform source is readable")
	assert_string_contains(src, "func _ensure_detected(",
		"a one-shot _ensure_detected() detection helper exists")
	# Every engine-invoked entry point that reads detection state must trigger
	# detection itself, so the platform works on engines that never call
	# _initialize() (Godot 4.5.x).
	for fn: String in ["_has_valid_export_configuration", "_has_valid_project_configuration", "_export_project"]:
		assert_string_contains(_func_body(src, fn), "_ensure_detected()",
			"%s() triggers lazy GDK detection (issue #127)" % fn)


func test_ensure_detected_is_guarded_one_shot() -> void:
	var src := FileAccess.get_file_as_string(SCRIPT_PATH)
	var body := _func_body(src, "_ensure_detected")
	assert_ne(body, "", "_ensure_detected() is defined")
	# Guarded by a flag so repeated dialog polls don't re-scan the filesystem
	# or re-emit "GDK not found" warnings.
	assert_string_contains(body, "_detected",
		"_ensure_detected() is guarded by the _detected flag")
	assert_string_contains(body, "_detect_gdk()",
		"_ensure_detected() runs _detect_gdk()")
	# _initialize() must funnel through the guarded helper (not call _detect_gdk
	# directly) so eager (4.6+) and lazy (4.5.x) paths share one detection.
	assert_string_contains(_func_body(src, "_initialize"), "_ensure_detected()",
		"_initialize() routes through _ensure_detected()")


# ── Export template version-dir resolution (issue #134) ───────────────────
#
# `_find_windows_template` used to build the export_templates subdir name as
# "<major>.<minor>.<status>" (e.g. "4.6.stable"), which never matched the real
# "4.6.2.stable" directory on a patch release — so patch releases silently fell
# back to the Godot editor binary as a stand-in template and shipped a package
# that failed at launch with "GDExtension dynamic library not found". The
# candidate names now include the patch component, most-specific first.

func test_template_version_dirs_prefers_patch_qualified() -> void:
	var dirs := ExportPlatform._template_version_dirs(
		{"major": 4, "minor": 6, "patch": 2, "status": "stable"})
	assert_eq(dirs[0], "4.6.2.stable",
		"patch-qualified dir name is tried first (issue #134)")
	assert_true(dirs.has("4.6.2.stable"),
		"the real patch-release template dir is a candidate")


func test_template_version_dirs_handles_x_y_zero() -> void:
	# Godot omits the patch component for x.y.0 releases ("4.5.stable"), so that
	# form must stay a candidate alongside the patch-qualified name.
	var dirs := ExportPlatform._template_version_dirs(
		{"major": 4, "minor": 5, "patch": 0, "status": "stable"})
	assert_true(dirs.has("4.5.stable"),
		"patch-less dir name (x.y.0) is still a candidate")


func test_template_version_dirs_statusless_fallbacks() -> void:
	# Dev/custom builds may carry no status; bare "major.minor.patch" and
	# "major.minor" must still be offered.
	var dirs := ExportPlatform._template_version_dirs(
		{"major": 4, "minor": 6, "patch": 2, "status": ""})
	assert_true(dirs.has("4.6.2"), "statusless patch dir is a candidate")
	assert_true(dirs.has("4.6"), "statusless minor dir is a candidate")


func test_find_windows_template_uses_version_dir_helper() -> void:
	var src := FileAccess.get_file_as_string(SCRIPT_PATH)
	assert_string_contains(_func_body(src, "_find_windows_template"),
		"_template_version_dirs",
		"_find_windows_template resolves candidates via _template_version_dirs")


# ── Zero-GDExtension-DLL guard (issue #134) ───────────────────────────────
#
# A build that stages no GDExtension main DLL loads with "GDExtension dynamic
# library not found". The exporter must abort at export time with an actionable
# message instead of silently packaging the broken build.

func test_missing_main_dll_message_is_actionable() -> void:
	var msg: String = ExportPlatform._missing_main_dll_message("release")
	assert_string_contains(msg, "release", "message names the failing config")
	assert_string_contains(msg, "GDExtension dynamic library not found",
		"message ties the failure to the runtime symptom")
	assert_string_contains(msg, "cmake --build build --preset release",
		"message gives the exact build command to fix a release export")


func test_missing_main_dll_message_debug_variant() -> void:
	var msg: String = ExportPlatform._missing_main_dll_message("debug")
	assert_string_contains(msg, "cmake --build build --preset debug",
		"debug variant names the debug build preset")


func test_copy_addon_dlls_fails_when_zero_main_dlls() -> void:
	var src := FileAccess.get_file_as_string(SCRIPT_PATH)
	var body := _func_body(src, "_copy_addon_dlls")
	assert_string_contains(body, "main_copied == 0",
		"_copy_addon_dlls detects the zero-main-DLL case")
	assert_string_contains(body, "_missing_main_dll_message",
		"_copy_addon_dlls surfaces the actionable message")
	assert_string_contains(body, "return ERR_FILE_NOT_FOUND",
		"_copy_addon_dlls aborts the export when no main DLL was staged")


# ── Editor-binary-as-template guard (issue #134) ──────────────────────────
#
# The Godot editor binary is not a valid game template: it enables
# has_feature("editor") and resolves GDExtension DLLs from the dev machine's
# source tree, so a package built from it fails "not found" elsewhere. Packaged
# exports must refuse it; only same-machine loose dev-register may fall back.

func test_export_refuses_editor_binary_when_packaging() -> void:
	var src := FileAccess.get_file_as_string(SCRIPT_PATH)
	var body := _func_body(src, "_export_project")
	assert_string_contains(body, "if not use_loose:",
		"a non-loose (packaged) export refuses to continue without a real template")
	assert_string_contains(body, "OS.get_executable_path()",
		"the editor-binary fallback remains reachable for loose dev-register")


# ── Export-plugin shared objects / C# assemblies (issue #144) ─────────────
#
# `export_pack()` calls `save_pack()` with a null shared-object sink and opens a
# second ExportNotifier inside the one EditorExportPlatformExtension already
# opened around `_export_project()`. That ran every export plugin's
# `_export_begin`/`_export_end` twice AND discarded everything the plugins
# registered via `add_shared_object()`. Godot ships a C#/.NET project's
# published assemblies exclusively as shared objects targeted at
# `data_<assembly>_windows_x86_64/`, so an `XBOX on PC` export produced a
# package with zero managed DLLs. `save_pack()` returns those entries so the
# platform can stage them itself.

func test_export_pck_uses_save_pack_to_capture_shared_objects() -> void:
	var src := FileAccess.get_file_as_string(SCRIPT_PATH)
	var body := _func_body(src, "_export_pck")
	assert_string_contains(body, "save_pack(",
		"_export_pck() uses save_pack() so so_files come back to the platform")
	assert_false(body.contains("export_pack("),
		"export_pack() drops add_shared_object() entries and double-fires " +
		"every export plugin's _export_begin/_export_end — issue #144")


func test_export_project_stages_shared_objects_before_addon_dlls() -> void:
	var src := FileAccess.get_file_as_string(SCRIPT_PATH)
	var body := _func_body(src, "_export_project")
	assert_string_contains(body, "_stage_shared_objects(",
		"the export stages the shared objects save_pack() reported")
	assert_string_contains(body, "so_files",
		"the export reads save_pack()'s so_files list")
	var so_at := body.find("_stage_shared_objects(")
	var dll_at := body.find("_copy_addon_dlls(")
	assert_true(so_at != -1 and dll_at != -1 and so_at < dll_at,
		"shared objects are staged before _copy_addon_dlls() so its " +
		"already-present guard keeps addon DLLs authoritative")


func test_shared_object_without_target_lands_next_to_the_exe() -> void:
	var dest: String = ExportPlatform._shared_object_destination(
		"C:/out/_gdk_staging", "C:/tmp/publish/Game.dll", "")
	assert_eq(dest, "C:/out/_gdk_staging/Game.dll",
		"an empty target folder means the package root")


func test_shared_object_target_folder_is_preserved() -> void:
	var dest: String = ExportPlatform._shared_object_destination(
		"C:/out/_gdk_staging", "C:/tmp/publish/Game.dll", "data_Game_windows_x86_64")
	assert_eq(dest, "C:/out/_gdk_staging/data_Game_windows_x86_64/Game.dll",
		"the C# data_<assembly>_windows_x86_64 target folder is honoured")


func test_shared_object_nested_target_folder_is_preserved() -> void:
	var dest: String = ExportPlatform._shared_object_destination(
		"C:/out/_gdk_staging", "C:/tmp/publish/sub/dep.dll",
		"data_Game_windows_x86_64/sub")
	assert_eq(dest, "C:/out/_gdk_staging/data_Game_windows_x86_64/sub/dep.dll",
		"nested publish subdirectories keep their relative layout")


func test_shared_object_target_cannot_escape_the_staging_dir() -> void:
	for target: String in ["..", "../../elsewhere", "/abs/path", "C:/abs", "res://x"]:
		var dest: String = ExportPlatform._shared_object_destination(
			"C:/out/_gdk_staging", "C:/tmp/publish/Game.dll", target)
		assert_eq(dest, "",
			"target %s must be rejected instead of writing outside the package" % target)


func test_addon_bin_libraries_are_left_to_copy_addon_dlls() -> void:
	var project := "C:/proj/"
	assert_true(ExportPlatform._is_addon_bin_library(
		"C:/proj/addons/godot_gdk/bin/godot_gdk.windows.debug.x86_64.dll", project),
		"addons/<name>/bin/ DLLs are staged by _copy_addon_dlls with config filtering")


func test_non_addon_bin_shared_objects_are_staged() -> void:
	var project := "C:/proj/"
	assert_false(ExportPlatform._is_addon_bin_library(
		"C:/tmp/godot-publish-dotnet/1234-ExportRelease-win-x64/Game.dll", project),
		"C# publish output is outside the project and must be staged")
	assert_false(ExportPlatform._is_addon_bin_library(
		"C:/proj/addons/other/lib/native.dll", project),
		"a GDExtension outside addons/<name>/bin/ still needs staging")
	assert_false(ExportPlatform._is_addon_bin_library(
		"C:/proj/addons/godot_gdk/bin/sub/nested.dll", project),
		"only files directly in addons/<name>/bin/ are owned by _copy_addon_dlls")

