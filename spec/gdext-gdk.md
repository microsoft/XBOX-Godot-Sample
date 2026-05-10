# GDK GDExtension Spec

## Overview

This document defines a **GDScript-first** plan for the `godot_gdk` Godot GDExtension plugin.

`godot_gdk` owns the GDK runtime, users, PC GDK helper services, and Xbox Services wrappers. Input is intentionally out of scope for this document; the companion input design lives in `gdext-gameinput.md`.

The core architectural rule is: **C++ is internal; GDScript is the primary public surface**. Xbox Services wrappers are scoped to the public `xsapi-c` headers and are exposed as service namespaces under the single `GDK` singleton.

## Design goals

1. **GDScript-first API**: snake_case methods, signals, Godot types, no raw native handles.
2. **GDExtension-only**: no custom Godot fork required.
3. **Service partitioning**: avoid one flat mega-singleton.
4. **Godot-native ergonomics**: `await`, signals, `RefCounted` wrappers, optional `Resource` configs.
5. **Graceful failure in editor/non-target runtime**: no crashes if GDK is unavailable.
6. **No input in `GDK`**: input lives in the companion `godot_gameinput` plugin.

## Scope

| Domain | Status | Notes |
| --- | --- | --- |
| Runtime init/shutdown | Implemented | `XGameRuntimeInitialize`, queue/bootstrap |
| User identity/sign-in | Implemented | `XUser`-backed; not an Xbox Services `xsapi-c` surface |
| PC GDK helpers | Implemented selectively | Includes launcher, game UI, accessibility, system metadata, and error reporting; these are outside the Xbox Services coverage matrix below |
| Package metadata and DLC content access | Implemented | `GDK.package` wraps PC-supported `XPackage` enumeration, install progress, package mounts, and resource-pack loading |
| Achievements | Implemented | Achievements Manager-backed |
| Presence | Implemented | `GDK.presence` covers set/clear, single/multi-user queries, social-group query, tracking, non-deprecated change handlers, and cache access |
| Social | Implemented | Social Manager graph/groups and reputation feedback are exposed; direct relationship paging remains intentionally excluded |
| Multiplayer activity | Implemented and capped | `GDK.multiplayer_activity` is the only multiplayer-related Xbox Services surface |
| Stats | Implemented | `GDK.stats` wraps `title_managed_statistics_c.h` and `user_statistics_c.h` for reads, staged writes, tracking, and cache access |
| Leaderboards | Implemented | `GDK.leaderboards` wraps read-only `leaderboard_c.h` queries and pagination |
| Privacy | Implemented | `GDK.privacy` wraps `privacy_c.h` permission checks and avoid/mute list reads; list-change handlers are not exported by the public XSAPI thunk libraries used by the addon |
| Profile | Implemented | `GDK.profile` wraps `profile_c.h` profile lookup APIs |
| String verification | Implemented | `GDK.string_verify` wraps `string_verify_c.h` text verification APIs |
| Title Storage | Implemented | `GDK.title_storage` wraps `title_storage_c.h`; do not confuse with PlayFab Game Saves or `XGameSaveFiles` |
| Capture | Implemented | `GDK.capture` wraps PC-supported `XAppCapture` metadata/state APIs; console-only paths excluded |
| Events | Excluded | Do not wrap `events_c.h` telemetry/configuration APIs |
| Multiplayer/session/matchmaking | Excluded | Do not wrap matchmaking, MPSD, multiplayer sessions, lobby/session transport, or legacy invite APIs |
| Store/commerce/licensing | Out of scope here | XStore is not Xbox Services; do not include it in this Xbox Services wrapper plan |

### Xbox Services coverage matrix

#### Already exposed

| Public service | Native Xbox Services APIs | Notes |
| --- | --- | --- |
| `GDK.achievements` | Achievements Manager APIs | Query/update/cache are manager-backed. |
| `GDK.stats` | `XblUserStatisticsGetSingleUserStatisticsAsync`, `XblUserStatisticsGetSingleUserStatisticsResultSize`, `XblUserStatisticsGetSingleUserStatisticsResult`, `XblUserStatisticsGetMultipleUserStatisticsAsync`, `XblUserStatisticsGetMultipleUserStatisticsResultSize`, `XblUserStatisticsGetMultipleUserStatisticsResult`, `XblUserStatisticsAddStatisticChangedHandler`, `XblUserStatisticsRemoveStatisticChangedHandler`, `XblUserStatisticsTrackStatistics`, `XblUserStatisticsStopTrackingStatistics`, `XblUserStatisticsStopTrackingUsers`, `XblTitleManagedStatsUpdateStatsAsync` | Query one or more users' stats, stage integer/number writes, flush staged stats, track changes, and read cached stats. |
| `GDK.leaderboards` | `XblLeaderboardGetLeaderboardAsync`, `XblLeaderboardGetLeaderboardResultSize`, `XblLeaderboardGetLeaderboardResult`, `XblLeaderboardResultGetNextAsync`, `XblLeaderboardResultGetNextResultSize`, `XblLeaderboardResultGetNextResult` | Read-only global, around-user, social leaderboard queries plus next-page pagination. |
| `GDK.privacy` | `XblPrivacyCheckPermissionAsync`, `XblPrivacyCheckPermissionResultSize`, `XblPrivacyCheckPermissionResult`, `XblPrivacyCheckPermissionForAnonymousUserAsync`, `XblPrivacyCheckPermissionForAnonymousUserResultSize`, `XblPrivacyCheckPermissionForAnonymousUserResult`, `XblPrivacyBatchCheckPermissionAsync`, `XblPrivacyBatchCheckPermissionResultSize`, `XblPrivacyBatchCheckPermissionResult`, `XblPrivacyGetAvoidListAsync`, `XblPrivacyGetAvoidListResultCount`, `XblPrivacyGetAvoidListResult`, `XblPrivacyGetMuteListAsync`, `XblPrivacyGetMuteListResultCount`, `XblPrivacyGetMuteListResult` | Permission checks against XUIDs or anonymous user types, batch permission checks, and avoid/mute list reads. |
| `GDK.presence` | `XblPresenceSetPresenceAsync`, `XblPresenceGetPresenceAsync`, `XblPresenceGetPresenceResult`, `XblPresenceGetPresenceForMultipleUsersAsync`, `XblPresenceGetPresenceForMultipleUsersResultCount`, `XblPresenceGetPresenceForMultipleUsersResult`, `XblPresenceGetPresenceForSocialGroupAsync`, `XblPresenceGetPresenceForSocialGroupResultCount`, `XblPresenceGetPresenceForSocialGroupResult`, `XblPresenceAddDevicePresenceChangedHandler`, `XblPresenceRemoveDevicePresenceChangedHandler`, `XblPresenceAddTitlePresenceChangedHandler`, `XblPresenceRemoveTitlePresenceChangedHandler`, `XblPresenceTrackUsers`, `XblPresenceStopTrackingUsers`, `XblPresenceTrackAdditionalTitles`, `XblPresenceStopTrackingAdditionalTitles`, `XblPresenceRecordGetXuid`, `XblPresenceRecordGetUserState`, `XblPresenceRecordGetDeviceRecords`, `XblPresenceRecordCloseHandle` | Set/clear local presence, query presence records, track change notifications, and cache translated records. |
| `GDK.social` | Social Manager APIs including `XblSocialManagerAddLocalUser`, `XblSocialManagerRemoveLocalUser`, `XblSocialManagerDoWork`, `XblSocialManagerCreateSocialUserGroupFromFilters`, `XblSocialManagerCreateSocialUserGroupFromList`, `XblSocialManagerDestroySocialUserGroup`, and `XblSocialManagerUserGroupGetUsers`; reputation APIs `XblSocialSubmitReputationFeedbackAsync` and `XblSocialSubmitBatchReputationFeedbackAsync` | Direct relationship REST-style wrappers are intentionally not exposed. |
| `GDK.profile` | `XblProfileGetUserProfileAsync`, `XblProfileGetUserProfileResult`, `XblProfileGetUserProfilesAsync`, `XblProfileGetUserProfilesResultCount`, `XblProfileGetUserProfilesResult`, `XblProfileGetUserProfilesForSocialGroupAsync`, `XblProfileGetUserProfilesForSocialGroupResultCount`, `XblProfileGetUserProfilesForSocialGroupResult` | Query one profile, multiple profiles by XUID list, or profiles for a named social group. |
| `GDK.string_verify` | `XblStringVerifyStringAsync`, `XblStringVerifyStringResultSize`, `XblStringVerifyStringResult`, `XblStringVerifyStringsAsync`, `XblStringVerifyStringsResultSize`, `XblStringVerifyStringsResult` | Verify one string or a batch of strings and return per-string result dictionaries. |
| `GDK.title_storage` | `XblTitleStorageGetQuotaAsync`, `XblTitleStorageGetQuotaResult`, `XblTitleStorageGetBlobMetadataAsync`, `XblTitleStorageGetBlobMetadataResult`, `XblTitleStorageBlobMetadataResultGetItems`, `XblTitleStorageBlobMetadataResultHasNext`, `XblTitleStorageBlobMetadataResultGetNextAsync`, `XblTitleStorageBlobMetadataResultGetNextResult`, `XblTitleStorageBlobMetadataResultDuplicateHandle`, `XblTitleStorageBlobMetadataResultCloseHandle`, `XblTitleStorageDownloadBlobAsync`, `XblTitleStorageDownloadBlobResult`, `XblTitleStorageUploadBlobAsync`, `XblTitleStorageUploadBlobResult`, `XblTitleStorageDeleteBlobAsync` | Query quota, list paged metadata, download blobs via metadata discovery, upload bytes, and delete blobs. |
| `GDK.multiplayer_activity` | `XblMultiplayerActivityUpdateRecentPlayers`, `XblMultiplayerActivityFlushRecentPlayersAsync`, `XblMultiplayerActivitySetActivityAsync`, `XblMultiplayerActivityGetActivityAsync`, `XblMultiplayerActivityGetActivityResultSize`, `XblMultiplayerActivityGetActivityResult`, `XblMultiplayerActivityDeleteActivityAsync`, `XblMultiplayerActivitySendInvitesAsync`, `XblMultiplayerActivityAddInviteHandler`, `XblMultiplayerActivityRemoveInviteHandler` | This is the only multiplayer-related Xbox Services surface. |

#### Accepted Xbox Services wrappers

| Planned public service | Public wrapper scope | Native Xbox Services APIs to wrap |
| --- | --- | --- |

#### Explicit no-wrap Xbox Services APIs

Do not expose wrappers for:

- `matchmaking_c.h`
- `multiplayer_c.h`
- `multiplayer_manager_c.h`
- MPSD, multiplayer sessions, lobby/session transport APIs
- `game_invite_c.h` legacy/deprecated invite APIs
- `events_c.h` telemetry/configuration APIs
- generic public `notification_c.h` subscription wrappers; use notification/RTA plumbing internally only if a wrapped service requires it
- `XblPrivacyAddMuteListChangedHandler`, `XblPrivacyRemoveMuteListChangedHandler`, `XblPrivacyAddBlockListChangedHandler`, and `XblPrivacyRemoveBlockListChangedHandler` while the addon links against `Microsoft.Xbox.Services.C.Thunks`; these header-declared APIs are not exported by the public thunk libraries
- deprecated social, presence, and statistics subscription APIs
- direct `XblSocialGetSocialRelationshipsAsync` relationship paging; the public social graph remains Social Manager-backed

## Rationale and prior art

This spec borrows the Godot-facing integration patterns that already work well in prior art and then reshapes them around the actual lifecycle of GDK. The main prior-art reference is [GodotSteam](https://godotsteam.com/) and its active source tree on [Codeberg](https://codeberg.org/godotsteam/godotsteam), which demonstrates the value of native singletons, project settings, callback dispatch, and optional Godot-native adapters in a platform plugin.

### Why GDScript-first wrappers instead of raw native handles

Godot's native async/event style is built around first-class [signals](https://docs.godotengine.org/en/stable/classes/class_signal.html), [`await`](https://docs.godotengine.org/en/stable/tutorials/scripting/gdscript/gdscript_basics.html#awaiting-signals-or-coroutines), and [`RefCounted`](https://docs.godotengine.org/en/stable/classes/class_refcounted.html) objects. Exposing raw native handles like `XUserHandle`, `XAsyncBlock`, or queue handles directly to GDScript would fight both Godot ergonomics and Godot lifetime rules.

Wrapping native state in Godot objects such as `GDKUser`, `GDKLeaderboard`, and `GDKTitleStorageBlobMetadata`, plus exposing one-shot work through completion signals or op objects where handles are still needed, makes the API fit normal GDScript usage patterns like signal connections and `await GDK.users.add_default_user_async()`. The common Godot pattern `await get_tree().create_timer(...).timeout` ([SceneTreeTimer](https://docs.godotengine.org/en/stable/classes/class_scenetreetimer.html), [SceneTree.create_timer](https://docs.godotengine.org/en/stable/classes/class_scenetree.html#class-scenetree-method-create-timer)) is the bar this API should feel native next to.

### Why service namespaces instead of a flat root

Microsoft documents stats/leaderboards, privacy, presence, social graph, profile, string verification, and Title Storage as distinct Xbox Services systems ([Stats and Leaderboards](https://learn.microsoft.com/en-us/gaming/gdk/docs/services/player-data/stats-leaderboards/live-stats-leaderboards-nav?view=gdk-2510), [Presence overview](https://learn.microsoft.com/en-us/gaming/gdk/docs/services/community/presence/live-presence-overview?view=gdk-2604), [Social Manager overview](https://learn.microsoft.com/en-us/gaming/gdk/docs/services/community/social-manager/live-social-manager-overview?view=gdk-2604)).

Mirroring that separation in the public API makes partial initialization, documentation, testing, and feature flags clearer. The `GDK` root singleton still gives the convenience of one entry point, but the Xbox Services surface is partitioned into `GDK.achievements`, `GDK.stats`, `GDK.leaderboards`, `GDK.privacy`, `GDK.presence`, `GDK.social`, `GDK.profile`, `GDK.string_verify`, and `GDK.title_storage`.

### Why main-thread dispatch is part of the public contract

GDScript async flows are signal-centric ([await](https://docs.godotengine.org/en/stable/tutorials/scripting/gdscript/gdscript_basics.html#awaiting-signals-or-coroutines), [Using Signals](https://docs.godotengine.org/en/stable/getting_started/step_by_step/signals.html), [Signal](https://docs.godotengine.org/en/stable/classes/class_signal.html)). GDK async flows are task-queue-centric and explicitly separate work and completion ports ([XTaskQueue overview](https://learn.microsoft.com/en-us/gaming/gdk/docs/features/common/async/async-libraries/async-library-xtaskqueue?view=gdk-2604)).

The spec therefore uses background native work plus a main-thread `GDK.dispatch()` that converts results into Godot types, updates caches, and then emits service signals followed by one-shot completion signals or op completion. That keeps all Godot-visible state changes on the main thread and makes direct `await` feel like normal GDScript.

This mirrors the same callback-dispatch integration idea used by GodotSteam, adapted to `XTaskQueue`.

### Why caches and service signals exist beside one-shot ops

Not all Xbox-facing systems are one-shot request/response flows. Presence and the social graph are ongoing state feeds ([Presence overview](https://learn.microsoft.com/en-us/gaming/gdk/docs/services/community/presence/live-presence-overview?view=gdk-2604), [Social Manager overview](https://learn.microsoft.com/en-us/gaming/gdk/docs/services/community/social-manager/live-social-manager-overview?view=gdk-2604)).

Godot's observer model is signal-based ([Using Signals](https://docs.godotengine.org/en/stable/getting_started/step_by_step/signals.html)). So the spec uses direct-await completion signals for one-shot requests and service-owned caches and signals for long-lived state. That split matches both the platform behavior and Godot's scripting model.

## Public API conventions

### Global singleton

- `GDK`

### Wrapper types exposed to GDScript

| Native concept | GDScript wrapper |
| --- | --- |
| one-shot async request | `Signal` |
| `HRESULT` + payload | `GDKResult` |
| `XUserHandle` | `GDKUser` |
| package mount lifetime | `GDKPackageMount` |
| loaded resource-pack metadata | `GDKPackageResourcePack` |
| Stats and leaderboard payloads | stats `Dictionary`, `GDKLeaderboard`, `GDKLeaderboardColumn`, `GDKLeaderboardRow` |
| Privacy check payloads | `Dictionary` |
| Presence payloads | `GDKPresenceRecord` |
| Social graph payloads | `GDKSocialUser`, `GDKSocialGroup` |
| process-wide `XAppCaptureMetadata*` calls | `GDKCaptureMetaData` |
| Profile payloads | `GDKUserProfile` |
| Title Storage payloads | `GDKTitleStorageBlobMetadata`, `GDKTitleStorageBlobMetadataResult` |

### General rules

1. Public methods use snake_case and Godot-native types.
2. One-shot async APIs return a completion `Signal`. Long-lived systems expose service signals and caches.
3. GDScript-facing values stay within Godot's type system: `bool`, `int`, `float`, `String`, `Dictionary`, `Array`, and `PackedByteArray`.
4. Long-lived script objects use `RefCounted`, `Resource`, or `Node` when lifecycle matters.
5. Public terminology stays at the gameplay/service level: containers/files, stat names/values, leaderboards/entries, and groups/users.
6. Raw handles, pointers, and native query structs stay internal to C++.

## Plugin spec

### Async model

The async model should behave like a Godot-native future/promise layer over GDK's queue-based async APIs. The key rule is:

> **Normative rule:** Every one-shot GDK request must return a completion `Signal`, and completion only becomes visible to GDScript after main-thread dispatch.

#### GDK vs Godot model delta

The implementation has to bridge two different ownership and threading models. GDK async APIs are built around caller-owned `XAsyncBlock` state, queue-driven callback dispatch, `HRESULT` status, and per-API result extraction functions. Godot script APIs are built around `Object` / `RefCounted` lifetime, signal-driven `await`, main-thread-visible object mutation, and stable Variant-friendly payloads.

The wrapper layer should therefore normalize the following mismatches:

| Concern | GDK assumption | Godot assumption | Wrapper rule |
| --- | --- | --- | --- |
| Ownership | The caller owns the `XAsyncBlock`, callback context, and any result buffers needed by `*Result()` functions. | Script code should never manage raw native handles or callback memory. | Native async state must stay inside extension-owned helper objects. |
| Completion | Completion may be observed by `XAsync` callback/result extraction or by manager/event state during dispatch. | Completion should be a signal that can be `await`ed. | Every one-shot wrapper returns a completion signal that emits the final result exactly once. |
| Threading | Work and completion threads are chosen by `XTaskQueue` ports and may differ. | Godot-visible object creation, cache mutation, and signal emission should happen on the main thread. | Use a shared queue with background work and manual completion dispatch, then finalize on `GDK.dispatch()`. |
| Results | Success/failure lives in `HRESULT` plus API-specific payload structs. | Script code expects one stable result shape with Godot-native data. | Convert native status into a reusable `GDKResult` carrying `ok`, `hresult`, `code`, `message`, and `data`. |
| Cancellation | `XAsyncCancel` is best effort and late completion is still legal. | Completion should resolve at most once even when shutdown or cancellation races native work. | Track cancel state in the internal pending request and ignore or normalize late completions deterministically. |
| Long-lived updates | Notifications and manager feeds may never finish. | Ongoing state is modeled as caches plus signals. | Use one-shot completion signals only for bounded request/response waits; long-lived systems update service-owned caches and emit service signals. |

#### Internal async bridge architecture

The public one-shot completion-signal contract needs a reusable internal bridge layer rather than per-service ad hoc callback code. The internal architecture should consist of:

1. **Shared runtime owner**
   - Owns the shared `XTaskQueue`.
   - Configures the queue for background work and manual completion dispatch.
   - Retains active one-shot request references so fire-and-forget script calls stay alive until completion.
   - Coordinates shutdown and queue termination.

2. **Script-facing async wrapper**
   - One-shot public methods return a completion `Signal`.
   - `GDKResult` is the only public success/error payload shape.
   - Immediate failures and sync-adapted helpers still complete through the same completion-signal surface.

3. **Internal `XAsync` bridge context**
   - Owns one `XAsyncBlock` and any per-call native context.
   - Stores the owning service/runtime references needed to finalize the operation.
   - Provides the callback thunk that transitions from native completion into Godot-facing finalization.
   - Implements best-effort cancellation through `XAsyncCancel` when available.

4. **Service finalization hook**
   - Extract native result data from the completed `XAsyncBlock`.
   - Translate native payloads into Godot wrapper objects or Variant-friendly data.
   - Update the owning service cache before exposing completion to GDScript.
   - Emit service signals before the completion signal resolves.

#### Required `XAsync` lifecycle

Every one-shot wrapper should follow this lifecycle:

1. Allocate an internal pending request.
2. For `XAsync`-backed work, allocate a bridge context that owns the `XAsyncBlock` and any native per-call state; for manager/event-driven waits, register the pending request in service-owned state.
3. Start the native async API or activate the required manager/user registration.
4. Let `GDK.dispatch()` pump the completion queue and any manager/event feed.
5. Extract native result data and refresh service caches.
6. Emit service signals.
7. Resolve the returned completion signal.
8. Release retained native/context/request state.

#### Immediate and sync-adapted operations

Not every public wrapper maps to a documented native async API. Some operations will adapt synchronous native calls into the same completion-signal shape so GDScript still sees one consistent async contract. Those operations should:

- return a completion `Signal` even when the result is already known
- resolve the signal with an already-finalized `GDKResult`
- preserve the same cache-before-completion ordering guarantees as native async calls

#### Shutdown behavior

Shutdown must be queue-safe and op-safe:

- active one-shot requests should be retained by the runtime until they reach a terminal state
- queue termination should surface cancellation as failed `GDKResult` values rather than silently dropping callbacks
- no service cache or Godot object should be mutated after the runtime starts teardown
- `GDK.shutdown()` should clean up services first, then terminate and close the shared queue, then uninitialize the GDK runtime

#### Core behavior

1. **One shared async runtime**
   - `GDK.initialize()` creates the GDK runtime and a shared `XTaskQueue`.
   - The queue should use a worker port for background work and a manual completion port.
   - `GDK.dispatch()` dispatches the completion port. If `embed_dispatch` is enabled, the extension should do this automatically each frame from Godot's main thread.

2. **One completion request per one-shot API**
   - Calls like `query_user_stats_async()` or `open_container_async()` create one internal pending request and return its completion `Signal`.
   - Manager-backed waits like `query_player_achievements_async()` follow the same public contract even though the service-owned state is different internally.
   - Each request owns or is paired with the native state needed for its completion model.
   - Services keep strong references to active requests until they complete so GDScript can safely fire-and-forget.

3. **Native work happens off-thread; Godot work happens on-thread**
   - GDK does its work through the shared queue in the background.
   - Completion callbacks must not mutate Godot objects from worker threads.
   - Completion is only finalized when `GDK.dispatch()` runs on the Godot main thread.

4. **Strict completion ordering**
   - Convert native payloads into Godot-friendly types.
   - Update the owning service cache (`GDK.stats`, `GDK.presence`, etc.).
   - Emit service-level signals like `stats_updated()` or `presence_changed()`.
   - Resolve the returned completion signal with the final `GDKResult`.

By the time the completion signal resolves, the relevant service cache should already be current.

#### Completion signal

```gdscript
await some_service.some_method_async() # -> GDKResult
```

#### `GDKResult`

```gdscript
ok: bool
hresult: int
code: String
message: String
data: Variant
```

#### Dispatch contract

- No Godot objects should be created or signaled from worker threads.
- All async completions depend on `GDK.dispatch()` running.
- If `embed_dispatch` is disabled and the game never calls `dispatch()`, `await` on completion signals will hang.
- Cancellation is best effort: use native cancellation when available; otherwise mark the pending request cancelled and ignore late completions.
- Even immediate failures should still return an already-completed one-shot completion signal of the appropriate type.

#### Async patterns

| Pattern | Used for | Public surface |
| --- | --- | --- |
| one-shot request/response or manager-backed wait | sign-in, achievement cache warm-up/updates, leaderboard queries, privacy checks, profile lookups, title-storage blob operations | completion `Signal` |
| Long-lived background state | social graph updates, presence changes, tracked stat change notifications | service signals + cached state |

#### Examples

#### Example: one-shot request with `await`

```gdscript
var init_result := GDK.initialize()
if not init_result.ok:
    push_error(init_result.message)
    return

var result: GDKResult = await GDK.users.add_default_user_async()
if result.ok:
    var user: GDKUser = result.data
```

#### Example: service cache is current before `completed`

```gdscript
GDK.stats.stats_updated.connect(_on_stats_updated)

var result: GDKResult = await GDK.stats.query_user_stats_async(user, PackedStringArray(["xp", "wins"]))
if result.ok:
    # Safe to read the cache here; service state should already be updated.
    print(GDK.stats.get_cached_stats(user))

func _on_stats_updated(updated_user: GDKUser, stats: Dictionary) -> void:
    print("Stats cache updated before op completion is observed")
```

#### Example: manual dispatch when `embed_dispatch` is disabled

```gdscript
func _process(_delta: float) -> void:
    if GDK.is_initialized():
        GDK.dispatch()
```

#### Example: fire-and-forget async operation

```gdscript
GDK.stats.query_user_stats_async(user, PackedStringArray(["xp"])).connect(func(result: GDKResult) -> void:
    if not result.ok:
        push_error(result.message)
)
```

#### Example: long-lived background state

```gdscript
var start_result := GDK.social.start_social_graph(user)
if start_result.ok:
    GDK.social.social_group_updated.connect(_on_social_group_updated)

func _on_social_group_updated(group: GDKSocialGroup) -> void:
    var users := GDK.social.get_group_users(group)
    print("Group now has %d users" % users.size())
```

### Root singleton

#### Root API

```gdscript
GDK.initialize(config: GDKConfig = null) -> GDKResult
GDK.shutdown() -> void
GDK.is_available() -> bool
GDK.is_initialized() -> bool
GDK.dispatch() -> int
GDK.get_last_error() -> GDKResult
GDK.get_capture() -> GDKCapture
GDK.get_system() -> GDKSystem
```

#### Root properties

```gdscript
GDK.users: GDKUsers
GDK.game_ui: GDKGameUI
GDK.system: GDKSystem
GDK.accessibility: GDKAccessibility
GDK.achievements: GDKAchievements
GDK.package: GDKPackage
GDK.stats: GDKStats
GDK.leaderboards: GDKLeaderboards
GDK.privacy: GDKPrivacy
GDK.presence: GDKPresence
GDK.social: GDKSocial
GDK.profile: GDKProfile
GDK.string_verify: GDKStringVerify
GDK.title_storage: GDKTitleStorage
GDK.error_reporting: GDKErrorReporting
GDK.launcher: GDKLauncher
GDK.capture: GDKCapture
GDK.multiplayer_activity: GDKMultiplayerActivity
```

#### Root signals

```gdscript
initialized()
shutdown_completed()
runtime_error(result: GDKResult)
availability_changed(available: bool)
```

#### Runtime behavior

- `initialize()` sets up the GDK runtime and the shared `XTaskQueue`.
- `dispatch()` manually dispatches pending completions when automatic dispatch is disabled or when deterministic control is needed.
- `gdk/runtime/embed_dispatch` defaults to `true` and enables automatic per-frame dispatch from the main thread.

#### Native runtime mapping

| Public surface | Native API(s) | Notes |
| --- | --- | --- |
| `GDK.initialize()` | `XGameRuntimeInitialize`, `XTaskQueueCreate` | Creates the shared task queue and runtime bootstrap state used by all one-shot completion signals and service-owned callback bridges. |
| `GDK.shutdown()` | `XTaskQueueTerminate`, `XTaskQueueCloseHandle`, `XGameRuntimeUninitialize` | Service and user cleanup should run first; queue/runtime teardown happens last. |
| `GDK.dispatch()` | `XTaskQueueDispatch`, `XblAchievementsManagerDoWork`, `XblSocialManagerDoWork`, `XErrorSetCallback` callback-drain bridge | Main-thread pump. Dispatch the completion port, translate native payloads into Godot objects, update caches, then emit signals. |
| `GDK.launcher.launch_uri()` | `XLaunchUri` (`XLauncher.h`, `xgameruntime.lib`) | PC-supported URI launcher surface for app-to-app, Store, and Settings destinations. |
| per-user Xbox services context | `XblContextCreateHandle`, `XblContextCloseHandle` | Create once for each admitted `GDKUser`; store inside the wrapper for achievements, stats, leaderboards, presence, and social calls. |

Every one-shot async wrapper should allocate an `XAsyncBlock` against the shared queue and complete it only after the Godot-side cache and wrapper state are current.

### Service specifications

Unless otherwise noted, the service sections below follow the global naming, type, and terminology rules defined in **Public API conventions**.

#### `GDK.launcher` service

##### Methods

```gdscript
launch_uri(uri: String, user: GDKUser = null) -> GDKResult
```

##### Validation contract

- `launch_uri` rejects blank/malformed input with `invalid_uri`.
- Unsupported URI destinations reject with `unsupported_launcher_destination`.
- Optional `user` must be a signed-in `GDKUser` when provided (`invalid_user`).

#### `GDK.users` service

##### Methods

```gdscript
add_default_user_async() -> Signal
add_user_with_ui_async() -> Signal
get_primary_user() -> GDKUser
get_users() -> Array[GDKUser]
check_privilege_async(user: GDKUser, privilege: int) -> Signal
resolve_privilege_with_ui_async(user: GDKUser, privilege: int) -> Signal
resolve_issue_with_ui_async(user: GDKUser, url := "") -> Signal
get_gamer_picture_async(user: GDKUser, size := "medium") -> Signal
get_token_and_signature_async(user: GDKUser, method: String, url: String, headers := {}, body := PackedByteArray(), force_refresh := false) -> Signal
```

##### Signals

```gdscript
user_changed(user: GDKUser, change_kind: String)
```

##### `GDKUser`

```gdscript
get_local_id() -> int
get_xuid() -> String
get_gamertag() -> String
get_age_group() -> GDKUser.AgeGroup
get_age_group_name() -> String
get_sign_in_state() -> GDKUser.SignInState
get_sign_in_state_name() -> String
is_guest() -> bool
is_signed_in() -> bool
is_store_user() -> bool
```

##### Native API mapping

| Wrapper/API | Native API(s) | Notes |
| --- | --- | --- |
| `add_default_user_async()` | `XUserAddAsync`, `XUserAddResult` | Uses the silent default-user path without guest support; on success, populate `GDKUser`, create `XblContextHandle`, and ensure change notifications are registered. |
| `add_user_with_ui_async()` | `XUserAddAsync`, `XUserAddResult` | Uses the UI-driven add path with guest selection enabled; post-processing is the same as the default-user path except that later adds do not replace the session primary user once one already exists. |
| `check_privilege_async()` | `XUserCheckPrivilege` | There is no documented async privilege-check API in `XUser`; this wrapper should convert the synchronous result into a deferred completion `Signal`. Successful results carry a `Dictionary` in `GDKResult.data` with `privilege`, `has_privilege`, `deny_reason`, and `deny_reason_value`. If the check returns `E_GAMEUSER_RESOLVE_USER_ISSUE_REQUIRED`, the request should fail and direct callers to `resolve_issue_with_ui_async()`. |
| `resolve_privilege_with_ui_async()` | `XUserResolvePrivilegeWithUiAsync`, `XUserResolvePrivilegeWithUiResult` | Use the native UI remediation path when `check_privilege_async()` reports that a privilege is denied and the title wants to let the player resolve it immediately. Successful results can carry a small `Dictionary` payload that echoes the resolved privilege id. |
| `resolve_issue_with_ui_async()` | `XUserResolveIssueWithUiAsync`, `XUserResolveIssueWithUiResult` | This is the remediation path for `E_GAMEUSER_RESOLVE_USER_ISSUE_REQUIRED` from `XUser` getters such as age-group lookups and from privilege checks. Treat an empty `url` as the Xbox services/default flow and pass a URL only when the underlying issue is request-specific. |
| `get_gamer_picture_async()` | `XUserGetGamerPictureAsync`, `XUserGetGamerPictureResultSize`, `XUserGetGamerPictureResult` | Accept `small`, `medium`, `large`, and `extra_large` size strings. Decode the returned PNG bytes into a Godot `Image` and place that `Image` in `GDKResult.data`. |
| `get_token_and_signature_async()` | `XUserGetTokenAndSignatureAsync`, `XUserGetTokenAndSignatureResultSize`, `XUserGetTokenAndSignatureResult` | First-pass token support should expose explicit request parameters: HTTP method, full URL, headers `Dictionary`, optional `PackedByteArray` body, and `force_refresh`. Successful results carry a `Dictionary` with `token` and `signature`. |
| `user_changed` | `XUserRegisterForChangeEvent`, `XUserUnregisterForChangeEvent` | Register one runtime-wide change callback against the shared queue and reconcile affected `GDKUser` wrappers by local id. `user_changed` is the only public users-service event and should emit the affected wrapper plus a snake_case `change_kind` string: `added`, `removed`, `signed_in_again`, `gamertag`, `gamer_picture`, or `privileges`. For `removed`, emit the removed wrapper after it has been removed from the users cache so handlers can read identity fields without seeing it in `get_users()`. |
| `GDKUser` getters | `XUserGetLocalId`, `XUserGetId`, `XUserGetGamertag`, `XUserGetAgeGroup`, `XUserGetIsGuest`, `XUserGetState`, `XUserIsStoreUser` | Pure wrapper accessors with no extra service traffic. Expose age-group and sign-in state as Godot enums with bound constants on `GDKUser`, and provide `get_age_group_name()` / `get_sign_in_state_name()` for human-readable snake_case strings such as `adult` and `signed_in`. |

Each `GDKUser` should own an `XUserHandle` and an `XblContextHandle`. The runtime should own a single change-event registration token. Cleanup order should be: unregister runtime change notifications, remove the user from service-owned caches/managers, close the Xbox services context, then call `XUserCloseHandle`. The first successful user add establishes the session primary user; later adds should not promote a different cached user to primary.

#### `GDK.accessibility` service

##### Methods

```gdscript
query_closed_caption_properties() -> GDKResult
set_closed_caption_enabled(enabled: bool) -> GDKResult
query_high_contrast_mode() -> GDKResult
```

##### Notes

- Scope is intentionally limited to concrete APIs verified in public PC GDK docs/headers for `_GAMING_DESKTOP`.
- Do not add unrelated families in this service (`XGameStreaming`, `XPersistentLocalStorage`, `XNetworking`, console-only `XAppCapture`).
- Speech-to-text overlay APIs are deferred for manual/UI-focused follow-up coverage.

##### Native API mapping

| Wrapper/API | Native API(s) | Notes |
| --- | --- | --- |
| `query_closed_caption_properties()` | `XClosedCaptionGetProperties` | Returns a `GDKClosedCaptionProperties` wrapper in `GDKResult.data`. |
| `set_closed_caption_enabled()` | `XClosedCaptionSetEnabled` | Returns an ok/error `GDKResult`; success payload includes `enabled`. |
| `query_high_contrast_mode()` | `XHighContrastGetMode` | Returns a `Dictionary` payload with `mode` and `mode_name`. |
| `GDKClosedCaptionProperties` getters | `XClosedCaptionProperties` struct fields | Wrapper exposes Godot-native colors/enums/flags without exposing native handles. |

#### `GDK.game_ui` service

##### Methods

```gdscript
show_message_dialog_async(title: String, message: String, first_button := "OK", second_button := "", third_button := "", default_button := "first", cancel_button := "first") -> Signal
set_notification_position_hint(position: String) -> GDKResult
show_player_profile_card_async(requesting_user: GDKUser, target_xuid: String) -> Signal
show_player_picker_async(requesting_user: GDKUser, prompt: String, selectable_xuids: PackedStringArray, preselected_xuids := PackedStringArray(), min_selection_count := 1, max_selection_count := 1) -> Signal
resolve_privilege_with_ui_async(user: GDKUser, privilege: int) -> Signal
```

##### Notes

- This service should expose only APIs verified as available in the public PC GDK (`_GAMING_DESKTOP`) headers/libs used by this repo.
- Do not add wrappers for console-only or unavailable surfaces (for example `XGameStreaming`, `XPersistentLocalStorage`, `XNetworking`, or console-only `XAppCapture` flows).
- `show_message_dialog_async()` and `show_player_picker_async()` should distinguish user-cancelled flows (`E_ABORT`) from other native failures.
- Keep `GDK.multiplayer_activity.show_invite_ui_async()` compatible; that API remains callable through the multiplayer-activity service.

##### Native API mapping

| Wrapper/API | Native API(s) | Notes |
| --- | --- | --- |
| `show_message_dialog_async()` | `XGameUiShowMessageDialogAsync`, `XGameUiShowMessageDialogResult` | Validate title/message/button layout before invoking native UI. Return button selection in `GDKResult.data` on success. |
| `set_notification_position_hint()` | `XGameUiSetNotificationPositionHint` | Accept snake_case positions (`bottom_center`, etc.). This is available on PC GDK even where the shell may ignore placement hints. |
| `show_player_profile_card_async()` | `XGameUiShowPlayerProfileCardAsync`, `XGameUiShowPlayerProfileCardResult` | Requires a signed-in `GDKUser` requesting handle and numeric target XUID. |
| `show_player_picker_async()` | `XGameUiShowPlayerPickerAsync`, `XGameUiShowPlayerPickerResultCount`, `XGameUiShowPlayerPickerResult` | Validate XUID lists and selection ranges up front; return selected XUIDs in `GDKResult.data`. |
| `resolve_privilege_with_ui_async()` | `XUserResolvePrivilegeWithUiAsync`, `XUserResolvePrivilegeWithUiResult` | Delegate to `GDK.users` privilege-remediation flow so existing users-service behavior remains authoritative. |

#### `GDK.achievements` service

##### Methods

```gdscript
query_player_achievements_async(user: GDKUser) -> Signal
update_achievement_async(user: GDKUser, achievement_id: String, percent_complete: int) -> Signal
get_cached_achievements(user: GDKUser) -> Array
```

##### Signals

```gdscript
achievement_unlocked(user: GDKUser, achievement_id: String)
achievements_updated(user: GDKUser)
```

##### Notes

- Keep public API achievement-centered.
- Stats, leaderboards, presence, and social should stay separate services instead of being folded into achievements.
- Use Achievements Manager as the authoritative v1 implementation rather than building a separate ad hoc achievement cache.
- Use completion `Signal` for manager/event-driven one-shot waits so callers can `await` the method directly while the service handles pending-state cleanup internally.
- For GDK Game OS titles, derive the default current-title SCID from `XGameGetXboxTitleId()` as a null GUID with the Title ID in the last 8 hex digits. Only require explicit SCID overrides for advanced cross-title scenarios.

##### Native API mapping

| Wrapper/API | Native API(s) | Notes |
| --- | --- | --- |
| user lifecycle | `XblAchievementsManagerAddLocalUser`, `XblAchievementsManagerRemoveLocalUser` | Register each signed-in local user with the manager and remove them on sign-out/shutdown. |
| `query_player_achievements_async()` | `XblAchievementsManagerAddLocalUser`, `XblAchievementsManagerIsUserInitialized`, `XblAchievementsManagerDoWork`, `XblAchievementsManagerGetAchievements` | Treat this as a cache-warm operation: ensure the user is registered, wait until the manager reports the user initialized, then copy the cache into Godot objects before completing. |
| `update_achievement_async()` | `XblAchievementsManagerUpdateAchievement`, `XblAchievementsManagerDoWork` | Resolve the returned completion signal only after the manager reports the updated achievement state on the dispatch thread. |
| `get_cached_achievements()` | `XblAchievementsManagerGetAchievements` | Reads the current manager cache and translates it into Godot-facing achievement objects. |
| `achievement_unlocked` / `achievements_updated` | `XblAchievementsManagerDoWork` | These signals come from manager update events, not from a separate polling or REST-style query path. |

The manager result handles should be copied into extension-owned data immediately on dispatch, because they are cache views rather than long-lived script-safe objects.

#### `GDK.package` service

##### Methods

```gdscript
enumerate_packages(package_kind := GDKPackage.PACKAGE_KIND_CONTENT, scope := GDKPackage.ENUMERATION_SCOPE_THIS_AND_RELATED) -> GDKResult
find_package_by_identifier(package_identifier: String, package_kind := GDKPackage.PACKAGE_KIND_CONTENT, scope := GDKPackage.ENUMERATION_SCOPE_THIS_AND_RELATED) -> GDKResult
get_current_process_package_identifier() -> GDKResult
mount_package_async(package_identifier: String) -> Signal
load_resource_pack_async(package_identifier: String, pack_relative_path: String, replace_files := false, offset := 0) -> Signal
get_loaded_resource_packs() -> Array
get_install_progress(package_identifier: String) -> GDKResult
```

##### Notes

- `mount_package_async()` returns a `GDKPackageMount` for temporary loose-file access under an open mount handle.
- `load_resource_pack_async()` is the primary Godot-native DLC path; successful loads retain service-owned mounts because `ProjectSettings.load_resource_pack()` has no unload counterpart.
- Missing package identifiers return `package_not_found`. Invalid package-relative paths and unsupported pack extensions return explicit validation errors.

##### Native API mapping

| Wrapper/API | Native API(s) | Notes |
| --- | --- | --- |
| `enumerate_packages()` / `find_package_by_identifier()` | `XPackageEnumeratePackages`, `XPackageDetails` | Enumerates game/content packages and maps stable package dictionaries. |
| `get_current_process_package_identifier()` | `XPackageGetCurrentProcessPackageIdentifier` | Returns current process package identity for diagnostics and follow-up package operations. |
| `mount_package_async()` | `XPackageMountWithUiAsync`, `XPackageMountWithUiResult`, `XPackageGetMountPathSize`, `XPackageGetMountPath`, `XPackageCloseMountHandle` | Async mount flow integrated with the existing runtime queue/pending-signal model. |
| `load_resource_pack_async()` | same mount APIs + `ProjectSettings.load_resource_pack` | Mounts package content, resolves package-relative `.pck`/`.zip`, and loads it into `res://` with retained mount lifetime. |
| `get_install_progress()` | `XPackageCreateInstallationMonitor`, `XPackageGetInstallationProgress`, `XPackageCloseInstallationMonitorHandle` | Snapshots install progress for one package identifier. |

#### `GDK.stats` service

##### Methods

```gdscript
query_user_stats_async(user: GDKUser, stat_names := PackedStringArray()) -> Signal
query_users_stats_async(user: GDKUser, xuids: PackedStringArray, stat_names := PackedStringArray()) -> Signal
set_stat_number(user: GDKUser, stat_name: String, value: float) -> GDKResult
set_stat_integer(user: GDKUser, stat_name: String, value: int) -> GDKResult
flush_stats_async(user: GDKUser) -> Signal
track_stats(user: GDKUser, stat_names: PackedStringArray) -> GDKResult
stop_tracking_stats(user: GDKUser, stat_names := PackedStringArray()) -> GDKResult
get_cached_stats(user: GDKUser) -> Dictionary
```

##### Signals

```gdscript
stats_updated(user: GDKUser, stats: Dictionary)
stat_changed(user: GDKUser, stat_name: String, value: Variant)
stats_flushed(user: GDKUser, result: GDKResult)
```

##### Notes

- Use title-managed stats as the v1 write path.
- The `set_* + flush` shape is an extension-owned batching convenience, not a native GDK API shape.
- Use `user_statistics_c` for explicit reads and optional real-time change tracking.
- Statistic query methods require at least one statistic name because the wrapped XSAPI query families are explicit-name APIs.
- Leaderboards should consume published stats rather than having a separate score-submission path.

##### Native API mapping

| Wrapper/API | Native API(s) | Notes |
| --- | --- | --- |
| `query_user_stats_async()` | `XblUserStatisticsGetSingleUserStatisticsAsync`, `XblUserStatisticsGetSingleUserStatisticsResultSize`, `XblUserStatisticsGetSingleUserStatisticsResult` | Use explicit user-statistics reads for query-style fetches and cache refreshes. |
| `query_users_stats_async()` | `XblUserStatisticsGetMultipleUserStatisticsAsync`, `XblUserStatisticsGetMultipleUserStatisticsResultSize`, `XblUserStatisticsGetMultipleUserStatisticsResult` | Use one local user context to read the requested stats for a target XUID list. |
| `set_stat_number()` / `set_stat_integer()` | extension-local staging only | Convert staged values into `XblTitleManagedStatistic` payloads. No native call is made until `flush_stats_async()`. |
| `flush_stats_async()` | `XblTitleManagedStatsUpdateStatsAsync`, `XblTitleManagedStatistic`, `XblTitleManagedStatType` | Flushes staged values through the documented title-managed stats write API. If the wrapper later needs full-document replacement semantics, add a separate path that uses `XblTitleManagedStatsWriteAsync`. |
| `track_stats()` | `XblUserStatisticsAddStatisticChangedHandler`, `XblUserStatisticsTrackStatistics` | Register one change handler per local user context, then track the requested stats for that user's XUID. |
| `stop_tracking_stats()` | `XblUserStatisticsStopTrackingStatistics`, `XblUserStatisticsStopTrackingUsers`, `XblUserStatisticsRemoveStatisticChangedHandler` | Stop specific stat tracking or all tracked stat updates for the local user. |
| `get_cached_stats()` | service cache only | Cache ownership stays in the extension; it is hydrated by explicit reads and tracked-stat change callbacks. |
| `stats_updated` / `stat_changed` / `stats_flushed` | Service cache plus statistic-change handlers | Query and tracked-stat callbacks update the cache before completion/notification; title-managed write completion drives `stats_flushed`. |

Earlier undocumented stats-family references were removed during audit. The documented write family is `title_managed_statistics_c`; the documented read and tracking family is `user_statistics_c`.

#### `GDK.leaderboards` service

##### Methods

```gdscript
get_leaderboard_async(user: GDKUser, stat_name: String, max_items := 25) -> Signal
get_leaderboard_around_user_async(user: GDKUser, stat_name: String, max_items := 25) -> Signal
get_social_leaderboard_async(user: GDKUser, stat_name: String, max_items := 25) -> Signal
get_next_page_async(leaderboard: GDKLeaderboard) -> Signal
get_cached_leaderboard(stat_name: String) -> GDKLeaderboard
```

##### Signals

```gdscript
leaderboard_updated(stat_name: String, leaderboard: GDKLeaderboard)
```

##### Notes

- Social/friends leaderboard queries belong here rather than in `GDK.social`.
- This service is read-only. Leaderboard values are driven by published stats rather than a separate submission API.

##### Native API mapping

| Wrapper/API | Native API(s) | Notes |
| --- | --- | --- |
| `get_leaderboard_async()` | `XblLeaderboardGetLeaderboardAsync`, `XblLeaderboardGetLeaderboardResultSize`, `XblLeaderboardGetLeaderboardResult`, `XblLeaderboardQuery`, `XblLeaderboardQueryType` | Global leaderboard query uses `XblLeaderboardQueryType::TitleManagedStatBackedGlobal`. |
| `get_leaderboard_around_user_async()` | `XblLeaderboardGetLeaderboardAsync`, `XblLeaderboardQuery`, `XblLeaderboardQueryType` | Use `XblLeaderboardQuery.skipToXboxUserId` to center the result set around the local user. |
| `get_social_leaderboard_async()` | `XblLeaderboardGetLeaderboardAsync`, `XblLeaderboardQuery`, `XblLeaderboardQueryType` | Social leaderboard query uses `XblLeaderboardQueryType::TitleManagedStatBackedSocial`. |
| pagination inside `GDKLeaderboard` | `XblLeaderboardResultGetNextAsync`, `XblLeaderboardResultGetNextResultSize`, `XblLeaderboardResultGetNextResult` | Store continuation state in the wrapper so GDScript can request another page later without exposing native handles. |
| `get_cached_leaderboard()` | service cache only | Return the most recent translated leaderboard snapshot. |

#### `GDK.privacy` service

##### Methods

```gdscript
check_permission_async(user: GDKUser, permission: String, target_xuid: String) -> Signal
check_permission_for_anonymous_user_async(user: GDKUser, permission: String, anonymous_user_type: String) -> Signal
batch_check_permission_async(user: GDKUser, permission: String, target_xuids: PackedStringArray) -> Signal
get_avoid_list_async(user: GDKUser) -> Signal
get_mute_list_async(user: GDKUser) -> Signal
```

##### Notes

- Permission names are Godot-style strings that map to `XblPermission`.
- Anonymous user type names are `cross_network_user` and `cross_network_friend`.
- Permission results are returned as dictionaries. Batch results are returned as an array of those dictionaries.
- Avoid/mute list query results are returned as `PackedStringArray` XUID values.
- The `XblPrivacy*ListChangedHandler` APIs are intentionally not exposed while the addon uses the public XSAPI thunk libraries, because those handler symbols are header-declared but not exported by the thunk libs used for runtime deployment.

##### Native API mapping

| Wrapper/API | Native API(s) | Notes |
| --- | --- | --- |
| `check_permission_async()` | `XblPrivacyCheckPermissionAsync`, `XblPrivacyCheckPermissionResultSize`, `XblPrivacyCheckPermissionResult` | Checks one permission against one target XUID. |
| `check_permission_for_anonymous_user_async()` | `XblPrivacyCheckPermissionForAnonymousUserAsync`, `XblPrivacyCheckPermissionForAnonymousUserResultSize`, `XblPrivacyCheckPermissionForAnonymousUserResult` | Checks one permission against a supported anonymous user type. |
| `batch_check_permission_async()` | `XblPrivacyBatchCheckPermissionAsync`, `XblPrivacyBatchCheckPermissionResultSize`, `XblPrivacyBatchCheckPermissionResult` | Checks one permission against a list of target XUIDs. |
| `get_avoid_list_async()` | `XblPrivacyGetAvoidListAsync`, `XblPrivacyGetAvoidListResultCount`, `XblPrivacyGetAvoidListResult` | Returns the avoid-list XUIDs for the local user context. |
| `get_mute_list_async()` | `XblPrivacyGetMuteListAsync`, `XblPrivacyGetMuteListResultCount`, `XblPrivacyGetMuteListResult` | Returns the mute-list XUIDs for the local user context. |

#### `GDK.presence` service

##### Methods

```gdscript
set_presence_async(user: GDKUser, state: String, rich_presence := {}) -> Signal
clear_presence_async(user: GDKUser) -> Signal
get_presence_async(xuids: PackedStringArray) -> Signal
get_presence_for_social_group_async(user: GDKUser, social_group: String) -> Signal
track_presence(user: GDKUser, xuids: PackedStringArray, title_ids := PackedInt64Array()) -> GDKResult
stop_tracking_presence(user: GDKUser, xuids := PackedStringArray(), title_ids := PackedInt64Array()) -> GDKResult
get_cached_presence(xuid: String) -> GDKPresenceRecord
```

##### Signals

```gdscript
presence_changed(xuid: String, presence: GDKPresenceRecord)
local_presence_set(user: GDKUser)
device_presence_changed(xuid: String)
title_presence_changed(xuid: String, title_id: int)
```

##### Notes

- Rich presence payloads are supplied as dictionaries or lightweight wrapper objects.
- This service owns the local and remote presence cache.
- For multi-user reads, prefer the documented multiple-user presence query instead of issuing one native request per XUID.
- There is no separate documented `clear presence` function in `presence_c`; clearing should be modeled as setting the local user inactive with no rich presence payload.
- Device/title presence change callbacks are emitted from `GDK.dispatch()` after `track_presence()` configures `XblPresenceTrackUsers` and optional additional title tracking.

##### Native API mapping

| Wrapper/API | Native API(s) | Notes |
| --- | --- | --- |
| `set_presence_async()` | `XblPresenceSetPresenceAsync`, `XblPresenceRichPresenceIds` | Local presence write path. The wrapper should translate lightweight Godot presence input into the documented rich-presence id shape. |
| `clear_presence_async()` | `XblPresenceSetPresenceAsync` | Wrapper convenience only. Implement by setting the local user inactive and omitting rich presence, since no dedicated clear API is documented. |
| `get_presence_async()` | `XblPresenceGetPresenceAsync`, `XblPresenceGetPresenceResult`, `XblPresenceGetPresenceForMultipleUsersAsync`, `XblPresenceGetPresenceForMultipleUsersResultCount`, `XblPresenceGetPresenceForMultipleUsersResult` | Use the single-user or multiple-user path based on the XUID count, then update the cache before completing the returned signal. |
| `get_presence_for_social_group_async()` | `XblPresenceGetPresenceForSocialGroupAsync`, `XblPresenceGetPresenceForSocialGroupResultCount`, `XblPresenceGetPresenceForSocialGroupResult` | Query a named social group using a supplied local-user context, then update the cache before completing the returned signal. |
| `track_presence()` | `XblPresenceAddDevicePresenceChangedHandler`, `XblPresenceAddTitlePresenceChangedHandler`, `XblPresenceTrackUsers`, `XblPresenceTrackAdditionalTitles` | Registers one handler set per local user and starts device/title presence tracking. |
| `stop_tracking_presence()` | `XblPresenceStopTrackingUsers`, `XblPresenceStopTrackingAdditionalTitles`, `XblPresenceRemoveDevicePresenceChangedHandler`, `XblPresenceRemoveTitlePresenceChangedHandler` | Stops requested tracked users/titles; empty arrays stop all values tracked by this service for the local user. |
| `get_cached_presence()` | service cache only | Presence records are cached and owned by `GDK.presence`, even when the social layer is also active. |

#### `GDK.social` service

##### Methods

```gdscript
start_social_graph(user: GDKUser) -> GDKResult
stop_social_graph(user: GDKUser) -> void
get_friends_async(user: GDKUser) -> Signal
create_social_group(user: GDKUser, filter: GDKSocialFilter = null) -> GDKSocialGroup
create_social_group_from_xuids(user: GDKUser, xuids: PackedStringArray) -> GDKSocialGroup
destroy_social_group(group: GDKSocialGroup) -> void
get_group_users(group: GDKSocialGroup) -> Array[GDKSocialUser]
submit_reputation_feedback_async(user: GDKUser, target_xuid: String, feedback_type: String, reason := "", evidence_id := "") -> Signal
submit_batch_reputation_feedback_async(user: GDKUser, feedback_items: Array) -> Signal
```

##### Signals

```gdscript
social_graph_changed(user: GDKUser)
social_group_updated(group: GDKSocialGroup)
social_user_changed(xuid: String, social_user: GDKSocialUser)
```

##### Notes

- Mirrors Xbox social graph concepts, but exposes groups and users as Godot objects.
- Presence-backed friend filtering belongs in the social layer; actual presence payloads remain owned by `GDK.presence`.
- v1 social graph implementation should be Social Manager-backed rather than a separate friend-list fetch layer.
- Reputation feedback accepts `XblReputationFeedbackType` names in snake_case and intentionally omits MPSD session references from the Godot surface.

##### Native API mapping

| Wrapper/API | Native API(s) | Notes |
| --- | --- | --- |
| `start_social_graph()` | `XblSocialManagerAddLocalUser`, `XblSocialManagerDoWork` | Registers the local user with Social Manager and starts the ongoing social feed. |
| `stop_social_graph()` | `XblSocialManagerRemoveLocalUser` | Stops tracking the local user's social graph. |
| `get_friends_async()` | `XblSocialManagerCreateSocialUserGroupFromFilters`, `XblSocialManagerDoWork` | Treat this as a default friends-group bootstrap op that completes after the first group population event is observed. |
| `create_social_group()` | `XblSocialManagerCreateSocialUserGroupFromFilters` | Filter-backed social groups map directly to native filtered groups. |
| `create_social_group_from_xuids()` | `XblSocialManagerCreateSocialUserGroupFromList` | Fixed-list groups map directly to native list-backed groups. |
| `destroy_social_group()` | `XblSocialManagerDestroySocialUserGroup` | Releases the native social-group handle. |
| future mutable list groups | `XblSocialManagerUpdateSocialUserGroup` | Keep this internal until a public update-group API is justified. |
| `get_group_users()` | `XblSocialManagerUserGroupGetUsers` | Reads the current user list from the native social group handle. |
| `submit_reputation_feedback_async()` | `XblSocialSubmitReputationFeedbackAsync` | Submit one feedback item without exposing native MPSD session-reference structs. |
| `submit_batch_reputation_feedback_async()` | `XblSocialSubmitBatchReputationFeedbackAsync`, `XblReputationFeedbackItem` | Batch items are Godot dictionaries with `target_xuid`, `feedback_type`, optional `reason`, and optional `evidence_id`. |
| social signals | `XblSocialManagerDoWork` | Group membership and user changes should be driven from Social Manager events and mirrored into Godot caches. |

#### `GDK.capture` service

PC-supported subset of `XAppCapture`. Metadata and capture-state APIs only.
Console-only paths (`XAppCaptureOpenLocalStorageFiles`, `XAppCaptureDiagnosticClipLocalId`) are excluded.

##### PC GDK availability inventory

| Native function | Header | Import lib | Microsoft Learn | PC GDK (_GAMING_DESKTOP) |
| --- | --- | --- | --- | --- |
| `XAppCaptureEnableRecord` | `XAppCapture.h` | `xgameruntime.lib` | [XAppCaptureEnableRecord](https://learn.microsoft.com/gaming/gdk/docs/reference/system/xappcapture/functions/xappcaptureenablerecord) | YES |
| `XAppCaptureDisableRecord` | `XAppCapture.h` | `xgameruntime.lib` | [XAppCaptureDisableRecord](https://learn.microsoft.com/gaming/gdk/docs/reference/system/xappcapture/functions/xappcapturedisablerecord) | YES |
| `XAppCaptureRecordDiagnosticClip` | `XAppCapture.h` | `xgameruntime.lib` | [XAppCaptureRecordDiagnosticClip](https://learn.microsoft.com/gaming/gdk/docs/reference/system/xappcapture/functions/xappcapturerecorddiagnosticclip) | YES (Game Bar) |
| `XAppCaptureTakeDiagnosticScreenshot` | `XAppCapture.h` | `xgameruntime.lib` | [XAppCaptureTakeDiagnosticScreenshot](https://learn.microsoft.com/gaming/gdk/docs/reference/system/xappcapture/functions/xappcapturetakediagnosticscreenshot) | YES (Game Bar) |
| `XAppCaptureMetadataAddStringEvent` | `XAppCapture.h` | `xgameruntime.lib` | [XAppCaptureMetadataAddStringEvent](https://learn.microsoft.com/gaming/gdk/docs/reference/system/xappcapture/functions/xappcapturemetadataaddstringevent) | YES |
| `XAppCaptureMetadataAddDoubleEvent` | `XAppCapture.h` | `xgameruntime.lib` | (same namespace) | YES |
| `XAppCaptureMetadataAddInt32Event` | `XAppCapture.h` | `xgameruntime.lib` | (same namespace) | YES |
| `XAppCaptureMetadataStartStringState` | `XAppCapture.h` | `xgameruntime.lib` | [XAppCaptureMetadataStartStringState](https://learn.microsoft.com/gaming/gdk/docs/reference/system/xappcapture/functions/xappcapturemetadatastartstringstate) | YES |
| `XAppCaptureMetadataStartDoubleState` | `XAppCapture.h` | `xgameruntime.lib` | (same namespace) | YES |
| `XAppCaptureMetadataStartInt32State` | `XAppCapture.h` | `xgameruntime.lib` | (same namespace) | YES |
| `XAppCaptureMetadataStopAllStates` | `XAppCapture.h` | `xgameruntime.lib` | [XAppCaptureMetadataStopAllStates](https://learn.microsoft.com/gaming/gdk/docs/reference/system/xappcapture/functions/xappcapturemetadatastopallstates) | YES |
| `XAppCaptureMetadataRemainingStorageBytesAvailable` | `XAppCapture.h` | `xgameruntime.lib` | [XAppCaptureMetadataRemainingStorageBytesAvailable](https://learn.microsoft.com/gaming/gdk/docs/reference/system/xappcapture/functions/xappcapturemetadataremainingstoragebytesavailable) | YES |

Excluded (console-only):
- `XAppCaptureOpenLocalStorageFiles` / `XAppCaptureCloseLocalStorageFilesHandle` — console clip-file access
- `XAppCaptureDiagnosticClipLocalId` result extraction — console-specific clip identifier

##### Methods

```gdscript
enable_capture() -> GDKResult
disable_capture() -> GDKResult
record_diagnostic_clip_async(duration: float) -> Signal
take_diagnostic_screenshot_async(path_hint: String) -> Signal
create_metadata(reserved_bytes := 0) -> GDKCaptureMetaData
```

##### `GDKCaptureMetaData`

```gdscript
is_valid() -> bool
close() -> void
stop_all_states() -> GDKResult
get_remaining_storage_bytes() -> int
add_string_event(name: String, value: String, priority := PRIORITY_GAMEPLAY) -> GDKResult
add_double_event(name: String, value: float, priority := PRIORITY_GAMEPLAY) -> GDKResult
add_int32_event(name: String, value: int, priority := PRIORITY_GAMEPLAY) -> GDKResult
start_string_state(name: String, value: String, priority := PRIORITY_GAMEPLAY) -> GDKResult
start_double_state(name: String, value: float, priority := PRIORITY_GAMEPLAY) -> GDKResult
start_int32_state(name: String, value: int, priority := PRIORITY_GAMEPLAY) -> GDKResult
```

##### Notes

- `create_metadata()` returns `null` when the runtime is not initialized.
- PC GDK metadata APIs are process-wide/stateless; `GDKCaptureMetaData` is a script-side context that gates intentional writes rather than a native handle wrapper.
- All event/state methods validate the context (`invalid_metadata_handle`) and the name (`invalid_metadata_name`) before calling native APIs.
- `record_diagnostic_clip_async` and `take_diagnostic_screenshot_async` require Game Bar active on the device; they return `GDKResult` errors when unavailable.
- `on_runtime_initialized()` for capture has no side-effects beyond setting `m_runtime_ready`; it cannot fail.

##### Native API mapping

| Wrapper/API | Native API(s) | Notes |
| --- | --- | --- |
| `enable_capture()` | `XAppCaptureEnableRecord` | Synchronous. |
| `disable_capture()` | `XAppCaptureDisableRecord` | Synchronous. |
| `record_diagnostic_clip_async()` | `XAppCaptureRecordDiagnosticClip` | Native API is synchronous; wrapper returns a deferred completion signal to match the public async contract. |
| `take_diagnostic_screenshot_async()` | `XAppCaptureTakeDiagnosticScreenshot` | Native API is synchronous; wrapper returns a deferred completion signal to match the public async contract. |
| `create_metadata()` | none | Opens a script-side context for stateless metadata calls; `reserved_bytes` is retained for compatibility and ignored. |
| `GDKCaptureMetaData::close()` | none | Closes the script-side context. |
| `add_string_event()` | `XAppCaptureMetadataAddStringEvent` | |
| `add_double_event()` | `XAppCaptureMetadataAddDoubleEvent` | |
| `add_int32_event()` | `XAppCaptureMetadataAddInt32Event` | Value clamped to `int32_t`. |
| `start_string_state()` | `XAppCaptureMetadataStartStringState` | |
| `start_double_state()` | `XAppCaptureMetadataStartDoubleState` | |
| `start_int32_state()` | `XAppCaptureMetadataStartInt32State` | Value clamped to `int32_t`. |
| `stop_all_states()` | `XAppCaptureMetadataStopAllStates` | |
| `get_remaining_storage_bytes()` | `XAppCaptureMetadataRemainingStorageBytesAvailable` | Returns `-1` on failure. |

#### `GDK.profile` service

##### Methods

```gdscript
get_profile_async(user: GDKUser, xuid: String) -> Signal
get_profiles_async(user: GDKUser, xuids: PackedStringArray) -> Signal
get_profiles_for_social_group_async(user: GDKUser, social_group: String) -> Signal
```

##### Wrapper types

```gdscript
GDKUserProfile
```

##### Notes

- Methods require a signed-in local `GDKUser` to create the Xbox Services context.
- XUID arguments are accepted as decimal strings to match other public Xbox identity surfaces.
- Single-profile queries return a `GDKUserProfile` in `GDKResult.data`; list and social-group queries return an `Array[GDKUserProfile]`.
- Supported profile fields mirror `XblUserProfile`: app/game display names and picture URIs, gamerscore, classic gamertag, modern gamertag, modern suffix, and unique modern gamertag.

##### Native API mapping

| Wrapper/API | Native API(s) | Notes |
| --- | --- | --- |
| `get_profile_async()` | `XblProfileGetUserProfileAsync`, `XblProfileGetUserProfileResult` | Queries a single XUID. |
| `get_profiles_async()` | `XblProfileGetUserProfilesAsync`, `XblProfileGetUserProfilesResultCount`, `XblProfileGetUserProfilesResult` | Requires at least one target XUID. |
| `get_profiles_for_social_group_async()` | `XblProfileGetUserProfilesForSocialGroupAsync`, `XblProfileGetUserProfilesForSocialGroupResultCount`, `XblProfileGetUserProfilesForSocialGroupResult` | Passes through named groups such as `People` or `Favorites`. |

#### `GDK.string_verify` service

##### Methods

```gdscript
verify_string_async(user: GDKUser, text: String) -> Signal
verify_strings_async(user: GDKUser, strings: PackedStringArray) -> Signal
```

##### Notes

- Methods require a signed-in local `GDKUser` to create the Xbox Services context.
- `verify_string_async()` returns one dictionary in `GDKResult.data`.
- `verify_strings_async()` requires at least one string and returns an `Array` of dictionaries.
- Result dictionaries contain `result_code` (`success`, `offensive`, `too_long`, or `unknown_error`), `acceptable`, and `first_offending_substring`.

##### Native API mapping

| Wrapper/API | Native API(s) | Notes |
| --- | --- | --- |
| `verify_string_async()` | `XblStringVerifyStringAsync`, `XblStringVerifyStringResultSize`, `XblStringVerifyStringResult` | Verifies one string. |
| `verify_strings_async()` | `XblStringVerifyStringsAsync`, `XblStringVerifyStringsResultSize`, `XblStringVerifyStringsResult` | Verifies a non-empty string list. |

#### `GDK.title_storage` service

##### Methods

```gdscript
get_quota_async(user: GDKUser, storage_type: String) -> Signal
list_blob_metadata_async(user: GDKUser, storage_type: String, blob_path := "", skip_items := 0, max_items := 25) -> Signal
get_next_blob_metadata_async(result: GDKTitleStorageBlobMetadataResult) -> Signal
download_blob_async(user: GDKUser, storage_type: String, blob_path: String) -> Signal
upload_blob_async(user: GDKUser, storage_type: String, blob_path: String, data: PackedByteArray, display_name := "", e_tag := "", match_condition := "not_used") -> Signal
delete_blob_async(user: GDKUser, storage_type: String, blob_path: String, e_tag := "", match_condition := "not_used") -> Signal
```

##### Wrapper types

```gdscript
GDKTitleStorageBlobMetadata
GDKTitleStorageBlobMetadataResult
```

##### Notes

- This wraps Xbox Services Title Storage from `title_storage_c.h`; it is not PlayFab Game Saves and not GDK `XGameSaveFiles`.
- Storage type strings are `trusted_platform`, `global`, and `universal`.
- Metadata results hold a native result handle and must stay alive while requesting additional pages.
- `download_blob_async()` first queries exact-path metadata so the native blob type and length are used for the download.
- `upload_blob_async()` creates binary blob metadata for new writes. Use `list_blob_metadata_async()` and `download_blob_async()` to inspect service-returned blob types.
- `delete_blob_async()` maps the native delete API's boolean ETag check, so `match_condition` supports only `not_used` and `if_match`.

##### Native API mapping

| Wrapper/API | Native API(s) | Notes |
| --- | --- | --- |
| `get_quota_async()` | `XblTitleStorageGetQuotaAsync`, `XblTitleStorageGetQuotaResult` | Returns used/quota bytes for one storage type. |
| `list_blob_metadata_async()` | `XblTitleStorageGetBlobMetadataAsync`, `XblTitleStorageGetBlobMetadataResult`, `XblTitleStorageBlobMetadataResultGetItems`, `XblTitleStorageBlobMetadataResultHasNext` | Returns a paged metadata result wrapper. |
| `get_next_blob_metadata_async()` | `XblTitleStorageBlobMetadataResultDuplicateHandle`, `XblTitleStorageBlobMetadataResultGetNextAsync`, `XblTitleStorageBlobMetadataResultGetNextResult`, `XblTitleStorageBlobMetadataResultCloseHandle` | Duplicates the source handle so the async next-page operation owns a stable handle. |
| `download_blob_async()` | `XblTitleStorageGetBlobMetadataAsync`, `XblTitleStorageGetBlobMetadataResult`, `XblTitleStorageBlobMetadataResultGetItems`, `XblTitleStorageBlobMetadataResultCloseHandle`, `XblTitleStorageDownloadBlobAsync`, `XblTitleStorageDownloadBlobResult` | Discovers exact metadata before downloading. |
| `upload_blob_async()` | `XblTitleStorageUploadBlobAsync`, `XblTitleStorageUploadBlobResult` | Uploads a Godot `PackedByteArray` as a binary blob. |
| `delete_blob_async()` | `XblTitleStorageDeleteBlobAsync`, `XAsyncGetStatus` | Native completion is status-only. |

#### `GDK.error_reporting` service

##### Methods

```gdscript
configure_options(debugger_present_options := GDKErrorReporting.ERROR_OPTIONS_NONE, debugger_not_present_options := GDKErrorReporting.ERROR_OPTIONS_NONE) -> GDKResult
set_callback_enabled(enabled: bool) -> GDKResult
is_callback_enabled() -> bool
```

##### Signals

```gdscript
error_reported(result: GDKResult)
```

##### Notes

- Scope is intentionally limited to the public PC GDK `XError` callback/options surface.
- `GDKErrorReporting.ErrorOptions` mirrors the public `XErrorOptions` flag bits: `ERROR_OPTIONS_OUTPUT_DEBUG_STRING_ON_ERROR` (`0x1`), `ERROR_OPTIONS_DEBUG_BREAK_ON_ERROR` (`0x2`), and `ERROR_OPTIONS_FAIL_FAST_ON_ERROR` (`0x4`).
- Do not invent report-submission wrappers when no public PC GDK submission API is available.
- If callers attach custom metadata to downstream telemetry triggered by callback events, callers own privacy/compliance review for that metadata.

##### Native API mapping

| Wrapper/API | Native API(s) | Notes |
| --- | --- | --- |
| `configure_options()` | `XErrorSetOptions` | Uses `GDKErrorReporting.ErrorOptions` enum flags (including bitwise OR combinations) to configure debugger-present and debugger-absent option sets. |
| `set_callback_enabled()` | `XErrorSetCallback`, `XErrorCallback` | Registers/unregisters the callback bridge and forwards events to main-thread Godot signals via `GDK.dispatch()`. |
| `is_callback_enabled()` | wrapper state only | Reports whether callback forwarding is active. |

## Plugin settings

### Runtime

| Setting | Default | Purpose |
| --- | --- | --- |
| `gdk/runtime/initialize_on_startup` | `false` | Calls `GDK.initialize()` automatically during startup. |
| `gdk/runtime/embed_dispatch` | `true` | Dispatches GDK completions automatically from the main thread each frame. |
| `gdk/runtime/auto_add_primary_user` | `false` | Starts a default local-user flow after initialization. |

### Service flags

| Setting | Default | Purpose |
| --- | --- | --- |
| `gdk/services/enable_achievements` | `true` | Registers and initializes the achievements service. |
| `gdk/services/enable_stats` | `true` | Registers and initializes the stats service. |
| `gdk/services/enable_leaderboards` | `true` | Registers and initializes the leaderboards service. |
| `gdk/services/enable_privacy` | `true` | Registers and initializes the privacy service. |
| `gdk/services/enable_presence` | `true` | Registers and initializes the presence service. |
| `gdk/services/enable_social` | `true` | Registers and initializes the social service. |
| `gdk/services/enable_profile` | `true` | Registers and initializes the profile service. |
| `gdk/services/enable_string_verify` | `true` | Registers and initializes the string verification service. |
| `gdk/services/enable_title_storage` | `true` | Registers and initializes the Title Storage service. |

## Build and packaging rules

1. **Plugin ships as its own `.gdextension`**
   - `godot_gdk.gdextension`

2. **Can share internal support code with companion plugins**
   - error mapping
   - string conversion
   - async-op base classes
   - logging

3. **Soft-fail outside supported runtimes**
   - editor should still load docs/classes
   - runtime-only methods return unavailable errors instead of crashing

## Rollout

| Step | Deliverable |
| --- | --- |
| 1 | shared core, `GDK` runtime, users |
| 2 | achievements + current presence/social + MPA |
| 3 | stats + leaderboards + privacy |
| 4 | presence/social completion APIs + profile + string verification + Title Storage |
