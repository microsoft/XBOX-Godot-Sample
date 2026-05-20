extends SceneTree
## PlayFab bootstrap mini-runner.
##
## Asserts: when `playfab/runtime/initialize_on_startup` is `true`, the
## `PlayFabBootstrap` autoload calls `PlayFab.initialize()` during `_ready()`.
##
## Invoked by `tools/run_all_tests.ps1`'s bootstrap stage as a fresh Godot
## child process via `--headless --script res://tests/bootstrap/<file>.gd`.
## Exit code: 0 on pass, non-zero on fail.
##
## On the test host `playfab/runtime/title_id` is intentionally blank, so PlayFab
## initialization will commonly fail with `title_id_required` -- but the
## autoload MUST still have *attempted* init. We check
## `bootstrap.get_last_initialize_result()` to distinguish "tried and
## failed" (a pass for this scenario) from "never tried" (a fail).

const SETTING_INITIALIZE_ON_STARTUP := "playfab/runtime/initialize_on_startup"
const SCENARIO := "initialize_on_startup_true"
const STARTUP_BUDGET_MSEC := 5000


func _initialize() -> void:
	# Force the project setting to true BEFORE autoloads load. The PlayFab
	# test host's project.godot already defaults this to true; we set it
	# explicitly so the assertion below is invariant to project.godot drift.
	ProjectSettings.set_setting(SETTING_INITIALIZE_ON_STARTUP, true)
	create_timer(float(STARTUP_BUDGET_MSEC) / 1000.0).timeout.connect(Callable(self, "_finish"))


func _finish() -> void:
	if not Engine.has_singleton("PlayFab"):
		printerr("BOOTSTRAP_FAIL: %s -- PlayFab singleton not available in mini-runner host" % SCENARIO)
		quit(2)
		return
	var playfab = Engine.get_singleton("PlayFab")
	if playfab == null:
		printerr("BOOTSTRAP_FAIL: %s -- Engine.get_singleton('PlayFab') returned null" % SCENARIO)
		quit(3)
		return
	var bootstrap = root.get_node_or_null("PlayFabBootstrap")
	if bootstrap == null:
		printerr("BOOTSTRAP_FAIL: %s -- PlayFabBootstrap autoload not registered under root" % SCENARIO)
		quit(5)
		return
	if not playfab.is_initialized():
		var init_result = bootstrap.get_last_initialize_result()
		if init_result == null:
			printerr("BOOTSTRAP_FAIL: %s -- autoload did not attempt PlayFab.initialize() (no result recorded)" % SCENARIO)
			quit(4)
			return
		print("BOOTSTRAP_OK: %s (autoload attempted init; result=%s)" % [SCENARIO, init_result.code])
		quit(0)
		return
	print("BOOTSTRAP_OK: %s (PlayFab.is_initialized() == true after autoload)" % SCENARIO)
	quit(0)
