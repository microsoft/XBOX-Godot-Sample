extends SceneTree
## Wave 4 bootstrap mini-runner.
##
## Asserts: when `gdk/runtime/initialize_on_startup` is `false`, the
## `GDKBootstrap` autoload does NOT call `GDK.initialize()` during `_ready()`.

const SETTING_INITIALIZE_ON_STARTUP := "gdk/runtime/initialize_on_startup"
const SETTING_AUTO_ADD_PRIMARY_USER := "gdk/runtime/auto_add_primary_user"
const SCENARIO := "initialize_on_startup_false"
const STARTUP_BUDGET_MSEC := 4000


func _initialize() -> void:
	# SceneTree._initialize() runs before autoloads attach. Flip the project
	# setting to false here; autoload _ready() will observe the new value.
	ProjectSettings.set_setting(SETTING_INITIALIZE_ON_STARTUP, false)
	# Disable auto-add-user too so we don't race with sign-in side effects.
	ProjectSettings.set_setting(SETTING_AUTO_ADD_PRIMARY_USER, false)
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
	if gdk.is_initialized():
		printerr("BOOTSTRAP_FAIL: %s -- GDK.is_initialized() == true; autoload should have skipped init when initialize_on_startup=false" % SCENARIO)
		quit(4)
		return
	print("BOOTSTRAP_OK: %s (GDK.is_initialized() == false after autoload)" % SCENARIO)
	quit(0)
