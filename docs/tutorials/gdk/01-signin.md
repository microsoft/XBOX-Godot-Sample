# GDK Tutorial 1 — Xbox-only sign-in

## What you'll build

Build the `GdkAuth` autoload used by the GDK track. It initializes `GDK`, checks for an already-signed-in primary user, tries silent sign-in, falls back to the system account picker, and exposes a read-only `xbox_user` once the state reaches `SIGNED_IN`. The scene `g01_signin` renders that state and lets the player retry.

## Prerequisites

- Complete the GDK parts of [Addons getting started](../../addon-getting-started.md).
- Build the addons so `sample/tutorial_gdk/addons/` is mirrored.
- Configure `sample/tutorial_gdk/MicrosoftGame.config` for your Partner Center title, SCID, and sandbox.
- No PlayFab title id is required for this track.

## Relevant addon surfaces

- [`GDK`](../../../addons/godot_gdk/doc_classes/GDK.xml) — runtime initialization.
- [`GDKUsers`](../../../addons/godot_gdk/doc_classes/GDKUsers.xml) — `get_primary_user`, `add_default_user_async`, and `add_user_with_ui_async`.
- [`GDKUser`](../../../addons/godot_gdk/doc_classes/GDKUser.xml) — signed-in user data (`gamertag`, `xuid`, `signed_in`).
- [`GDKResult`](../../../addons/godot_gdk/doc_classes/GDKResult.xml) — normalized async results.

## Steps

### Step 1 — Add the `GdkAuth` autoload

Create `res://autoload/gdk_auth.gd`, then register it in **Project → Project Settings → Autoload** as `GdkAuth`. The sample uses a small state machine so every scene can await the same sign-in attempt safely.

```gdscript
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
```

### Step 2 — Render the current sign-in state

The scene keeps UI code separate from the autoload. It connects `state_changed`, calls `sign_in()` on button press, and reads `xbox_user` only when signed in.

```gdscript
extends Control

## GDK Tutorial 1 reference scene — Xbox sign-in status panel.
##
## Reads the `GdkAuth` autoload and renders the current sign-in state via
## GdkAuth.state_changed. Pressing **Sign in** re-runs the
## check → silent → UI fallback; **Back** returns to the picker.
##
## NOTE: scene scripts use `get_node("/root/GdkAuth")` instead of the bare
## `GdkAuth.` reference shown in the tutorial markdown so that the headless
## parse gate (`tools\check_gd_scripts_headless.ps1`) — which does not
## resolve GDScript autoloads — stays clean.
##
## Source: docs/tutorials/gdk/01-signin.md

@onready var _identity: Label = $Root/Identity
@onready var _status: Label = $Root/Status
@onready var _sign_in_button: Button = $Root/Buttons/SignIn
@onready var _back_button: Button = $Root/Buttons/Back

var _auth: Node = null

func _ready() -> void:
	_back_button.pressed.connect(_on_back_pressed)
	_sign_in_button.pressed.connect(_on_sign_in_pressed)

	_auth = get_node_or_null("/root/GdkAuth")
	if _auth == null:
		_status.text = "GdkAuth autoload missing — register autoload/gdk_auth.gd in project.godot."
		_sign_in_button.disabled = true
		return

	_auth.state_changed.connect(_on_auth_state_changed)
	_refresh()

func _refresh() -> void:
	if _auth == null:
		return
	if _auth.call("is_signed_in"):
		_refresh_identity(_auth.get("xbox_user"))
		_status.text = "Signed in."
	elif _auth.call("is_signing_in"):
		_refresh_identity(null)
		_status.text = "Signing in…"
	elif _auth.call("is_failed"):
		_refresh_identity(null)
		_status.text = "Sign-in failed at %s: %s" % [
				_auth.call("get_last_error_stage"),
				_auth.call("get_last_error_message")]
	else:
		_refresh_identity(null)
		_status.text = "Not signed in."

func _refresh_identity(xbox_user) -> void:
	if xbox_user == null:
		_identity.text = "Xbox: (not signed in)"
		return
	var gamertag := str(xbox_user.gamertag)
	var xuid := str(xbox_user.xuid)
	_identity.text = "Xbox: %s (%s)" % [gamertag, xuid]

func _on_auth_state_changed(_state) -> void:
	_refresh()

func _on_sign_in_pressed() -> void:
	if _auth == null:
		return
	# sign_in() is idempotent — if already signed in returns immediately;
	# if signing in joins the in-flight attempt; otherwise starts fresh.
	await _auth.call("sign_in")

func _on_back_pressed() -> void:
	get_tree().change_scene_to_file("res://shared/tutorial_picker.tscn")
```

### Step 3 — Consume sign-in from later GDK scenes

```gdscript
if not await GdkAuth.sign_in():
	push_error("Sign-in failed at %s: %s" % [
		GdkAuth.get_last_error_stage(),
		GdkAuth.get_last_error_message(),
	])
	return

var user: GDKUser = GdkAuth.xbox_user
```

## Verify

Run `sample/tutorial_gdk`, open `g01_signin`, and press **Sign in**. You should see the gamertag and XUID. Output should include `[GdkAuth] Xbox primary user:` and `[GdkAuth] Sign-in complete.`

## Common failures

| Output | Diagnosis | Fix |
|---|---|---|
| `GdkAuth autoload missing` | The autoload is not registered or has the wrong name. | Register `autoload/gdk_auth.gd` as `GdkAuth`. |
| `gdk.missing` | The GDK extension did not load. | Build the addons and confirm the mirrored `addons/godot_gdk/bin` files exist. |
| `no_default_user` before UI | No Xbox app default user is available. | This is handled; choose a test account in the UI picker. |
| `gdk.add_user_with_ui` failure | Picker canceled, wrong sandbox, or test account issue. | Retry, verify sandbox, and confirm the account has access to the title. |

## Reference implementation

- Scene: [`sample/tutorial_gdk/g01_signin.tscn`](../../../sample/tutorial_gdk/g01_signin.tscn)
- Scene script: [`sample/tutorial_gdk/g01_signin.gd`](../../../sample/tutorial_gdk/g01_signin.gd)
- Autoload: [`sample/tutorial_gdk/autoload/gdk_auth.gd`](../../../sample/tutorial_gdk/autoload/gdk_auth.gd)

## Next

Continue to [GDK Tutorial 2 — Unlock an achievement](02-achievement.md).
