# Using the PlayFab addon from C# (`godot_playfab_csharp`)

`godot_playfab_csharp` is the managed facade over the native `godot_playfab`
GDExtension, following the exact same pattern as
[`../gdk/csharp.md`](../gdk/csharp.md): one static `PlayFab` entry point, typed
service namespaces, `Task<PlayFabResult>` async, and typed value wrappers. The
native DLL is unchanged.

## Setup

1. Build the native addon and configure `playfab/runtime/title_id` in Project
   Settings.
2. Reference both the PlayFab **and** GDK facades (PlayFab sign-in takes a GDK
   user):

   ```xml
   <ItemGroup>
     <ProjectReference Include="addons/godot_gdk_csharp/GodotGdkCSharp.csproj" />
     <ProjectReference Include="addons/godot_playfab_csharp/GodotPlayFabCSharp.csproj" />
   </ItemGroup>
   ```

3. (Optional) C# bootstrap autoload:

   ```csharp
   // Autoload/PlayFabBootstrap.cs
   public partial class PlayFabBootstrap : GodotPlayFab.Runtime.PlayFabRuntime { }
   ```

## Async + results

```csharp
using GodotPlayFab;
using GodotPlayFab.Types;

PlayFabResult init = PlayFab.Initialize();   // uses playfab/runtime/title_id
PlayFabResult signIn = await PlayFab.Users.SignInWithXUserAsync(xboxUser);
if (!signIn.Ok)
{
    GD.PushWarning($"PlayFab sign-in failed: {signIn.Message} ({signIn.Code})");
    return;
}

PlayFabUser pf = signIn.DataAs<PlayFabUser>();
Godot.Collections.Dictionary key = pf.EntityKey;   // { "type": ..., "id": ... }
```

`PlayFabResult` has the same shape as `GdkResult` (`Ok`, `Code`, `Message`,
`Data`, `DataObject`, `DataAs<T>()`).

## Cross-addon sign-in (GDK user → PlayFab)

`PlayFab.Users.SignInWithXUserAsync` takes a **`GodotGdk.Types.GdkUser`** and
passes its underlying `GodotObject` through unchanged. The boundary is
deliberately duck-typed (`Object` on the native side) because `godot_gdk` and
`godot_playfab` are **separate GDExtension DLLs** and `Ref<>` types cannot cross
that boundary. Never marshal a raw local Xbox user id across — always pass the
`GdkUser`:

```csharp
GdkUser xbox = Gdk.Users.GetPrimaryUser()
    ?? (await Gdk.Users.AddUserWithUiAsync()).DataAs<GdkUser>();

PlayFabResult result = await PlayFab.Users.SignInWithXUserAsync(xbox);
```

## Services

`PlayFab` exposes all 18 native service namespaces as typed accessors:
`PlayFab.Users`, `PlayFab.GameSaves`, `PlayFab.Leaderboards`,
`PlayFab.Multiplayer`, `PlayFab.Party`, `PlayFab.Accounts`, `PlayFab.Catalog`,
`PlayFab.CloudScript`, `PlayFab.EntityData`, `PlayFab.Events`,
`PlayFab.Experimentation`, `PlayFab.Friends`, `PlayFab.Groups`,
`PlayFab.Inventory`, `PlayFab.Localization`, `PlayFab.PlayerData`,
`PlayFab.Statistics`, and `PlayFab.TitleData`.

```csharp
PlayFabResult saved = await PlayFab.GameSaves.UploadAsync(pf, "slot0", bytes);
PlayFabResult board = await PlayFab.Leaderboards.GetLeaderboardAsync("score", 0, 10);
```

### Lobby, Matchmaking, and Party

`PlayFab.Multiplayer` covers lobby + matchmaking flows and `PlayFab.Party` covers
low-latency network/chat. Their background callback-queue signals
(`multiplayer_error`, `party_error`, and the lobby/network/chat state-change
signals) are exposed as C# `event Action<...>` members on those services. Party's
Godot-RPC-over-network path returns a `MultiplayerPeer` you can assign to
`SceneTree.GetMultiplayer().MultiplayerPeer`.

## Parity guarantee

Covered by `tests/csharp/FacadeParity.Tests` (run via
`tools/run_csharp_tests.ps1`) — every native `doc_classes` member is asserted to
have a managed wrapper.
