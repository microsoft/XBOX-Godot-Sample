extends "res://addons/godot_gdk_tests/gdk_test_base.gd"
## Shared GUT base for the `godot_playfab` coverage suite.
##
## Extends `XboxTestBase` to reuse reflection, async, and environment helpers.
## Dedicated PlayFab coverage uses custom-ID sign-in by default. CMake mirrors
## the GDK addon into this host when `GODOT_PLAYFAB_TEST_HOST_WITH_GDK=ON` so
## optional Xbox-backed compatibility flows can also run; those helpers skip
## cleanly when the addon is intentionally omitted.
##
## Wave 3 PlayFab tests should
## `extends "res://addons/godot_gdk_tests/playfab_test_base.gd"`.

const PLAYFAB_EXTENSION_PATH := "res://addons/godot_playfab/godot_playfab.gdextension"
const PLAYFAB_TITLE_ID_SETTING := "playfab/runtime/title_id"
const PLAYFAB_ENDPOINT_SETTING := "playfab/runtime/endpoint"
const PLAYFAB_EMBED_DISPATCH_SETTING := "playfab/runtime/embed_dispatch"
const PLAYFAB_INITIALIZE_ON_STARTUP_SETTING := "playfab/runtime/initialize_on_startup"
const PLAYFAB_TEST_CUSTOM_ID_SETTING := "playfab/tests/custom_id"
const PLAYFAB_TITLE_ID_ENV := "PLAYFAB_TITLE_ID"
const PLAYFAB_TEST_CUSTOM_ID_ENV := "PLAYFAB_CUSTOM_ID"
const PLAYFAB_SINGLETON_NAME_SETTING := "playfab/runtime/singleton_name"
const PLAYFAB_DEFAULT_SINGLETON_NAME := "PlayFab"
const PLAYFAB_RATE_LIMIT_HRESULT := 0x892354DD
const PLAYFAB_RATE_LIMIT_RETRY_DELAY_MSEC := 150000
# Native class the singleton must be an instance of. The singleton *name* is
# configurable; the class it resolves to is not.
const PLAYFAB_SINGLETON_CLASS_NAME := "PlayFab"

var _playfab_extension: Resource = null
var _playfab_live_sessions: Dictionary = {}
var _playfab_live_runtime: Object = null


func after_all() -> void:
	shutdown_playfab_live_sessions()


# ── Singleton + runtime helpers ──────────────────────────────────────────

## Returns the Engine singleton name configured by
## `playfab/runtime/singleton_name`, falling back to `"PlayFab"`. Kept in sync
## with the resolution in the addon's `register_types.cpp` and
## `playfab_bootstrap.gd`.
func playfab_singleton_name() -> String:
	var configured := str(
			ProjectSettings.get_setting(
					PLAYFAB_SINGLETON_NAME_SETTING, PLAYFAB_DEFAULT_SINGLETON_NAME)).strip_edges()
	if configured.is_empty() or not configured.is_valid_ascii_identifier():
		return PLAYFAB_DEFAULT_SINGLETON_NAME
	return configured


# Resolves the singleton by configured name, retrying under the default name
# because the C++ side falls back to "PlayFab" when the configured name is
# unusable. Candidates are class-checked so a configured name that collides
# with an unrelated engine singleton (say `Input`) is not mistaken for the
# runtime.
func _find_playfab_singleton() -> Object:
	var configured := playfab_singleton_name()
	if Engine.has_singleton(configured):
		var candidate: Object = Engine.get_singleton(configured)
		if candidate != null and candidate.is_class(PLAYFAB_SINGLETON_CLASS_NAME):
			return candidate
	if configured != PLAYFAB_DEFAULT_SINGLETON_NAME and Engine.has_singleton(PLAYFAB_DEFAULT_SINGLETON_NAME):
		var fallback: Object = Engine.get_singleton(PLAYFAB_DEFAULT_SINGLETON_NAME)
		if fallback != null and fallback.is_class(PLAYFAB_SINGLETON_CLASS_NAME):
			return fallback
	return null


func get_playfab() -> Object:
	apply_playfab_env_configuration()
	var playfab: Object = _find_playfab_singleton()
	if playfab != null:
		return playfab

	if _playfab_extension == null and FileAccess.file_exists(PLAYFAB_EXTENSION_PATH):
		_playfab_extension = load(PLAYFAB_EXTENSION_PATH)

	return _find_playfab_singleton()


func apply_playfab_env_configuration() -> void:
	var env_title_id := OS.get_environment(PLAYFAB_TITLE_ID_ENV).strip_edges()
	if not env_title_id.is_empty():
		ProjectSettings.set_setting(PLAYFAB_TITLE_ID_SETTING, env_title_id)

	var env_custom_id := OS.get_environment(PLAYFAB_TEST_CUSTOM_ID_ENV).strip_edges()
	if not env_custom_id.is_empty():
		ProjectSettings.set_setting(PLAYFAB_TEST_CUSTOM_ID_SETTING, env_custom_id)


func reset_playfab_runtime() -> void:
	clear_playfab_live_session_cache()
	apply_playfab_env_configuration()
	var playfab: Object = get_playfab()
	if playfab != null:
		playfab.shutdown()


func pending_unless_playfab_available() -> bool:
	if get_playfab() == null:
		pending("PlayFab singleton is not available in this host")
		return true
	return false


# Returns the active PlayFab title id from project settings, or "".
func get_active_playfab_title_id() -> String:
	apply_playfab_env_configuration()
	if not ProjectSettings.has_setting(PLAYFAB_TITLE_ID_SETTING):
		return ""
	return str(ProjectSettings.get_setting(PLAYFAB_TITLE_ID_SETTING))


func get_configured_playfab_custom_id() -> String:
	apply_playfab_env_configuration()
	var env_custom_id := OS.get_environment(PLAYFAB_TEST_CUSTOM_ID_ENV).strip_edges()
	if not env_custom_id.is_empty():
		return env_custom_id
	if ProjectSettings.has_setting(PLAYFAB_TEST_CUSTOM_ID_SETTING):
		return str(ProjectSettings.get_setting(PLAYFAB_TEST_CUSTOM_ID_SETTING, "")).strip_edges()
	return ""


func clear_playfab_live_session_cache() -> void:
	_playfab_live_sessions.clear()
	_playfab_live_runtime = null


func shutdown_playfab_live_sessions() -> void:
	var playfab: Object = _playfab_live_runtime
	clear_playfab_live_session_cache()
	if playfab != null:
		playfab.shutdown()


func begin_playfab_live_session(
		label: String,
		write_required: bool = false,
		xbox_backed: bool = false,
		create_account: bool = false,
		strict_failures: bool = false,
		timeout_msec: int = DEFAULT_ASYNC_TIMEOUT_MSEC) -> Dictionary:
	var outcome := {
		"playfab_user": null,
		"playfab": null,
		"result": null,
		"custom_id": "",
		"failure_kind": "",
		"failure_message": "",
	}

	if write_required:
		if not requires_live_write():
			return outcome
	elif not requires_live():
		return outcome

	var cache_key := "%d:%d" % [
		int(xbox_backed),
		int(create_account),
	]
	if _playfab_live_sessions.has(cache_key):
		var cached: Dictionary = _playfab_live_sessions[cache_key]
		var cached_playfab: Object = cached.get("playfab")
		if cached.get("playfab_user") != null and (
				cached_playfab == null or not cached_playfab.is_initialized()):
			_playfab_live_sessions.erase(cache_key)
		else:
			if not str(cached.get("failure_kind", "")).is_empty():
				_report_playfab_failure(
					str(cached.get("failure_message", "")),
					strict_failures)
			return cached

	var playfab: Object = get_playfab()
	outcome["playfab"] = playfab
	if playfab == null:
		return _cache_playfab_live_session_failure(
			cache_key,
			outcome,
			"playfab_unavailable",
			"PlayFab singleton is not available in this host.",
			strict_failures)
	_playfab_live_runtime = playfab

	var configured_title_id := get_active_playfab_title_id().strip_edges()
	if configured_title_id.is_empty():
		return _cache_playfab_live_session_failure(
			cache_key,
			outcome,
			"configuration",
			"%s requires ProjectSettings['%s'] or %s." % [
				label,
				PLAYFAB_TITLE_ID_SETTING,
				PLAYFAB_TITLE_ID_ENV,
			],
			strict_failures)

	if not playfab.is_initialized():
		var init_result = playfab.initialize()
		outcome["result"] = init_result
		if init_result == null:
			return _cache_playfab_live_session_failure(
				cache_key,
				outcome,
				"initialize_timeout",
				"PlayFab.initialize() returned no result for %s." % label,
				strict_failures)
		if not init_result.ok:
			return _cache_playfab_live_session_failure(
				cache_key,
				outcome,
				"initialize_failed",
				"PlayFab.initialize() failed for %s: %s" % [label, init_result.message],
				strict_failures)

	var sign_in_operation: Callable
	var sign_in_label: String
	if xbox_backed:
		var xbox_session = await ensure_gdk_primary_user_for_playfab(timeout_msec)
		var xbox_user = xbox_session.get("user")
		if xbox_user == null:
			return _cache_playfab_live_session_failure(
				cache_key,
				outcome,
				"xbox_setup_failed",
				"Xbox-backed PlayFab setup failed for %s: %s" % [
					label,
					str(xbox_session.get("skip_reason", "no Xbox user returned")),
				],
				strict_failures)

		sign_in_label = "%s Xbox-backed sign-in" % label
		sign_in_operation = func():
			return playfab.users.sign_in_with_xuser_async(xbox_user, create_account)
	else:
		var custom_id := get_configured_playfab_custom_id()
		outcome["custom_id"] = custom_id
		if custom_id.is_empty():
			return _cache_playfab_live_session_failure(
				cache_key,
				outcome,
				"custom_id_unconfigured",
				"%s requires ProjectSettings['%s'] or %s." % [
					label,
					PLAYFAB_TEST_CUSTOM_ID_SETTING,
					PLAYFAB_TEST_CUSTOM_ID_ENV,
				],
				strict_failures)

		sign_in_label = "%s custom-ID sign-in" % label
		sign_in_operation = func():
			return playfab.users.sign_in_with_custom_id_async(custom_id, create_account)

	var sign_in := await _sign_in_playfab_user(
		sign_in_operation,
		sign_in_label,
		timeout_msec)
	outcome["result"] = sign_in.get("result")
	var failure_kind := str(sign_in.get("failure_kind", ""))
	if not failure_kind.is_empty():
		return _cache_playfab_live_session_failure(
			cache_key,
			outcome,
			failure_kind,
			str(sign_in.get("failure_message", "")),
			strict_failures)

	outcome["playfab_user"] = sign_in.get("playfab_user")
	_playfab_live_sessions[cache_key] = outcome
	return outcome


func sign_in_with_configured_custom_id(
		playfab: Object,
		label: String,
		timeout_msec: int = DEFAULT_ASYNC_TIMEOUT_MSEC,
		create_account: bool = false,
		strict_failures: bool = false) -> Dictionary:
	var custom_id := get_configured_playfab_custom_id()
	var outcome := {
		"custom_id": custom_id,
		"playfab_user": null,
		"result": null,
		"failure_kind": "",
		"failure_message": "",
	}
	if custom_id.is_empty():
		outcome["failure_kind"] = "custom_id_unconfigured"
		outcome["failure_message"] = (
			"Set ProjectSettings['%s'] or %s to exercise %s."
		) % [
				PLAYFAB_TEST_CUSTOM_ID_SETTING,
				PLAYFAB_TEST_CUSTOM_ID_ENV,
				label,
			]
		_report_playfab_failure(str(outcome["failure_message"]), strict_failures)
		return outcome

	var operation := func():
		return playfab.users.sign_in_with_custom_id_async(custom_id, create_account)
	outcome = await _sign_in_playfab_user(operation, label, timeout_msec)
	outcome["custom_id"] = custom_id
	if not str(outcome.get("failure_kind", "")).is_empty():
		_report_playfab_failure(
			str(outcome.get("failure_message", "")),
			strict_failures)
	return outcome


func await_playfab_result_with_rate_limit_retry(
		operation: Callable,
		label: String,
		timeout_msec: int = DEFAULT_ASYNC_TIMEOUT_MSEC) -> Dictionary:
	var outcome := {
		"result": null,
		"failure_kind": "",
		"failure_message": "",
	}

	for attempt_index in range(2):
		var completion_signal: Variant = operation.call()
		if typeof(completion_signal) != TYPE_SIGNAL:
			outcome["failure_kind"] = "did_not_start"
			outcome["failure_message"] = "%s did not return a Signal." % label
			return outcome

		var result = await await_completion(completion_signal, timeout_msec)
		outcome["result"] = result
		if result == null:
			outcome["failure_kind"] = "timeout"
			outcome["failure_message"] = "%s timed out." % label
			return outcome
		if not is_playfab_rate_limit_result(result):
			return outcome
		if attempt_index == 1:
			outcome["failure_kind"] = "rate_limit"
			outcome["failure_message"] = (
				"%s remained rate-limited after one 150-second cooldown and retry "
				+ "(E_PF_API_CLIENT_REQUEST_RATE_LIMIT_EXCEEDED, HRESULT 0x892354DD). "
				+ "Wait at least 150 seconds for the per-player window to clear, then rerun the live tier."
			) % label
			return outcome

		print(
			"[PlayFab live] %s hit the per-player request rate limit; retrying once in 150s."
			% label)
		await _wait_for_playfab_retry(PLAYFAB_RATE_LIMIT_RETRY_DELAY_MSEC)

	return outcome


func is_playfab_rate_limit_result(result: Variant) -> bool:
	if result == null:
		return false

	if result is Dictionary:
		if bool(result.get("ok", false)):
			return false
		return (
			int(result.get("hresult", 0)) & 0xFFFFFFFF
		) == PLAYFAB_RATE_LIMIT_HRESULT
	if result is Object:
		if bool(result.get("ok")):
			return false
		return (
			int(result.get("hresult")) & 0xFFFFFFFF
		) == PLAYFAB_RATE_LIMIT_HRESULT
	return false


func _sign_in_playfab_user(
		operation: Callable,
		label: String,
		timeout_msec: int) -> Dictionary:
	var outcome := await await_playfab_result_with_rate_limit_retry(
		operation,
		label,
		timeout_msec)
	outcome["playfab_user"] = null
	if not str(outcome.get("failure_kind", "")).is_empty():
		return outcome

	var result = outcome.get("result")
	if result == null:
		outcome["failure_kind"] = "sign_in_timeout"
		outcome["failure_message"] = "%s timed out." % label
	elif not result.ok:
		outcome["failure_kind"] = "sign_in_failed"
		outcome["failure_message"] = "%s failed: %s" % [label, result.message]
	elif result.data == null:
		outcome["failure_kind"] = "sign_in_missing_user"
		outcome["failure_message"] = "%s returned no PlayFabUser." % label
	else:
		outcome["playfab_user"] = result.data
	return outcome


func _cache_playfab_live_session_failure(
		cache_key: String,
		outcome: Dictionary,
		failure_kind: String,
		failure_message: String,
		strict_failures: bool) -> Dictionary:
	outcome["failure_kind"] = failure_kind
	outcome["failure_message"] = failure_message
	_playfab_live_sessions[cache_key] = outcome
	_report_playfab_failure(failure_message, strict_failures)
	return outcome


func _report_playfab_failure(failure_message: String, strict_failures: bool) -> void:
	if strict_failures:
		fail(failure_message)
	else:
		pending(failure_message)


func _wait_for_playfab_retry(delay_msec: int) -> void:
	var playfab: Object = get_playfab()
	var gdk: Object = get_gdk()
	var main_loop: MainLoop = Engine.get_main_loop()
	var started_msec := Time.get_ticks_msec()
	while Time.get_ticks_msec() - started_msec < delay_msec:
		if playfab != null:
			playfab.dispatch()
		if gdk != null:
			gdk.dispatch()
		if main_loop != null and main_loop.has_signal("process_frame"):
			await main_loop.process_frame
		else:
			OS.delay_msec(ASYNC_POLL_INTERVAL_MSEC)


# ── Async helpers (override) ─────────────────────────────────────────────

# PlayFab tests pump both playfab.dispatch() and, when present, gdk.dispatch()
# each loop iteration so optional Xbox-backed compatibility flows still settle.
# Overrides `await_completion_state` from `XboxTestBase` so PlayFab consumers
# get the dual-pump behavior automatically without having to remember to
# call a separate helper.
func await_completion_state(state: Dictionary, timeout_msec: int = DEFAULT_ASYNC_TIMEOUT_MSEC) -> Variant:
	var playfab: Object = get_playfab()
	var gdk: Object = get_gdk()
	var main_loop: MainLoop = Engine.get_main_loop()
	var started_msec := Time.get_ticks_msec()
	while not bool(state.get("completed", false)):
		if playfab != null:
			playfab.dispatch()
		if gdk != null:
			gdk.dispatch()
		if Time.get_ticks_msec() - started_msec >= timeout_msec:
			return null
		if main_loop != null and main_loop.has_signal("process_frame"):
			await main_loop.process_frame
		else:
			OS.delay_msec(ASYNC_POLL_INTERVAL_MSEC)

	if playfab != null:
		playfab.dispatch()
	if gdk != null:
		gdk.dispatch()
	return state.get("result")


# ── PlayFabResult assertions ─────────────────────────────────────────────

func assert_playfab_result_ok(result: Variant, name: String) -> void:
	assert_not_null(result, "%s returns PlayFabResult" % name)
	if result == null:
		return
	assert_true(result.ok, "%s result.ok == true" % name)


func assert_playfab_result_failed(result: Variant, name: String) -> void:
	assert_not_null(result, "%s returns PlayFabResult" % name)
	if result == null:
		return
	assert_false(result.ok, "%s result.ok == false" % name)


# Note: deliberately distinct from inherited `assert_result_error`. The
# message label says "PlayFabResult" instead of "XboxResult" so failure
# output points at the right type. Behavior is identical otherwise.
func assert_playfab_result_error(result: Variant, expected_code: String, name: String) -> void:
	assert_not_null(result, "%s returns PlayFabResult" % name)
	if result == null:
		return
	assert_false(result.ok, "%s result.ok == false" % name)
	assert_eq(result.code, expected_code, "%s error code" % name)
	assert_true(result.message.length() > 0, "%s error message present" % name)


# ── Composite GDK + PlayFab user flow ────────────────────────────────────

# Mirrors `ensure_gdk_primary_user` from the historical (now-removed)
# `playfab_demo` `test_context.gd`: ensures GDK is initialized and a
# primary user is signed in before PlayFab tests attempt PlayFab
# sign-in. Returns a Dictionary with the same shape so suites can
# drive `pending(...)` off `skip_reason` consistently.
func ensure_gdk_primary_user_for_playfab(timeout_msec: int = DEFAULT_ASYNC_TIMEOUT_MSEC) -> Dictionary:
	var outcome := {
		"user": null,
		"result": null,
		"signal": null,
		"op": null,
		"skip_reason": "",
	}

	var gdk: Object = get_gdk()
	if gdk == null:
		outcome["skip_reason"] = "GDK singleton is not available."
		return outcome

	if not gdk.is_initialized():
		var init_result: Variant = gdk.initialize()
		outcome["result"] = init_result
		if init_result == null or not init_result.ok:
			outcome["skip_reason"] = init_result.message if init_result != null else "GDK.initialize() failed."
			return outcome

	var user: Variant = gdk.users.get_primary_user()
	if user != null and user.signed_in:
		outcome["user"] = user
		return outcome

	var completion_signal: Variant = gdk.users.add_default_user_async()
	outcome["signal"] = completion_signal
	outcome["op"] = completion_signal
	if typeof(completion_signal) != TYPE_SIGNAL:
		outcome["skip_reason"] = "GDK.users.add_default_user_async() did not start."
		return outcome

	var result: Variant = await await_completion(completion_signal, timeout_msec)
	outcome["result"] = result
	if result == null:
		outcome["skip_reason"] = "Timed out waiting for the GDK default-user flow."
		return outcome
	if not result.ok:
		outcome["skip_reason"] = result.message
		return outcome

	user = result.data if result.data != null else gdk.users.get_primary_user()
	if user == null or not user.signed_in:
		outcome["skip_reason"] = "GDK default-user flow did not return a signed-in user."
		return outcome

	outcome["user"] = user
	return outcome
