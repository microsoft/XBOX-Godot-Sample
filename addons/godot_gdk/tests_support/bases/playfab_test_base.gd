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
# Native class the singleton must be an instance of. The singleton *name* is
# configurable; the class it resolves to is not.
const PLAYFAB_SINGLETON_CLASS_NAME := "PlayFab"

var _playfab_extension: Resource = null


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


func sign_in_with_configured_custom_id(playfab: Object, label: String, timeout_msec: int = DEFAULT_ASYNC_TIMEOUT_MSEC) -> Dictionary:
	var outcome := {
		"custom_id": "",
		"playfab_user": null,
		"result": null,
		"skip_reason": "",
	}

	var custom_id := get_configured_playfab_custom_id()
	outcome["custom_id"] = custom_id
	if custom_id.is_empty():
		pending("Set ProjectSettings['%s'] or %s to exercise %s." % [PLAYFAB_TEST_CUSTOM_ID_SETTING, PLAYFAB_TEST_CUSTOM_ID_ENV, label])
		outcome["skip_reason"] = "custom_id_unconfigured"
		return outcome

	var sign_in_signal = playfab.users.sign_in_with_custom_id_async(custom_id, false)
	if typeof(sign_in_signal) != TYPE_SIGNAL:
		pending("%s skipped: PlayFab.users.sign_in_with_custom_id_async() did not start." % label)
		outcome["skip_reason"] = "sign_in_did_not_start"
		return outcome

	var sign_in_result = await await_completion(sign_in_signal, timeout_msec)
	outcome["result"] = sign_in_result
	if sign_in_result == null:
		pending("%s skipped: custom-ID sign-in timed out." % label)
		outcome["skip_reason"] = "sign_in_timeout"
		return outcome
	if not sign_in_result.ok:
		pending("%s skipped: %s" % [label, sign_in_result.message])
		outcome["skip_reason"] = "sign_in_failed"
		return outcome

	outcome["playfab_user"] = sign_in_result.data
	return outcome


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
