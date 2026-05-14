extends SceneTree
## Wave 4 bootstrap mini-runner.
##
## Asserts: when `gdk/runtime/initialize_on_startup` is `true`, the
## `GDKBootstrap` autoload calls `GDK.initialize()` during `_ready()`.
##
## Invoked by `tools/run_all_tests.ps1`'s bootstrap stage as a fresh Godot
## child process via `--headless --script res://tests/bootstrap/<file>.gd`.
## Exit code: 0 on pass, non-zero on fail.

const SETTING_INITIALIZE_ON_STARTUP := "gdk/runtime/initialize_on_startup"
const SCENARIO := "initialize_on_startup_true"
const STARTUP_BUDGET_MSEC := 5000


func _initialize() -> void:
	# Force the project setting to true BEFORE autoloads load. The GDK test host's
	# project.godot already defaults this to true; we set it explicitly so
	# the assertion below is invariant to project.godot drift.
	ProjectSettings.set_setting(SETTING_INITIALIZE_ON_STARTUP, true)
	# Always exit by the end of the budget. We can't reliably hook the
	# autoload's `_ready()` from here (autoloads are added under the root
	# during the engine's main-loop init), so we use a fixed budget instead.
	create_timer(float(STARTUP_BUDGET_MSEC) / 1000.0).timeout.connect(Callable(self, "_finish"))


func _finish() -> void:
	if not Engine.has_singleton("GDK"):
		printerr("BOOTSTRAP_FAIL: %s -- GDK singleton not available in mini-runner host" % SCENARIO)
		quit(2)
		return
	var gdk = Engine.get_singleton("GDK")
	if gdk == null:
		printerr("BOOTSTRAP_FAIL: %s -- Engine.get_singleton('GDK') returned null" % SCENARIO)
		quit(3)
		return
	var bootstrap = root.get_node_or_null("GDKBootstrap")
	if bootstrap == null:
		printerr("BOOTSTRAP_FAIL: %s -- GDKBootstrap autoload not registered under root" % SCENARIO)
		quit(5)
		return
	if not gdk.is_initialized():
		# An init failure on this dev machine is acceptable (e.g. no GDK
		# runtime), but the autoload MUST have attempted init. We check
		# get_last_initialize_result() (cached by the autoload) to
		# distinguish "tried and failed" from "never tried". Either is a
		# pass for this scenario; "never tried" is a fail.
		var init_result = bootstrap.get_last_initialize_result()
		if init_result == null:
			printerr("BOOTSTRAP_FAIL: %s -- autoload did not attempt GDK.initialize() (no result recorded)" % SCENARIO)
			quit(4)
			return
		print("BOOTSTRAP_OK: %s (autoload attempted init; result=%s)" % [SCENARIO, init_result.code])
		quit(0)
		return
	print("BOOTSTRAP_OK: %s (GDK.is_initialized() == true after autoload)" % SCENARIO)
	quit(0)
