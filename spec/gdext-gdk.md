# GDK GDExtension Spec

## Overview

This document defines a **GDScript-first** plan for the `godot_gdk` Godot GDExtension plugin.

`godot_gdk` owns the GDK runtime, users, PC GDK helper services (including XStore commerce wrappers), and Xbox Services wrappers. Input is intentionally out of scope for this document; the companion input design lives in `gdext-gameinput.md`.

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
| Runtime init/shutdown | Implemented | `XGameRuntimeInitializeWithOptions` (`File` source) only in Godot editor sessions (editor + editor-launched runs, via the `editor` feature tag); every exported/templated build uses plain `XGameRuntimeInitialize`; queue/bootstrap |
| User identity/sign-in | Implemented | `XUser`-backed; not an Xbox Services `xsapi-c` surface |
| PC GDK helpers | Implemented selectively | Includes launcher, game UI, accessibility, system metadata, and error reporting; these are outside the Xbox Services coverage matrix below |
| Package metadata and DLC content access | Implemented | `GDK.package` wraps PC-supported `XPackage` enumeration, install progress, package mounts, and resource-pack loading |
| Achievements | Implemented | Achievements Manager-backed |
| Presence | Implemented | `GDK.presence` covers set/clear, single/multi-user queries, social-group query, tracking, non-deprecated change handlers, and cache access |
| Social | Implemented | Social Manager graph/groups (create/update/destroy list-based groups), rich-presence polling toggle, and reputation feedback are exposed; direct relationship paging remains intentionally excluded |
| Multiplayer activity | Implemented and capped | `GDK.multiplayer_activity` is the only multiplayer-related Xbox Services surface |
| Stats | Implemented | `GDK.stats` wraps `title_managed_statistics_c.h` and `user_statistics_c.h` for single/multiple-stat reads, staged merge writes plus replace-all/delete writes, tracking, and cache access |
| Leaderboards | Implemented | `GDK.leaderboards` wraps read-only `leaderboard_c.h` queries and pagination |
| Privacy | Implemented | `GDK.privacy` wraps `privacy_c.h` permission checks and avoid/mute list reads; list-change handlers are not exported by the public XSAPI thunk libraries used by the addon |
| Profile | Implemented | `GDK.profile` wraps `profile_c.h` profile lookup APIs |
| String verification | Implemented | `GDK.string_verify` wraps `string_verify_c.h` text verification APIs |
| Title Storage | Implemented | `GDK.title_storage` wraps `title_storage_c.h`; do not confuse with PlayFab Game Saves or `XGameSaveFiles` |
| Capture | Implemented | `GDK.capture` wraps PC-supported `XAppCapture` metadata/state APIs; console-only paths excluded |
| Display | Implemented | `GDK.display` wraps `XDisplay.h`: HDR mode probe/enable and idle display-timeout deferrals |
| Game activation events | Implemented | `GDK.activation` wraps `XGameActivation.h` on April 2026+ editions; on October 2025 editions (where `XGameActivation.h` is absent) it transparently falls back to `XGameProtocol.h` + `XGameInvite.h`. The backend is selected at compile time via `grdk.h`'s `_GRDK_EDITION`; the public surface is identical either way. |
| Text-to-speech | Implemented | `GDK.speech` wraps `XSpeechSynthesizer.h`: enumerate installed voices, select default/custom voice, and synthesize text/SSML to WAV/PCM bytes locally (no network) |
| Events | Implemented | `GDK.events` wraps the per-title `XGameEvent.h` writer (`XGameEventWrite`) for GDK-native in-game telemetry. Complementary to PlayFab analytics. The `events_c.h` Xbox Services configuration/tuning APIs (`XblEventsSet*`) remain internal-gated and unwrapped |
| Game Save (files) | Implemented | `GDK.game_save` wraps `XGameSaveFiles.h` (`XGameSaveFilesGetFolderWithUiAsync`, `XGameSaveFilesGetRemainingQuota`) for GDK-native file-style saves backed by Connected Storage. Requires the title's Partner Center Xbox services configuration (`TitleId`/`MSAAppId`, matching SCID, Connected Storage enabled) and registered package identity. Unrelated to Xbox Services Title Storage (`GDK.title_storage`). The richer `XGameSave.h` connected-storage container API is intentionally left unwrapped (overlaps PlayFab Game Saves) |
| Runtime feature probe | Implemented | `GDK.system.is_feature_available(name)` wraps `XGameRuntimeIsFeatureAvailable` (`XGameRuntimeFeature.h`) so titles can gate optional GDK features (e.g. Events, GameChat) at runtime |
| Voice and text chat | Implemented | `GDK.game_chat` wraps the Game Chat 2 (`GameChat2.h`) `chat_manager` for GDK-native voice + text chat: user management, communication relationships, mute/volume controls, text, and text-to-speech. The wrapper exposes Game Chat's opaque data-frame surface (`outgoing_data_frame` signal + `process_incoming_data_frame`) and builds **no** network transport — titles ferry frames over their own transport (sample/tests use single-process loopback). This is the GDK-native alternative to PlayFab Party (`godot_playfab`) |
| Multiplayer/session/matchmaking | Excluded | Do not wrap matchmaking, MPSD, multiplayer sessions, lobby/session transport, or legacy invite APIs |
| Store/commerce/licensing | Implemented (XStore-only) | Exposed via `GDK.store` using public XStore APIs; excluded from the Xbox Services coverage matrix below |

### Xbox Services coverage matrix

#### Already exposed

| Public service | Native Xbox Services APIs | Notes |
| --- | --- | --- |
| `GDK.achievements` | Achievements Manager APIs including `XblAchievementsManagerGetAchievementsByState` | Query/update/cache are manager-backed; cached achievements can be filtered by progress state. |
| `GDK.stats` | `XblUserStatisticsGetSingleUserStatisticAsync`, `XblUserStatisticsGetSingleUserStatisticResultSize`, `XblUserStatisticsGetSingleUserStatisticResult`, `XblUserStatisticsGetSingleUserStatisticsAsync`, `XblUserStatisticsGetSingleUserStatisticsResultSize`, `XblUserStatisticsGetSingleUserStatisticsResult`, `XblUserStatisticsGetMultipleUserStatisticsAsync`, `XblUserStatisticsGetMultipleUserStatisticsResultSize`, `XblUserStatisticsGetMultipleUserStatisticsResult`, `XblUserStatisticsAddStatisticChangedHandler`, `XblUserStatisticsRemoveStatisticChangedHandler`, `XblUserStatisticsTrackStatistics`, `XblUserStatisticsStopTrackingStatistics`, `XblUserStatisticsStopTrackingUsers`, `XblTitleManagedStatsUpdateStatsAsync`, `XblTitleManagedStatsWriteAsync`, `XblTitleManagedStatsDeleteStatsAsync` | Query a single named stat or one/more users' stats, stage merge writes, replace-all or delete title-managed stats, track changes, and read cached stats. |
| `GDK.leaderboards` | `XblLeaderboardGetLeaderboardAsync`, `XblLeaderboardGetLeaderboardResultSize`, `XblLeaderboardGetLeaderboardResult`, `XblLeaderboardResultGetNextAsync`, `XblLeaderboardResultGetNextResultSize`, `XblLeaderboardResultGetNextResult` | Read-only global, around-user, social leaderboard queries plus next-page pagination. |
| `GDK.privacy` | `XblPrivacyCheckPermissionAsync`, `XblPrivacyCheckPermissionResultSize`, `XblPrivacyCheckPermissionResult`, `XblPrivacyCheckPermissionForAnonymousUserAsync`, `XblPrivacyCheckPermissionForAnonymousUserResultSize`, `XblPrivacyCheckPermissionForAnonymousUserResult`, `XblPrivacyBatchCheckPermissionAsync`, `XblPrivacyBatchCheckPermissionResultSize`, `XblPrivacyBatchCheckPermissionResult`, `XblPrivacyGetAvoidListAsync`, `XblPrivacyGetAvoidListResultCount`, `XblPrivacyGetAvoidListResult`, `XblPrivacyGetMuteListAsync`, `XblPrivacyGetMuteListResultCount`, `XblPrivacyGetMuteListResult` | Permission checks against XUIDs or anonymous user types, batch permission checks, and avoid/mute list reads. |
| `GDK.presence` | `XblPresenceSetPresenceAsync`, `XblPresenceGetPresenceAsync`, `XblPresenceGetPresenceResult`, `XblPresenceGetPresenceForMultipleUsersAsync`, `XblPresenceGetPresenceForMultipleUsersResultCount`, `XblPresenceGetPresenceForMultipleUsersResult`, `XblPresenceGetPresenceForSocialGroupAsync`, `XblPresenceGetPresenceForSocialGroupResultCount`, `XblPresenceGetPresenceForSocialGroupResult`, `XblPresenceAddDevicePresenceChangedHandler`, `XblPresenceRemoveDevicePresenceChangedHandler`, `XblPresenceAddTitlePresenceChangedHandler`, `XblPresenceRemoveTitlePresenceChangedHandler`, `XblPresenceTrackUsers`, `XblPresenceStopTrackingUsers`, `XblPresenceTrackAdditionalTitles`, `XblPresenceStopTrackingAdditionalTitles`, `XblPresenceRecordGetXuid`, `XblPresenceRecordGetUserState`, `XblPresenceRecordGetDeviceRecords`, `XblPresenceRecordCloseHandle` | Set/clear local presence, query presence records, track change notifications, and cache translated records. |
| `GDK.social` | Social Manager APIs including `XblSocialManagerAddLocalUser`, `XblSocialManagerRemoveLocalUser`, `XblSocialManagerDoWork`, `XblSocialManagerCreateSocialUserGroupFromFilters`, `XblSocialManagerCreateSocialUserGroupFromList`, `XblSocialManagerUpdateSocialUserGroup`, `XblSocialManagerDestroySocialUserGroup`, `XblSocialManagerUserGroupGetUsers`, and `XblSocialManagerSetRichPresencePollingStatus`; reputation APIs `XblSocialSubmitReputationFeedbackAsync` and `XblSocialSubmitBatchReputationFeedbackAsync` | Direct relationship REST-style wrappers are intentionally not exposed. |
| `GDK.profile` | `XblProfileGetUserProfileAsync`, `XblProfileGetUserProfileResult`, `XblProfileGetUserProfilesAsync`, `XblProfileGetUserProfilesResultCount`, `XblProfileGetUserProfilesResult`, `XblProfileGetUserProfilesForSocialGroupAsync`, `XblProfileGetUserProfilesForSocialGroupResultCount`, `XblProfileGetUserProfilesForSocialGroupResult` | Query one profile, multiple profiles by XUID list, or profiles for a named social group. |
| `GDK.string_verify` | `XblStringVerifyStringAsync`, `XblStringVerifyStringResultSize`, `XblStringVerifyStringResult`, `XblStringVerifyStringsAsync`, `XblStringVerifyStringsResultSize`, `XblStringVerifyStringsResult` | Verify one string or a batch of strings and return per-string result dictionaries. |
| `GDK.title_storage` | `XblTitleStorageGetQuotaAsync`, `XblTitleStorageGetQuotaResult`, `XblTitleStorageGetBlobMetadataAsync`, `XblTitleStorageGetBlobMetadataResult`, `XblTitleStorageBlobMetadataResultGetItems`, `XblTitleStorageBlobMetadataResultHasNext`, `XblTitleStorageBlobMetadataResultGetNextAsync`, `XblTitleStorageBlobMetadataResultGetNextResult`, `XblTitleStorageBlobMetadataResultDuplicateHandle`, `XblTitleStorageBlobMetadataResultCloseHandle`, `XblTitleStorageDownloadBlobAsync`, `XblTitleStorageDownloadBlobResult`, `XblTitleStorageUploadBlobAsync`, `XblTitleStorageUploadBlobResult`, `XblTitleStorageDeleteBlobAsync` | Query quota, list paged metadata, download blobs via metadata discovery, upload bytes, and delete blobs. |
| `GDK.multiplayer_activity` | `XblMultiplayerActivityUpdateRecentPlayers`, `XblMultiplayerActivityFlushRecentPlayersAsync`, `XblMultiplayerActivitySetActivityAsync`, `XblMultiplayerActivityGetActivityAsync`, `XblMultiplayerActivityGetActivityResultSize`, `XblMultiplayerActivityGetActivityResult`, `XblMultiplayerActivityDeleteActivityAsync`, `XblMultiplayerActivitySendInvitesAsync` | This is the only multiplayer-related Xbox Services surface. Invite launch events are forwarded from `GDK.activation`'s single activation-event owner (one `XGameActivationRegisterForEvent` registration on April 2026+, or the `XGameProtocol` / `XGameInvite` registrations on October 2025) rather than registering another native activation callback. |

#### Accepted Xbox Services wrappers

| Planned public service | Public wrapper scope | Native Xbox Services APIs to wrap |
| --- | --- | --- |

#### Explicit no-wrap PC GDK families

Do not expose wrappers for:

- `XGameProtocol.h` — not a *public* wrapper surface. On April 2026+ editions `XGameProtocolRegisterForActivation` / `XGameProtocolUnregisterForActivation` are explicitly `__declspec(deprecated)` and superseded by `XGameActivationRegisterForEvent` / `XGameActivationUnregisterForEvent`; on October 2025 editions (no `XGameActivation.h`) `GDK.activation` uses them internally as the protocol-activation backend. Either way, consume `GDK.activation.protocol_activated` — never a standalone `XGameProtocol` wrapper.
- `XGameInvite.h` — not a *public* wrapper surface. On April 2026+ editions every entry point (`XGameInviteRegisterForEvent`, `XGameInviteRegisterForPendingEvent`, the matching `Unregister` calls, and `XGameInviteAcceptPendingInvite`) is `__declspec(deprecated)` and the SDK points each one at the `XGameActivation*` equivalent; on October 2025 editions `GDK.activation` uses them internally as the pending/accepted-invite backend. Consume `GDK.activation` (the `pending_invite_received` and `invite_accepted` signals plus `accept_pending_invite()`); `XboxMultiplayerActivity` subscribes to `GDK.activation`'s internal event fan-out and must not register its own native activation callback.

> Note: `XGameEvent.h`'s `XGameEventWrite` is **wrapped** by `GDK.events` (see the Events scope row and the `GDK.events` service section) and is intentionally not listed here.

#### Explicit no-wrap Xbox Services APIs

Do not expose wrappers for:

- `matchmaking_c.h`
- `multiplayer_c.h`
- `multiplayer_manager_c.h`
- MPSD, multiplayer sessions, lobby/session transport APIs
- `game_invite_c.h` legacy/deprecated invite APIs
- `events_c.h` Xbox Services configuration/tuning APIs (`XblEventsSet*`); the per-title in-game event *writer* is wrapped instead via `GDK.events` → `XGameEventWrite` (`XGameEvent.h`). `XblEventsWriteInGameEvent` is an alternate write path and remains unwrapped in favor of the primary `XGameEventWrite`
- generic public `notification_c.h` subscription wrappers; use notification/RTA plumbing internally only if a wrapped service requires it
- `XblPrivacyAddMuteListChangedHandler`, `XblPrivacyRemoveMuteListChangedHandler`, `XblPrivacyAddBlockListChangedHandler`, and `XblPrivacyRemoveBlockListChangedHandler` while the addon links against `Microsoft.Xbox.Services.C.Thunks`; these header-declared APIs are not exported by the public thunk libraries
- deprecated social, presence, and statistics subscription APIs
- direct `XblSocialGetSocialRelationshipsAsync` relationship paging; the public social graph remains Social Manager-backed

## Rationale and prior art

This spec borrows the Godot-facing integration patterns that already work well in prior art and then reshapes them around the actual lifecycle of GDK. The main prior-art reference is [GodotSteam](https://godotsteam.com/) and its active source tree on [Codeberg](https://codeberg.org/godotsteam/godotsteam), which demonstrates the value of native singletons, project settings, callback dispatch, and optional Godot-native adapters in a platform plugin.

### Why GDScript-first wrappers instead of raw native handles

Godot's native async/event style is built around first-class [signals](https://docs.godotengine.org/en/stable/classes/class_signal.html), [`await`](https://docs.godotengine.org/en/stable/tutorials/scripting/gdscript/gdscript_basics.html#awaiting-signals-or-coroutines), and [`RefCounted`](https://docs.godotengine.org/en/stable/classes/class_refcounted.html) objects. Exposing raw native handles like `XUserHandle`, `XAsyncBlock`, or queue handles directly to GDScript would fight both Godot ergonomics and Godot lifetime rules.

Wrapping native state in Godot objects such as `XboxUser`, `XboxLeaderboard`, and `XboxTitleStorageBlobMetadata`, plus exposing one-shot work through completion signals or op objects where handles are still needed, makes the API fit normal GDScript usage patterns like signal connections and `await GDK.users.add_default_user_async()`. The common Godot pattern `await get_tree().create_timer(...).timeout` ([SceneTreeTimer](https://docs.godotengine.org/en/stable/classes/class_scenetreetimer.html), [SceneTree.create_timer](https://docs.godotengine.org/en/stable/classes/class_scenetree.html#class-scenetree-method-create-timer)) is the bar this API should feel native next to.

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

### Naming: `Xbox*` classes, `GDK` singleton

Godot-facing classes registered by this addon use the `Xbox` prefix (`XboxUser`,
`XboxAchievement`, `XboxResult`, …), and the native root class is `Xbox`.

The engine **singleton name** is deliberately *not* renamed: the root class is
registered under the name `GDK` by default, so `GDK.initialize()`,
`GDK.users`, and every other `GDK.*` call site keeps working. This preserves
source compatibility for forks and titles already written against the `GDK`
global while moving the type names to the `Xbox*` convention.

Consequences worth knowing:

- The singleton *name* and the singleton's *class name* are now different
  strings. Code that validates a resolved singleton must call
  `is_class("Xbox")` (C#: `IsClass("Xbox")`), not `is_class(<singleton name>)`.
- `gdk/runtime/*` Project Settings (including `gdk/runtime/singleton_name`,
  default `"GDK"`), the `addons/godot_gdk` folder, and `godot_gdk.gdextension`
  are unchanged.
- No deprecated `GDK*` class aliases are registered. The old class names are
  gone from `ClassDB`; only the singleton name is preserved for compatibility.
- The C# facade follows the same rule: namespace `GodotXbox`, static entry
  point `Xbox`, wrapper types `Xbox*` — resolving the singleton named `GDK`.

### Global singleton

- `GDK` (engine singleton name) — an instance of the `Xbox` class.

### Wrapper types exposed to GDScript

| Native concept | GDScript wrapper |
| --- | --- |
| one-shot async request | `Signal` |
| `HRESULT` + payload | `XboxResult` |
| `XUserHandle` | `XboxUser` |
| package mount lifetime | `XboxPackageMount` |
| loaded resource-pack metadata | `XboxPackageResourcePack` |
| Stats and leaderboard payloads | stats `Dictionary`, `XboxLeaderboard`, `XboxLeaderboardColumn`, `XboxLeaderboardRow` |
| Privacy check payloads | `Dictionary` |
| Presence payloads | `XboxPresenceRecord` |
| Social graph payloads | `XboxSocialUser`, `XboxSocialGroup` |
| process-wide `XAppCaptureMetadata*` calls | `XboxCaptureMetaData` |
| Profile payloads | `XboxUserProfile` |
| Title Storage payloads | `XboxTitleStorageBlobMetadata`, `XboxTitleStorageBlobMetadataResult` |

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
| Results | Success/failure lives in `HRESULT` plus API-specific payload structs. | Script code expects one stable result shape with Godot-native data. | Convert native status into a reusable `XboxResult` carrying `ok`, `hresult`, `code`, `message`, and `data`. |
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
   - `XboxResult` is the only public success/error payload shape.
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
- resolve the signal with an already-finalized `XboxResult`
- preserve the same cache-before-completion ordering guarantees as native async calls

#### Shutdown behavior

Shutdown must be queue-safe and op-safe:

- active one-shot requests should be retained by the runtime until they reach a terminal state
- queue termination should surface cancellation as failed `XboxResult` values rather than silently dropping callbacks
- no service cache or Godot object should be mutated after the runtime starts teardown
- `GDK.shutdown()` should clean up services first, then terminate and close the shared queue. `XGameRuntimeUninitialize()` is process-lifetime cleanup that runs once when the extension is torn down.

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
   - Resolve the returned completion signal with the final `XboxResult`.

By the time the completion signal resolves, the relevant service cache should already be current.

#### Completion signal

```gdscript
await some_service.some_method_async() # -> XboxResult
```

#### `XboxResult`

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

var result: XboxResult = await GDK.users.add_default_user_async()
if result.ok:
    var user: XboxUser = result.data
```

#### Example: service cache is current before `completed`

```gdscript
GDK.stats.stats_updated.connect(_on_stats_updated)

var result: XboxResult = await GDK.stats.query_user_stats_async(user, PackedStringArray(["xp", "wins"]))
if result.ok:
    # Safe to read the cache here; service state should already be updated.
    print(GDK.stats.get_cached_stats(user))

func _on_stats_updated(updated_user: XboxUser, stats: Dictionary) -> void:
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
GDK.stats.query_user_stats_async(user, PackedStringArray(["xp"])).connect(func(result: XboxResult) -> void:
    if not result.ok:
        push_error(result.message)
)
```

#### Example: long-lived background state

```gdscript
var start_result := GDK.social.start_social_graph(user)
if start_result.ok:
    GDK.social.social_group_updated.connect(_on_social_group_updated)

func _on_social_group_updated(group: XboxSocialGroup) -> void:
    var users_result := GDK.social.get_group_users(group)
    if users_result.ok:
        print("Group now has %d users" % users_result.data.size())
```

### Root singleton

#### Root API

```gdscript
GDK.initialize(config: Variant = null) -> XboxResult
GDK.shutdown() -> void
GDK.is_available() -> bool
GDK.is_initialized() -> bool
GDK.dispatch() -> int
GDK.get_capture() -> XboxCapture
GDK.get_system() -> XboxSystem
```

#### Root properties

```gdscript
GDK.users: XboxUsers
GDK.game_ui: XboxGameUI
GDK.system: XboxSystem
GDK.accessibility: XboxAccessibility
GDK.achievements: XboxAchievements
GDK.package: XboxPackage
GDK.stats: XboxStats
GDK.leaderboards: XboxLeaderboards
GDK.privacy: XboxPrivacy
GDK.presence: XboxPresence
GDK.social: XboxSocial
GDK.profile: XboxProfile
GDK.string_verify: XboxStringVerify
GDK.title_storage: XboxTitleStorage
GDK.error_reporting: XboxErrorReporting
GDK.launcher: XboxLauncher
GDK.capture: XboxCapture
GDK.activation: XboxActivation
GDK.multiplayer_activity: XboxMultiplayerActivity
GDK.speech: XboxSpeechSynthesizer
GDK.events: XboxEvents
GDK.game_save: XboxGameSave
GDK.game_chat: XboxGameChat
```

#### Root signals

```gdscript
initialized()
shutdown_completed()
runtime_error(result: XboxResult)  # XError-only: see Error reporting
```

`runtime_error` on the root singleton is reserved for `XError` callback events
sourced from `XboxErrorReporting::dispatch()`. Caller-driven failures are
delivered as the per-call `XboxResult` (return value or async signal payload),
not via this signal. Per-service unsolicited errors (e.g. background social
graph or achievement dispatch failures) are emitted as
`GDK.<service>.runtime_error(result)` (currently `GDK.social.runtime_error`
and `GDK.achievements.runtime_error`).

#### Runtime behavior

- `initialize()` sets up the GDK runtime and the shared `XTaskQueue`. When `config` is a `Dictionary`, Xbox services accepts the first SCID override found at `scid`, `service_configuration_id`, `xbox_live/scid`, or nested `xbox_live.scid`; otherwise it derives the current-title SCID from `XGameGetXboxTitleId()`.
- Calling `initialize()` again without `shutdown()` returns `XboxResult.code == "already_initialized"`, so startup helpers should guard with `GDK.is_initialized()`.
- `is_available()` reflects the compile-time `_GAMING_DESKTOP` gate exposed by `XboxRuntime::is_available()`.
- `dispatch()` manually dispatches pending completions when automatic dispatch is disabled or when deterministic control is needed.
- `gdk/runtime/embed_dispatch` defaults to `true` and enables automatic per-frame dispatch from the main thread.

#### Native runtime mapping

| Public surface | Native API(s) | Notes |
| --- | --- | --- |
| `GDK.initialize()` | `XGameRuntimeInitializeWithOptions` (`File` source pointing at `res://MicrosoftGame.config`) in Godot editor sessions only, otherwise plain `XGameRuntimeInitialize`, `XTaskQueueCreate` | Creates the shared task queue and runtime bootstrap state used by all one-shot completion signals and service-owned callback bridges. The `WithOptions` path is gated to editor sessions (the editor and editor-launched `godot project.godot` runs, detected via the `editor` feature tag), which lack package identity, so downstream `XGameGetXboxTitleId` / `XblInitialize` calls do not surface `xbox_title_id_unavailable`. Every exported/templated build (packaged, registered, or loose) gets identity from its package and takes the plain-init path; packaged builds reject custom options with `E_GAMERUNTIME_OPTIONS_NOT_SUPPORTED` (`0x8924010A`). |
| `GDK.shutdown()` | `XTaskQueueTerminate`, `XTaskQueueCloseHandle` | Service and user cleanup runs first; queue teardown happens last. `XGameRuntimeUninitialize()` is intentionally reserved for extension teardown (`~XboxRuntime()`), not each shutdown cycle. |
| `GDK.dispatch()` | `XTaskQueueDispatch`, `XblAchievementsManagerDoWork`, `XblSocialManagerDoWork`, `XErrorSetCallback` callback-drain bridge | Main-thread pump. Dispatch the completion port, translate native payloads into Godot objects, update caches, then emit signals. |
| `GDK.launcher.launch_uri()` | `XLaunchUri` (`XLauncher.h`, `xgameruntime.lib`) | PC-supported URI launcher surface for app-to-app, Store, and Settings destinations. |
| per-user Xbox services context | `XblContextCreateHandle`, `XblContextCloseHandle` | Create once for each admitted `XboxUser`; store inside the wrapper for achievements, stats, leaderboards, presence, and social calls. |

Every one-shot async wrapper should allocate an `XAsyncBlock` against the shared queue and complete it only after the Godot-side cache and wrapper state are current. Shutdown cancellation is terminal: retained pending signals must queue exactly one completion with `XboxResult.code == "cancelled"` before the shared queue is terminated.

### Service specifications

Unless otherwise noted, the service sections below follow the global naming, type, and terminology rules defined in **Public API conventions**.

#### `GDK.launcher` service

##### Methods

```gdscript
launch_uri(uri: String, user: XboxUser = null) -> XboxResult
```

##### Validation contract

- `launch_uri` rejects blank/malformed input with `invalid_uri`.
- Unsupported URI destinations reject with `unsupported_launcher_destination`.
- Optional `user` must be a signed-in `XboxUser` when provided (`invalid_user`).

#### `GDK.speech` service

##### Methods

```gdscript
get_installed_voices() -> Array            # [{id, display_name, language, gender, description}, ...]
set_default_voice() -> XboxResult
set_custom_voice(voice_id: String) -> XboxResult
synthesize_text(text: String) -> XboxResult        # data.audio_wav := PackedByteArray (RIFF/WAV), data.byte_count
synthesize_ssml(ssml: String) -> XboxResult        # data.audio_wav := PackedByteArray (RIFF/WAV), data.byte_count
synthesize_to_stream(text: String) -> AudioStreamWAV   # convenience; null on failure
```

##### Behavior contract

- Synthesis runs locally on the device and produces WAV/PCM bytes with no network call, so it is headless-testable.
- The service lazily creates a single native `XSpeechSynthesizer` the first time a voice is selected or speech is synthesized, and releases it on `GDK.shutdown`.
- Voice selection (`set_default_voice` / `set_custom_voice`) applies to every subsequent `synthesize_*` call.
- Empty input rejects with `invalid_input`; an empty voice id rejects with `invalid_voice_id`.
- Calls before `GDK.initialize()` return a `not_initialized` error; a failure to create the synthesizer returns `speech_synthesizer_unavailable`.

##### Native API mapping

| Wrapper/API | Native API(s) | Notes |
| --- | --- | --- |
| `get_installed_voices()` | `XSpeechSynthesizerEnumerateInstalledVoices` | Collects `XSpeechSynthesizerVoiceInformation` rows into dictionaries. |
| `set_default_voice()` | `XSpeechSynthesizerCreate`, `XSpeechSynthesizerSetDefaultVoice` | Creates the synthesizer lazily. |
| `set_custom_voice()` | `XSpeechSynthesizerCreate`, `XSpeechSynthesizerSetCustomVoice` | Selects a voice by id. |
| `synthesize_text()` | `XSpeechSynthesizerCreateStreamFromText`, `XSpeechSynthesizerGetStreamDataSize`, `XSpeechSynthesizerGetStreamData`, `XSpeechSynthesizerCloseStreamHandle` | Returns RIFF/WAV bytes in `XboxResult.data.audio_wav`. |
| `synthesize_ssml()` | `XSpeechSynthesizerCreateStreamFromSsml`, `XSpeechSynthesizerGetStreamDataSize`, `XSpeechSynthesizerGetStreamData`, `XSpeechSynthesizerCloseStreamHandle` | SSML variant of `synthesize_text`. |
| `synthesize_to_stream()` | (decodes the `synthesize_text` WAV bytes) | Parses the RIFF/WAV header and returns a ready-to-play `AudioStreamWAV`. |
| service shutdown | `XSpeechSynthesizerCloseHandle` | Releases the cached synthesizer. |

#### `GDK.events` service

##### Methods

```gdscript
write_event(user: XboxUser, event_name: String, dimensions := {}, measurements := {}) -> XboxResult
set_play_session_id(play_session_id: String)   # empty string regenerates a fresh GUID
get_play_session_id() -> String
```

##### Behavior contract

- `GDK.events` is the GDK-native in-game telemetry path, complementary to PlayFab analytics (`godot_playfab`). Titles may use either or both.
- `write_event()` is synchronous and returns an `XboxResult`; the runtime batches and uploads events asynchronously.
- `dimensions` hold fields with a finite set of values (map id, difficulty, mode, boolean settings); `measurements` hold scalar numeric metrics (score, time, counters, position). Both `Dictionary` payloads are serialized to JSON internally.
- The SCID is pulled from the cached `XboxServices` state; `play_session_id` is a per-session GUID auto-generated on runtime init and overridable via `set_play_session_id()`.
- The event name and the names of all `dimensions`/`measurements` fields must match the title's Xbox Live service-configuration event manifest (case-insensitive); the service **silently drops** events whose names do not match.
- Graceful degradation: returns `events_feature_unavailable` on `E_NOTIMPL` when the GRTS XGameEvent feature/runtime is not installed. Other validation failures return `invalid_event_name`, `invalid_user`, `not_initialized`, or `xbox_services_uninitialized`.

##### Native API mapping

| Wrapper/API | Native API(s) | Notes |
| --- | --- | --- |
| `write_event()` | `XGameEventWrite` (`XGameEvent.h`) | Primary GDK telemetry write path. Requires a signed-in `XboxUser` handle and the cached SCID; `dimensions`/`measurements` are JSON-serialized. |
| `set_play_session_id()` / `get_play_session_id()` | (wrapper-managed GUID) | Session-scoped GUID forwarded as `playSessionId` to `XGameEventWrite`. |

> The alternate Xbox Services write path `XblEventsWriteInGameEvent` (`events_c.h`) is intentionally **not** wrapped; `XGameEventWrite` is the primary GDK path. The `XblEventsSet*` configuration/tuning functions remain internal-gated.

#### `GDK.game_save` service

##### Methods

```gdscript
get_folder_async(user: XboxUser) -> Signal            # XboxResult.data.path := absolute save-folder path
get_remaining_quota_async(user: XboxUser) -> Signal   # XboxResult.data.bytes := remaining quota in bytes
```

##### Behavior contract

- `GDK.game_save` wraps the GDK-native file-style save API (`XGameSaveFiles.h`), which is backed by **Connected Storage**. It is distinct from PlayFab Game Saves (`godot_playfab`) and from Xbox Services Title Storage (`GDK.title_storage`).
- The title must be configured for Connected Storage-backed saves: `TitleId` + `MSAAppId` in `MicrosoftGame.config`, a SCID matching Partner Center (**Xbox services → Xbox settings**), **Connected Storage** enabled for the title in Partner Center, and registered package identity (the per-user store lives under `%LOCALAPPDATA%\Packages\<package>\SystemAppData\xgs\`). The Connected Storage option currently sits on Partner Center's **Gameplay settings → Title Storage** page, but Connected Storage is a different system from Xbox Services Title Storage — enabling the title/global/universal storage types does nothing for Game Saves. A mismatch surfaces as `E_GS_NO_ACCESS` (`0x80830002`), which the wrapper propagates verbatim. The `SaveGameStorage` element is **not** required for these APIs — it configures the separate no-code cloud-saves feature.
- `get_folder_async()` wraps `XGameSaveFilesGetFolderWithUiAsync`: it resolves (and may surface a system UI for) the user's save folder, returning the absolute path in `XboxResult.data.path`. Titles then read/write ordinary files under that folder.
- `get_remaining_quota_async()` wraps `XGameSaveFilesGetRemainingQuota`, returning the remaining byte quota in `XboxResult.data.bytes`. The native entry point is synchronous and `XGameSaveFiles` exposes no async variant, but the GDK fails the call with `E_GS_ASYNC_FUNCTION_REQUIRED` (`0x8083000E`) when it is issued from a time-sensitive thread such as Godot's main thread. The wrapper therefore drives it through a custom `XAsyncProvider` that executes the call on the shared task queue's work port and completes the pending signal from the queue's completion port on the main thread. Unlike `get_folder_async()`, the request binds no cancel handler: the blocking native call cannot be interrupted, so forwarding to `XAsyncCancel` would be inert. This is not caller-visible — neither method exposes a cancel API, since both return a bare `Signal` and `XboxPendingSignal` is an internal class. Runtime shutdown still resolves the awaiter with a cancelled `XboxResult`, so no `await` can hang.
- The service configuration id (SCID) is pulled from the cached `XboxServices` state. Validation failures return `not_initialized`, `invalid_user`, or `xbox_services_uninitialized`.

##### Native API mapping

| Wrapper/API | Native API(s) | Notes |
| --- | --- | --- |
| `get_folder_async()` | `XGameSaveFilesGetFolderWithUiAsync`, `XGameSaveFilesGetFolderWithUiResult` | Returns the absolute save-folder path; may show a system UI. |
| `get_remaining_quota_async()` | `XGameSaveFilesGetRemainingQuota`, `XAsyncBegin` / `XAsyncSchedule` / `XAsyncComplete` | Native call is synchronous; the wrapper runs it on the task queue's work port so the GDK does not reject it with `E_GS_ASYNC_FUNCTION_REQUIRED`. Returns remaining quota bytes. |

> The richer `XGameSave.h` connected-storage container API (27 functions) is intentionally **not** wrapped: it is a more complex API than `XGameSaveFiles` and overlaps PlayFab Game Saves.

#### `GDK.game_chat` service

##### Methods

```gdscript
initialize(max_users := 16, default_relationship := XboxGameChat.RELATIONSHIP_SEND_AND_RECEIVE_ALL) -> XboxResult
is_initialized() -> bool
cleanup() -> void
add_local_user(user: XboxUser) -> XboxResult                                  # XboxResult.data.xuid
add_remote_user(xuid: String, endpoint_id: int) -> XboxResult               # XboxResult.data := {xuid, endpoint_id}
remove_user(xuid: String) -> XboxResult
set_communication_relationship(local_xuid, target_xuid, relationship: int) -> XboxResult
set_microphone_muted(local_xuid: String, muted: bool) -> XboxResult
set_remote_user_muted(local_xuid: String, target_xuid: String, muted: bool) -> XboxResult
set_audio_render_volume(local_xuid: String, target_xuid: String, volume: float) -> XboxResult
send_text(local_xuid: String, text: String) -> XboxResult
synthesize_text_to_speech(local_xuid: String, text: String) -> XboxResult
process_incoming_data_frame(source_endpoint_id: int, bytes: PackedByteArray) -> XboxResult
get_chat_users() -> Array          # [{xuid, is_local, chat_indicator}]
```

##### Signals

```gdscript
outgoing_data_frame(target_endpoint_ids: PackedInt64Array, bytes: PackedByteArray, transport_requirement: int)
text_chat_received(sender_xuid: String, message: String)
transcribed_chat_received(speaker_xuid: String, message: String)
```

##### Behavior contract

- `GDK.game_chat` wraps the Game Chat 2 (`GameChat2.h`) C++ `chat_manager` singleton. It is the GDK-native voice + text chat option; PlayFab Party (`godot_playfab`) is the batteries-included alternative that owns its own transport.
- **No transport is built or selected.** Game Chat encodes its own opaque audio/text data frames; the wrapper exposes that surface only. During the per-frame pump (driven by `GDK.dispatch()`), each frame Game Chat wants to send is emitted on `outgoing_data_frame`; the title delivers those bytes over whatever transport it already has and calls `process_incoming_data_frame()` on each receiving instance. The sample and tests demonstrate this with single-process **loopback** (feeding each `outgoing_data_frame` straight back into `process_incoming_data_frame`).
- `initialize()` requires the GDK runtime to be initialized; it then initializes the `chat_manager` for `max_users` combined local + remote users with `default_relationship` between new users. Re-initializing without `cleanup()` first returns `already_initialized`; `max_users <= 0` returns `invalid_max_users`.
- `add_local_user()` requires a signed-in `XboxUser` (its decimal XUID); remote users are added by XUID + a title-assigned `endpoint_id` used to address outgoing frames and attribute incoming ones.
- Independent voice and text control mirrors PlayFab Party: `set_communication_relationship()` takes a bitwise combination of `XboxGameChat.CommunicationRelationship` flags (separate send/receive bits for microphone, text-to-speech audio, and text), and `set_microphone_muted()` / `set_remote_user_muted()` / `set_audio_render_volume()` provide per-user mute and volume.
- Inbound text and transcription surface on `text_chat_received` / `transcribed_chat_received` during the pump. `shutdown()` calls `cleanup()` automatically.
- Real voice capture/render requires multi-machine audio hardware and cannot be validated headlessly (mirrors the Party "voice deferred" stance); headless coverage exercises the service surface, enum constants, lifecycle, and data-frame plumbing.

##### Native API mapping

| Wrapper/API | Native API(s) | Notes |
| --- | --- | --- |
| `initialize()` | `chat_manager::initialize` | Sets max users and the default local↔remote relationship. |
| `cleanup()` / service shutdown | `chat_manager::cleanup` | Releases all Game Chat resources. |
| `add_local_user()` | `chat_manager::add_local_user` | Adds a signed-in user's XUID as a local chat user. |
| `add_remote_user()` | `chat_manager::add_remote_user` | Adds a remote XUID reachable on a title-assigned endpoint id. |
| `remove_user()` | `chat_manager::get_chat_users`, `chat_manager::remove_user` | Locates the user by XUID, then removes it. |
| `set_communication_relationship()` | `chat_user::chat_user_local::set_communication_relationship` | Bitwise `game_chat_communication_relationship_flags`. |
| `set_microphone_muted()` | `chat_user::chat_user_local::set_microphone_muted` | Local microphone mute. |
| `set_remote_user_muted()` | `chat_user::chat_user_local::set_remote_user_muted` | Per-remote-user mute for a local user. |
| `set_audio_render_volume()` | `chat_user::chat_user_local::set_audio_render_volume` | Per-remote render volume. |
| `send_text()` | `chat_user::chat_user_local::send_chat_text` | 1–1023 character chat text. |
| `synthesize_text_to_speech()` | `chat_user::chat_user_local::synthesize_text_to_speech` | TTS-as-microphone for the local user. |
| `process_incoming_data_frame()` | `chat_manager::process_incoming_data` | Hands a received frame to Game Chat by source endpoint id. |
| `get_chat_users()` | `chat_manager::get_chat_users`, `chat_user::xbox_user_id`, `chat_user::local`, `chat_user::chat_indicator` | Snapshots current users into dictionaries. |
| `outgoing_data_frame` (pump) | `chat_manager::start_processing_data_frames`, `chat_manager::finish_processing_data_frames` | Emits each frame's targets, bytes, and transport requirement. |
| `text_chat_received` / `transcribed_chat_received` (pump) | `chat_manager::start_processing_state_changes`, `chat_manager::finish_processing_state_changes` | Drains text + transcription state changes. |

> Game Chat 2 ships in its own `GameChat2.lib`/`GameChat2.dll`. The wrapper links `Xbox::GameChat2` and includes `GameChat2Impl.h` in exactly one translation unit (the class methods are defined out-of-line there). No network transport is part of this addon.

#### `GDK.users` service

##### Methods

```gdscript
add_default_user_async() -> Signal
add_user_with_ui_async(allow_guests := false) -> Signal
add_user_by_id_with_ui_async(xuid: String) -> Signal
get_primary_user() -> XboxUser
get_users() -> Array[XboxUser]
get_max_users() -> XboxResult                       # data: int
is_sign_out_available() -> bool
sign_out_async(user: XboxUser) -> Signal
acquire_sign_out_deferral() -> XboxResult           # data: XboxUserSignOutDeferral
find_user_by_xuid(xuid: String) -> XboxResult       # data: XboxUser
find_user_by_local_id(local_id: int) -> XboxResult  # data: XboxUser
find_user_for_device(device_id: String) -> XboxResult  # data: XboxUser
find_controller_for_user_with_ui_async(user: XboxUser) -> Signal
get_device_associations() -> Array                 # [{device_id, user_local_id}]
get_devices_for_user(user: XboxUser) -> PackedStringArray
get_default_audio_endpoint(user: XboxUser, kind := XboxUsers.AUDIO_ENDPOINT_KIND_COMMUNICATION_RENDER) -> XboxResult
check_privilege_async(user: XboxUser, privilege: int) -> Signal
resolve_privilege_with_ui_async(user: XboxUser, privilege: int) -> Signal
resolve_issue_with_ui_async(user: XboxUser, url := "") -> Signal
get_gamer_picture_async(user: XboxUser, size := "medium") -> Signal
get_token_and_signature_async(user: XboxUser, method: String, url: String, headers := {}, body := PackedByteArray(), force_refresh := false) -> Signal
```

##### Signals

```gdscript
user_changed(user: XboxUser, change_kind: String)
device_association_changed(device_id: String, old_user_local_id: int, new_user_local_id: int)
default_audio_endpoint_changed(user_local_id: int, kind: int, endpoint_id: String)
```

##### `XboxUser`

```gdscript
get_local_id() -> int
get_xuid() -> String
get_gamertag() -> String                  # classic component
get_modern_gamertag() -> String
get_modern_gamertag_suffix() -> String
get_unique_modern_gamertag() -> String
get_age_group() -> XboxUser.AgeGroup
get_age_group_name() -> String
get_sign_in_state() -> XboxUser.SignInState
get_sign_in_state_name() -> String
is_guest() -> bool
is_signed_in() -> bool
is_store_user() -> bool
is_valid() -> bool
is_same_user(other: XboxUser) -> bool
duplicate_user() -> XboxUser
```

##### `XboxUserSignOutDeferral`

```gdscript
is_valid() -> bool
release() -> void
```

##### Native API mapping

| Wrapper/API | Native API(s) | Notes |
| --- | --- | --- |
| `add_default_user_async()` | `XUserAddAsync`, `XUserAddResult` | Uses the silent default-user path without guest support; on success, populate `XboxUser`, create `XblContextHandle`, and ensure change notifications are registered. |
| `add_user_with_ui_async(allow_guests := false)` | `XUserAddAsync`, `XUserAddResult` | Interactive add path. **Requires the advanced user model** — under the simplified (PC-default) user model `XUserAddAsync` rejects every interactive option (`None`, `AddDefaultUserAllowingUI`, `AllowGuests`) with `E_INVALIDARG`; only the silent `add_default_user_async()` works there. By default (`allow_guests = false`) uses `AddDefaultUserAllowingUI` to resolve the launching default user with the system sign-in UI. Pass `allow_guests = true` to open the full account picker (`AllowGuests`, without the default/silent options) so the player can choose any account, including guests. Whether dismissing the system UI completes the request is GDK platform behavior; when the platform reports cancellation (`E_ABORT`) the wrapper normalizes it to a cancelled `XboxResult`. Post-processing matches the default-user path except that later adds do not replace the session primary user once one already exists. |
| `add_user_by_id_with_ui_async(xuid)` | `XUserAddByIdWithUiAsync`, `XUserAddByIdWithUiResult` | Re-establishes one specific account by decimal XUID, showing UI only if that account needs attention. This is the XR-112 "resume the prior user's session" path: after resume the title asks for the same user it had rather than opening a blind account picker. Post-processing is identical to the other add paths. |
| `get_max_users()` | `XUserGetMaxUsers` | Synchronous capability probe returning the platform's simultaneous local-user cap in `XboxResult.data`. |
| `is_sign_out_available()` | `XUserIsSignOutPresent` | Synchronous `bool`. It is the documented gate for `sign_out_async()`; the wrapper also enforces it and fails with `sign_out_not_available` when the platform has no title-driven sign-out. |
| `sign_out_async(user)` | `XUserSignOutAsync`, `XUserSignOutResult` | On success the wrapper reconciles the users cache itself instead of waiting for the `XUserChangeEvent::SignedOut` callback, so `get_users()`/`get_primary_user()` are already correct when the completion `Signal` resolves. Whichever path (completion or change event) runs first removes the user and emits `user_changed(user, "removed")` exactly once. |
| `acquire_sign_out_deferral()` | `XUserGetSignOutDeferral`, `XUserCloseSignOutDeferralHandle` | Returns an `XboxUserSignOutDeferral` in `XboxResult.data`, mirroring `XboxDisplayTimeoutDeferral`. The handle is closed on `release()` or when the wrapper is freed; closing does not require an initialized runtime, so `release()` is safe after `GDK.shutdown()`. |
| `find_user_by_xuid(xuid)` | `XUserFindUserById` | Synchronous, non-prompting lookup of a signed-in user. Used on resume to validate that an expected user is still signed in. When the found user is already cached, the wrapper returns the cached `XboxUser` (matching object identity with `get_users()`) and releases the freshly opened handle. |
| `find_user_by_local_id(local_id)` | `XUserFindUserByLocalId` | Same contract as `find_user_by_xuid()`; the natural way to resolve a local id delivered by `device_association_changed`. |
| `find_user_for_device(device_id)` | `XUserFindForDevice` | Resolves the user currently paired with a device id. `device_id` is the 64-character lowercase hex encoding of `APP_LOCAL_DEVICE_ID`. |
| `find_controller_for_user_with_ui_async(user)` | `XUserFindControllerForUserWithUiAsync`, `XUserFindControllerForUserWithUiResult` | **XR-112 requirement.** Opens the system controller-selection dialog when a user has no assigned controller at activation or after resume. Successful results carry `device_id` and `has_device` (false when the platform returns `XUserNullDeviceId`). |
| `get_device_associations()`, `get_devices_for_user()`, `device_association_changed` | `XUserRegisterForDeviceAssociationChanged`, `XUserUnregisterForDeviceAssociationChanged` | One runtime-wide registration against the shared queue, installed during `GDK.initialize()`. The platform replays every existing pairing when the registration is added, so the service's association cache is the authoritative user↔controller view XR-112 requires. Registration failure is **non-fatal**: it is reported through `GDK.runtime_error` and leaves the cache empty rather than failing initialization. A local id of `0` means "no user" and removes the device from the cache. |
| `get_default_audio_endpoint()`, `default_audio_endpoint_changed` | `XUserGetDefaultAudioEndpointUtf16`, `XUserRegisterForDefaultAudioEndpointUtf16Changed`, `XUserUnregisterForDefaultAudioEndpointUtf16Changed` | Per-user default communication render/capture endpoint id. The change registration is installed and torn down alongside the device-association registration and is non-fatal in the same way. |
| `check_privilege_async()` | `XUserCheckPrivilege` | There is no documented async privilege-check API in `XUser`; this wrapper should convert the synchronous result into a deferred completion `Signal`. Successful results carry a `Dictionary` in `XboxResult.data` with `privilege`, `has_privilege`, `deny_reason`, and `deny_reason_value`. If the check returns `E_GAMEUSER_RESOLVE_USER_ISSUE_REQUIRED`, the request should fail and direct callers to `resolve_issue_with_ui_async()`. |
| `resolve_privilege_with_ui_async()` | `XUserResolvePrivilegeWithUiAsync`, `XUserResolvePrivilegeWithUiResult` | Use the native UI remediation path when `check_privilege_async()` reports that a privilege is denied and the title wants to let the player resolve it immediately. Successful results can carry a small `Dictionary` payload that echoes the resolved privilege id. |
| `resolve_issue_with_ui_async()` | `XUserResolveIssueWithUiAsync`, `XUserResolveIssueWithUiResult` | This is the remediation path for `E_GAMEUSER_RESOLVE_USER_ISSUE_REQUIRED` from `XUser` getters such as age-group lookups and from privilege checks. Treat an empty `url` as the Xbox services/default flow and pass a URL only when the underlying issue is request-specific. |
| `get_gamer_picture_async()` | `XUserGetGamerPictureAsync`, `XUserGetGamerPictureResultSize`, `XUserGetGamerPictureResult` | Accept `small`, `medium`, `large`, and `extra_large` size strings. Decode the returned PNG bytes into a Godot `Image` and place that `Image` in `XboxResult.data`. |
| `get_token_and_signature_async()` | `XUserGetTokenAndSignatureAsync`, `XUserGetTokenAndSignatureResultSize`, `XUserGetTokenAndSignatureResult` | First-pass token support should expose explicit request parameters: HTTP method, full URL, headers `Dictionary`, optional `PackedByteArray` body, and `force_refresh`. Successful results carry a `Dictionary` with `token` and `signature`. |
| `user_changed` | `XUserRegisterForChangeEvent`, `XUserUnregisterForChangeEvent` | Register one runtime-wide change callback against the shared queue and reconcile affected `XboxUser` wrappers by local id. `user_changed` is the only public users-service event and should emit the affected wrapper plus a snake_case `change_kind` string: `added`, `removed`, `signed_in_again`, `gamertag`, `gamer_picture`, or `privileges`. For `removed`, emit the removed wrapper after it has been removed from the users cache so handlers can read identity fields without seeing it in `get_users()`. |
| `XboxUser` getters | `XUserGetLocalId`, `XUserGetId`, `XUserGetGamertag`, `XUserGetAgeGroup`, `XUserGetIsGuest`, `XUserGetState`, `XUserIsStoreUser` | Pure wrapper accessors with no extra service traffic. Expose age-group and sign-in state as Godot enums with bound constants on `XboxUser`, and provide `get_age_group_name()` / `get_sign_in_state_name()` for human-readable snake_case strings such as `adult` and `signed_in`. `XUserGetGamertag` is read once per component: `get_gamertag()` is the required `Classic` component and fails the populate, while the `Modern`, `ModernSuffix`, and `UniqueModern` components are optional metadata that degrade to empty strings on accounts or platforms that do not expose them. |
| `XboxUser.is_same_user()`, `XboxUser.duplicate_user()` | `XUserCompare`, `XUserDuplicateHandle` | `is_same_user()` compares two handles so a cached wrapper and a fresh `find_user_*()` lookup for the same account compare equal. `duplicate_user()` hands back an independently owned handle for titles that need a user to outlive the service cache. |

Each `XboxUser` should own an `XUserHandle` and an `XblContextHandle`. The runtime should own a single change-event registration token. Cleanup order should be: unregister runtime change notifications, remove the user from service-owned caches/managers, close the Xbox services context, then call `XUserCloseHandle`. The first successful user add establishes the session primary user; later adds should not promote a different cached user to primary.

##### Intentionally unwrapped `XUser` APIs

| Native API(s) | Why it stays unwrapped |
| --- | --- |
| `XUserGetTokenAndSignatureUtf16Async` / `...Utf16ResultSize` / `...Utf16Result`, `XUserResolveIssueWithUiUtf16Async` / `...Utf16Result` | UTF-16 duplicates of surfaces already wrapped in their UTF-8 form. Godot `String` conversion happens inside the wrapper, so a second binding would add a redundant public surface with identical semantics. |
| `XUserGetMsaTokenSilentlyAsync` / `...Result` / `...ResultSize` | Deprecated by the GDK (`__declspec(deprecated)`); no longer supported. |
| `XUserPlatformRemoteConnectSetEventHandlers`, `XUserPlatformRemoteConnectCancelPrompt`, `XUserPlatformSpopPromptSetEventHandlers`, `XUserPlatformSpopPromptComplete` | Platform-implementer hooks for hosting the sign-in prompt UI, not title-facing APIs. Out of scope for a Godot title-side addon. |
| `XUserDuplicateHandle`, `XUserCloseHandle`, `XUserCompare`, `XUserCloseSignOutDeferralHandle` | Handle lifetime primitives. These are used internally by `XboxUser` / `XboxUserSignOutDeferral` and surface as `duplicate_user()`, `is_same_user()`, `release()`, and normal Godot `RefCounted` lifetime — raw handles are never exposed to GDScript. |

##### XR-112 coverage

[XR-112](https://learn.microsoft.com/gaming/gdk/docs/store/policies/xr/xr112) ("Establishing a User and Controller During Initial Activation and Resume") is the certification requirement this service must make satisfiable. Mapping of the requirement's obligations to the public surface:

| XR-112 obligation | Public surface |
| --- | --- |
| Simplified user model: acquire the launching user silently | `add_default_user_async()` |
| Advanced user model: prompt for a user when no default is present, and provide an entry point to the account picker | `add_user_with_ui_async(allow_guests := true)` |
| Ensure a controller is assigned to the user; engage the system dialog when none is | `get_devices_for_user()` → `find_controller_for_user_with_ui_async()` |
| Respect the controller/user binding set by the platform or account picker | `get_device_associations()`, `find_user_for_device()` |
| React to controller re-pairing on resume | `device_association_changed` |
| Validate that the expected user is still signed in after resume | `find_user_by_xuid()`, `find_user_by_local_id()`, `XboxUser.is_signed_in()` |
| Re-establish the prior user rather than opening a blind picker | `add_user_by_id_with_ui_async(xuid)` |
| Display the gamertag before any profile-related action | `XboxUser.get_gamertag()` / `get_unique_modern_gamertag()`, `get_gamer_picture_async()` |
| Flush per-user state when the platform reports a pending sign-out | `user_changed` (`removed`), `acquire_sign_out_deferral()` |

The addon deliberately does not implement the requirement *for* the title: it does not auto-open the controller dialog or auto-re-add users, because the correct reaction is game-design dependent (XR-112 allows removing the player, showing a picker, or resuming). The addon's contract is that every decision the requirement asks a title to make is observable and actionable from GDScript.

###### Known issue: shutdown during an in-flight user async

Calling `GDK.shutdown()` while an `XUser` async operation is still in flight terminates the process with exit code `0x80004004` (`E_ABORT`) — no crash dump and no Windows event-log entry. `XboxRuntime::shutdown()` cancels pending signals, force-completes them, calls `XTaskQueueTerminate(queue, false, ...)`, then drains the completion port and closes the handle; the XAsync completion thunk runs during that post-terminate drain and deletes its own context.

This is pre-existing (reproduced against the commit before the XUser wrapper work landed) and is **not** caused by the wrappers, but it is directly relevant to XR-112: the resume path is exactly where a title is most likely to shut the runtime down with sign-in work outstanding. Two consequences today:

- A title must let any pending `add_*_async()` / `find_controller_for_user_with_ui_async()` settle before calling `GDK.shutdown()`.
- The GDK coverage host therefore leaves `runtime/auto_add_primary_user` at `false`, so the bootstrap's startup sign-in cannot race GUT's per-test `reset_runtime()`. See `tests/godot/gdk/project.godot`.

The fix belongs in `XboxRuntime::shutdown()` (drain or explicitly cancel-and-await in-flight XAsync contexts before terminating the queue) and is tracked as follow-up work.

#### `GDK.accessibility` service

##### Methods

```gdscript
query_closed_caption_properties() -> XboxResult
set_closed_caption_enabled(enabled: bool) -> XboxResult
query_high_contrast_mode() -> XboxResult
```

##### Notes

- Scope is intentionally limited to concrete APIs verified in public PC GDK docs/headers for `_GAMING_DESKTOP`.
- Do not add unrelated families in this service (`XGameStreaming`, `XPersistentLocalStorage`, `XNetworking`, console-only `XAppCapture`).
- Speech-to-text overlay APIs are deferred for manual/UI-focused follow-up coverage.

##### Native API mapping

| Wrapper/API | Native API(s) | Notes |
| --- | --- | --- |
| `query_closed_caption_properties()` | `XClosedCaptionGetProperties` | Returns an `XboxClosedCaptionProperties` wrapper in `XboxResult.data`. |
| `set_closed_caption_enabled()` | `XClosedCaptionSetEnabled` | Returns an ok/error `XboxResult`; success payload includes `enabled`. |
| `query_high_contrast_mode()` | `XHighContrastGetMode` | Returns a `Dictionary` payload with `mode` and `mode_name`. |
| `XboxClosedCaptionProperties` getters | `XClosedCaptionProperties` struct fields | Wrapper exposes Godot-native colors/enums/flags without exposing native handles. |

#### `GDK.game_ui` service

##### Methods

```gdscript
show_message_dialog_async(title: String, message: String, first_button := "OK", second_button := "", third_button := "", default_button := "first", cancel_button := "first") -> Signal
set_notification_position_hint(position: String) -> XboxResult
show_player_profile_card_async(requesting_user: XboxUser, target_xuid: String) -> Signal
show_player_picker_async(requesting_user: XboxUser, prompt: String, selectable_xuids: PackedStringArray, preselected_xuids := PackedStringArray(), min_selection_count := 1, max_selection_count := 1) -> Signal
resolve_privilege_with_ui_async(user: XboxUser, privilege: int) -> Signal
show_achievements_async(requesting_user: XboxUser) -> Signal
show_error_dialog_async(error_code: int, context := "") -> Signal
show_send_game_invite_async(requesting_user: XboxUser, session_configuration_id: String, session_template_name: String, session_id: String, invitation_text := "", custom_activation_context := "") -> Signal
show_text_entry_async(title_text := "", description_text := "", default_text := "", input_scope := "default", max_text_length := 0) -> Signal
```

##### Notes

- This service should expose only APIs verified as available in the public PC GDK (`_GAMING_DESKTOP`) headers/libs used by this repo.
- Do not add wrappers for console-only or unavailable surfaces (for example `XGameStreaming`, `XPersistentLocalStorage`, `XNetworking`, or console-only `XAppCapture` flows).
- `show_message_dialog_async()` and `show_player_picker_async()` should distinguish user-cancelled flows (`E_ABORT`) from other native failures.
- Keep `GDK.multiplayer_activity.show_invite_ui_async()` compatible; that API remains callable through the multiplayer-activity service.

##### Native API mapping

| Wrapper/API | Native API(s) | Notes |
| --- | --- | --- |
| `show_message_dialog_async()` | `XGameUiShowMessageDialogAsync`, `XGameUiShowMessageDialogResult` | Validate title/message/button layout before invoking native UI. Return button selection in `XboxResult.data` on success. |
| `set_notification_position_hint()` | `XGameUiSetNotificationPositionHint` | Accept snake_case positions (`bottom_center`, etc.). This is available on PC GDK even where the shell may ignore placement hints. |
| `show_player_profile_card_async()` | `XGameUiShowPlayerProfileCardAsync`, `XGameUiShowPlayerProfileCardResult` | Requires a signed-in `XboxUser` requesting handle and numeric target XUID. |
| `show_player_picker_async()` | `XGameUiShowPlayerPickerAsync`, `XGameUiShowPlayerPickerResultCount`, `XGameUiShowPlayerPickerResult` | Validate XUID lists and selection ranges up front; return selected XUIDs in `XboxResult.data`. |
| `resolve_privilege_with_ui_async()` | `XUserResolvePrivilegeWithUiAsync`, `XUserResolvePrivilegeWithUiResult` | Delegate to `GDK.users` privilege-remediation flow so existing users-service behavior remains authoritative. |
| `show_achievements_async()` | `XGameUiShowAchievementsAsync`, `XGameUiShowAchievementsResult` | Title ID resolved via `XGameGetXboxTitleId`. Requires a signed-in `XboxUser`. |
| `show_error_dialog_async()` | `XGameUiShowErrorDialogAsync`, `XGameUiShowErrorDialogResult` | Takes an HRESULT `error_code` plus optional `context` text. |
| `show_send_game_invite_async()` | `XGameUiShowSendGameInviteAsync`, `XGameUiShowSendGameInviteResult` | Requires session configuration/template/id; optional invitation text and custom activation context. Title owns the MPSD session identifiers. |
| `show_text_entry_async()` | `XGameUiShowTextEntryAsync`, `XGameUiShowTextEntryResultSize`, `XGameUiShowTextEntryResult` | Gamepad/virtual-keyboard text entry. `input_scope` maps to `XGameUiTextEntryInputScope`; returns the entered text in `XboxResult.data.text`. |

> Excluded from `GDK.game_ui` (engine/host overlap): `XGameUiShowStateShareAsync` and `XGameUiShowWebAuthenticationAsync`/`WithOptions` — Godot already provides web-auth/state-share equivalents.

#### `GDK.achievements` service

##### Methods

```gdscript
query_player_achievements_async(user: XboxUser) -> Signal
update_achievement_async(user: XboxUser, achievement_id: String, percent_complete: int) -> Signal
get_cached_achievements(user: XboxUser) -> Array
get_achievements_by_state(user: XboxUser, progress_state: String) -> XboxResult  # data.achievements: Array[XboxAchievement]
```

##### Signals

```gdscript
achievement_unlocked(user: XboxUser, achievement_id: String)
achievements_updated(user: XboxUser)
runtime_error(result: XboxResult)  # unsolicited achievement-service errors
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
| `get_achievements_by_state()` | `XblAchievementsManagerGetAchievementsByState`, `XblAchievementsManagerResultGetAchievements`, `XblAchievementsManagerResultCloseHandle` | Filters the cached achievements by progress state (`Achieved`/`NotStarted`/`InProgress`/`Unknown`); returns `achievements_not_loaded` until the user's cache is warmed by `query_player_achievements_async()`. |
| `achievement_unlocked` / `achievements_updated` | `XblAchievementsManagerDoWork` | These signals come from manager update events, not from a separate polling or REST-style query path. |

The manager result handles should be copied into extension-owned data immediately on dispatch, because they are cache views rather than long-lived script-safe objects.

#### `GDK.package` service

##### Methods

```gdscript
enumerate_packages(package_kind := XboxPackage.PACKAGE_KIND_CONTENT, scope := XboxPackage.ENUMERATION_SCOPE_THIS_AND_RELATED) -> XboxResult
find_package_by_identifier(package_identifier: String, package_kind := XboxPackage.PACKAGE_KIND_CONTENT, scope := XboxPackage.ENUMERATION_SCOPE_THIS_AND_RELATED) -> XboxResult
get_current_process_package_identifier() -> XboxResult
mount_package_async(package_identifier: String) -> Signal
load_resource_pack_async(package_identifier: String, pack_relative_path: String, replace_files := false, offset := 0) -> Signal
get_loaded_resource_packs() -> Array
get_install_progress(package_identifier: String) -> XboxResult
```

##### Notes

- `mount_package_async()` returns an `XboxPackageMount` for temporary loose-file access under an open mount handle.
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
query_user_stats_async(user: XboxUser, stat_names := PackedStringArray()) -> Signal
query_users_stats_async(user: XboxUser, xuids: PackedStringArray, stat_names := PackedStringArray()) -> Signal
get_single_stat_async(user: XboxUser, stat_name: String) -> Signal
set_stat_number(user: XboxUser, stat_name: String, value: float) -> XboxResult
set_stat_integer(user: XboxUser, stat_name: String, value: int) -> XboxResult
flush_stats_async(user: XboxUser) -> Signal
write_stats_async(user: XboxUser, stats: Dictionary) -> Signal
delete_stats_async(user: XboxUser, stat_names: PackedStringArray) -> Signal
track_stats(user: XboxUser, stat_names: PackedStringArray) -> XboxResult
stop_tracking_stats(user: XboxUser, stat_names := PackedStringArray()) -> XboxResult
get_cached_stats(user: XboxUser) -> Dictionary
```

##### Signals

```gdscript
stats_updated(user: XboxUser, stats: Dictionary)
stat_changed(user: XboxUser, stat_name: String, value: Variant)
stats_flushed(user: XboxUser, result: XboxResult)
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
| `get_single_stat_async()` | `XblUserStatisticsGetSingleUserStatisticAsync`, `XblUserStatisticsGetSingleUserStatisticResultSize`, `XblUserStatisticsGetSingleUserStatisticResult` | Reads one named stat for the signed-in user; returns the same result shape as the plural read with a single entry. |
| `query_users_stats_async()` | `XblUserStatisticsGetMultipleUserStatisticsAsync`, `XblUserStatisticsGetMultipleUserStatisticsResultSize`, `XblUserStatisticsGetMultipleUserStatisticsResult` | Use one local user context to read the requested stats for a target XUID list. |
| `set_stat_number()` / `set_stat_integer()` | extension-local staging only | Convert staged values into `XblTitleManagedStatistic` payloads. No native call is made until `flush_stats_async()`. |
| `flush_stats_async()` | `XblTitleManagedStatsUpdateStatsAsync`, `XblTitleManagedStatistic`, `XblTitleManagedStatType` | Flushes staged values through the documented title-managed stats write API (merge semantics). |
| `write_stats_async()` | `XblTitleManagedStatsWriteAsync`, `XblTitleManagedStatistic`, `XblTitleManagedStatType` | Replace-all write: the supplied `stats` Dictionary becomes the complete title-managed stat document for the user. Live-write surface. |
| `delete_stats_async()` | `XblTitleManagedStatsDeleteStatsAsync` | Deletes the named title-managed stats for the user. Live-write surface. |
| `track_stats()` | `XblUserStatisticsAddStatisticChangedHandler`, `XblUserStatisticsTrackStatistics` | Register one change handler per local user context, then track the requested stats for that user's XUID. |
| `stop_tracking_stats()` | `XblUserStatisticsStopTrackingStatistics`, `XblUserStatisticsStopTrackingUsers`, `XblUserStatisticsRemoveStatisticChangedHandler` | Stop specific stat tracking or all tracked stat updates for the local user. |
| `get_cached_stats()` | service cache only | Cache ownership stays in the extension; it is hydrated by explicit reads and tracked-stat change callbacks. |
| `stats_updated` / `stat_changed` / `stats_flushed` | Service cache plus statistic-change handlers | Query and tracked-stat callbacks update the cache before completion/notification; title-managed write completion drives `stats_flushed`. |

Earlier undocumented stats-family references were removed during audit. The documented write family is `title_managed_statistics_c`; the documented read and tracking family is `user_statistics_c`.

#### `GDK.leaderboards` service

##### Methods

```gdscript
get_leaderboard_async(user: XboxUser, stat_name: String, max_items := 25) -> Signal
get_leaderboard_around_user_async(user: XboxUser, stat_name: String, max_items := 25) -> Signal
get_social_leaderboard_async(user: XboxUser, stat_name: String, max_items := 25) -> Signal
get_next_page_async(leaderboard: XboxLeaderboard) -> Signal
get_cached_leaderboard(stat_name: String) -> XboxLeaderboard
```

##### Signals

```gdscript
leaderboard_updated(stat_name: String, leaderboard: XboxLeaderboard)
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
| pagination inside `XboxLeaderboard` | `XblLeaderboardResultGetNextAsync`, `XblLeaderboardResultGetNextResultSize`, `XblLeaderboardResultGetNextResult` | Store continuation state in the wrapper so GDScript can request another page later without exposing native handles. |
| `get_cached_leaderboard()` | service cache only | Return the most recent translated leaderboard snapshot. |

#### `GDK.privacy` service

##### Methods

```gdscript
check_permission_async(user: XboxUser, permission: String, target_xuid: String) -> Signal
check_permission_for_anonymous_user_async(user: XboxUser, permission: String, anonymous_user_type: String) -> Signal
batch_check_permission_async(user: XboxUser, permission: String, target_xuids: PackedStringArray) -> Signal
get_avoid_list_async(user: XboxUser) -> Signal
get_mute_list_async(user: XboxUser) -> Signal
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
set_presence_async(user: XboxUser, state: String, rich_presence := {}) -> Signal
clear_presence_async(user: XboxUser) -> Signal
get_presence_async(xuids: PackedStringArray) -> Signal
get_presence_for_social_group_async(user: XboxUser, social_group: String) -> Signal
track_presence(user: XboxUser, xuids: PackedStringArray, title_ids := PackedInt64Array()) -> XboxResult
stop_tracking_presence(user: XboxUser, xuids := PackedStringArray(), title_ids := PackedInt64Array()) -> XboxResult
get_cached_presence(xuid: String) -> XboxPresenceRecord
```

##### Signals

```gdscript
presence_changed(xuid: String, presence: XboxPresenceRecord)
local_presence_set(user: XboxUser)
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
start_social_graph(user: XboxUser) -> XboxResult
stop_social_graph(user: XboxUser) -> void
get_friends_async(user: XboxUser) -> Signal
create_social_group(user: XboxUser, filter: XboxSocialFilter = null) -> XboxResult  # data: XboxSocialGroup
create_social_group_from_xuids(user: XboxUser, xuids: PackedStringArray) -> XboxResult  # data: XboxSocialGroup
update_social_user_group(group: XboxSocialGroup, xuids: PackedStringArray) -> XboxResult  # data.count: int
set_rich_presence_polling(user: XboxUser, enabled: bool) -> XboxResult  # data.enabled: bool
destroy_social_group(group: XboxSocialGroup) -> void
get_group_users(group: XboxSocialGroup) -> XboxResult  # data: Array[XboxSocialUser]
submit_reputation_feedback_async(user: XboxUser, target_xuid: String, feedback_type: String, reason := "", evidence_id := "") -> Signal
submit_batch_reputation_feedback_async(user: XboxUser, feedback_items: Array) -> Signal
```

##### Signals

```gdscript
social_graph_changed(user: XboxUser)
social_group_updated(group: XboxSocialGroup)
social_user_changed(xuid: String, social_user: XboxSocialUser)
runtime_error(result: XboxResult)  # unsolicited social-service errors
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
| `update_social_user_group()` | `XblSocialManagerUpdateSocialUserGroup` | Replaces the tracked-user list of a list-based group with a new XUID set. Filter-based groups cannot be updated this way. |
| `set_rich_presence_polling()` | `XblSocialManagerSetRichPresencePollingStatus`, `XblSocialManagerAddLocalUser` | Toggles Social Manager rich-presence polling for the user; starts the user's social graph first if needed. |
| `destroy_social_group()` | `XblSocialManagerDestroySocialUserGroup` | Releases the native social-group handle. |
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
enable_capture() -> XboxResult
disable_capture() -> XboxResult
record_diagnostic_clip_async(duration: float) -> Signal
take_diagnostic_screenshot_async(path_hint: String) -> Signal
create_metadata(reserved_bytes := 0) -> XboxCaptureMetaData
```

##### `XboxCaptureMetaData`

```gdscript
is_valid() -> bool
close() -> void
stop_all_states() -> XboxResult
get_remaining_storage_bytes() -> int
add_string_event(name: String, value: String, priority := PRIORITY_GAMEPLAY) -> XboxResult
add_double_event(name: String, value: float, priority := PRIORITY_GAMEPLAY) -> XboxResult
add_int32_event(name: String, value: int, priority := PRIORITY_GAMEPLAY) -> XboxResult
start_string_state(name: String, value: String, priority := PRIORITY_GAMEPLAY) -> XboxResult
start_double_state(name: String, value: float, priority := PRIORITY_GAMEPLAY) -> XboxResult
start_int32_state(name: String, value: int, priority := PRIORITY_GAMEPLAY) -> XboxResult
```

##### Notes

- `create_metadata()` returns `null` when the runtime is not initialized.
- PC GDK metadata APIs are process-wide/stateless; `XboxCaptureMetaData` is a script-side context that gates intentional writes rather than a native handle wrapper.
- All event/state methods validate the context (`invalid_metadata_handle`) and the name (`invalid_metadata_name`) before calling native APIs.
- `record_diagnostic_clip_async` and `take_diagnostic_screenshot_async` require Game Bar active on the device; they return `XboxResult` errors when unavailable.
- `on_runtime_initialized()` for capture has no side-effects beyond setting `m_runtime_ready`; it cannot fail.

##### Native API mapping

| Wrapper/API | Native API(s) | Notes |
| --- | --- | --- |
| `enable_capture()` | `XAppCaptureEnableRecord` | Synchronous. |
| `disable_capture()` | `XAppCaptureDisableRecord` | Synchronous. |
| `record_diagnostic_clip_async()` | `XAppCaptureRecordDiagnosticClip` | Native API is synchronous; wrapper returns a deferred completion signal to match the public async contract. |
| `take_diagnostic_screenshot_async()` | `XAppCaptureTakeDiagnosticScreenshot` | Native API is synchronous; wrapper returns a deferred completion signal to match the public async contract. |
| `create_metadata()` | none | Opens a script-side context for stateless metadata calls; `reserved_bytes` is retained for compatibility and ignored. |
| `XboxCaptureMetaData::close()` | none | Closes the script-side context. |
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
get_profile_async(user: XboxUser, xuid: String) -> Signal
get_profiles_async(user: XboxUser, xuids: PackedStringArray) -> Signal
get_profiles_for_social_group_async(user: XboxUser, social_group: String) -> Signal
```

##### Wrapper types

```gdscript
XboxUserProfile
```

##### Notes

- Methods require a signed-in local `XboxUser` to create the Xbox Services context.
- XUID arguments are accepted as decimal strings to match other public Xbox identity surfaces.
- Single-profile queries return an `XboxUserProfile` in `XboxResult.data`; list and social-group queries return an `Array[XboxUserProfile]`.
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
verify_string_async(user: XboxUser, text: String) -> Signal
verify_strings_async(user: XboxUser, strings: PackedStringArray) -> Signal
```

##### Notes

- Methods require a signed-in local `XboxUser` to create the Xbox Services context.
- `verify_string_async()` returns one dictionary in `XboxResult.data`.
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
get_quota_async(user: XboxUser, storage_type: String) -> Signal
list_blob_metadata_async(user: XboxUser, storage_type: String, blob_path := "", skip_items := 0, max_items := 25) -> Signal
get_next_blob_metadata_async(result: XboxTitleStorageBlobMetadataResult) -> Signal
download_blob_async(user: XboxUser, storage_type: String, blob_path: String) -> Signal
upload_blob_async(user: XboxUser, storage_type: String, blob_path: String, data: PackedByteArray, display_name := "", e_tag := "", match_condition := "not_used") -> Signal
delete_blob_async(user: XboxUser, storage_type: String, blob_path: String, e_tag := "", match_condition := "not_used") -> Signal
```

##### Wrapper types

```gdscript
XboxTitleStorageBlobMetadata
XboxTitleStorageBlobMetadataResult
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
configure_options(debugger_present_options := XboxErrorReporting.ERROR_OPTIONS_NONE, debugger_not_present_options := XboxErrorReporting.ERROR_OPTIONS_NONE) -> XboxResult
set_callback_enabled(enabled: bool) -> XboxResult
is_callback_enabled() -> bool
```

##### Signals

```gdscript
error_reported(result: XboxResult)
```

##### Notes

- Scope is intentionally limited to the public PC GDK `XError` callback/options surface.
- `XboxErrorReporting.ErrorOptions` mirrors the public `XErrorOptions` flag bits: `ERROR_OPTIONS_OUTPUT_DEBUG_STRING_ON_ERROR` (`0x1`), `ERROR_OPTIONS_DEBUG_BREAK_ON_ERROR` (`0x2`), and `ERROR_OPTIONS_FAIL_FAST_ON_ERROR` (`0x4`).
- Do not invent report-submission wrappers when no public PC GDK submission API is available.
- If callers attach custom metadata to downstream telemetry triggered by callback events, callers own privacy/compliance review for that metadata.

##### Native API mapping

| Wrapper/API | Native API(s) | Notes |
| --- | --- | --- |
| `configure_options()` | `XErrorSetOptions` | Uses `XboxErrorReporting.ErrorOptions` enum flags (including bitwise OR combinations) to configure debugger-present and debugger-absent option sets. |
| `set_callback_enabled()` | `XErrorSetCallback`, `XErrorCallback` | Registers/unregisters the callback bridge and forwards events to main-thread Godot signals via `GDK.dispatch()`. |
| `is_callback_enabled()` | wrapper state only | Reports whether callback forwarding is active. |

## Plugin settings

### Runtime

| Setting | Default | Purpose |
| --- | --- | --- |
| `gdk/runtime/initialize_on_startup` | `false` | Calls `GDK.initialize()` automatically during startup. |
| `gdk/runtime/embed_dispatch` | `true` | Dispatches GDK completions automatically from the main thread each frame. |
| `gdk/runtime/auto_add_primary_user` | `false` | Starts a default local-user flow after initialization. |
| `gdk/runtime/singleton_name` | `"GDK"` | Name the extension registers its Engine singleton under. Read once at extension load, before any project script runs, so it must be set in `project.godot` (or an `override.cfg`) — runtime writes take effect on the next launch. Blank values, non-identifiers, and names that collide with an already-registered singleton are rejected with a warning and the default is kept, so the addon is never left unreachable. Every in-repo consumer (bootstrap autoload, C# facade, GUT test base) resolves the singleton through this setting, class-checks the result so a name colliding with an unrelated engine singleton is not mistaken for the runtime, and retries the default name, so a rename needs no further changes. |

There are no `gdk/services/enable_*` Project Settings. The runtime settings
above are the only `godot_gdk` settings registered by the addon; public service
objects are constructed as part of the single `GDK` root singleton and are not
gated by per-service enable flags.

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
