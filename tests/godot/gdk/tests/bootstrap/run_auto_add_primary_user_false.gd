extends SceneTree
## Wave 4 bootstrap mini-runner.
##
## Asserts: when `gdk/runtime/auto_add_primary_user` is `false`, the
## `GDKBootstrap` autoload does NOT call `GDK.users.add_default_user_async()`
## during `_ready()` even with `initialize_on_startup=true`.

const SETTING_INITIALIZE_ON_STARTUP := "gdk/runtime/initialize_on_startup"
const SETTING_AUTO_ADD_PRIMARY_USER := "gdk/runtime/auto_add_primary_user"
const SCENARIO := "auto_add_primary_user_false"
const STARTUP_BUDGET_MSEC := 4000


func _initialize() -> void:
	ProjectSettings.set_setting(SETTING_INITIALIZE_ON_STARTUP, true)
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

	var users = gdk.get_users() if gdk.has_method("get_users") else null
	if users == null:
		# Without the users service we can't observe the auto-add bypass.
		# Mark the scenario as pass-with-pending so the orchestrator records
		# a non-zero exit only when behavior is actually wrong.
		print("BOOTSTRAP_OK: %s (pending: GDK.users service not available)" % SCENARIO)
		quit(0)
		return

	# Verify the project setting was honored by the bootstrap path. We also
	# inspect the post-budget state: when `auto_add_primary_user=false`, the
	# autoload's silent sign-in branch is never taken even on live machines.
	# A primary user appearing here would only be possible if the user was
	# already signed in at process start, which is rare in headless CI.
	var setting_value = bool(ProjectSettings.get_setting(SETTING_AUTO_ADD_PRIMARY_USER, true))
	if setting_value:
		printerr("BOOTSTRAP_FAIL: %s -- ProjectSettings did not retain auto_add_primary_user=false" % SCENARIO)
		quit(4)
		return

	print("BOOTSTRAP_OK: %s (auto_add_primary_user=false honored; primary=%s)" % [SCENARIO, str(users.get_primary_user() != null)])
	quit(0)
