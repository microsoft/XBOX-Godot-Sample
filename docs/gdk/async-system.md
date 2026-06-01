# Godot Microsoft GDK async system

This document explains how the `godot_gdk` async system works today: the shared runtime, the generic async wrappers, the internal `XAsync` bridge, the shared XBOX services scaffold, and the current concrete services built on top of it (`GDK.users`, `GDK.system`, `GDK.game_ui`, `GDK.accessibility`, `GDK.achievements`, `GDK.package`, `GDK.stats`, `GDK.leaderboards`, `GDK.privacy`, `GDK.presence`, `GDK.social`, `GDK.profile`, `GDK.string_verify`, `GDK.title_storage`, `GDK.error_reporting`, `GDK.activation`, `GDK.multiplayer_activity`, `GDK.capture`, `GDK.launcher`, and `GDK.store`).

For the plugin-wide view, including build, editor tooling, sample integration, and current scope boundaries, see [`gdk/plugin.md`](plugin.md).

## Why this exists

Microsoft GDK async APIs are queue- and callback-driven. Godot script APIs are signal- and `await`-driven.

The system in `addons\godot_gdk\src\` exists to bridge those two models without exposing raw `XAsyncBlock`, `XTaskQueueHandle`, or `XUserHandle` values to GDScript.

The current baseline gives us:

- one root singleton: `GDK`
- one shared native async runtime: `GDKRuntime`
- one script-facing one-shot completion shape: `Signal`
- one normalized result type: `GDKResult`
- one internal `XAsync` bridge base: `GDKSignalXAsyncContext`
- one internal one-shot signal helper: `GDKPendingSignal`
- one internal XBOX services scaffold: `GDKXboxServices`
- the concrete services that use this pattern across XBOX identity, services,
  package metadata, commerce, capture, launcher, error reporting, and
  system metadata (see [API reference](api-reference.md) for the
  full list)

## Public surface

### `GDK`

`GDK` is the only engine singleton registered by the extension. It owns the shared runtime and exposes the first service namespace.

Current public methods:

- `initialize(config := null) -> GDKResult`
- `shutdown() -> void`
- `is_available() -> bool`
- `is_initialized() -> bool`
- `dispatch() -> int`
- `get_users() -> GDKUsers`
- `get_system() -> GDKSystem`
- `get_game_ui() -> GDKGameUI`
- `get_accessibility() -> GDKAccessibility`
- `get_achievements() -> GDKAchievements`
- `get_package() -> GDKPackage`
- `get_stats() -> GDKStats`
- `get_leaderboards() -> GDKLeaderboards`
- `get_privacy() -> GDKPrivacy`
- `get_presence() -> GDKPresence`
- `get_social() -> GDKSocial`
- `get_store() -> GDKStore`
- `get_profile() -> GDKProfile`
- `get_string_verify() -> GDKStringVerify`
- `get_title_storage() -> GDKTitleStorage`
- `get_error_reporting() -> GDKErrorReporting`
- `get_launcher() -> GDKLauncher`
- `get_multiplayer_activity() -> GDKMultiplayerActivity`
- `get_capture() -> GDKCapture`
- `get_activation() -> GDKActivation`

Current public signals:

- `initialized()`
- `shutdown_completed()`
- `runtime_error(result: GDKResult)` — reserved for `XError` callback events. Caller-driven failures are returned as the per-call `GDKResult`; per-service unsolicited errors are emitted on `GDK.<service>.runtime_error` (e.g. `GDK.social.runtime_error`, `GDK.achievements.runtime_error`).

### `GDK.users`

`GDK.users` is a `RefCounted` service object returned by `GDK.get_users()`.

Current public methods:

- `add_default_user_async() -> Signal`
- `add_user_with_ui_async() -> Signal`
- `get_primary_user() -> GDKUser`
- `get_users() -> Array`
- `check_privilege_async(user, privilege) -> Signal`
- `resolve_privilege_with_ui_async(user, privilege) -> Signal`
- `resolve_issue_with_ui_async(user, url := "") -> Signal`
- `get_gamer_picture_async(user, size := "medium") -> Signal`
- `get_token_and_signature_async(user, method, url, headers := {}, body := PackedByteArray(), force_refresh := false) -> Signal`

Current public signals:

- `user_changed(user: GDKUser, change_kind: String)`

`user_changed` is the only public users-service event. `change_kind` is `added`, `removed`, `signed_in_again`, `gamertag`, `gamer_picture`, or `privileges`; for `removed`, `user` identifies the removed user and is no longer present in `get_users()`.

Current `GDKUser` getters:

- `get_local_id() -> int`
- `get_xuid() -> String`
- `get_gamertag() -> String`
- `get_age_group() -> GDKUser.AgeGroup`
- `get_age_group_name() -> String`
- `get_sign_in_state() -> GDKUser.SignInState`
- `get_sign_in_state_name() -> String`
- `is_guest() -> bool`
- `is_signed_in() -> bool`
- `is_store_user() -> bool`

### `GDK.game_ui`

`GDK.game_ui` is a `RefCounted` service object returned by `GDK.get_game_ui()`.

Current public methods:

- `show_message_dialog_async(title, message, first_button := "OK", second_button := "", third_button := "", default_button := "first", cancel_button := "first") -> Signal`
- `set_notification_position_hint(position) -> GDKResult`
- `show_player_profile_card_async(requesting_user, target_xuid) -> Signal`
- `show_player_picker_async(requesting_user, prompt, selectable_xuids, preselected_xuids := PackedStringArray(), min_selection_count := 1, max_selection_count := 1) -> Signal`
- `resolve_privilege_with_ui_async(user, privilege) -> Signal`

UI-facing requests report native cancellations as `GDKResult.code == "cancelled"` where the native API provides that distinction (for example message dialog and player picker flows).

### `GDK.achievements`

`GDK.achievements` is a `RefCounted` service object returned by `GDK.get_achievements()`.

Current public methods:

- `query_player_achievements_async(user) -> Signal`
- `update_achievement_async(user, achievement_id, percent_complete) -> Signal`
- `get_cached_achievements(user) -> Array`

Current public signals:

- `achievement_unlocked(user: GDKUser, achievement_id: String)`
- `achievements_updated(user: GDKUser)`

### `GDK.presence`

`GDK.presence` is a `RefCounted` service object returned by `GDK.get_presence()`.

Current public methods:

- `set_presence_async(user, state, rich_presence := {}) -> Signal`
- `clear_presence_async(user) -> Signal`
- `get_presence_async(xuids) -> Signal`
- `get_cached_presence(xuid) -> GDKPresenceRecord`

Current public signals:

- `presence_changed(xuid: String, presence: GDKPresenceRecord)`
- `local_presence_set(user: GDKUser)`

Important current behavior:

- `state` is the configured rich-presence string ID for the title's SCID in Partner Center.
- `get_presence_async(xuids)` still requires a signed-in primary user because the XSAPI read needs a caller context.

### `GDK.social`

`GDK.social` is a `RefCounted` service object returned by `GDK.get_social()`.

Current public methods:

- `start_social_graph(user) -> GDKResult`
- `stop_social_graph(user) -> void`
- `get_friends_async(user) -> Signal`
- `create_social_group(user, filter := null) -> GDKResult` (`data` is the `GDKSocialGroup`)
- `create_social_group_from_xuids(user, xuids) -> GDKResult` (`data` is the `GDKSocialGroup`)
- `destroy_social_group(group) -> void`
- `get_group_users(group) -> GDKResult` (`data` is an `Array[GDKSocialUser]`)

Current public signals:

- `social_graph_changed(user: GDKUser)`
- `social_group_updated(group: GDKSocialGroup)`
- `social_user_changed(xuid: String, social_user: GDKSocialUser)`

### Completion signals

Every one-shot async API now returns a completion `Signal` that emits exactly one `GDKResult`.

Important behaviors:

- callers `await service.method_async()` directly
- immediate failures still return a completion signal
- same-turn completion is deferred so the returned signal cannot be missed
- runtime shutdown queues a cancelled completion for still-pending one-shot signals before the shared task queue is terminated

### `GDKResult`

`GDKResult` normalizes native status into a stable Godot-facing payload.

Fields:

- `ok: bool`
- `hresult: int`
- `code: String`
- `message: String`
- `data: Variant`

`data` carries the operation payload. In the current implementation, successful user-add calls complete with a `GDKUser` in `data`, privilege and token/signature calls complete with `Dictionary` payloads, gamer-picture requests complete with a Godot `Image`, successful achievement queries/updates complete with cached `GDKAchievement` data in `data`, successful presence queries complete with an `Array` of `GDKPresenceRecord`, successful friends-group queries complete with a `GDKSocialGroup`, and successful store-license queries complete with a `GDKStoreLicenseStatus`.

## File map

### Root/runtime

- `gdk.cpp` / `gdk.h`  
  Root singleton. Owns `GDKRuntime`, `GDKXboxServices`, and every concrete
  service (`GDKUsers`, `GDKSystem`, `GDKGameUI`, `GDKAccessibility`,
  `GDKAchievements`, `GDKPackage`, `GDKStats`, `GDKLeaderboards`,
  `GDKPrivacy`, `GDKPresence`, `GDKSocial`, `GDKStore`, `GDKProfile`,
  `GDKStringVerify`, `GDKTitleStorage`,   `GDKErrorReporting`, `GDKLauncher`, `GDKActivation`,
  `GDKMultiplayerActivity`, and `GDKCapture`).
- `gdk_runtime.cpp` / `gdk_runtime.h`  
  Shared Microsoft GDK runtime owner. Creates the queue, retains active pending signals, dispatches completions, and shuts everything down safely. During shutdown it cancels every retained pending signal and queues a cancelled completion so GDScript `await` sites are not stranded by queue teardown.

- `gdk_xbox_services.cpp` / `gdk_xbox_services.h`
  Shared XBOX services bootstrap. Derives the current-title SCID from `XGameGetXboxTitleId()`, initializes XSAPI, and caches per-user `XblContextHandle` objects.

### Generic async layer

- `gdk_result.cpp` / `gdk_result.h`
  Shared result type and HRESULT formatting helpers.

- `gdk_pending_signal.cpp` / `gdk_pending_signal.h`
  Internal one-shot completion emitter retained by the runtime until completion.

- `gdk_signal_xasync_context.cpp` / `gdk_signal_xasync_context.h`
  Internal base class that owns one `XAsyncBlock`, binds it to the shared queue, wires cancellation, and forwards the raw block to the operation-specific finalizer.

### Current concrete services

- `gdk_user.cpp` / `gdk_user.h`  
  `GDKUser`, `GDKUsers`, and the first concrete `XAsync` bridge context (`AddUserAsyncContext`).

- `gdk_system.cpp` / `gdk_system.h`  
  `GDKSystem` synchronous title/runtime metadata reads.

- `gdk_game_ui.cpp` / `gdk_game_ui.h`  
  `GDKGameUI` `XGameUI`-backed dialog, picker, and notification flows.

- `gdk_accessibility.cpp` / `gdk_accessibility.h`  
  `GDKAccessibility`, `GDKClosedCaptionProperties`, and synchronous `XAccessibility` reads.

- `gdk_achievement.cpp` / `gdk_achievement.h`  
  `GDKAchievement`, `GDKAchievements`, the Achievements Manager cache, and manager-driven completion signals.

- `gdk_package.cpp` / `gdk_package.h`  
  `GDKPackage`, `GDKPackageMount`, `GDKPackageResourcePack`, and `XPackage` enumeration / mount / DLC resource-pack flows.

- `gdk_stats.cpp` / `gdk_stats.h`  
  `GDKStats` XBOX Services user statistics with cache + tracking signals.

- `gdk_leaderboards.cpp` / `gdk_leaderboards.h`  
  `GDKLeaderboards`, `GDKLeaderboard`, `GDKLeaderboardColumn`, `GDKLeaderboardRow`, and read-only XBOX Services leaderboard queries.

- `gdk_privacy.cpp` / `gdk_privacy.h`  
  `GDKPrivacy` permission/avoid-list/mute-list reads.

- `gdk_presence.cpp` / `gdk_presence.h`  
  `GDKPresence`, `GDKPresenceRecord`, the presence cache, and XAsync-backed presence set/clear/query flows.

- `gdk_social.cpp` / `gdk_social.h`  
  `GDKSocial`, `GDKSocialFilter`, `GDKSocialGroup`, `GDKSocialUser`, and Social Manager-backed completion signals.

- `gdk_store.cpp` / `gdk_store.h`  
  `GDKStore`, `GDKStoreLicenseStatus`, the per-product license cache, and `XStore` license/refresh/purchase flows.

- `gdk_profile.cpp` / `gdk_profile.h`  
  `GDKProfile`, `GDKUserProfile`, and XBOX Services profile reads.

- `gdk_string_verify.cpp` / `gdk_string_verify.h`  
  `GDKStringVerify` XBOX Live string verification.

- `gdk_title_storage.cpp` / `gdk_title_storage.h`  
  `GDKTitleStorage`, `GDKTitleStorageBlobMetadata`, `GDKTitleStorageBlobMetadataResult`, and XBOX Services Title Storage blob/quota flows.

- `gdk_error_reporting.cpp` / `gdk_error_reporting.h`  
  `GDKErrorReporting` `XError` callback/options wrapper with `error_reported` mirrored through `GDK.runtime_error`.

- `gdk_launcher.cpp` / `gdk_launcher.h`  
  `GDKLauncher` `XLaunchUri`-only launcher with destination validation.

- `gdk_activation.cpp` / `gdk_activation.h`
  `GDKActivation` owns the single native `XGameActivationRegisterForEvent` subscription and fans out activation dictionaries to internal service listeners.

- `gdk_multiplayer_activity.cpp` / `gdk_multiplayer_activity.h`  
  `GDKMultiplayerActivity`, `GDKMultiplayerActivityInfo`, MPA cache, recent-players staging, and invite signals forwarded from `GDKActivation`.

- `gdk_capture.cpp` / `gdk_capture.h`  
  `GDKCapture`, `GDKCaptureMetaData`, and the PC-supported `XAppCapture` capture-state and metadata flows.

- `register_types.cpp`  
  Registers every public class listed above plus `GDKResult` and the
  internal `GDKPendingSignal`, then publishes the `GDK` singleton.

## Core model

### 1. One shared native queue

`GDKRuntime::initialize()` does two things:

1. calls `XGameRuntimeInitialize()`
2. creates one `XTaskQueue` with:
   - `ThreadPool` work dispatch
   - `Manual` completion dispatch

That split is the key design choice.

Native work can run off-thread, but Godot-visible completion is only surfaced on
the main-thread pump. By default `godot_gdk` registers a frame callback and
calls:

```gdscript
GDK.dispatch()
```

each process frame while `gdk/runtime/embed_dispatch` is enabled. Games can
still call `GDK.dispatch()` directly when that setting is disabled or when they
need deterministic control.

Internally `dispatch()` drains the queue with:

```cpp
XTaskQueueDispatch(m_task_queue, XTaskQueuePort::Completion, 0)
```

This keeps Godot-facing state changes tied to the main-thread pump instead of worker-thread callbacks.

For XBOX services features, `GDK.dispatch()` also pumps manager-driven state like `XblAchievementsManagerDoWork()`. So the same per-frame dispatch contract covers both `XAsync` completions and non-`XAsync` service feeds.

### 2. One `XAsyncBlock` per XAsync-backed request

Each concrete async request gets its own heap-allocated context object derived from `GDKSignalXAsyncContext` or completes through service-owned pending-signal state.

That base class owns:

- one `XAsyncBlock`
- the shared runtime pointer
- the `GDKPendingSignal`

Its constructor sets:

- `async.queue` to the shared runtime queue
- `async.context` to the context object itself
- `async.callback` to a single shared thunk

The important part is what the thunk does:

```cpp
void CALLBACK GDKSignalXAsyncContext::_completion_thunk(XAsyncBlock *p_async_block) {
    auto *context = static_cast<GDKSignalXAsyncContext *>(p_async_block->context);

    context->clear_cancel_handler();
    context->finalize(p_async_block);
    delete context;
}
```

The thunk does **not** try to decode results generically.

That is intentional. Each Microsoft GDK async API has its own result contract:

- some use `*Result()`
- some use `*ResultSize()` + `*Result()`
- some use manager state instead of a classic async result payload

So the base layer only handles lifetime, queue binding, and cancellation plumbing. The service-specific context owns result extraction.

### 3. `GDKRuntime` retains active requests

`GDKRuntime::retain_pending_signal()` keeps strong references to in-flight requests so GDScript can safely fire-and-forget signal-returning calls.

That matters because script is allowed to fire-and-forget:

```gdscript
GDK.users.add_default_user_async()
```

If the runtime did not retain the request, it could be destroyed before completion.

When a request completes, `GDKPendingSignal::complete()` runs its release hook and `GDKRuntime::release_pending_signal()` drops the retained reference.

### 4. XBOX services bootstrap is shared

`GDKXboxServices` exists so XBOX services features can share title metadata and per-user XSAPI context management.

It currently:

- calls `XGameGetXboxTitleId()`
- derives the Game OS SCID as a null GUID with the title id in the last 8 hex digits
- calls `XblInitialize(...)`
- creates and caches per-user `XblContextHandle` objects on demand
- tears XSAPI down before the main runtime queue shuts down

That avoids repeating title-id lookup and context creation in each service.

## Request flow

This is the current end-to-end flow for `GDK.users.add_default_user_async()`:

1. GDScript calls `GDK.users.add_default_user_async()`.
2. `GDKUsers::_start_add_user_async()` checks that the runtime is initialized.
3. If the runtime is unavailable, it returns an already-scheduled error signal using `GDKRuntime::make_error_signal()`.
4. Otherwise it:
   - instantiates `GDKPendingSignal`
   - asks `GDKRuntime` to retain it
   - allocates `AddUserAsyncContext`
   - binds cancellation through `XAsyncCancel`
   - starts `XUserAddAsync(...)`
5. Microsoft GDK performs the work on the queue's work port.
6. Completion lands on the queue's completion port.
7. The completion only becomes visible when `GDK.dispatch()` pumps the queue.
8. `GDKSignalXAsyncContext::_completion_thunk()` forwards the raw `XAsyncBlock` to `AddUserAsyncContext::finalize(...)`.
9. `AddUserAsyncContext::finalize(...)` calls:

```cpp
XUserAddResult(p_async_block, &user_handle)
```

10. `GDKUsers::complete_add_user(...)` wraps the native handle in a `GDKUser`, updates service state, emits service signals, and only then completes the returned signal with:

```cpp
GDKResult::ok_result(user)
```

11. `GDKPendingSignal::complete()` emits `completed(result)`.
12. The runtime release hook drops the retained strong reference.

## Users service state

`GDKUsers` currently owns:

- `m_users` — the current local user wrappers
- `m_primary_user` — the active primary user
- one runtime-wide `XUserRegisterForChangeEvent` subscription

On successful user add:

1. `GDKUser::adopt_handle()` takes ownership of the returned `XUserHandle`
2. `_populate_from_handle()` reads:
   - local id
   - XUID
   - gamertag
   - age group
   - guest state
   - sign-in state
   - store-user state
3. `GDKUsers::complete_add_user()` updates the cache and emits `user_changed(user, "added")` for a newly cached user or `user_changed(user, "signed_in_again")` for a refreshed cached user.
4. only after those updates does it complete the returned signal

That ordering is important. Future services should follow the same rule: update cache first, then complete the returned request.

The newer users-service one-shot requests (`resolve_privilege_with_ui_async()`, `resolve_issue_with_ui_async()`, `get_gamer_picture_async()`, and `get_token_and_signature_async()`) now reuse that same retained `GDKPendingSignal` + `GDKSignalXAsyncContext` pattern. The only difference is the payload translation that happens in the concrete finalizer: `Dictionary` for privilege/token results and `Image` for gamer pictures.

## Achievements service state

`GDKAchievements` currently owns:

- per-user Achievements Manager registration state
- per-user cached `GDKAchievement` wrappers
- pending query ops waiting for `LocalUserInitialStateSynced`
- pending update ops waiting for `AchievementProgressUpdated` or `AchievementUnlocked`

Unlike `GDK.users`, this service does not create an `XAsyncBlock` per request. Instead, it adapts Achievements Manager's cache-and-event model into a completion-signal contract:

1. script requests a query or update
2. the service ensures the user is registered with Achievements Manager
3. the service returns a retained completion signal
4. `GDK.dispatch()` pumps `XblAchievementsManagerDoWork()`
5. manager events update the service cache
6. service signals are emitted
7. the pending completion signal resolves

That is the concrete example of the "manager state instead of a classic async result payload" rule described earlier.

## Cancellation and shutdown

### Request cancellation

`GDKPendingSignal` owns the shared cancel state for in-flight requests.

- For `GDKSignalXAsyncContext`, cancellation calls `XAsyncCancel(&m_async_block)`.
- For manager-driven waits such as achievements and social friends queries, the cancel handler removes the pending request from service-owned state and completes it with `GDKResult::cancelled(...)`.

### Runtime shutdown

`GDKRuntime::shutdown()`:

1. marks the runtime as shutting down
2. cancels every retained pending request
3. calls `XTaskQueueTerminate(...)`
4. dispatches the completion port until the queue termination callback fires
5. closes the queue handle
6. clears retained pending requests
7. leaves `XGameRuntimeUninitialize()` to the process-lifetime teardown in `~GDKRuntime()`

Because the runtime sets `m_shutting_down` first, service finalizers can refuse to mutate state during teardown. `GDK.shutdown()` intentionally does not call `XGameRuntimeUninitialize()`; tests and games may cycle initialize/shutdown multiple times in one process, while the matching native uninitialize runs once when the extension is torn down.

### Finalizer contract

Every `GDKSignalXAsyncContext::finalize(XAsyncBlock *)` implementation must short-circuit before result extraction or service/cache mutation when `get_runtime()->is_shutting_down()` or `get_pending_signal()->was_cancel_requested()` is true. The finalizer completes its pending signal with `GDKResult::cancelled(...)` and returns, so shutdown and explicit cancellation do not continue the success path after the runtime has started tearing down.

If a future finalizer must perform native cleanup during shutdown, keep the cancelled-result gate first and document the cleanup-only exception both inline and in this section.

## Why the base bridge does not use generic `XAsyncGetStatus`

This is the most important implementation rule for future work.

`GDKSignalXAsyncContext` is intentionally **not** a generic result decoder. It should not assume that:

- `XAsyncGetStatus()` is the real result contract
- a single status check is enough to finish every operation
- all async APIs can be handled without calling their own `*Result` / `*ResultSize` functions

Instead:

- the base layer handles shared mechanics
- the concrete context handles operation-specific extraction

That is why `AddUserAsyncContext::finalize(...)` takes the raw `XAsyncBlock *` and explicitly calls `XUserAddResult(...)`.

Future wrappers should follow the same pattern.

## How to add another async wrapper

When adding a new one-shot wrapper, follow this checklist:

1. Decide whether the wrapper is `XAsync`-backed or manager/dispatch-backed.
2. Add the public service method that returns a completion `Signal`.
3. For `XAsync`-backed work, create a service-specific context derived from `GDKSignalXAsyncContext`.
4. In `finalize(XAsyncBlock *p_async_block)`, first apply the [finalizer contract](#finalizer-contract) shutdown/cancellation gate.
5. Call the API-specific result functions.
6. Translate native payloads into Godot wrappers or Variants.
7. Update service-owned cache/state first.
8. Emit service-level signals next.
9. Complete the returned signal last.
10. Use `GDKRuntime::make_error_signal()` for immediate startup/availability failures.

For manager/event-driven waits like achievements and social friends queries, store a retained `GDKPendingSignal` in service-owned pending state and use a cancel handler that unregisters it immediately if the request is cancelled during teardown.

## Current scope

Today the system covers:

- runtime bootstrap and shutdown
- shared queue ownership
- retained pending-signal/result wrappers
- one reusable `XAsync` context base
- the full XBOX identity, XBOX services, package/DLC, commerce, capture,
  launcher, error-reporting, and system-metadata service set listed in
  [Public surface](#public-surface)

All shipped services share the same completion-signal pattern end to end.
Game Saves are intentionally not part of this addon; they live in
`godot_playfab` under `PlayFab.game_saves`. Server / admin / private Microsoft GDK
surfaces remain out of scope for the public PC client wrappers.
