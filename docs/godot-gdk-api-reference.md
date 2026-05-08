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
| `get_accessibility()` | `GDKAccessibility` | Access the accessibility service |
| `get_achievements()` | `GDKAchievements` | Access the achievements service |
| `get_presence()` | `GDKPresence` | Access the presence service |
| `get_social()` | `GDKSocial` | Access the social graph service |
| `get_launcher()` | `GDKLauncher` | Access the launcher service for URI/store/settings flows |
| `get_multiplayer_activity()` | `GDKMultiplayerActivity` | Access the multiplayer activity service |

### Signals

| Signal | Description |
|--------|-------------|
| `initialized()` | Runtime initialized successfully |
| `shutdown_completed()` | Runtime shutdown complete |
| `runtime_error(result: GDKResult)` | A runtime error occurred |
| `availability_changed(available: bool)` | Runtime availability changed |

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

## Presence service: `GDK.presence`

`GDK.presence` is a `RefCounted` service object returned by `GDK.get_presence()`.

### Methods

| Method | Returns | Description |
|--------|---------|-------------|
| `set_presence_async(user, state, rich_presence)` | `Signal` | Set rich presence for a local user |
| `clear_presence_async(user)` | `Signal` | Clear presence for a local user |
| `get_presence_async(xuids)` | `Signal` | Query presence records for a list of XUIDs |
| `get_cached_presence(xuid)` | `GDKPresenceRecord` | Get a cached presence record by XUID |

**Notes:**
- `state` is the configured rich-presence string ID for the title's SCID in Partner Center. It is not arbitrary display text.
- `get_presence_async(xuids)` uses the signed-in primary user as its Xbox services caller context, so `GDK.users.get_primary_user()` must be non-null.

### Signals

| Signal | Description |
|--------|-------------|
| `presence_changed(record: GDKPresenceRecord)` | A cached presence record was updated |
| `local_presence_set(user: GDKUser)` | Local user presence was set successfully |

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

### Signals

| Signal | Description |
|--------|-------------|
| `social_graph_changed(user: GDKUser)` | The social graph loaded or changed for a user |
| `social_group_updated(group: GDKSocialGroup)` | A social group's membership was updated |
| `social_user_changed(social_user: GDKSocialUser)` | A tracked user's social/presence data changed |

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
