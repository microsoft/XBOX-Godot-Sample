# Godot Microsoft GDK async system

This document explains how the `godot_gdk` async system works today: the shared runtime, the generic async wrappers, the internal `XAsync` bridge, the shared XBOX services scaffold, and the current concrete services built on top of it (`GDK.users`, `GDK.system`, `GDK.game_ui`, `GDK.accessibility`, `GDK.achievements`, `GDK.package`, `GDK.stats`, `GDK.leaderboards`, `GDK.privacy`, `GDK.presence`, `GDK.social`, `GDK.profile`, `GDK.string_verify`, `GDK.title_storage`, `GDK.error_reporting`, `GDK.activation`, `GDK.multiplayer_activity`, `GDK.capture`, `GDK.launcher`, `GDK.networking`, and `GDK.store`).

For the plugin-wide view, including build, editor tooling, sample integration, and current scope boundaries, see [`gdk/plugin.md`](plugin.md).

## Why this exists

Microsoft GDK async APIs are queue- and callback-driven. Godot script APIs are signal- and `await`-driven.

The system in `addons\godot_gdk\src\` exists to bridge those two models without exposing raw `XAsyncBlock`, `XTaskQueueHandle`, or `XUserHandle` values to GDScript.

The current baseline gives us:

- one root singleton: `GDK`
- one shared native async runtime: `XboxRuntime`
- one script-facing one-shot completion shape: `Signal`
- one normalized result type: `XboxResult`
- one internal `XAsync` bridge base: `XboxSignalXAsyncContext`
- one internal one-shot signal helper: `XboxPendingSignal`
- one internal XBOX services scaffold: `XboxServices`
- the concrete services that use this pattern across XBOX identity, services,
  package metadata, commerce, capture, launcher, error reporting, and
  system metadata (see [API reference](api-reference.md) for the
  full list)

## Public surface

### `GDK`

`GDK` is the only engine singleton registered by the extension. It owns the shared runtime and exposes the first service namespace.

Current public methods:

- `initialize(config := null) -> XboxResult`
- `shutdown() -> void`
- `is_available() -> bool`
- `is_initialized() -> bool`
- `dispatch() -> int`
- `get_users() -> XboxUsers`
- `get_system() -> XboxSystem`
- `get_game_ui() -> XboxGameUI`
- `get_accessibility() -> XboxAccessibility`
- `get_achievements() -> XboxAchievements`
- `get_package() -> XboxPackage`
- `get_stats() -> XboxStats`
- `get_leaderboards() -> XboxLeaderboards`
- `get_privacy() -> XboxPrivacy`
- `get_presence() -> XboxPresence`
- `get_social() -> XboxSocial`
- `get_store() -> XboxStore`
- `get_profile() -> XboxProfile`
- `get_string_verify() -> XboxStringVerify`
- `get_title_storage() -> XboxTitleStorage`
- `get_error_reporting() -> XboxErrorReporting`
- `get_launcher() -> XboxLauncher`
- `get_multiplayer_activity() -> XboxMultiplayerActivity`
- `get_capture() -> XboxCapture`
- `get_activation() -> XboxActivation`
- `get_networking() -> XboxNetworking`

Current public signals:

- `initialized()`
- `shutdown_completed()`
- `runtime_error(result: XboxResult)` — reserved for `XError` callback events. Caller-driven failures are returned as the per-call `XboxResult`; per-service unsolicited errors are emitted on `GDK.<service>.runtime_error` (e.g. `GDK.social.runtime_error`, `GDK.achievements.runtime_error`).

### `GDK.users`

`GDK.users` is a `RefCounted` service object returned by `GDK.get_users()`.

Current public methods:

- `add_default_user_async() -> Signal`
- `add_user_with_ui_async(allow_guests := false) -> Signal`
- `get_primary_user() -> XboxUser`
- `get_users() -> Array`
- `check_privilege_async(user, privilege) -> Signal`
- `resolve_privilege_with_ui_async(user, privilege) -> Signal`
- `resolve_issue_with_ui_async(user, url := "") -> Signal`
- `get_gamer_picture_async(user, size := "medium") -> Signal`
- `get_token_and_signature_async(user, method, url, headers := {}, body := PackedByteArray(), force_refresh := false) -> Signal`

Current public signals:

- `user_changed(user: XboxUser, change_kind: String)`

`user_changed` is the only public users-service event. `change_kind` is `added`, `removed`, `signed_in_again`, `gamertag`, `gamer_picture`, or `privileges`; for `removed`, `user` identifies the removed user and is no longer present in `get_users()`.

Current `XboxUser` getters:

- `get_local_id() -> int`
- `get_xuid() -> String`
- `get_gamertag() -> String`
- `get_age_group() -> XboxUser.AgeGroup`
- `get_age_group_name() -> String`
- `get_sign_in_state() -> XboxUser.SignInState`
- `get_sign_in_state_name() -> String`
- `is_guest() -> bool`
- `is_signed_in() -> bool`
- `is_store_user() -> bool`

### `GDK.game_ui`

`GDK.game_ui` is a `RefCounted` service object returned by `GDK.get_game_ui()`.

Current public methods:

- `show_message_dialog_async(title, message, first_button := "OK", second_button := "", third_button := "", default_button := "first", cancel_button := "first") -> Signal`
- `set_notification_position_hint(position) -> XboxResult`
- `show_player_profile_card_async(requesting_user, target_xuid) -> Signal`
- `show_player_picker_async(requesting_user, prompt, selectable_xuids, preselected_xuids := PackedStringArray(), min_selection_count := 1, max_selection_count := 1) -> Signal`
- `resolve_privilege_with_ui_async(user, privilege) -> Signal`

UI-facing requests report native cancellations as `XboxResult.code == "cancelled"` where the native API provides that distinction (for example message dialog and player picker flows).

### `GDK.achievements`

`GDK.achievements` is a `RefCounted` service object returned by `GDK.get_achievements()`.

Current public methods:

- `query_player_achievements_async(user) -> Signal`
- `update_achievement_async(user, achievement_id, percent_complete) -> Signal`
- `get_cached_achievements(user) -> Array`

Current public signals:

- `achievement_unlocked(user: XboxUser, achievement_id: String)`
- `achievements_updated(user: XboxUser)`

### `GDK.presence`

`GDK.presence` is a `RefCounted` service object returned by `GDK.get_presence()`.

Current public methods:

- `set_presence_async(user, state, rich_presence := {}) -> Signal`
- `clear_presence_async(user) -> Signal`
- `get_presence_async(xuids) -> Signal`
- `get_cached_presence(xuid) -> XboxPresenceRecord`

Current public signals:

- `presence_changed(xuid: String, presence: XboxPresenceRecord)`
- `local_presence_set(user: XboxUser)`

Important current behavior:

- `state` is the configured rich-presence string ID for the title's SCID in Partner Center.
- `get_presence_async(xuids)` still requires a signed-in primary user because the XSAPI read needs a caller context.

### `GDK.social`

`GDK.social` is a `RefCounted` service object returned by `GDK.get_social()`.

Current public methods:

- `start_social_graph(user) -> XboxResult`
- `stop_social_graph(user) -> void`
- `get_friends_async(user) -> Signal`
- `create_social_group(user, filter := null) -> XboxResult` (`data` is the `XboxSocialGroup`)
- `create_social_group_from_xuids(user, xuids) -> XboxResult` (`data` is the `XboxSocialGroup`)
- `destroy_social_group(group) -> void`
- `get_group_users(group) -> XboxResult` (`data` is an `Array[XboxSocialUser]`)

Current public signals:

- `social_graph_changed(user: XboxUser)`
- `social_group_updated(group: XboxSocialGroup)`
- `social_user_changed(xuid: String, social_user: XboxSocialUser)`

### Completion signals

Every one-shot async API now returns a completion `Signal` that emits exactly one `XboxResult`.

Important behaviors:

- callers `await service.method_async()` directly
- immediate failures still return a completion signal
- same-turn completion is deferred so the returned signal cannot be missed
- runtime shutdown queues a cancelled completion for still-pending one-shot signals before the shared task queue is terminated

### `XboxResult`

`XboxResult` normalizes native status into a stable Godot-facing payload.

Fields:

- `ok: bool`
- `hresult: int`
- `code: String`
- `message: String`
- `data: Variant`

`data` carries the operation payload. In the current implementation, successful user-add calls complete with an `XboxUser` in `data`, privilege and token/signature calls complete with `Dictionary` payloads, gamer-picture requests complete with a Godot `Image`, successful achievement queries/updates complete with cached `XboxAchievement` data in `data`, successful presence queries complete with an `Array` of `XboxPresenceRecord`, successful friends-group queries complete with an `XboxSocialGroup`, and successful store-license queries complete with an `XboxStoreLicenseStatus`.

## File map

### Root/runtime

- `xbox.cpp` / `xbox.h`  
  Root singleton. Owns `XboxRuntime`, `XboxServices`, and every concrete
  service (`XboxUsers`, `XboxSystem`, `XboxGameUI`, `XboxAccessibility`,
  `XboxAchievements`, `XboxPackage`, `XboxStats`, `XboxLeaderboards`,
  `XboxPrivacy`, `XboxPresence`, `XboxSocial`, `XboxStore`, `XboxProfile`,
  `XboxStringVerify`, `XboxTitleStorage`,   `XboxErrorReporting`, `XboxLauncher`, `XboxActivation`,
  `XboxMultiplayerActivity`, `XboxCapture`, and `XboxNetworking`).
- `xbox_runtime.cpp` / `xbox_runtime.h`  
  Shared Microsoft GDK runtime owner. Creates the queue, retains active pending signals, dispatches completions, and shuts everything down safely. During shutdown it cancels every retained pending signal and queues a cancelled completion so GDScript `await` sites are not stranded by queue teardown.

- `xbox_services.cpp` / `xbox_services.h`
  Shared XBOX services bootstrap. Derives the current-title SCID from `XGameGetXboxTitleId()`, initializes XSAPI, and caches per-user `XblContextHandle` objects.

### Generic async layer

- `xbox_result.cpp` / `xbox_result.h`
  Shared result type and HRESULT formatting helpers.

- `xbox_pending_signal.cpp` / `xbox_pending_signal.h`
  Internal one-shot completion emitter retained by the runtime until completion.

- `xbox_signal_xasync_context.cpp` / `xbox_signal_xasync_context.h`
  Internal base class that owns one `XAsyncBlock`, binds it to the shared queue, wires cancellation, and forwards the raw block to the operation-specific finalizer.

### Current concrete services

- `xbox_user.cpp` / `xbox_user.h`  
  `XboxUser`, `XboxUsers`, and the first concrete `XAsync` bridge context (`AddUserAsyncContext`).

- `xbox_system.cpp` / `xbox_system.h`  
  `XboxSystem` synchronous title/runtime metadata reads.

- `xbox_game_ui.cpp` / `xbox_game_ui.h`  
  `XboxGameUI` `XGameUI`-backed dialog, picker, and notification flows.

- `xbox_accessibility.cpp` / `xbox_accessibility.h`  
  `XboxAccessibility`, `XboxClosedCaptionProperties`, and synchronous `XAccessibility` reads.

- `xbox_achievement.cpp` / `xbox_achievement.h`  
  `XboxAchievement`, `XboxAchievements`, the Achievements Manager cache, and manager-driven completion signals.

- `xbox_package.cpp` / `xbox_package.h`  
  `XboxPackage`, `XboxPackageMount`, `XboxPackageResourcePack`, and `XPackage` enumeration / mount / DLC resource-pack flows.

- `xbox_stats.cpp` / `xbox_stats.h`  
  `XboxStats` XBOX Services user statistics with cache + tracking signals.

- `xbox_leaderboards.cpp` / `xbox_leaderboards.h`  
  `XboxLeaderboards`, `XboxLeaderboard`, `XboxLeaderboardColumn`, `XboxLeaderboardRow`, and read-only XBOX Services leaderboard queries.

- `xbox_privacy.cpp` / `xbox_privacy.h`  
  `XboxPrivacy` permission/avoid-list/mute-list reads.

- `xbox_presence.cpp` / `xbox_presence.h`  
  `XboxPresence`, `XboxPresenceRecord`, the presence cache, and XAsync-backed presence set/clear/query flows.

- `xbox_social.cpp` / `xbox_social.h`  
  `XboxSocial`, `XboxSocialFilter`, `XboxSocialGroup`, `XboxSocialUser`, and Social Manager-backed completion signals.

- `xbox_store.cpp` / `xbox_store.h`  
  `XboxStore`, `XboxStoreLicenseStatus`, the per-product license cache, and `XStore` license/refresh/purchase flows.

- `xbox_profile.cpp` / `xbox_profile.h`  
  `XboxProfile`, `XboxUserProfile`, and XBOX Services profile reads.

- `xbox_string_verify.cpp` / `xbox_string_verify.h`  
  `XboxStringVerify` XBOX Live string verification.

- `xbox_title_storage.cpp` / `xbox_title_storage.h`  
  `XboxTitleStorage`, `XboxTitleStorageBlobMetadata`, `XboxTitleStorageBlobMetadataResult`, and XBOX Services Title Storage blob/quota flows.

- `xbox_error_reporting.cpp` / `xbox_error_reporting.h`  
  `XboxErrorReporting` `XError` callback/options wrapper with `error_reported` mirrored through `GDK.runtime_error`.

- `xbox_launcher.cpp` / `xbox_launcher.h`  
  `XboxLauncher` `XLaunchUri`-only launcher with destination validation.

- `xbox_activation.cpp` / `xbox_activation.h`
  `XboxActivation` owns the single native activation subscription (`XGameActivationRegisterForEvent` on April 2026+, or the `XGameProtocol` / `XGameInvite` registrations on October 2025) and fans out activation dictionaries to internal service listeners.

- `xbox_multiplayer_activity.cpp` / `xbox_multiplayer_activity.h`  
  `XboxMultiplayerActivity`, `XboxMultiplayerActivityInfo`, MPA cache, recent-players staging, and invite signals forwarded from `XboxActivation`.

- `xbox_capture.cpp` / `xbox_capture.h`  
  `XboxCapture`, `XboxCaptureMetaData`, and the PC-supported `XAppCapture` capture-state and metadata flows.

- `xbox_networking.cpp` / `xbox_networking.h`  
  `XboxNetworking` and `XboxNetworkingSecurityInformation`: `XNetworking.h` preferred multiplayer port (sync + async), connectivity hints, the two change registrations, NSAL security information, and the TCP queued-receive-buffer configuration/statistics surfaces.

- `register_types.cpp`  
  Registers every public class listed above plus `XboxResult` and the
  internal `XboxPendingSignal`, then publishes the `GDK` singleton.

## Core model

### 1. One shared native queue

`XboxRuntime::initialize()` does two things:

1. calls `XGameRuntimeInitializeWithOptions()` with `File` source pointing at
   `<project_root>/MicrosoftGame.config` when that file is on disk (so
   unpackaged Godot dev runs get explicit package identity); falls back to
   `XGameRuntimeInitialize()` for packaged GDK launches that get identity from
   the registered package
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

Each concrete async request gets its own heap-allocated context object derived from `XboxSignalXAsyncContext` or completes through service-owned pending-signal state.

That base class owns:

- one `XAsyncBlock`
- the shared runtime pointer
- the `XboxPendingSignal`

Its constructor sets:

- `async.queue` to the shared runtime queue
- `async.context` to the context object itself
- `async.callback` to a single shared thunk

The important part is what the thunk does:

```cpp
void CALLBACK XboxSignalXAsyncContext::_completion_thunk(XAsyncBlock *p_async_block) {
    auto *context = static_cast<XboxSignalXAsyncContext *>(p_async_block->context);

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

### 3. `XboxRuntime` retains active requests

`XboxRuntime::retain_pending_signal()` keeps strong references to in-flight requests so GDScript can safely fire-and-forget signal-returning calls.

That matters because script is allowed to fire-and-forget:

```gdscript
GDK.users.add_default_user_async()
```

If the runtime did not retain the request, it could be destroyed before completion.

When a request completes, `XboxPendingSignal::complete()` runs its release hook and `XboxRuntime::release_pending_signal()` drops the retained reference.

### 4. XBOX services bootstrap is shared

`XboxServices` exists so XBOX services features can share title metadata and per-user XSAPI context management.

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
2. `XboxUsers::_start_add_user_async()` checks that the runtime is initialized.
3. If the runtime is unavailable, it returns an already-scheduled error signal using `XboxRuntime::make_error_signal()`.
4. Otherwise it:
   - instantiates `XboxPendingSignal`
   - asks `XboxRuntime` to retain it
   - allocates `AddUserAsyncContext`
   - binds cancellation through `XAsyncCancel`
   - starts `XUserAddAsync(...)`
5. Microsoft GDK performs the work on the queue's work port.
6. Completion lands on the queue's completion port.
7. The completion only becomes visible when `GDK.dispatch()` pumps the queue.
8. `XboxSignalXAsyncContext::_completion_thunk()` forwards the raw `XAsyncBlock` to `AddUserAsyncContext::finalize(...)`.
9. `AddUserAsyncContext::finalize(...)` calls:

```cpp
XUserAddResult(p_async_block, &user_handle)
```

10. `XboxUsers::complete_add_user(...)` wraps the native handle in an `XboxUser`, updates service state, emits service signals, and only then completes the returned signal with:

```cpp
XboxResult::ok_result(user)
```

11. `XboxPendingSignal::complete()` emits `completed(result)`.
12. The runtime release hook drops the retained strong reference.

## Users service state

`XboxUsers` currently owns:

- `m_users` — the current local user wrappers
- `m_primary_user` — the active primary user
- one runtime-wide `XUserRegisterForChangeEvent` subscription

On successful user add:

1. `XboxUser::adopt_handle()` takes ownership of the returned `XUserHandle`
2. `_populate_from_handle()` reads:
   - local id
   - XUID
   - gamertag
   - age group
   - guest state
   - sign-in state
   - store-user state
3. `XboxUsers::complete_add_user()` updates the cache and emits `user_changed(user, "added")` for a newly cached user or `user_changed(user, "signed_in_again")` for a refreshed cached user.
4. only after those updates does it complete the returned signal

That ordering is important. Future services should follow the same rule: update cache first, then complete the returned request.

The newer users-service one-shot requests (`resolve_privilege_with_ui_async()`, `resolve_issue_with_ui_async()`, `get_gamer_picture_async()`, and `get_token_and_signature_async()`) now reuse that same retained `XboxPendingSignal` + `XboxSignalXAsyncContext` pattern. The only difference is the payload translation that happens in the concrete finalizer: `Dictionary` for privilege/token results and `Image` for gamer pictures.

## Achievements service state

`XboxAchievements` currently owns:

- per-user Achievements Manager registration state
- per-user cached `XboxAchievement` wrappers
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

`XboxPendingSignal` owns the shared cancel state for in-flight requests.

- For `XboxSignalXAsyncContext`, cancellation calls `XAsyncCancel(&m_async_block)`.
- For manager-driven waits such as achievements and social friends queries, the cancel handler removes the pending request from service-owned state and completes it with `XboxResult::cancelled(...)`.

### Runtime shutdown

`XboxRuntime::shutdown()`:

1. marks the runtime as shutting down
2. cancels every retained pending request
3. calls `XTaskQueueTerminate(...)`
4. dispatches the completion port until the queue termination callback fires
5. closes the queue handle
6. clears retained pending requests
7. leaves `XGameRuntimeUninitialize()` to the process-lifetime teardown in `~XboxRuntime()`

Because the runtime sets `m_shutting_down` first, service finalizers can refuse to mutate state during teardown. `GDK.shutdown()` intentionally does not call `XGameRuntimeUninitialize()`; tests and games may cycle initialize/shutdown multiple times in one process, while the matching native uninitialize runs once when the extension is torn down.

### Finalizer contract

Every `XboxSignalXAsyncContext::finalize(XAsyncBlock *)` implementation must short-circuit before result extraction or service/cache mutation when `get_runtime()->is_shutting_down()` or `get_pending_signal()->was_cancel_requested()` is true. The finalizer completes its pending signal with `XboxResult::cancelled(...)` and returns, so shutdown and explicit cancellation do not continue the success path after the runtime has started tearing down.

If a future finalizer must perform native cleanup during shutdown, keep the cancelled-result gate first and document the cleanup-only exception both inline and in this section.

## Why the base bridge does not use generic `XAsyncGetStatus`

This is the most important implementation rule for future work.

`XboxSignalXAsyncContext` is intentionally **not** a generic result decoder. It should not assume that:

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
3. For `XAsync`-backed work, create a service-specific context derived from `XboxSignalXAsyncContext`.
4. In `finalize(XAsyncBlock *p_async_block)`, first apply the [finalizer contract](#finalizer-contract) shutdown/cancellation gate.
5. Call the API-specific result functions.
6. Translate native payloads into Godot wrappers or Variants.
7. Update service-owned cache/state first.
8. Emit service-level signals next.
9. Complete the returned signal last.
10. Use `XboxRuntime::make_error_signal()` for immediate startup/availability failures.

For manager/event-driven waits like achievements and social friends queries, store a retained `XboxPendingSignal` in service-owned pending state and use a cancel handler that unregisters it immediately if the request is cancelled during teardown.

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
