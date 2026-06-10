extends Node

const AddonApi = preload("res://shared/addon_api.gd")

## PlayFab Tutorial — standalone PlayFab sign-in (state-machine autoload).
##
## The `PlayFabAuth` autoload is the PlayFab-only track's identity service.
## Unlike the integrated track's `Auth` autoload, it never touches Xbox: it
## signs into PlayFab with a title-defined custom id
## (PlayFab.users.sign_in_with_custom_id_async), so the track runs without
## the godot_gdk extension at all.
##   1. UNINITIALIZED  → SIGNING_IN (PlayFab.sign_in_with_custom_id)
##   2. SIGNING_IN → SIGNED_IN (or FAILED)
##
## Consumers gate work by awaiting [code]PlayFabAuth.sign_in()[/code], which
## is idempotent and joins an in-flight attempt instead of starting a new
## one. The single [code]state_changed[/code] signal carries the new state.
##
##     if not await PlayFabAuth.sign_in():
##         _show_error(PlayFabAuth.get_last_error_stage(), PlayFabAuth.get_last_error_message())
##         return
##     var user = PlayFabAuth.playfab_user
##
## Source: docs/tutorials/playfab/01-signin.md

# Title-defined custom id. A shipping title derives a stable per-device or
# per-account id (e.g. a persisted GUID); the tutorial uses a fixed string
# so the sample signs into the same PlayFab account every run.
const CUSTOM_ID := "godot-playfab-tutorial-player"

enum State {
	UNINITIALIZED,
	SIGNING_IN,
	SIGNED_IN,
	FAILED,
}

signal state_changed(state: State)

var _state: State = State.UNINITIALIZED
var _playfab_user = null
var _last_error_stage: String = ""
var _last_error_message: String = ""

# Read-only addon-object accessor. playfab_user is intentionally null
# unless the chain reached SIGNED_IN so consumers cannot use a
# half-completed session.
var playfab_user:
	get:
		return _playfab_user if _state == State.SIGNED_IN else null
	set(_value):
		push_error("[PlayFabAuth] playfab_user is read-only — drive state via sign_in()")

func get_state() -> State:
	return _state

func is_signed_in() -> bool:
	return _state == State.SIGNED_IN

func is_signing_in() -> bool:
	return _state == State.SIGNING_IN

func is_failed() -> bool:
	return _state == State.FAILED

func get_last_error_stage() -> String:
	return _last_error_stage

func get_last_error_message() -> String:
	return _last_error_message

## Idempotent. Joins an in-flight attempt if one is already running.
## Returns [code]true[/code] when signed in, [code]false[/code] on failure.
func sign_in() -> bool:
	if _state == State.SIGNED_IN:
		return true
	if is_signing_in():
		while is_signing_in():
			await state_changed
		return _state == State.SIGNED_IN
	return await _do_sign_in()

func _ready() -> void:
	# Kick off sign-in immediately so the first scene to load can simply
	# `await PlayFabAuth.sign_in()` and join the in-flight attempt.
	sign_in()

func _do_sign_in() -> bool:
	_last_error_stage = ""
	_last_error_message = ""
	_playfab_user = null

	_set_state(State.SIGNING_IN)
	var pf = await _ensure_playfab_user()
	if pf == null:
		_set_state(State.FAILED)
		return false
	_playfab_user = pf

	var key: Dictionary = pf.entity_key
	print("[PlayFabAuth] PlayFab session: %s:%s" % [key.get("type", ""), key.get("id", "")])
	print("[PlayFabAuth] Sign-in complete.")

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
	push_warning("[PlayFabAuth] sign-in failed at %s: %s" % [stage, message])

func _ensure_playfab_user():
	if not Engine.has_singleton("PlayFab"):
		_set_error("playfab.missing", "godot_playfab extension is not loaded")
		return null

	if not AddonApi.singleton("PlayFab").is_initialized():
		var init = AddonApi.singleton("PlayFab").initialize()
		if not init.ok:
			_set_error("playfab.initialize", init.message)
			return null

	# Reuse a cached custom-id session if one already exists.
	var cached = AddonApi.singleton("PlayFab").users.get_user_by_custom_id(CUSTOM_ID)
	if cached != null:
		return cached

	# Title-defined custom id sign-in. create_account=true provisions a
	# new PlayFab account on first run; later runs reuse it.
	var result = await AddonApi.singleton("PlayFab").users.sign_in_with_custom_id_async(CUSTOM_ID, true)
	if not result.ok:
		_set_error("playfab.sign_in", result.message)
		return null

	return result.data
