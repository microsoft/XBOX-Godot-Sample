extends SceneTree
## Bootstrap mini-runner: end-to-end singleton rename.
##
## Asserts: `game_input/runtime/singleton_name` actually changes the Engine
## singleton name the `godot_gameinput` GDExtension registers.
##
## The setting is read once during extension initialization, which happens
## before any script in the project runs — so this cannot be exercised in-process.
## Instead we write an `override.cfg` (Godot's supported project-setting
## override file, read at engine startup) and relaunch Godot headless against
## this same project, then parse the child's `PROBE` line.
##
## The `override.cfg` is removed immediately after the blocking child call
## returns, and again on entry, so a crashed earlier run cannot poison later
## runs of this host.
##
## Invoked by `tools/run_all_tests.ps1`'s bootstrap stage.
## Exit code: 0 on pass, non-zero on fail.

const SCENARIO := "singleton_name_override"
const OVERRIDE_PATH := "res://override.cfg"
const OVERRIDE_SECTION := "game_input"
const OVERRIDE_KEY := "runtime/singleton_name"
const DEFAULT_SINGLETON_NAME := "GameInput"
const RENAMED_SINGLETON_NAME := "GameInputSingletonRenameProbe"
const PROBE_SCRIPT := "res://tests/support/probe_singleton_name.gd"


func _initialize() -> void:
	if not Engine.has_singleton(DEFAULT_SINGLETON_NAME):
		printerr("BOOTSTRAP_FAIL: %s -- '%s' singleton not registered in the parent host; is the extension built?"
				% [SCENARIO, DEFAULT_SINGLETON_NAME])
		quit(2)
		return

	_remove_override()

	var config := ConfigFile.new()
	config.set_value(OVERRIDE_SECTION, OVERRIDE_KEY, RENAMED_SINGLETON_NAME)
	var save_error := config.save(OVERRIDE_PATH)
	if save_error != OK:
		printerr("BOOTSTRAP_FAIL: %s -- could not write %s (error %d)" % [SCENARIO, OVERRIDE_PATH, save_error])
		quit(3)
		return

	var output: Array = []
	# `-- --gd-script-check` keeps the child's bootstrap autoload from booting
	# the runtime; this scenario only cares about singleton registration.
	var child_exit := OS.execute(OS.get_executable_path(), [
		"--headless",
		"--path", ProjectSettings.globalize_path("res://"),
		"--script", PROBE_SCRIPT,
		"--", "--gd-script-check",
	], output, true)

	_remove_override()

	if child_exit != 0:
		printerr("BOOTSTRAP_FAIL: %s -- probe process exited %d" % [SCENARIO, child_exit])
		quit(4)
		return

	var probe_line := _find_probe_line(output)
	if probe_line.is_empty():
		printerr("BOOTSTRAP_FAIL: %s -- probe process emitted no PROBE line" % SCENARIO)
		quit(5)
		return

	var expected := "PROBE setting=%s configured_registered=true default_registered=false" % RENAMED_SINGLETON_NAME
	if probe_line != expected:
		printerr("BOOTSTRAP_FAIL: %s -- expected '%s', got '%s'" % [SCENARIO, expected, probe_line])
		quit(6)
		return

	print("BOOTSTRAP_OK: %s (%s registered as '%s' via override.cfg)"
			% [SCENARIO, DEFAULT_SINGLETON_NAME, RENAMED_SINGLETON_NAME])
	quit(0)


func _remove_override() -> void:
	if FileAccess.file_exists(OVERRIDE_PATH):
		# Globalized: DirAccess.remove_absolute() is documented against filesystem
		# paths, so don't rely on res:// being resolved for us here.
		DirAccess.remove_absolute(ProjectSettings.globalize_path(OVERRIDE_PATH))


func _find_probe_line(output: Array) -> String:
	for chunk in output:
		for line in str(chunk).split("\n"):
			var trimmed := line.strip_edges()
			if trimmed.begins_with("PROBE "):
				return trimmed
	return ""
