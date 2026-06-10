extends Node

const AddonApi = preload("res://shared/addon_api.gd")

## GDK Tutorial — Xbox sign-in (state-machine autoload).
##
## The `GdkAuth` autoload is the GDK-only track's identity service. Unlike
## the integrated track's `Auth` autoload, it stops at Xbox: there is no
## PlayFab step. It runs a phased sign-in:
##   1. UNINITIALIZED  → SIGNING_IN_XBOX (GDK: check → silent → UI)
##   2. SIGNING_IN_XBOX → SIGNED_IN (or FAILED at any step)
##
## Consumers gate work by awaiting [code]GdkAuth.sign_in()[/code], which is
## idempotent and joins an in-flight attempt instead of starting a new one.
## The single [code]state_changed[/code] signal carries the new state;
## accessors return the current truth.
##
##     if not await GdkAuth.sign_in():
##         _show_error(GdkAuth.get_last_error_stage(), GdkAuth.get_last_error_message())
##         return
##     var user = GdkAuth.xbox_user
##
## Source: docs/tutorials/gdk/01-signin.md

enum State {
	UNINITIALIZED,
	SIGNING_IN_XBOX,
	SIGNED_IN,
	FAILED,
}

signal state_changed(state: State)

var _state: State = State.UNINITIALIZED
var _xbox_user = null
var _last_error_stage: String = ""
var _last_error_message: String = ""

# Read-only addon-object accessor. xbox_user is intentionally null unless
# the chain reached SIGNED_IN so consumers cannot accidentally use a
# half-completed session.
var xbox_user:
	get:
		return _xbox_user if _state == State.SIGNED_IN else null
	set(_value):
		push_error("[GdkAuth] xbox_user is read-only — drive state via sign_in()")

func get_state() -> State:
	return _state

func is_signed_in() -> bool:
	return _state == State.SIGNED_IN

func is_signing_in() -> bool:
	return _state == State.SIGNING_IN_XBOX

func is_failed() -> bool:
	return _state == State.FAILED

func get_last_error_stage() -> String:
	return _last_error_stage

func get_last_error_message() -> String:
	return _last_error_message

## Idempotent. Joins an in-flight attempt if one is already running.
## Returns [code]true[/code] when the local user is signed in,
## [code]false[/code] on failure (read [code]get_last_error_stage()[/code] /
## [code]get_last_error_message()[/code] for diagnosis).
func sign_in() -> bool:
	if _state == State.SIGNED_IN:
		return true
	if is_signing_in():
		# Coalesce concurrent callers. The first caller's _do_sign_in()
		# set SIGNING_IN_XBOX synchronously before its first await, so
		# subsequent callers always observe the in-flight state here.
		while is_signing_in():
			await state_changed
		return _state == State.SIGNED_IN
	# UNINITIALIZED or FAILED → start a fresh attempt.
	return await _do_sign_in()

func _ready() -> void:
	# Kick off silent sign-in immediately so the first scene to load can
	# simply `await GdkAuth.sign_in()` and join the in-flight attempt.
	sign_in()

func _do_sign_in() -> bool:
	_last_error_stage = ""
	_last_error_message = ""
	_xbox_user = null

	_set_state(State.SIGNING_IN_XBOX)
	var xbox = await _ensure_xbox_user()
	if xbox == null:
		_set_state(State.FAILED)
		return false
	_xbox_user = xbox
	print("[GdkAuth] Xbox primary user: %s" % xbox.gamertag)
	print("[GdkAuth] Sign-in complete.")

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
	push_warning("[GdkAuth] sign-in failed at %s: %s" % [stage, message])

func _ensure_xbox_user():
	if not Engine.has_singleton("GDK"):
		_set_error("gdk.missing", "godot_gdk extension is not loaded")
		return null

	if not AddonApi.singleton("GDK").is_initialized():
		var init = AddonApi.singleton("GDK").initialize()
		if not init.ok:
			_set_error("gdk.initialize", init.message)
			return null

	# 1. Already have a primary user (auto-init, prior sign-in)? Use it.
	var primary = AddonApi.singleton("GDK").users.get_primary_user()
	if primary != null and primary.signed_in:
		return primary

	# 2. Try the silent path. This picks up the Xbox-app account on the
	#    PC without surfacing any UI. Common failure: no_default_user.
	var silent = await AddonApi.singleton("GDK").users.add_default_user_async()
	if silent.ok and silent.data != null and silent.data.signed_in:
		return silent.data

	print("[GdkAuth] Silent sign-in failed (%s) — falling back to UI." % silent.message)

	# 3. UI fallback. Shows the system account picker.
	var ui = await AddonApi.singleton("GDK").users.add_user_with_ui_async()
	if ui.ok and ui.data != null and ui.data.signed_in:
		return ui.data

	_set_error("gdk.add_user_with_ui", ui.message)
	return null
