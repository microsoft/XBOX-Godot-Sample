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
| `dispatch()` | `int` | Pump async completions and manager state (call every frame) |
| `get_last_error()` | `GDKResult` | Last error result |
| `get_users()` | `GDKUsers` | Access the users service |
| `get_achievements()` | `GDKAchievements` | Access the achievements service |

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

func _process(_delta):
    GDK.dispatch()

func _exit_tree():
    GDK.shutdown()
```

## Users service: `GDK.users`

`GDK.users` is a `RefCounted` service object returned by `GDK.get_users()`.

### Methods

| Method | Returns | Description |
|--------|---------|-------------|
| `add_default_user_async(allow_guests)` | `GDKAsyncOp` | Silent Xbox sign-in |
| `add_user_with_ui_async()` | `GDKAsyncOp` | Xbox sign-in with UI prompt |
| `get_primary_user()` | `GDKUser` | Current primary user (or `null`) |
| `get_users()` | `Array` | All local users |
| `check_privilege_async(user, privilege)` | `GDKAsyncOp` | Check user privilege |
| `resolve_privilege_with_ui_async(user, privilege)` | `GDKAsyncOp` | Resolve privilege with UI |
| `resolve_issue_with_ui_async(user, url)` | `GDKAsyncOp` | Resolve account issue with UI |
| `get_gamer_picture_async(user, size)` | `GDKAsyncOp` | Fetch user's profile picture |
| `get_token_and_signature_async(user, method, url, headers, body, force_refresh)` | `GDKAsyncOp` | Get Xbox Live auth token |

### Signals

| Signal | Description |
|--------|-------------|
| `user_added(user: GDKUser)` | A new user was added |
| `user_removed(local_id: int)` | A user was removed |
| `user_changed(user: GDKUser)` | A user's state changed |
| `primary_user_changed(user: GDKUser)` | The primary user changed |

### Usage

```gdscript
func _ready():
    GDK.users.user_added.connect(_on_user_added)
    var op = GDK.users.add_default_user_async()
    var result = await op.completed
    if result.ok:
        print("Signed in: ", result.data.gamertag)

func _on_user_added(user: GDKUser):
    print("User added: ", user.gamertag)
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
| `is_guest()` | `bool` | Whether the user is a guest |
| `is_signed_in()` | `bool` | Whether the user is signed in |
| `is_store_user()` | `bool` | Whether the user is a store user |

## Achievements service: `GDK.achievements`

`GDK.achievements` is a `RefCounted` service object returned by
`GDK.get_achievements()`. It uses the Achievements Manager pattern
with dispatch-backed ops.

### Methods

| Method | Returns | Description |
|--------|---------|-------------|
| `query_player_achievements_async(user)` | `GDKDispatchOp` | Query achievements for a user |
| `update_achievement_async(user, achievement_id, percent_complete)` | `GDKDispatchOp` | Update achievement progress |
| `get_cached_achievements(user)` | `Array` | Get cached achievement list |

### Signals

| Signal | Description |
|--------|-------------|
| `achievement_unlocked(user: GDKUser, achievement_id: String)` | An achievement was unlocked |
| `achievements_updated(user: GDKUser)` | Achievement cache was updated |

### Usage

```gdscript
# Query achievements
var op = GDK.achievements.query_player_achievements_async(user)
var result = await op.completed
if result.ok:
    for achievement in GDK.achievements.get_cached_achievements(user):
        print(achievement.name, ": ", achievement.progress_percent, "%")

# Update progress (25% increments)
var update_op = GDK.achievements.update_achievement_async(user, "1", 25)
await update_op.completed
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

## Async operation types

### `GDKAsyncOp`

One-shot wrapper for `XAsync`-backed requests.

| Member | Type | Description |
|--------|------|-------------|
| `completed` | Signal | Emitted once with `GDKResult` |
| `is_done()` | `bool` | Whether the op has completed |
| `cancel()` | `bool` | Best-effort cancellation |
| `get_result()` | `GDKResult` | Result (only valid after completion) |

### `GDKDispatchOp`

One-shot wrapper for dispatch/manager-driven waits (e.g., Achievements
Manager).

Same surface as `GDKAsyncOp` (`completed`, `is_done()`, `cancel()`,
`get_result()`), but cancel immediately unregisters from service state and
completes with a cancelled result.

### `GDKResult`

Normalized result payload returned by all async operations.

| Field | Type | Description |
|-------|------|-------------|
| `ok` | `bool` | Whether the operation succeeded |
| `hresult` | `int` | Native HRESULT code |
| `code` | `String` | Error code string |
| `message` | `String` | Human-readable error message |
| `data` | `Variant` | Operation payload (e.g., `GDKUser`, `Dictionary`, `Image`) |
