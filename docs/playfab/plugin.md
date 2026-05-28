# Godot PlayFab plugin

This is the landing page for the `godot_playfab` docs set.

`godot_playfab` is the PlayFab-focused GDExtension addon in this repository. It implements a single `PlayFab` root singleton, manual user sign-in keyed by a `GDKUser` object or title-defined custom id, PlayFab Game Saves wrappers, PlayFab leaderboard submission/query flows, client-safe PlayFab Services SDK wrappers, and an MLP PlayFab Multiplayer lobby/matchmaking surface.

## Current implementation status

### Implemented now

- root singleton registration through `PlayFab`
- shared PlayFab runtime lifecycle through `XGameRuntimeInitialize`, `PFInitialize`, and `PFServicesInitialize`
- shared Game Saves runtime lifecycle through `PFGameSaveFilesInitialize` and `PFGameSaveFilesUninitializeAsync`
- default auto-dispatch through `playfab/runtime/embed_dispatch`
- project-settings-backed PlayFab config through `playfab/runtime/title_id` and `playfab/runtime/endpoint`
- manual Xbox-backed PlayFab sign-in through `PlayFab.users.sign_in_with_xuser_async(...)`
- custom-ID PlayFab sign-in through `PlayFab.users.sign_in_with_custom_id_async(...)`
- cached `PlayFabUser` wrappers keyed by local Xbox user id or custom id
- Game Saves add/sync, upload, folder/quota queries, cloud connectivity queries, save description updates, and cloud reset through `PlayFab.game_saves`
- leaderboard submit, global query, around-user query, and friends/social leaderboard query
- client-safe PlayFab service wrappers under `PlayFab.accounts`, `PlayFab.catalog`, `PlayFab.cloud_script`, `PlayFab.entity_data`, `PlayFab.experimentation`, `PlayFab.friends`, `PlayFab.groups`, `PlayFab.inventory`, `PlayFab.localization`, `PlayFab.player_data`, `PlayFab.statistics`, and `PlayFab.title_data`
- `PlayFab.events` as a reserved service namespace; the current GDK PlayFab headers do not expose an active client event/telemetry operation in the client wrapper scope
- PlayFab Multiplayer initialization, lobby create/join/search, lobby-owned leave and property updates, match-ticket-owned cancel/status refresh, and explicit arranged-lobby joins
- PlayFab Party network host (`create_and_join_network_async`) and join (`join_network_async`) flows over the PartyManager runtime, with peer-id handshake, descriptor publishing, chat controls (voice/text/transcription), mute, and permission management; the per-network peer object is a Godot `MultiplayerPeerExtension`
- GUT coverage under `tests\godot\playfab\tests\`
- idempotent extension registration so duplicated synced `.gdextension` files do not spam duplicate singleton/class registration

### Not implemented yet

- custom non-Windows Game Saves UI callback/response wrappers are not yet exposed as a public Godot surface
- server/admin/title-secret PlayFab APIs are intentionally excluded from the client wrapper set

## Runtime configuration

> **Where do I get a PlayFab title id?** Sign up at the
> [PlayFab developer portal](https://developer.playfab.com/) and follow
> [PlayFab — Game Manager quickstart](https://learn.microsoft.com/en-us/gaming/playfab/gamemanager/quickstart)
> to create your account, studio, and first title. The Title ID lives in
> Game Manager under **your title → Settings → API features**. For the
> broader product tour see [Microsoft Learn — PlayFab](https://learn.microsoft.com/en-us/gaming/playfab/)
> and the [PlayFab — Get started](https://learn.microsoft.com/en-us/gaming/playfab/get-started/) hub.
>
> For the full per-tutorial walkthrough — sign-in mode selection, the
> Game Manager fixtures required by each tutorial (the "Allow client
> to update entries" leaderboard toggle, the Lobby and Party feature
> switches, the `CloudSaves` block in `MicrosoftGame.config`), and the
> `configure_playfab_test_title.ps1` helper — see
> [PlayFab title prerequisites](prerequisites.md).

The PlayFab runtime reads these settings from Project Settings:

- `playfab/runtime/title_id` — required; the PlayFab title id
- `playfab/runtime/endpoint` — optional; leave blank to derive `https://<titleid>.playfabapi.com`
- `playfab/runtime/initialize_on_startup` — defaults to `false`; when `true`, the `PlayFabBootstrap` autoload calls `PlayFab.initialize()` during `_ready` (parallels `gdk/runtime/initialize_on_startup`). PlayFab sign-in is **not** auto-driven — it requires a per-player key (`GDKUser` or custom id) and stays in title code.
- `playfab/runtime/embed_dispatch` — defaults to `true`; disable only when you want to pump completions manually with `PlayFab.dispatch()`

The `GodotPlayFab` editor plugin installs the `PlayFabBootstrap` autoload at `res://addons/godot_playfab/runtime/playfab_bootstrap.gd` when enabled, mirroring the `GDKBootstrap` pattern. Disabling the plugin removes the autoload again.

## Public GDScript surface

- `PlayFab`
- `PlayFab.users`
- `PlayFab.game_saves`
- `PlayFab.leaderboards`
- `PlayFab.multiplayer`
- `PlayFab.party`
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
- `PlayFabMultiplayer`
- `PlayFabLobby`
- `PlayFabMatchTicket`
- `PlayFabParty`
- `PlayFabPartyNetwork`
- `PlayFabPartyPeer`
- `PlayFabUser`
- `PlayFabResult`

All PlayFab one-shot async methods now return completion signals that you await directly. `PlayFab.users` is intentionally cache/result-driven and does not expose user lifecycle signals.

Service methods use the common shape `service.method_async(playfab_user, request := {})`. The `request` dictionary uses snake_case versions of the PlayFab C SDK request fields, and successful response payloads are converted to Godot dictionaries and arrays. Operations that need an `XUserHandle` accept a signed-in `GDKUser` object in `request.user`; raw local ids are not accepted. The service contract test in `tests\godot\playfab\tests\test_api_services.gd` is the source of truth for the expected Godot-facing method matrix.

## Sample usage

```gdscript
if not PlayFab.is_initialized():
    var init_result = PlayFab.initialize()
    if not init_result.ok:
        push_warning(init_result.message)
        return

var sign_in_result = await PlayFab.users.sign_in_with_custom_id_async("my-title-defined-id", false)
if not sign_in_result.ok:
    push_warning(sign_in_result.message)
    return

var playfab_user = sign_in_result.data
await PlayFab.statistics.update_statistics_async(playfab_user, {
    "statistics": [
        {"name": "pong_score", "scores": ["42"]},
    ],
})

var title_data_result = await PlayFab.title_data.get_title_data_async(playfab_user, {
    "keys": ["welcome_message"],
})
if title_data_result.ok:
    print(title_data_result.data)
```

Game Saves still requires an Xbox-backed PlayFab session because the PlayFab Game Saves C API needs a local user handle. Use `PlayFab.users.sign_in_with_xuser_async(GDK.users.get_primary_user())` before calling `PlayFab.game_saves`; custom-ID users return `xbox_user_required` from Game Saves methods.

Lobby and matchmaking calls use the signed-in user's native PlayFab entity handle. Match tickets do not auto-join arranged lobbies; title code decides whether to pass the reported connection string to `join_arranged_lobby_async`.

```gdscript
var mp_result = await PlayFab.multiplayer.initialize_async()
if not mp_result.ok:
    push_warning(mp_result.message)
    return

var lobby_config := PlayFabLobbyConfig.new()
lobby_config.access_policy = PlayFabLobbyConfig.ACCESS_POLICY_PUBLIC
lobby_config.search_properties = {"string_key1": "duos"}

var lobby_result = await PlayFab.multiplayer.create_lobby_async(playfab_user, lobby_config)
if lobby_result.ok:
    var lobby: PlayFabLobby = lobby_result.data
    print("Join with: ", lobby.get_connection_string())
```

Use `tools\configure_playfab_test_title.ps1` with a PlayFab developer secret in `PLAYFAB_DEVELOPER_SECRET_KEY` to provision a sandbox title for live coverage. The script creates the custom-ID smoke account, Multiplayer worker accounts, a two-player matchmaking queue keyed by a `run_id` equality rule, leaderboard/statistic definitions, service fixture accounts, title/publisher/player data keys, a catalog draft item, and a title-data marker describing those resources.

## Testing this addon

`godot_playfab` is exercised by the `tests\godot\playfab\` host. The host covers the root singleton, class registration, runtime initialization, PlayFab user wrappers, Game Saves services, leaderboard services, API service contracts, Multiplayer service contracts, Party public-surface contract, validation/error paths, and live custom-ID/Game Saves/leaderboard flows through files such as `tests\godot\playfab\tests\test_game_saves_live.gd`, `tests\godot\playfab\tests\test_leaderboards_live.gd`, `tests\godot\playfab\tests\test_api_services.gd`, `tests\godot\playfab\tests\test_multiplayer_contract.gd`, `tests\godot\playfab\tests\test_party.gd`, and `tests\godot\playfab\tests\test_validation_walk.gd`.

Default runs keep live prerequisites pending when a developer machine is not configured for PlayFab sign-in. The sandbox title currently used for repo live validation is `10D176`. Before the first live run against a title, configure the title with a developer secret key stored in an environment variable:

```powershell
$env:PLAYFAB_DEVELOPER_SECRET_KEY = "<developer-secret-key>"
pwsh -NoLogo -NoProfile -ExecutionPolicy Bypass -File .\tools\configure_playfab_test_title.ps1
```

The setup script defaults to title `10D176`, reads `PLAYFAB_DEVELOPER_SECRET_KEY` from the process, user, or machine environment without printing it, creates the `godot-gdk-ext-live-smoke` custom-ID account used by `create_account=false` live tests, creates the `godot-gdk-ext-live-smoke-multiplayer-host/client/observer` worker accounts used by multi-client Lobby tests, creates or validates the `godot_gdk_ext_live_smoke_queue` matchmaking queue with a `run_id` equality rule, creates or validates the `wave4_settle_smoke` leaderboard definition, prepares API-service fixture accounts, title/publisher/player data keys, statistics, and catalog draft items, and writes a title-data marker. Lobby search keys do not require title setup; the live Multiplayer runner uses PlayFab's reserved `string_key1`/`number_key1` search properties.

Live tests run with `-Live` and require a PlayFab title id plus a pre-existing custom id. You can provide them as runner parameters:

```powershell
pwsh -NoLogo -NoProfile -ExecutionPolicy Bypass -File .\tools\run_all_tests.ps1 -Hosts tests\godot\playfab -Live -PlayFabTitleId "10D176" -PlayFabCustomId "godot-gdk-ext-live-smoke" -PlayFabMatchmakingQueue "godot_gdk_ext_live_smoke_queue"
```

The runner forwards those values only to child processes as `PLAYFAB_TITLE_ID`, `PLAYFAB_CUSTOM_ID`, and `PLAYFAB_MULTIPLAYER_MATCH_QUEUE`; the PlayFab test base applies the title id to `playfab/runtime/title_id` and uses the custom id for `create_account=false` sign-in. The Multiplayer runner derives worker accounts from `PLAYFAB_CUSTOM_ID` as `<custom-id>-multiplayer-host/client/observer` unless `PLAYFAB_MULTIPLAYER_CUSTOM_ID_PREFIX` overrides the prefix. The developer secret key is only consumed by `tools\configure_playfab_test_title.ps1`, not by Godot test processes. Project settings (`playfab/runtime/title_id`, `playfab/tests/custom_id`) and the `PLAYFAB_CUSTOM_ID` environment variable remain supported for manual runs. Some `-Live` tests write online state, such as leaderboard submissions and the PlayFab Multiplayer multi-client lobby smoke, so run live PlayFab coverage only against a personal sandbox title. Leaderboard read-after-write checks poll up to `playfab/tests/leaderboard_settle_msec` and mark pending, not failed, when the service is eventually consistent beyond that budget. The live Multiplayer orchestration uses three worker processes and covers lobby creation, search isolation, private lobby discovery behavior, invalid joins, three-member snapshots, member/lobby property propagation, leave/rejoin behavior, owner migration, and cleanup; when `-PlayFabMatchmakingQueue` or `PLAYFAB_MULTIPLAYER_MATCH_QUEUE` is set it also covers match ticket create/cancel, two-player match completion, explicit arranged-lobby joins, and arranged-lobby cleanup.

The PlayFab host uses custom-ID sign-in for default coverage. By default, CMake also mirrors `godot_gdk` into `tests\godot\playfab` so optional Xbox-backed compatibility tests can call `ensure_gdk_primary_user_for_playfab()`. Configure with `-DGODOT_PLAYFAB_TEST_HOST_WITH_GDK=OFF` to omit that mirror; GDK-backed helpers skip cleanly when the addon is not present.

Run the standard pipeline from the repository root:

```powershell
pwsh -NoLogo -NoProfile -ExecutionPolicy Bypass -File .\tools\run_all_tests.ps1
```

See [`gdk/sample-and-tests.md`](../gdk/sample-and-tests.md) for the orchestrator stages, env vars, live-test safety model, baselines, cleanup pointer, and troubleshooting links.

## Reference

- [`gdk/sample-and-tests.md`](../gdk/sample-and-tests.md) — repo-wide test pipeline
- [`troubleshooting.md#tests`](../troubleshooting.md#tests) — common test issues
