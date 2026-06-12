# PlayFab Tutorial 1 — Custom-id sign-in

## What you'll build

Build the `PlayFabAuth` autoload used by the PlayFab-only track. It initializes `PlayFab`, resolves a per-instance custom id from `--pf-user` (with environment/default fallbacks), signs in through `PlayFab.users.sign_in_with_custom_id_async`, exposes `playfab_user`, and never touches Xbox/GDK. Because each local instance can resolve a different custom id, you can run multiple local PlayFab users for Lobby and Party testing.

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

Create `res://autoload/playfab_auth.gd`, then register it as `PlayFabAuth`. The autoload resolves the custom id in this order: `--pf-user=<token>` (or `--pf-user <token>`) from command-line/user args, then the `PF_CUSTOM_ID` environment variable, then the default token `player`. The selected token is namespaced under `PREFIX` (`godot-playfab-tutorial-`), so `--pf-user=alice` signs in as `godot-playfab-tutorial-alice`.

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
## Multiple instances, different users. The custom id is resolved per
## instance so you can run several copies of the project locally — each
## signed into its own PlayFab account — to exercise the Lobby and Party
## tutorials without a second machine. Pass a distinct user token on the
## command line:
##
##     godot --path sample/tutorial_playfab -- --pf-user=alice
##
## In the editor, use [b]Debug → Customize Run Instances…[/b] to give each
## instance its own [code]--pf-user=<name>[/code] argument. Resolution order
## is: [code]--pf-user[/code] command-line argument, then the
## [code]PF_CUSTOM_ID[/code] environment variable, then the default token
## [code]player[/code]. The token is namespaced under [constant PREFIX], so
## [code]--pf-user=alice[/code] signs in as [code]godot-playfab-tutorial-alice[/code].
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

# Custom ids are namespaced under this prefix so each tutorial user is a
# distinct PlayFab account (e.g. godot-playfab-tutorial-alice). A shipping
# title derives a stable per-device or per-account id instead.
const PREFIX := "godot-playfab-tutorial-"
const DEFAULT_USER := "player"
# Command-line flag / environment variable that select the per-instance user.
const USER_ARG := "--pf-user"
const USER_ENV := "PF_CUSTOM_ID"

enum State {
	UNINITIALIZED,
	SIGNING_IN,
	SIGNED_IN,
	FAILED,
}

signal state_changed(state: State)

var _state: State = State.UNINITIALIZED
var _playfab_user = null
var _custom_id: String = ""
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

## Returns the resolved per-instance PlayFab custom id (e.g.
## "godot-playfab-tutorial-alice"). Stable for the lifetime of the process.
func get_custom_id() -> String:
	if _custom_id.is_empty():
		_custom_id = _resolve_custom_id()
	return _custom_id

# Resolves the per-instance user token, namespaced under PREFIX. Order:
#   1. --pf-user=<token> (or "--pf-user <token>") on the command line —
#      including user args passed after `--`. Set per instance via the
#      editor's Debug -> Customize Run Instances dialog.
#   2. PF_CUSTOM_ID environment variable.
#   3. DEFAULT_USER ("player").
func _resolve_custom_id() -> String:
	var token := _read_user_arg()
	if token.is_empty():
		token = OS.get_environment(USER_ENV).strip_edges()
	if token.is_empty():
		token = DEFAULT_USER
	return PREFIX + token

func _read_user_arg() -> String:
	var args: PackedStringArray = OS.get_cmdline_args()
	args.append_array(OS.get_cmdline_user_args())
	for i in args.size():
		var arg := args[i]
		if arg.begins_with(USER_ARG + "="):
			return arg.substr((USER_ARG + "=").length()).strip_edges()
		if arg == USER_ARG and i + 1 < args.size():
			return args[i + 1].strip_edges()
	return ""

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
	# Resolve the per-instance custom id before the first sign-in so every
	# consumer (and get_custom_id) sees the same value.
	_custom_id = _resolve_custom_id()
	print("[PlayFabAuth] custom id: %s" % _custom_id)
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
	var custom_id := get_custom_id()
	var cached = AddonApi.singleton("PlayFab").users.get_user_by_custom_id(custom_id)
	if cached != null:
		return cached

	# Title-defined custom id sign-in. create_account=true provisions a
	# new PlayFab account on first run; later runs reuse it.
	var result = await AddonApi.singleton("PlayFab").users.sign_in_with_custom_id_async(custom_id, true)
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
@onready var _subtitle: Label = $Root/Subtitle
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

	# Show which per-instance user this copy signs in as. Run several
	# instances with distinct `--pf-user=<name>` arguments (Debug ->
	# Customize Run Instances) to put different users in a lobby/Party.
	var custom_id: String = _auth.call("get_custom_id")
	_subtitle.text = "Signs in as custom id '%s'. Override per instance with --pf-user=<name>." % custom_id

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

### Running multiple instances

The PlayFab-only track has no Xbox single-user constraint, so you can run several local instances as different PlayFab users. In the editor, use **Debug → Customize Run Instances** and give each instance a distinct user argument such as `--pf-user=alice` and `--pf-user=bob`. From the command line, pass the user argument after Godot's `--` separator:

```powershell
godot --path sample/tutorial_playfab -- --pf-user=alice
```

Each distinct token resolves to a separate namespaced PlayFab custom id (`godot-playfab-tutorial-alice`, `godot-playfab-tutorial-bob`, and so on). Use this to put two local users in the same PlayFab Lobby or Party without a second machine. This is specific to the PlayFab-only track; the GDK and integrated tracks still have the Xbox single-user-per-PC constraint.

## Verify

Run `sample/tutorial_playfab`, open `p01_signin`, and press **Sign in**. Output should include `[PlayFabAuth] custom id: godot-playfab-tutorial-player` (or your `--pf-user`/`PF_CUSTOM_ID` override) followed by `[PlayFabAuth] Sign-in complete.` The P1 subtitle shows the active custom id, and the identity label should show `PlayFab: title_player_account:<id>`.

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
