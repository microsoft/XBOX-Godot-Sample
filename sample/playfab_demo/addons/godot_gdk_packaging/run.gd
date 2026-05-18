@tool
class_name GdkPackagingRunner
extends SceneTree
## Headless entry point for `addons/godot_gdk_packaging`.
##
## Three equivalent invocations:
##   godot --headless -s res://addons/godot_gdk_packaging/run.gd -- <verb> [flags]
##   godot --headless --main-loop GdkPackagingRunner -- <verb> [flags]
##   addons\godot_gdk_packaging\gdkpkg.cmd <verb> [flags]   (Windows)
##   addons/godot_gdk_packaging/gdkpkg.sh  <verb> [flags]   (POSIX)
##
## Always prints a one-line summary; emits a single
## `PACKAGING_RESULT_JSON:<json>` line unless `--no-json` is supplied. The
## process exit code mirrors the verb's [code]PackagingResult.exit_code[/code].

const PackagingCli = preload("res://addons/godot_gdk_packaging/core/packaging_cli.gd")
const PackagingConfig = preload("res://addons/godot_gdk_packaging/core/packaging_config.gd")
const PackagingResult = preload("res://addons/godot_gdk_packaging/core/packaging_result.gd")
const PackagingService = preload("res://addons/godot_gdk_packaging/core/packaging_service.gd")


func _init() -> void:
	# Defer to the next idle frame so SceneTree finishes initialising before
	# we call quit(). This keeps both --script and --main-loop happy.
	var exit_code: int = _execute()
	# `SceneTree.quit(code)` requests an orderly shutdown.
	quit(exit_code)


func _execute() -> int:
	var argv: PackedStringArray = OS.get_cmdline_user_args()
	var parsed: Dictionary = PackagingCli.parse(argv)
	var emit_json: bool = not bool(parsed["options"].get("no-json", false))

	if not parsed["ok"]:
		printerr("[packaging] error: %s" % parsed["error"])
		printerr(PackagingCli.render_usage())
		var err_result: Dictionary = PackagingResult.fail(
			str(parsed.get("verb", "")),
			str(parsed["error"]),
			int(parsed.get("error_code", PackagingResult.EXIT_USAGE)))
		_print_summary(err_result)
		if emit_json:
			print(PackagingResult.to_json_line(err_result))
		return err_result["exit_code"]

	if bool(parsed["help"]) and str(parsed["verb"]).is_empty():
		print(PackagingCli.render_usage())
		return PackagingResult.EXIT_OK

	if bool(parsed["help"]):
		print(PackagingCli.render_verb_usage(str(parsed["verb"])))
		return PackagingResult.EXIT_OK

	var resolved: Dictionary = PackagingConfig.resolve(parsed["options"])
	var service: RefCounted = PackagingService.new()
	var result: Dictionary = service.dispatch(str(parsed["verb"]), resolved)
	_print_summary(result)
	if emit_json:
		print(PackagingResult.to_json_line(result))
	return int(result.get("exit_code", PackagingResult.EXIT_FAIL))


static func _print_summary(result: Dictionary) -> void:
	var status: String = "ok" if bool(result.get("ok", false)) else "fail"
	var verb: String = str(result.get("verb", ""))
	var duration: int = int(result.get("duration_ms", 0))
	var message: String = str(result.get("message", ""))
	print("[packaging] %s %s in %dms: %s" % [verb, status, duration, message])
