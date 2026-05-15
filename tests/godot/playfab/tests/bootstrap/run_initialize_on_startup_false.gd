extends SceneTree
## PlayFab bootstrap mini-runner.
##
## Asserts: when `playfab/runtime/initialize_on_startup` is `false`, the
## `PlayFabBootstrap` autoload does NOT call `PlayFab.initialize()` during
## `_ready()`.

const SETTING_INITIALIZE_ON_STARTUP := "playfab/runtime/initialize_on_startup"
const SCENARIO := "initialize_on_startup_false"
const STARTUP_BUDGET_MSEC := 4000


func _initialize() -> void:
	# SceneTree._initialize() runs before autoloads attach. Flip the project
	# setting to false here; autoload _ready() will observe the new value.
	ProjectSettings.set_setting(SETTING_INITIALIZE_ON_STARTUP, false)
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
	if playfab.is_initialized():
		printerr("BOOTSTRAP_FAIL: %s -- PlayFab.is_initialized() == true; autoload should have skipped init when initialize_on_startup=false" % SCENARIO)
		quit(4)
		return
	var bootstrap = root.get_node_or_null("PlayFabBootstrap")
	if bootstrap != null and bootstrap.get_last_initialize_result() != null:
		printerr("BOOTSTRAP_FAIL: %s -- autoload recorded an initialize_result; it should not have attempted init" % SCENARIO)
		quit(5)
		return
	print("BOOTSTRAP_OK: %s (PlayFab.is_initialized() == false after autoload)" % SCENARIO)
	quit(0)
