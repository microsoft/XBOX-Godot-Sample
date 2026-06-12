# Tutorials

Task-oriented walkthroughs for the XBOX Godot Sample addons. The tutorials are split by altitude so you can choose the smallest sample that matches your game:

- **GDK** — Xbox sign-in, achievements, Title Storage, statistics, Social, presence, and Multiplayer Activity.
- **PlayFab** — custom-id sign-in, leaderboards, lobbies, and Party without Xbox dependencies.
- **Integrated** — Xbox sign-in linked into PlayFab plus a capstone that combines the surfaces.
- **GameInput** — standalone controller/action bridge.

Start with [Addons getting started](../addon-getting-started.md) if you have not copied addons into a project yet. For async result handling, read [Async patterns](../async-patterns.md). For common setup failures, see [Troubleshooting](../troubleshooting.md).

## How the tutorials are structured

Each tutorial follows the same shape: **What you'll build**, **Prerequisites**, **Relevant addon surfaces**, numbered steps, **Verify**, **Common failures**, and **Next**.

## GDK track

| # | Tutorial | Scene | Approx. time |
|---|----------|-------|--------------|
| 1 | [Xbox-only sign-in](gdk/01-signin.md) | `g01_signin.tscn` | 15 min |
| 2 | [Unlock an achievement](gdk/02-achievement.md) | `g02_achievement.tscn` | 20 min |
| 3 | [Title Storage and stats](gdk/03-storage-stats.md) | `g03_storage_stats.tscn` | 25 min |
| 4 | [Multiplayer Activity](gdk/04-mpa.md) | `g04_mpa.tscn` | 25 min |

## PlayFab track

| # | Tutorial | Scene | Approx. time |
|---|----------|-------|--------------|
| 1 | [PlayFab custom-id sign-in](playfab/01-signin.md) | `p01_signin.tscn` | 15 min |
| 2 | [Leaderboard](playfab/02-leaderboard.md) | `p02_leaderboard.tscn` | 20 min |
| 3 | [Lobby](playfab/03-lobby.md) | `p03_lobby.tscn` | 30 min |
| 4 | [Party](playfab/04-party.md) | `p04_party.tscn` | 30 min |

## Integrated track

| # | Tutorial | Scene | Approx. time |
|---|----------|-------|--------------|
| 1 | [Xbox to PlayFab sign-in](integrated/01-signin.md) | `i01_signin.tscn` | 20 min |
| 2 | [Integration tech demo](integrated/02-tech-demo.md) | `i02_integration/i02_integration.tscn` | 45 min |

## Standalone GameInput track

| Tutorial | Scene | Approx. time |
|----------|-------|--------------|
| [GameInput action bridge](gameinput-action-bridge.md) | `sample/tutorial_gameinput/main.tscn` | 20 min |

## Reference samples

If your project drifts from a tutorial, open the matching sample scene and compare.

- [`sample/tutorial_gdk/`](../../sample/tutorial_gdk/README.md) — GDK-only track (`g01` → `g04`).
- [`sample/tutorial_playfab/`](../../sample/tutorial_playfab/README.md) — PlayFab-only track (`p01` → `p05`).
- [`sample/tutorial_integrated/`](../../sample/tutorial_integrated/README.md) — integrated Xbox + PlayFab track (`i01` → `i02`).
- [`sample/tutorial_gameinput/`](../../sample/tutorial_gameinput/README.md) — standalone GameInput sample.

Samples have CMake-mirrored `addons/` folders; run `cmake --build build --preset debug` from the repo root once before opening them in Godot.

## Recommended reading order

- **Xbox services only**: GDK 1 → 2 → 3 → 4.
- **PlayFab services only**: PlayFab 1 → 2 → 3 → 4 → 5.
- **Xbox-linked PlayFab title**: Integrated 1 → 2, then consult the GDK and PlayFab tracks for focused surface walkthroughs.
- **Controller support, any title**: GameInput action bridge.

GDScript snippets reference real, declared APIs. The repository's `addons/<addon>/doc_classes/*.xml` files are the source of truth for reference pages.

## Known Issues

> [!WARNING]
> **Multiple Instances (GDK / Integrated tracks).** Godot's **Debug → Run Multiple Instances** option lets you launch several copies of a project while debugging. This is **not recommended** for the GDK and Integrated tracks, which sign into Xbox services: only one GDK user can be signed in on the PC at a time, so additional instances will fail to sign in and may show failed connections or crashes. To exercise those multiplayer scenarios with more than one player, export the game and run the second copy on a separate computer with a different account.
>
> [!TIP]
> **Multiple Instances (PlayFab track).** The PlayFab-only track has no Xbox single-user constraint, so it *does* support running multiple local instances as different users. Give each instance a distinct `--pf-user=<name>` argument (via **Debug → Customize Run Instances…**, or `-- --pf-user=alice` on the command line) and each signs into its own PlayFab account — enough to put two players in the same lobby or Party on one machine. See [PlayFab Tutorial 1 — Custom-id sign-in](playfab/01-signin.md).
