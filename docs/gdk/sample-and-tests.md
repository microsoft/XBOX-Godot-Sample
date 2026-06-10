# Godot Microsoft GDK sample and tests

This document explains how the repo-wide test pipeline validates
the addons.

See also:

- [`gdk/plugin.md`](plugin.md)
- [`gdk/native-runtime.md`](native-runtime.md)
- [`troubleshooting.md#tests`](../troubleshooting.md#tests)

## Sample projects

The repository currently ships four tutorial-driven sample projects:

- `sample\tutorial_gdk\` — GDK-only tutorial track (sign-in, achievements,
  title storage & stats, Multiplayer Activity). The CMake build mirrors
  `godot_gdk` and `godot_gdk_packaging` into this project (no PlayFab).
- `sample\tutorial_playfab\` — PlayFab-only tutorial track (sign-in,
  leaderboards, lobby, Party). The CMake build mirrors only
  `godot_playfab` into this project (no GDK).
- `sample\tutorial_integrated\` — integrated tutorial track (sign-in,
  Multiplayer Activity, Party, and the final integration tech demo). The CMake
  build mirrors `godot_gdk`, `godot_playfab`, and `godot_gdk_packaging` into
  this project.
- `sample\tutorial_gameinput\` — standalone GameInput tutorial sample. It is
  wired for the GameInput addon rather than the Microsoft GDK runtime addon.

The test hosts under `tests\godot\` remain the automated coverage projects;
the sample projects are reader-facing tutorial references, not GUT hosts.

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
| `tests\godot\gdk\` | `godot_gdk` runtime, system metadata, accessibility, users, achievements, presence, social, launcher URI validation, multiplayer activity, result helpers, embed dispatch, bootstrap behavior, and `godot_gdk_packaging` editor-helper logic under `tests\godot\gdk\tests\packaging\`. |
| `tests\godot\playfab\` | `godot_playfab` root singleton, users, custom-ID sign-in, Game Saves, leaderboards, validation paths, and live PlayFab flows. |
| `tests\godot\gameinput\` | `godot_gameinput` singleton, device/readings wrappers, resources, mapper/action bridge, threading smoke, and bootstrap autoload behavior. |

The hardware-specific GameInput paths are covered by the dedicated GameInput host plus the manual hardware checklist in [`gameinput/manual-tests.md`](../gameinput/manual-tests.md).

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
| `LIVE_TESTS=1` | Enables tests that need a live Microsoft GDK or PlayFab session. | unset |
| `LIVE_WRITE_TESTS=1` | Enables sandbox-only tests that write online state. Use only with sandbox titles. | unset |
| `PLAYFAB_TITLE_ID` | Overrides `playfab/runtime/title_id` inside PlayFab test hosts. Prefer `-PlayFabTitleId` when using the orchestrator. | unset |
| `PLAYFAB_CUSTOM_ID` | Supplies a pre-existing custom id for PlayFab live sign-in tests. | unset |
| `PLAYFAB_DEVELOPER_SECRET_KEY` | Supplies the PlayFab title developer secret key to `tools\configure_playfab_test_title.ps1` only. The script reads it from the process, user, or machine environment. Never forwarded to Godot child processes. | unset |
| `PLAYFAB_MULTIPLAYER_CUSTOM_ID_PREFIX` | Optional prefix for pre-created Multiplayer worker custom IDs. If unset, the runner derives `<PLAYFAB_CUSTOM_ID>-multiplayer`. | unset |

The orchestrator forwards the live/test variables to child Godot processes and scrubs `PLAYFAB_DEVELOPER_SECRET_KEY` from child environments. Prefer the script switches instead of mutating `$env:*` yourself:

| Switch | Effect |
|--------|--------|
| `-Live` | Sets `LIVE_TESTS=1` for every Godot child process. |
| `-AllowLiveWrites` | Sets `LIVE_WRITE_TESTS=1` for every Godot child process. Use only with sandbox titles. |
| `-PlayFabTitleId <id>` | Sets `PLAYFAB_TITLE_ID` for Godot child processes; the PlayFab test base applies it to `playfab/runtime/title_id`. |
| `-PlayFabCustomId <id>` | Sets `PLAYFAB_CUSTOM_ID` for Godot child processes; PlayFab live tests use it with `create_account=false`. |
| `-SkipBuild` | Skips the CMake build stage. Use only when the debug build and mirrored GUT support are already current. |
| `-OutDir <dir>` | Writes `run-summary.json` and `run-summary.md` to another directory. The default is `build\test-results`. |

## PlayFab live title setup

The current sandbox title for PlayFab live validation is `10D176`. Before the first live run against that title, configure it with the repo setup script:

```powershell
$env:PLAYFAB_DEVELOPER_SECRET_KEY = "<developer-secret-key>"
pwsh -NoLogo -NoProfile -ExecutionPolicy Bypass -File .\tools\configure_playfab_test_title.ps1
```

The script reads the secret from the process, user, or machine environment without printing it, ensures the `godot-gdk-ext-live-smoke` custom-ID account exists for `create_account=false` live tests, creates the `godot-gdk-ext-live-smoke-multiplayer-host/client/observer` worker accounts for multi-client Lobby and Matchmaking tests, creates or validates the `godot_gdk_ext_live_smoke_queue` two-player matchmaking queue with a `run_id` equality rule, creates or validates the `wave4_settle_smoke` leaderboard definition, and writes a title-data marker. PlayFab Lobby search keys do not require title configuration; the Multiplayer live runner uses reserved keys such as `string_key1`.

After setup, run the PlayFab live suite with:

```powershell
pwsh -NoLogo -NoProfile -ExecutionPolicy Bypass -File .\tools\run_all_tests.ps1 -Hosts tests\godot\playfab -Live -AllowLiveWrites -PlayFabTitleId "10D176" -PlayFabCustomId "godot-gdk-ext-live-smoke" -PlayFabMatchmakingQueue "godot_gdk_ext_live_smoke_queue"
```

## Live test settings

| Setting | Type | Default | Use |
|---------|------|---------|-----|
| `playfab/tests/custom_id` | String | empty | Pre-existing custom id used by PlayFab live sign-in tests when `PLAYFAB_CUSTOM_ID` is not set. Tests call custom-ID login with `create_account=false`. |
| `playfab/tests/leaderboard_settle_msec` | int | `30000` | Test-host-only polling budget for live leaderboard read-after-write checks before they are marked pending. This key is not registered by the public addon. |

## GUT layout

GUT v9.6.0 is sourced as a git submodule at `third_party\Gut\` (upstream `https://github.com/bitwes/Gut.git`, pinned to tag `v9.6.0`). The CMake build mirrors `third_party\Gut\addons\gut\` into each coverage host as `<host>\addons\gut\`; those mirrored copies are generated and git-ignored. The submodule itself is also un-edited — to refresh GUT, bump the submodule pin (`cd third_party\Gut && git fetch && git checkout <tag>`) and commit the updated submodule pointer.

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
Microsoft GDK machine for successful destinations (for example
`GDK.launcher.launch_uri("ms-settings:privacy-microphone")` and
`GDK.launcher.launch_uri("ms-windows-store://pdp/?productid=<StoreProductId>")`).

After live runs that write online state, use `tools\reset_player_data.ps1` when sandbox state needs to be cleared for a test account. This is especially useful after mutating leaderboard, stats, or achievement-style data in a developer sandbox. Run it only against a test sandbox and test account, never production data.

## Leaderboard eventual consistency

PlayFab leaderboard writes may take several seconds to appear in subsequent reads. Live leaderboard write tests use `TestEnv.poll_until(...)` and the `playfab/tests/leaderboard_settle_msec` budget to poll for the submitted score. If the score is still not visible when the budget expires, the test is marked pending, not failing, because the timeout reflects service eventual consistency rather than a deterministic regression.

## Adding a new test

- Put addon coverage in the owning host: Microsoft GDK and packaging tests in `tests\godot\gdk\tests\`, PlayFab tests in `tests\godot\playfab\tests\`, and GameInput tests in `tests\godot\gameinput\tests\`.
- Extend the matching shared base: `res://addons/godot_gdk_tests/gdk_test_base.gd`, `res://addons/godot_gdk_tests/playfab_test_base.gd`, or `res://addons/godot_gdk_tests/gameinput_test_base.gd`.
- Use `await_completion(...)` or `await_completion_state(...)` for async signal waits. PlayFab tests inherit a dual-pump override that dispatches both PlayFab and, when the optional Microsoft GDK mirror is present, Microsoft GDK queues.
- Use `pending_unless_live()` for live gates, and derive mutating identifiers with `with_unique_id(...)`.
- Use `assert_has_method_named(...)` and `assert_has_signal_named(...)` for reflection checks.
- If a directory tree contains GUT-extending `.gd` files, place an empty `.gut_skip_validation` sentinel at that tree root so `tools\check_gd_scripts_headless.ps1` skips standalone parsing that would otherwise produce UID or GUT class-name warnings.
- For startup-only autoload or project-setting behavior, add a bootstrap mini-runner under `tests\bootstrap\` instead of trying to mutate those settings inside a normal GUT test.

## Troubleshooting pointers

See [`troubleshooting.md#tests`](../troubleshooting.md#tests) for common discovery, GUT, bootstrap, doctest, live-test, and leaderboard issues.
