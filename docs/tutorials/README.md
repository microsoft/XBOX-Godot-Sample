# Tutorials

Task-oriented walkthroughs for the GodotGDK addons. Each tutorial picks
**one concrete thing you want to ship** (a working sign-in flow, an
unlocked achievement, a posted leaderboard entry, a synced cloud save, a
joinable lobby, a controller mapped through GameInput) and walks from a
fresh project to a running result.

If you have not enabled the addons yet, start with
[Addons getting started](../addon-getting-started.md). It covers
copying the addons into a project, setting the PlayFab title id,
creating the `MicrosoftGame.config`, switching the Xbox sandbox, and
signing a user into both Xbox Live and PlayFab. Every tutorial below
assumes you have completed that quickstart.

For a deeper repo-wide guide (building from source, samples, full API
surface), see [Getting Started](../getting-started.md) and the
[documentation index](../README.md).

## How the tutorials are structured

Each tutorial follows the same shape so they read as a series:

1. **What you'll build** — one paragraph plus the log output or
   behavior you'll see when you finish.
2. **Prerequisites** — addon-specific setup the quickstart did not
   cover (an achievement declared in Partner Center, a leaderboard
   stat configured, a PlayFab matchmaking queue, etc.).
3. **Numbered steps** — small, copy-pasteable GDScript blocks with
   inline explanation. Snippets are typed and assume the standard
   `extends Node` host.
4. **Verify** — what the editor Output panel should show on success,
   plus the most common failure modes and how to recognize them.
5. **What's next** — a forward link to the next tutorial in the
   series or to a related reference doc.

GDScript snippets reference real, declared APIs. When a snippet shows
a service like `GDK.achievements` or `PlayFab.game_saves`, you can
press **F1** on the class name in the Godot editor for the full
reference page.

## Recommended learning path

The tutorials are independent — you can jump straight to the one that
matches the feature you need — but a sensible learning order is:

1. **[Sign in a user](01-sign-in-user.md)** — Xbox + PlayFab sign-in
   with a silent → UI fallback. Every other tutorial assumes you can
   reach a signed-in `GDKUser` and `PlayFabUser`.
2. **[Unlock an achievement](02-unlock-achievement.md)** — your first
   end-to-end Xbox Services round-trip; introduces awaiting async
   completion and reading `GDKResult`.
3. **[Post and query a leaderboard](03-gdk-leaderboard.md)** —
   submitting a title-managed stat with `GDK.stats` and reading it
   back through `GDK.leaderboards`.
4. **[Save the player's progress](04-game-saves.md)** — PlayFab Game
   Saves on top of the Xbox-backed PlayFab session.
5. **[Create and join a lobby](05-multiplayer-lobby.md)** — PlayFab
   Multiplayer lobbies, the foundation for any multiplayer feature
   built on this addon set.
6. **[Map a controller through GameInput](06-gameinput-action-bridge.md)** —
   GameInput → Godot `InputMap` via `GameInputActionMap` so the rest
   of your game keeps using `Input.is_action_pressed("jump")`.

## Tutorials in this set

| # | Tutorial | Addon | Approx. time |
|---|----------|-------|--------------|
| 1 | [Sign in a user](01-sign-in-user.md) | `godot_gdk` + `godot_playfab` | 15 min |
| 2 | [Unlock an achievement](02-unlock-achievement.md) | `godot_gdk` | 20 min |
| 3 | [Post and query a leaderboard](03-gdk-leaderboard.md) | `godot_gdk` | 20 min |
| 4 | [Save the player's progress](04-game-saves.md) | `godot_playfab` | 25 min |
| 5 | [Create and join a lobby](05-multiplayer-lobby.md) | `godot_playfab` | 30 min |
| 6 | [Map a controller through GameInput](06-gameinput-action-bridge.md) | `godot_gameinput` | 20 min |

## Coming next

Future docs passes will round out the tutorial set. These are tracked
here so the next session has a single source of truth:

**Xbox Services (`godot_gdk`):**

- Track and update statistics — `GDK.stats` deeper coverage (tracked
  stats, the `stats_updated` signal, query batching)
- Set rich presence — `GDK.presence` end-to-end
- Look up friends and presence — `GDK.social` filter / group flows
- Multiplayer activity — `GDK.multiplayer_activity` activity setup
- Store commerce — `GDK.store` queries and purchases
- Capture and screenshots — `GDK.capture` metadata
- Title Storage — `GDK.title_storage` blobs

**PlayFab (`godot_playfab`):**

- PlayFab Leaderboards — score submission and friends-only queries
- Matchmaking — `PlayFab.multiplayer.create_match_ticket_async` end-to-end
- PlayFab Party — voice + data networks
- Events and telemetry — `PlayFab.events`
- Player data — `PlayFab.player_data`
- CloudScript — `PlayFab.cloud_script` Azure Functions integration

**GameInput (`godot_gameinput`):**

- Direct polling — bypassing the action bridge for per-frame raw reads
- Vibration / rumble — `GameInput.set_vibration` patterns
- Hot-plug — handling `device_connected` and `device_disconnected`
  in a split-screen flow

**Packaging (`godot_gdk_packaging`):**

- Authoring `MicrosoftGame.config` from scratch
- Switching the Xbox sandbox
- Building loose and MSIXVC packages from the editor
- Managing installed packages with the **Package Manager** dialog

If you need one of the tutorials above before a future docs pass
lands, the corresponding `doc_classes/*.xml` for the relevant service
already documents every method and signal end-to-end — press **F1**
on the class name in the Godot editor.
