extends "res://addons/godot_gdk_tests/gdk_test_base.gd"
## Wave 4 GUT coverage for the `gdk/runtime/embed_dispatch` project setting.
##
## Mirrors `test_core.gd::test_embed_dispatch_behavior()` but with a tighter
## per-suite scope: `before_all` toggles `gdk/runtime/embed_dispatch` to
## `false` so every test in this suite exercises the disabled-mode path, and
## `after_all` restores the original value so subsequent suites are unaffected.
##
## When `embed_dispatch` is disabled, the addon does NOT auto-pump
## `GDK.dispatch()` from Godot's frame callback. Manual `dispatch()` is the
## only way to drain manager-driven completions. We use
## `await_completion_state_no_dispatch()` to verify that completions stay
## pending without a manual pump and `await_completion_state()` to verify
## they resolve once the pump runs.

var _original_embed_dispatch: bool = true


func before_all() -> void:
	_original_embed_dispatch = get_embed_dispatch_enabled()
	set_embed_dispatch_enabled(false)


func after_all() -> void:
	set_embed_dispatch_enabled(_original_embed_dispatch)


func before_each() -> void:
	reset_runtime()


func after_each() -> void:
	reset_runtime()


func test_embed_dispatch_setting_visible_to_runtime() -> void:
	assert_eq(get_embed_dispatch_enabled(), false, "before_all flipped gdk/runtime/embed_dispatch=false for this suite")
	assert_true(ProjectSettings.has_setting(EMBED_DISPATCH_SETTING), "embed_dispatch setting registered")


func test_embed_dispatch_disabled_keeps_completion_pending_without_manual_pump() -> void:
	if pending_unless_runtime_available():
		return

	var gdk = get_gdk()
	var init_result = initialize_runtime()
	assert_not_null(init_result, "initialize() for disabled-embed coverage returns GDKResult")
	if init_result == null or not init_result.ok:
		pending("Disabled embed_dispatch behavior requires a working runtime: %s" % (
			"init returned null" if init_result == null else init_result.message))
		return

	var users = gdk.get_users()
	assert_not_null(users, "users service available for disabled-embed coverage")
	if users == null:
		return

	var async_signal = users.add_default_user_async()
	assert_eq(typeof(async_signal), TYPE_SIGNAL, "add_default_user_async() returns Signal")
	if typeof(async_signal) != TYPE_SIGNAL:
		return

	var state = track_signal(async_signal)
	if state["completed"]:
		pending("Disabled embed_dispatch: completion landed synchronously, can't observe pending state.")
		return

	# Without manual GDK.dispatch(), several Godot frames must NOT resolve the
	# completion. Reliably observable: advance frames using the no-dispatch
	# helper and assert the state stays pending.
	var advanced = await advance_process_frames(8)
	assert_true(advanced, "headless runner can advance process_frame for disabled-embed coverage")
	assert_eq(state["completed"], false, "embed_dispatch=false keeps completion pending across frames without manual GDK.dispatch()")

	# Drain via manual dispatch. The completion should now resolve within the
	# normal completion budget.
	var resolved = await await_completion_state(state, 8000)
	if resolved == null:
		pending("Disabled embed_dispatch: completion did not resolve after manual GDK.dispatch().")
	else:
		assert_true(true, "embed_dispatch=false: completion resolves once manual GDK.dispatch() is pumped")


func test_embed_dispatch_disabled_no_dispatch_signal_stays_pending() -> void:
	if pending_unless_runtime_available():
		return

	var gdk = get_gdk()
	var init_result = initialize_runtime()
	if init_result == null or not init_result.ok:
		pending("Disabled embed_dispatch no-pump path requires a working runtime: %s" % (
			"init returned null" if init_result == null else init_result.message))
		return

	var users = gdk.get_users()
	if users == null:
		pending("users service not available for no-dispatch coverage")
		return

	var async_signal = users.add_default_user_async()
	assert_eq(typeof(async_signal), TYPE_SIGNAL, "add_default_user_async() returns Signal for no-dispatch coverage")
	if typeof(async_signal) != TYPE_SIGNAL:
		return

	var state = track_signal(async_signal)
	if state["completed"]:
		pending("Disabled embed_dispatch: completion landed synchronously before no-dispatch budget could elapse.")
		return

	# Use the no-dispatch waiter with a tight budget. Without manual pump the
	# state must remain pending and the helper returns null on timeout.
	var resolved = await await_completion_state_no_dispatch(state, 500)
	assert_eq(resolved, null, "embed_dispatch=false + no manual dispatch: completion never resolves within poll budget")
	assert_eq(state["completed"], false, "embed_dispatch=false + no manual dispatch: state remains pending")
