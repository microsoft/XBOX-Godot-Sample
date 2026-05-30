# Godot Test Hosts

This directory contains the Godot test hosts (one per addon) that the repo-root orchestrator (`tools\run_all_tests.ps1`) drives:

- `tests\godot\gdk\` — GUT suites for `godot_gdk`
- `tests\godot\playfab\` — GUT suites for `godot_playfab`
- `tests\godot\gameinput\` — GUT suites for `godot_gameinput`

Each host has its addon mirrored in by CMake when you run `cmake --build build --preset debug`. The shared test bases live at `addons\godot_gdk\tests_support\bases\` and are mirrored into each host as `addons\godot_gdk_tests\`.

> `requires_live()` / `requires_live_write()` helpers gate live and live-write GUT tests. The repo orchestrator forwards `LIVE_TESTS=1` via `-Live` and `LIVE_WRITE_TESTS=1` via `-AllowLiveWrites`; only use live writes against a dedicated sandbox PlayFab title.

## Test Tiers

Every GUT test belongs to exactly one tier. The tier governs how the test is selected and what external state it may touch.

### `contract` (default)

- Offline.
- Runs on every orchestrator invocation, including bare `tools\run_all_tests.ps1`.
- May not touch any live PlayFab title or live Xbox service.
- Verifies bindings, default values, soft-fail paths, async signal contracts, doc/string round-trips, and other behavior that does not require a network round trip.

No declaration needed — tests default to this tier. Most tests should be `contract`.

### `live_read`

- Requires `LIVE_TESTS=1`. Opt in via `tools\run_all_tests.ps1 -Live`.
- May read live state (signed-in user profile, leaderboard entries, lobby queries, …) but may not write state that persists in the title.
- Declares the tier by calling `requires_live()` at the top of `before_all` (or `before_each` for per-test gating). When `LIVE_TESTS` is not set, the helper marks the test pending and the rest of the test short-circuits.

Example:

```gdscript
func before_all() -> void:
    if not requires_live():
        return
    # … set up live signed-in user, fetch read-only state, etc.
```

### `live_write`

- Requires both `LIVE_TESTS=1` and `LIVE_WRITE_TESTS=1`. Opt in via `tools\run_all_tests.ps1 -Live -AllowLiveWrites`.
- Writes state that persists in the live PlayFab title (create lobby, post leaderboard entry, save Game Save, …).
- **Must run against a dedicated sandbox PlayFab title.** Never run against a shared title id and never against a production title id. The orchestrator prints the active title id when `-AllowLiveWrites` is set so it cannot be missed in a CI log.
- Declares the tier by calling `requires_live_write()` at the top of `before_all` / `before_each`. When either flag is missing, the helper marks the test pending.

Example:

```gdscript
func before_each() -> void:
    if not requires_live_write():
        return
    # … create lobby, mutate, tear down …
```

## How `LIVE_TESTS` and `LIVE_WRITE_TESTS` Flow

`tools\run_all_tests.ps1` is the only place these environment variables are set, and it scrubs unrelated PlayFab secrets from child processes (see existing handling for `PLAYFAB_DEVELOPER_SECRET_KEY`).

| Invocation                                       | `LIVE_TESTS` | `LIVE_WRITE_TESTS` | Effect on tests                                                |
| ------------------------------------------------ | :----------: | :----------------: | :------------------------------------------------------------- |
| `run_all_tests.ps1`                              |      —       |         —          | `contract` runs. `live_read` and `live_write` mark pending.    |
| `run_all_tests.ps1 -Live`                        |    `1`       |         —          | `contract` and `live_read` run. `live_write` marks pending.    |
| `run_all_tests.ps1 -Live -AllowLiveWrites`       |    `1`       |       `1`          | All three tiers run. Banner prints the active title id.        |

`-AllowLiveWrites` without `-Live` is invalid — the orchestrator should refuse it.

## Authoring a New Test

1. Pick the tier honestly. Default to `contract`; promote to `live_read` only if the test cannot be meaningfully asserted offline; promote to `live_write` only if persistent state mutation is the point.
2. Place the test under `tests\godot\<addon>\tests\` as `test_<scenario>.gd`.
3. `extends` the matching base — `gdk_test_base.gd` / `playfab_test_base.gd` / `gameinput_test_base.gd`.
4. For `live_read` and `live_write` tests, call `requires_live()` / `requires_live_write()` at the top of `before_all` (or `before_each` for per-test gating) and short-circuit on `false`.
5. Run the orchestrator at least once offline (`tools\run_all_tests.ps1`) to confirm the new test marks pending instead of failing when live access is missing.

## Why This Matters

PR review repeatedly catches "this test would have failed silently against a shared title" and "this test only passed because LIVE_TESTS was unset". Codifying the tiers behind explicit helpers makes the test pipeline auditable from the test source alone: a reader can tell what a test will do just by reading the first three lines of its `before_all`.
