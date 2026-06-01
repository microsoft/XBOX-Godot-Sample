# Godot Microsoft GDK native runtime

This document explains the current native runtime architecture of the `godot_gdk` addon.

For the lower-level async details, see [`gdk/async-system.md`](async-system.md).

See also:

- [`gdk/plugin.md`](plugin.md)
- [`gdk/build-and-loading.md`](build-and-loading.md)

## Runtime structure

The current native implementation has one root singleton and 21 public service namespaces:

- root singleton: `GDK`
- service namespaces: `GDK.users`, `GDK.game_ui`, `GDK.accessibility`, `GDK.achievements`, `GDK.package`, `GDK.stats`, `GDK.leaderboards`, `GDK.privacy`, `GDK.presence`, `GDK.social`, `GDK.store`, `GDK.profile`, `GDK.string_verify`, `GDK.title_storage`, `GDK.error_reporting`, `GDK.launcher`, `GDK.multiplayer_activity`, `GDK.capture`, `GDK.system`, `GDK.display`, and `GDK.activation`
- wrapper types: `GDKResult`, `GDKUsers`, `GDKUser`, `GDKGameUI`, `GDKAccessibility`, `GDKClosedCaptionProperties`, `GDKAchievements`, `GDKAchievement`, `GDKPackage`, `GDKPackageMount`, `GDKPackageResourcePack`, `GDKStats`, `GDKLeaderboards`, `GDKLeaderboard`, `GDKLeaderboardColumn`, `GDKLeaderboardRow`, `GDKPrivacy`, `GDKPresence`, `GDKPresenceRecord`, `GDKSocial`, `GDKSocialFilter`, `GDKSocialGroup`, `GDKSocialUser`, `GDKStore`, `GDKStoreLicenseStatus`, `GDKProfile`, `GDKUserProfile`, `GDKStringVerify`, `GDKTitleStorage`, `GDKTitleStorageBlobMetadata`, `GDKTitleStorageBlobMetadataResult`, `GDKErrorReporting`, `GDKLauncher`, `GDKMultiplayerActivity`, `GDKMultiplayerActivityInfo`, `GDKCapture`, `GDKCaptureMetaData`, `GDKSystem`, `GDKDisplay`, `GDKDisplayTimeoutDeferral`, and `GDKActivation`
- internal direct-await helpers: `GDKPendingSignal`, `GDKSignalXAsyncContext`
- internal XBOX services scaffold: `GDKXboxServices`

## Root object: `GDK`

`GDK` is the public root singleton.

It owns:

- `GDKRuntime`
- `GDKXboxServices`
- service objects for `GDKUsers`, `GDKGameUI`, `GDKAccessibility`, `GDKAchievements`, `GDKPackage`, `GDKStats`, `GDKLeaderboards`, `GDKPrivacy`, `GDKPresence`, `GDKSocial`, `GDKStore`, `GDKProfile`, `GDKStringVerify`, `GDKTitleStorage`, `GDKErrorReporting`, `GDKLauncher`, `GDKMultiplayerActivity`, `GDKCapture`, `GDKSystem`, `GDKDisplay`, and `GDKActivation`

Its responsibilities are:

- runtime initialization
- runtime shutdown
- queue dispatch
- service access through the 21 public namespaces
- root-level runtime signals

Current public shape:

- `initialize(config := null) -> GDKResult`
- `shutdown()`
- `is_available() -> bool`
- `is_initialized() -> bool`
- `dispatch() -> int`
- `get_users() -> GDKUsers`
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
- `get_system() -> GDKSystem`
- `get_display() -> GDKDisplay`
- `get_activation() -> GDKActivation`

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

## XBOX services scaffold: `GDKXboxServices`

`GDKXboxServices` is the shared native helper for features that sit on top of XBOX services rather than the raw game runtime.

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

- `user_changed(user, change_kind)` for all user lifecycle and XBOX-facing state changes. `change_kind` is `added`, `removed`, `signed_in_again`, `gamertag`, `gamer_picture`, or `privileges`.

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

`GDKAchievements` is the first service implemented on top of the XBOX services scaffold.

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

`GDKPresence` is the XAsync-backed presence layer implemented on top of the shared XBOX services scaffold.

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
- `get_presence_async(xuids)` reads presence by XUID, but it still requires a signed-in primary user because the XSAPI call needs an XBOX services context.

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

## Profile service

`GDKProfile` is the XSAPI-backed profile layer for XBOX Services profile reads.

It currently exposes:

- `get_profile_async(user, xuid)`
- `get_profiles_async(user, xuids)`
- `get_profiles_for_social_group_async(user, social_group)`

`GDKUserProfile` is the script-visible wrapper around one XBOX Services profile
record. Calls duplicate from the shared cached XBOX services context via
`GDKXboxServices::duplicate_context_for_user` (which calls
`XblContextDuplicateHandle(...)`); methods return typed errors when the
scaffold isn't initialized.

## Privacy service

`GDKPrivacy` is the XSAPI-backed privacy layer for permission, avoid-list, and
mute-list reads.

It currently exposes:

- `check_permission_async(user, permission, target_xuid)`
- `check_permission_for_anonymous_user_async(user, permission, anonymous_user_type)`
- `batch_check_permission_async(user, permission, target_xuids)`
- `get_avoid_list_async(user)`
- `get_mute_list_async(user)`

Permission strings and anonymous user types are normalized case-insensitively
before being mapped to native enums. Mute/block list-change handlers are
intentionally not exposed because the XSAPI symbols are not exported by the
linked thunk libs.

## Stats service

`GDKStats` is the XSAPI-backed user statistics layer wired through the XBOX
services scaffold.

It currently exposes:

- `query_user_stats_async(user, stat_names)`
- `query_users_stats_async(user, xuids, stat_names)`
- `set_stat_integer(user, stat_name, value)`
- `set_stat_number(user, stat_name, value)`
- `flush_stats_async(user)`
- `track_stats(user, stat_names)`
- `stop_tracking_stats(user, stat_names)`
- `get_cached_stats(user)`

It emits:

- `stats_updated`
- `stat_changed`
- `stats_flushed`

The service maintains a per-user cache keyed by stat name. Title-managed
statistic updates are staged locally and submitted by `flush_stats_async()`.

## Leaderboards service

`GDKLeaderboards` is the XSAPI-backed read-only leaderboards layer backed by
title-managed statistics.

It currently exposes:

- `get_leaderboard_async(user, stat_name, max_items)`
- `get_leaderboard_around_user_async(user, stat_name, max_items)`
- `get_social_leaderboard_async(user, stat_name, max_items)`
- `get_next_page_async(leaderboard)`
- `get_cached_leaderboard(stat_name)`

It emits:

- `leaderboard_updated`

`GDKLeaderboard` carries `query_type`, `total_row_count`, `has_next`, columns,
and rows. `GDKLeaderboardRow.column_values` carries JSON-encoded column values
returned by XBOX Services.

## String verification service

`GDKStringVerify` is the XSAPI-backed wrapper around XBOX Live string
verification.

It currently exposes:

- `verify_string_async(user, text)`
- `verify_strings_async(user, strings)`

Result dictionaries normalize the native verification result code into
`success`, `offensive`, `too_long`, or `unknown_error`, plus an `acceptable`
boolean and the first offending substring when available.

## Title Storage service

`GDKTitleStorage` is the XSAPI-backed XBOX Services Title Storage layer
wrapping `title_storage_c.h`. It is unrelated to PlayFab Game Saves and the
Microsoft GDK `XGameSaveFiles` API.

It currently exposes:

- `get_quota_async(user, storage_type)`
- `list_blob_metadata_async(user, storage_type, blob_path, skip_items, max_items)`
- `get_next_blob_metadata_async(result)`
- `download_blob_async(user, storage_type, blob_path)`
- `upload_blob_async(user, storage_type, blob_path, data, display_name, e_tag, match_condition)`
- `delete_blob_async(user, storage_type, blob_path, e_tag, match_condition)`

`GDKTitleStorageBlobMetadata` and `GDKTitleStorageBlobMetadataResult` are the
script-visible wrappers around blob metadata records and paged metadata
results. Storage types accepted are `trusted_platform`, `global`, and
`universal`.

## Multiplayer activity service

`GDKMultiplayerActivity` is the XSAPI-backed Multiplayer Activity (MPA) layer
plus recent-players staging. Invite launch notifications are forwarded from
`GDKActivation`, which owns the single native `XGameActivationRegisterForEvent`
subscription for the addon.

It currently exposes:

- `set_activity_async(user, connection_string, join_restriction, max_players, current_players, group_id, allow_cross_platform_join)`
- `get_activities_async(user, xuids)`
- `get_cached_activity(xuid)`
- `delete_activity_async(user)`
- `send_invites_async(user, xuids, allow_cross_platform_join, connection_string)`
- `show_invite_ui_async(user)`
- `update_recent_players(user, xuids, encounter_type)`
- `flush_recent_players_async(user)`
- `accept_pending_invite(invite_uri)`

It emits:

- `activities_updated`
- `pending_invite_received`
- `invite_accepted`

`GDKMultiplayerActivityInfo` is the script-visible wrapper around one cached
activity snapshot. Pending invites are surfaced when `GDKActivation` receives an
invite URI on launch; `GDK.activation` and `GDK.multiplayer_activity` emit the
same parsed invite dictionary.

## Package service

`GDKPackage` is the `XPackage`-backed package metadata and DLC content service.

It currently exposes:

- `enumerate_packages(package_kind, scope)`
- `find_package_by_identifier(package_identifier, package_kind, scope)`
- `get_current_process_package_identifier()`
- `mount_package_async(package_identifier)`
- `load_resource_pack_async(package_identifier, pack_relative_path, replace_files, offset)`
- `get_loaded_resource_packs()`
- `get_install_progress(package_identifier)`

`GDKPackageMount` is the RAII wrapper around one mount handle returned by
`mount_package_async()`. `GDKPackageResourcePack` records resource packs
loaded through `load_resource_pack_async()`; service-owned mounts stay alive
until `GDK.shutdown()`. Async early failures are surfaced through the
completion signal payload (`GDKResult`); there is no global `last_error`
mirror to poll.

## Store service

`GDKStore` is the `XStore` commerce layer with a per-product license cache.

It currently exposes:

- `query_license_status_async(user, store_id)`
- `refresh_entitlements_async(user, store_id)`
- `show_purchase_ui_async(user, store_id)`
- `get_cached_license_status(store_id)`
- `check_cached_license_status(store_id)`

`GDKStoreLicenseStatus` is the script-visible wrapper around an `XStore`
license-acquire result. The service owns one `XStoreContextHandle` per
process; handles are recreated lazily and dropped on `shutdown()`.

## Capture service

`GDKCapture` is the `XAppCapture`-backed capture-state and capture-metadata
service. It exposes only the PC-supported subset of `XAppCapture.h`.

It currently exposes:

- `enable_capture()` / `disable_capture()`
- `record_diagnostic_clip_async(duration)`
- `take_diagnostic_screenshot_async(path_hint)`
- `create_metadata(reserved_bytes)`

`GDKCaptureMetaData` is the script-side write context around the process-wide
`XAppCaptureMetadata*` APIs. It is a validity gate — there is no native
metadata handle; methods are dispatched directly against the global capture
metadata state and refuse calls after `close()`.

## Launcher service

`GDKLauncher` is the `XLaunchUri`-only launcher service. The public surface is
intentionally limited to URI launches.

It currently exposes:

- `launch_uri(uri, user)`

The wrapper validates URIs before forwarding to `XLaunchUri`. Blank or
malformed URIs return `invalid_uri`; blocked destinations such as `file:`,
`javascript:`, `data:`, `about:`, and `ms-*` URIs other than `ms-settings:` /
`ms-windows-store:` return `unsupported_launcher_destination`.

## Error reporting service

`GDKErrorReporting` wraps the public Microsoft GDK `XError` callback and options
APIs (`XErrorSetCallback`, `XErrorSetOptions`). It does not submit reports to
external endpoints.

It currently exposes:

- `configure_options(debugger_present_options, debugger_not_present_options)`
- `set_callback_enabled(enabled)`
- `is_callback_enabled()`

It emits:

- `error_reported`

The same payload is mirrored through `GDK.runtime_error(result)` so callers
can listen at the runtime level. `ErrorOptions` flags can be combined with
bitwise OR.

## System service

`GDKSystem` is the synchronous title/runtime metadata service. It exposes
PC-supported XGameRuntime metadata reads plus a passthrough to the shared
XBOX services scaffold.

It currently exposes:

- `get_title_id()` / `get_title_id_hex()`
- `get_sandbox_id()`
- `get_service_configuration_id()`
- `is_xbox_services_initialized()`

The service has no native handle of its own; reads are direct calls to
`XGameGetXboxTitleId`, `XSystemGetXboxLiveSandboxId`, and the XBOX services
scaffold's cached SCID.

## Display service

`GDKDisplay` wraps PC-supported `XDisplay.h` display helpers.

It currently exposes:

- `try_enable_hdr_mode(preference)`
- `acquire_timeout_deferral()`

`try_enable_hdr_mode()` returns a `GDKResult` with HDR mode details. A
successful `acquire_timeout_deferral()` returns a `GDKDisplayTimeoutDeferral`
in `GDKResult.data`; releasing that wrapper calls the native timeout-deferral
release path.

## Activation service

`GDKActivation` owns the addon's single native
`XGameActivationRegisterForEvent` registration and fans protocol, file, and
invite activation dictionaries out to script and interested services.

It currently exposes:

- `accept_pending_invite(invite_uri)`

It emits:

- `protocol_activated`
- `file_activated`
- `pending_invite_received`
- `invite_accepted`

`GDKMultiplayerActivity` listens to this service instead of registering a
second native activation callback.

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

1. `GDKAchievements` ensures XBOX services are initialized from title metadata.
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
