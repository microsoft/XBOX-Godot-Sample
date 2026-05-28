# Tutorial App — integrated sample

This Godot 4.x project is the **cumulative end-state** of the
[main tutorial chain](../../docs/tutorials/README.md) (T1 → T8).
Every tutorial introduces one surface and a matching scene; this
sample holds the finished version of each. Use it as a reference
when your own project drifts from the tutorial — open the matching
`tNN_<topic>.tscn` and compare.

## Quick start

1. **Build the addons.** Sample `addons/` mirror directories are
   populated by CMake — they are not tracked in git. From the repo
   root:

   ```powershell
   cmake --preset default
   cmake --build build --preset debug
   ```

2. **Fill in your Partner Center / PlayFab values.** From the repo
   root:

   ```powershell
   pwsh -File .\tools\setup_sample.ps1
   ```

   The script writes `sample_config.cfg` and `MicrosoftGame.config`
   into this directory from your inputs. If you prefer, copy the
   `*.template` files manually and edit them.

3. **Open the project in Godot.** The default scene is the
   tutorial picker — pick a tutorial and the matching scene loads.

## Scenes

| # | Scene | Tutorial |
|---|-------|----------|
| 1 | `t01_signin.tscn` | [T1 — Sign in](../../docs/tutorials/01-sign-in-user.md) |
| 2 | `t02_achievement.tscn` | [T2 — Unlock achievement](../../docs/tutorials/02-unlock-achievement.md) |
| 3 | `t03_leaderboard.tscn` | [T3 — PlayFab leaderboard](../../docs/tutorials/03-playfab-leaderboard.md) |
| 4 | `t04_game_saves.tscn` | [T4 — Game Saves](../../docs/tutorials/04-game-saves.md) |
| 5 | `t05_lobby.tscn` | [T5 — Multiplayer lobby](../../docs/tutorials/05-multiplayer-lobby.md) |
| 6 | `t06_mpa.tscn` | [T6 — Multiplayer Activity](../../docs/tutorials/06-multiplayer-activity.md) |
| 7 | `t07_party.tscn` | [T7 — PlayFab Party](../../docs/tutorials/07-playfab-party.md) |
| 8 | `t08_integration/t08_integration.tscn` | [T8 — Integration tech demo](../../docs/tutorials/08-integration-tech-demo.md) |

`shared/tutorial_picker.tscn` is the default scene — a plain
Control with one button per tutorial that calls
`get_tree().change_scene_to_file()`.

## Autoloads

Three project-level autoloads carry the cumulative state across
scenes (matching the tutorial prose):

- `Auth` (`autoload/auth.gd`) — introduced in [T1](../../docs/tutorials/01-sign-in-user.md);
  used by every scene that needs `Auth.xbox_user` / `Auth.playfab_user`.
- `Lobby` (`autoload/lobby.gd`) — introduced in [T5](../../docs/tutorials/05-multiplayer-lobby.md);
  extended in [T6](../../docs/tutorials/06-multiplayer-activity.md) with
  Multiplayer Activity wiring and presence; used by T5–T8.
- `Party` (`autoload/party.gd`) — introduced in [T7](../../docs/tutorials/07-playfab-party.md);
  hosts/joins a PlayFab Party network on top of the active lobby; used
  by T7 and T8.

A tutorial reader doing the chain in their own project adds these
autoloads as they reach each tutorial. The sample registers all three
from the start so any scene in the picker runs out of the box.

## Files generated locally (git-ignored)

- `MicrosoftGame.config` — generated from `MicrosoftGame.config.template`
- `sample_config.cfg` — generated from `sample_config.cfg.template`
- `addons/godot_*/` — mirrored from `addons/<name>/` by CMake build
- `.godot/` — Godot's import cache
- `Build/` — Godot export output

## See also

- [Tutorials index](../../docs/tutorials/README.md)
- [Getting started](../../docs/getting-started.md)
- [Async patterns](../../docs/async-patterns.md)
- [Troubleshooting](../../docs/troubleshooting.md)
- Standalone GameInput sample: [`sample/tutorial_gameinput/`](../tutorial_gameinput/README.md)
