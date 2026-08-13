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
const FEATURES_PLUGIN_PATH := "res://addons/godot_gdk/editor/gdk_export_features_plugin.gd"
const EDITOR_PLUGIN_PATH := "res://addons/godot_gdk/editor/gdk_editor_plugin.gd"

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


# ── Export template resolution (issues #134, #144 follow-up) ─────────────
#
# Template lookup used to be hand-rolled: it built `%APPDATA%\Godot\
# export_templates\<version>\` itself and guessed the version directory name
# from Engine.get_version_info(), including the `.mono` suffix for .NET editor
# builds. Getting that name wrong is what made patch releases silently fall
# back to the Godot editor binary and ship a package that died at launch with
# "GDExtension dynamic library not found" (issue #134).
#
# Godot exposes the exact lookup its own Windows exporter uses, so the platform
# now delegates to it. find_export_template() resolves
# `<templates dir>/<VERSION_FULL_CONFIG>/<file>`, which already encodes the
# patch component, the `.mono` suffix, self-contained mode and a relocated
# editor data directory — none of which the hand-rolled probe handled.

func test_template_file_name_matches_godot_naming() -> void:
	assert_eq(ExportPlatform._template_file_name(true), "windows_debug_x86_64.exe",
		"debug template file name matches Godot's Windows exporter")
	assert_eq(ExportPlatform._template_file_name(false), "windows_release_x86_64.exe",
		"release template file name matches Godot's Windows exporter")


func test_find_windows_template_delegates_to_engine_lookup() -> void:
	var src := FileAccess.get_file_as_string(SCRIPT_PATH)
	var body := _func_body(src, "_find_windows_template")
	assert_ne(body, "", "_find_windows_template() is defined")
	assert_string_contains(body, "find_export_template(",
		"template lookup delegates to EditorExportPlatform.find_export_template()")
	# The hand-rolled probe must be gone, not merely bypassed: it is the piece
	# that got the version directory wrong in issue #134.
	assert_false(body.contains("APPDATA"),
		"the hand-rolled %APPDATA% probe no longer exists")
	assert_false(body.contains("export_templates"),
		"the templates directory is resolved by the engine, not rebuilt here")
	assert_false(src.contains("_template_version_dirs"),
		"the version-dir guessing helper is removed along with its only caller")


func test_find_windows_template_unwraps_engine_result_dictionary() -> void:
	# find_export_template() is bound as returning {result, path, error_string}
	# on both Godot 4.5 and 4.6 — treating it as a String silently yields a
	# Dictionary-to-String conversion instead of a path.
	var src := FileAccess.get_file_as_string(SCRIPT_PATH)
	var body := _func_body(src, "_find_windows_template")
	assert_string_contains(body, "\"path\"",
		"the resolved path is read out of the result dictionary")
	assert_string_contains(body, "\"result\"",
		"the result code is checked before trusting the path")


# ── Custom export templates ──────────────────────────────────────────────
#
# `custom_template/debug` and `custom_template/release` are the standard Godot
# way to package against a custom engine build. EditorExportPlatformPC honours
# them ahead of the installed templates; this platform ignored them entirely,
# so a title built on a patched engine could not be packaged at all.

func test_custom_template_options_are_offered() -> void:
	var names: PackedStringArray = ExportPlatform.declared_option_names()
	assert_true("custom_template/debug" in names,
		"a custom debug template option is offered")
	assert_true("custom_template/release" in names,
		"a custom release template option is offered")


func test_custom_template_path_selects_option_by_build_config() -> void:
	var src := FileAccess.get_file_as_string(SCRIPT_PATH)
	var body := _func_body(src, "_custom_template_path")
	assert_string_contains(body, "custom_template/debug",
		"a debug export reads the debug custom template")
	assert_string_contains(body, "custom_template/release",
		"a release export reads the release custom template")


func test_custom_template_wins_over_installed_template() -> void:
	var src := FileAccess.get_file_as_string(SCRIPT_PATH)
	var body := _func_body(src, "_find_windows_template")
	var custom_idx: int = body.find("_custom_template_path")
	var engine_idx: int = body.find("find_export_template(")
	assert_gt(custom_idx, -1, "_find_windows_template consults the custom template option")
	assert_gt(engine_idx, custom_idx,
		"a configured custom template is preferred over the installed template")


func test_missing_custom_template_is_reported_as_itself() -> void:
	# A broken custom template path is the user's own explicit configuration, so
	# it must not be reported as (or silently treated as) a missing *installed*
	# template.
	var msg: String = ExportPlatform._missing_custom_template_message(
		false, "C:\\builds\\my_template.exe")
	assert_string_contains(msg, "C:\\builds\\my_template.exe",
		"the failing path is named")
	assert_string_contains(msg, "custom_template/release",
		"the offending option is named")
	var src := FileAccess.get_file_as_string(SCRIPT_PATH)
	var body := _func_body(src, "_has_valid_export_configuration")
	var custom_idx: int = body.find("_missing_custom_template_message")
	var installed_idx: int = body.find("_missing_template_message")
	assert_gt(custom_idx, -1, "validation reports a broken custom template")
	assert_gt(installed_idx, custom_idx,
		"a broken custom template is reported instead of the installed-template message")


func test_validation_reports_broken_custom_template() -> void:
	var src := FileAccess.get_file_as_string(SCRIPT_PATH)
	var body := _func_body(src, "_has_valid_export_configuration")
	assert_string_contains(body, "_missing_custom_template_message",
		"export validation surfaces a broken custom template before export starts")


func test_dotnet_project_detection_requires_assembly_name() -> void:
	var src := FileAccess.get_file_as_string(SCRIPT_PATH)
	var body := _func_body(src, "_is_dotnet_project")
	assert_string_contains(body, "_is_dotnet_editor()",
		"a C# project is only possible on a .NET editor build")
	assert_string_contains(body, "dotnet/project/assembly_name",
		"C# projects are detected via the assembly name Godot resolves for them")
	assert_string_contains(_func_body(src, "_is_dotnet_editor"), "CSharpScript",
		"only .NET editor builds expose CSharpScript")


func test_dotnet_missing_template_message_is_actionable() -> void:
	var msg: String = ExportPlatform._dotnet_requires_template_message(false)
	assert_string_contains(msg, ".NET",
		"the message names the .NET template package the user must install")
	assert_string_contains(msg, "Manage Export Templates",
		"the message points at the editor's template manager")
	assert_string_contains(msg, "assemblies not found",
		"the message ties the failure to the runtime symptom the user sees")
	assert_string_contains(ExportPlatform._missing_template_message(false, true), ".NET",
		"the shared template message can name the .NET flavor")


func test_dotnet_project_needs_a_real_template_up_front() -> void:
	var src := FileAccess.get_file_as_string(SCRIPT_PATH)
	var validation := _func_body(src, "_has_valid_export_configuration")
	assert_string_contains(validation, "_dotnet_requires_template_message",
		"export validation blocks a template-less C# export up front")
	assert_string_contains(validation, "set_config_missing_templates(true)",
		"the dialog is told to offer the Manage Export Templates shortcut")


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


# ── Delegation to the built-in Windows exporter ───────────────────────────
#
# The .exe and .pck used to be produced here: copy a template, stamp its icon
# with rcedit, stage the D3D12 redistributables, save_pack(), place the shared
# objects. None of that is GDK-specific (a GDK title on PC is an ordinary Win32
# application) and every step was a chance to drift from a plain
# `Windows Desktop` export — issues #134 and #144 were both instances of that
# drift, and Godot 4.5 removing rcedit outright broke the icon path completely.
#
# So the platform hands its own preset to EditorExportPlatformWindows and
# layers GDK packaging on the result. These tests pin that the in-house
# pipeline is really gone rather than merely bypassed.

func test_export_delegates_the_exe_and_pck_to_godot() -> void:
	var src := FileAccess.get_file_as_string(SCRIPT_PATH)
	var body := _func_body(src, "_run_windows_export")
	assert_string_contains(body, "WINDOWS_PLATFORM_CLASS",
		"the built-in Windows platform is what produces the binaries")
	assert_string_contains(body, "export_project(p_preset, p_debug, exe_path, p_flags)",
		"this platform's own preset is handed straight to the built-in exporter")
	assert_eq(ExportPlatform.WINDOWS_PLATFORM_CLASS, "EditorExportPlatformWindows",
		"the delegate is Godot's Windows exporter")


func test_export_project_delegates_before_layering_gdk_packaging() -> void:
	var src := FileAccess.get_file_as_string(SCRIPT_PATH)
	var body := _func_body(src, "_export_project")
	var delegate_at: int = body.find("_run_windows_export(")
	var dll_at: int = body.find("_copy_addon_dlls(")
	var config_at: int = body.find("_stage_microsoft_game_config(")
	var pack_at: int = body.find("_makepkg_pack(")
	assert_gt(delegate_at, -1, "the export delegates to the built-in Windows exporter")
	assert_gt(dll_at, delegate_at, "addon DLLs are staged onto the delegated output")
	assert_gt(config_at, dll_at, "MicrosoftGame.config is staged after the DLLs")
	assert_gt(pack_at, config_at, "packaging runs last")


func test_delegated_export_failure_aborts_the_package() -> void:
	var src := FileAccess.get_file_as_string(SCRIPT_PATH)
	var body := _func_body(src, "_export_project")
	assert_string_contains(body, "if delegate_err != OK:",
		"a failed Windows export must not be packaged")
	var run_body := _func_body(src, "_run_windows_export")
	assert_string_contains(run_body, "FileAccess.file_exists(exe_path)",
		"a success return is still verified against a real executable on disk")


func test_delegated_exporter_messages_reach_the_dialog() -> void:
	# The delegate is a bare instance, so its message log dies with it unless
	# it is copied across — losing exactly the diagnostics that explain a
	# failure the user is looking at.
	var src := FileAccess.get_file_as_string(SCRIPT_PATH)
	var body := _func_body(src, "_forward_messages")
	assert_string_contains(body, "get_message_count()",
		"every message the delegate logged is walked")
	assert_string_contains(body, "add_message(",
		"the delegate's messages are re-emitted on this platform")
	assert_string_contains(_func_body(src, "_run_windows_export"), "_forward_messages(windows)",
		"messages are forwarded whether the delegated export succeeded or not")


func test_in_house_export_pipeline_is_gone() -> void:
	var src := FileAccess.get_file_as_string(SCRIPT_PATH)
	# rcedit was removed from Godot in 4.5 (replaced by a native PE resource
	# writer that is not exposed to GDScript), so an rcedit-based icon path
	# cannot work on any version this addon supports.
	for helper: String in ["_resolve_rcedit_path", "_rcedit_args", "_modify_staged_executable"]:
		assert_false(src.contains(helper),
			"%s is removed, not merely unused — Godot 4.5 dropped rcedit" % helper)
	assert_false(src.contains("_write_ico"),
		"the hand-rolled .ico writer is gone; Godot stamps the icon natively")
	assert_false(src.contains("save_pack("),
		"the .pck is written by the built-in exporter")
	assert_false(src.contains("_stage_shared_objects"),
		"export-plugin shared objects (C# assemblies included) are placed by Godot")
	assert_false(src.contains("AGILITY_SDK_LIBS"),
		"the D3D12 Agility SDK / PIX redistributables are staged by Godot")
	assert_false(src.contains("OS.get_executable_path()"),
		"the editor binary is never substituted for an export template (issue #134)")


func test_gdk_feature_tag_survives_the_delegated_export() -> void:
	# Export feature tags come from the platform performing the export, so the
	# delegated run reports `windows`/`pc` and would silently drop `gdk`. An
	# EditorExportPlugin's features are consulted whichever platform is running.
	var src := FileAccess.get_file_as_string(SCRIPT_PATH)
	assert_string_contains(_func_body(src, "_run_windows_export"), "exporting_for_gdk = true",
		"the delegated export is flagged so the feature can be restored")
	assert_string_contains(_func_body(src, "_run_windows_export"), "exporting_for_gdk = false",
		"the flag is cleared again so a plain Windows export is unaffected")

	var plugin_src := FileAccess.get_file_as_string(FEATURES_PLUGIN_PATH)
	assert_ne(plugin_src, "", "the feature-restoring export plugin exists")
	assert_string_contains(plugin_src, "_get_export_features",
		"the plugin contributes export features")
	assert_string_contains(plugin_src, "exporting_for_gdk",
		"the plugin only contributes during a GDK export")
	assert_string_contains(plugin_src, "\"gdk\"", "the restored tag is `gdk`")

	var plugin_body := _func_body(plugin_src, "_get_export_features")
	assert_string_contains(plugin_body, "return PackedStringArray()",
		"a plain Windows Desktop export in the same session gets no gdk tag")


func test_features_plugin_is_registered_with_the_platform() -> void:
	var plugin_src := FileAccess.get_file_as_string(EDITOR_PLUGIN_PATH)
	assert_string_contains(plugin_src, "add_export_plugin(",
		"the feature-restoring plugin is registered")
	assert_string_contains(plugin_src, "remove_export_plugin(",
		"and removed again on _exit_tree, so disabling the addon leaves no trace")


# ── Export option coverage ────────────────────────────────────────────────
#
# Handing our preset to the built-in exporter only works because we declare
# every option it reads. A missing name reads back as null *inside Godot*, and
# the failure is neither obvious nor local: omitting `custom_template/debug`
# aborts the export with "Mismatching custom export template executable
# architecture: found 'invalid'".
#
# The expected set below is EditorExportPlatformWindows::get_export_options()
# plus its EditorExportPlatformPC base, which are identical on Godot 4.5 and
# 4.6. Regenerate by grepping `PropertyInfo(Variant::` out of
# platform/windows/export/export_plugin.cpp and
# editor/export/editor_export_platform_pc.cpp.

const GODOT_WINDOWS_EXPORT_OPTIONS: Array[String] = [
	# EditorExportPlatformPC
	"custom_template/debug",
	"custom_template/release",
	"debug/export_console_wrapper",
	"binary_format/embed_pck",
	"texture_format/s3tc_bptc",
	"texture_format/etc2_astc",
	"shader_baker/enabled",
	# EditorExportPlatformWindows
	"binary_format/architecture",
	"codesign/enable",
	"codesign/identity_type",
	"codesign/identity",
	"codesign/password",
	"codesign/timestamp",
	"codesign/timestamp_server_url",
	"codesign/digest_algorithm",
	"codesign/description",
	"codesign/custom_options",
	"application/modify_resources",
	"application/icon",
	"application/console_wrapper_icon",
	"application/icon_interpolation",
	"application/file_version",
	"application/product_version",
	"application/company_name",
	"application/product_name",
	"application/file_description",
	"application/copyright",
	"application/trademarks",
	"application/export_angle",
	"application/export_d3d12",
	"application/d3d12_agility_sdk_multiarch",
	"ssh_remote_deploy/enabled",
	"ssh_remote_deploy/host",
	"ssh_remote_deploy/port",
	"ssh_remote_deploy/extra_args_ssh",
	"ssh_remote_deploy/extra_args_scp",
	"ssh_remote_deploy/run_script",
	"ssh_remote_deploy/cleanup_script",
]


func test_every_builtin_windows_option_is_declared() -> void:
	var declared: PackedStringArray = ExportPlatform.declared_option_names()
	var missing: Array[String] = []
	for name: String in GODOT_WINDOWS_EXPORT_OPTIONS:
		if not (name in declared):
			missing.append(name)
	assert_eq(missing, [] as Array[String],
		"every option the built-in Windows exporter reads must be declared, or " +
		"it reads back as null inside Godot and fails the export in a way that " +
		"points nowhere near this platform")


func test_gdk_specific_options_are_declared() -> void:
	var declared: PackedStringArray = ExportPlatform.declared_option_names()
	for name: String in ["packaging/ekb_file", "dev/register_loose", "dev/sandbox_id"]:
		assert_true(name in declared, "%s is offered by this platform" % name)


func test_option_names_are_unique() -> void:
	var seen: Dictionary = {}
	for name: String in ExportPlatform.declared_option_names():
		assert_false(seen.has(name), "option %s is declared exactly once" % name)
		seen[name] = true


func test_declared_options_have_a_complete_shape() -> void:
	# EditorExportPlatformExtension wants a flat dict; the nested
	# {"option": {...}} shape used by EditorExportPlugin crashes the editor with
	# `Condition "!d.has("name")" is true`.
	for entry: Dictionary in ExportPlatform.export_option_list():
		assert_true(entry.has("name"), "each option carries a top-level name")
		assert_true(entry.has("type"), "each option carries a type")
		assert_true(entry.has("default_value"), "each option carries a default")
		assert_false(entry.has("option"),
			"the nested EditorExportPlugin shape is rejected by the extension API")


# ── Silent disabled Export button ─────────────────────────────────────────
#
# Godot greys out "Export Project…" / "Export All…" purely on the boolean the
# validation callbacks return, and shows no explanation unless the platform
# reports one through set_config_error(). Returning a bare `false` therefore
# leaves the user with a dead button and no diagnosis.

func test_validation_messages_are_actionable() -> void:
	assert_string_contains(ExportPlatform._gdk_missing_message(), "winget install Microsoft.Gaming.GDK",
		"the GDK-missing message names the install command")

	var config_msg: String = ExportPlatform._missing_game_config_message("C:/proj/MicrosoftGame.config")
	assert_string_contains(config_msg, "C:/proj/MicrosoftGame.config",
		"the game-config message names the exact path that was probed")
	assert_string_contains(config_msg, "Create Game Config",
		"the game-config message points at the editor menu that authors it")
	assert_string_contains(config_msg, "gdk/packaging/game_config_dir",
		"the game-config message names the Project Setting that relocates it")

	assert_string_contains(ExportPlatform._missing_template_message(false), "release",
		"the template message names the failing configuration")
	assert_string_contains(ExportPlatform._missing_template_message(true), "debug",
		"the debug variant names the debug configuration")
	assert_string_contains(ExportPlatform._missing_template_message(false), "Manage Export Templates",
		"the template message points at the editor's template manager")


func test_has_valid_export_configuration_reports_every_blocker() -> void:
	var src := FileAccess.get_file_as_string(SCRIPT_PATH)
	var body := _func_body(src, "_has_valid_export_configuration")
	assert_ne(body, "", "_has_valid_export_configuration() is defined")
	assert_string_contains(body, "set_config_error(",
		"validation surfaces its reason via set_config_error()")
	for message_fn: String in ["_gdk_missing_message", "_missing_game_config_message", "_missing_template_message"]:
		assert_string_contains(body, message_fn,
			"%s() feeds the export-dialog error text" % message_fn)
	assert_string_contains(body, "set_config_missing_templates(true)",
		"a missing export template is flagged so the dialog offers the template manager")
	# The .exe is produced by Godot's own exporter now, so there is no
	# editor-binary stand-in left and loose dev-register is no longer exempt
	# from the export-template requirement.
	assert_false(body.contains("dev/register_loose"),
		"a missing export template blocks loose dev-register too")


func test_has_valid_project_configuration_reports_missing_gdk() -> void:
	var src := FileAccess.get_file_as_string(SCRIPT_PATH)
	var body := _func_body(src, "_has_valid_project_configuration")
	assert_string_contains(body, "set_config_error(",
		"project validation surfaces its reason via set_config_error()")
	assert_string_contains(body, "_gdk_missing_message",
		"the missing-GDK reason is shared with export validation")


# ── Texture-format feature tags (issue #144 follow-up) ────────────────────
#
# Godot matches a preset's feature tags against the `path.<feature>` keys in
# every imported resource's .import file and ERASES any remap whose feature is
# missing. A VRAM-compressed texture's only variant is `path.s3tc`, so a
# platform that never reports "s3tc" silently dropped every such texture from
# the .pck: the package came out a fraction of its real size and the game died
# at load with "Can't load dependency: res://...". The tags must be derived
# from the preset exactly the way EditorExportPlatformPC derives them.

func test_texture_format_features_default_desktop_set() -> void:
	var features := ExportPlatform._texture_format_features(true, false)
	assert_true(features.has("s3tc"),
		"the default desktop preset reports s3tc, or every VRAM-compressed texture is dropped")
	assert_true(features.has("bptc"), "s3tc_bptc reports bptc alongside s3tc")
	assert_false(features.has("etc2"), "etc2 is not reported unless etc2_astc is enabled")
	assert_false(features.has("astc"), "astc is not reported unless etc2_astc is enabled")


func test_texture_format_features_mobile_set() -> void:
	var features := ExportPlatform._texture_format_features(false, true)
	assert_true(features.has("etc2"), "etc2_astc reports etc2")
	assert_true(features.has("astc"), "etc2_astc reports astc")
	assert_false(features.has("s3tc"), "s3tc is not reported when s3tc_bptc is off")


func test_texture_format_features_can_report_both() -> void:
	var features := ExportPlatform._texture_format_features(true, true)
	for tag: String in ["s3tc", "bptc", "etc2", "astc"]:
		assert_true(features.has(tag), "%s is reported when both toggles are on" % tag)


func test_texture_format_features_none_when_both_disabled() -> void:
	assert_eq(ExportPlatform._texture_format_features(false, false).size(), 0,
		"disabling both toggles reports no texture-format tags")


func test_preset_features_derive_texture_tags_from_the_preset() -> void:
	var src := FileAccess.get_file_as_string(SCRIPT_PATH)
	var body := _func_body(src, "_get_preset_features")
	assert_string_contains(body, "_texture_format_features(",
		"_get_preset_features() reports the texture-format tags")
	assert_string_contains(body, "texture_format/s3tc_bptc",
		"the s3tc/bptc tags follow the preset toggle")
	assert_string_contains(body, "texture_format/etc2_astc",
		"the etc2/astc tags follow the preset toggle")


func test_texture_format_options_exist_with_pc_defaults() -> void:
	var src := FileAccess.get_file_as_string(SCRIPT_PATH)
	var body := _func_body(src, "export_option_list")
	assert_string_contains(body, '_opt("texture_format/s3tc_bptc", TYPE_BOOL, true',
		"s3tc_bptc defaults to true, matching EditorExportPlatformPC")
	assert_string_contains(body, '_opt("texture_format/etc2_astc", TYPE_BOOL, false',
		"etc2_astc defaults to false, matching EditorExportPlatformPC")


func test_preset_bool_falls_back_for_presets_predating_the_option() -> void:
	# An export_presets.cfg written before an option existed has no value for
	# it; the default must still apply rather than reading as `false`.
	assert_true(ExportPlatform._preset_bool(null, "texture_format/s3tc_bptc", true),
		"a missing preset value falls back to the option default")
	assert_false(ExportPlatform._preset_bool(null, "texture_format/etc2_astc", false),
		"the fallback is the option's own default, not a hardcoded true")



# ── Platform feature tags mirror a plain Win32 target ────────────────────
#
# A GDK title on PC is an ordinary Win32 application running the stock Godot
# Windows export template. The platform used to also report "xbox" and "d3d12",
# which Godot defines for no platform, so the same project packaged through
# gdkpkg (which drives the built-in Windows Desktop preset) resolved
# OS.has_feature() checks, `setting.<tag>` ProjectSettings overrides and
# `.import` remaps differently than it did here. That is the same class of
# silent divergence as issue #144.

func test_platform_features_match_godot_pc_plus_gdk() -> void:
	var src := FileAccess.get_file_as_string(SCRIPT_PATH)
	var body := _func_body(src, "_get_platform_features")
	assert_ne(body, "", "_get_platform_features() is defined")
	# EditorExportPlatformPC reports "pc" plus the lowercased OS name.
	for tag: String in ["\"pc\"", "\"windows\""]:
		assert_string_contains(body, tag,
			"%s is reported, matching EditorExportPlatformPC" % tag)
	assert_string_contains(body, "\"gdk\"",
		"the one platform-specific tag, gdk, is still reported")


func test_invented_platform_feature_tags_are_gone() -> void:
	var src := FileAccess.get_file_as_string(SCRIPT_PATH)
	var body := _func_body(src, "_get_platform_features")
	assert_false(body.contains("\"xbox\""),
		"the invented xbox tag is gone — GDK on PC is a plain Win32 target")
	assert_false(body.contains("\"d3d12\""),
		"the invented d3d12 tag is gone — Godot defines no such platform tag")


func test_architecture_tag_is_reported_with_the_preset() -> void:
	# Godot puts the architecture in get_preset_features(), not the platform
	# feature list; dropping it from one without the other would erase it from
	# the exported feature set entirely.
	var src := FileAccess.get_file_as_string(SCRIPT_PATH)
	assert_string_contains(_func_body(src, "_get_preset_features"), "x86_64",
		"the architecture tag is carried by the preset features, as in Godot")


# ── Export option visibility ─────────────────────────────────────────────
#
# Visibility is cosmetic — every option stays declared and readable by the
# built-in exporter — but it has to mirror Godot's grouping or the inspector
# shows a wall of fields that a `Windows Desktop` preset keeps collapsed.

func test_d3d12_options_are_offered_with_godot_defaults() -> void:
	var src := FileAccess.get_file_as_string(SCRIPT_PATH)
	var body := _func_body(src, "export_option_list")
	assert_string_contains(body, "application/export_d3d12",
		"the export_d3d12 mode option is offered")
	assert_string_contains(body, "Auto,Yes,No",
		"the mode option uses Godot's Auto/Yes/No enum")
	assert_string_contains(body, "application/d3d12_agility_sdk_multiarch",
		"the multiarch option is offered")


func test_renderer_options_stay_visible_without_resource_modification() -> void:
	# These share the `application/` prefix with the executable-resource fields
	# but describe DLL staging, so turning off resource modification must not
	# hide them — Godot exempts the same three.
	var src := FileAccess.get_file_as_string(SCRIPT_PATH)
	var body := _func_body(src, "_get_export_option_visibility")
	assert_string_contains(body, "NON_RESOURCE_APPLICATION_OPTIONS",
		"the renderer options are exempt from the modify_resources collapse")
	for name: String in ["application/export_angle", "application/export_d3d12",
			"application/d3d12_agility_sdk_multiarch"]:
		assert_string_contains(body, name, "%s is exempt, as in Godot" % name)


func test_codesign_and_ssh_options_collapse_behind_their_toggle() -> void:
	var src := FileAccess.get_file_as_string(SCRIPT_PATH)
	var body := _func_body(src, "_get_export_option_visibility")
	assert_string_contains(body, "codesign/enable",
		"the codesign fields collapse behind codesign/enable, as in Godot")
	assert_string_contains(body, "ssh_remote_deploy/enabled",
		"the SSH fields collapse behind ssh_remote_deploy/enabled, as in Godot")



# ── Export dialog diagnostics ────────────────────────────────────────────
#
# push_error()/push_warning() only reach the Output panel, which the export
# dialog does not surface — an export could fail with the dialog showing
# nothing at all. EditorExportPlatform.add_message() feeds the dialog's own
# message log, exactly as the built-in exporters do.

func test_export_diagnostics_reach_the_export_dialog() -> void:
	var src := FileAccess.get_file_as_string(SCRIPT_PATH)
	var err_body := _func_body(src, "_report_error")
	var warn_body := _func_body(src, "_report_warning")
	assert_string_contains(err_body, "EXPORT_MESSAGE_ERROR",
		"errors are added to the dialog log at error severity")
	assert_string_contains(warn_body, "EXPORT_MESSAGE_WARNING",
		"warnings are added to the dialog log at warning severity")
	# Console output is kept so headless/CI runs still show the reason.
	assert_string_contains(err_body, "push_error(",
		"errors still reach the Output panel for headless runs")
	assert_string_contains(warn_body, "push_warning(",
		"warnings still reach the Output panel for headless runs")


func test_export_starts_from_a_clean_message_log() -> void:
	var src := FileAccess.get_file_as_string(SCRIPT_PATH)
	var body := _func_body(src, "_export_project")
	assert_string_contains(body, "clear_messages()",
		"a new export clears messages left over from the previous run")


func test_export_pipeline_reports_through_the_dialog_helpers() -> void:
	# Every failure the export pipeline can hit must be visible in the dialog,
	# so the pipeline functions route through the helpers rather than calling
	# push_error()/push_warning() directly.
	var src := FileAccess.get_file_as_string(SCRIPT_PATH)
	for fn: String in ["_export_project", "_stage_microsoft_game_config",
			"_copy_addon_dlls", "_run_windows_export", "_wdapp_register"]:
		var body := _func_body(src, fn)
		assert_ne(body, "", "%s() is defined" % fn)
		assert_false(body.contains("push_error("),
			"%s() reports errors through _report_error()" % fn)


func test_dialog_messages_do_not_repeat_the_category() -> void:
	# add_message() renders the category beside the message, so a message that
	# already opens with it reads "GDK Export: GDK Export: ...". Observed in a
	# real export before the prefix was stripped.
	assert_eq(ExportPlatform._strip_category("GDK Export: wdapp register failed"),
		"wdapp register failed",
		"the redundant category prefix is removed for the dialog")
	assert_eq(ExportPlatform._strip_category("makepkg pack failed"),
		"makepkg pack failed",
		"a message without the prefix is passed through untouched")


func test_console_output_keeps_the_category_prefix() -> void:
	# The prefix is what makes these lines findable in the Output panel, so only
	# the dialog copy is stripped.
	var src := FileAccess.get_file_as_string(SCRIPT_PATH)
	for fn: String in ["_report_error", "_report_warning"]:
		var body := _func_body(src, fn)
		var push_idx: int = body.find("push_")
		var strip_idx: int = body.find("_strip_category(")
		assert_gt(push_idx, -1, "%s() writes to the console" % fn)
		assert_gt(strip_idx, push_idx,
			"%s() strips the prefix only for the dialog, not the console" % fn)
