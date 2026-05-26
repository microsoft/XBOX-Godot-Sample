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
  - All four addons (`godot_gdk`, `godot_playfab`, optionally
    `godot_gameinput`, plus `godot_gdk_packaging`) are enabled
    in **Project Settings → Plugins**.
  - `playfab/runtime/title_id` is set.
  - `MicrosoftGame.config` has real Title ID and SCID values
    (not the template defaults).
  - The PC is switched into the right Xbox sandbox.
- An Xbox **test account** signed into the Xbox app on the dev PC for
  the silent path to succeed without surfacing UI.
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
- [`PlayFab.users`](../playfab/plugin.md) —
  `sign_in_with_xuser_async`, `sign_in_with_custom_id_async`.
- [`PlayFabUser`](../playfab/plugin.md) — the typed wrapper your
  `Auth.playfab_user` holds. Read `entity_key`,
  `has_local_user_handle`.
- [`GDKResult`](../../addons/godot_gdk/doc_classes/GDKResult.xml)
  / `PlayFabResult` — the normalized result type returned by every
  `_async` call (see [Async patterns](../async-patterns.md)).

## Step 1 — Add an `Auth` autoload

Create `res://auth/auth.gd`:

```gdscript
extends Node

signal sign_in_completed(xbox_user: GDKUser, playfab_user: PlayFabUser)
signal sign_in_failed(stage: String, message: String)

var xbox_user: GDKUser = null
var playfab_user: PlayFabUser = null
```

Then register it as an autoload in **Project Settings → Globals →
Autoload**:

| Path | Node Name |
|---|---|
| `res://auth/auth.gd` | `Auth` |

Every other tutorial can now reach the signed-in users as
`Auth.xbox_user` and `Auth.playfab_user`.

## Step 2 — Reach a `GDKUser` (check → silent → UI)

Add the Xbox sign-in routine to `auth.gd`:

```gdscript
func _ensure_xbox_user() -> GDKUser:
    if not Engine.has_singleton("GDK"):
        push_error("[Auth] godot_gdk extension is not loaded")
        return null

    if not GDK.is_initialized():
        var init: GDKResult = GDK.initialize()
        if not init.ok:
            sign_in_failed.emit("gdk.initialize", init.message)
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

    sign_in_failed.emit("gdk.add_user_with_ui", ui.message)
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
        push_error("[Auth] godot_playfab extension is not loaded")
        return null

    if xbox == null or not xbox.signed_in:
        sign_in_failed.emit("playfab.sign_in", "Xbox user is not signed in")
        return null

    if not PlayFab.is_initialized():
        var init: PlayFabResult = PlayFab.initialize()
        if not init.ok:
            sign_in_failed.emit("playfab.initialize", init.message)
            return null

    # Pass the GDKUser object directly. The addon reads the local user
    # handle out of it internally; the boundary is intentionally typed
    # as Object because Ref<> types cannot cross GDExtension DLLs.
    var result: PlayFabResult = await PlayFab.users.sign_in_with_xuser_async(xbox)
    if not result.ok:
        sign_in_failed.emit("playfab.sign_in", result.message)
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

## Step 4 — Wire the two pieces together

Add an entry point and run both stages in order:

```gdscript
func _ready() -> void:
    await _sign_in()

func _sign_in() -> void:
    xbox_user = await _ensure_xbox_user()
    if xbox_user == null:
        return

    print("[Auth] Xbox primary user: %s" % xbox_user.gamertag)

    playfab_user = await _ensure_playfab_user(xbox_user)
    if playfab_user == null:
        return

    var key: Dictionary = playfab_user.entity_key
    print("[Auth] PlayFab session: %s:%s" % [key.get("type", ""), key.get("id", "")])
    print("[Auth] Sign-in complete.")

    sign_in_completed.emit(xbox_user, playfab_user)
```

That's the whole `Auth` autoload — single responsibility, awaitable,
and idempotent so a screen that needs the session later can call
`await Auth._sign_in()` defensively without double-signing-in.

## Step 5 — Use the signed-in users from a scene

Anywhere else in your game, gate work on `Auth.xbox_user` and
`Auth.playfab_user`. For one-shot work, await the completion signal:

```gdscript
extends Node

func _ready() -> void:
    if Auth.playfab_user == null:
        await Auth.sign_in_completed

    print("Welcome, %s" % Auth.xbox_user.gamertag)
```

For UI screens that show different content depending on sign-in
state, listen to the `sign_in_failed` signal too and render an
"Offline" badge on `gdk.add_user_with_ui` failures — those are the
ones the user can recover from by signing into the Xbox app and
relaunching.

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
| `sign_in_failed("gdk.initialize", "...")` | GDK runtime did not initialize. | Confirm the GDK is installed (`winget install Microsoft.Gaming.GDK`) and the addon `bin/` folder shipped into the project. |
| `sign_in_failed("playfab.initialize", "...")` | PlayFab runtime did not initialize. Usually `title_id_required`. | Set `playfab/runtime/title_id` in Project Settings. |
| `sign_in_failed("playfab.sign_in", "invalid_xuser")` | The Xbox user signed out between sign-in stages. | Retry. If it persists, check the sandbox setting and the test account state. |
| `sign_in_failed("gdk.add_user_with_ui", "...")` | The picker was dismissed or the account flow was canceled. | Run again — `add_user_with_ui_async` is idempotent. |

## What's next

You have a signed-in Xbox user and a signed-in PlayFab session.
Tutorial 2 takes that `GDKUser` and uses it to unlock an Xbox
achievement end-to-end:

- [**Tutorial 2 — Unlock an achievement**](02-unlock-achievement.md)
- Reference: [GDKUsers](../gdk/api-reference.md),
  [PlayFabUsers](../playfab/plugin.md)
