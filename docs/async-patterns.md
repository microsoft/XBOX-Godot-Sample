# Async patterns in the GodotGDK addons

Every long-running call in the GodotGDK addons returns a one-shot
**Godot Signal** that resolves to a typed **Result** object. The
pattern is identical across `godot_gdk`, `godot_playfab`, and (when
relevant) `godot_gameinput`, so once you know it for one method you
know it for all of them.

This page is the one-page intro the tutorials assume. For the deeper
implementation view of the GDK side specifically (native runtime
queue, `XAsyncBlock` bridge, `XTaskQueueHandle`), see
[`gdk/async-system.md`](gdk/async-system.md).

## The `_async` naming convention

A method whose name ends in `_async` returns a Godot
[`Signal`](https://docs.godotengine.org/en/stable/classes/class_signal.html)
that fires **exactly once** when the underlying GDK / PlayFab /
GameInput operation completes:

| Addon           | Example                                                 |
|-----------------|---------------------------------------------------------|
| `godot_gdk`     | `GDK.users.add_default_user_async()`                    |
| `godot_gdk`     | `GDK.achievements.update_achievement_async(user, id, %)`|
| `godot_playfab` | `PlayFab.users.sign_in_with_xuser_async(xbox_user)`     |
| `godot_playfab` | `PlayFab.multiplayer.create_lobby_async(user, config)`  |

Methods **without** the `_async` suffix are synchronous — they
return the value directly (`GDK.is_initialized() -> bool`,
`GDK.presence.get_cached_presence(xuid) -> GDKPresenceRecord`).

## `await` is the only thing you need

You do not need `connect()` or callbacks for one-shot completions.
`await` works directly on the returned Signal:

```gdscript
func sign_in() -> void:
    var result: GDKResult = await GDK.users.add_default_user_async()
    if not result.ok:
        push_warning("[Auth] silent sign-in failed: %s" % result.message)
        return
    print("[Auth] signed in as %s" % result.data.gamertag)
```

The signal fires on the main thread during the addon's per-frame
dispatch tick, so your `await` resumes from a safe context — you
can touch scene-tree nodes, mutate Godot objects, or call further
`_async` methods directly.

If you want to drive several calls in parallel and wait for them
together, use a small fan-in helper rather than awaiting each one
serially:

```gdscript
func warm_caches() -> void:
    var ach_signal: Signal = GDK.achievements.query_player_achievements_async(Auth.xbox_user)
    var board_signal: Signal = PlayFab.leaderboards.get_leaderboard_async(
            Auth.playfab_user, "high_score", 1, 25)

    var ach_result: GDKResult = await ach_signal
    var board_result: PlayFabResult = await board_signal
    # Both calls were in flight at the same time; this function waits
    # only as long as the slower of the two.
```

Each call's signal is independent, so two awaited signals do not
serialize each other.

## Result objects

Every async call resolves to a normalized result type that carries
the success bit, a payload, and an error description:

| Addon           | Result class    | Success check     | Payload field |
|-----------------|-----------------|-------------------|---------------|
| `godot_gdk`     | `GDKResult`     | `result.ok`       | `result.data` |
| `godot_playfab` | `PlayFabResult` | `result.ok`       | `result.data` |
| `godot_gameinput` (rare async methods) | `GameInputResult` | `result.ok` | `result.data` |

A typical handler looks like:

```gdscript
func _push_progress(percent: int) -> void:
    var result: GDKResult = await GDK.achievements.update_achievement_async(
        Auth.xbox_user, "1", percent)
    if not result.ok:
        push_warning("[Ach] update failed: %s (%s)" % [result.message, result.code])
        return
    print("[Ach] Updated to %d%%" % percent)
```

`result.data` is the typed return value for the operation —
`GDKUser` for sign-in, a `Dictionary` for a PlayFab leaderboard
fetch (`{ rankings: Array, version: int, ... }`),
`PlayFabLobby` for a lobby create / join, and so on. The doc_classes
XML page for the service describes the exact `data` shape per
method (press **F1** on the service class name in the Godot editor
to read it).

When `result.ok` is `false`, `result.message` is a short
human-readable description and `result.code` is a stable string id
you can branch on:

```gdscript
match result.code:
    "no_default_user":
        # Expected on a clean PC — fall through to UI fallback.
        return await _ui_fallback()
    "title_id_required":
        push_error("Set playfab/runtime/title_id in Project Settings.")
    _:
        push_warning("Unhandled: %s" % result.message)
```

## Awaited failures vs. unsolicited errors

There are two distinct error surfaces. Tutorials lean on both:

1. **The awaited `Result`** — your call's failure. This is what you
   get back from the `await`. Use it to drive per-call recovery
   ("the silent sign-in failed, fall back to UI").
2. **Service-level `runtime_error` signals** — failures that
   surface *between* your calls (network dropped mid-frame, a
   background fetch refresh failed, the Achievements Manager bubbled
   a service error during dispatch). Wire these once at startup to
   drive global UI state like "Achievements offline":

   ```gdscript
   func _ready() -> void:
       GDK.achievements.runtime_error.connect(_on_achievements_runtime_error)
       GDK.social.runtime_error.connect(_on_social_runtime_error)

   func _on_achievements_runtime_error(result: GDKResult) -> void:
       push_warning("[Ach] subsystem error: %s" % result.message)
   ```

Service-level runtime signals exist on most GDK services that have
their own native callback path (`GDK.achievements`, `GDK.social`,
`GDK.presence`, `GDK.multiplayer_activity`, …) and on the major
PlayFab services that wrap a background callback queue
(`PlayFab.multiplayer.multiplayer_error`,
`PlayFab.party.party_error`). Press **F1** on a service class in the
editor to see whether it exposes one.

## When to call `dispatch()` manually

Both addons pump async completions automatically each process
frame via the `gdk/runtime/embed_dispatch` and
`playfab/runtime/embed_dispatch` project settings (default `true`).

You only need to call `GDK.dispatch()` or `PlayFab.dispatch()`
yourself when:

- you turned `embed_dispatch` off for deterministic test control
- you want a synchronous pump from outside the engine main loop
  (rare — usually only test scaffolding)
- you are on an older Godot (4.3 / 4.4) where `_process` does not
  fire reliably from autoloads

In normal app code you should never need to call `dispatch()`.

## Common pitfalls

- **Don't `await` inside a `for` loop on a per-frame basis.** Each
  `await` yields back to the engine; a per-frame await chains four
  frames of latency onto a four-element loop for no benefit. Build
  the requests in parallel (see "fan-in helper" above), or batch
  with the service's bulk method when one exists.
- **Don't drive sign-in from `GDK.users.user_changed`.** That signal
  fires for every user lifecycle event (adds, removes, picture
  changes, privilege updates). Use the dedicated `_async` entry
  point in `Auth.sign_in()` (see [Tutorial 1](tutorials/01-sign-in-user.md))
  — it's idempotent and joins any in-flight attempt instead of
  starting a second one.
- **Don't shut the runtime down inside an `await`.** Call
  `GDK.shutdown()` / `PlayFab.shutdown()` from `_exit_tree` only
  after every in-flight `await` has resolved. Outstanding signals
  fire on shutdown with a failure result; if you `await` something
  that races with shutdown your handler resumes in a partly
  torn-down state.

## See also

- [`gdk/async-system.md`](gdk/async-system.md) — the deep view of the
  GDK side (native runtime, `XAsyncBlock` bridge, `XTaskQueueHandle`
  ownership, per-service `runtime_error` semantics).
- [`gdk/api-reference.md`](gdk/api-reference.md) — the full GDK
  surface, organized by service. Every `_async` method is listed
  with its return signal and `Result.data` shape.
- [`playfab/plugin.md`](playfab/plugin.md) — the PlayFab side,
  including which services expose `runtime_error` style signals.
- Every tutorial under [`tutorials/`](tutorials/README.md) — every
  snippet in the tutorial chain follows the patterns described here.
