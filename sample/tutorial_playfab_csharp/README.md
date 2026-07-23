# Tutorial PlayFab (C#) — PlayFab services sample

This Godot 4.x **C#/.NET** project mirrors the GDScript
[`sample/tutorial_playfab`](../tutorial_playfab/README.md) reference,
scene-for-scene, using the `godot_playfab_csharp` facade. It uses
`godot_playfab` only: no Xbox sign-in and no `MicrosoftGame.config` are
required for the PlayFab-only scenes.

## Quick start

1. Build the addons and the C# facades from the repository root:

   ```powershell
   cmake --preset default
   cmake --build build --preset debug
   ```

2. Set the PlayFab title id for this project (`playfab/runtime/title_id`) or
   fill in the sample config template.
3. Open `sample/tutorial_playfab_csharp/project.godot` in Godot (with the
   .NET / Mono build) and run the picker. The first build restores the
   `GodotGdkCSharp` and `GodotPlayFabCSharp` project references.

## Scenes

| # | Scene | C# script | Mirrors |
|---|-------|-----------|---------|
| 1 | `p01_signin.tscn` | `P01Signin.cs` | `tutorial_playfab/p01_signin` |
| 2 | `p02_leaderboard.tscn` | `P02Leaderboard.cs` | `tutorial_playfab/p02_leaderboard` |
| 3 | `p03_lobby.tscn` | `P03Lobby.cs` | `tutorial_playfab/p03_lobby` |
| 4 | `p04_party.tscn` | `P04Party.cs` | `tutorial_playfab/p04_party` |

`Shared/tutorial_picker.tscn` is the default scene.

## Running multiple instances as different users

This track signs in with a PlayFab custom id resolved per instance, so you can
run several local copies — each its own PlayFab user — to exercise the Lobby
(P3) and Party (P4) tutorials without a second machine:

- **Editor:** **Debug → Customize Run Instances…**, enable multiple instances,
  and give each one a distinct argument such as `--pf-user=alice` /
  `--pf-user=bob`.
- **Command line:** `godot --path sample/tutorial_playfab_csharp -- --pf-user=alice`

Resolution order is `--pf-user=<name>`, then the `PF_CUSTOM_ID` environment
variable, then the default token `player`. Each distinct token maps to a
separate PlayFab account (namespaced as `godot-playfab-tutorial-<name>`).

## Autoloads

- `PlayFabBootstrap` (`Autoload/PlayFabBootstrap.cs`) — `GodotPlayFab.Runtime.PlayFabRuntime`; initializes the PlayFab runtime and pumps dispatch.
- `PlayFabAuth` (`Autoload/PlayFabAuth.cs`) — custom-id PlayFab sign-in; exposes `PlayFabUser`, `GetCustomId()`, `SignInAsync()`, state helpers, and `StateChanged`.
- `Lobby` (`Autoload/Lobby.cs`) — PlayFab-native lobby state, search, member/lobby properties, and received invites.
- `Party` (`Autoload/Party.cs`) — PlayFab Party network layered on the active lobby; enables text and voice for all peers in this track.

## Public API used (C# facade)

- `PlayFab.Users` — `SignInWithCustomIdAsync`, `GetUserByCustomId`
- `PlayFab.Statistics` — `UpdateStatisticsAsync`
- `PlayFab.Leaderboards` — `GetLeaderboardAsync`, `GetLeaderboardAroundUserAsync`, `GetFriendLeaderboardAsync`
- `PlayFab.Multiplayer` — `InitializeAsync`, `CreateLobbyAsync`, `JoinLobbyAsync`, `FindLobbiesAsync`
- `PlayFabLobby` — `SetMemberPropertiesAsync`, `SetPropertiesAsync`, `LeaveAsync`, `StateChanged`
- `PlayFab.Party` — `InitializeAsync`, `CreateAndJoinNetworkAsync`, `JoinNetworkAsync`, `GetChat`
- `PlayFabPartyChat` — `CreateLocalChatControlAsync`, `SendTextAsync`, `SetAudioMutedAsync`, `SetChatPermissionsAsync`

## Configuration

- Requires a PlayFab title id.
- Does not require `MicrosoftGame.config`.
- Game Saves is **not** part of this track: PlayFab Game Saves requires an
  Xbox-backed PlayFab user, which the custom-id PlayFab-only session cannot
  provide. See the integrated sample for a working Game Saves run.

## Files generated locally (git-ignored)

- `sample_config.cfg`
- `addons/godot_*/`
- `.godot/`
- `Build/`
- `.mono/` / `obj/` / `bin/`

## See also

- GDScript counterpart: [`sample/tutorial_playfab/`](../tutorial_playfab/README.md)
- [Tutorials index](../../docs/tutorials/README.md)
- [PlayFab prerequisites](../../docs/playfab/prerequisites.md)
- [Troubleshooting](../../docs/troubleshooting.md)

