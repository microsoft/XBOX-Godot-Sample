extends SceneTree
## Child-process probe for `tests/bootstrap/run_singleton_name_override.gd`.
##
## Launched in a nested headless Godot process after that mini-runner writes an
## `override.cfg` renaming the PlayFab engine singleton. Prints a single `PROBE`
## line for the parent to parse and never asserts on its own — the parent owns
## the pass/fail decision so it can always clean the `override.cfg` back up.
##
## Not named `test_*.gd`, so GUT's `-gdir=res://tests` discovery skips it, and
## it lives outside `tests/bootstrap/` so the orchestrator does not run it as a
## top-level mini-runner.

const SETTING_SINGLETON_NAME := "playfab/runtime/singleton_name"


func _initialize() -> void:
	var configured := str(ProjectSettings.get_setting(SETTING_SINGLETON_NAME, "<unset>"))
	print("PROBE setting=%s configured_registered=%s default_registered=%s" % [
		configured,
		str(Engine.has_singleton(configured)),
		str(Engine.has_singleton("PlayFab")),
	])
	quit(0)
