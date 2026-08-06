extends "res://addons/godot_gdk_tests/gdk_test_base.gd"
## Shared GUT base for the `godot_playfab` coverage suite.
##
## Extends `GdkTestBase` to reuse reflection, async, and environment helpers.
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

# Custom-ID sign-in is the single busiest live call in this host: every live
# test signs in independently, so a full suite run fires ~20 LoginWithCustomID
# calls at the same title player within a few seconds. That reliably trips the
# service-side 429 (E_PF_API_CLIENT_REQUEST_RATE_LIMIT_EXCEEDED), which under
# the mandatory live tier turns every downstream test red for a reason that has
# nothing to do with the code under test.
#
# Mirrors the proven pacing/retry approach in
# tests/godot/mp_test_client/scripts/playfab_runtime.gd: proactively space the
# calls out, then reactively back off when the service says no anyway.
const PLAYFAB_RATE_LIMIT_HRESULT := "0x892354DD"
const PLAYFAB_RATE_LIMIT_SERVICE_SIGNALS := [
	"rate limit",
	"too many requests",
]
const PLAYFAB_SIGN_IN_MAX_ATTEMPTS := 6
const PLAYFAB_SIGN_IN_BACKOFF_MSEC := 3_000
const PLAYFAB_SIGN_IN_BACKOFF_CAP_MSEC := 20_000
# Minimum spacing between two custom-ID sign-ins anywhere in the run. Static so
# it is shared by every test script that extends this base.
const PLAYFAB_SIGN_IN_MIN_SPACING_MSEC := 1_200

static var _last_sign_in_started_msec := -1

var _playfab_extension: Resource = null


# ── Singleton + runtime helpers ──────────────────────────────────────────

func get_playfab() -> Object:
	apply_playfab_env_configuration()
	if Engine.has_singleton("PlayFab"):
		return Engine.get_singleton("PlayFab")

	if _playfab_extension == null and FileAccess.file_exists(PLAYFAB_EXTENSION_PATH):
		_playfab_extension = load(PLAYFAB_EXTENSION_PATH)

	if Engine.has_singleton("PlayFab"):
		return Engine.get_singleton("PlayFab")
	return null


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


func sign_in_with_configured_custom_id(playfab: Object, label: String, timeout_msec: int = DEFAULT_ASYNC_TIMEOUT_MSEC, create_account: bool = false) -> Dictionary:
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

	var sign_in_result: Variant = null
	for attempt in range(PLAYFAB_SIGN_IN_MAX_ATTEMPTS):
		await _space_out_custom_id_sign_in()

		var sign_in_signal = playfab.users.sign_in_with_custom_id_async(custom_id, create_account)
		if typeof(sign_in_signal) != TYPE_SIGNAL:
			pending("%s skipped: PlayFab.users.sign_in_with_custom_id_async() did not start." % label)
			outcome["skip_reason"] = "sign_in_did_not_start"
			return outcome

		sign_in_result = await await_completion(sign_in_signal, timeout_msec)
		if sign_in_result != null and sign_in_result.ok:
			break
		if attempt + 1 >= PLAYFAB_SIGN_IN_MAX_ATTEMPTS:
			break
		if not is_playfab_rate_limit_error(sign_in_result):
			break

		var backoff_msec: int = mini(
			PLAYFAB_SIGN_IN_BACKOFF_CAP_MSEC,
			PLAYFAB_SIGN_IN_BACKOFF_MSEC * (attempt + 1))
		print("[playfab_test_base] %s: custom-ID sign-in rate-limited; attempt %d/%d, waiting %dms" % [
			label, attempt + 1, PLAYFAB_SIGN_IN_MAX_ATTEMPTS, backoff_msec])
		await sleep_msec(backoff_msec)

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


# True when `result` carries a PlayFab service-side rate-limit (HTTP 429).
func is_playfab_rate_limit_error(result: Variant) -> bool:
	if result == null:
		return false
	var message := ""
	if "message" in result:
		message = str(result.message)
	if message.findn(PLAYFAB_RATE_LIMIT_HRESULT) >= 0:
		return true
	for service_signal in PLAYFAB_RATE_LIMIT_SERVICE_SIGNALS:
		if message.findn(service_signal) >= 0:
			return true
	return false


# Sleep that keeps pumping the PlayFab (and GDK) dispatchers, so in-flight
# operations still settle while we wait out a rate-limit backoff.
func sleep_msec(duration_msec: int) -> void:
	var tree := Engine.get_main_loop() as SceneTree
	var deadline := Time.get_ticks_msec() + maxi(0, duration_msec)
	while Time.get_ticks_msec() < deadline:
		var playfab: Object = get_playfab()
		if playfab != null:
			playfab.dispatch()
		var gdk = get_gdk()
		if gdk != null:
			gdk.dispatch()
		if tree != null:
			await tree.process_frame
		else:
			OS.delay_msec(16)


# Proactively space consecutive custom-ID sign-ins so a full-suite run does not
# burst past the service quota in the first place.
func _space_out_custom_id_sign_in() -> void:
	var now := Time.get_ticks_msec()
	if _last_sign_in_started_msec >= 0:
		var elapsed := now - _last_sign_in_started_msec
		if elapsed < PLAYFAB_SIGN_IN_MIN_SPACING_MSEC:
			await sleep_msec(PLAYFAB_SIGN_IN_MIN_SPACING_MSEC - elapsed)
	_last_sign_in_started_msec = Time.get_ticks_msec()


# ── Async helpers (override) ─────────────────────────────────────────────

# PlayFab tests pump both playfab.dispatch() and, when present, gdk.dispatch()
# each loop iteration so optional Xbox-backed compatibility flows still settle.
# Overrides `await_completion_state` from `GdkTestBase` so PlayFab consumers
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
# message label says "PlayFabResult" instead of "GDKResult" so failure
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
