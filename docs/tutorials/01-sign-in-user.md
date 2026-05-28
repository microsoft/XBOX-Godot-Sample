# Tutorial 1 — Sign in a user

## What you'll build

A single autoload that brings up Xbox Live first, then PlayFab on top
of it, and exposes both sessions to the rest of your game. The flow
is the **check → silent → UI** fallback for Xbox, followed by
`sign_in_with_xuser_async` for PlayFab. When it works, your Output
panel ends with something like:

```
[Auth] Xbox primary user: SteelGorilla
[Auth] PlayFab session: title_player_account:6F4B...
[Auth] Sign-in complete.
```

Everything in tutorials 2–5 assumes you can reach this state.

## Prerequisites

- The [addons quickstart](../addon-getting-started.md) is complete:
  all four addons (`godot_gdk`, `godot_playfab`, optionally
  `godot_gameinput`, plus `godot_gdk_packaging`) are enabled in
  **Project Settings → Plugins**, and `MicrosoftGame.config` has
  real Title ID and SCID values (not the template defaults). See
  [Configuring Xbox services (Title ID + SCID)](https://learn.microsoft.com/en-us/gaming/gdk/docs/services/fundamentals/portal-config/live-service-config-ids-mp)
  for where these values live in
  [Partner Center](https://partner.microsoft.com/dashboard).
- Your **PlayFab title** is provisioned and `playfab/runtime/title_id`
  is set. The walkthrough — creating the title and configuring the
  Title ID in Project Settings — is documented in
  [PlayFab title prerequisites — §1 Create the title](../playfab/prerequisites.md#1-create-the-title-and-capture-the-title-id).
- Your **PC is in the right Xbox sandbox** and at least one Xbox
  **test account** is signed into the Xbox app for the silent path.
  See [Xbox sandbox and test accounts](../platform/xbox-sandbox-and-test-accounts.md).
- `gdk/runtime/initialize_on_startup` and
  `playfab/runtime/initialize_on_startup` are both `true` (the
  quickstart sets these explicitly — they ship `false` out of the
  box). The bootstraps will call `GDK.initialize()` and
  `PlayFab.initialize()` for you; the snippets below still guard with
  `is_initialized()` so they survive a project that has the auto-init
  turned off.

## Relevant addon surfaces

- [`GDK.users`](../../addons/godot_gdk/doc_classes/GDKUsers.xml)
  — `get_primary_user`, `add_default_user_async`,
  `add_user_with_ui_async`, `user_changed` (avoid for sign-in
  bootstrap).
- [`GDKUser`](../../addons/godot_gdk/doc_classes/GDKUser.xml) —
  the typed wrapper your `Auth.xbox_user` holds. Read
  `signed_in`, `gamertag`, `xbox_user_id`.
- [`PlayFab.users`](../../addons/godot_playfab/doc_classes/PlayFabUsers.xml) —
  `sign_in_with_xuser_async`, `sign_in_with_custom_id_async`.
- [`PlayFabUser`](../../addons/godot_playfab/doc_classes/PlayFabUser.xml) — the typed wrapper your
  `Auth.playfab_user` holds. Read `entity_key`,
  `has_local_user_handle`.
- [`GDKResult`](../../addons/godot_gdk/doc_classes/GDKResult.xml)
  / [`PlayFabResult`](../../addons/godot_playfab/doc_classes/PlayFabResult.xml) — the normalized result type returned by every
  `_async` call (see [Async patterns](../async-patterns.md)).

## Step 1 — Add an `Auth` autoload

Sign-in is a phased flow with explicit failure modes, so model
it as a state machine. Create `res://auth/auth.gd`:

```gdscript
extends Node

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

# Guarded getters so callers can't accidentally use a half-completed
# session (e.g. Xbox signed in, PlayFab still in flight).
var xbox_user: GDKUser:
    get:
        return _xbox_user if _state == State.SIGNED_IN else null

var playfab_user: PlayFabUser:
    get:
        return _playfab_user if _state == State.SIGNED_IN else null

func get_state() -> State: return _state
func is_signed_in() -> bool: return _state == State.SIGNED_IN
func is_signing_in() -> bool:
    return _state == State.SIGNING_IN_XBOX or _state == State.SIGNING_IN_PLAYFAB
func is_failed() -> bool: return _state == State.FAILED
func get_last_error_stage() -> String: return _last_error_stage
func get_last_error_message() -> String: return _last_error_message
```

Then register it as an autoload in **Project Settings → Globals →
Autoload**:

| Path | Node Name |
|---|---|
| `res://auth/auth.gd` | `Auth` |

Every other tutorial can now reach the signed-in users as
`Auth.xbox_user` and `Auth.playfab_user`, listen for transitions
on `Auth.state_changed`, and call `await Auth.sign_in()` when
they need to gate work on a completed session.

## Step 2 — Reach a `GDKUser` (check → silent → UI)

Add the Xbox sign-in routine to `auth.gd`:

```gdscript
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
```

A few things worth calling out:

- The three signed-in checks (`signed_in`, `.data != null`,
  `.ok`) all matter. `add_default_user_async` resolves to a result
  that is "ok" with `data == null` in a handful of edge cases (for
  example a stale handle); the explicit chain rejects those.
- This is the **only** Xbox sign-in entry point your game should
  call. Do **not** drive sign-in from `GDK.users.user_changed` —
  that signal fires for every user lifecycle event (adds, removes,
  gamertag changes, picture changes, privilege changes) and is not
  the right hook for session bootstrap.
- `add_user_with_ui_async()` opens system UI, so it must be called
  in response to a user action when the engine is in a state that
  allows UI presentation. Running it from `_ready` is fine; running
  it during shutdown or from a worker thread is not.

## Step 3 — Hand the Xbox user to PlayFab

PlayFab does not have its own silent/UI distinction — once you have a
signed-in `GDKUser`, `sign_in_with_xuser_async` does the rest:

```gdscript
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
```

The two failure codes worth recognizing here are:

- `invalid_xuser` — the GDK user passed in was null or signed out
  between the check above and the call. The chain in step 2 makes
  this unlikely, but a user who signs out between frames will
  surface it.
- `title_id_required` — `playfab/runtime/title_id` is empty.
  Re-read the [addons quickstart](../addon-getting-started.md) and
  the [PlayFab plugin overview](../playfab/plugin.md).

## Step 4 — Drive the state machine

Wire `_ensure_xbox_user()` and `_ensure_playfab_user()` together
under a single public `sign_in()` entry point. The transition
helpers (`_set_state`, `_set_error`) keep state mutations + signal
emission in one place; the public entry point is idempotent and
joins an in-flight attempt so callers can `await Auth.sign_in()`
defensively. Add the following to `auth.gd`:

```gdscript
func _ready() -> void:
    # Kick off silent sign-in immediately so the first scene to load
    # can simply `await Auth.sign_in()` and join the in-flight attempt.
    sign_in()

func sign_in() -> bool:
    if _state == State.SIGNED_IN:
        return true
    if is_signing_in():
        # Coalesce concurrent callers. The first caller's _do_sign_in()
        # sets SIGNING_IN_XBOX before its first await, so anyone arriving
        # later sees the in-flight state and waits here for completion.
        while is_signing_in():
            await state_changed
        return _state == State.SIGNED_IN
    # UNINITIALIZED or FAILED → start a fresh attempt.
    return await _do_sign_in()

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
```

That's the whole `Auth` autoload — single responsibility, awaitable,
and idempotent. `sign_in()` is safe to call from any number of
scenes and panels: the first call drives the state machine
forward, every subsequent call (before the first finishes) waits
on `state_changed` for the same transition.

## Step 5 — Use the signed-in users from a scene

Anywhere else in your game, gate work on `await Auth.sign_in()`
and react to transitions via `Auth.state_changed`:

```gdscript
extends Node

func _ready() -> void:
    if not await Auth.sign_in():
        push_warning("Sign-in failed at %s: %s" % [
                Auth.get_last_error_stage(),
                Auth.get_last_error_message()])
        return

    print("Welcome, %s" % Auth.xbox_user.gamertag)
```

For UI screens that show different content depending on sign-in
state, connect `state_changed` and branch on the accessors:

```gdscript
func _ready() -> void:
    Auth.state_changed.connect(_on_state_changed)
    _on_state_changed(Auth.get_state())

func _on_state_changed(_state: Auth.State) -> void:
    if Auth.is_signed_in():
        _badge.text = "Signed in as %s" % Auth.xbox_user.gamertag
    elif Auth.is_signing_in():
        _badge.text = "Signing in…"
    elif Auth.is_failed():
        _badge.text = "Offline (%s)" % Auth.get_last_error_stage()
    else:
        _badge.text = "(not signed in)"
```

A `gdk.add_user_with_ui` failure is the one the user can recover
from by signing into the Xbox app and tapping a retry button that
calls `await Auth.sign_in()` again — `sign_in()` resets stale
state on each fresh attempt so it's safe to retry.

## Verify

A successful first run prints, in order:

```
[GDK] Bootstrap: GDK.initialize() succeeded.
[GDK] Runtime initialized
[PlayFab] Bootstrap: PlayFab.initialize() succeeded.
[PlayFab] Runtime initialized
[Auth] Xbox primary user: SteelGorilla
[Auth] PlayFab session: title_player_account:6F4BAA...
[Auth] Sign-in complete.
```

If the silent path falls back to UI you will see the system account
picker pop up; pick a test account and the rest of the sequence runs.

The most common failure paths and how to read them:

| Output | Diagnosis | Fix |
|---|---|---|
| `[Auth] Silent sign-in failed (no_default_user) — falling back to UI.` | No Xbox-app account on the PC. The fallback handles it. | Pick a test account in the picker, or sign one into the Xbox app first. |
| `[Auth] sign-in failed at gdk.initialize: ...` | GDK runtime did not initialize. | Confirm the GDK is installed (`winget install Microsoft.Gaming.GDK`) and the addon `bin/` folder shipped into the project. |
| `[Auth] sign-in failed at playfab.initialize: ...` | PlayFab runtime did not initialize. Usually `title_id_required`. | Set `playfab/runtime/title_id` in Project Settings. |
| `[Auth] sign-in failed at playfab.sign_in: invalid_xuser` | The Xbox user signed out between sign-in stages. | Retry. If it persists, check the sandbox setting and the test account state. |
| `[Auth] sign-in failed at gdk.add_user_with_ui: ...` | The picker was dismissed or the account flow was canceled. | Run again — `add_user_with_ui_async` is idempotent, and `Auth.sign_in()` resets failure state on each retry. |

## Reference implementation

The cumulative end-state of this tutorial lives in the integrated
sample at [`sample/tutorial_app/`](../../sample/tutorial_app/README.md).
Open the matching scene and compare if your project drifts:

- Scene: [`sample/tutorial_app/t01_signin.tscn`](../../sample/tutorial_app/t01_signin.tscn)
- Script: [`sample/tutorial_app/t01_signin.gd`](../../sample/tutorial_app/t01_signin.gd)
- Autoload introduced here: [`sample/tutorial_app/autoload/auth.gd`](../../sample/tutorial_app/autoload/auth.gd)

> **Path note.** The tutorial places `auth.gd` at `res://auth/auth.gd`
> (one folder per topic) so a reader can follow the chain
> incrementally. The sample collapses every autoload under
> `res://autoload/` because all three (`Auth`, `Lobby`, `Party`) are
> registered from the first tutorial so any picker scene runs out of
> the box. Same code, different folder.

## What's next
