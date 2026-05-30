# PlayFab GDExtension Spec

## Overview

This document defines the current design direction for the `godot_playfab` addon.

`godot_playfab` owns PlayFab runtime bootstrap, manual PlayFab sign-in keyed by a `GDKUser` object or title-defined custom id, Game Saves flows, leaderboard flows, client-safe PlayFab Services SDK wrappers, and the MLP lobby/matchmaking surface built on top of the PlayFab C SDKs. The public API is intentionally GDScript-first: a single `PlayFab` root singleton, `RefCounted` wrapper types, dictionaries for rich SDK payloads, and direct-await completion signals for one-shot services.

Lobby/matchmaking and Party design work are tracked separately in `spec\gdext-playfab-lobby-matchmaking.md` and `spec\gdext-playfab-party.md`. The MLP adds `PlayFab.multiplayer` for lobbies and matchmaking while keeping Party transport deferred to `PlayFab.party`.

## Design goals

1. **Single root singleton** — expose one `PlayFab` entry point instead of multiple global singletons.
2. **Manual sign-in** — PlayFab sign-in is an explicit gameplay action, even though the addon can resolve local Xbox users through `XUser`.
3. **Project-settings-backed config** — runtime configuration comes from `playfab/runtime/title_id` and `playfab/runtime/endpoint`.
4. **Godot-native async flow** — one-shot requests return completion `Signal` values that resolve with `PlayFabResult`.
5. **Typed service entry points** — higher-level services require an already-signed-in `PlayFabUser`; rich payloads use `Dictionary` request/response data.
6. **Idempotent loading** — multiple synced `.gdextension` files pointing at the same DLL must not duplicate class or singleton registration.

## Scope

| Domain | Included | Notes |
| --- | --- | --- |
| Runtime init/shutdown | Yes | Process-lifetime `XGameRuntimeInitialize` reference, re-armable PlayFab init/shutdown, shared queue |
| Manual sign-in | Yes | `GDKUser` object and custom-ID entry points |
| Cached user sessions | Yes | `PlayFabUser` keyed by local Xbox user id or custom id |
| Game Saves | Yes | add/sync, upload, folder/quota/cloud-state queries |
| Leaderboards | Yes | submit, global, around-user, friends/social |
| Client services | Yes | accounts, catalog, CloudScript, entity data, experimentation, friends, groups, inventory, localization, player data, statistics, title data |
| Events/telemetry | Reserved | `PlayFab.events` exists, but no active client event operation is available from the current GDK headers |
| Multiplayer lobbies / matchmaking | Yes | MLP implemented; see `spec\gdext-playfab-lobby-matchmaking.md` |
| Party transport | Yes | Full Party transport, host/join, chat/mute/permissions, and `MultiplayerPeerExtension` exposed under `PlayFab.party`. See `spec\gdext-playfab-party.md`. |
| Server/admin/title-secret APIs | No | excluded from the client addon surface |

## Public API summary

### Root singleton

- `PlayFab`

### Wrapper types

| Native concept | GDScript wrapper |
| --- | --- |
| one-shot async request | `Signal` |
| HRESULT + payload | `PlayFabResult` |
| PlayFab session | `PlayFabUser` |

### Service surfaces

- `PlayFab.users`
- `PlayFab.game_saves`
- `PlayFab.leaderboards`
- `PlayFab.multiplayer` for lobbies and matchmaking
- `PlayFab.accounts`
- `PlayFab.catalog`
- `PlayFab.cloud_script`
- `PlayFab.entity_data`
- `PlayFab.events`
- `PlayFab.experimentation`
- `PlayFab.friends`
- `PlayFab.groups`
- `PlayFab.inventory`
- `PlayFab.localization`
- `PlayFab.player_data`
- `PlayFab.statistics`
- `PlayFab.title_data`
- `PlayFab.party` for Party network host/join, peer transport (a Godot `MultiplayerPeerExtension`), chat controls, mute, and permissions. See `spec\gdext-playfab-party.md`.

## Project Settings

The addon registers these Project Settings, but they are consumed by different layers. Only `playfab/runtime/title_id` and `playfab/runtime/endpoint` are read by `PlayFabRuntime::initialize()`.

| Setting | Default | Consumed by | Notes |
| --- | --- | --- | --- |
| `playfab/runtime/title_id` | `""` | `PlayFabRuntime::initialize()` | Required PlayFab Title ID. |
| `playfab/runtime/endpoint` | `""` | `PlayFabRuntime::initialize()` | Optional; when blank, the runtime derives `https://<titleid>.playfabapi.com`. |
| `playfab/runtime/initialize_on_startup` | `false` | `PlayFabBootstrap` autoload | When `true`, the autoload calls `PlayFab.initialize()` during `_ready`; sign-in remains title-owned. |
| `playfab/runtime/embed_dispatch` | `true` | extension frame callback | On Godot builds with frame-callback support, auto-pumps `PlayFab.dispatch()` each process frame while initialized. |
| `playfab/party/local_udp_socket_bind_port` | `-1` | `PlayFab.party` | Valid range `-1..65535`. `-1` keeps the SDK default bind address; `0` asks the OS for an ephemeral port to avoid same-host Party UDP collisions in dev/CI; `1..65535` pins a port. `PlayFabParty.initialize_async(..., local_udp_port)` can override it per call. |

## Runtime lifecycle

`PlayFab.initialize()` acquires this addon's GDK `XGameRuntimeInitialize` reference the first time the runtime initializes successfully and keeps it for the process lifetime. `PlayFab.shutdown()` tears down PlayFab-owned state (`PFGameSaveFiles`, service config, `PFServices`, `PFInitialize`, pending signals, cached users, and the shared task queue) but deliberately does not call `XGameRuntimeUninitialize`; the matching GDK runtime release happens once during extension teardown. This mirrors the `godot_gdk` singleton/runtime pattern and allows titles or tests to cycle `initialize() -> shutdown() -> initialize()` without racing the process-wide Gaming Runtime, even when both addons are loaded.

## Async model

The addon uses one shared task queue owned by the PlayFab runtime. Native completion stays inside the extension until it is converted into Godot-friendly state and emitted through completion signals. See `docs/playfab/async-system.md` for the user-facing contract.

Rules:

1. `PlayFab.users.sign_in_with_xuser_async()`, `PlayFab.users.sign_in_with_custom_id_async()`, Game Saves calls, leaderboard calls, Party calls, Multiplayer calls, and PlayFab service calls all return completion signals awaited directly.
2. Each completion signal is one-shot: it resolves at most once, always with a `PlayFabResult`.
3. `PlayFabResult.data` uses Godot-native types (`Dictionary`, `Array`, `String`, `int`, etc.).
4. Completions are main-thread work. SDK callbacks are drained by the manual completion queue when `PlayFab.dispatch()` runs; immediate and synchronous completion paths use Godot `call_deferred` before emitting.
5. With `playfab/runtime/embed_dispatch = true`, the extension frame callback pumps `PlayFab.dispatch()` automatically each process frame.
6. When embed dispatch is disabled, callers must pump the queue manually with `PlayFab.dispatch()`.
7. `PlayFab.dispatch()` pumps the shared PlayFab runtime queue, PlayFab Multiplayer lobby/matchmaking state changes, and PlayFab Party state changes.
8. Shutdown cancels outstanding Party and Multiplayer completion signals before native SDK teardown, rejects new Party/Multiplayer work while shutdown is in progress, defers native teardown until any active SDK state-change batch has been finished, and only frees native async context storage after `PartyManager::Cleanup()` / `PFMultiplayerUninitialize()` has returned.

For Multiplayer lobbies, successful local `PlayFabLobby.set_member_properties_async()` writes update the local member snapshot eagerly before the completion signal settles; remote member-property changes continue to arrive through SDK-driven `MEMBER_UPDATED` state changes.

## User/session model

`PlayFabUser` represents one signed-in PlayFab session associated with either a local Xbox user id or a title-defined custom id.

Publicly exposed data is intentionally narrow:

- `local_id`
- `custom_id`
- `entity_key`
- `has_local_user_handle`

Xbox-facing identity details do not belong on the PlayFab user wrapper. The wrapper only exposes what higher-level PlayFab systems need. Custom-ID users have `local_id == 0`, a populated `custom_id`, and no local user handle.

`PlayFab.users` is intentionally cache/result-driven and does not expose user lifecycle signals. Titles should use explicit sign-in results plus cache lookups (`get_user_by_local_id()`, `get_user_by_custom_id()`, and `get_users()`) instead.

## Game Saves

Game Saves methods require an Xbox-backed signed-in `PlayFabUser`. The runtime initializes `PFGameSaveFiles` alongside the rest of the PlayFab bootstrap, and Xbox-backed user wrappers retain the internal `PFLocalUserHandle` needed by the Game Saves APIs. Custom-ID users are valid PlayFab sessions for services that use entity handles, but Game Saves methods reject them with `xbox_user_required`.

Supported calls:

- `add_user_with_ui_async(user, options := PlayFabGameSaves.ADD_USER_OPTION_NONE)`
- `upload_with_ui_async(user, release_device_as_active := false)`
- `set_save_description_async(user, short_save_description)`
- `reset_cloud_async(user)`
- `get_folder(user)`
- `get_folder_size(user)`
- `get_remaining_quota(user)`
- `is_connected_to_cloud(user)`

The add-user wrapper returns a snapshot dictionary containing the synced folder path, folder size, cloud-connection state, local id, and entity key. When the user is connected to the cloud, the result also includes the remaining quota in bytes.

## Leaderboards

Leaderboard methods require a signed-in `PlayFabUser` and do not accept a raw local id or arbitrary variant.

Supported calls:

- `submit_score_async(user, leaderboard_name, score, additional_scores := [], metadata := "")`
- `get_leaderboard_async(user, leaderboard_name, start_position := 1, page_size := 10, version := -1)`
- `get_leaderboard_around_user_async(user, leaderboard_name, max_surrounding_entries := 10, version := -1)`
- `get_friend_leaderboard_async(user, leaderboard_name, include_xbox_friends := true, version := -1)`

Recommended client writes use `PlayFab.statistics.update_statistics_async()` against a statistic-backed leaderboard, then read through `PlayFab.leaderboards`. `submit_score_async()` maps to the direct LeaderboardsV2 update path; treat non-statistic-backed direct writes as server/trusted-backend work that uses a developer secret key outside the Godot client. Never ship a PlayFab developer secret in a Godot project.

## PlayFab Services SDK wrappers

PlayFab services cover the client-safe, non-Multiplayer, non-Party PlayFab Services SDK operations that are active in the GDK header set. Server/admin/title-secret APIs are excluded. Multiplayer and Party APIs are owned by dedicated surfaces.

Method shape:

- `service.method_async(user: PlayFabUser, request := {}) -> Signal`
- `request` uses snake_case versions of the PlayFab C SDK request field names
- successful result payloads are converted to Godot `Dictionary` and `Array` values under `PlayFabResult.data`
- void PlayFab operations complete with `PlayFabResult.data == null`
- operations with GDK-only `XUserHandle` request fields require a signed-in `GDKUser` object in `request.user`; raw local ids are not accepted

Service buckets:

| Root property | Class | Current operations |
| --- | --- | --- |
| `accounts` | `PlayFabAccounts` | 31 account, profile, contact-email, link/unlink, and identity lookup operations |
| `catalog` | `PlayFabCatalog` | 26 catalog config, item, review, upload URL, search, and moderation operations |
| `cloud_script` | `PlayFabCloudScript` | 3 CloudScript/function execution operations |
| `entity_data` | `PlayFabEntityData` | 7 entity file and object operations |
| `events` | `PlayFabEvents` | reserved namespace; no active client operation available from the current GDK headers |
| `experimentation` | `PlayFabExperimentation` | 1 treatment assignment operation |
| `friends` | `PlayFabFriends` | 4 friends-list and friend-tag operations |
| `groups` | `PlayFabGroups` | 25 group membership, role, invitation, application, and block operations |
| `inventory` | `PlayFabInventory` | 17 inventory collection, item, purchase, transfer, redemption, and transaction operations |
| `localization` | `PlayFabLocalization` | 1 language-list operation |
| `player_data` | `PlayFabPlayerData` | 10 user data, publisher data, and custom-property operations |
| `statistics` | `PlayFabStatistics` | 10 statistic definition and entity statistic operations |
| `title_data` | `PlayFabTitleData` | 4 publisher data, title data, title news, and server-time operations |

## Multiplayer lobbies and matchmaking

The MLP `PlayFab.multiplayer` service supports PlayFab Multiplayer initialization, lobby create/join/search, matchmaking ticket create, ticket enumeration, and explicit arranged-lobby joins. `PlayFabLobby` owns lobby leave and lobby/member property updates. `PlayFabMatchTicket` owns ticket cancellation and status refresh. User-owned native calls use the signed-in `PlayFabUser`'s internal `PFEntityHandle` overloads.

`create_match_ticket_async` resolves only after the SDK assigns a non-empty `ticket_id` to the returned `PlayFabMatchTicket`; half-created local handles remain internal. Match tickets report `match_id` and `arranged_lobby_connection_string` through `PlayFabMatchTicketStateChange`; title code decides whether to call `join_arranged_lobby_async(...)`. The addon does not automatically join arranged lobbies.

Failed lobby create/join completions are terminal for their temporary wrapper: the wrapper is removed from `PlayFab.multiplayer.get_lobbies()` and marked disconnected before the failure result is surfaced. If `PFLobbyFinishStateChanges` or `PFMatchmakingFinishStateChanges` fails, the service emits `multiplayer_error`, tears down native Multiplayer state (including the task queue), marks wrappers detached, and returns to `is_initialized() == false`; titles must call `initialize_async()` before issuing more Multiplayer calls.

`PlayFab.party` follows the same fail-closed recovery contract for `PartyManager::FinishProcessingStateChanges`: it emits `party_error`, detaches active network/chat wrappers, resets `PartyManager`, and requires `initialize_async()` before further Party calls.

Party and Multiplayer shutdown paths are cancellation-first: they emit cancelled `PlayFabResult`s for pending operations, tolerate handlers that re-enter the addon, defer native teardown until any active SDK state-change batch has unwound, then keep native async context storage alive until after the relevant SDK cleanup call completes.

## Samples

Current `sample\` contents:

- `sample\tutorial_app\` — committed integrated tutorial sample for the GDK + PlayFab chain, including PlayFab sign-in, statistic-backed leaderboards, Game Saves, Lobby, Multiplayer Activity, Party, and the integration tech demo.
- `sample\tutorial_gameinput\` — committed standalone GameInput sample; it does not exercise PlayFab.

Use `sample\tutorial_app\` and `tests\godot\playfab\` for PlayFab reference behavior.

## Tests

- `tests\godot\playfab\tests\` is the PlayFab contract suite.
- It should keep the root singleton, settings registration, deterministic
  validation errors, runtime lifecycle re-arm behavior, API service contracts, Multiplayer class contracts, custom-ID sign-in, and optional live smoke flows aligned
  with the shipped addon behavior.
- `tools\configure_playfab_test_title.ps1` prepares the current sandbox title (`10D176` by default) for live coverage by ensuring the custom-ID account, Multiplayer worker accounts, a two-player matchmaking queue with a `run_id` equality rule, leaderboard/statistic definitions, and API-service fixtures used by the tests exist. It reads the developer secret from `PLAYFAB_DEVELOPER_SECRET_KEY` and never forwards the secret to Godot child processes.
- `tools\run_all_tests.ps1 -Live -Hosts tests\godot\playfab -PlayFabTitleId "10D176" -PlayFabCustomId "godot-gdk-ext-live-smoke" -PlayFabMatchmakingQueue "godot_gdk_ext_live_smoke_queue"` also runs the opt-in PlayFab Multiplayer multi-client lobby orchestration plus matchmaking create/cancel, two-player match completion, explicit arranged-lobby join, and arranged-lobby cleanup coverage.
