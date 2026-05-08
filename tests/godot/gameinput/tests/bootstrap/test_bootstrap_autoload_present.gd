extends SceneTree
## Wave 4 — Bootstrap mini-runner: GameInputBootstrap autoload PRESENT.
##
## Run via:
##   godot --headless --script res://tests/bootstrap/test_bootstrap_autoload_present.gd
##
## Asserts (on a headless host with no GameInput devices):
##   * The `GameInputBootstrap` autoload node is registered and reachable at
##     `/root/GameInputBootstrap`.
##   * It is wired to `res://addons/godot_gameinput/runtime/gameinput_bootstrap.gd`.
##   * The `GameInput` singleton it depends on is registered.
##   * The autoload survived its own `_ready()` (so any InputMap mutation /
##     default-mapper spawn it would have done did not crash the boot).
##   * Calling `GameInput.poll()` (which the autoload also calls in `_process`
##     when `auto_poll` is on) is safe to invoke from script.
##
## Exits 0 on success and prints `BOOTSTRAP_AUTOLOAD_PRESENT: PASS` as the
## final status line. On any failure it prints `BOOTSTRAP_AUTOLOAD_PRESENT: FAIL`
## with a reason and exits 1.

const STATUS_TAG := "BOOTSTRAP_AUTOLOAD_PRESENT"
const AUTOLOAD_NODE_NAME := "GameInputBootstrap"
const AUTOLOAD_SCRIPT_PATH := "res://addons/godot_gameinput/runtime/gameinput_bootstrap.gd"


func _find_autoload() -> Node:
	# Autoloads sit directly under root. Avoid get_node_or_null with absolute
	# paths because that errors when the SceneTree isn't fully active yet
	# (e.g. inside a `--script` entry point's first idle frame).
	for child in root.get_children():
		if child.name == AUTOLOAD_NODE_NAME:
			return child
	return null


func _initialize() -> void:
	# Defer one idle frame so the SceneTree finishes wiring its autoload
	# children before we inspect them.
	await process_frame
	var failures: PackedStringArray = PackedStringArray()

	var autoload := _find_autoload()
	if autoload == null:
		failures.append("GameInputBootstrap autoload missing under root")
	else:
		var script: Script = autoload.get_script()
		if script == null or script.resource_path != AUTOLOAD_SCRIPT_PATH:
			failures.append(
				"GameInputBootstrap script mismatch: expected '%s' got '%s'" %
					[AUTOLOAD_SCRIPT_PATH,
					 ("(none)" if script == null else script.resource_path)])

	if not Engine.has_singleton("GameInput"):
		failures.append("Engine.has_singleton('GameInput') == false")
	else:
		var gi: Object = Engine.get_singleton("GameInput")
		if gi == null:
			failures.append("Engine.get_singleton('GameInput') returned null")
		else:
			# Calling poll() must be safe regardless of init state. The
			# bootstrap autoload is in process mode, so this exercises the
			# same call site it would hit in `_process`.
			gi.poll()

	if failures.is_empty():
		print("%s: PASS" % STATUS_TAG)
		quit(0)
	else:
		for line in failures:
			push_error("%s: %s" % [STATUS_TAG, line])
		print("%s: FAIL (%d issue%s)" %
				[STATUS_TAG, failures.size(),
				 ("" if failures.size() == 1 else "s")])
		quit(1)
