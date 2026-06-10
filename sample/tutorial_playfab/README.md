# Tutorial PlayFab — PlayFab services sample

This Godot 4.x project is the reference implementation for the [PlayFab tutorial track](../../docs/tutorials/README.md#playfab-track). It uses `godot_playfab` only: no Xbox sign-in and no `MicrosoftGame.config` are required for the PlayFab-only scenes.

## Quick start

1. Build the addons from the repository root:

   ```powershell
   cmake --preset default
   cmake --build build --preset debug
   ```

2. Set the PlayFab title id for this project (`playfab/runtime/title_id`) or fill in the sample config template.
3. Open `sample/tutorial_playfab/project.godot` in Godot and run the picker.

## Scenes

| # | Scene | Tutorial |
|---|-------|----------|
| 1 | `p01_signin.tscn` | [PlayFab 1 — Custom-id sign-in](../../docs/tutorials/playfab/01-signin.md) |
| 2 | `p02_leaderboard.tscn` | [PlayFab 2 — Leaderboard](../../docs/tutorials/playfab/02-leaderboard.md) |
| 3 | `p03_lobby.tscn` | [PlayFab 3 — Lobby](../../docs/tutorials/playfab/03-lobby.md) |
| 4 | `p04_party.tscn` | [PlayFab 4 — Party](../../docs/tutorials/playfab/04-party.md) |

`shared/tutorial_picker.tscn` is the default scene.

## Autoloads

- `PlayFabAuth` (`autoload/playfab_auth.gd`) — custom-id PlayFab sign-in; exposes `playfab_user`.
- `Lobby` (`autoload/lobby.gd`) — PlayFab-native lobby state, search, member/lobby properties, and received invites.
- `Party` (`autoload/party.gd`) — PlayFab Party network layered on the active lobby; enables text and voice for all peers in this track.

## Configuration

- Requires a PlayFab title id.
- Does not require `MicrosoftGame.config`.
- Game Saves is **not** part of this track: PlayFab Game Saves requires an Xbox-backed PlayFab user, which the custom-id PlayFab-only session cannot provide. See the integrated sample for a working Game Saves run.

## Files generated locally (git-ignored)

- `sample_config.cfg`
- `addons/godot_*/`
- `.godot/`
- `Build/`

## See also

- [Tutorials index](../../docs/tutorials/README.md)
- [PlayFab prerequisites](../../docs/playfab/prerequisites.md)
- [Troubleshooting](../../docs/troubleshooting.md)
