# PlayFab — title prerequisites

This page is the addon-agnostic reference for the PlayFab title-side
configuration required before any PlayFab tutorial or sample in this
repository runs.

It complements
[Xbox sandbox and test accounts](../platform/xbox-sandbox-and-test-accounts.md),
which covers the PC- and test-account-side configuration.

Each PlayFab tutorial under [`docs/tutorials/`](../tutorials/README.md)
links back to the section of this page that documents its title-side
prerequisites.

---

## 1. Create the title and capture the Title ID

If a PlayFab title is not already provisioned:

1. Register at the
   [PlayFab developer portal](https://developer.playfab.com/). Either
   a Microsoft account or a PlayFab-specific account is accepted.
2. Follow
   [PlayFab — Game Manager quickstart](https://learn.microsoft.com/en-us/gaming/playfab/gamemanager/quickstart)
   to create a studio and an initial title. See
   [Microsoft Learn — PlayFab](https://learn.microsoft.com/en-us/gaming/playfab/)
   and [PlayFab — Get started](https://learn.microsoft.com/en-us/gaming/playfab/get-started/)
   for broader product documentation.
3. After the title is created, the **Title ID** is shown in Game
   Manager under **your title → Settings → API features** (a short
   alphanumeric string such as `A1B2C`).

Set the Title ID in **Project — Project Settings → General** with
**Advanced Settings** enabled in the top-right:

| Setting | Value |
|---|---|
| `playfab/runtime/title_id` | the PlayFab Title ID |
| `playfab/runtime/initialize_on_startup` | `true` (recommended); `PlayFabBootstrap` will call `PlayFab.initialize()` automatically |

The equivalent `project.godot` entries:

```ini
[playfab]

runtime/title_id="A1B2C"
runtime/initialize_on_startup=true
```

`playfab/runtime/endpoint` may be left blank. When blank, the addon
derives `https://<titleid>.playfabapi.com` from the Title ID.

`PlayFab.initialize()` / `PlayFab.shutdown()` may be cycled in tools and
runtime flows. The PlayFab Core/Services/Game Save state and task queue are
recreated on each initialize, while the GDK `XGameRuntimeInitialize` reference
is held for the process lifetime and released once when the extension unloads.

> **`PlayFab.initialize()` failing with `title_id_required`** indicates
> the setting is empty at runtime. See
> [Troubleshooting → `PlayFab.initialize()` fails with `title_id_required`](../troubleshooting.md#playfabinitialize-fails-with-title_id_required).

---

## 2. Per-tutorial title-side fixtures

Each section below lists the Game Manager configuration (or, where
the resource is Xbox-side, the `MicrosoftGame.config` configuration)
required by the matching tutorial. Sections that do not apply may be
skipped.

### Leaderboards (T3, T8)

[Tutorial 3 — Post and query a PlayFab leaderboard](../tutorials/03-playfab-leaderboard.md)
uses the **statistic-backed** leaderboard pattern: the client writes
values to a statistic, and a leaderboard sourced from that statistic
ranks the values. This is the recommended client-safe path: enable
**Allow client to post player stats**, call
`PlayFab.statistics.update_statistics_async()`, and read the linked
leaderboard through `PlayFab.leaderboards`.

> **Do not put a PlayFab developer secret key in a Godot client.**
> PlayFab's direct LeaderboardsV2 write endpoint
> (`LeaderboardsV2/UpdateLeaderboardEntries`, exposed by
> `PlayFab.leaderboards.submit_score_async`) is the non-statistic-backed
> direct write path. Treat it as server/trusted-backend work that holds
> the developer secret key outside the client. A direct client call returns
> HRESULT `0x89235472` (`E_PF_API_NOT_ENABLED_FOR_GAME_CLIENT_ACCESS`,
> errorCode `1082`) on titles where that server path is not enabled for
> client access.

Two title-side resources are required, plus a title-wide setting that
enables client writes to statistics:

1. **A statistic with a known name.** Create the statistic in Game
   Manager → **Statistics → New statistic**. The T3 snippets
   reference the name `"high_score"`; substitute an alternative name
   and update the matching `STATISTIC_NAME` constant in the tutorial
   code.
   - Entity type: `title_player_account`.
   - Columns: a single column is the simplest match for the T3
     snippets. `AggregationMethod = Last` records every write
     verbatim; `Max` retains the player's best value across writes.
   - `update_statistics_async` serializes scores as decimal strings,
     so a single-column statistic accepts
     `{"statistics": [{"name": "high_score", "scores": ["1234"]}]}`.

2. **A leaderboard sourced from the statistic.** Create the
   leaderboard in Game Manager → **LeaderboardsV2 → New leaderboard**
   and configure its source to be the statistic created in step 1.
   The leaderboard name and statistic name may differ; the T3
   snippets use the same name (`"high_score"`) for both so the code
   carries only one constant. The leaderboard's column count must
   match the statistic's column count.

3. **Enable client-side statistic writes.** Game clients are blocked
   from writing to statistics until the title-wide
   **Allow client to post player stats** setting is enabled. This
   setting gates both the legacy `Client/UpdatePlayerStatistics`
   endpoint and the V2 `Statistic/UpdateStatistics` endpoint that the
   addon's `update_statistics_async` calls.
   - In Game Manager, select the title and open
     **Title settings → API Features**.
   - Locate the **Allow client to post player stats** row and enable
     the toggle.
   - **Save**. The change is title-wide; it does not need to be
     repeated per statistic.
   - The configure script (see §3) probes this setting after
     provisioning the statistic and prints an actionable warning
     when the setting is still disabled.

After all three steps are complete, the T3 snippets work end-to-end:

- Writes go through `PlayFab.statistics.update_statistics_async(user, {...})`.
- Reads go through `PlayFab.leaderboards.get_leaderboard_async`,
  `get_leaderboard_around_user_async`, and
  `get_friend_leaderboard_async` against the leaderboard name.
- The direct-write entry point
  `PlayFab.leaderboards.submit_score_async` is intentionally **not**
  used by the T3 snippets. For non-statistic-backed leaderboard writes,
  run a trusted backend or CloudScript-style flow that holds the developer
  secret key outside the client. Direct client calls commonly return the
  verbatim service error:

  ```json
  {
    "error": "APINotEnabledForGameClientAccess",
    "errorCode": 1082,
    "errorMessage":
      "This API must be enabled for client access in the Game Manager API Features settings"
  }
  ```

  The statistic-backed pattern above is the supported client write
  path. The same `1082` error returned by
  `update_statistics_async` indicates the **Allow client to post
  player stats** setting is still disabled — see step 3 above. Full
  diagnostic in
  [Troubleshooting → PlayFab leaderboard submit returns 0x89235472](../troubleshooting.md#playfab-leaderboard-submit-fails-with-e_pf_api_not_enabled_for_game_client_access-0x89235472).

For production titles that require validated writes (anti-cheat,
server-authoritative scoring), keep client writes off the direct
leaderboard endpoint and route the writes through CloudScript, Azure
Functions, or a trusted backend that holds the developer secret key. The
statistic-backed pattern remains appropriate for any value the client
is trusted to compute.

### Game Saves (T4, T8)

[Tutorial 4 — Save the player's progress](../tutorials/04-game-saves.md)
requires:

- **`CloudSaves` block in `MicrosoftGame.config`.** The
  **GDK → Create MicrosoftGame.config** menu (provided by the
  `godot_gdk_packaging` addon) writes a template that includes the
  CloudSaves block. Configurations created before the template was
  added must be updated in `GameConfigEditor.exe` to include a
  `<CloudSaves>` section.
- **Xbox-backed PlayFab session.** Every `PlayFab.game_saves` call
  rejects a custom-ID session with `xbox_user_required`. Game Saves
  is the Xbox-attached blob store and has no PlayFab-only path.
  Enablement is controlled entirely by `MicrosoftGame.config`; no
  PlayFab Game Manager setting is involved.
- **Network connectivity** for cloud sync round-trips. Game Saves
  also operates offline with reduced semantics; each method's result
  exposes a `cloud connected` indicator.

> **Game Saves vs. PlayFab title data / entity data.** Game Saves is
> the Xbox-attached blob store that follows the Xbox account across
> devices and is surfaced in the system Cloud Saves UI. Use it for
> player-progress blobs. Use `PlayFab.entity_data` or
> `PlayFab.player_data` for structured per-player JSON that does not
> require Xbox-backed sync semantics.

### Lobby (T5, T6, T7, T8)

[Tutorial 5 — Create and join a lobby](../tutorials/05-multiplayer-lobby.md)
requires:

- **PlayFab Multiplayer → Lobby feature enabled.** Open Game
  Manager → **Multiplayer → Lobby** and confirm the title reports
  "Lobby enabled." Recently created titles enable this by default;
  older titles require the feature to be enabled manually.
- **Two Godot processes** (host and client) signed into different
  Xbox test accounts in the same sandbox. A typical setup runs the
  host scene in the editor and the client as an exported build. Two
  editors with separate PlayFab sessions are also supported.
- **Xbox-backed PlayFab session on both sides** when Xbox-shell
  invites (Game Bar, friends list) are required. Custom-ID lobbies
  are restricted to joining by connection string.

### Multiplayer Activity (T6, T8)

[Tutorial 6 — Advertise your lobby with MPA](../tutorials/06-multiplayer-activity.md)
is the Xbox-side advertisement layer that sits on top of a PlayFab
lobby. There is no PlayFab Game Manager fixture for MPA; the
prerequisites are the Xbox-side SCID, an Xbox-backed PlayFab session,
and the in-sandbox test accounts documented in
[Xbox sandbox and test accounts](../platform/xbox-sandbox-and-test-accounts.md).
The only PlayFab-side prerequisite is that the T5 Lobby fixture is in
place.

### Party (T7, T8)

[Tutorial 7 — Stand up a PlayFab Party network](../tutorials/07-playfab-party.md)
requires:

- **PlayFab Multiplayer → Party feature enabled.** Open Game
  Manager → **Multiplayer → Party** and confirm the title reports
  "Party enabled." Recently created titles enable this by default;
  older titles require the feature to be enabled manually.
- **A functional microphone on both sides** for voice-path testing.
  Text and RPC traffic do not require a microphone.
- **The two-process / two-test-account setup described in §2 Lobby.**
  Party's Xbox-shell invites require Xbox-backed sessions on both
  sides.

#### Same-host Party UDP port override

The addon registers `playfab/party/local_udp_socket_bind_port` for `PlayFab.party` initialization. Default `-1` keeps the Party SDK default bind address; valid values are `-1..65535`. Use `0` in same-host development or CI when multiple local Godot processes may otherwise collide on the Party UDP bind port. Values `1..65535` pin a specific local UDP port. `PlayFab.party.initialize_async(..., local_udp_port)` can override the Project Setting for a single initialization.

### Capstone (T8)

[Tutorial 8 — Integration tech demo](../tutorials/08-integration-tech-demo.md)
composes T1 through T7. Its title-side prerequisites are the union of
the sections above: a `"high_score"` statistic and linked leaderboard
plus the title-wide **Allow client to post player stats** setting
enabled (step 3 of §2 Leaderboards), the `CloudSaves` block in
`MicrosoftGame.config`, and both the **Lobby** and **Party** features
enabled.

---

## 3. Optional — provision live-test fixtures with the configure script

`tools\configure_playfab_test_title.ps1` provisions the resources
used by the PlayFab live tests and the cumulative samples. The script
is idempotent and creates:

- the leaderboard `wave4_settle_smoke` (single integer column, size
  limit 1000) used by the live-smoke matchmaking and leaderboard tests
- the **statistic `high_score` and a linked leaderboard `high_score`**
  used by the Tutorial 3 / Tutorial 8 reference sample. The
  leaderboard's column is sourced from the statistic's `value`
  column, so client writes via
  `PlayFab.statistics.update_statistics_async` surface in
  `PlayFab.leaderboards.get_leaderboard_async` reads. Pass
  `-SkipTutorialFixtures` to opt out, or override the names with
  `-TutorialStatisticName` / `-TutorialLeaderboardName`. After
  provisioning the pair the script probes the title's
  **Allow client to post player stats** setting (see §2 Leaderboards
  step 3) by attempting a player-side write; it prints `OK: client-side
  write to statistic '<name>' is enabled` on success and an actionable
  `WARN` pointing at the Game Manager toggle when the setting is still
  disabled.
- a custom-ID account `godot-gdk-ext-live-smoke` and the
  Multiplayer worker accounts used by both live runners:
  - **Legacy runner** (`tools\run_playfab_multiplayer_live.ps1`) —
    four unsuffixed accounts: `...-host`, `...-client`,
    `...-client2`, `...-observer`.
  - **mp_orchestrator harness** (`tests/godot/mp_orchestrator/`) —
    sixteen pooled accounts named `...-{host,client,client2,observer}-{1..4}`.
    The harness rotates through the pool between scenarios to
    spread the per-(title_player_account) PlayFab rate limits
    (e.g. `createlobby` is capped at 6 calls per account per 120
    seconds; four pooled accounts give 24 creates per window of
    headroom per role). The pool size is captured in the title-
    data marker under `multiplayer_custom_id_pool_size`.
- a Multiplayer matchmaking queue `godot_gdk_ext_live_smoke_queue`
- API-service fixtures (accounts, friends, player data, title data,
  publisher data, a statistic, a catalog draft item) used by the
  PlayFab service-wrapper tests
- a title-data marker that identifies the configured sandbox title

### Invocation

The script reads the PlayFab developer secret key from the
`PLAYFAB_DEVELOPER_SECRET_KEY` environment variable (process, user,
or machine scope; the first scope set is used). The secret is not
printed, written to disk, or forwarded to the Godot test runner.
[`tools\run_all_tests.ps1`](../gdk/sample-and-tests.md) scrubs
`PLAYFAB_DEVELOPER_SECRET_KEY` from child process environments before
launching Godot or pwsh workers.

```powershell
# One-time: store the secret in the user environment.
[Environment]::SetEnvironmentVariable(
    'PLAYFAB_DEVELOPER_SECRET_KEY',
    '<developer secret key from Game Manager → Settings → Secret Keys>',
    'User')

# Configure a sandbox title (substitute the target title id).
pwsh -NoLogo -NoProfile -ExecutionPolicy Bypass `
    -File .\tools\configure_playfab_test_title.ps1 `
    -TitleId 10D176
```

A successful run prints `OK:` lines for each resource (accounts,
queue, leaderboard, fixtures, marker). Re-running the script is
safe; existing resources are detected and preserved.

> **Sandbox titles only.** Direct the script at a PlayFab title
> reserved for development or testing. The script writes player data,
> leaderboard entries, and matchmaking queue configuration; it must
> not be run against a production title.

#### Obtaining a developer secret key

Open Game Manager → the title → **Settings → Secret Keys**. Each key
has a friendly name (for example `default`) and a hidden value; copy
the value into the environment variable above. Developer secret keys
must be treated as production credentials.

---

## 4. Verification checklist

| Check | Verification |
|---|---|
| Title ID set in Project Settings | `Get-Content .\project.godot \| Select-String "title_id"` reports the Title ID under `[playfab]` |
| `PlayFab.initialize()` succeeds | Bootstrap log line `[PlayFab] Bootstrap: PlayFab.initialize() succeeded.` is emitted at editor or runtime startup |
| Xbox-backed sign-in succeeds (T1) | T1 log line `[PlayFab] signed in: title_player_account:<entity-id>` is emitted |
| Leaderboard accepts client writes (T3) | T3 statistic write emits `[Lead] Recorded score …`. HRESULT `0x89235472` on `submit_score_async` indicates the code is calling the direct leaderboard write path, which should be handled by a trusted backend with a developer secret key for non-statistic-backed leaderboards. Use `update_statistics_async` for client-safe statistic-backed leaderboards. The same `0x89235472` on `update_statistics_async` indicates the title's **Allow client to post player stats** setting is disabled (see §2 step 3) |
| Game Saves accepts the local user (T4) | T4 add-user emits `[Save] User context registered`. `xbox_user_required` indicates a custom-ID rather than Xbox-backed session |
| Lobby create succeeds (T5) | T5 host emits `[Lobby] hosting <lobbyId>` |
| Party create succeeds (T7) | T7 host emits `[Party] network <id> hosted with descriptor <…>` |

If a check fails, the corresponding tutorial's **Common failures**
table and the [Troubleshooting](../troubleshooting.md) page document
the diagnostic next steps.

---

## Related docs

- [Addons getting started](../addon-getting-started.md) — the
  addon-zip quickstart, including the `playfab/runtime/title_id`
  setting step that this page elaborates on.
- [Xbox sandbox and test accounts](../platform/xbox-sandbox-and-test-accounts.md)
  — the Xbox-side companion to this page. PlayFab tutorials assume
  both pages are satisfied.
- [PlayFab plugin overview](plugin.md) — `godot_playfab` runtime
  configuration, public surface, and architecture notes.
- [PlayFab async system](async-system.md) — PlayFab completion,
  dispatch, and shutdown lifecycle contract.
- [Tutorials index](../tutorials/README.md) — the cumulative T1
  through T8 tutorial chain and the standalone GameInput track.
- [Async patterns](../async-patterns.md) — one-page primer on the
  `_async` naming convention, `await`-on-coroutine, and
  `PlayFabResult`.
- [Troubleshooting](../troubleshooting.md) — error-code references
  for common failure modes.
