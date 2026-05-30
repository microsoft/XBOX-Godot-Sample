## PlayFabRuntime — loads the PlayFab GDExtension into the test client
## process and exposes a dispatch hook + a sign-in helper.
##
## Mirrors the PlayFab host setup patterns (_ensure_playfab,
## _apply_env_configuration, _await_completion) but factors them into a
## stand-alone RefCounted so the TestClient can compose it.
extends RefCounted

const EXTENSION_PATH := "res://addons/godot_playfab/godot_playfab.gdextension"
const SETTING_TITLE_ID := "playfab/runtime/title_id"
const SETTING_ENDPOINT := "playfab/runtime/endpoint"
const ENV_TITLE_ID := "PLAYFAB_TITLE_ID"
const ENV_ENDPOINT := "PLAYFAB_ENDPOINT"

const DEFAULT_AWAIT_TIMEOUT_MS := 60_000

# PlayFab Core error code emitted when the title throttles client requests
# from the *local SDK* side. See <GDK>/windows/include/playfab/core/PFErrors.h
# E_PF_API_CLIENT_REQUEST_RATE_LIMIT_EXCEEDED. Sign-in is the most common
# place we hit this in the orchestrator because every scenario re-enters
# sign_in across multiple clients; retrying it keeps live MP sweeps from
# failing on transient SDK-side throttles.
const RATE_LIMIT_HRESULT := "0x892354DD"

# PlayFab service-side rate-limit signals. These are surfaced by the GDK
# wrappers as a non-zero HRESULT plus a human-readable message; the
# message strings are stable across SDK versions and are the most reliable
# detection signal for service-side 429s (the HRESULT itself is generic
# E_FAIL in some code paths). See addons/godot_playfab and the live run
# output in build/test-results/mp-test/ — example messages include
# "the PlayFab service failed with 429 Too Many Requests" and
# "a request rate limit was exceeded".
const RATE_LIMIT_SERVICE_SIGNALS: Array[String] = [
	"429",
	"rate limit was exceeded",
	"rate limit exceeded",
	"Too Many Requests",
]

const RATE_LIMIT_MAX_ATTEMPTS := 8
const RATE_LIMIT_BACKOFF_MS := 3_000
# Per-attempt backoff is capped here so the sum of N attempts stays
# tractable: with formula min(BACKOFF_CAP_MS, BACKOFF_MS * attempt) the
# 8-attempt schedule is 3s/6s/9s/12s/15s/18s/21s/24s = 108s total wait
# (well under any lobby scenario's TIMEOUT_SEC of 180+s). This is the
# reactive backstop — proactive pacing (RATE_BUDGETS below) should
# prevent us from hitting the limit in the first place, but a single
# attempt at retry handles surprises like the SDK issuing an extra
# subscribe-to-resource call under the hood.
const RATE_LIMIT_BACKOFF_CAP_MS := 30_000

# Proactive per-op sliding-window budgets.
#
# Sourced directly from the PlayFab Multiplayer service rate-limit
# documentation, taken at the title_player_account scope (this is the
# entity that the test client signs in as). We use values slightly
# below the documented limit so the proactive limiter leaves headroom
# for:
#   * SDK-internal calls under the same endpoint family (e.g. a
#     subscribe-to-resource call that PlayFab issues as part of a
#     create_lobby).
#   * Out-of-band traffic from sample apps sharing the same title.
#   * Tightly-coupled tests where one scenario's leave race could
#     leave a stale subscription.
#
# When a budget would be exceeded, _reserve_call_slot sleeps until the
# oldest tracked call falls out of the sliding window. The window is
# the documented quota window (120s for lobby ops, 60s for match ops).
#
# Each entry: { max_calls: int, window_ms: int }.
#
# Lobby ops (per title_player_account / 120s):
#   create_lobby      : 6   → cap at 5
#   join_lobby        : 6   → cap at 5
#   find_lobbies      : 12  → cap at 10
#   leave_lobby       : 20  → cap at 18
#   set_properties    : 40  → cap at 35 (rarely a bottleneck)
#   set_member_props  : 40  → cap at 35 (rarely a bottleneck)
#
# Match ops (per default / 60s, per the API surface):
#   create_match_ticket : 5 → cap at 4
#   cancel_match_ticket : 6 → cap at 5
const RATE_BUDGETS: Dictionary = {
	"create_lobby_async":           { "max_calls": 5,  "window_ms": 120_000 },
	"join_lobby_async":             { "max_calls": 5,  "window_ms": 120_000 },
	"find_lobbies_async":           { "max_calls": 10, "window_ms": 120_000 },
	"leave_lobby_async":            { "max_calls": 18, "window_ms": 120_000 },
	"set_properties_async":         { "max_calls": 35, "window_ms": 120_000 },
	"set_member_properties_async":  { "max_calls": 35, "window_ms": 120_000 },
	"create_match_ticket_async":    { "max_calls": 4,  "window_ms": 60_000  },
	"cancel_match_ticket_async":    { "max_calls": 5,  "window_ms": 60_000  },
}

var _tree: SceneTree = null
var _extension_handle: Resource = null
var _playfab: Object = null
var _playfab_user: Object = null
var _multiplayer: Object = null
var _initialized: bool = false
# Tracks the custom_id of the currently signed-in user so repeat sign_in
# calls within a single client process are idempotent. PlayFab Core does
# allow re-issuing LoginWithCustomID with the same id, but doing so
# burns rate-limit budget and we have no operational need — the test
# client process is per-role and only ever signs in as one identity.
var _signed_in_custom_id: String = ""
# Per-op sliding-window call timestamps for proactive rate-limit
# pacing (see RATE_BUDGETS). Keyed by op_name (the same op_name passed
# to await_completion_with_rate_limit_retry). Each entry is an Array of
# Time.get_ticks_msec() values, oldest first. The test client process
# is per-role so this naturally tracks per-(account, op) load — which
# matches PlayFab's per-(title_player_account) rate-limit dimension.
var _call_history: Dictionary = {}


func bind_tree(tree: SceneTree) -> void:
	_tree = tree


func is_available() -> bool:
	# Capability advertised to the orchestrator during handshake. The previous
	# implementation treated `.gdextension` file presence as availability, which
	# silently reported `true` even when the extension failed to load on the
	# current platform — then scenarios that depended on PlayFab would run and
	# fail mid-flight instead of being skipped. We now actually attempt to
	# load and require the registered singleton to be present.
	if Engine.has_singleton("PlayFab"):
		_playfab = Engine.get_singleton("PlayFab")
		return true
	if not FileAccess.file_exists(EXTENSION_PATH):
		return false
	# Best-effort load — extension is registered as a runtime resource; loading
	# it surfaces the singleton if the platform supports it.
	if _extension_handle == null:
		_extension_handle = load(EXTENSION_PATH)
	if Engine.has_singleton("PlayFab"):
		_playfab = Engine.get_singleton("PlayFab")
		return true
	return false


func ensure_loaded() -> Object:
	_apply_env_configuration()
	if Engine.has_singleton("PlayFab"):
		_playfab = Engine.get_singleton("PlayFab")
		return _playfab
	if _extension_handle == null and FileAccess.file_exists(EXTENSION_PATH):
		_extension_handle = load(EXTENSION_PATH)
	if Engine.has_singleton("PlayFab"):
		_playfab = Engine.get_singleton("PlayFab")
	return _playfab


func dispatch() -> void:
	if _playfab != null:
		_playfab.dispatch()


func has_user() -> bool:
	return _playfab_user != null


func get_playfab() -> Object:
	return _playfab


func get_user() -> Object:
	return _playfab_user


func clear_session() -> void:
	_playfab_user = null


func get_multiplayer() -> Object:
	return _multiplayer


func sign_in_with_custom_id(custom_id: String, create_account: bool, initialize_multiplayer: bool = true) -> Dictionary:
	var pf: Object = ensure_loaded()
	if pf == null:
		return _err("playfab_unavailable", "PlayFab singleton not available")

	if not _initialized:
		var init_res: Variant = pf.initialize()
		if init_res == null or not bool(init_res.ok):
			return _err("playfab_init_failed", _describe_result(init_res, "PlayFab.initialize"))
		_initialized = true

	if custom_id.is_empty():
		return _err("invalid_custom_id", "sign_in requires a non-empty custom_id")

	# Idempotent fast-path: if we're already signed in as the requested
	# identity, return the cached entity without re-issuing the PlayFab
	# call. Avoids burning client rate-limit budget on the scenario-runner
	# pattern where a scenario may call sign_in multiple times.
	if _playfab_user != null and _signed_in_custom_id == custom_id:
		return _signed_in_payload(custom_id)
	# Different identity on the same client process is now the EXPECTED
	# path between scenarios: TestClient rotates through a pool of N
	# accounts per role so per-(account, endpoint) PlayFab rate-limit
	# budgets are spread across many identities instead of stacked on
	# one. Transparently drop the local reference and the per-op call
	# history (the new account has a fresh budget) before issuing the
	# fresh LoginWithCustomID — see _internal_sign_out for details and
	# tests/godot/mp_test_client/scripts/test_client.gd for the
	# rotation index. The PlayFab SDK supports multiple resident user
	# tokens; we just track the most recent one.
	if _playfab_user != null and _signed_in_custom_id != custom_id:
		_internal_sign_out()

	var users: Object = pf.get_users()
	var sign_in_res: Variant = await await_completion_with_rate_limit_retry(
		func(): return users.sign_in_with_custom_id_async(custom_id, create_account),
		"sign_in_with_custom_id_async",
		DEFAULT_AWAIT_TIMEOUT_MS,
	)
	if sign_in_res == null or not bool(sign_in_res.ok):
		return _err("sign_in_failed", _describe_result(sign_in_res, "sign_in_with_custom_id_async"))
	_playfab_user = sign_in_res.data
	_signed_in_custom_id = custom_id

	_multiplayer = pf.get_multiplayer()
	if initialize_multiplayer and _multiplayer != null and not _multiplayer.is_initialized():
		var mp_res: Variant = await await_completion(_multiplayer.initialize_async(), DEFAULT_AWAIT_TIMEOUT_MS)
		if mp_res == null or not bool(mp_res.ok):
			return _err("multiplayer_init_failed", _describe_result(mp_res, "multiplayer.initialize_async"))

	return _signed_in_payload(custom_id)


## Drop the local PlayFab user reference and clear per-op call history.
## Does NOT call into the SDK to sign out — PlayFab Core supports multiple
## resident user tokens, and the next sign_in_with_custom_id will produce
## a fresh user entity that becomes the active identity for subsequent
## per-user calls (the explicit `user` argument to create_lobby_async,
## etc.). Scenarios must release lobby/party/match handles owned by the
## old user BEFORE this is called (orchestrator does this via
## reset_client between scenarios).
func _internal_sign_out() -> void:
	_playfab_user = null
	_signed_in_custom_id = ""
	# Fresh account = fresh per-(account, endpoint) budget. Drop the
	# history so the proactive limiter doesn't pessimistically pace the
	# new account against the previous account's call cadence.
	_call_history.clear()


func _signed_in_payload(custom_id: String) -> Dictionary:
	return {
		"ok": true,
		"custom_id": custom_id,
		"entity_key": _playfab_user.get_entity_key(),
		"multiplayer_initialized": _multiplayer != null and _multiplayer.is_initialized(),
	}


func _sleep_ms(ms: int) -> void:
	if _tree == null:
		# Best-effort: spin in real-time without a SceneTree. Production
		# orchestrator always provides a tree; this path only triggers
		# in unit-style smoke tests.
		var deadline: int = Time.get_ticks_msec() + ms
		while Time.get_ticks_msec() < deadline:
			pass
		return
	var timer: SceneTreeTimer = _tree.create_timer(float(ms) / 1000.0)
	await timer.timeout


## Await a single-arg completion signal returned by an *_async function.
## Returns the emitted value, or null on timeout.
func await_completion(async_signal: Variant, timeout_ms: int = DEFAULT_AWAIT_TIMEOUT_MS) -> Variant:
	if typeof(async_signal) != TYPE_SIGNAL:
		return null
	var state: Dictionary = { "done": false, "value": null }
	async_signal.connect(
		func(value):
			state["done"] = true
			state["value"] = value,
		CONNECT_ONE_SHOT,
	)
	var deadline_ms: int = Time.get_ticks_msec() + timeout_ms
	while not bool(state["done"]):
		dispatch()
		if Time.get_ticks_msec() >= deadline_ms:
			return null
		if _tree != null:
			await _tree.process_frame
	dispatch()
	return state["value"]


## Await an *_async call with automatic retry on PlayFab rate-limit errors.
##
## Takes a `signal_factory` Callable (not a signal directly) because each
## retry has to re-issue the original async call to get a fresh signal —
## a single emitted signal cannot be re-awaited. The factory is called
## once per attempt with no arguments and is expected to return the
## Signal produced by an `*_async` method.
##
## Rate-limit detection covers both the client-side HRESULT
## (E_PF_API_CLIENT_REQUEST_RATE_LIMIT_EXCEEDED, 0x892354DD) and the
## service-side 429 messages (see RATE_LIMIT_SERVICE_SIGNALS). Non
## rate-limit failures fall through immediately so we don't burn the
## retry budget on real bugs. Backoff is 3s/6s/... (RATE_LIMIT_BACKOFF_MS
## * attempt) for up to RATE_LIMIT_MAX_ATTEMPTS attempts total.
##
## Operations covered today: sign_in_with_custom_id_async,
## create_lobby_async, set_properties_async, set_member_properties_async.
## Extend the list by wiring more ops through this helper — the heuristic
## is "any PlayFab op that scenarios can fan out quickly enough to trip
## a 429".
func await_completion_with_rate_limit_retry(signal_factory: Callable, op_name: String, timeout_ms: int = DEFAULT_AWAIT_TIMEOUT_MS) -> Variant:
	# Proactive pacing: if this op has a known PlayFab quota and we'd
	# exceed it in the current sliding window, sleep until the oldest
	# tracked call falls out of the window. This avoids actually hitting
	# the service-side 429 on the fast path; the reactive retry below is
	# the backstop for unexpected limits (SDK-internal calls under the
	# same endpoint family, e.g. subscribe-to-resource hidden inside a
	# create_lobby).
	await _reserve_call_slot(op_name)

	var result: Variant = null
	for attempt in range(RATE_LIMIT_MAX_ATTEMPTS):
		var sig: Variant = signal_factory.call()
		result = await await_completion(sig, timeout_ms)
		if result != null and bool(result.ok):
			return result
		if attempt + 1 >= RATE_LIMIT_MAX_ATTEMPTS:
			return result
		if not _is_rate_limit_error(result):
			return result
		var backoff_ms: int = mini(RATE_LIMIT_BACKOFF_CAP_MS, RATE_LIMIT_BACKOFF_MS * (attempt + 1))
		printerr("[playfab_runtime] %s rate-limited; attempt %d/%d, waiting %dms" % [
			op_name, attempt + 1, RATE_LIMIT_MAX_ATTEMPTS, backoff_ms,
		])
		await _sleep_ms(backoff_ms)
	return result


## Proactive sliding-window admission: if RATE_BUDGETS has an entry for
## op_name and the current window is full, sleep until it isn't, then
## record the new call's timestamp. No-op when op_name isn't in the
## budget table (the reactive retry above will still handle surprises).
func _reserve_call_slot(op_name: String) -> void:
	var budget: Variant = RATE_BUDGETS.get(op_name, null)
	if budget == null:
		return
	var max_calls: int = int(budget["max_calls"])
	var window_ms: int = int(budget["window_ms"])
	if not _call_history.has(op_name):
		_call_history[op_name] = []
	var history: Array = _call_history[op_name]
	_evict_expired(history, window_ms)
	if history.size() >= max_calls:
		var wait_ms: int = (int(history[0]) + window_ms) - Time.get_ticks_msec() + 250
		if wait_ms > 0:
			printerr("[playfab_runtime] %s budget reached (%d/%d in last %dms); pacing %dms" % [
				op_name, history.size(), max_calls, window_ms, wait_ms,
			])
			await _sleep_ms(wait_ms)
		_evict_expired(history, window_ms)
	history.append(Time.get_ticks_msec())


func _evict_expired(history: Array, window_ms: int) -> void:
	var now_ms: int = Time.get_ticks_msec()
	while not history.is_empty() and now_ms - int(history[0]) >= window_ms:
		history.pop_front()


func _is_rate_limit_error(result: Variant) -> bool:
	if result == null:
		return false
	var message: String = String(result.message) if "message" in result else ""
	if message.findn(RATE_LIMIT_HRESULT) >= 0:
		return true
	for signal_text in RATE_LIMIT_SERVICE_SIGNALS:
		if message.findn(signal_text) >= 0:
			return true
	return false


## Poll `condition` until it returns true or `timeout_ms` elapses. Pumps
## the PlayFab dispatcher between polls so SDK state-change events can
## apply to the local cache (set_*_properties acknowledges at the service
## but the local cache is event-driven, so an immediate read can be
## stale). Yields one frame per iteration when a SceneTree is bound; on
## the unit-test path with no tree, busy-spins at real time.
func wait_until(condition: Callable, timeout_ms: int = DEFAULT_AWAIT_TIMEOUT_MS) -> bool:
	var deadline_ms: int = Time.get_ticks_msec() + timeout_ms
	while Time.get_ticks_msec() < deadline_ms:
		dispatch()
		if bool(condition.call()):
			return true
		if _tree != null:
			await _tree.process_frame
	dispatch()
	return bool(condition.call())


# ---------------------------------------------------------------------------
# Internals
# ---------------------------------------------------------------------------

func _apply_env_configuration() -> void:
	var title_id: String = OS.get_environment(ENV_TITLE_ID).strip_edges()
	if not title_id.is_empty():
		ProjectSettings.set_setting(SETTING_TITLE_ID, title_id)
	var endpoint: String = OS.get_environment(ENV_ENDPOINT).strip_edges()
	if not endpoint.is_empty():
		ProjectSettings.set_setting(SETTING_ENDPOINT, endpoint)


func _err(code: String, message: String) -> Dictionary:
	return {
		"ok": false,
		"error": { "code": code, "message": message },
	}


func _describe_result(result: Variant, call_name: String) -> String:
	if result == null:
		return "%s returned null (timeout?)" % call_name
	if not "code" in result and not "message" in result:
		return "%s failed (no diagnostic)" % call_name
	var code: String = String(result.code) if "code" in result else "?"
	var msg: String = String(result.message) if "message" in result else ""
	return "%s failed: %s (%s)" % [call_name, msg, code]
