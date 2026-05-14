# Godot GDK API reference

This document describes the current public GDScript API surface of the
`godot_gdk` addon. It is built from the actual bound methods and signals in
the native implementation.

For architecture details, see
[Native Runtime](godot-gdk-native-runtime.md) and
[Async System](godot-gdk-async-system.md).

## Root singleton: `GDK`

`GDK` is the only engine singleton registered by the addon. All services are
accessed as namespaces under this root.

### Methods

| Method | Returns | Description |
|--------|---------|-------------|
| `initialize(config := null)` | `GDKResult` | Start the GDK runtime. `config` is optional and reserved for future use. |
| `shutdown()` | `void` | Clean up the runtime |
| `is_available()` | `bool` | Whether the GDK runtime is available on this platform |
| `is_initialized()` | `bool` | Whether the runtime has been initialized |
| `dispatch()` | `int` | Pump async completions and manager state manually when `gdk/runtime/embed_dispatch` is disabled or when deterministic control is needed |
| `get_last_error()` | `GDKResult` | Last error result |
| `get_users()` | `GDKUsers` | Access the users service |
| `get_game_ui()` | `GDKGameUI` | Access the system UI service |
| `get_accessibility()` | `GDKAccessibility` | Access the accessibility service |
| `get_achievements()` | `GDKAchievements` | Access the achievements service |
| `get_package()` | `GDKPackage` | Access package metadata and DLC content-loading helpers |
| `get_stats()` | `GDKStats` | Access the Xbox Services statistics service |
| `get_leaderboards()` | `GDKLeaderboards` | Access the Xbox Services leaderboard service |
| `get_privacy()` | `GDKPrivacy` | Access the Xbox Services privacy service |
| `get_presence()` | `GDKPresence` | Access the presence service |
| `get_social()` | `GDKSocial` | Access the social graph service |
| `get_store()` | `GDKStore` | Access the XStore commerce service |
| `get_profile()` | `GDKProfile` | Access the Xbox Services profile service |
| `get_string_verify()` | `GDKStringVerify` | Access the Xbox Services string verification service |
| `get_title_storage()` | `GDKTitleStorage` | Access the Xbox Services Title Storage service |
| `get_error_reporting()` | `GDKErrorReporting` | Access the PC GDK `XError` callback/options service |
| `get_launcher()` | `GDKLauncher` | Access the launcher service for URI/store/settings flows |
| `get_multiplayer_activity()` | `GDKMultiplayerActivity` | Access the multiplayer activity service |
| `get_capture()` | `GDKCapture` | Access the capture metadata and capture-state service |
| `get_system()` | `GDKSystem` | Access title/runtime metadata and environment facts |
| `get_display()` | `GDKDisplay` | Access HDR mode probing and display timeout deferrals |
| `get_activation()` | `GDKActivation` | Access game activation events (protocol/file/invite launches) |

### Signals

| Signal | Description |
|--------|-------------|
| `initialized()` | Runtime initialized successfully |
| `shutdown_completed()` | Runtime shutdown complete |
| `runtime_error(result: GDKResult)` | A runtime error occurred |

### Usage

```gdscript
func _ready():
    GDK.initialize()

func _exit_tree():
    GDK.shutdown()
```

By default the addon pumps completions automatically each process frame through
`gdk/runtime/embed_dispatch`.

If you disable that setting, call `dispatch()` manually:

```gdscript
func _process(_delta):
    GDK.dispatch()
```

## System service: `GDK.system`

`GDK.system` is a `RefCounted` service object returned by `GDK.get_system()`.

### Methods

| Method | Returns | Description |
|--------|---------|-------------|
| `get_title_id()` | `GDKResult` | Read the current Xbox title ID (`data` is `int`) |
| `get_title_id_hex()` | `GDKResult` | Read the current Xbox title ID as uppercase `0x`-prefixed hex (`data` is `String`) |
| `get_sandbox_id()` | `GDKResult` | Read the current sandbox ID (`data` is `String`) |
| `get_service_configuration_id()` | `GDKResult` | Read the current SCID from the shared Xbox services scaffold (`data` is `String`) |
| `is_xbox_services_initialized()` | `bool` | Check whether the shared Xbox services scaffold is initialized |

### Usage

```gdscript
var title_result: GDKResult = GDK.system.get_title_id()
if title_result.ok:
    print("Title ID:", title_result.data)

var sandbox_result: GDKResult = GDK.system.get_sandbox_id()
if sandbox_result.ok:
    print("Sandbox:", sandbox_result.data)
```

## Users service: `GDK.users`

`GDK.users` is a `RefCounted` service object returned by `GDK.get_users()`.

### Methods

| Method | Returns | Description |
|--------|---------|-------------|
| `add_default_user_async()` | `Signal` | Silent Xbox sign-in for a non-guest user |
| `add_user_with_ui_async()` | `Signal` | Xbox sign-in with UI prompt for another local user or guest-capable picker flow; it does not replace the session primary user |
| `get_primary_user()` | `GDKUser` | Current primary user (or `null`) |
| `get_users()` | `Array` | All local users |
| `check_privilege_async(user, privilege)` | `Signal` | Check user privilege |
| `resolve_privilege_with_ui_async(user, privilege)` | `Signal` | Resolve privilege with UI |
| `resolve_issue_with_ui_async(user, url)` | `Signal` | Resolve account issue with UI |
| `get_gamer_picture_async(user, size)` | `Signal` | Fetch user's profile picture |
| `get_token_and_signature_async(user, method, url, headers, body, force_refresh)` | `Signal` | Get Xbox Live auth token |

### Signals

| Signal | Description |
|--------|-------------|
| `user_changed(user: GDKUser, change_kind: String)` | The single user lifecycle/state event. `change_kind` is `added`, `removed`, `signed_in_again`, `gamertag`, `gamer_picture`, or `privileges`; for `removed`, `user` identifies the removed user and is no longer present in `get_users()` |

### Usage

```gdscript
func _ready():
    GDK.users.user_changed.connect(_on_user_changed)
    var result = await GDK.users.add_default_user_async()
    if result.ok:
        print("Signed in: ", result.data.gamertag)

func _on_user_changed(user: GDKUser, change_kind: String):
    print("User %s: %s" % [change_kind, user.gamertag])
```

## Game UI service: `GDK.game_ui`

`GDK.game_ui` is a `RefCounted` service object returned by `GDK.get_game_ui()`.

### Methods

| Method | Returns | Description |
|--------|---------|-------------|
| `show_message_dialog_async(title, message, first_button, second_button, third_button, default_button, cancel_button)` | `Signal` | Show a system message dialog; success data includes `selected_button` and `selected_button_index` |
| `set_notification_position_hint(position)` | `GDKResult` | Set the notification position hint (`bottom_center`, `bottom_left`, `bottom_right`, `top_center`, `top_left`, `top_right`) |
| `show_player_profile_card_async(requesting_user, target_xuid)` | `Signal` | Show the profile card UI for a target XUID |
| `show_player_picker_async(requesting_user, prompt, selectable_xuids, preselected_xuids, min_selection_count, max_selection_count)` | `Signal` | Show player picker UI; success data includes `selected_xuids` and `selection_count` |
| `resolve_privilege_with_ui_async(user, privilege)` | `Signal` | Forward to users-service privilege remediation UI flow |

## `GDKUser`

Script-visible wrapper around a local Xbox user.

### Properties

| Property | Type | Description |
|----------|------|-------------|
| `local_id` | `int` | Local user ID |
| `xuid` | `String` | Xbox User ID |
| `gamertag` | `String` | Display name |
| `age_group` | `GDKUser.AgeGroup` | Age group enum |
| `sign_in_state` | `GDKUser.SignInState` | Sign-in state enum |
| `guest` | `bool` | Whether the user is a guest |
| `signed_in` | `bool` | Whether the user is signed in |
| `store_user` | `bool` | Whether the user is a store user |

### Methods

| Method | Returns | Description |
|--------|---------|-------------|
| `get_age_group_name()` | `String` | Age group as human-readable string |
| `get_sign_in_state_name()` | `String` | Sign-in state as human-readable string |

## Accessibility service: `GDK.accessibility`

`GDK.accessibility` is a `RefCounted` service object returned by `GDK.get_accessibility()`.

### Methods

| Method | Returns | Description |
|--------|---------|-------------|
| `query_closed_caption_properties()` | `GDKResult` | Query closed-caption properties from `XClosedCaptionGetProperties`; on success `result.data` is `GDKClosedCaptionProperties` |
| `set_closed_caption_enabled(enabled)` | `GDKResult` | Set caption-enabled state using `XClosedCaptionSetEnabled` |
| `query_high_contrast_mode()` | `GDKResult` | Query current high-contrast mode from `XHighContrastGetMode`; on success `result.data` includes `mode` and `mode_name` |
| `get_high_contrast_mode_name(mode)` | `String` | Convert `GDKAccessibility.HighContrastMode` to snake_case name |

### Notes

- These wrappers are scoped to concrete PC-supported APIs documented under `XAccessibility.h`.
- Speech-to-text overlay APIs are intentionally excluded from this first deterministic accessibility surface.

## `GDKClosedCaptionProperties`

Script-visible wrapper around the native `XClosedCaptionProperties` payload.

### Properties

| Property | Type | Description |
|----------|------|-------------|
| `background_color` | `Color` | Caption background color |
| `font_color` | `Color` | Caption font color |
| `window_color` | `Color` | Caption window color |
| `font_edge_attribute` | `GDKClosedCaptionProperties.FontEdgeAttribute` | Caption edge style |
| `font_style` | `GDKClosedCaptionProperties.FontStyle` | Caption font style |
| `font_scale` | `float` | Caption font scale |
| `enabled` | `bool` | Whether captions are enabled |

## Achievements service: `GDK.achievements`

`GDK.achievements` is a `RefCounted` service object returned by
`GDK.get_achievements()`. It uses the Achievements Manager pattern
with direct-await completion signals.

### Methods

| Method | Returns | Description |
|--------|---------|-------------|
| `query_player_achievements_async(user)` | `Signal` | Query achievements for a user |
| `update_achievement_async(user, achievement_id, percent_complete)` | `Signal` | Update achievement progress |
| `get_cached_achievements(user)` | `Array` | Get cached achievement list |

### Signals

| Signal | Description |
|--------|-------------|
| `achievement_unlocked(user: GDKUser, achievement_id: String)` | An achievement was unlocked |
| `achievements_updated(user: GDKUser)` | Achievement cache was updated |

### Usage

```gdscript
# Query achievements
var result = await GDK.achievements.query_player_achievements_async(user)
if result.ok:
    for achievement in GDK.achievements.get_cached_achievements(user):
        print(achievement.name, ": ", achievement.progress_percent, "%")

# Update progress (25% increments)
await GDK.achievements.update_achievement_async(user, "1", 25)
```

## `GDKAchievement`

Script-visible wrapper around a cached achievement.

### Properties

| Property | Type | Description |
|----------|------|-------------|
| `id` | `String` | Achievement identifier |
| `name` | `String` | Localized display name |
| `service_configuration_id` | `String` | Service config ID |
| `progress_state` | `String` | Progress state |
| `progress_percent` | `int` | Computed progress (0–100) |
| `unlocked` | `bool` | Whether fully unlocked |
| `secret` | `bool` | Whether hidden until unlocked |
| `locked_description` | `String` | Description shown when locked |
| `unlocked_description` | `String` | Description shown when unlocked |

### Methods

| Method | Returns | Description |
|--------|---------|-------------|
| `is_unlocked()` | `bool` | Whether the achievement is fully unlocked |
| `is_secret()` | `bool` | Whether the achievement is hidden until unlocked |

## Package service: `GDK.package`

`GDK.package` is a `RefCounted` service object returned by `GDK.get_package()`.

### Methods

| Method | Returns | Description |
|--------|---------|-------------|
| `enumerate_packages(package_kind := GDKPackage.PACKAGE_KIND_CONTENT, scope := GDKPackage.ENUMERATION_SCOPE_THIS_AND_RELATED)` | `GDKResult` | Enumerate installed packages; `data` is an `Array` of package dictionaries |
| `find_package_by_identifier(package_identifier, package_kind := GDKPackage.PACKAGE_KIND_CONTENT, scope := GDKPackage.ENUMERATION_SCOPE_THIS_AND_RELATED)` | `GDKResult` | Find one installed package by package identifier |
| `get_current_process_package_identifier()` | `GDKResult` | Resolve package identity for the current process |
| `mount_package_async(package_identifier)` | `Signal` | Mount package content and return `GDKPackageMount` in `GDKResult.data` |
| `load_resource_pack_async(package_identifier, pack_relative_path, replace_files := false, offset := 0)` | `Signal` | Mount package content and load package-relative `.pck`/`.zip` into `res://` |
| `get_loaded_resource_packs()` | `Array` | Return service-owned `GDKPackageResourcePack` metadata for loaded packs |
| `get_install_progress(package_identifier)` | `GDKResult` | Snapshot install progress for a package identifier |

### Notes

- `mount_package_async()` is the loose-file path; callers are responsible for closing the returned mount.
- `load_resource_pack_async()` is the Godot-native DLC path; mounts for loaded resource packs stay service-owned until `GDK.shutdown()`.

## Stats service: `GDK.stats`

`GDK.stats` is a `RefCounted` service object returned by `GDK.get_stats()`.
It wraps Xbox Services user statistics reads, title-managed statistic updates,
real-time statistic tracking, and a per-user in-memory cache.

### Methods

| Method | Returns | Description |
|--------|---------|-------------|
| `query_user_stats_async(user, stat_names := PackedStringArray())` | `Signal` | Query named stats for one local user; success data is a `Dictionary` keyed by stat name |
| `query_users_stats_async(user, xuids, stat_names := PackedStringArray())` | `Signal` | Query named stats for multiple target XUIDs using the local user as caller context; success data is keyed by XUID |
| `set_stat_integer(user, stat_name, value)` | `GDKResult` | Stage an integer title-managed statistic for the user |
| `set_stat_number(user, stat_name, value)` | `GDKResult` | Stage a numeric title-managed statistic for the user |
| `flush_stats_async(user)` | `Signal` | Submit staged title-managed statistics for the user |
| `track_stats(user, stat_names)` | `GDKResult` | Start tracking real-time changes for named stats |
| `stop_tracking_stats(user, stat_names := PackedStringArray())` | `GDKResult` | Stop tracking named stats, or all tracked stats for the user when empty |
| `get_cached_stats(user)` | `Dictionary` | Return cached stats for the user keyed by stat name |

### Signals

| Signal | Description |
|--------|-------------|
| `stats_updated(user: GDKUser, stats: Dictionary)` | Cached stats changed for a user |
| `stat_changed(user: GDKUser, stat_name: String, value: Variant)` | A tracked statistic changed |
| `stats_flushed(user: GDKUser, result: GDKResult)` | Staged statistics finished flushing |

### Data shape

Stat dictionaries are keyed by statistic name. Each value contains `name`,
`type`, `value`, and `service_configuration_id`. Native statistic values are
returned as strings because Xbox Services user-stat query results expose values
as string payloads.

### Usage

```gdscript
var query_result = await GDK.stats.query_user_stats_async(user, ["score"])
if query_result.ok:
    print(query_result.data["score"]["value"])

GDK.stats.set_stat_number(user, "score", 123.0)
var flush_result = await GDK.stats.flush_stats_async(user)
if flush_result.ok:
    print("Stats flushed")
```

## Leaderboards service: `GDK.leaderboards`

`GDK.leaderboards` is a `RefCounted` service object returned by
`GDK.get_leaderboards()`. It wraps read-only Xbox Services leaderboard queries
backed by title-managed statistics.

### Methods

| Method | Returns | Description |
|--------|---------|-------------|
| `get_leaderboard_async(user, stat_name, max_items := 25)` | `Signal` | Query the global leaderboard for a stat |
| `get_leaderboard_around_user_async(user, stat_name, max_items := 25)` | `Signal` | Query the global leaderboard around the local user's XUID |
| `get_social_leaderboard_async(user, stat_name, max_items := 25)` | `Signal` | Query the followed-people social leaderboard for a stat |
| `get_next_page_async(leaderboard)` | `Signal` | Fetch the next page for a returned `GDKLeaderboard` |
| `get_cached_leaderboard(stat_name)` | `GDKLeaderboard` | Return the cached leaderboard for a stat, or `null` |

### Signals

| Signal | Description |
|--------|-------------|
| `leaderboard_updated(stat_name: String, leaderboard: GDKLeaderboard)` | A leaderboard query or next-page request updated the cache |

### `GDKLeaderboard`

| Property | Type | Description |
|----------|------|-------------|
| `stat_name` | `String` | Statistic backing the leaderboard |
| `query_type` | `String` | `global`, `around_user`, or `social` |
| `total_row_count` | `int` | Total rows reported by Xbox Services |
| `has_next` | `bool` | Whether another page can be fetched |
| `columns` | `Array[GDKLeaderboardColumn]` | Column metadata |
| `rows` | `Array[GDKLeaderboardRow]` | Row data |

`GDKLeaderboardRow.column_values` contains the JSON-encoded column values
returned by Xbox Services.

### Usage

```gdscript
var result = await GDK.leaderboards.get_leaderboard_async(user, "score", 25)
if result.ok:
    var leaderboard: GDKLeaderboard = result.data
    for row in leaderboard.rows:
        print("%s: %s" % [row.unique_modern_gamertag, row.column_values])

    if leaderboard.has_next:
        await GDK.leaderboards.get_next_page_async(leaderboard)
```

## Privacy service: `GDK.privacy`

`GDK.privacy` is a `RefCounted` service object returned by
`GDK.get_privacy()`. It wraps Xbox Services privacy permission checks plus
avoid-list and mute-list reads.

### Methods

| Method | Returns | Description |
|--------|---------|-------------|
| `check_permission_async(user, permission, target_xuid)` | `Signal` | Check whether `user` has a permission with the target XUID |
| `check_permission_for_anonymous_user_async(user, permission, anonymous_user_type)` | `Signal` | Check whether `user` has a permission with an anonymous user type |
| `batch_check_permission_async(user, permission, target_xuids)` | `Signal` | Check one permission against multiple target XUIDs |
| `get_avoid_list_async(user)` | `Signal` | Query the avoid list for `user`; success data is a `PackedStringArray` of XUID strings |
| `get_mute_list_async(user)` | `Signal` | Query the mute list for `user`; success data is a `PackedStringArray` of XUID strings |

### Data shape

Permission check results are dictionaries with `allowed`, `target_xuid`,
`target_user_type`, `permission`, and `reasons`. Each deny reason contains
`reason`, `restricted_privilege`, and `restricted_privacy_setting`.

Supported permission strings are normalized case-insensitively and include
`communicate_using_text`, `communicate_using_video`,
`communicate_using_voice`, `view_target_profile`,
`view_target_game_history`, `view_target_video_history`,
`view_target_music_history`, `view_target_exercise_info`,
`view_target_presence`, `view_target_video_status`,
`view_target_music_status`, `play_multiplayer`,
`view_target_user_created_content`, `broadcast_with_twitch`,
`write_comment`, `share_item`, and
`share_target_content_to_external_networks`.

Anonymous user type values are `cross_network_user` and
`cross_network_friend`.

### Usage

```gdscript
var permission_result = await GDK.privacy.check_permission_async(
        user,
        "communicate_using_voice",
        "2814639012345678")
if permission_result.ok and permission_result.data["allowed"]:
    print("Voice chat is allowed")

var mute_list_result = await GDK.privacy.get_mute_list_async(user)
if mute_list_result.ok:
    print("Muted users: ", mute_list_result.data)
```

## Presence service: `GDK.presence`

`GDK.presence` is a `RefCounted` service object returned by `GDK.get_presence()`.

### Methods

| Method | Returns | Description |
|--------|---------|-------------|
| `set_presence_async(user, state, rich_presence)` | `Signal` | Set rich presence for a local user |
| `clear_presence_async(user)` | `Signal` | Clear presence for a local user |
| `get_presence_async(xuids)` | `Signal` | Query presence records for a list of XUIDs |
| `get_presence_for_social_group_async(user, social_group)` | `Signal` | Query presence records for a named social group |
| `track_presence(user, xuids, title_ids := PackedInt64Array())` | `GDKResult` | Track device/title presence changes for XUIDs and optional title IDs |
| `stop_tracking_presence(user, xuids := PackedStringArray(), title_ids := PackedInt64Array())` | `GDKResult` | Stop tracking specific XUIDs/title IDs, or all tracked values when arrays are empty |
| `get_cached_presence(xuid)` | `GDKPresenceRecord` | Get a cached presence record by XUID |

**Notes:**
- `state` is the configured rich-presence string ID for the title's SCID in Partner Center. It is not arbitrary display text.
- `get_presence_async(xuids)` uses the signed-in primary user as its Xbox services caller context, so `GDK.users.get_primary_user()` must be non-null.
- `get_presence_for_social_group_async(user, social_group)` uses the supplied local user as its Xbox Services caller context.

### Signals

| Signal | Description |
|--------|-------------|
| `presence_changed(xuid: String, presence: GDKPresenceRecord)` | A cached presence record was updated |
| `local_presence_set(user: GDKUser)` | Local user presence was set successfully |
| `device_presence_changed(xuid: String)` | A tracked user's device presence changed |
| `title_presence_changed(xuid: String, title_id: int)` | A tracked user's title presence changed |

### Usage

```gdscript
# Set presence
await GDK.presence.set_presence_async(user, "InGame")

# Query presence for a list of XUIDs
var result = await GDK.presence.get_presence_async(["1234567890123456"])
if result.ok:
    var record = GDK.presence.get_cached_presence("1234567890123456")
    print(record.gamertag, " is ", record.presence_text)
```

## `GDKPresenceRecord`

Script-visible wrapper around a cached Xbox presence record.

### Properties

| Property | Type | Description |
|----------|------|-------------|
| `xuid` | `String` | Xbox User ID |
| `gamertag` | `String` | Gamertag |
| `online` | `bool` | Whether the user is online |
| `presence_text` | `String` | Human-readable presence string |

## Social service: `GDK.social`

`GDK.social` is a `RefCounted` service object returned by `GDK.get_social()`.

### Methods

| Method | Returns | Description |
|--------|---------|-------------|
| `start_social_graph(user)` | `GDKResult` | Start tracking the social graph for a user |
| `stop_social_graph(user)` | `void` | Stop tracking the social graph for a user |
| `get_friends_async(user)` | `Signal` | Query the default friends group |
| `create_social_group(user, filter)` | `GDKSocialGroup` | Create a filtered social group |
| `create_social_group_from_xuids(user, xuids)` | `GDKSocialGroup` | Create a social group from explicit XUIDs |
| `destroy_social_group(group)` | `void` | Destroy a social group |
| `get_group_users(group)` | `Array` | Get the `GDKSocialUser` list for a group |
| `submit_reputation_feedback_async(user, target_xuid, feedback_type, reason := "", evidence_id := "")` | `Signal` | Submit one reputation feedback item |
| `submit_batch_reputation_feedback_async(user, feedback_items)` | `Signal` | Submit multiple reputation feedback items |

Batch reputation feedback items are dictionaries with `target_xuid` and
`feedback_type`, plus optional `reason` and `evidence_id`. Supported feedback
types are the `XblReputationFeedbackType` names in snake_case, such as
`fair_play_cheater`, `fair_play_quitter`, `communications_abusive_voice`,
`comms_text_message`, `positive_skilled_player`, `positive_helpful_player`,
and `user_content_gamerpic`.

### Signals

| Signal | Description |
|--------|-------------|
| `social_graph_changed(user: GDKUser)` | The social graph loaded or changed for a user |
| `social_group_updated(group: GDKSocialGroup)` | A social group's membership was updated |
| `social_user_changed(xuid: String, social_user: GDKSocialUser)` | A tracked user's social/presence data changed |

### Usage

```gdscript
GDK.social.social_graph_changed.connect(_on_graph_changed)
var result = GDK.social.start_social_graph(user)
if result.ok:
    var friends_result = await GDK.social.get_friends_async(user)
    if friends_result.ok:
        for friend in GDK.social.get_group_users(friends_result.data):
            print(friend.gamertag)
```

## `GDKSocialUser`

Script-visible wrapper around a tracked social user.

### Properties

| Property | Type | Description |
|----------|------|-------------|
| `xuid` | `String` | Xbox User ID |
| `gamertag` | `String` | Gamertag |
| `display_name` | `String` | Display name |
| `real_name` | `String` | Real name (if available) |
| `online` | `bool` | Whether the user is online |
| `playing_title_id` | `int` | Title ID the user is currently playing |
| `title_name` | `String` | Name of the title being played |
| `presence_text` | `String` | Human-readable presence string |

## `GDKSocialGroup`

Script-visible wrapper around a Social Manager group.

### Properties

| Property | Type | Description |
|----------|------|-------------|
| `presence_filter` | `GDKSocialFilter.PresenceFilter` | Presence filter used to create this group |
| `relationship_filter` | `GDKSocialFilter.RelationshipFilter` | Relationship filter used to create this group |

## `GDKSocialFilter`

Namespace for social filter enums.

### Enums

**`PresenceFilter`**

| Value | Description |
|-------|-------------|
| `UNKNOWN` | Unknown |
| `TITLE_ONLINE` | Online in this title |
| `TITLE_ALL_USERS` | All users for this title |
| `ALL_ONLINE` | All online users |
| `ALL_DEVICES` | All users on any device |
| `ALL_USERS` | All users |

**`RelationshipFilter`**

| Value | Description |
|-------|-------------|
| `FRIENDS` | Friends only |
| `FAVORITE` | Favorite friends only |

## Store service: `GDK.store`

`GDK.store` is a `RefCounted` service object returned by `GDK.get_store()`.
It wraps PC-supported `XStore` commerce APIs and caches the latest
license-acquire result per Store product ID. All operations require a
signed-in [`GDKUser`](#users-service-gdkusers) and an initialized [`GDK`](#root-singleton-gdk) runtime.

### Methods

| Method | Returns | Description |
|--------|---------|-------------|
| `query_license_status_async(user, store_id)` | `Signal` | Query whether `user` can acquire a license for `store_id`. Completion `GDKResult.data` is a `GDKStoreLicenseStatus` on success. |
| `refresh_entitlements_async(user, store_id)` | `Signal` | Refresh entitlements for `store_id` by re-querying license-acquire status. |
| `show_purchase_ui_async(user, store_id)` | `Signal` | Show the system purchase UI for `store_id`. |
| `get_cached_license_status(store_id)` | `GDKStoreLicenseStatus` | Returns the cached license status for `store_id`, or `null` when no cached value exists. |
| `check_cached_license_status(store_id)` | `GDKResult` | Synchronous cache-only check; returns `license_status_not_cached` when no cached status exists. |

### Usage

```gdscript
var result = await GDK.store.query_license_status_async(user, "9NBLGGH4R315")
if result.ok:
    var status: GDKStoreLicenseStatus = result.data
    print(status.store_id, status.licensable_sku, status.status)
```

### `GDKStoreLicenseStatus`

Typed wrapper around an `XStore` license-acquire result.

| Property | Type | Description |
|----------|------|-------------|
| `store_id` | `String` | Store product ID used for the query |
| `licensable_sku` | `String` | SKU value returned by `XStore` |
| `status` | `int` | Native `XStore` license status enum value |

## Profile service: `GDK.profile`

`GDK.profile` is a `RefCounted` service object returned by `GDK.get_profile()`.

### Methods

| Method | Returns | Description |
|--------|---------|-------------|
| `get_profile_async(user, xuid)` | `Signal` | Query one Xbox profile |
| `get_profiles_async(user, xuids)` | `Signal` | Query Xbox profiles for a list of XUID strings |
| `get_profiles_for_social_group_async(user, social_group)` | `Signal` | Query Xbox profiles for a social group such as `People` or `Favorites` |

On success, `get_profile_async()` returns a `GDKResult` whose `data` is a
`GDKUserProfile`. Batch and social-group queries return an `Array` of
`GDKUserProfile` objects.

### Usage

```gdscript
var result = await GDK.profile.get_profile_async(user, target_xuid)
if result.ok:
    var profile: GDKUserProfile = result.data
    print(profile.unique_modern_gamertag)
```

## `GDKUserProfile`

Script-visible wrapper around an Xbox Services profile record.

### Properties

| Property | Type | Description |
|----------|------|-------------|
| `xuid` | `String` | Xbox User ID |
| `app_display_name` | `String` | Application display name |
| `app_display_picture_resize_uri` | `String` | Application display picture resize URI |
| `game_display_name` | `String` | Game display name |
| `game_display_picture_resize_uri` | `String` | Game display picture resize URI |
| `gamerscore` | `String` | Gamer score string |
| `gamertag` | `String` | Classic gamertag |
| `modern_gamertag` | `String` | Modern gamertag |
| `modern_gamertag_suffix` | `String` | Modern gamertag suffix, if present |
| `unique_modern_gamertag` | `String` | Unique modern gamertag and suffix |

## String verification service: `GDK.string_verify`

`GDK.string_verify` is a `RefCounted` service object returned by
`GDK.get_string_verify()`.

### Methods

| Method | Returns | Description |
|--------|---------|-------------|
| `verify_string_async(user, text)` | `Signal` | Verify one string for Xbox Live acceptability |
| `verify_strings_async(user, strings)` | `Signal` | Verify multiple strings for Xbox Live acceptability |

On success, `verify_string_async()` returns a `GDKResult` whose `data` is a
dictionary with:

| Key | Type | Description |
|-----|------|-------------|
| `result_code` | `String` | `success`, `offensive`, `too_long`, or `unknown_error` |
| `acceptable` | `bool` | Whether the service accepted the string |
| `first_offending_substring` | `String` | First offending substring when available |

`verify_strings_async()` returns an `Array` of the same dictionaries.

### Usage

```gdscript
var result = await GDK.string_verify.verify_string_async(user, player_name)
if result.ok and not result.data.acceptable:
    push_warning("Rejected text: %s" % result.data.result_code)
```

## Title Storage service: `GDK.title_storage`

`GDK.title_storage` is a `RefCounted` service object returned by
`GDK.get_title_storage()`. This wraps Xbox Services Title Storage from
`title_storage_c.h`; it is unrelated to PlayFab Game Saves or GDK
`XGameSaveFiles`.

### Methods

| Method | Returns | Description |
|--------|---------|-------------|
| `get_quota_async(user, storage_type)` | `Signal` | Query quota for `trusted_platform`, `global`, or `universal` storage |
| `list_blob_metadata_async(user, storage_type, blob_path := "", skip_items := 0, max_items := 25)` | `Signal` | List blob metadata and return a paged result |
| `get_next_blob_metadata_async(result)` | `Signal` | Fetch the next metadata page |
| `download_blob_async(user, storage_type, blob_path)` | `Signal` | Download a blob by first querying its metadata |
| `upload_blob_async(user, storage_type, blob_path, data, display_name := "", e_tag := "", match_condition := "not_used")` | `Signal` | Upload bytes using binary blob metadata |
| `delete_blob_async(user, storage_type, blob_path, e_tag := "", match_condition := "not_used")` | `Signal` | Delete a binary blob; `match_condition` supports `not_used` and `if_match` |

Quota results return a dictionary with `storage_type`, `used_bytes`, and
`quota_bytes`. Download results return a dictionary with `metadata` and `data`.
Upload results return a `GDKTitleStorageBlobMetadata`.

### Usage

```gdscript
var list_result = await GDK.title_storage.list_blob_metadata_async(
        user, "universal", "saves", 0, 25)
if list_result.ok:
    var page: GDKTitleStorageBlobMetadataResult = list_result.data
    for metadata: GDKTitleStorageBlobMetadata in page.items:
        print(metadata.blob_path, " -> ", metadata.length)
```

## `GDKTitleStorageBlobMetadata`

Script-visible wrapper around Title Storage blob metadata.

### Properties

| Property | Type | Description |
|----------|------|-------------|
| `blob_path` | `String` | Blob path |
| `blob_type` | `String` | `binary`, `json`, `config`, or `unknown` |
| `storage_type` | `String` | `trusted_platform`, `global`, `universal`, or `unknown` |
| `display_name` | `String` | Friendly display name |
| `e_tag` | `String` | Service ETag |
| `client_timestamp` | `int` | Client timestamp |
| `length` | `int` | Blob length in bytes |
| `service_configuration_id` | `String` | SCID |
| `xuid` | `String` | Owning XUID when present |

## `GDKTitleStorageBlobMetadataResult`

Paged Title Storage metadata result. Keep the object alive while requesting
additional pages.

### Properties

| Property | Type | Description |
|----------|------|-------------|
| `items` | `Array` | `GDKTitleStorageBlobMetadata` items |
| `has_next` | `bool` | Whether another page can be fetched |
| `storage_type` | `String` | Storage type for the query |
| `blob_path` | `String` | Blob path prefix for the query |

## Error reporting service: `GDK.error_reporting`

`GDK.error_reporting` is a `RefCounted` service object returned by
`GDK.get_error_reporting()`.

This service wraps the public PC GDK `XError` callback/options APIs
(`XErrorSetCallback`, `XErrorSetOptions`). It does not submit reports to
external endpoints.

### Methods

| Method | Returns | Description |
|--------|---------|-------------|
| `configure_options(debugger_present_options := GDKErrorReporting.ERROR_OPTIONS_NONE, debugger_not_present_options := GDKErrorReporting.ERROR_OPTIONS_NONE)` | `GDKResult` | Configure `XError` behavior using `GDKErrorReporting.ErrorOptions` enum flags (bitwise OR supported) |
| `set_callback_enabled(enabled)` | `GDKResult` | Enable/disable forwarding from `XError` callback into Godot signals |
| `is_callback_enabled()` | `bool` | Whether callback forwarding is currently enabled |

### `ErrorOptions` enum

| Value | Description |
|--------|-------------|
| `ERROR_OPTIONS_NONE` (`0`) | No special error behavior options |
| `ERROR_OPTIONS_OUTPUT_DEBUG_STRING_ON_ERROR` (`1`) | Request `OutputDebugString` behavior when an error occurs |
| `ERROR_OPTIONS_DEBUG_BREAK_ON_ERROR` (`2`) | Request debug-break behavior when an error occurs |
| `ERROR_OPTIONS_FAIL_FAST_ON_ERROR` (`4`) | Request fail-fast behavior when an error occurs |

### Signals

| Signal | Description |
|--------|-------------|
| `error_reported(result: GDKResult)` | Emitted when `XError` callback reports an error; also mirrored through `GDK.runtime_error(result)` |

**Privacy note:** if your title attaches metadata to downstream telemetry based
on callback events, your title owns privacy/compliance review for that metadata.

## Multiplayer activity service: `GDK.multiplayer_activity`

`GDK.multiplayer_activity` is a `RefCounted` service object returned by
`GDK.get_multiplayer_activity()`.

### Methods

| Method | Returns | Description |
|--------|---------|-------------|
| `set_activity_async(user, connection_string, join_restriction, max_players, current_players, group_id, allow_cross_platform_join)` | `Signal` | Set the current multiplayer activity for a user |
| `get_activities_async(user, xuids)` | `Signal` | Fetch activities for a list of XUIDs |
| `get_cached_activity(xuid)` | `GDKMultiplayerActivityInfo` | Get a cached activity by XUID (or `null`) |
| `delete_activity_async(user)` | `Signal` | Delete the current user's activity |
| `send_invites_async(user, xuids, allow_cross_platform_join, connection_string)` | `Signal` | Send invites to the given XUIDs |
| `show_invite_ui_async(user)` | `Signal` | Show the system invite UI |
| `update_recent_players(user, xuids, encounter_type)` | `GDKResult` | Record recent-player encounters |
| `flush_recent_players_async(user)` | `Signal` | Flush pending recent-player records |
| `accept_pending_invite(invite_uri)` | `GDKResult` | Parse and accept a pending invite URI |

### Signals

| Signal | Description |
|--------|-------------|
| `activities_updated(xuids: PackedStringArray)` | One or more activities were updated in the cache |
| `pending_invite_received(invite: Dictionary)` | A pending invite was received at startup |
| `invite_accepted(invite: Dictionary)` | The user accepted a multiplayer invite |

### Usage

```gdscript
var mpa = GDK.multiplayer_activity

# Set your activity
await mpa.set_activity_async(user, "myserver://connect?session=abc",
        "followed", 4, 1)

# Fetch another player's activity
var result = await mpa.get_activities_async(user, [other_xuid])
if result.ok:
    var info = mpa.get_cached_activity(other_xuid)
    print(info.get_connection_string())

# React to accepted invites
mpa.invite_accepted.connect(func(invite):
    get_tree().change_scene_to_file("res://multiplayer.tscn")
    # use invite["connection_string"] to connect
)
```

## `GDKMultiplayerActivityInfo`

Script-visible wrapper around a cached multiplayer activity snapshot.

### Methods

| Method | Returns | Description |
|--------|---------|-------------|
| `get_xuid()` | `String` | XUID of the player this activity belongs to |
| `get_connection_string()` | `String` | Connection string for joining the session |
| `get_join_restriction()` | `String` | Join restriction (`"public"`, `"followed"`, `"invite_only"`) |
| `get_max_players()` | `int` | Maximum players in the session |
| `get_current_players()` | `int` | Current player count |
| `get_group_id()` | `String` | Optional group identifier |
| `get_platform()` | `String` | Platform the activity was set from |

## Capture service: `GDK.capture`

`GDK.capture` is a `RefCounted` service object returned by `GDK.get_capture()`.

### Methods

| Method | Returns | Description |
|--------|---------|-------------|
| `enable_capture()` | `GDKResult` | Re-enable Game Bar capture for this title after a previous `disable_capture()` call |
| `disable_capture()` | `GDKResult` | Disable Game Bar capture for this title |
| `record_diagnostic_clip_async(duration: float)` | `Signal` | Record a diagnostic video clip of the given duration in seconds and return a deferred completion signal. Requires Game Bar. |
| `take_diagnostic_screenshot_async(path_hint: String)` | `Signal` | Take a diagnostic screenshot and return a deferred completion signal. Requires Game Bar. |
| `create_metadata(reserved_bytes := 0)` | `GDKCaptureMetaData` | Create a script-side metadata write context, or `null` if the runtime is not initialized. `reserved_bytes` is retained for compatibility and ignored. |

### PC GDK availability

All wrapped APIs are available in `_GAMING_DESKTOP` builds via `XAppCapture.h` / `xgameruntime.lib`.

| Native function | PC GDK |
|---|---|
| `XAppCaptureEnableRecord` | YES |
| `XAppCaptureDisableRecord` | YES |
| `XAppCaptureRecordDiagnosticClip` | YES (Game Bar) |
| `XAppCaptureTakeDiagnosticScreenshot` | YES (Game Bar) |
| `XAppCaptureMetadataAddStringEvent` | YES |
| `XAppCaptureMetadataAddDoubleEvent` | YES |
| `XAppCaptureMetadataAddInt32Event` | YES |
| `XAppCaptureMetadataStartStringState` | YES |
| `XAppCaptureMetadataStartDoubleState` | YES |
| `XAppCaptureMetadataStartInt32State` | YES |
| `XAppCaptureMetadataStopAllStates` | YES |
| `XAppCaptureMetadataRemainingStorageBytesAvailable` | YES |

Excluded (console-only): `XAppCaptureOpenLocalStorageFiles`, `XAppCaptureCloseLocalStorageFilesHandle`, `XAppCaptureDiagnosticClipLocalId` result extraction.

### Usage

```gdscript
# Annotate capture with metadata
var meta = GDK.capture.create_metadata()
if meta:
    meta.start_string_state("zone", "lobby")
    meta.add_int32_event("score", 9001)
    # ... gameplay ...
    meta.stop_all_states()
    meta.close()

# Record a short clip (requires Game Bar active)
var result = await GDK.capture.record_diagnostic_clip_async(10.0)
if result.ok:
    print("Clip recorded")
```

## `GDKCaptureMetaData`

Script-side write context for the process-wide `XAppCaptureMetadata*` APIs. Created by `GDK.capture.create_metadata()`.

### Methods

| Method | Returns | Description |
|--------|---------|-------------|
| `is_valid()` | `bool` | Whether the script-side metadata context is open |
| `close()` | `void` | Close the script-side context early (also closed automatically on free) |
| `stop_all_states()` | `GDKResult` | Stop all active persistent metadata states |
| `get_remaining_storage_bytes()` | `int` | Bytes remaining in the local metadata buffer (`-1` on error) |
| `add_string_event(name, value, priority := 0)` | `GDKResult` | Write a one-shot string event |
| `add_double_event(name, value, priority := 0)` | `GDKResult` | Write a one-shot double event |
| `add_int32_event(name, value, priority := 0)` | `GDKResult` | Write a one-shot int32 event |
| `start_string_state(name, value, priority := 0)` | `GDKResult` | Begin a persistent string state |
| `start_double_state(name, value, priority := 0)` | `GDKResult` | Begin a persistent double state |
| `start_int32_state(name, value, priority := 0)` | `GDKResult` | Begin a persistent int32 state |

**`Priority` enum**

| Value | Description |
|-------|-------------|
| `PRIORITY_GAMEPLAY` (`0`) | Standard gameplay-level priority (`XAppCaptureMetadataPriority::Informational`) |
| `PRIORITY_IMPORTANT` (`1`) | Higher-importance priority (`XAppCaptureMetadataPriority::Important`) |

## Launcher service: `GDK.launcher`

`GDK.launcher` is a `RefCounted` service object returned by `GDK.get_launcher()`.
It wraps PC-supported `XLaunchUri` flows.

### Methods

| Method | Returns | Description |
|--------|---------|-------------|
| `launch_uri(uri, user := null)` | `GDKResult` | Launch an absolute URI with `XLaunchUri` |

### Validation notes

- Blank or malformed URIs return `invalid_uri`.
- `unsupported_launcher_destination` is returned for blocked destinations such as
  `file:`, `javascript:`, `data:`, and `about:` URIs, and for `ms-*` URIs other
  than `ms-settings:` and `ms-windows-store:`.
- Unsigned/invalid optional users return `invalid_user`.

### Manual smoke coverage

Launcher success paths depend on host OS URI handlers and package context and are
not deterministic for CI. Use a manual smoke pass on a PC GDK machine:

```gdscript
var result = GDK.launcher.launch_uri("ms-settings:privacy-microphone")
print(result.ok, result.code, result.message)
```

### `GDKResult`

Normalized result payload returned by all async operations.

| Field | Type | Description |
|-------|------|-------------|
| `ok` | `bool` | Whether the operation succeeded |
| `hresult` | `int` | Native HRESULT code |
| `code` | `String` | Error code string |
| `message` | `String` | Human-readable error message |
| `data` | `Variant` | Operation payload (e.g., `GDKUser`, `Dictionary`, `Image`) |

## Display service: `GDK.display`

`GDK.display` is a `RefCounted` service object returned by `GDK.get_display()`. It
wraps the PC GDK `XDisplay.h` family: HDR mode probe/enable and idle display
timeout deferrals.

### Methods

| Method | Returns | Description |
|--------|---------|-------------|
| `try_enable_hdr_mode(preference := HDR_MODE_PREFERENCE_PREFER_HDR)` | `GDKResult` | Probe and best-effort enable HDR via `XDisplayTryEnableHdrMode` |
| `acquire_timeout_deferral()` | `GDKResult` | Acquire a `GDKDisplayTimeoutDeferral` that suppresses idle display blanking |

### Constants

| Constant | Value | Description |
|----------|-------|-------------|
| `HDR_MODE_UNKNOWN` (`0`) | Mode could not be determined |
| `HDR_MODE_ENABLED` (`1`) | Display reports HDR enabled |
| `HDR_MODE_DISABLED` (`2`) | Display reports HDR disabled |
| `HDR_MODE_PREFERENCE_PREFER_HDR` (`0`) | Prefer HDR even if it lowers refresh rate |
| `HDR_MODE_PREFERENCE_PREFER_REFRESH_RATE` (`1`) | Prefer high refresh rate over HDR |

### Result data

`try_enable_hdr_mode` success payload (`GDKResult.data`) is a `Dictionary`:

| Key | Type | Description |
|-----|------|-------------|
| `mode` | `int` | One of the `HDR_MODE_*` constants above |
| `info` | `Dictionary` | Present when `mode == HDR_MODE_ENABLED`. Contains `min_tone_map_luminance`, `max_tone_map_luminance`, and `max_full_frame_tone_map_luminance` (all `float`). |

`acquire_timeout_deferral` success payload is a `GDKDisplayTimeoutDeferral`
ref-counted handle wrapper. Call `release()` (or drop all references) to release
the deferral and re-enable system idle behavior.

### Validation notes

- All methods return `not_initialized` when called before `GDK.initialize()`.
- `try_enable_hdr_mode` returns `invalid_preference` for unknown preference values.
- Native failures surface as `hdr_mode_failed` / `acquire_timeout_deferral_failed`
  with the underlying HRESULT formatted into `message`.

### `GDKDisplayTimeoutDeferral`

Ref-counted wrapper around an `XDisplayTimeoutDeferralHandle`.

| Method | Returns | Description |
|--------|---------|-------------|
| `is_valid()` | `bool` | `true` while the underlying handle is open |
| `release()` | `void` | Closes the handle (idempotent); also runs in the destructor |

Construction is internal — instances are produced by
`GDK.display.acquire_timeout_deferral()`.

## Activation service: `GDK.activation`

`GDK.activation` is a `RefCounted` service object returned by
`GDK.get_activation()`. It wraps `XGameActivation.h`, the modern replacement for
the deprecated `XGameProtocol.h` registration. Subscribe to the typed signals to
react to protocol launches, file launches, pending invites, and accepted
invites.

### Methods

| Method | Returns | Description |
|--------|---------|-------------|
| `accept_pending_invite(invite_uri)` | `GDKResult` | Accept a pending invite URI via `XGameActivationAcceptPendingInvite` |

### Signals

| Signal | Arguments | Description |
|--------|-----------|-------------|
| `protocol_activated` | `uri: String` | The title was launched via a protocol URI |
| `file_activated` | `file: String` | The title was launched with a file association |
| `pending_invite_received` | `invite_uri: String` | A multiplayer invite is pending; pair with `accept_pending_invite` |
| `invite_accepted` | `invite_uri: String` | An invite was accepted (typically by the system) |
| `activated` | `info: Dictionary` | Catch-all event; `info.type` is one of `ACTIVATION_TYPE_*` and includes `uri`/`file`/`invite_uri` matching the typed signal |

### Constants

| Constant | Value | Description |
|----------|-------|-------------|
| `ACTIVATION_TYPE_PROTOCOL` (`0`) | Protocol activation |
| `ACTIVATION_TYPE_FILE` (`1`) | File activation |
| `ACTIVATION_TYPE_PENDING_GAME_INVITE` (`2`) | Pending game invite |
| `ACTIVATION_TYPE_ACCEPTED_GAME_INVITE` (`3`) | Accepted game invite |

### Validation notes

- `accept_pending_invite` returns `not_initialized` before `GDK.initialize()`.
- Empty or whitespace-only URIs return `invalid_invite_uri`.
- Native failures surface as `accept_pending_invite_failed` with the underlying
  HRESULT formatted into `message`.
- If activation registration fails on this host (e.g., a partially registered
  package), the synchronous `accept_pending_invite` API still works; the
  signal-driven flow simply emits no events. A `push_warning` is logged once.
