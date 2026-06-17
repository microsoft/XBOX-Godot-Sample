# Godot .NET (C#) parallel for the XBOX Godot Sample addons — GDExtension Spec

## Overview

This document defines a **C# (Godot .NET) parallel surface** for the XBOX Godot
Sample addons. The goal is to let teams building their title in the **.NET build of
Godot** consume the same Microsoft GDK / PlayFab / GameInput integration that GDScript
callers use today, with idiomatic C# ergonomics (typed members, `Task`-based async,
`event`-based error/notification surfaces).

The defining architectural fact is that **the native runtime is not re-ported**. The
value-bearing code is the per-addon C++ **GDExtension** DLLs, which register engine
singletons (`GDK`, `PlayFab`, `GameInput`) and their wrapper classes into `ClassDB`.
GDExtension is language-agnostic on the consumer side: the **same**
`*.windows.{debug,release}.x86_64.dll` files load in a Godot .NET editor/runtime and
expose the identical surface. The C# track is therefore a thin, **additive** managed
facade layered over the unchanged DLLs — never a second native implementation.

This work is **strictly additive**. It introduces no changes to any C++ DLL, to the
GDScript bootstraps, or to the existing GDScript sample/test/tooling tracks. The
GDScript surface remains the primary, first-class surface; the C# surface is a
parallel consumer of the same runtimes.

### Addon coverage at a glance

| Addon | Native singleton | Registered classes | C# track |
| --- | --- | --- | --- |
| `godot_gdk` | `GDK` (abstract) | 42 (21 services + result + 19 value types) | Service/runtime facade |
| `godot_playfab` | `PlayFab` (abstract) | 46 (18 services + result + 27 value/config/state types) | Service/runtime facade |
| `godot_gameinput` | `GameInput` (concrete) | 6 (singleton + 5 device/reading/mapping types) | **Input-integration facade** (different shape) |
| `godot_gdk_packaging` | none (GDScript editor plugin) | 0 native | **No port** — runs unchanged; optional C# CLI shim |

Per the repo's top-level conventions, `godot_gdk` and `godot_playfab` are treated as
**service/runtime** addons (one root singleton with typed service namespaces beneath
it); `godot_gameinput` is an **input-integration** addon (optimized for Godot's
`Input`/`InputMap` flow, not forced into the same service shape); and
`godot_gdk_packaging` is **editor tooling**, not a runtime service.

## Design goals

1. **Wrap, don't reimplement.** Each C++ GDExtension stays the single source of truth
   for all runtime, async, manager, networking, and result-normalization logic. The
   C# layer only adapts the existing `GodotObject` surface into typed C#.
2. **Idiomatic C# ergonomics.** `Task<TResult>` instead of awaited `Signal`,
   strongly-typed wrapper classes instead of `GodotObject.Call`/`Get`, C# `event`
   wrappers for signal-based surfaces (`runtime_error`, `multiplayer_error`,
   `party_error`, `device_connected`, …).
3. **No DLL changes for C#'s benefit.** Preserve the Signal/`await` contract that the
   GDScript surface and `docs/async-patterns.md` depend on. (See **Rejected
   alternatives**.)
4. **Single shared runtime per addon.** GDScript and C# consume the same DLLs, the
   same singletons, and the same per-frame `dispatch()` ticks. No parallel native
   integration.
5. **Engine-native objects.** C# wrappers hold and surface real Godot objects
   (`GDKUser`, `PlayFabLobby`, `GameInputDevice`, `Signal`, `Resource`-derived action
   maps) so C# scenes, UI, autoloads, and the Godot multiplayer stack compose for free.
6. **Respect each addon's shape.** Service/runtime addons get a singleton-rooted typed
   facade; the input addon gets a thin facade aligned to `Input`/`InputMap`; the
   packaging tooling is left as GDScript.
7. **Graceful absence.** When a DLL is not loaded (editor without the extension,
   non-target runtime), the C# facade degrades the same way the GDScript bootstraps
   do — no crashes.

## Scope

| Domain | In scope | Notes |
| --- | --- | --- |
| C# facade — `godot_gdk` | Yes | All 21 services + result + value types |
| C# facade — `godot_playfab` | Yes | All 18 services + result + Multiplayer/Party state-change & networking |
| C# facade — `godot_gameinput` | Yes | Input-integration shape (sync polling, signals, action-map Resources, InputMap bridge) |
| Signal → `Task` async bridge | Yes | Shared by `godot_gdk` + `godot_playfab` (`gameinput` is synchronous) |
| C# runtime autoloads | Yes | C# equivalents of each addon's GDScript bootstrap |
| Cross-addon sign-in (GDK user → PlayFab) | Yes | Duck-typed `GodotObject` pass-through; no `Ref<>` across DLLs |
| PlayFab Party networking / Godot RPC bridge | Yes | C# must compose with Godot's high-level multiplayer (`MultiplayerApi`) |
| C# sample track | Yes | Parallel C# ports of `sample/tutorial_app` and `sample/tutorial_gameinput` |
| C# test hosts | Yes | In-engine C# tests (GoDotTest) per coverage host |
| C# docs | Yes | `docs/*/csharp.md` usage + parity notes |
| `godot_gdk_packaging` C# port | **No** | GDScript editor plugin runs unchanged in a .NET project; optional thin C# wrapper over the headless CLI only |
| C++ DLL changes | **No** | DLLs are unchanged; see Rejected alternatives |
| Editor tooling / export platform port | **No** | GDScript editor plugins run unchanged in a .NET project |

## Consumption model (shared by all addons)

Godot generates C# bindings only for **built-in** `ClassDB` classes at **engine-build
time**. Classes a GDExtension registers **at runtime** (`GDK`, `PlayFabLobby`,
`GameInputDevice`, …) get **no** generated C# types. From C# they are reachable only
dynamically:

```csharp
var gdk = Engine.GetSingleton("GDK");          // GodotObject (untyped)
var users = (GodotObject)gdk.Get("users");
bool ok = (bool)result.Get("ok");
```

The C# facade exists to replace that stringly-typed access with typed members. Each
facade type **wraps**, never copies: it holds the underlying `GodotObject` and exposes
`Get()`/`Call()`-backed typed properties and methods.

### Async bridge (the central design problem for `gdk` + `playfab`)

Every `_async` method on `godot_gdk` and `godot_playfab` returns a one-shot Godot
`Signal` that resolves to a typed `*Result` object (see `docs/async-patterns.md`). The
managed bridge awaits the returned `Signal` generically — extracting the emitter and
signal name from the C# `Signal` value — so **no DLL change and no dedicated host node
are required**:

```csharp
internal static async Task<T> AwaitResult<T>(Signal completion, Func<GodotObject, T> wrap)
{
    Variant[] payload = await completion.Owner.ToSignal(completion.Owner, completion.Name);
    return wrap((GodotObject)payload[0]);
}
```

Open items to lock down in Phase 0 (apply to both runtime addons):

- **Direct `Signal` await vs. named emitter signal** — confirm the exact mechanism for
  awaiting the anonymous returned `Signal` from C#.
- **Main-thread continuation** — native completions fire on the main thread during the
  per-frame `dispatch()` tick (`gdk/runtime/embed_dispatch`,
  `playfab/runtime/embed_dispatch`, both default `true`). Verify `Task` continuations
  resume on the main thread under Godot's C# sync context.
- **Shutdown races** — outstanding signals fire with a failure result on shutdown; the
  bridge must resolve, not hang.

### Multi-argument signal subscription (resolved gotcha)

Godot C#'s `Callable.From` has **no variadic overload** — only fixed-arity
`Action`/`Func` forms. Writing `Callable.From((Variant[] a) => …)` binds to the
single-argument generic `From<T0>(Action<T0>)` with `T0 = Variant[]`, producing a
callable that expects **exactly one** argument. Connecting it to a signal that emits a
different number of arguments throws `System.ArgumentException: Invalid argument count`
at emit time (caught in-engine for `GDK.users.user_changed`, a two-argument signal).

Every facade signal subscription therefore uses the conventional Godot C# idiom: an
**exact-arity typed lambda** whose parameter count matches the native signal, e.g.

```csharp
_o.Connect("user_changed", Callable.From((Variant a0, Variant a1) =>
    UserChanged?.Invoke(GdkUser.From(a0.AsGodotObject()), a1.AsString())));
```

`Variant`-typed parameters keep the per-argument `.AsX()` conversions explicit. Signal
arities come from the native `doc_classes` `<signal>` definitions; the parity test suite
also asserts each wrapped signal exists, so an arity drift surfaces as a build/test
failure rather than a silent runtime mismatch.

## `godot_gdk` — C# layer map

Singleton facade `Gdk` (lazy `Engine.GetSingleton("GDK")`), result type `GdkResult`
(`Ok`, `Message`, `Code`, `Data`, `DataAs<T>()`), and one wrapper per service member
and value type.

| Native | C# facade | Kind |
| --- | --- | --- |
| `GDK` | `Gdk` | singleton entry |
| `GDKResult` | `GdkResult` | result |
| `GDK.users` / `GDKUsers` | `Gdk.Users` / `GdkUsers` | service |
| `GDK.achievements` / `GDKAchievements` | `Gdk.Achievements` / `GdkAchievements` | service |
| `GDK.leaderboards` / `GDKLeaderboards` | `Gdk.Leaderboards` / `GdkLeaderboards` | service |
| `GDK.stats` / `GDKStats` | `Gdk.Stats` / `GdkStats` | service |
| `GDK.presence` / `GDKPresence` | `Gdk.Presence` / `GdkPresence` | service |
| `GDK.social` / `GDKSocial` | `Gdk.Social` / `GdkSocial` | service |
| `GDK.privacy` / `GDKPrivacy` | `Gdk.Privacy` / `GdkPrivacy` | service |
| `GDK.profile` / `GDKProfile` | `Gdk.Profile` / `GdkProfile` | service |
| `GDK.title_storage` / `GDKTitleStorage` | `Gdk.TitleStorage` / `GdkTitleStorage` | service |
| `GDK.string_verify` / `GDKStringVerify` | `Gdk.StringVerify` / `GdkStringVerify` | service |
| `GDK.package` / `GDKPackage` | `Gdk.Package` / `GdkPackage` | service |
| `GDK.store` / `GDKStore` | `Gdk.Store` / `GdkStore` | service |
| `GDK.multiplayer_activity` / `GDKMultiplayerActivity` | `Gdk.MultiplayerActivity` | service |
| `GDK.game_ui` / `GDKGameUI` | `Gdk.GameUi` / `GdkGameUi` | service |
| `GDK.accessibility` / `GDKAccessibility` | `Gdk.Accessibility` | service |
| `GDK.capture` / `GDKCapture` | `Gdk.Capture` / `GdkCapture` | service |
| `GDK.system` / `GDKSystem` | `Gdk.System` / `GdkSystem` | service |
| `GDK.display` / `GDKDisplay` | `Gdk.Display` / `GdkDisplay` | service |
| `GDK.activation` / `GDKActivation` | `Gdk.Activation` | service |
| `GDK.launcher` / `GDKLauncher` | `Gdk.Launcher` / `GdkLauncher` | service |
| `GDK.error_reporting` / `GDKErrorReporting` | `Gdk.ErrorReporting` | service |
| `GDKUser`, `GDKUserProfile` | `GdkUser`, `GdkUserProfile` | value |
| `GDKAchievement` | `GdkAchievement` | value |
| `GDKLeaderboard`, `GDKLeaderboardRow`, `GDKLeaderboardColumn` | matching wrappers | value |
| `GDKPresenceRecord` | `GdkPresenceRecord` | value |
| `GDKSocialUser`, `GDKSocialGroup`, `GDKSocialFilter` | matching wrappers | value |
| `GDKStoreLicenseStatus` | `GdkStoreLicenseStatus` | value |
| `GDKPackageMount`, `GDKPackageResourcePack` | matching wrappers | value |
| `GDKTitleStorageBlobMetadata`, `GDKTitleStorageBlobMetadataResult` | matching wrappers | value |
| `GDKMultiplayerActivityInfo` | `GdkMultiplayerActivityInfo` | value |
| `GDKCaptureMetaData` | `GdkCaptureMetaData` | value |
| `GDKClosedCaptionProperties` | `GdkClosedCaptionProperties` | value |
| `GDKDisplayTimeoutDeferral` | `GdkDisplayTimeoutDeferral` | value (holds native deferral; dispose semantics) |

Service-level `runtime_error` signals (on `achievements`, `social`, `presence`,
`multiplayer_activity`, …) are exposed as C# `event Action<GdkResult>`.

### Target usage

```csharp
GdkResult result = await Gdk.Users.AddDefaultUserAsync();
if (!result.Ok) { GD.PushWarning($"sign-in failed: {result.Message} ({result.Code})"); return; }
GdkUser user = result.DataAs<GdkUser>();
GD.Print($"signed in as {user.Gamertag}");

Gdk.Achievements.RuntimeError += r => GD.PushWarning($"[Ach] {r.Message}");
```

## `godot_playfab` — C# layer map

Singleton facade `PlayFab` (lazy `Engine.GetSingleton("PlayFab")`), result type
`PlayFabResult`, config props (`TitleId`, `Endpoint`), one wrapper per service member,
plus the Multiplayer/Party value/config/state types.

| Native | C# facade | Kind |
| --- | --- | --- |
| `PlayFab` | `PlayFab` (facade) | singleton entry |
| `PlayFabResult` | `PlayFabResult` | result |
| `PlayFab.users` / `PlayFabUsers`, `PlayFabUser` | `PlayFab.Users`, `PlayFabUser` | service + value |
| `PlayFab.game_saves` / `PlayFabGameSaves` | `PlayFab.GameSaves` | service |
| `PlayFab.leaderboards` / `PlayFabLeaderboards` | `PlayFab.Leaderboards` | service |
| `PlayFab.accounts` / `PlayFabAccounts` | `PlayFab.Accounts` | client service |
| `PlayFab.catalog` / `PlayFabCatalog` | `PlayFab.Catalog` | client service |
| `PlayFab.cloud_script` / `PlayFabCloudScript` | `PlayFab.CloudScript` | client service |
| `PlayFab.entity_data` / `PlayFabEntityData` | `PlayFab.EntityData` | client service |
| `PlayFab.events` / `PlayFabEvents` | `PlayFab.Events` | client service |
| `PlayFab.experimentation` / `PlayFabExperimentation` | `PlayFab.Experimentation` | client service |
| `PlayFab.friends` / `PlayFabFriends` | `PlayFab.Friends` | client service |
| `PlayFab.groups` / `PlayFabGroups` | `PlayFab.Groups` | client service |
| `PlayFab.inventory` / `PlayFabInventory` | `PlayFab.Inventory` | client service |
| `PlayFab.localization` / `PlayFabLocalization` | `PlayFab.Localization` | client service |
| `PlayFab.player_data` / `PlayFabPlayerData` | `PlayFab.PlayerData` | client service |
| `PlayFab.statistics` / `PlayFabStatistics` | `PlayFab.Statistics` | client service |
| `PlayFab.title_data` / `PlayFabTitleData` | `PlayFab.TitleData` | client service |
| `PlayFab.multiplayer` / `PlayFabMultiplayer` | `PlayFab.Multiplayer` | service (Lobby + Matchmaking) |
| `PlayFabMultiplayerConfig`, `PlayFabMultiplayerStateChange` | matching wrappers | config / state |
| `PlayFabLobby`, `PlayFabLobbyConfig`, `PlayFabLobbyJoinConfig`, `PlayFabLobbySearchConfig`, `PlayFabLobbyMember`, `PlayFabLobbyInvite`, `PlayFabLobbySummary`, `PlayFabLobbySearchResult`, `PlayFabLobbyStateChange` | matching wrappers | lobby value/config/state |
| `PlayFabMatchTicket`, `PlayFabMatchmakingMember`, `PlayFabMatchmakingTicketConfig`, `PlayFabMatchTicketStateChange` | matching wrappers | matchmaking value/config/state |
| `PlayFab.party` / `PlayFabParty` | `PlayFab.Party` | service (real-time net + chat) |
| `PlayFabPartyConfig`, `PlayFabPartyTextMessageConfig`, `PlayFabPartyMember`, `PlayFabPartyNetwork`, `PlayFabPartyNetworkStateChange`, `PlayFabPartyPeer`, `PlayFabPartyChat`, `PlayFabPartyChatControl`, `PlayFabPartyChatMessage`, `PlayFabPartyChatStateChange` | matching wrappers | party value/config/state/peer/chat |

PlayFab-specific nuances the C# layer must honor:

- **Cross-addon sign-in.** `PlayFab.users.sign_in_with_xuser_async` accepts a **GDK
  user object** (duck-typed `Object`, not a `Ref<GDKUser>`, because the two addons are
  separate DLLs). The C# `PlayFab.Users.SignInWithXUserAsync(GdkUser user)` must pass
  the **underlying `GodotObject`** through unchanged — never marshal a raw local Xbox
  user id. This mirrors the repo anti-pattern guidance.
- **Background error/state surfaces.** `PlayFab.multiplayer.multiplayer_error` and
  `PlayFab.party.party_error` are background callback-queue signals → exposed as C#
  `event Action<PlayFabResult>`. Lobby/match/party `*StateChange` notifications are
  surfaced as typed C# events as well.
- **Party networking + Godot RPC.** PlayFab Party provides a `MultiplayerPeer`-style
  transport (`PlayFabPartyPeer`) intended to drive Godot's high-level multiplayer. The
  C# facade must let a C# project assign the peer to `SceneTree.GetMultiplayer().MultiplayerPeer`
  and use `[Rpc]`-attributed methods, the C# analog of the GDScript Party RPC flow.
- **Title configuration.** `PlayFab.TitleId` / `PlayFab.Endpoint` map to the
  `playfab/runtime/title_id` and `playfab/runtime/endpoint` project settings; the C#
  runtime autoload reads them exactly as the GDScript bootstrap does.

## `godot_gameinput` — C# layer map (input-integration shape)

`godot_gameinput` is **not** a service/runtime addon and is intentionally **not**
forced into the singleton-with-service-namespaces shape. It is a **synchronous polling
+ signal** API with no `_async` methods, so the async bridge does not apply.

| Native | C# facade | Notes |
| --- | --- | --- |
| `GameInput` (singleton) | `GameInput` facade | `Initialize()`, `Shutdown()`, `IsInitialized()`, `Poll()`, `GetDevices()`, `GetPrimaryDevice()`, `GetCurrentReading()`, `SetVibration()`, `StopHaptics()`, `GetConnectedDeviceCount()` |
| `device_connected` / `device_disconnected` signals | C# `event Action<GameInputDevice>` | hot-plug |
| `GameInputDevice` | `GameInputDevice` | device handle/metadata |
| `GameInputReading` | `GameInputReading` | per-poll input snapshot |
| `GameInputBinding` | `GameInputBinding` | binding entry |
| `GameInputActionMap` | `GameInputActionMap` | **`Resource`-derived**; loadable from `*.tres` |
| `GameInputMapper` | `GameInputMapper` | action bridge into Godot `InputMap` |

C#-specific considerations:

- **`InputMap`/`Input` alignment.** The mapper bridges GameInput readings into Godot's
  `InputMap`. The C# facade should expose this so a C# game reads input through the
  normal `Input.IsActionPressed(...)` flow, consistent with the addon's design intent;
  the typed `GameInput` surface is additive (rumble, raw readings, device enumeration,
  hot-plug), not a replacement for `Input`.
- **Action-map Resources.** `GameInputActionMap`/`GameInputMapper` are Godot
  `Resource`s referenced by the `game_input/mapper/default_action_map` project
  setting; C# can load them with `GD.Load<Resource>()` / `ResourceLoader` — no special
  bridge needed.
- **No async bridge.** All calls are synchronous; results are returned directly or via
  a `GameInputResult` where one exists. Per-frame `Poll()` is driven by the bootstrap
  (`game_input/runtime/auto_poll`).

## `godot_gdk_packaging` — no C# port

`godot_gdk_packaging` is a **pure-GDScript editor plugin** (18 `.gd` files: headless
packaging core under `core/`, editor dialogs/wizard under `editor/`, CLI entry
`run.gd`). It registers **no native classes and no engine singleton**.

- A GDScript editor plugin **runs unchanged inside a Godot .NET project** — Godot
  supports GDScript and C# side by side, and editor tooling executes in the editor
  regardless of the game's scripting language. **No C# port is required.**
- The only optional C# work is a thin **C# wrapper over the headless packaging CLI**
  (`packaging_cli.gd` / `run.gd`) for teams that prefer to invoke packaging from a C#
  build pipeline. This is out of scope for the initial track and listed only for
  completeness.

## Runtime autoloads (bootstrap parity)

Each runtime addon ships a GDScript bootstrap autoload; the C# track provides a C#
equivalent that reads the **same** project settings and drives the same lifecycle. A
C# project registers the C# autoload **instead of** the GDScript one (one bootstrap,
not both).

| GDScript bootstrap | C# autoload | Reads settings |
| --- | --- | --- |
| `addons/godot_gdk/runtime/gdk_bootstrap.gd` | `GdkRuntime : Node` | `gdk/runtime/initialize_on_startup`, `gdk/runtime/auto_add_primary_user` |
| `godot_playfab` bootstrap | `PlayFabRuntime : Node` | `playfab/runtime/initialize_on_startup`, `playfab/runtime/title_id`, `playfab/runtime/endpoint` |
| `godot_gameinput` bootstrap | `GameInputRuntime : Node` | `game_input/runtime/initialize_on_startup`, `game_input/runtime/auto_poll`, `game_input/mapper/default_action_map` |

Each C# autoload binds the addon's lifecycle/notification signals, shuts the runtime
down on tree exit, and skips itself under the headless parse/test paths — mirroring the
GDScript bootstrap behavior exactly.

## Repo structure

The C# track lands in **parallel, additive** folders. No existing path changes
behavior. Each C# facade reuses the **existing** addon `bin/*.dll`; none rebuilds a
DLL, and CMake/the superproject are untouched.

```
addons/godot_gdk/                 # unchanged (source of truth)
addons/godot_gdk_csharp/          # NEW: GDK C# facade class library
addons/godot_playfab/             # unchanged
addons/godot_playfab_csharp/      # NEW: PlayFab C# facade class library
addons/godot_gameinput/           # unchanged
addons/godot_gameinput_csharp/    # NEW: GameInput C# facade class library
addons/godot_gdk_packaging/       # unchanged GDScript editor plugin (no C# port)

sample/tutorial_app_csharp/       # NEW: C# port of the GDK + PlayFab tutorial app
sample/tutorial_gameinput_csharp/ # NEW: C# port of the GameInput tutorial

tests/godot/gdk_csharp/           # NEW: C# in-engine test host (GoDotTest)
tests/godot/playfab_csharp/       # NEW: C# in-engine test host
tests/godot/gameinput_csharp/     # NEW: C# in-engine test host

docs/gdk/csharp.md                # NEW
docs/playfab/csharp.md            # NEW
docs/gameinput/csharp.md          # NEW
spec/gdext-csharp.md              # THIS FILE
```

Each `*_csharp` facade is **pure managed code** (`*.csproj`) referencing the sibling
addon's `bin/*.dll`. The C# build is `dotnet build` driven by the Godot .NET
sample/test projects' generated `.csproj`/`.sln`.

## Samples

- **`tutorial_app_csharp`** — C# port of `sample/tutorial_app` (GDK + PlayFab),
  starting with **T1 sign-in** (`t01_signin.gd` + `autoload/auth.gd`). The GDScript
  `Auth` autoload state machine (check → silent → UI fallback) maps to a C#
  `Auth : Node` autoload exposing `Task SignInAsync()` and a `StateChanged` event.
  Later scenes cover achievement, leaderboard, Game Saves, lobby, MPA, and Party flows
  — the Party scene exercises the C# Godot-RPC-over-Party path.
- **`tutorial_gameinput_csharp`** — C# port of `sample/tutorial_gameinput`, exercising
  device enumeration, polling, the `InputMap` bridge, rumble, and hot-plug.

Both samples require a **`_mono`** Godot editor and `dotnet/project/assembly_name`
configured in their `project.godot`.

## Tests

GUT is GDScript-only, so the C# track needs a C# runner.

- **Primary:** **GoDotTest** (Chickensoft) — xUnit-style tests that run inside a Godot
  .NET scene; closest analog to GUT for headless CI. One host per addon under
  `tests/godot/<addon>_csharp/`.
- **Parity tests** (one per runtime addon) assert that every native service member and
  every `_async` method has a corresponding C# wrapper member, catching drift as the
  native surface evolves. For `godot_gameinput`, the parity test covers the singleton
  method/signal surface and the device/reading/mapping types.

Each C# host is wired into the canonical green bar via a new orchestrator stage (or a
sibling `tools/run_csharp_tests.ps1`) so a C# build + headless test run joins
`tools/run_all_tests.ps1`. Note: `tools/check_gd_scripts_headless.ps1` is GDScript-only
and does **not** cover C#; the C# track adds its own `dotnet build` + headless test
gate. Live-service tiering (`-Live`, `-AllowLiveWrites`, sandbox title id) follows the
same contract as `tests/godot/README.md`.

## Build & CI integration

- Acquire a **`_mono`** Godot editor (parallel to the existing
  `sample/Godot_v4.6.1-stable_win64.exe`) for the C# sample/test projects.
- C# build = `dotnet build` on the generated sample/test `.csproj`.
- Add a C# stage to the test orchestrator (build + headless GoDotTest run per host).
- Validate an end-to-end **.NET MSIXVC export** early (see Risks #1) — this is the
  intersection with `godot_gdk_packaging`, which assumes a native/GDScript export
  profile.

## Error/result conventions

- `godot_gdk` async → `GdkResult`; `godot_playfab` async → `PlayFabResult`; the rare
  `godot_gameinput` failures → `GameInputResult`. All expose `Ok`, `Message`, `Code`
  (stable string id), `Data`, and a typed `DataAs<T>()`.
- Background/notification signals (`runtime_error`, `multiplayer_error`, `party_error`,
  `*StateChange`, `device_connected`/`device_disconnected`) are exposed as C# `event`s.

## Risks & open questions

1. **GDExtension + .NET co-existence in an exported Xbox-on-PC MSIXVC package.**
   Highest-risk unknown. Both work in-editor; the exported managed-runtime package is
   unverified, and `godot_gdk_packaging` assumes a native/GDScript export profile.
   **Must validate a .NET export in Phase 0** before investing in full coverage.
2. **Anonymous returned `Signal` await from C#** — confirm the bridge mechanism.
3. **Main-thread `Task` continuation** after native `dispatch()`.
4. **PlayFab Party `MultiplayerPeer` from C#** — confirm a `PlayFabPartyPeer` can be
   assigned to `Multiplayer.MultiplayerPeer` and drive `[Rpc]` methods from C#, the
   analog of the GDScript Party RPC flow.
5. **`GameInputActionMap` Resources from C#** — confirm `*.tres` action maps load and
   bind through the mapper unchanged under .NET.
6. **Two editors in-repo** (`_mono` + non-mono) increases setup friction and the CI
   matrix.
7. **Surface drift** — three parallel C# surfaces must track ~94 native classes.
   Mitigation: keep facades thin (wrap, don't reimplement) + per-addon parity tests.
8. **Export-platform recognition** — confirm the GDScript export platform plugin
   recognizes a .NET project unchanged.

## Rejected alternatives

- **Change the DLLs to a `Callable`-completion (or otherwise C#-friendly) async model.**
  Rejected. It does not remove the dominant cost (typed wrappers are unavoidable —
  Godot generates no C# bindings for runtime GDExtension classes regardless), and
  replacing the Signal-return pattern would regress the GDScript surface, whose entire
  documented contract is "`await` is the only thing you need" (`docs/async-patterns.md`).
  Additive `Callable` overloads avoid the regression but double native maintenance for
  a small managed-side win. Revisit **only** if the Phase-0 spike shows awaiting the
  returned `Signal` from C# is genuinely painful — and then prefer additive overloads.
- **A separate C# thunk (P/Invoke) directly to the GDK / PlayFab / GameInput flat-C
  APIs.** Rejected for this repo's goals. The C++ GDExtensions embed the hardest, most
  valuable logic — the `XAsyncBlock`/`XTaskQueue` → Godot `dispatch()` → `Signal`
  bridges, manager `DoWork` pump loops, handle lifetime, PlayFab/Party background
  callback queues and networking, and `HRESULT` → result normalization. A thunk
  re-implements all of it in C#, creating a second source of truth, losing
  engine-native object flow into scenes/UI/multiplayer, and violating the
  "wrap, don't reimplement / one runtime" principle. A thunk is only appropriate for a
  **Godot-independent .NET SDK**, which is out of scope. If dynamic `GodotObject.Call`
  ergonomics ever become a measured problem, the narrower lever is a few `extern "C"`
  exports on the **existing** GDExtension DLLs (a thunk over the C++ layer, not the
  flat-C API), added only where profiled.
- **Port `godot_gdk_packaging` to C#.** Rejected as unnecessary — the GDScript editor
  plugin runs unchanged in a .NET project. Only a thin C# CLI shim over the headless
  packaging entry point is worth considering, and only on demand.

## Plan

Phases are additive and each lands its public C# surface **plus** the matching test
tier and doc update in the same change, per the repo's "Before Reporting Completion"
checklist. `godot_gdk` leads; `godot_playfab` and `godot_gameinput` follow the proven
pattern.

- **Phase 0 — Spike / feasibility.** Minimal `godot_gdk` facade (`Gdk`, `GdkUsers`,
  `GdkResult`, async bridge) + one C# scene that awaits `AddDefaultUserAsync()` and
  prints the gamertag in a `_mono` editor. **Plus** a throwaway `.NET` MSIXVC export to
  de-risk Risk #1. Decision gate: confirm Risks #1–#3 before committing to full
  coverage. Tier: manual/smoke.
- **Phase 1 — GDK facade core.** Hardened async bridge, `GdkResult`, `GdkRuntime`
  autoload, and the `users`/`achievements`/`leaderboards` services + value types.
  Parity-test scaffold. Tier: in-engine unit (GoDotTest).
- **Phase 2 — GDK full coverage.** Remaining 18 GDK services + value types +
  `runtime_error` event wrappers. Parity test asserts complete GDK coverage.
- **Phase 3 — GDK sample + docs.** C# port of `tutorial_app` T1–T2 with a C# `Auth`
  autoload; `docs/gdk/csharp.md`.
- **Phase 4 — PlayFab facade core.** `PlayFab` facade, `PlayFabResult`,
  `PlayFabRuntime` autoload, `users` (incl. cross-addon `SignInWithXUserAsync`),
  `game_saves`, `leaderboards`, and the client services. Parity test + docs.
- **Phase 5 — PlayFab Multiplayer + Party.** `multiplayer` (Lobby + Matchmaking) and
  `party` (real-time net + chat), including the C# Godot-RPC-over-Party path and the
  background error/state event wrappers. Extend `tutorial_app_csharp` (lobby, MPA,
  Party scenes). Resolve Risk #4.
- **Phase 6 — GameInput facade + sample.** `GameInput` facade (sync polling,
  hot-plug events, vibration), `GameInputRuntime` autoload, action-map Resource +
  `InputMap` bridge, `tutorial_gameinput_csharp`, `docs/gameinput/csharp.md`. Resolve
  Risk #5.
- **Phase 7 — Tests + CI consolidation.** GoDotTest hosts for all three addons wired
  into the orchestrator; C# build/parse gate; finalize the `_mono` editor acquisition
  step.
- **Phase 8 — Docs polish + README.** README addon-table note for the C# track,
  cross-links, and GDScript-parity notes across `docs/*/csharp.md`.

(`godot_gdk_packaging` has no phase — it runs unchanged; an optional C# CLI shim can be
added later on demand.)

## Progress

Built on branch `feat/gdk-csharp` (not pushed). The C# track is validated with
`dotnet build` + `dotnet test` (headless parity suite) **and** in-engine headless runs
against a `Godot_v4.6.3-stable_mono_win64` editor. All C# `.csproj` files pin
`Godot.NET.Sdk/4.6.3` to match that editor. The headless parity suite remains the
fast green bar; in-engine smoke runs confirm the facades drive the native singletons
end-to-end (GDK init → `user_changed` → Xbox primary user → cross-addon PlayFab
sign-in → Lobby/Party wiring; GameInput init → hot-plug device enumeration).

- **Phase 1–2 — GDK facade: ✅ shipped.** `GodotGdk.Gdk` static entry point wiring
  all 21 services, 19 value types, `GdkResult`, the Signal→`Task` bridge,
  `runtime_error`/root signal event wrappers, and the `GdkRuntime` autoload.
  Compiles clean (`addons/godot_gdk_csharp`).
- **Phase 4–5 — PlayFab facade: ✅ shipped.** `GodotPlayFab.PlayFab` with all 18
  services, 26 value types (incl. Lobby/Matchmaking/Party), `PlayFabResult`,
  background error/state event wrappers, cross-addon
  `SignInWithXUserAsync(GdkUser)` (duck-typed `GodotObject` pass-through via a
  ProjectReference to the GDK facade), and `PlayFabRuntime` autoload.
- **Phase 6 — GameInput facade + sample: ✅ shipped.** `GodotGameInput.GameInput`
  (sync poll model, hot-plug events, vibration), device/reading wrappers with
  `Button`/`Axis`/`Source`/`DeviceKind` enums, `GameInputActionMap`/`Binding`/
  `Mapper` authoring wrappers + `InputMap` bridge, `GameInputRuntime` autoload,
  and `sample/tutorial_gameinput_csharp`. In-engine smoke: device hot-plug enumerated,
  clean shutdown.
- **Phase 3 — GDK + PlayFab sample (`tutorial_app_csharp`): ✅ shipped.** All eight
  tutorial scenes, autoloads (`Auth`/`Lobby`/`Party`/bootstraps), and panels ported.
  In-engine headless smoke completes the full sign-in flow with no errors.
- **Phase 7 — Tests: ✅ partial.** `tests/csharp/FacadeParity.Tests` (xUnit, 97
  tests) reflects over all three facade assemblies and asserts every native
  `doc_classes` method/member/signal has a managed wrapper; run via
  `tools/run_csharp_tests.ps1`. The suite already caught and fixed real drift in
  the GDK Social wrappers. Dedicated in-engine GoDotTest hosts remain a follow-up,
  but the facades are now exercised in-engine via the sample smoke runs.
- **Phase 8 — Docs: ✅ shipped.** `docs/gdk/csharp.md`, `docs/playfab/csharp.md`,
  `docs/gameinput/csharp.md`, and the C# section in `docs/README.md`.
- **Phase 0 — `_mono` editor acquisition: ✅ done.** Validated against
  `Godot_v4.6.3-stable_mono_win64`; the signal-arity subscription bug surfaced and
  was fixed (see "Multi-argument signal subscription"). MSIXVC .NET export de-risk
  remains ⬜ deferred.

