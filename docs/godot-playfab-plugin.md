# Godot PlayFab plugin

This is the landing page for the `godot_playfab` docs set.

`godot_playfab` is the PlayFab-focused GDExtension addon in this repository. It implements a single `PlayFab` root singleton, manual user sign-in keyed by a `GDKUser` object or title-defined custom id, PlayFab Game Saves wrappers, PlayFab leaderboard submission/query flows, and generated client-safe PlayFab Services SDK wrappers.

## Current implementation status

### Implemented now

- root singleton registration through `PlayFab`
- shared PlayFab runtime lifecycle through `XGameRuntimeInitialize`, `PFInitialize`, and `PFServicesInitialize`
- shared Game Saves runtime lifecycle through `PFGameSaveFilesInitialize` and `PFGameSaveFilesUninitializeAsync`
- default auto-dispatch through `playfab/runtime/embed_dispatch`
- project-settings-backed PlayFab config through `playfab/titleid` and `playfab/endpoint`
- manual Xbox-backed PlayFab sign-in through `PlayFab.sign_in_with_xuser_async(...)` / `PlayFab.users.sign_in_with_xuser_async(...)`
- custom-ID PlayFab sign-in through `PlayFab.sign_in_with_custom_id_async(...)` / `PlayFab.users.sign_in_with_custom_id_async(...)`
- cached `PlayFabUser` wrappers keyed by local Xbox user id or custom id
- Game Saves add/sync, upload, folder/quota queries, cloud connectivity queries, save description updates, and cloud reset through `PlayFab.game_saves`
- leaderboard submit, global query, around-user query, and friends/social leaderboard query
- generated client-safe PlayFab service wrappers under `PlayFab.accounts`, `PlayFab.catalog`, `PlayFab.cloud_script`, `PlayFab.entity_data`, `PlayFab.experimentation`, `PlayFab.friends`, `PlayFab.groups`, `PlayFab.inventory`, `PlayFab.localization`, `PlayFab.player_data`, `PlayFab.statistics`, and `PlayFab.title_data`
- `PlayFab.events` as a reserved service namespace; the current GDK PlayFab headers do not expose an active client event/telemetry operation in the generated scope
- sample demos wired to the root singleton, including multiplayer_pong's
  sample-local service wrapper for Game Saves and leaderboard sync
- GUT coverage under `tests\godot\playfab\tests\`
- idempotent extension registration so duplicated synced `.gdextension` files do not spam duplicate singleton/class registration

### Not implemented yet

- broader PlayFab feature areas from the previous codebase such as multiplayer or party services are not part of the new public root API
- custom non-Windows Game Saves UI callback/response wrappers are not yet exposed as a public Godot surface
- server/admin/title-secret PlayFab APIs and Multiplayer/Party APIs are intentionally excluded from the generated client wrapper set

## Runtime configuration

The PlayFab runtime reads these settings from Project Settings:

- `playfab/titleid` — required; the PlayFab title id
- `playfab/endpoint` — optional; leave blank to derive `https://<titleid>.playfabapi.com`
- `playfab/runtime/embed_dispatch` — defaults to `true`; disable only when you want to pump completions manually with `PlayFab.dispatch()`
- `playfab/tests/leaderboard_settle_msec` — int, default `30000`; polling budget for live leaderboard read-after-write checks

## Public GDScript surface

- `PlayFab`
- `PlayFab.users`
- `PlayFab.game_saves`
- `PlayFab.leaderboards`
- `PlayFab.accounts`
- `PlayFab.catalog`
- `PlayFab.cloud_script`
- `PlayFab.entity_data`
- `PlayFab.events`
- `PlayFab.experimentation`
- `PlayFab.friends`
- `PlayFab.groups`
- `PlayFab.inventory`
- `PlayFab.localization`
- `PlayFab.player_data`
- `PlayFab.statistics`
- `PlayFab.title_data`
- `PlayFabGameSaves`
- `PlayFabUser`
- `PlayFabResult`

All PlayFab one-shot async methods now return completion signals that you await directly. `PlayFab.users` is intentionally cache/result-driven and does not expose user lifecycle signals.

Generated service methods use the common shape `service.method_async(playfab_user, request := {})`. The `request` dictionary uses snake_case versions of the PlayFab C SDK request fields, and successful response payloads are converted to Godot dictionaries and arrays. Operations that need an `XUserHandle` accept a signed-in `GDKUser` object in `request.user`; raw local ids are not accepted. The generated service contract test in `tests\godot\playfab\tests\test_generated_services.gd` is the source of truth for the expected Godot-facing method matrix.

## Sample usage

```gdscript
if not PlayFab.is_initialized():
    var init_result = PlayFab.initialize()
    if not init_result.ok:
        push_warning(init_result.message)
        return

var sign_in_result = await PlayFab.sign_in_with_custom_id_async("my-title-defined-id", false)
if not sign_in_result.ok:
    push_warning(sign_in_result.message)
    return

var playfab_user = sign_in_result.data
await PlayFab.leaderboards.submit_score_async(playfab_user, "pong_score", 42)

var title_data_result = await PlayFab.title_data.get_title_data_async(playfab_user, {
    "keys": ["welcome_message"],
})
if title_data_result.ok:
    print(title_data_result.data)
```

Game Saves still requires an Xbox-backed PlayFab session because the PlayFab Game Saves C API needs a local user handle. Use `PlayFab.sign_in_with_xuser_async(GDK.users.get_primary_user())` before calling `PlayFab.game_saves`; custom-ID users return `xbox_user_required` from Game Saves methods.

## Testing this addon

`godot_playfab` is exercised by the `tests\godot\playfab\` host. The host covers the root singleton, class registration, runtime initialization, PlayFab user wrappers, Game Saves services, leaderboard services, validation/error paths, and live custom-ID/Game Saves/leaderboard flows through files such as `tests\godot\playfab\tests\test_game_saves_live.gd`, `tests\godot\playfab\tests\test_leaderboards_live.gd`, and `tests\godot\playfab\tests\test_validation_walk.gd`.

Default runs keep live prerequisites pending when a developer machine is not configured for PlayFab sign-in. Live tests run with `-Live` and require a PlayFab title id plus a pre-existing custom id. You can provide them as runner parameters:

```powershell
pwsh -NoLogo -NoProfile -ExecutionPolicy Bypass -File .\tools\run_all_tests.ps1 -Hosts tests\godot\playfab -Live -PlayFabTitleId "<title-id>" -PlayFabCustomId "<existing-custom-id>"
```

Use `tools\configure_playfab_test_title.ps1` with a PlayFab developer secret in `PLAYFAB_DEVELOPER_SECRET_KEY` to provision a sandbox title for live coverage. The script creates the custom-ID smoke account, Multiplayer worker accounts, leaderboard/statistic definitions, generated-service fixture accounts, title/publisher/player data keys, a catalog draft item, and a title-data marker describing those resources.

The runner forwards those values only to Godot child processes as `PLAYFAB_TITLE_ID` and `PLAYFAB_CUSTOM_ID`; the PlayFab test base applies the title id to `playfab/titleid` and uses the custom id for `create_account=false` sign-in. Project settings (`playfab/titleid`, `playfab/tests/custom_id`) and the `PLAYFAB_CUSTOM_ID` environment variable remain supported for manual runs. Some `-Live` tests write online state, such as leaderboard submissions, so run live PlayFab coverage only against a personal sandbox title. Leaderboard read-after-write checks poll up to `playfab/tests/leaderboard_settle_msec` and mark pending, not failed, when the service is eventually consistent beyond that budget.

The PlayFab host uses custom-ID sign-in for default coverage. By default, CMake also mirrors `godot_gdk` into `tests\godot\playfab` so optional Xbox-backed compatibility tests can call `ensure_gdk_primary_user_for_playfab()`. Configure with `-DGODOT_PLAYFAB_TEST_HOST_WITH_GDK=OFF` to omit that mirror; GDK-backed helpers skip cleanly when the addon is not present.

Run the standard pipeline from the repository root:

```powershell
pwsh -NoLogo -NoProfile -ExecutionPolicy Bypass -File .\tools\run_all_tests.ps1
```

See [`godot-gdk-sample-and-tests.md`](godot-gdk-sample-and-tests.md) for the orchestrator stages, env vars, live-test safety model, baselines, cleanup pointer, and troubleshooting links.

## Reference

- [`godot-gdk-sample-and-tests.md`](godot-gdk-sample-and-tests.md) — repo-wide test pipeline
- [`troubleshooting.md#tests`](troubleshooting.md#tests) — common test issues
- [`../spec/gdext-playfab.md`](../spec/gdext-playfab.md) — design spec
