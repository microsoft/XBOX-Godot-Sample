# Godot GDK native runtime

This document explains the current native runtime architecture of the `godot_gdk` addon.

For the lower-level async details, see [`godot-gdk-async-system.md`](godot-gdk-async-system.md).

See also:

- [`godot-gdk-plugin.md`](godot-gdk-plugin.md)
- [`godot-gdk-build-and-loading.md`](godot-gdk-build-and-loading.md)

## Runtime structure

The current native implementation is the runtime/users/accessibility/achievements/presence/social/store/launcher baseline:

- root singleton: `GDK`
- service namespace: `GDK.users`
- service namespace: `GDK.accessibility`
- service namespace: `GDK.achievements`
- service namespace: `GDK.presence`
- service namespace: `GDK.social`
- service namespace: `GDK.store`
- service namespace: `GDK.launcher`
- wrapper types: `GDKResult`, `GDKUsers`, `GDKUser`, `GDKAccessibility`, `GDKClosedCaptionProperties`, `GDKAchievements`, `GDKAchievement`, `GDKPresence`, `GDKPresenceRecord`, `GDKSocial`, `GDKSocialFilter`, `GDKSocialGroup`, `GDKSocialUser`, `GDKStore`, `GDKStoreLicenseStatus`, `GDKMultiplayerActivity`, `GDKMultiplayerActivityInfo`
- internal direct-await helpers: `GDKPendingSignal`, `GDKSignalXAsyncContext`
- internal Xbox services scaffold: `GDKXboxServices`

## Root object: `GDK`

`GDK` is the public root singleton.

It owns:

- `GDKRuntime`
- `GDKUsers`
- `GDKAccessibility`
- `GDKAchievements`
- `GDKPresence`
- `GDKSocial`
- `GDKStore`
- `GDKLauncher`
- `GDKXboxServices`

Its responsibilities are:

- runtime initialization
- runtime shutdown
- queue dispatch
- service access
- last-error exposure
- root-level runtime signals

Current public shape:

- `initialize(config := null) -> GDKResult`
- `shutdown()`
- `is_available() -> bool`
- `is_initialized() -> bool`
- `dispatch() -> int`
- `get_last_error() -> GDKResult`
- `get_users() -> GDKUsers`
- `get_accessibility() -> GDKAccessibility`
- `get_achievements() -> GDKAchievements`
- `get_presence() -> GDKPresence`
- `get_social() -> GDKSocial`
- `get_store() -> GDKStore`
- `get_launcher() -> GDKLauncher`

## Shared runtime: `GDKRuntime`

`GDKRuntime` is the native core behind the root singleton.

It is responsible for:

- calling `XGameRuntimeInitialize()`
- creating the shared `XTaskQueue`
- retaining in-flight completion signals
- pumping manual completion dispatch
- terminating the queue safely on shutdown

The queue is configured as:

- work port: `ThreadPool`
- completion port: `Manual`

That means native work may happen off-thread, but Godot-visible completion only becomes visible when `GDK.dispatch()` drains the completion queue. By default the addon now calls `GDK.dispatch()` from a native process-frame callback while `gdk/runtime/embed_dispatch` is enabled, and games can fall back to manual dispatch when they disable that setting.

## Xbox services scaffold: `GDKXboxServices`

`GDKXboxServices` is the shared native helper for features that sit on top of Xbox services rather than the raw game runtime.

It is responsible for:

- retrieving the current title id through `XGameGetXboxTitleId()`
- deriving the current-title SCID as a null GUID with the title id in the last 8 hex digits
- initializing XSAPI with `XblInitialize(...)`
- caching per-user `XblContextHandle` objects for future services
- cleaning up XSAPI state before the runtime queue shuts down

This layer exists so achievements, stats, leaderboards, presence, and social features can all share one services bootstrap path instead of each service redoing title metadata lookup and context creation.

## Async wrapper layer

The async layer is shared infrastructure, not a users-only feature.

It consists of:

- `GDKResult` — normalized result payload
- `GDKPendingSignal` — internal completion emitter retained until a request resolves
- `GDKSignalXAsyncContext` — internal base class for XAsync-backed signal-returning work

This layer exists so future services can expose Godot-native async APIs without duplicating queue, cancellation, and lifetime machinery.

Important rule: the base bridge handles shared mechanics only. Each concrete operation still owns its own result extraction logic.

## Users service

`GDKUsers` is the first service implemented on top of the shared runtime.

It currently owns:

- the local-user cache
- the primary-user reference
- the runtime-wide `XUserRegisterForChangeEvent` registration

It currently exposes:

- `add_default_user_async()` (returns a completion `Signal`)
- `add_user_with_ui_async()` (returns a completion `Signal`)
- `get_primary_user()`
- `get_users()`
- `check_privilege_async()` (returns a completion `Signal`)
- `resolve_privilege_with_ui_async()` (returns a completion `Signal`)
- `resolve_issue_with_ui_async()` (returns a completion `Signal`)
- `get_gamer_picture_async()` (returns a completion `Signal`)
- `get_token_and_signature_async()` (returns a completion `Signal`)

It emits:

- `user_changed(user, change_kind)` for all user lifecycle and Xbox-facing state changes. `change_kind` is `added`, `removed`, `signed_in_again`, `gamertag`, `gamer_picture`, or `privileges`.

`GDKUser` is the script-visible wrapper around a local user. It stores:

- local id
- XUID
- gamertag
- enum-backed age group plus a string name helper
- enum-backed sign-in state plus a string name helper
- guest flag
- store-user flag
- owned `XUserHandle`

## Game UI service

`GDKGameUI` is the runtime-level wrapper for PC-supported `XGameUI` surfaces.

It currently exposes:

- `show_message_dialog_async()`
- `set_notification_position_hint()`
- `show_player_profile_card_async()`
- `show_player_picker_async()`
- `resolve_privilege_with_ui_async()` (delegates to `GDKUsers`)

The async UI methods use `GDKSignalXAsyncContext` and return completion `Signal` values. When native APIs report `E_ABORT`, the wrapper returns `GDKResult.code == "cancelled"` so scripts can distinguish user cancellation from native failures.

## Accessibility service

`GDKAccessibility` is a synchronous service for concrete APIs from `XAccessibility.h`.

It currently exposes:

- `query_closed_caption_properties() -> GDKResult`
- `set_closed_caption_enabled(enabled: bool) -> GDKResult`
- `query_high_contrast_mode() -> GDKResult`

`GDKClosedCaptionProperties` is the script-visible wrapper around native
`XClosedCaptionProperties` data and exposes colors, font style/edge enums,
font scale, and enabled state.

These wrappers intentionally cover only the concrete PC-supported APIs used in
this change (`XClosedCaptionGetProperties`, `XClosedCaptionSetEnabled`,
`XHighContrastGetMode`). Speech-to-text and other families remain out of scope.

## Achievements service

`GDKAchievements` is the first service implemented on top of the Xbox services scaffold.

It currently owns:

- the per-user Achievements Manager registration state
- the per-user achievements cache
- pending query/update requests that complete from dispatch-driven manager events

It currently exposes:

- `query_player_achievements_async()`
- `update_achievement_async()`
- `get_cached_achievements()`

It emits:

- `achievement_unlocked`
- `achievements_updated`

`GDKAchievement` is the script-visible wrapper around one cached achievement. It stores:

- achievement id
- localized name
- service configuration id
- progress state
- computed progress percent
- unlock/secret flags
- locked/unlocked descriptions

## Presence service

`GDKPresence` is the XAsync-backed presence layer implemented on top of the shared Xbox services scaffold.

It currently exposes:

- `set_presence_async(user, state, rich_presence := {})` (returns a completion `Signal`)
- `clear_presence_async(user)` (returns a completion `Signal`)
- `get_presence_async(xuids)` (returns a completion `Signal`)
- `get_cached_presence(xuid)`

It emits:

- `presence_changed`
- `local_presence_set`

`GDKPresenceRecord` is the script-visible wrapper around one cached presence snapshot. It stores:

- XUID
- enum-backed user state plus a string name helper
- translated title/device presence records as Godot dictionaries

Important contract details:

- `state` is the configured rich-presence string ID for the current title SCID in Partner Center, not arbitrary text.
- `rich_presence` can override `scid` and pass `token_ids` / `tokens` for rich-presence formatting.
- `get_presence_async(xuids)` reads presence by XUID, but it still requires a signed-in primary user because the XSAPI call needs an Xbox services context.

## Social service

`GDKSocial` is the dispatch-driven Social Manager layer.

It currently exposes:

- `start_social_graph(user)`
- `stop_social_graph(user)`
- `get_friends_async(user)`
- `create_social_group(user, filter := null)`
- `create_social_group_from_xuids(user, xuids)`
- `destroy_social_group(group)`
- `get_group_users(group)`

It emits:

- `social_graph_changed`
- `social_group_updated`
- `social_user_changed`

Its main wrapper types are:

- `GDKSocialFilter` for filter-based group creation
- `GDKSocialGroup` for tracked Social Manager groups
- `GDKSocialUser` for copied social-graph user snapshots

## Request flow

The current `add_default_user_async()` flow is the best end-to-end example of the runtime behavior:

1. GDScript calls `GDK.users.add_default_user_async()`
2. `GDKUsers` checks runtime availability
3. it creates a `GDKPendingSignal`
4. `GDKRuntime` retains that pending request
5. a service-specific context (`AddUserAsyncContext`) is allocated
6. that context owns one `XAsyncBlock`
7. the request starts with `XUserAddAsync(...)`
8. completion lands on the shared completion port
9. `GDK.dispatch()` pumps the completion port
10. `GDKSignalXAsyncContext` forwards the raw block to the concrete finalizer
11. the users service calls `XUserAddResult(...)`
12. the native user handle is wrapped into `GDKUser`
13. service cache and service signals are updated
14. the returned completion signal emits a `GDKResult`
15. the runtime drops its retained reference to the pending request

That same pattern is already reused by the other direct-await users-service and presence requests.

The newer XUser-facing methods now reuse that same flow as well: privilege resolution, issue resolution, gamer-picture fetches, and token/signature requests all allocate a concrete `GDKSignalXAsyncContext`, translate native results into Godot `Dictionary` or `Image` payloads, and complete a retained `GDKPendingSignal` on the main-thread dispatch path.

The current `query_player_achievements_async()` and `update_achievement_async()` paths are the complementary non-`XAsyncBlock` example:

1. `GDKAchievements` ensures Xbox services are initialized from title metadata.
2. it registers the user with Achievements Manager through `XblAchievementsManagerAddLocalUser(...)`
3. it returns a retained completion signal
4. the op stays pending until `GDK.dispatch()` pumps `XblAchievementsManagerDoWork()`
5. manager events update the service cache first
6. service signals are emitted
7. only then does the pending completion signal resolve

`GDKSocial` follows that same dispatch-driven model: it registers local users with Social Manager, creates `GDKSocialGroup` wrappers around Social Manager group handles, reacts to `XblSocialManagerDoWork()` events on `GDK.dispatch()`, refreshes cached group/user/presence state, emits service signals, and only then completes pending friends-group ops.

That is why `GDK.dispatch()` now pumps the shared `XTaskQueue`, Achievements Manager, and Social Manager in one main-thread pass.

## Why the async layer matters plugin-wide

Even though the implementation is still early, the async subsystem is already a plugin-wide architectural rule because users, achievements, presence, and social all depend on it.

Future one-shot wrappers should all follow the same contract:

- return a completion `Signal`
- surface payload through `GDKResult.data`
- update service cache before the completion signal resolves
- emit service signals before the completion signal resolves
- keep native handle management internal to C++

## How to extend the native runtime

When adding a new service or wrapper, fit it into the existing runtime structure:

1. add native files under `addons\godot_gdk\src\`
2. wire them into `addons\godot_gdk\CMakeLists.txt`
3. register any new Godot classes in `register_types.cpp`
4. expose them from `GDK` as a service namespace instead of adding more flat singletons
5. reuse `GDKRuntime`, `GDKPendingSignal`, `GDKResult`, and `GDKSignalXAsyncContext` as appropriate
6. update sample usage and headless tests
7. update docs in `docs\`

That keeps the plugin coherent across native code, sample behavior, and docs.
