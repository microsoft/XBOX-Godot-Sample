extends Node

## Tutorial 1 — Sign in a user (state-machine autoload).
##
## The `Auth` autoload is the tutorial app's identity service. It runs a
## phased sign-in:
##   1. UNINITIALIZED  → SIGNING_IN_XBOX     (GDK: check → silent → UI)
##   2. SIGNING_IN_XBOX → SIGNING_IN_PLAYFAB (PlayFab.sign_in_with_xuser)
##   3. SIGNING_IN_PLAYFAB → SIGNED_IN (or FAILED at any step)
##
## Consumers gate work by awaiting [code]Auth.sign_in()[/code], which is
## idempotent and joins an in-flight attempt instead of starting a new
## one. The single [code]state_changed[/code] signal carries the new
## state; accessors return the current truth.
##
##     if not await Auth.sign_in():
##         _show_error(Auth.get_last_error_stage(), Auth.get_last_error_message())
##         return
##     var user: GDKUser = Auth.xbox_user
##
## Source: docs/tutorials/01-sign-in-user.md

enum State {
	UNINITIALIZED,
	SIGNING_IN_XBOX,
	SIGNING_IN_PLAYFAB,
	SIGNED_IN,
	FAILED,
}

signal state_changed(state: State)

var _state: State = State.UNINITIALIZED
var _xbox_user: GDKUser = null
var _playfab_user: PlayFabUser = null
var _last_error_stage: String = ""
var _last_error_message: String = ""

# Read-only typed accessors. xbox_user/playfab_user are intentionally
# null unless the full chain reached SIGNED_IN so consumers cannot
# accidentally use a half-completed session (e.g. Xbox-only without
# PlayFab) — that ambiguity is the bug item 7 from the sample review.
var xbox_user: GDKUser:
	get:
		return _xbox_user if _state == State.SIGNED_IN else null
	set(_value):
		push_error("[Auth] xbox_user is read-only — drive state via sign_in()")

var playfab_user: PlayFabUser:
	get:
		return _playfab_user if _state == State.SIGNED_IN else null
	set(_value):
		push_error("[Auth] playfab_user is read-only — drive state via sign_in()")

func get_state() -> State:
	return _state

func is_signed_in() -> bool:
	return _state == State.SIGNED_IN

func is_signing_in() -> bool:
	return _state == State.SIGNING_IN_XBOX or _state == State.SIGNING_IN_PLAYFAB

func is_failed() -> bool:
	return _state == State.FAILED

func get_last_error_stage() -> String:
	return _last_error_stage

func get_last_error_message() -> String:
	return _last_error_message

## Idempotent. Joins an in-flight attempt if one is already running.
## Returns [code]true[/code] when the local user is signed in, [code]false[/code]
## on failure (read [code]get_last_error_stage()[/code] / [code]get_last_error_message()[/code]
## for diagnosis).
func sign_in() -> bool:
	if _state == State.SIGNED_IN:
		return true
	if is_signing_in():
		# Coalesce concurrent callers. The first caller's _do_sign_in()
		# set the SIGNING_IN_* state synchronously before its first
		# await, so subsequent callers always observe the in-flight
		# state and wait for completion here.
		while is_signing_in():
			await state_changed
		return _state == State.SIGNED_IN
	# UNINITIALIZED or FAILED → start a fresh attempt.
	return await _do_sign_in()

func _ready() -> void:
	# Kick off silent sign-in immediately so the rest of the autoloads
	# (Lobby, Party) and the first scene to load can simply
	# `await Auth.sign_in()` and join the in-flight attempt.
	sign_in()

func _do_sign_in() -> bool:
	_last_error_stage = ""
	_last_error_message = ""
	_xbox_user = null
	_playfab_user = null

	_set_state(State.SIGNING_IN_XBOX)
	var xbox: GDKUser = await _ensure_xbox_user()
	if xbox == null:
		_set_state(State.FAILED)
		return false
	_xbox_user = xbox
	print("[Auth] Xbox primary user: %s" % xbox.gamertag)

	_set_state(State.SIGNING_IN_PLAYFAB)
	var pf: PlayFabUser = await _ensure_playfab_user(xbox)
	if pf == null:
		_set_state(State.FAILED)
		return false
	_playfab_user = pf

	var key: Dictionary = pf.entity_key
	print("[Auth] PlayFab session: %s:%s" % [key.get("type", ""), key.get("id", "")])
	print("[Auth] Sign-in complete.")

	_set_state(State.SIGNED_IN)
	return true

func _set_state(new_state: State) -> void:
	if _state == new_state:
		return
	_state = new_state
	state_changed.emit(_state)

func _set_error(stage: String, message: String) -> void:
	_last_error_stage = stage
	_last_error_message = message
	push_warning("[Auth] sign-in failed at %s: %s" % [stage, message])

func _ensure_xbox_user() -> GDKUser:
	if not Engine.has_singleton("GDK"):
		_set_error("gdk.missing", "godot_gdk extension is not loaded")
		return null

	if not GDK.is_initialized():
		var init: GDKResult = GDK.initialize()
		if not init.ok:
			_set_error("gdk.initialize", init.message)
			return null

	# 1. Already have a primary user (auto-init, prior sign-in)? Use it.
	var primary: GDKUser = GDK.users.get_primary_user()
	if primary != null and primary.signed_in:
		return primary

	# 2. Try the silent path. This picks up the Xbox-app account on the
	#    PC without surfacing any UI. Common failure: no_default_user.
	var silent: GDKResult = await GDK.users.add_default_user_async()
	if silent.ok and silent.data != null and silent.data.signed_in:
		return silent.data

	print("[Auth] Silent sign-in failed (%s) — falling back to UI." % silent.message)

	# 3. UI fallback. Shows the system account picker.
	var ui: GDKResult = await GDK.users.add_user_with_ui_async()
	if ui.ok and ui.data != null and ui.data.signed_in:
		return ui.data

	_set_error("gdk.add_user_with_ui", ui.message)
	return null

func _ensure_playfab_user(xbox: GDKUser) -> PlayFabUser:
	if not Engine.has_singleton("PlayFab"):
		_set_error("playfab.missing", "godot_playfab extension is not loaded")
		return null

	if xbox == null or not xbox.signed_in:
		_set_error("playfab.sign_in", "Xbox user is not signed in")
		return null

	if not PlayFab.is_initialized():
		var init: PlayFabResult = PlayFab.initialize()
		if not init.ok:
			_set_error("playfab.initialize", init.message)
			return null

	# Pass the GDKUser object directly. The addon reads the local user
	# handle out of it internally; the boundary is intentionally typed
	# as Object because Ref<> types cannot cross GDExtension DLLs.
	var result: PlayFabResult = await PlayFab.users.sign_in_with_xuser_async(xbox)
	if not result.ok:
		_set_error("playfab.sign_in", result.message)
		return null

	return result.data
