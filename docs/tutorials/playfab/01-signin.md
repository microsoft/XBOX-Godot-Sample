# PlayFab Tutorial 1 — Custom-id sign-in

## What you'll build

Build the `PlayFabAuth` autoload used by the PlayFab-only track. It initializes `PlayFab`, signs in with a stable custom id through `PlayFab.users.sign_in_with_custom_id_async`, exposes `playfab_user`, and never touches Xbox/GDK.

## Prerequisites

- Complete the PlayFab portions of [Addons getting started](../../addon-getting-started.md).
- Set `playfab/runtime/title_id` for `sample/tutorial_playfab`.
- Build the addons so `sample/tutorial_playfab/addons/` is mirrored.
- No `MicrosoftGame.config` is required.

## Relevant addon surfaces

- [`PlayFab`](../../../addons/godot_playfab/doc_classes/PlayFab.xml) — runtime initialization.
- [`PlayFabUsers`](../../../addons/godot_playfab/doc_classes/PlayFabUsers.xml) — `get_user_by_custom_id` and `sign_in_with_custom_id_async`.
- [`PlayFabUser`](../../../addons/godot_playfab/doc_classes/PlayFabUser.xml) — signed-in entity key.
- [`PlayFabResult`](../../../addons/godot_playfab/doc_classes/PlayFabResult.xml) — normalized async results.

## Steps

### Step 1 — Add the `PlayFabAuth` autoload

Create `res://autoload/playfab_auth.gd`, then register it as `PlayFabAuth`. The custom id should be stable for a device/account in a real game; the tutorial uses a fixed id so repeated runs reuse the same PlayFab account.

```gdscript
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
```

### Step 2 — Render sign-in state in `p01_signin`

The scene mirrors the GDK sign-in panel but only displays the PlayFab entity key.

```gdscript
extends Control

## PlayFab Tutorial 1 reference scene — PlayFab sign-in status panel.
##
## Reads the `PlayFabAuth` autoload and renders the current sign-in state
## via PlayFabAuth.state_changed. Pressing **Sign in** re-runs the custom-id
## sign-in; **Back** returns to the picker. PlayFab-only: no Xbox step.
##
## NOTE: scene scripts use `get_node("/root/PlayFabAuth")` instead of the
## bare `PlayFabAuth.` reference so the headless parse gate stays clean.
##
## Source: docs/tutorials/playfab/01-signin.md

@onready var _identity: Label = $Root/Identity
@onready var _status: Label = $Root/Status
@onready var _sign_in_button: Button = $Root/Buttons/SignIn
@onready var _back_button: Button = $Root/Buttons/Back

var _auth: Node = null

func _ready() -> void:
	_back_button.pressed.connect(_on_back_pressed)
	_sign_in_button.pressed.connect(_on_sign_in_pressed)

	_auth = get_node_or_null("/root/PlayFabAuth")
	if _auth == null:
		_status.text = "PlayFabAuth autoload missing — register autoload/playfab_auth.gd in project.godot."
		_sign_in_button.disabled = true
		return

	_auth.state_changed.connect(_on_auth_state_changed)
	_refresh()

func _refresh() -> void:
	if _auth == null:
		return
	if _auth.call("is_signed_in"):
		_refresh_identity(_auth.get("playfab_user"))
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

func _refresh_identity(playfab_user) -> void:
	if playfab_user == null:
		_identity.text = "PlayFab: (not signed in)"
		return
	var key: Dictionary = playfab_user.entity_key
	_identity.text = "PlayFab: %s:%s" % [key.get("type", ""), key.get("id", "")]

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

### Step 3 — Consume PlayFab sign-in from later scenes

```gdscript
if not await PlayFabAuth.sign_in():
	push_error("PlayFab sign-in failed at %s: %s" % [
		PlayFabAuth.get_last_error_stage(),
		PlayFabAuth.get_last_error_message(),
	])
	return

var user: PlayFabUser = PlayFabAuth.playfab_user
```

## Verify

Run `sample/tutorial_playfab`, open `p01_signin`, and press **Sign in**. The scene should show `PlayFab: title_player_account:<id>` and Output should include `[PlayFabAuth] Sign-in complete.`

## Common failures

| Output | Diagnosis | Fix |
|---|---|---|
| `PlayFabAuth autoload missing` | Autoload missing or wrong name. | Register `autoload/playfab_auth.gd` as `PlayFabAuth`. |
| `playfab.missing` | The PlayFab extension did not load. | Build the addons and confirm mirrored binaries exist. |
| `playfab.initialize` | Title id missing or invalid. | Set `playfab/runtime/title_id`. |
| `playfab.sign_in` | Custom-id login disabled or service unavailable. | Enable custom-id sign-in and retry. |

## Reference implementation

- Scene: [`sample/tutorial_playfab/p01_signin.tscn`](../../../sample/tutorial_playfab/p01_signin.tscn)
- Scene script: [`sample/tutorial_playfab/p01_signin.gd`](../../../sample/tutorial_playfab/p01_signin.gd)
- Autoload: [`sample/tutorial_playfab/autoload/playfab_auth.gd`](../../../sample/tutorial_playfab/autoload/playfab_auth.gd)

## Next

Continue to [PlayFab Tutorial 2 — Leaderboard](02-leaderboard.md).
