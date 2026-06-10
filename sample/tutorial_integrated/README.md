# Tutorial Integrated — Xbox + PlayFab sample

This Godot 4.x project is the reference implementation for the [integrated tutorial track](../../docs/tutorials/README.md#integrated-track). It signs into Xbox through GDK, links that identity into PlayFab, and demonstrates the combined services in a capstone scene.

## Quick start

1. Build the addons from the repository root:

   ```powershell
   cmake --preset default
   cmake --build build --preset debug
   ```

2. Fill in Partner Center and PlayFab values:

   ```powershell
   pwsh -File .\tools\setup_sample.ps1
   ```

   The script writes `sample_config.cfg` and `MicrosoftGame.config` into this directory from your inputs. You can also copy the `*.template` files manually and edit them.

3. Open `sample/tutorial_integrated/project.godot` in Godot and run the picker.

## Producing a packaged build

The sample includes a committed `export_presets.cfg` so a clean clone has editable `Windows Desktop` and `XBOX on PC` presets under **Project → Export**.

- Install the Godot 4.6.1 export templates under `%APPDATA%\Godot\export_templates\` before exporting either preset.
- The `XBOX on PC` preset also needs the public GDK toolchain on `PATH` and a `MicrosoftGame.config` authored through the packaging addon's **Create Game Config** verb.
- User-specific identity stays local: EKB path, signing identity, sandbox ID, Partner Center title ID, SCID, and PlayFab title id are intentionally not committed.

## Scenes

| # | Scene | Tutorial |
|---|-------|----------|
| 1 | `i01_signin.tscn` | [Integrated 1 — Xbox to PlayFab sign-in](../../docs/tutorials/integrated/01-signin.md) |
| 2 | `i02_integration/i02_integration.tscn` | [Integrated 2 — Integration tech demo](../../docs/tutorials/integrated/02-tech-demo.md) |

`shared/tutorial_picker.tscn` is the default scene.

## Autoloads

- `Auth` (`autoload/auth.gd`) — Xbox sign-in followed by `PlayFab.users.sign_in_with_xuser_async`; exposes `xbox_user` and `playfab_user`.
- `Lobby` (`autoload/lobby.gd`) — integrated PlayFab lobby plus Xbox Multiplayer Activity, social, presence, and multiplayer privilege gates.
- `Party` (`autoload/party.gd`) — PlayFab Party network layered on the active lobby with Xbox communications and per-peer permission gates.

## Files generated locally (git-ignored)

- `MicrosoftGame.config`
- `sample_config.cfg`
- `addons/godot_*/`
- `.godot/`
- `Build/`

## See also

- [Tutorials index](../../docs/tutorials/README.md)
- [Getting started](../../docs/getting-started.md)
- [Async patterns](../../docs/async-patterns.md)
- [Troubleshooting](../../docs/troubleshooting.md)
- GDK-only sample: [`sample/tutorial_gdk/`](../tutorial_gdk/README.md)
- PlayFab-only sample: [`sample/tutorial_playfab/`](../tutorial_playfab/README.md)
- Standalone GameInput sample: [`sample/tutorial_gameinput/`](../tutorial_gameinput/README.md)
