# Godot PlayFab plugin

This is the landing page for the `godot_playfab` docs set.

`godot_playfab` is the PlayFab-focused GDExtension addon in this repository. It currently implements a single `PlayFab` root singleton, manual user sign-in keyed by local Xbox user id, PlayFab Game Saves wrappers, and PlayFab leaderboard submission/query flows.

## Current implementation status

### Implemented now

- root singleton registration through `PlayFab`
- shared PlayFab runtime lifecycle through `XGameRuntimeInitialize`, `PFInitialize`, and `PFServicesInitialize`
- shared Game Saves runtime lifecycle through `PFGameSaveFilesInitialize` and `PFGameSaveFilesUninitializeAsync`
- default auto-dispatch through `playfab/runtime/embed_dispatch`
- project-settings-backed PlayFab config through `playfab/titleid` and `playfab/endpoint`
- manual PlayFab sign-in through `PlayFab.sign_in_async(...)` / `PlayFab.users.sign_in_async(...)`
- cached `PlayFabUser` wrappers keyed by local Xbox user id
- Game Saves add/sync, upload, folder/quota queries, cloud connectivity queries, save description updates, and cloud reset through `PlayFab.game_saves`
- leaderboard submit, global query, around-user query, and friends/social leaderboard query
- sample demos wired to the new root singleton, including multiplayer_pong's
  sample-local service wrapper for Game Saves and leaderboard sync
- headless contract coverage under `sample\playfab_demo\tests\run_tests.gd`
- idempotent extension registration so duplicated synced `.gdextension` files do not spam duplicate singleton/class registration

### Not implemented yet

- broader PlayFab feature areas from the legacy codebase such as multiplayer or party services are not part of the new public root API
- custom non-Windows Game Saves UI callback/response wrappers are not yet exposed as a public Godot surface

## Runtime configuration

The PlayFab runtime reads these settings from Project Settings:

- `playfab/titleid` — required; the PlayFab title id
- `playfab/endpoint` — optional; leave blank to derive `https://<titleid>.playfabapi.com`
- `playfab/runtime/embed_dispatch` — defaults to `true`; disable only when you want to pump completions manually with `PlayFab.dispatch()`

## Public GDScript surface

- `PlayFab`
- `PlayFab.users`
- `PlayFab.game_saves`
- `PlayFab.leaderboards`
- `PlayFabGameSaves`
- `PlayFabUser`
- `PlayFabResult`

All PlayFab one-shot async methods now return completion signals that you await directly. `PlayFab.users` is intentionally cache/result-driven and does not expose user lifecycle signals.

## Sample usage

```gdscript
if not PlayFab.is_initialized():
    var init_result = PlayFab.initialize()
    if not init_result.ok:
        push_warning(init_result.message)
        return

var sign_in_result = await PlayFab.sign_in_async(GDK.users.get_primary_user())
if not sign_in_result.ok:
    push_warning(sign_in_result.message)
    return

var playfab_user = sign_in_result.data
var gamesave_sync = await PlayFab.game_saves.add_user_with_ui_async(playfab_user)
if not gamesave_sync.ok:
    push_warning(gamesave_sync.message)
    return

var save_folder_result = PlayFab.game_saves.get_folder(playfab_user)
if not save_folder_result.ok:
    push_warning(save_folder_result.message)
    return

print("Game Saves folder: %s" % save_folder_result.data)
await PlayFab.leaderboards.submit_score_async(playfab_user, "pong_score", 42)
```

## Reference

- [`../spec/gdext-playfab.md`](../spec/gdext-playfab.md) — design spec
