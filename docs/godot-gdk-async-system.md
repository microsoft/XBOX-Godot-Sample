# Godot GDK async system

This document explains how the new `godot_gdk` async system works today: the shared runtime, the generic async wrappers, the internal `XAsync` bridge, the shared Xbox services scaffold, and the current concrete services built on top of it (`GDK.users`, `GDK.achievements`, `GDK.presence`, and `GDK.social`).

For the plugin-wide view, including build, editor tooling, sample integration, and current scope boundaries, see [`godot-gdk-plugin.md`](godot-gdk-plugin.md).

## Why this exists

GDK async APIs are queue- and callback-driven. Godot script APIs are signal- and `await`-driven.

The system in `addons\godot_gdk\src\` exists to bridge those two models without exposing raw `XAsyncBlock`, `XTaskQueueHandle`, or `XUserHandle` values to GDScript.

The current baseline gives us:

- one root singleton: `GDK`
- one shared native async runtime: `GDKRuntime`
- two script-facing one-shot op types: `GDKAsyncOp` and `GDKDispatchOp`
- one normalized result type: `GDKResult`
- one internal `XAsync` bridge base: `GDKXAsyncContext`
- one internal Xbox services scaffold: `GDKXboxServices`
- four concrete services using the pattern: `GDK.users`, `GDK.achievements`, `GDK.presence`, and `GDK.social`

## Public surface

### `GDK`

`GDK` is the only engine singleton registered by the extension. It owns the shared runtime and exposes the first service namespace.

Current public methods:

- `initialize(config := null) -> GDKResult`
- `shutdown() -> void`
- `is_available() -> bool`
- `is_initialized() -> bool`
- `dispatch() -> int`
- `get_last_error() -> GDKResult`
- `get_users() -> GDKUsers`
- `get_achievements() -> GDKAchievements`
- `get_presence() -> GDKPresence`
- `get_social() -> GDKSocial`

Current public signals:

- `initialized()`
- `shutdown_completed()`
- `runtime_error(result: GDKResult)`
- `availability_changed(available: bool)`

### `GDK.users`

`GDK.users` is a `RefCounted` service object returned by `GDK.get_users()`.

Current public methods:

- `add_default_user_async(allow_guests := false) -> GDKAsyncOp`
- `add_user_with_ui_async() -> GDKAsyncOp`
- `get_primary_user() -> GDKUser`
- `get_users() -> Array`
- `check_privilege_async(user, privilege) -> GDKAsyncOp`
- `resolve_privilege_with_ui_async(user, privilege) -> GDKAsyncOp`
- `resolve_issue_with_ui_async(user, url := "") -> GDKAsyncOp`
- `get_gamer_picture_async(user, size := "medium") -> GDKAsyncOp`
- `get_token_and_signature_async(user, method, url, headers := {}, body := PackedByteArray(), force_refresh := false) -> GDKAsyncOp`

Current public signals:

- `user_added(user: GDKUser)`
- `user_removed(local_id: int)`
- `user_changed(user: GDKUser)`
- `primary_user_changed(user: GDKUser)`

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

### `GDK.achievements`

`GDK.achievements` is a `RefCounted` service object returned by `GDK.get_achievements()`.

Current public methods:

- `query_player_achievements_async(user) -> GDKDispatchOp`
- `update_achievement_async(user, achievement_id, percent_complete) -> GDKDispatchOp`
- `get_cached_achievements(user) -> Array`

Current public signals:

- `achievement_unlocked(user: GDKUser, achievement_id: String)`
- `achievements_updated(user: GDKUser)`

### `GDK.presence`

`GDK.presence` is a `RefCounted` service object returned by `GDK.get_presence()`.

Current public methods:

- `set_presence_async(user, state, rich_presence := {}) -> GDKAsyncOp`
- `clear_presence_async(user) -> GDKAsyncOp`
- `get_presence_async(xuids) -> GDKAsyncOp`
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
- `get_friends_async(user) -> GDKAsyncOp`
- `create_social_group(user, filter := null) -> GDKSocialGroup`
- `create_social_group_from_xuids(user, xuids) -> GDKSocialGroup`
- `destroy_social_group(group) -> void`
- `get_group_users(group) -> Array`

Current public signals:

- `social_graph_changed(user: GDKUser)`
- `social_group_updated(group: GDKSocialGroup)`
- `social_user_changed(xuid: String, social_user: GDKSocialUser)`

### `GDKAsyncOp`

`GDKAsyncOp` is the script-visible wrapper for one-shot work backed by a native `XAsyncBlock`.

It provides:

- `completed(result: GDKResult)`
- `is_done() -> bool`
- `cancel() -> bool`
- `get_result() -> GDKResult`

Important behaviors:

- XAsync-backed one-shot wrappers return a `GDKAsyncOp`
- immediate failures still return an op
- `cancel()` is best effort
- `completed` is emitted exactly once

### `GDKDispatchOp`

`GDKDispatchOp` is the script-visible wrapper for one-shot work that completes from service-managed dispatch state rather than from a native `XAsyncBlock`.

It inherits the same `completed(result)`, `is_done()`, `cancel()`, and `get_result()` surface as `GDKAsyncOp`, but its cancel path is different:

- it unregisters itself from the service's pending dispatch state immediately
- it completes with a cancelled `GDKResult` right away
- it is intended for manager/event-driven systems such as Achievements Manager and Social Manager-backed waits

### `GDKResult`

`GDKResult` normalizes native status into a stable Godot-facing payload.

Fields:

- `ok: bool`
- `hresult: int`
- `code: String`
- `message: String`
- `data: Variant`

`data` carries the operation payload. In the current implementation, successful user-add calls complete with a `GDKUser` in `data`, privilege and token/signature calls complete with `Dictionary` payloads, gamer-picture requests complete with a Godot `Image`, successful achievement queries/updates complete with cached `GDKAchievement` data in `data`, successful presence queries complete with an `Array` of `GDKPresenceRecord`, and successful friends-group queries complete with a `GDKSocialGroup`.

## File map

### Root/runtime

- `gdk.cpp` / `gdk.h`  
  Root singleton. Owns `GDKRuntime`, `GDKXboxServices`, `GDKUsers`, `GDKAchievements`, `GDKPresence`, and `GDKSocial`.

- `gdk_runtime.cpp` / `gdk_runtime.h`  
  Shared GDK runtime owner. Creates the queue, retains active ops, dispatches completions, and shuts everything down safely.

- `gdk_xbox_services.cpp` / `gdk_xbox_services.h`
  Shared Xbox services bootstrap. Derives the current-title SCID from `XGameGetXboxTitleId()`, initializes XSAPI, and caches per-user `XblContextHandle` objects.

### Generic async layer

- `gdk_result.cpp` / `gdk_result.h`  
  Shared result type and HRESULT formatting helpers.

- `gdk_async_op.cpp` / `gdk_async_op.h`  
  Shared XAsync-backed one-shot operation wrapper with completion, cancel, and release hooks.

- `gdk_dispatch_op.cpp` / `gdk_dispatch_op.h`
  Shared dispatch-backed one-shot operation wrapper for manager/event-driven service waits.

- `gdk_xasync_context.cpp` / `gdk_xasync_context.h`  
  Internal base class that owns one `XAsyncBlock`, binds it to the shared queue, wires cancellation, and forwards the raw block to the operation-specific finalizer.

### Current concrete services

- `gdk_user.cpp` / `gdk_user.h`  
  `GDKUser`, `GDKUsers`, and the first concrete `XAsync` bridge context (`AddUserAsyncContext`).

- `gdk_achievement.cpp` / `gdk_achievement.h`
  `GDKAchievement`, `GDKAchievements`, the Achievements Manager cache, and manager-driven `GDKDispatchOp` completion.

- `gdk_presence.cpp` / `gdk_presence.h`
  `GDKPresence`, `GDKPresenceRecord`, the presence cache, and XAsync-backed presence set/clear/query flows.

- `gdk_social.cpp` / `gdk_social.h`
  `GDKSocial`, `GDKSocialFilter`, `GDKSocialGroup`, `GDKSocialUser`, and Social Manager-backed dispatch completion.

- `register_types.cpp`  
  Registers `GDK`, `GDKUsers`, `GDKUser`, `GDKAchievements`, `GDKAchievement`, `GDKPresence`, `GDKPresenceRecord`, `GDKSocial`, `GDKSocialFilter`, `GDKSocialGroup`, `GDKSocialUser`, `GDKAsyncOp`, `GDKDispatchOp`, and `GDKResult`, then publishes the `GDK` singleton.

## Core model

### 1. One shared native queue

`GDKRuntime::initialize()` does two things:

1. calls `XGameRuntimeInitialize()`
2. creates one `XTaskQueue` with:
   - `ThreadPool` work dispatch
   - `Manual` completion dispatch

That split is the key design choice.

Native work can run off-thread, but Godot-visible completion is only surfaced when the game calls:

```gdscript
GDK.dispatch()
```

Internally this drains the queue with:

```cpp
XTaskQueueDispatch(m_task_queue, XTaskQueuePort::Completion, 0)
```

This keeps Godot-facing state changes tied to the main-thread pump instead of worker-thread callbacks.

For Xbox services features, `GDK.dispatch()` also pumps manager-driven state like `XblAchievementsManagerDoWork()`. So the same per-frame dispatch contract covers both `XAsync` completions and non-`XAsync` service feeds.

### 2. One `XAsyncBlock` per XAsync-backed request

Each concrete async request gets its own heap-allocated context object derived from `GDKXAsyncContext`.

That base class owns:

- one `XAsyncBlock`
- the shared runtime pointer
- the `GDKAsyncOp`

Its constructor sets:

- `async.queue` to the shared runtime queue
- `async.context` to the context object itself
- `async.callback` to a single shared thunk

The important part is what the thunk does:

```cpp
void CALLBACK GDKXAsyncContext::_completion_thunk(XAsyncBlock *p_async_block) {
    auto *context = static_cast<GDKXAsyncContext *>(p_async_block->context);

    context->clear_cancel_handler();
    context->finalize(p_async_block);
    delete context;
}
```

The thunk does **not** try to decode results generically.

That is intentional. Each GDK async API has its own result contract:

- some use `*Result()`
- some use `*ResultSize()` + `*Result()`
- some use manager state instead of a classic async result payload

So the base layer only handles lifetime, queue binding, and cancellation plumbing. The service-specific context owns result extraction.

### 3. `GDKRuntime` retains active ops

`GDKRuntime::retain_op()` keeps strong references to in-flight one-shot ops. Today that means `GDKAsyncOp` plus `GDKDispatchOp`, which inherits the same retention/release mechanics.

That matters because script is allowed to fire-and-forget:

```gdscript
GDK.users.add_default_user_async()
```

If the runtime did not retain the op, it could be destroyed before completion.

When an op completes, `GDKAsyncOp::complete()` runs its release hook and `GDKRuntime::release_op()` drops the retained reference.

### 4. Xbox services bootstrap is shared

`GDKXboxServices` exists so Xbox services features can share title metadata and per-user XSAPI context management.

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
3. If the runtime is unavailable, it returns an already-completed error op using `GDKRuntime::make_error_async_op()`.
4. Otherwise it:
   - instantiates `GDKAsyncOp`
   - asks `GDKRuntime` to retain it
   - allocates `AddUserAsyncContext`
   - binds cancellation through `XAsyncCancel`
   - starts `XUserAddAsync(...)`
5. GDK performs the work on the queue's work port.
6. Completion lands on the queue's completion port.
7. The completion only becomes visible when `GDK.dispatch()` pumps the queue.
8. `GDKXAsyncContext::_completion_thunk()` forwards the raw `XAsyncBlock` to `AddUserAsyncContext::finalize(...)`.
9. `AddUserAsyncContext::finalize(...)` calls:

```cpp
XUserAddResult(p_async_block, &user_handle)
```

10. `GDKUsers::complete_add_user(...)` wraps the native handle in a `GDKUser`, updates service state, emits service signals, and only then completes the op with:

```cpp
GDKResult::ok_result(user)
```

11. `GDKAsyncOp::complete()` emits `completed(result)`.
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
3. `GDKUsers::complete_add_user()` updates the cache and emits:
   - `user_added` or `user_changed`
   - `primary_user_changed` if needed
4. only after those updates does it complete the `GDKAsyncOp`

That ordering is important. Future services should follow the same rule: update cache first, then complete the op.

The newer users-service one-shot requests (`resolve_privilege_with_ui_async()`, `resolve_issue_with_ui_async()`, `get_gamer_picture_async()`, and `get_token_and_signature_async()`) reuse that same retained `GDKAsyncOp` + `GDKXAsyncContext` pattern. The only difference is the payload translation that happens in the concrete finalizer: `Dictionary` for privilege/token results and `Image` for gamer pictures.

## Achievements service state

`GDKAchievements` currently owns:

- per-user Achievements Manager registration state
- per-user cached `GDKAchievement` wrappers
- pending query ops waiting for `LocalUserInitialStateSynced`
- pending update ops waiting for `AchievementProgressUpdated` or `AchievementUnlocked`

Unlike `GDK.users`, this service does not create an `XAsyncBlock` per request. Instead, it adapts Achievements Manager's cache-and-event model into a dispatch-backed `GDKDispatchOp` contract:

1. script requests a query or update
2. the service ensures the user is registered with Achievements Manager
3. the service returns a retained `GDKDispatchOp`
4. `GDK.dispatch()` pumps `XblAchievementsManagerDoWork()`
5. manager events update the service cache
6. service signals are emitted
7. the pending `GDKDispatchOp` completes

That is the concrete example of the "manager state instead of a classic async result payload" rule described earlier.

## Cancellation and shutdown

### Operation cancellation

`GDKAsyncOp::cancel()` only marks the op once and then calls the context-specific cancel handler.

For `GDKXAsyncContext`, that handler is:

```cpp
XAsyncCancel(&m_async_block)
```

The operation-specific finalizer is still responsible for interpreting cancellation correctly. In the users path:

- an explicit cancel request becomes `GDKResult::cancelled(...)`
- `E_ABORT` from `XUserAddResult(...)` also becomes `GDKResult::cancelled(...)`

`GDKDispatchOp::cancel()` is intentionally different. Because manager-backed waits do not own a native `XAsyncBlock`, cancel:

1. unregisters the op from the owning service's pending dispatch state
2. completes the op immediately with `GDKResult::cancelled(...)`

That gives Achievements Manager-style waits deterministic local cancellation instead of leaving them parked until some future manager event happens to arrive.

### Runtime shutdown

`GDKRuntime::shutdown()`:

1. marks the runtime as shutting down
2. cancels every retained active op
3. calls `XTaskQueueTerminate(...)`
4. dispatches the completion port until the queue termination callback fires
5. closes the queue handle
6. clears retained ops
7. calls `XGameRuntimeUninitialize()`

Because the runtime sets `m_shutting_down` first, service finalizers can refuse to mutate state during teardown.

## Why the base bridge does not use generic `XAsyncGetStatus`

This is the most important implementation rule for future work.

`GDKXAsyncContext` is intentionally **not** a generic result decoder. It should not assume that:

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

1. Decide whether the wrapper is `XAsync`-backed or dispatch-backed.
2. For `XAsync`-backed work, add the public service method that returns `GDKAsyncOp`.
3. Create a service-specific context derived from `GDKXAsyncContext`.
4. In `finalize(XAsyncBlock *p_async_block)`, call the API-specific result functions.
5. Translate native payloads into Godot wrappers or Variants.
6. Update service-owned cache/state first.
7. Emit service-level signals next.
8. Complete the `GDKAsyncOp` last.
9. Use `GDKRuntime::make_error_async_op()` for immediate startup/availability failures.

For manager/event-driven waits like achievements and future social-manager-backed flows, return `GDKDispatchOp` instead, store it in service-owned pending state, and use a cancel handler that unregisters it immediately when script calls `cancel()`.

## Current scope

This is still the first implementation slice.

Today the system covers:

- runtime bootstrap and shutdown
- shared queue ownership
- async op/result wrappers
- one reusable `XAsync` context base
- the users baseline

It does **not** yet implement every service namespace from the spec (`save`, `stats`, and `leaderboards` are still missing), but presence and social now reuse the same shared async/dispatch pattern beside users and achievements.
