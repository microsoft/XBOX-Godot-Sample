# Godot GDK sample and tests

This document explains how the sample projects use the addons and how the repo-wide test pipeline validates them.

See also:

- [`godot-gdk-plugin.md`](godot-gdk-plugin.md)
- [`godot-gdk-native-runtime.md`](godot-gdk-native-runtime.md)
- [`troubleshooting.md#tests`](troubleshooting.md#tests)

## Sample project role

The `sample\` directory contains multiple sample projects:

| Sample | Description |
|--------|-------------|
| `sample\gdk_demo\` | Baseline GDK runtime/users/achievements/presence/social demo. |
| `sample\gdk_launch_point\` | GDK Launch Point scenario shell for manual runtime exploration. |
| `sample\multiplayer_pong\` | Gameplay demo with Xbox identity and GameInput rumble. It is not a test host. |
| `sample\playfab_demo\` | PlayFab smoke sample. |

All samples that use GDK share the addon-owned bootstrap path, `res://addons/godot_gdk/runtime/gdk_bootstrap.gd`, alongside the `plugin.cfg` editor plugin and the addon files synced by the CMake build system.

## Autoload bootstrap

Each GDK-facing sample's `project.godot` autoloads the addon-owned `GDKBootstrap` singleton from `addons\godot_gdk\runtime\gdk_bootstrap.gd`.
The bootstrap listens to the single `GDK.users.user_changed` event for user lifecycle and state output.

| Sample | `gdk/runtime/initialize_on_startup` | `gdk/runtime/auto_add_primary_user` | Role |
|--------|-------------------------------------|--------------------------------------|------|
| `gdk_demo` | `true` | `true` | Baseline demo starts the runtime and silent sign-in automatically. |
| `gdk_launch_point` | `false` | `false` | Launch Point stays manual so the scenario shell can drive runtime actions explicitly. |
| `multiplayer_pong` | `true` | `true` | Pong wants Xbox identity ready for the lobby flow. |
| `playfab_demo` | `true` | `true` | PlayFab demo depends on GDK runtime startup and silent sign-in before PlayFab calls. |

All samples still set `gdk/runtime/embed_dispatch=true`. Demo-style samples therefore expect native auto-dispatch to stay enabled, while Launch Point keeps runtime startup under explicit scenario control.

## Demo scenes

### GDK Demo (`sample\gdk_demo\main.gd`)

A minimal runtime/users/achievements/presence/social demo. It reflects runtime state, shows the primary user's gamertag and XUID, retries silent sign-in, queries and updates achievement `1`, displays cached presence, starts the Social Manager graph, requests the default friends group, and shows the tracked friend count.

### GDK Launch Point (`sample\gdk_launch_point\main.gd`)

`sample\gdk_launch_point\main.gd` builds a scenario catalog with grouped runtime, users, achievements, multiplayer activity, and GameInput actions. It provides nested navigation, a tile-style menu, a persistent event log, and a side panel that reflects the selected scenario and live state.

### Multiplayer Pong (`sample\multiplayer_pong\`)

A gameplay demo imported from `godot-demo-projects` and extended with Xbox identity, single-player AI, visual effects, and controller rumble. It is intentionally not a GUT coverage host.

## Overview

`tools\run_all_tests.ps1` is the single-command local definition of green for this repository. It validates GDScript parseability, builds the debug configuration, runs the pure C++ doctest executable, runs GUT in each coverage host, executes startup-only bootstrap mini-runners in fresh Godot processes, and writes machine- and reviewer-readable summaries under `build\test-results\`.

## Quick start

Run the full default pipeline from the repository root:

```powershell
pwsh -NoLogo -NoProfile -ExecutionPolicy Bypass -File .\tools\run_all_tests.ps1
```

Use this command as the standard path. Direct Godot `--script` or GUT invocations are useful only for ad-hoc debugging of a specific host.

## The 6 stages

1. **Parse gate** — runs `tools\check_gd_scripts_headless.ps1` before anything else. This catches GDScript syntax, standalone-parse, and validation issues without waiting for a build.
2. **CMake build** — runs `cmake --build build --preset debug` unless `-SkipBuild` is supplied. The build produces native binaries, `build\bin\Debug\gdk_unit_tests.exe`, and mirrors test support into the coverage hosts.
3. **C++ doctest** — runs `build\bin\Debug\gdk_unit_tests.exe`. This pins pure helper logic that can run outside Godot.
4. **GUT host runs** — imports each host once with `--headless --import`, then runs GUT with `-gdir=res://tests -ginclude_subdirs -gexit`. The orchestrator parses GUT's summary and fails the stage if no tests were discovered, even when GUT exits `0`.
5. **Bootstrap mini-runners** — launches startup-only scripts under each bootstrap-capable host's `tests\bootstrap\` directory as separate Godot `--script` processes. These cover project settings and autoload behavior that must be fixed before GUT starts.
6. **Aggregate** — writes `build\test-results\run-summary.json` and `build\test-results\run-summary.md`, prints `Overall: pass` or `Overall: fail`, and exits with the matching process status.

## Test hosts

| Host | Covers |
|------|--------|
| `tests\godot\gdk\` | `godot_gdk` runtime, users, achievements, presence, social, launcher URI validation, multiplayer activity, result helpers, embed dispatch, bootstrap behavior, and `godot_gdk_packaging` editor-helper logic under `tests\godot\gdk\tests\packaging\`. |
| `tests\godot\playfab\` | `godot_playfab` root singleton, users, custom-ID sign-in, Game Saves, leaderboards, validation paths, and live PlayFab flows. |
| `tests\godot\gameinput\` | `godot_gameinput` singleton, device/readings wrappers, resources, mapper/action bridge, threading smoke, and bootstrap autoload behavior. |

`sample\gdk_demo\`, `sample\playfab_demo\`, `sample\gdk_launch_point\`, and `sample\multiplayer_pong\` are demo projects, not test hosts. GUT is not mirrored into GDK Demo, PlayFab Demo, or Launch Point once their coverage has moved, and the hardware-specific GameInput paths demonstrated by the samples are covered by the dedicated GameInput host plus the manual hardware checklist in [`godot-gameinput-manual-tests.md`](godot-gameinput-manual-tests.md).

## Definition of green

A local run is green when all of the following are true:

- `tools\run_all_tests.ps1` exits `0` and prints `Overall: pass`.
- `build\test-results\run-summary.json` and `build\test-results\run-summary.md` were written for that run.
- Every GUT host discovered at least one test, has zero failing tests, and satisfies its parity baseline.
- Every per-host baseline in `tests\baselines\<host>.json` has `post.asserts >= pre.asserts`, with zero post-migration failures.
- Bootstrap mini-runners either pass or are explicitly skipped because the selected host filter excluded them.

## Env-var matrix

| Var | Effect | Default |
|---|---|---|
| `LIVE_TESTS=1` | Enables tests that need a live GDK or PlayFab session, including tests that write online state. | unset |
| `PLAYFAB_TITLE_ID` | Overrides `playfab/titleid` inside PlayFab test hosts. Prefer `-PlayFabTitleId` when using the orchestrator. | unset |
| `PLAYFAB_CUSTOM_ID` | Supplies a pre-existing custom id for PlayFab live sign-in tests. | unset |

The orchestrator forwards these variables only to child Godot processes. Prefer the script switches instead of mutating `$env:*` yourself:

| Switch | Effect |
|--------|--------|
| `-Live` | Sets `LIVE_TESTS=1` for every Godot child process. |
| `-PlayFabTitleId <id>` | Sets `PLAYFAB_TITLE_ID` for Godot child processes; the PlayFab test base applies it to `playfab/titleid`. |
| `-PlayFabCustomId <id>` | Sets `PLAYFAB_CUSTOM_ID` for Godot child processes; PlayFab live tests use it with `create_account=false`. |
| `-SkipBuild` | Skips the CMake build stage. Use only when the debug build and mirrored GUT support are already current. |
| `-OutDir <dir>` | Writes `run-summary.json` and `run-summary.md` to another directory. The default is `build\test-results`. |

## Live test settings

| Setting | Type | Default | Use |
|---------|------|---------|-----|
| `gdk/tests/live_required` | bool | `false` | Records that a sample configuration expects live GDK prerequisites. Tests still use the env-var helpers to decide whether to run or mark pending. |
| `playfab/tests/custom_id` | String | empty | Pre-existing custom id used by PlayFab live sign-in tests when `PLAYFAB_CUSTOM_ID` is not set. Tests call custom-ID login with `create_account=false`. |
| `playfab/tests/leaderboard_settle_msec` | int | `30000` | Polling budget for live leaderboard read-after-write checks before they are marked pending. |

## GUT layout

GUT v9.6.0 is vendored once at `addons\godot_gdk\tests_support\gut\`. The CMake build mirrors it into each coverage host as `<host>\addons\gut\`; those mirrored copies are generated and git-ignored.

Each host owns tests under its local `tests\` directory. Shared bases are mirrored as `res://addons/godot_gdk_tests/` and come from `addons\godot_gdk\tests_support\bases\`.

Important: GUT v9.6.0's `-gdir` is non-recursive. The orchestrator therefore passes `-ginclude_subdirs`:

```powershell
& $godot --headless -s res://addons/gut/gut_cmdln.gd -gdir=res://tests -ginclude_subdirs -gexit
```

Any manual GUT invocation that expects files below `tests\smoke\`, `tests\packaging\`, or another subdirectory must include `-ginclude_subdirs` too.

## Bootstrap mini-runners

Bootstrap mini-runners are small `SceneTree` scripts launched as separate Godot processes for tests that mutate startup-only project settings before autoloads run. They live under host-local `tests\bootstrap\` directories, for example `tests\godot\gdk\tests\bootstrap\run_initialize_on_startup_true.gd` and `tests\godot\gameinput\tests\bootstrap\test_bootstrap_autoload_present.gd`.

The preferred naming pattern for new mini-runners is `run_*.gd`, not `test_*.gd`, so recursive GUT discovery does not try to treat a direct `--script` runner as a GUT test file. The orchestrator's bootstrap stage runs every `.gd` file in `tests\bootstrap\` for `tests\godot\gdk\` and `tests\godot\gameinput\` when those hosts are selected.

A runner should print a clear status line using `BOOTSTRAP_OK:` on success or `BOOTSTRAP_FAIL:` on failure, then call `quit(0)` for pass or a non-zero `quit(...)` code for failure. The current aggregate pass/fail is based on the process exit code; the literal prefixes are the log contract that makes bootstrap failures readable in summaries and manual runs.

## C++ doctest

The doctest target is `gdk_unit_tests.exe`, built at `build\bin\Debug\gdk_unit_tests.exe` by the default debug build. Its sources live under `tests\cpp\`.

Current doctest coverage pins `format_hresult_hex` for both addons:

- `tests\cpp\result_codes\test_gdk_result_codes.cpp`
- `tests\cpp\result_codes\test_playfab_result_codes.cpp`

Doctest is limited to pure helpers. `godot::String`, `godot::Variant`, and other Variant-family types cannot be safely instantiated in a free-standing executable because the GDExtension function table is populated only when Godot loads the extension. Helpers that touch Variant-family types stay in GUT tests that run inside real Godot processes.

## Test parity baselines

Parity baselines live at `tests\baselines\gdk.json`, `tests\baselines\playfab.json`, and `tests\baselines\gameinput.json`. Each file records the migrated host's baseline counts:

- `host` — the sample host name.
- `pre` — assertion count and method from the pre-GUT runner before migration.
- `post` — current GUT assertions, tests, passing, failing, pending, and any host-specific bootstrap counts.
- `captured_at` and `godot_version` — provenance for the recorded counts.

Update a baseline only when the host's intentional coverage changes. Do not lower `post.asserts` below `pre.asserts`; if coverage decreases for a legitimate reason, call it out in review instead of silently changing the baseline.

## Live test cleanup

## Manual launcher smoke checks

`GDK.launcher` success paths invoke OS-level URI handlers (`XLaunchUri`) and are
not deterministic in CI/headless hosts. Keep automated coverage focused on input
validation and unsupported-destination errors, then run manual smoke checks on a
PC GDK machine for successful destinations (for example
`GDK.launcher.launch_uri("ms-settings:privacy-microphone")` and
`GDK.launcher.launch_uri("ms-windows-store://pdp/?productid=<StoreProductId>")`).

After live runs that write online state, use `tools\reset_player_data.ps1` when sandbox state needs to be cleared for a test account. This is especially useful after mutating leaderboard, stats, or achievement-style data in a developer sandbox. Run it only against a test sandbox and test account, never production data.

## Leaderboard eventual consistency

PlayFab leaderboard writes may take several seconds to appear in subsequent reads. Live leaderboard write tests use `TestEnv.poll_until(...)` and the `playfab/tests/leaderboard_settle_msec` budget to poll for the submitted score. If the score is still not visible when the budget expires, the test is marked pending, not failing, because the timeout reflects service eventual consistency rather than a deterministic regression.

## Adding a new test

- Put addon coverage in the owning host: GDK and packaging tests in `tests\godot\gdk\tests\`, PlayFab tests in `tests\godot\playfab\tests\`, and GameInput tests in `tests\godot\gameinput\tests\`.
- Extend the matching shared base: `res://addons/godot_gdk_tests/gdk_test_base.gd`, `res://addons/godot_gdk_tests/playfab_test_base.gd`, or `res://addons/godot_gdk_tests/gameinput_test_base.gd`.
- Use `await_completion(...)` or `await_completion_state(...)` for async signal waits. PlayFab tests inherit a dual-pump override that dispatches both PlayFab and, when the optional GDK mirror is present, GDK queues.
- Use `pending_unless_live()` for live gates, and derive mutating identifiers with `with_unique_id(...)`.
- Use `assert_has_method_named(...)` and `assert_has_signal_named(...)` for reflection checks.
- If a directory tree contains GUT-extending `.gd` files, place an empty `.gut_skip_validation` sentinel at that tree root so `tools\check_gd_scripts_headless.ps1` skips standalone parsing that would otherwise produce UID or GUT class-name warnings.
- For startup-only autoload or project-setting behavior, add a bootstrap mini-runner under `tests\bootstrap\` instead of trying to mutate those settings inside a normal GUT test.

## Troubleshooting pointers

See [`troubleshooting.md#tests`](troubleshooting.md#tests) for common discovery, GUT, bootstrap, doctest, live-test, and leaderboard issues.
