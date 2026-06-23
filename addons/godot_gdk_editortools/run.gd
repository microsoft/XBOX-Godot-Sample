@tool
class_name GdkEditorToolsRunner
extends SceneTree
## Headless entry point for `addons/godot_gdk_editortools`.
##
## Three equivalent invocations:
##   godot --headless -s res://addons/godot_gdk_editortools/run.gd -- <verb> [flags]
##   godot --headless --main-loop GdkEditorToolsRunner -- <verb> [flags]
##   addons\godot_gdk_editortools\gdkpkg.cmd <verb> [flags]   (Windows)
##   addons/godot_gdk_editortools/gdkpkg.sh  <verb> [flags]   (POSIX)
##
## Always prints a one-line summary; emits a single
## `PACKAGING_RESULT_JSON:<json>` line unless `--no-json` is supplied. The
## process exit code mirrors the verb's [code]EditorToolsResult.exit_code[/code].

const EditorToolsCli = preload("res://addons/godot_gdk_editortools/core/editortools_cli.gd")
const EditorToolsConfig = preload("res://addons/godot_gdk_editortools/core/editortools_config.gd")
const EditorToolsResult = preload("res://addons/godot_gdk_editortools/core/editortools_result.gd")
const EditorToolsService = preload("res://addons/godot_gdk_editortools/core/editortools_service.gd")


func _init() -> void:
	# Defer to the next idle frame so SceneTree finishes initialising before
	# we call quit(). This keeps both --script and --main-loop happy.
	var exit_code: int = _execute()
	# `SceneTree.quit(code)` requests an orderly shutdown.
	quit(exit_code)


func _execute() -> int:
	var argv: PackedStringArray = OS.get_cmdline_user_args()
	var parsed: Dictionary = EditorToolsCli.parse(argv)
	var emit_json: bool = not bool(parsed["options"].get("no-json", false))

	if not parsed["ok"]:
		printerr("[editortools] error: %s" % parsed["error"])
		printerr(EditorToolsCli.render_usage())
		var err_result: Dictionary = EditorToolsResult.fail(
			str(parsed.get("verb", "")),
			str(parsed["error"]),
			int(parsed.get("error_code", EditorToolsResult.EXIT_USAGE)))
		_print_summary(err_result)
		if emit_json:
			print(EditorToolsResult.to_json_line(err_result))
		return err_result["exit_code"]

	if bool(parsed["help"]) and str(parsed["verb"]).is_empty():
		print(EditorToolsCli.render_usage())
		return EditorToolsResult.EXIT_OK

	if bool(parsed["help"]):
		print(EditorToolsCli.render_verb_usage(str(parsed["verb"])))
		return EditorToolsResult.EXIT_OK

	var config_override: String = str(parsed["options"].get("config", ""))
	var resolved: Dictionary = EditorToolsConfig.resolve(parsed["options"], "",
		EditorToolsConfig.PACKAGING_SETTINGS_PATH, config_override)
	var service: RefCounted = EditorToolsService.new()
	var result: Dictionary = service.dispatch(str(parsed["verb"]), resolved)
	_print_summary(result)
	if emit_json:
		print(EditorToolsResult.to_json_line(result))
	return int(result.get("exit_code", EditorToolsResult.EXIT_FAIL))


static func _print_summary(result: Dictionary) -> void:
	var status: String = "ok" if bool(result.get("ok", false)) else "fail"
	var verb: String = str(result.get("verb", ""))
	var duration: int = int(result.get("duration_ms", 0))
	var message: String = str(result.get("message", ""))
	print("[editortools] %s %s in %dms: %s" % [verb, status, duration, message])
