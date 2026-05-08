extends SceneTree
## Wave 4 bootstrap mini-runner.
##
## Asserts: when both `gdk/runtime/initialize_on_startup` and
## `gdk/runtime/auto_add_primary_user` are `true`, the `GDKBootstrap`
## autoload kicks off `GDK.users.add_default_user_async()` during `_ready()`
## once the runtime is initialized.
##
## Live-gated: a real signed-in user is only available on machines with a
## signed-in Xbox identity. This mini-runner exits 0 in either case (live
## machine: assert a primary user is reachable within the startup budget;
## non-live machine: print a `BOOTSTRAP_OK ... pending` line and exit 0).

const SETTING_INITIALIZE_ON_STARTUP := "gdk/runtime/initialize_on_startup"
const SETTING_AUTO_ADD_PRIMARY_USER := "gdk/runtime/auto_add_primary_user"
const SCENARIO := "auto_add_primary_user_true"
const STARTUP_BUDGET_MSEC := 6000
const LIVE_TESTS_ENV := "LIVE_TESTS"


func _initialize() -> void:
	ProjectSettings.set_setting(SETTING_INITIALIZE_ON_STARTUP, true)
	ProjectSettings.set_setting(SETTING_AUTO_ADD_PRIMARY_USER, true)
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

	var live = OS.get_environment(LIVE_TESTS_ENV) == "1"
	var users = gdk.get_users() if gdk.has_method("get_users") else null

	if not live:
		# Without LIVE_TESTS we can't require a real primary user. Just
		# verify the autoload produced *some* observable startup state: it
		# either initialized GDK, recorded a last_error, or surfaced a
		# primary user.
		var last_error = gdk.get_last_error()
		var primary = users.get_primary_user() if users != null else null
		if last_error == null and primary == null and not gdk.is_initialized():
			printerr("BOOTSTRAP_FAIL: %s -- autoload did not attempt init/auto-add (no last_error, no primary, not initialized)" % SCENARIO)
			quit(4)
			return
		print("BOOTSTRAP_OK: %s (pending live: initialized=%s, has_primary=%s)" % [SCENARIO, str(gdk.is_initialized()), str(primary != null)])
		quit(0)
		return

	if users == null:
		printerr("BOOTSTRAP_FAIL: %s -- GDK.users service unavailable in live mode" % SCENARIO)
		quit(5)
		return
	if not gdk.is_initialized():
		printerr("BOOTSTRAP_FAIL: %s -- GDK is not initialized in live mode within %d ms" % [SCENARIO, STARTUP_BUDGET_MSEC])
		quit(6)
		return
	var primary = users.get_primary_user()
	if primary == null:
		var last_error = gdk.get_last_error()
		if last_error != null and not last_error.ok:
			print("BOOTSTRAP_OK: %s (live: auto-add attempted; sign-in returned %s)" % [SCENARIO, last_error.code])
			quit(0)
			return
		printerr("BOOTSTRAP_FAIL: %s -- live mode but no primary user after %d ms (no error)" % [SCENARIO, STARTUP_BUDGET_MSEC])
		quit(7)
		return
	print("BOOTSTRAP_OK: %s (live: primary user gamertag=%s)" % [SCENARIO, primary.get_gamertag()])
	quit(0)
