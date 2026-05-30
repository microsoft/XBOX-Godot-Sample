extends GutTest
## Shared GUT base for `godot_gdk` coverage suites.
##
## Wave 3 GUT-based tests should `extends "res://addons/godot_gdk_tests/gdk_test_base.gd"`
## (the mirrored res:// path produced by the second
## `godot_addon_mirror_test_support` call in the root `CMakeLists.txt`).
##
## Helpers absorbed from the now-removed `sample/gdk_demo/tests/test_context.gd`
## (and their near-duplicates in the PlayFab test_context). Helpers used by only
## one suite were intentionally left in their suite files; Wave 3 may move them
## here later if cross-cutting use emerges.

const TestEnv = preload("res://addons/godot_gdk_tests/test_env.gd")

const DEFAULT_ASYNC_TIMEOUT_MSEC := 5000
const ASYNC_POLL_INTERVAL_MSEC := 10
const FLOAT_EPSILON := 0.0001

const GDK_EXTENSION_PATH := "res://addons/godot_gdk/godot_gdk.gdextension"
const EMBED_DISPATCH_SETTING := "gdk/runtime/embed_dispatch"

var _gdk_extension: Resource = null


# ── Singleton + runtime helpers ──────────────────────────────────────────

func get_gdk():
	if Engine.has_singleton("GDK"):
		return Engine.get_singleton("GDK")

	if _gdk_extension == null and FileAccess.file_exists(GDK_EXTENSION_PATH):
		_gdk_extension = load(GDK_EXTENSION_PATH)

	if Engine.has_singleton("GDK"):
		return Engine.get_singleton("GDK")
	return null


func reset_runtime() -> void:
	var gdk: Object = get_gdk()
	if gdk != null:
		gdk.shutdown()


func initialize_runtime() -> Variant:
	var gdk: Object = get_gdk()
	if gdk == null:
		return null
	reset_runtime()
	return gdk.initialize()


# Pending the current test if the GDK singleton is unavailable. Returns true
# when the runtime is missing (caller should `return` after).
func pending_unless_runtime_available() -> bool:
	if get_gdk() == null:
		pending("GDK singleton is not available in this host")
		return true
	return false


# ── Project-setting helpers ──────────────────────────────────────────────

func get_embed_dispatch_enabled() -> bool:
	return bool(ProjectSettings.get_setting(EMBED_DISPATCH_SETTING, true))


func set_embed_dispatch_enabled(enabled: bool) -> void:
	ProjectSettings.set_setting(EMBED_DISPATCH_SETTING, enabled)


func get_setting_default(setting_name: String):
	if ProjectSettings.property_can_revert(setting_name):
		return ProjectSettings.property_get_revert(setting_name)
	return null


# ── Async signal helpers ─────────────────────────────────────────────────

func track_signal(async_signal) -> Dictionary:
	var state := {
		"completed": false,
		"result": null,
	}
	if typeof(async_signal) != TYPE_SIGNAL:
		return state
	async_signal.connect(
		func(result):
			state["completed"] = true
			state["result"] = result,
		CONNECT_ONE_SHOT)
	return state


# Drains the GDK manual completion queue while waiting. Mirrors the historic
# `wait_for_tracked_signal` from the now-removed
# `sample/gdk_demo/tests/test_context.gd` — pumps `GDK.dispatch()` each loop
# iteration so manager-driven completions resolve under both
# `embed_dispatch=true` and `embed_dispatch=false`.
#
# Renamed from the pre-GUT `wait_for_tracked_signal` to avoid shadowing
# `GutTest.wait_for_signal(Signal, max_time, msg)` which has a different
# signature and semantics.
func await_completion_state(state: Dictionary, timeout_msec: int = DEFAULT_ASYNC_TIMEOUT_MSEC) -> Variant:
	var gdk: Object = get_gdk()
	var main_loop: MainLoop = Engine.get_main_loop()
	var started_msec := Time.get_ticks_msec()
	while not bool(state.get("completed", false)):
		if gdk != null:
			gdk.dispatch()
		if Time.get_ticks_msec() - started_msec >= timeout_msec:
			return null
		if main_loop != null and main_loop.has_signal("process_frame"):
			await main_loop.process_frame
		else:
			OS.delay_msec(ASYNC_POLL_INTERVAL_MSEC)

	if gdk != null:
		gdk.dispatch()
	return state.get("result")


func await_completion(async_signal: Variant, timeout_msec: int = DEFAULT_ASYNC_TIMEOUT_MSEC) -> Variant:
	if typeof(async_signal) != TYPE_SIGNAL:
		return null
	return await await_completion_state(track_signal(async_signal), timeout_msec)


# Sibling waiter that does NOT call `gdk.dispatch()` — used by tests that
# verify the embed_dispatch=true auto-pump path without having a manual
# dispatch confounder.
func await_completion_state_no_dispatch(state: Dictionary, timeout_msec: int = DEFAULT_ASYNC_TIMEOUT_MSEC) -> Variant:
	var main_loop: MainLoop = Engine.get_main_loop()
	if main_loop == null or not main_loop.has_signal("process_frame"):
		return null

	var started_msec := Time.get_ticks_msec()
	while not bool(state.get("completed", false)):
		if Time.get_ticks_msec() - started_msec >= timeout_msec:
			return null
		await main_loop.process_frame

	return state.get("result")


func await_completion_no_dispatch(async_signal: Variant, timeout_msec: int = DEFAULT_ASYNC_TIMEOUT_MSEC) -> Variant:
	if typeof(async_signal) != TYPE_SIGNAL:
		return null
	return await await_completion_state_no_dispatch(track_signal(async_signal), timeout_msec)


func advance_process_frames(frame_count: int) -> bool:
	var main_loop: MainLoop = Engine.get_main_loop()
	if main_loop == null or not main_loop.has_signal("process_frame"):
		return false
	for _frame_index: int in range(frame_count):
		await main_loop.process_frame
	return true


# ── User-flow helpers ────────────────────────────────────────────────────

func ensure_primary_user(timeout_msec: int = DEFAULT_ASYNC_TIMEOUT_MSEC) -> Dictionary:
	var outcome := {
		"had_existing_user": false,
		"signal": null,
		"op": null,
		"result": null,
		"user": null,
	}

	var gdk: Object = get_gdk()
	if gdk == null:
		return outcome

	var users: Object = gdk.get_users()
	if users == null:
		return outcome

	var primary_user: Variant = users.get_primary_user()
	if primary_user != null:
		outcome["had_existing_user"] = true
		outcome["user"] = primary_user
		return outcome

	var completion_signal: Variant = users.add_default_user_async()
	outcome["signal"] = completion_signal
	outcome["op"] = completion_signal
	if typeof(completion_signal) != TYPE_SIGNAL:
		return outcome

	var result: Variant = await await_completion(completion_signal, timeout_msec)
	outcome["result"] = result
	if result != null and result.ok and result.data != null:
		outcome["user"] = result.data
	else:
		outcome["user"] = users.get_primary_user()
	return outcome


# ── GDKResult / signal-result assertions ─────────────────────────────────

func assert_result_ok(result, name: String) -> void:
	assert_not_null(result, "%s returns GDKResult" % name)
	if result == null:
		return
	assert_true(result.ok, "%s result.ok == true" % name)


func assert_result_failed(result, name: String) -> void:
	assert_not_null(result, "%s returns GDKResult" % name)
	if result == null:
		return
	assert_false(result.ok, "%s result.ok == false" % name)


func assert_result_error(result, expected_code: String, name: String) -> void:
	assert_not_null(result, "%s returns GDKResult" % name)
	if result == null:
		return
	assert_false(result.ok, "%s result.ok == false" % name)
	assert_eq(result.code, expected_code, "%s error code" % name)
	assert_true(result.message.length() > 0, "%s error message present" % name)


func assert_signal_result_error(async_signal, expected_code: String, name: String, timeout_msec: int = DEFAULT_ASYNC_TIMEOUT_MSEC) -> void:
	assert_eq(typeof(async_signal), TYPE_SIGNAL, "%s returns completion Signal" % name)
	if typeof(async_signal) != TYPE_SIGNAL:
		return
	assert_result_error(await await_completion(async_signal, timeout_msec), expected_code, name)


# ── Reflection / class-introspection sugar ───────────────────────────────

func instantiate_class(target_class: String):
	if not ClassDB.class_exists(target_class) or not ClassDB.can_instantiate(target_class):
		return null
	return ClassDB.instantiate(target_class)


func get_class_constant(target_class: String, constant_name: String) -> int:
	return ClassDB.class_get_integer_constant(target_class, constant_name)


func is_class_instance(value, target_class: String) -> bool:
	if value == null or typeof(value) != TYPE_OBJECT:
		return false
	var object_value: Object = value
	return object_value.is_class(target_class)


func assert_object_is(value, target_class: String, name: String) -> void:
	assert_true(is_class_instance(value, target_class), name)


func assert_has_method_named(obj: Object, method_name: String, test_name: String = "") -> void:
	var label := test_name if test_name else "%s.%s() exists" % [obj.get_class(), method_name]
	assert_true(obj.has_method(method_name), label)


func assert_has_signal_named(obj: Object, signal_name: String, test_name: String = "") -> void:
	var label := test_name if test_name else "%s.%s signal exists" % [obj.get_class(), signal_name]
	assert_true(obj.has_signal(signal_name), label)


func assert_dict_has_key(dictionary: Dictionary, key: String, name: String) -> void:
	assert_true(dictionary.has(key), name)


# Float comparison sugar for C++ properties that round-trip through 32-bit
# storage and won't equal 64-bit double literals exactly. Normalised here
# (vs the gameinput-only earlier `assert_eq_approx`) so any addon under the
# GDK family can use it.
func assert_eq_approx(actual: float, expected: float, name: String, eps: float = FLOAT_EPSILON) -> void:
	if absf(actual - expected) <= eps:
		assert_true(true, "%s ≈ %s" % [name, str(expected)])
	else:
		assert_true(false, "%s expected ≈ %s, got %s" % [name, str(expected), str(actual)])


# ── Signal-handler housekeeping ──────────────────────────────────────────

func disconnect_signal_handlers(obj: Object, signal_names: Array) -> void:
	for signal_name: String in signal_names:
		for conn: Dictionary in obj.get_signal_connection_list(signal_name):
			obj.disconnect(signal_name, conn["callable"])


# ── TestEnv convenience wrappers ─────────────────────────────────────────

# Returns true when LIVE_TESTS=1 is set; otherwise marks the current test pending.
func requires_live() -> bool:
	if TestEnv.live_tests_enabled():
		return true
	pending("Skipped without LIVE_TESTS=1")
	return false


# Returns true when both LIVE_TESTS=1 and LIVE_WRITE_TESTS=1 are set.
func requires_live_write() -> bool:
	if TestEnv.live_write_tests_enabled():
		return true
	pending("Skipped without LIVE_TESTS=1 and LIVE_WRITE_TESTS=1")
	return false


# Pending the current test unless LIVE_TESTS=1 is set. Returns true when
# the test was marked pending (caller should `return` immediately after).
func pending_unless_live() -> bool:
	return not requires_live()


# Returns "<prefix>-<unique_run_id>" so live write tests can derive
# correlated, run-stable ids from one shared run id.
func with_unique_id(prefix: String) -> String:
	return prefix + "-" + TestEnv.unique_run_id()
