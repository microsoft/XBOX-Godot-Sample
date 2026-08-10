# Tutorial GDK (C#) — Xbox services sample

This Godot 4.x **C#/.NET** project mirrors the GDScript
[`sample/tutorial_gdk`](../tutorial_gdk/README.md) reference, scene-for-scene,
using the `godot_gdk_csharp` facade instead of the bare GDExtension. It uses
`godot_gdk` runtime surfaces and does not require PlayFab — the same modularity
the GDScript samples demonstrate, expressed in C#.

## Quick start

1. Build the addons and the C# facades from the repository root:

   ```powershell
   cmake --preset default
   cmake --build build --preset debug
   ```

2. Create `MicrosoftGame.config` for this sample. Run the setup helper or copy/edit the template manually:

   ```powershell
   pwsh -File .\tools\setup_sample.ps1
   ```

3. Open `sample/tutorial_gdk_csharp/project.godot` in Godot (with the .NET / Mono
   build) and run the picker. The first build restores the
   `GodotGdkCSharp` project reference.

## Scenes

| # | Scene | C# script | Mirrors |
|---|-------|-----------|---------|
| 1 | `g01_signin.tscn` | `G01Signin.cs` | `tutorial_gdk/g01_signin` |
| 2 | `g02_achievement.tscn` | `G02Achievement.cs` | `tutorial_gdk/g02_achievement` |
| 3 | `g03_storage_stats.tscn` | `G03StorageStats.cs` | `tutorial_gdk/g03_storage_stats` |
| 4 | `g04_mpa.tscn` | `G04Mpa.cs` | `tutorial_gdk/g04_mpa` |
| 5 | `g05_speech.tscn` | `G05Speech.cs` | `tutorial_gdk/g05_speech` |

`Shared/tutorial_picker.tscn` is the default scene.

## Autoloads

- `XboxBootstrap` (`Autoload/XboxBootstrap.cs`) — `GodotXbox.Runtime.XboxRuntime`; initializes the GDK runtime and pumps dispatch.
- `XboxAuth` (`Autoload/XboxAuth.cs`) — Xbox-only sign-in; exposes `XboxUser`, `SignInAsync()`, state helpers, and `StateChanged`.

## Public API used (C# facade)

- `Xbox.Users` — `GetPrimaryUser`, `AddDefaultUserAsync`, `AddUserWithUiAsync`
- `Xbox.Achievements` — `QueryPlayerAchievementsAsync`, `GetCachedAchievements`, `UpdateAchievementAsync`, `AchievementUnlocked`
- `Xbox.TitleStorage` — `ListBlobMetadataAsync`, `DownloadBlobAsync`
- `Xbox.Stats` — `TrackStats`, `SetStatInteger`, `FlushStatsAsync`, `QueryUserStatsAsync`, `StatChanged`
- `Xbox.Social` — `StartSocialGraph`, `GetFriendsAsync`, `GetGroupUsers`, `DestroySocialGroup`, `StopSocialGraph`
- `Xbox.Privacy` — `BatchCheckPermissionAsync`
- `Xbox.MultiplayerActivity` — `SetActivityAsync`, `SendInvitesAsync`, `ShowInviteUiAsync`, `GetActivitiesAsync`, `GetCachedActivity`
- `Xbox.Presence` — `TrackPresence`, `GetPresenceAsync`, `StopTrackingPresence`
- `Xbox.Speech` — `GetInstalledVoices`, `SetDefaultVoice`, `SynthesizeToStream`, `SynthesizeSsml`

## Configuration

- Requires `MicrosoftGame.config` with title id, SCID, sandbox, and service configuration.
- Does not require a PlayFab title id.

## Files generated locally (git-ignored)

- `MicrosoftGame.config`
- `sample_config.cfg`
- `addons/godot_*/`
- `.godot/`
- `Build/`
- `.mono/` / `obj/` / `bin/`

## See also

- GDScript counterpart: [`sample/tutorial_gdk/`](../tutorial_gdk/README.md)
- [C# facade reference](../../docs/gdk/csharp.md)
- [Tutorials index](../../docs/tutorials/README.md)
