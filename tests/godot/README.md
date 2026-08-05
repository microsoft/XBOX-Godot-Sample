# Godot Test Hosts

This directory contains the Godot test hosts (one per addon) that the repo-root orchestrator (`tools\run_all_tests.ps1`) drives:

- `tests\godot\gdk\` — GUT suites for `godot_gdk`
- `tests\godot\playfab\` — GUT suites for `godot_playfab`
- `tests\godot\gameinput\` — GUT suites for `godot_gameinput`

Each host has its addon mirrored in by CMake when you run `cmake --build build --preset debug`. The shared test bases live at `addons\godot_gdk\tests_support\bases\` and are mirrored into each host as `addons\godot_gdk_tests\`.

GUT is mirrored into each host in two flavors: `addons\gut\` (v9.6.0, the default, for Godot 4.6+) and `addons\gut-4.5\` (bitwes/Gut `b366b70` = v9.5.0 + the #778 push_warning fix, for Godot 4.5.x — GUT 9.6.0 hard-requires Godot 4.6+). The orchestrator picks the right one for the Godot under test via `Select-GutForGodotVersion`, so running against Godot 4.5.1 (`GODOT_CONSOLE=<4.5.1 console exe>`) or 4.6.x both "just work".

The repo-root orchestrator owns both GUT hosts and the PlayFab Multiplayer multi-process orchestrator. Use `tools\run_all_tests.ps1 -PlayFabTitleId <sandbox> -PlayFabCustomId <custom-id> -PlayFabMatchmakingQueue <queue>` for the full sweep.

## Test Tiers

Every GUT test belongs to exactly one tier. The tier describes what external state a test may touch. **All three tiers always run** — there is no opt-in flag and no way to skip the live or destructive tiers.

> **The live and live-write tiers are mandatory.** Every invocation of
> `tools\run_all_tests.ps1` sets `LIVE_TESTS=1` and `LIVE_WRITE_TESTS=1`, talks
> to live services, and **mutates the configured PlayFab title**. Always point
> it at a dedicated sandbox title — never a shared or production title id.

### `contract` (default)

- Offline.
- May not touch any live PlayFab title or live Xbox service.
- Verifies bindings, default values, soft-fail paths, async signal contracts, doc/string round-trips, and other behavior that does not require a network round trip.

No declaration needed — tests default to this tier. Most tests should be `contract`.

### `live_read`

- Reads live state (signed-in user profile, leaderboard entries, lobby queries, …) but does not write state that persists in the title.
- Declares the tier by calling `requires_live()` at the top of `before_all` (or `before_each` for per-test gating).
- `requires_live()` **fails** the test when `LIVE_TESTS` is unset. It no longer marks the test pending: a missing flag means the suite was launched outside the supported entry point, and silently skipping would report green for coverage that never ran.

```gdscript
func before_all() -> void:
    if not requires_live():
        return
    # … set up live signed-in user, fetch read-only state, etc.
```

### `live_write`

- Writes state that persists in the live PlayFab title (create lobby, post leaderboard entry, save Game Save, …).
- **Must run against a dedicated sandbox PlayFab title.** The orchestrator prints the active title id on every run so it cannot be missed in a log.
- Declares the tier by calling `requires_live_write()`, which likewise **fails** rather than pends when `LIVE_WRITE_TESTS` is unset.

```gdscript
func before_each() -> void:
    if not requires_live_write():
        return
    # … create lobby, mutate, tear down …
```

## Required Configuration

`tools\run_all_tests.ps1` runs a **preflight check before any stage executes** and aborts with a precise list of what is missing. A run that cannot reach live services fails; it never reports green.

| Setting | Parameter | Environment variable | How to obtain |
| --- | --- | --- | --- |
| Sandbox title id | `-PlayFabTitleId` | `PLAYFAB_TITLE_ID` | A dedicated sandbox title. **Never** a shared or production id — the live-write tier mutates it. |
| Custom id | `-PlayFabCustomId` | `PLAYFAB_CUSTOM_ID` | A pre-existing account; live sign-in uses `create_account=false`. Provision with `tools\configure_playfab_test_title.ps1`. |
| Matchmaking queue | `-PlayFabMatchmakingQueue` | `PLAYFAB_MULTIPLAYER_MATCH_QUEUE` | A configured queue on the sandbox title. Provision with `tools\configure_playfab_test_title.ps1`. |

Parameters take precedence over environment variables. `PLAYFAB_DEVELOPER_SECRET_KEY` is scrubbed from every child process and is only used by `tools\configure_playfab_test_title.ps1`.

Typical invocation:

```powershell
pwsh -NoLogo -NoProfile -ExecutionPolicy Bypass -File .\tools\run_all_tests.ps1 `
    -PlayFabTitleId <sandbox-title> `
    -PlayFabCustomId <custom-id> `
    -PlayFabMatchmakingQueue <queue>
```

### Runtime preflight (stage `[4/8]`)

Configuration values only prove that *settings* are present. Stage 4 — the **live environment probe** — proves the *machine* can actually reach the services those settings describe, before a single GUT test runs.

For each host in the run, the orchestrator stages `tools\ci\live_env_probe.gd` into the host project, runs it headless, and removes it again. The probe asserts a per-host capability set:

| Host | Required capabilities |
| --- | --- |
| `tests\godot\gdk` | `gdk`, `xuser` |
| `tests\godot\playfab` | `gdk`, `xuser`, `playfab` |
| `tests\godot\gameinput` | `gameinput`, `gamepad` |

- `gdk` — the GDK runtime initializes.
- `xuser` — a signed-in Xbox user is resolvable (silent sign-in, 30 s budget).
- `playfab` — the PlayFab runtime initializes against the configured title id.
- `gameinput` — the GameInput runtime initializes.
- `gamepad` — at least one gamepad is connected.

A failed probe aborts the run and prints the specific missing capability, e.g.:

```
[live-probe] OK   gdk: runtime initialized.
[live-probe]   * xuser: no signed-in Xbox user is available on this machine. …
```

This is the difference between "your config is wrong" (caught by the configuration preflight, stage 0) and "your machine cannot run this tier" (caught here). Both abort; neither degrades to a green tick.

### Pending semantics

`pending()` used to mean two unrelated things: *"this tier was not selected"* and *"this precondition is unavailable"*. The first meaning is gone — tiers are always selected — so `pending()` is now strict:

- **`pending(reason)`** — when `LIVE_TESTS` is set (i.e. every orchestrator run), the shared bases override `pending()` to **fail** the test. A live-tier run that degrades to pending is degraded coverage reported as green, which is exactly the failure mode this model exists to prevent.
- **`pending_tolerated(reason)`** — always behaves like stock GUT `pending()`. Reserved for genuinely separate opt-in axes that are *not* part of the live-tier contract: `GDK_CAPTURE_*` tests, `GDK_TEST_DLC_*` package tests, the `GameDKCoreLatest` version probe, and documented service variance.

Running GUT directly from a host root (`godot -s res://addons/gut/gut_cmdln.gd …`) does not set `LIVE_TESTS`, so `pending()` keeps stock behavior there. Only orchestrator runs are strict — iterating on a single suite stays cheap.

## Authoring a New Test

1. Pick the tier honestly. Default to `contract`; promote to `live_read` only if the test cannot be meaningfully asserted offline; promote to `live_write` only if persistent state mutation is the point.
2. Place the test under `tests\godot\<addon>\tests\` as `test_<scenario>.gd`.
3. `extends` the matching base — `gdk_test_base.gd` / `playfab_test_base.gd` / `gameinput_test_base.gd`.
4. For `live_read` and `live_write` tests, call `requires_live()` / `requires_live_write()` at the top of `before_all` (or `before_each`) and return when it reports false. The `pending_unless_live()` / `pending_unless_live_write()` aliases remain for legacy callers but now fail rather than pend.
5. **Do not reach for `pending()` to paper over a missing precondition.** Under the orchestrator it fails anyway. If the precondition belongs to the live tier, let it fail — that is the signal. If it belongs to a separate opt-in axis (a `GDK_CAPTURE_*`-style env var, an installed-SDK check), use `pending_tolerated(reason)` and say in the reason exactly which knob turns the test on.
6. Run the orchestrator once against your sandbox title to confirm the new test actually executes.

## PlayFab Multiplayer Orchestrator

`tests\godot\mp_orchestrator\` plus `tests\godot\mp_test_client\` is the canonical Multiplayer/Party entry point. The retired `tools\run_playfab_multiplayer_live.ps1` and `tests\godot\playfab_multiplayer_worker\` harness are no longer used.

- Repo-wide sweep: `tools\run_all_tests.ps1 -PlayFabTitleId <sandbox> -PlayFabCustomId <custom-id> -PlayFabMatchmakingQueue <queue>`
- Direct scenario runs: `tools\run_mp_orchestrator.ps1 -Roles host,guest,guest2,observer -Filter "^party\.network\."`

The repo-wide stage dynamically selects C1 P0/P1 scenario files from `tests\godot\mp_orchestrator\scenarios\`. P2/P3 scenarios stay out of that default sweep until promoted. Scenario helpers that previously returned `skip()` for missing live configuration now return `fail()`.

## Why This Matters

PR review repeatedly caught "this test would have failed silently against a shared title" and "this test only passed because LIVE_TESTS was unset". The second class of problem is now structurally impossible: the tiers cannot be disabled, and missing configuration aborts the run instead of quietly degrading it to a green tick.
