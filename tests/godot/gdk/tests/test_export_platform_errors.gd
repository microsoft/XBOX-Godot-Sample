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


# ── Return-code regression (issue #123) ───────────────────────────────────

func test_tool_steps_no_longer_return_err_bug() -> void:
	var src := FileAccess.get_file_as_string(SCRIPT_PATH)
	assert_ne(src, "", "export platform source is readable")
	assert_false(src.contains("return ERR_BUG"),
		"tool-failure paths must not return ERR_BUG (Godot error 47) — issue #123")
	assert_true(src.contains("return FAILED"),
		"tool-failure paths return FAILED so the dialog shows a real error")
