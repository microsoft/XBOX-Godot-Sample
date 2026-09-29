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

3. Add a solution file next to `project.godot`. Godot's .NET export plugin
   **requires** `<assembly_name>.sln` in the project root — without it every
   export fails with *"This project contains C# files but no solution file was
   found"*. Include both facade projects so solution builds use the same
   `Debug` / `ExportDebug` / `ExportRelease` configuration. See
   [`../gdk/csharp.md`](../gdk/csharp.md#setup) and the `.sln` files in
   `sample/tutorial_playfab_csharp/` for the expected shape.

4. (Optional) C# bootstrap autoload:

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

`PlayFabResult` has the same shape as `XboxResult` (`Ok`, `Code`, `Message`,
`Data`, `DataObject`, `DataAs<T>()`).

## Cross-addon sign-in (GDK user → PlayFab)

`PlayFab.Users.SignInWithXUserAsync` takes a **`GodotXbox.Types.XboxUser`** and
passes its underlying `GodotObject` through unchanged. The boundary is
deliberately duck-typed (`Object` on the native side) because `godot_gdk` and
`godot_playfab` are **separate GDExtension DLLs** and `Ref<>` types cannot cross
that boundary. Never marshal a raw local Xbox user id across — always pass the
`XboxUser`:

```csharp
XboxUser xbox = Xbox.Users.GetPrimaryUser()
    ?? (await Xbox.Users.AddUserWithUiAsync()).DataAs<XboxUser>();

PlayFabResult result = await PlayFab.Users.SignInWithXUserAsync(xbox);
```

## Token-based sign-in (Steam, OpenID Connect, Battle.net)

Besides the Xbox and custom-id flows, `PlayFab.Users` wraps the token-based
identity providers available on the GDK SDK. The title authenticates the player
with the provider first (its own SDK or OAuth flow) and forwards the resulting
token; the returned `PlayFabUser` is keyed by its PlayFab entity id (`LocalId == 0`,
empty `CustomId`).

```csharp
// Steam — steamTicket is a hex-encoded Steamworks session ticket.
PlayFabResult steam = await PlayFab.Users.SignInWithSteamAsync(steamTicket, create_account: true, ticket_is_service_specific: false);

// OpenID Connect — connectionId names the OIDC connection in PlayFab Game Manager.
PlayFabResult oidc = await PlayFab.Users.SignInWithOpenIdConnectAsync("my-oidc-connection", idToken);

// Battle.net — identityToken is the JWT from the Battle.net OAuth flow.
PlayFabResult battleNet = await PlayFab.Users.SignInWithBattleNetAsync(identityToken);
```

The PlayFab-native credential flows (email/username/register) and other platform
logins are not available in the GDK SDK build; see the sign-in coverage matrix in
`spec/gdext-playfab.md`.

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

### Friend leaderboard sources

The managed leaderboard service preserves the original bool method and exposes
the native source bitfield through the nested
`[Flags] PlayFabLeaderboards.FriendSources : long` enum:

```csharp
using FriendSources = GodotPlayFab.Services.PlayFabLeaderboards.FriendSources;

// Ordinary provider flags can be combined. Xbox requires an Xbox-backed
// PlayFabUser returned by SignInWithXUserAsync.
PlayFabResult steamAndXbox =
    await PlayFab.Leaderboards.GetFriendLeaderboardWithSourcesAsync(
        xboxBackedUser,
        "high_score",
        FriendSources.Steam | FriendSources.Xbox);
```

The values are `None = 0`, `Steam = 1`, `Facebook = 2`, `Xbox = 4`,
`Psn = 8`, and `All = 16`. `All` must be used alone and is treated as
Xbox-containing by the addon. The unchanged
`GetFriendLeaderboardAsync(user, leaderboard_name,
include_xbox_friends: true, version: -1)` remains available; `false` maps to
`None` and `true` maps to `Xbox`.

See the [PlayFab plugin guide](plugin.md#friend-leaderboard-sources) for the
complete source-selection behavior and
[PlayFab prerequisites](prerequisites.md) for provider setup.

The facade does not duplicate validation or token acquisition. Invalid masks,
missing Xbox-backed sessions, provider failures, and other native errors arrive
through the returned `PlayFabResult`, not as managed argument exceptions.

### Lobby, Matchmaking, and Party

`PlayFab.Multiplayer` covers lobby + matchmaking flows and `PlayFab.Party` covers
low-latency network/chat. Their background callback-queue signals
(`multiplayer_error`, `party_error`, and the lobby/network/chat state-change
signals) are exposed as C# `event Action<...>` members on those services. Party's
Godot-RPC-over-network path returns a `MultiplayerPeer` you can assign to
`SceneTree.GetMultiplayer().MultiplayerPeer`.

#### Premade-group matchmaking

`PlayFabMatchmakingTicketConfig.Members` contains local signed-in users.
`MembersToMatchWith` contains remote entity-key dictionaries. Following the
read-only facade convention, assign the latter through `Raw.Set`:

PlayFab rejects a ticket whose members already fill the queue's `MaxMatchSize`,
so a premade group of N players needs a queue whose `MaxMatchSize` exceeds N.

```csharp
GodotObject raw = ClassDB.Instantiate("PlayFabMatchmakingTicketConfig").AsGodotObject();
raw.Set("queue_name", "squads"); // MaxMatchSize > the premade group's size
raw.Set("timeout_seconds", 120);
raw.Set("members_to_match_with", new Godot.Collections.Array
{
    new Godot.Collections.Dictionary
    {
        ["id"] = friendEntityId,
        ["type"] = "title_player_account",
    },
});
PlayFabMatchmakingTicketConfig config = PlayFabMatchmakingTicketConfig.From(raw);

PlayFabResult created = await PlayFab.Multiplayer.CreateMatchTicketAsync(user, config);
```

The remote member joins with
`JoinMatchTicketAsync(user, ticketId, queueName, localMembers)`;
`localMembers: null` sends an empty Array and therefore auto-includes `user`
with empty attributes. The Task succeeds only after the ticket leaves
`STATUSJOINING` for an accepted status; that acceptance is not a matched result.

The canonical status/event contract is documented on the native
`PlayFabMatchTicket` class. In C#, subscribe to `StateChanged`, inspect
`ticket.Status` immediately, and reconcile it on every event kind.

`PlayFabMatchTicket.CancelAsync()` follows that native completion contract: a
successful cancel has null data, while
`match_ticket_cancel_lost_race` carries the matched ticket. `PlayFabResult.HResult`
is a sign-extended `long`, so compare native hexadecimal HRESULT bits with a
mask:

```csharp
if ((result.HResult & 0xFFFFFFFFL) == 0x89235652L)
{
    GD.PushWarning("The matchmaking ticket group is too large.");
}
```

#### Arranged-lobby initialization

`JoinArrangedLobbyAsync` initializes the lobby it creates from
`PlayFabLobbyJoinConfig`. Following the read-only facade convention, the managed
type exposes `MaxMemberCount`, `AccessPolicy`, `OwnerMigrationPolicy`, and
`RestrictInvitesToLobbyOwner` for reading; callers assign through `Raw.Set`:

```csharp
GodotObject raw = ClassDB.Instantiate("PlayFabLobbyJoinConfig").AsGodotObject();
raw.Set("max_member_count", 4);   // this game mode holds 4
raw.Set("restrict_invites_to_lobby_owner", false);
PlayFabLobbyJoinConfig config = PlayFabLobbyJoinConfig.From(raw);

PlayFabResult joined =
    await PlayFab.Multiplayer.JoinArrangedLobbyAsync(user, connectionString, config);
```

The C# tutorial samples wrap these through optional
`TutorialSupport.LobbyJoinConfig(...)` arguments. The canonical defaults,
presence behavior, compile-time GDK edition gate, and Xbox-activity distinction
are documented on the native `PlayFabLobbyJoinConfig` class.

## Parity guarantee

Covered by `tests/csharp/FacadeParity.Tests` (run via
`tools/run_csharp_tests.ps1`) — every native `doc_classes` member is asserted to
have a managed wrapper. Targeted reflection assertions also lock matchmaking
method/property shapes, both matchmaking value spaces, the legacy and
source-selecting friend method signatures/defaults, the `[Flags]`/`long` enum
shape, and all six friend-source values against the native XML.
