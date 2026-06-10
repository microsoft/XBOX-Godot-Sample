# Tutorial GDK — Xbox services sample

This Godot 4.x project is the reference implementation for the [GDK tutorial track](../../docs/tutorials/README.md#gdk-track). It uses `godot_gdk` runtime surfaces and does not require PlayFab.

## Quick start

1. Build the addons from the repository root:

   ```powershell
   cmake --preset default
   cmake --build build --preset debug
   ```

2. Create `MicrosoftGame.config` for this sample. Run the setup helper or copy/edit the template manually:

   ```powershell
   pwsh -File .\tools\setup_sample.ps1
   ```

3. Open `sample/tutorial_gdk/project.godot` in Godot and run the picker.

## Scenes

| # | Scene | Tutorial |
|---|-------|----------|
| 1 | `g01_signin.tscn` | [GDK 1 — Xbox-only sign-in](../../docs/tutorials/gdk/01-signin.md) |
| 2 | `g02_achievement.tscn` | [GDK 2 — Unlock achievement](../../docs/tutorials/gdk/02-achievement.md) |
| 3 | `g03_storage_stats.tscn` | [GDK 3 — Title Storage and stats](../../docs/tutorials/gdk/03-storage-stats.md) |
| 4 | `g04_mpa.tscn` | [GDK 4 — Multiplayer Activity](../../docs/tutorials/gdk/04-mpa.md) |

`shared/tutorial_picker.tscn` is the default scene.

## Autoloads

- `GdkAuth` (`autoload/gdk_auth.gd`) — Xbox-only sign-in; exposes `xbox_user`, `sign_in()`, state helpers, and `state_changed`.

## Configuration

- Requires `MicrosoftGame.config` with title id, SCID, sandbox, and service configuration.
- Does not require a PlayFab title id.

## Files generated locally (git-ignored)

- `MicrosoftGame.config`
- `sample_config.cfg`
- `addons/godot_*/`
- `.godot/`
- `Build/`

## See also

- [Tutorials index](../../docs/tutorials/README.md)
- [GDK setup](../../docs/gdk/sample-setup.md)
- [Troubleshooting](../../docs/troubleshooting.md)
