# PlayFab GDExtension Spec

## Overview

This document defines the current design direction for the `godot_playfab` addon.

`godot_playfab` owns PlayFab runtime bootstrap, manual PlayFab sign-in keyed by a `GDKUser` object or title-defined custom id, Game Saves flows, leaderboard flows, and client-safe PlayFab Services SDK wrappers. The public API is intentionally GDScript-first: a single `PlayFab` root singleton, `RefCounted` wrapper types, dictionaries for rich SDK payloads, and direct-await completion signals for one-shot services.

Lobby/matchmaking and Party design work are tracked separately in `spec\gdext-playfab-lobby-matchmaking.md` and `spec\gdext-playfab-party.md`. Those planned surfaces add `PlayFab.multiplayer` and `PlayFab.party` while keeping lobby/matchmaking APIs separate from Party transport.

## Design goals

1. **Single root singleton** — expose one `PlayFab` entry point instead of multiple global singletons.
2. **Manual sign-in** — PlayFab sign-in is an explicit gameplay action, even though the addon can resolve local Xbox users through `XUser`.
3. **Project-settings-backed config** — runtime configuration comes from `playfab/titleid` and `playfab/endpoint`.
4. **Godot-native async flow** — one-shot requests return completion `Signal` values that resolve with `PlayFabResult`.
5. **Typed service entry points** — higher-level services require an already-signed-in `PlayFabUser`; rich payloads use `Dictionary` request/response data.
6. **Idempotent loading** — multiple synced `.gdextension` files pointing at the same DLL must not duplicate class or singleton registration.

## Scope

| Domain | Included | Notes |
| --- | --- | --- |
| Runtime init/shutdown | Yes | `XGameRuntimeInitialize`, PlayFab init, shared queue |
| Manual sign-in | Yes | `GDKUser` object and custom-ID entry points |
| Cached user sessions | Yes | `PlayFabUser` keyed by local Xbox user id or custom id |
| Game Saves | Yes | add/sync, upload, folder/quota/cloud-state queries |
| Leaderboards | Yes | submit, global, around-user, friends/social |
| Client services | Yes | accounts, catalog, CloudScript, entity data, experimentation, friends, groups, inventory, localization, player data, statistics, title data |
| Events/telemetry | Reserved | `PlayFab.events` exists, but no active client event operation is available from the current GDK headers |
| Multiplayer lobbies / matchmaking | Planned | see `spec\gdext-playfab-lobby-matchmaking.md` |
| Party transport | Planned | see `spec\gdext-playfab-party.md` |
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
- Planned: `PlayFab.multiplayer` for lobbies and matchmaking
- Planned: `PlayFab.party` for Party transport and chat

## Runtime configuration

The runtime reads these Project Settings keys:

- `playfab/titleid`
- `playfab/endpoint`
- `playfab/runtime/embed_dispatch`

The endpoint setting is optional. When blank, the runtime derives the default endpoint from the title id.

## Async model

The addon uses one shared task queue owned by the PlayFab runtime. Native completion stays inside the extension until it is converted into Godot-friendly state and emitted through completion signals.

Rules:

1. `PlayFab.users.sign_in_with_xuser_async()`, `PlayFab.users.sign_in_with_custom_id_async()`, Game Saves calls, leaderboard calls, and PlayFab service calls all return completion signals awaited directly.
2. Completion data is delivered through `PlayFabResult`.
3. `PlayFabResult.data` uses Godot-native types (`Dictionary`, `Array`, `String`, `int`, etc.).
4. With `playfab/runtime/embed_dispatch = true`, the addon pumps completions automatically each process frame.
5. When embed dispatch is disabled, callers must pump the queue manually with `PlayFab.dispatch()`.

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

## PlayFab Services SDK wrappers

PlayFab services cover the client-safe, non-Multiplayer, non-Party PlayFab Services SDK operations that are active in the GDK header set. Server/admin/title-secret APIs are excluded, as are Multiplayer and Party APIs owned by separate workstreams.

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

## Samples

- `sample\playfab_demo` demonstrates settings-backed init plus manual PlayFab sign-in
- `sample\multiplayer_pong` demonstrates a sample-local wrapper that signs the
  active Xbox user into PlayFab, persists save JSON through Game Saves, and
  submits and queries the roguelike leaderboard

## Tests

- `tests\godot\playfab\tests\` is the PlayFab contract suite.
- It should keep the root singleton, settings registration, deterministic
  validation errors, custom-ID sign-in, and optional live smoke flows aligned
  with the shipped addon behavior.
