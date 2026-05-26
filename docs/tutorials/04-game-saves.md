# Tutorial 4 — Save the player's progress

## What you'll build

A persistent save flow on top of PlayFab Game Saves, backed by the
Xbox-signed-in PlayFab session you reached in tutorial 1. By the end
you will:

- Add the signed-in PlayFab user to Game Saves with
  `add_user_with_ui_async`.
- Resolve the on-disk save folder for that user, write a save file
  into it through normal Godot file IO, and call
  `upload_with_ui_async` to push it to the cloud.
- Inspect the cloud-connection state, remaining quota, and folder
  size for debugging and pre-flight checks.
- Handle conflict and rollback states by passing the
  `ADD_USER_OPTION_*` flags.

Sample output:

```
[Save] Game Saves folder: C:/Users/.../LocalAppState/.../GameSaves/...
[Save] Cloud connected: true, quota left: 268435456 bytes
[Save] Wrote save: highscore=1234
[Save] Upload complete
```

## Prerequisites

- [Tutorial 1 — Sign in a user](01-sign-in-user.md) is complete; the
  snippets below read `Auth.playfab_user` and need
  `has_local_user_handle == true`. PlayFab sessions that were created
  with `sign_in_with_custom_id_async` are rejected by every Game
  Saves call with the `xbox_user_required` code — Game Saves is
  Xbox-backed only.
- Your `MicrosoftGame.config` declares Game Saves storage. The
  template that the **GDK → Create MicrosoftGame.config** menu writes
  already sets a `CloudSaves` block; if your config was created
  before that template was added, open `GameConfigEditor.exe` and
  confirm a CloudSaves section is present.
- Network connectivity for the cloud sync round-trips. Game Saves
  also works offline (with reduced semantics) — every method's
  result has an explicit "cloud connected" bit you can check.

> **Game Saves vs. PlayFab title data / entity data.** Game Saves is
> the Xbox-attached blob store that follows the Xbox account across
> devices and surfaces in the system Cloud Saves UI. Use it for
> player-progress blobs. Use `PlayFab.entity_data` or
> `PlayFab.player_data` for structured per-player JSON that does not
> need the Xbox-backed sync semantics.

## Relevant addon surfaces

- [`PlayFab.game_saves`](../playfab/plugin.md) —
  `add_user_with_ui_async`, `upload_with_ui_async`,
  `get_folder`, `get_folder_size`, `get_remaining_quota`,
  `is_connected_to_cloud`, `set_save_description_async`,
  `reset_cloud_async`.
- [`PlayFabUser`](../playfab/plugin.md) — read
  `has_local_user_handle` to confirm an Xbox-backed session.
- One-page primer on the addons' async model:
  [Async patterns](../async-patterns.md).

## Step 1 — Add the user to Game Saves

Game Saves treats your signed-in PlayFab session as the identity for
all reads and writes. Call `add_user_with_ui_async` once after
sign-in; subsequent reads come from cache:

```gdscript
extends Node

var _save_folder: String = ""

func _ready() -> void:
    if Auth.playfab_user == null:
        await Auth.sign_in_completed

    await _add_to_game_saves()

func _add_to_game_saves() -> void:
    var user: PlayFabUser = Auth.playfab_user
    if not user.has_local_user_handle:
        push_error("[Save] PlayFab session is custom-id; Game Saves needs Xbox.")
        return

    var result: PlayFabResult = await PlayFab.game_saves.add_user_with_ui_async(user)
    if not result.ok:
        push_warning("[Save] Add user failed: %s (%s)" % [result.message, result.code])
        return

    var data: Dictionary = result.data
    _save_folder = data.get("folder", "")
    var connected: bool = data.get("connected_to_cloud", false)
    var quota: int = data.get("remaining_quota", -1)

    print("[Save] Game Saves folder: %s" % _save_folder)
    print("[Save] Cloud connected: %s, quota left: %d bytes" % [str(connected), quota])
```

The result payload contains the synced folder path, the cloud
connection state, the local user id (for cross-referencing your own
data structures), the entity key, and — when cloud-connected — the
remaining quota. Cache `_save_folder` once and reuse it; subsequent
calls do not need to re-add the user.

The first call to `add_user_with_ui_async` may surface system UI to
resolve a save conflict ("which save do you want to keep?") if one
exists. That is the system UI mentioned in the method name, not a
sign-in prompt.

## Step 2 — Write a save file

Once you have the folder, **write to it the way you would any other
file** — Game Saves is a file-system-backed store. The Godot
`FileAccess` API works directly against the resolved path:

```gdscript
func _write_save(highscore: int) -> void:
    if _save_folder.is_empty():
        push_error("[Save] _save_folder not resolved yet")
        return

    var path := _save_folder.path_join("progress.json")
    var file := FileAccess.open(path, FileAccess.WRITE)
    if file == null:
        push_error("[Save] Open failed: %s" % path)
        return

    var payload := {
        "highscore": highscore,
        "saved_at": Time.get_datetime_string_from_system(true),
    }
    file.store_string(JSON.stringify(payload))
    file.close()

    print("[Save] Wrote save: highscore=%d" % highscore)
```

A few rules of thumb:

- **Treat the folder as flat.** Game Saves is a single container per
  user. Subdirectories are allowed, but most titles keep one or two
  top-level files (`progress.json`, `slot1.dat`).
- **Stay under the quota.** Game Saves quota per user is typically a
  small number of MB — read `remaining_quota` from the
  `add_user_with_ui_async` result, or query it ad-hoc with
  `get_remaining_quota` before a write. Exceeding the quota fails
  the upload, not the local write.
- **Atomic-write on top of `FileAccess`.** For multi-megabyte saves,
  write to a sibling temp file (`progress.tmp`), `close()`, then
  `DirAccess.rename_absolute(...)`. Game Saves does not give you a
  transaction primitive — interrupted partial writes survive.

## Step 3 — Upload to the cloud

Local writes do **not** sync until you call `upload_with_ui_async`.
That is intentional — it lets you batch multiple file writes into
one cloud round-trip:

```gdscript
func _upload(description: String) -> void:
    var user: PlayFabUser = Auth.playfab_user

    if not description.is_empty():
        var desc_result: PlayFabResult = await PlayFab.game_saves.set_save_description_async(user, description)
        if not desc_result.ok:
            push_warning("[Save] Description set failed: %s" % desc_result.message)

    var result: PlayFabResult = await PlayFab.game_saves.upload_with_ui_async(user, false)
    if result.ok:
        print("[Save] Upload complete")
    else:
        push_warning("[Save] Upload failed: %s (%s)" % [result.message, result.code])
```

- The optional **save description** is the short string that surfaces
  in the system Cloud Saves UI when the user resolves a conflict
  ("Slot 1 — Level 3, 12:42:08"). Set it before the upload so the
  UI shows something meaningful.
- The second parameter (`release_device_as_active`) is `false` for
  the normal case where this device keeps owning the save. Pass
  `true` when the user is intentionally switching devices and you
  want the next device to be the active one without going through
  the "active device" conflict UI.

## Step 4 — Inspect cloud state before saving

For pre-flight diagnostics or a "your save is X% full" HUD, the
helpers under `PlayFab.game_saves` return synchronous
`PlayFabResult` values backed by cache:

```gdscript
func _print_cloud_state() -> void:
    var user: PlayFabUser = Auth.playfab_user

    var connected: PlayFabResult = PlayFab.game_saves.is_connected_to_cloud(user)
    if connected.ok:
        print("[Save] Cloud connected: %s" % str(connected.data))

    var folder_size: PlayFabResult = PlayFab.game_saves.get_folder_size(user)
    if folder_size.ok:
        print("[Save] Folder size on disk: %d bytes" % int(folder_size.data))

    var quota: PlayFabResult = PlayFab.game_saves.get_remaining_quota(user)
    if quota.ok:
        print("[Save] Remaining quota: %d bytes" % int(quota.data))
```

These never call out to the cloud; they read state that the most
recent `add_user_with_ui_async` or `upload_with_ui_async` cached.
That makes them cheap to call on every HUD redraw.

## Step 5 — Handle conflicts and rollback

When two devices write conflicting saves, the system surfaces a
conflict in the Cloud Saves UI. Game Saves preserves both sides,
and the next `add_user_with_ui_async` call can pick which one
becomes the new canonical save by passing one of the
`ADD_USER_OPTION_*` constants:

```gdscript
func _recover_after_conflict() -> void:
    var user: PlayFabUser = Auth.playfab_user

    # Roll back to the most recent verified-good cloud snapshot.
    var result: PlayFabResult = await PlayFab.game_saves.add_user_with_ui_async(
        user, PlayFabGameSaves.ADD_USER_OPTION_ROLLBACK_TO_LAST_KNOWN_GOOD)
    if result.ok:
        print("[Save] Rolled back to last known good save")
    else:
        push_warning("[Save] Rollback failed: %s" % result.message)
```

The three options today are:

| Constant | Behavior |
|---|---|
| `ADD_USER_OPTION_NONE` | Default. Sync to the latest cloud save. |
| `ADD_USER_OPTION_ROLLBACK_TO_LAST_KNOWN_GOOD` | Sync to the most recently verified earlier cloud save instead of the latest upload (when such a snapshot exists). |
| `ADD_USER_OPTION_ROLLBACK_TO_LAST_CONFLICT` | Restore the "losing" side from the most recent conflict resolution (when available). |

For development-only recovery you can also call
`reset_cloud_async`, which clears the cloud state without touching
the local folder. **Do not** ship `reset_cloud_async` behind any
player-facing UI — it is a destructive recovery hatch for the title
team.

## Verify

A successful first run prints:

```
[Save] Game Saves folder: C:/Users/.../LocalAppState/.../GameSaves/...
[Save] Cloud connected: true, quota left: 268435456 bytes
[Save] Wrote save: highscore=1234
[Save] Upload complete
[Save] Folder size on disk: 86 bytes
[Save] Remaining quota: 268435370 bytes
```

The save shows up in the system Cloud Saves UI for that account,
and the saved JSON survives a restart of the editor / shipped game.

Common failures:

| Output | Diagnosis | Fix |
|---|---|---|
| `Add user failed: xbox_user_required` | The PlayFab session is custom-id, not Xbox-backed. | Sign in through `sign_in_with_xuser_async` (tutorial 1), not `sign_in_with_custom_id_async`. |
| `Add user failed: not_initialized` | PlayFab runtime did not initialize. | Set `playfab/runtime/title_id` and re-run sign-in. |
| `Upload failed: out_of_quota` | The save folder is over the per-user quota. | Trim files in `_save_folder` before retrying; `get_remaining_quota` reports the budget. |
| `Upload failed: not_connected` | The PC has no internet, or PlayFab Game Saves connectivity dropped. | Cache the local write and retry the upload when connectivity returns. `is_connected_to_cloud(user)` is your check. |
| `Open failed` on `FileAccess.open` | Game Saves has not yet resolved a folder for this user. | Confirm `add_user_with_ui_async` succeeded and `_save_folder` is non-empty before any file IO. |

## What's next

You now have persistent, Xbox-backed cloud saves. Tutorial 5 sets up
the multiplayer side — creating and joining a PlayFab lobby:

- [**Tutorial 5 — Create and join a lobby**](05-multiplayer-lobby.md)
- Reference: [PlayFabGameSaves](../playfab/plugin.md),
  [PlayFabUser](../playfab/plugin.md)
