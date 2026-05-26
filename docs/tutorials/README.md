# Tutorials

Task-oriented walkthroughs for the GodotGDK addons. The tutorial
chain takes you from a fresh project to a single integrated tech
demo — sign-in, achievements, leaderboards, game saves, lobby,
multiplayer activity, Party chat — with each step landing one
addon surface. A separate standalone track covers GameInput.

If you have not enabled the addons yet, start with
[Addons getting started](../addon-getting-started.md). It covers
copying the addons into a project, setting the PlayFab title id,
creating the `MicrosoftGame.config`, switching the Xbox sandbox, and
signing a user into both Xbox Live and PlayFab. Every tutorial below
assumes you have completed that quickstart.

For a deeper repo-wide guide (building from source, samples, full API
surface), see [Getting Started](../getting-started.md) and the
[documentation index](../README.md). For the one-page async-pattern
primer every tutorial assumes, read
[Async patterns](../async-patterns.md).

When something goes wrong while you follow the tutorials, the
[Troubleshooting](../troubleshooting.md) page collects the failures
we see most often (DLL load 126, SCID mismatch, sandbox mismatch,
schema errors in `MicrosoftGame.config`).

## How the tutorials are structured

Each tutorial follows the same shape so they read as a series:

1. **What you'll build** — one paragraph plus the log output or
   behavior you'll see when you finish.
2. **Prerequisites** — addon-specific setup the quickstart did not
   cover (an achievement declared in Partner Center, a leaderboard
   stat configured, a PlayFab matchmaking queue, etc.).
3. **Relevant addon surfaces** — the `GDK.<ns>` / `PlayFab.<ns>`
   classes you touch in this tutorial, each linked to its
   `doc_classes/*.xml` page (the same page **F1** opens in the
   Godot editor).
4. **Numbered steps** — small, copy-pasteable GDScript blocks with
   inline explanation. Snippets are complete files or complete
   functions you can paste verbatim.
5. **Verify** — what the editor Output panel should show on success.
6. **Common failures** — frequent slip-ups, the error surface
   you'll see, and the fix.
7. **Next** — a one-sentence pointer to the next tutorial in the
   chain (or "you're done" for the standalone track).

## Main cumulative track

Each tutorial builds on the previous one. The same `Auth` autoload
introduced in T1 is reused all the way through; the `Lobby` autoload
introduced in T5 grows MPA wiring in T6 and a Party companion in T7.
Tutorial 8 is the capstone — one Control scene with one panel per
surface, all running against one signed-in identity.

| # | Tutorial | Addon | Approx. time |
|---|----------|-------|--------------|
| 1 | [Sign in a user](01-sign-in-user.md) | `godot_gdk` + `godot_playfab` | 15 min |
| 2 | [Unlock an achievement](02-unlock-achievement.md) | `godot_gdk` | 20 min |
| 3 | [Post and query a PlayFab leaderboard](03-playfab-leaderboard.md) | `godot_playfab` | 20 min |
| 4 | [Save the player's progress](04-game-saves.md) | `godot_playfab` | 25 min |
| 5 | [Create and join a lobby](05-multiplayer-lobby.md) | `godot_playfab` | 30 min |
| 6 | [Advertise your lobby with Multiplayer Activity](06-multiplayer-activity.md) | `godot_gdk` | 25 min |
| 7 | [Stand up a PlayFab Party network](07-playfab-party.md) | `godot_playfab` | 30 min |
| 8 | [Integration tech demo (capstone)](08-integration-tech-demo.md) | all of the above | 45 min |

## Standalone tracks

These tutorials are independent of the main cumulative chain. You
can read them at any time — they do not depend on the `Auth` or
`Lobby` autoloads.

| Tutorial | Addon | Approx. time |
|----------|-------|--------------|
| [GameInput action bridge](gameinput-action-bridge.md) | `godot_gameinput` | 20 min |

## Recommended reading order

- **End-to-end multiplayer game**: T1 → T2 → T3 → T4 → T5 → T6 → T7 → T8.
  This is the design intent of the chain. Each surface lands once
  and is reused in the capstone. Drop the standalone GameInput
  tutorial wherever it fits your input-handling story.
- **Sign-in + identity only**: T1. Stop after T1 if all you need
  is "let the user sign into Xbox Live and PlayFab from my game".
- **Single-player progression**: T1 → T2 → T3 → T4. Stop after T4 if
  you do not need any multiplayer surface.
- **Multiplayer without voice**: T1 → T5 → T6. Stop after T6 if you
  want shell-level discovery (Game Bar invites) but not real-time
  audio.
- **Multiplayer with voice + RPC**: T1 → T5 → T6 → T7. Add T7 when
  you need a peer transport for game traffic and / or voice chat.
- **Controller support, any title**: GameInput action bridge.
  Independent of every other tutorial.

GDScript snippets reference real, declared APIs. When a snippet shows
a service like `GDK.achievements` or `PlayFab.game_saves`, you can
press **F1** on the class name in the Godot editor for the full
reference page. The repository's `addons/<addon>/doc_classes/*.xml`
files are the source of truth for those reference pages.
