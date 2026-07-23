# Tutorial Integrated (C#) — Xbox + PlayFab sample

This Godot 4.x **C#/.NET** project mirrors the GDScript
[`sample/tutorial_integrated`](../tutorial_integrated/README.md) reference,
scene-for-scene, using the `godot_gdk_csharp` and `godot_playfab_csharp`
facades. It signs into Xbox through GDK, links that identity into PlayFab,
and demonstrates the combined services in a capstone scene.

## Quick start

1. Build the addons and C# facades from the repository root:

   ```powershell
   cmake --preset default
   cmake --build build --preset debug
   ```

2. Fill in Partner Center and PlayFab values:

   ```powershell
   pwsh -File .\tools\setup_sample.ps1
   ```

3. Open `sample/tutorial_integrated_csharp/project.godot` in Godot (with the
   .NET / Mono build) and run the picker. The first build restores the
   `GodotGdkCSharp` and `GodotPlayFabCSharp` project references.

## Scenes

| # | Scene | C# script | Mirrors |
|---|-------|-----------|---------|
| 1 | `i01_signin.tscn` | `I01Signin.cs` | `tutorial_integrated/i01_signin` |
| 2 | `I02Integration/i02_integration.tscn` | `I02Integration/I02Integration.cs` | `tutorial_integrated/i02_integration` |

`Shared/tutorial_picker.tscn` is the default scene.

## I2 integration tabs

Achievements, Leaderboard, Game Saves, Lobby, MPA, Party, and Game Chat. The
Game Chat tab uses GDK-native voice/text with in-process loopback frames; the
Party tab uses PlayFab Party's managed transport.

## Autoloads

- `GDKBootstrap` (`Autoload/GdkBootstrap.cs`) — `GodotGdk.Runtime.GdkRuntime`.
- `PlayFabBootstrap` (`Autoload/PlayFabBootstrap.cs`) — `GodotPlayFab.Runtime.PlayFabRuntime`.
- `Auth` (`Autoload/Auth.cs`) — Xbox sign-in followed by `PlayFab.Users.SignInWithXUserAsync`; exposes `XboxUser` and `PlayFabUser`.
- `Lobby` (`Autoload/Lobby.cs`) — PlayFab Lobby plus Xbox Multiplayer Activity, social, presence, and multiplayer privilege gates.
- `Party` (`Autoload/Party.cs`) — PlayFab Party network layered on the active lobby with Xbox communications gates.

## Public API used (C# facades)

- `Gdk.Users`, `Gdk.Achievements`, `Gdk.Privacy`, `Gdk.Social`, `Gdk.MultiplayerActivity`, `Gdk.Presence`, `Gdk.GameChat`
- `PlayFab.Users`, `PlayFab.GameSaves`, `PlayFab.Leaderboards`, `PlayFab.Statistics`, `PlayFab.Multiplayer`, `PlayFab.Party`

## Files generated locally (git-ignored)

- `MicrosoftGame.config`
- `sample_config.cfg`
- `addons/godot_*/`
- `.godot/`
- `Build/`
- `.mono/` / `obj/` / `bin/`

## See also

- GDScript counterpart: [`sample/tutorial_integrated/`](../tutorial_integrated/README.md)
- [C# facade reference](../../docs/gdk/csharp.md)
- [Tutorials index](../../docs/tutorials/README.md)
