# Test Strategy

## Overview

This document is the durable record of the test-coverage strategy for the four addons in this repository (`godot_gdk`, `godot_playfab`, `godot_gameinput`, `godot_gdk_packaging`), the GDScript test hosts that exercise them (`tests\godot\gdk`, `tests\godot\playfab`, `tests\godot\gameinput`), and the C++ helper code that backs them. It captures the framework choice (GUT for GDScript, doctest for C++), the vendoring and mirroring layout, the orchestrator contract, the live-tests guard semantics, the autoload/bootstrap mini-runner pattern, and the definition of "green" used in lieu of CI. GitHub Actions / CI workflows, expansion of the manual hardware checklist in `docs\gameinput\manual-tests.md`, and any testing of the (now-removed) sample projects are explicitly out of scope; the legacy sample projects (`gdk_demo`, `playfab_demo`, `gdk_launch_point`, `multiplayer_pong`) were demo projects, not coverage hosts. The tutorial-driven sample revamp (PR 3) reintroduces `sample\tutorial_app\` and `sample\tutorial_gameinput\`, which will likewise be demo / tutorial reference projects, not coverage hosts.

## Test surface inventory

- GDScript test hosts: `tests\godot\gdk` (covers `godot_gdk` and `godot_gdk_packaging`), `tests\godot\playfab` (covers `godot_playfab`), `tests\godot\gameinput` (covers `godot_gameinput`). The legacy sample projects (`gdk_demo`, `playfab_demo`, `gdk_launch_point`, `multiplayer_pong`) were intentionally excluded as demo projects. They have since been removed; the tutorial-driven samples returning in PR 3 (`sample\tutorial_app\`, `sample\tutorial_gameinput\`) will likewise stay out of the coverage host set.
- GUT framework, sourced as a git submodule at `third_party\Gut\` (upstream `https://github.com/bitwes/Gut.git`, pinned to tag `v9.6.0`); CMake mirrors the embedded `addons\gut\` into coverage hosts. Mirrored copies are git-ignored.
- C++ unit tests via doctest, vendored at `tests\cpp\third_party\doctest\doctest.h`, built behind `GDK_BUILD_TESTS=ON` (default ON) into the `gdk_unit_tests` executable.
- Headless GDScript validator `tools\check_gd_scripts_headless.ps1` (already enforced by the pre-commit hook).
- Orchestrator `tools\run_all_tests.ps1` (lands in Wave 3 (`infra-orchestrator`)).
- Bootstrap mini-runners under `tests\godot\gdk\tests\bootstrap\` (and the analogous folder under `tests\godot\gameinput\tests\bootstrap\` for the GameInput autoload). Each bootstrap scenario runs in its own Godot child process so the autoload starts fresh with the desired project settings already in place.

## GUT layout

GUT is sourced as a git submodule at:

```
third_party\Gut\
```

pinned to upstream tag `v9.6.0` (Godot 4 line) from `https://github.com/bitwes/Gut.git`. The upstream repo keeps its plugin source under `addons/gut/`, so the GUT addon source-of-truth from the superproject's perspective is `third_party\Gut\addons\gut\`. The upstream `LICENSE.md` (MIT, Tom "Butch" Wesley) and `plugin.cfg` (which records the upstream version) ship inside the submodule and need no local mirror.

CMake mirrors that single source into coverage hosts via the `godot_addon_mirror_test_support` function in `cmake\GodotExtensionCommon.cmake`:

```cmake
godot_addon_mirror_test_support(
    SOURCE_DIR  "${CMAKE_SOURCE_DIR}/third_party/Gut/addons/gut"
    DEST_SUBDIR "gut"
    HOST_DIRS "${CMAKE_SOURCE_DIR}/tests/godot/gdk"
              "${CMAKE_SOURCE_DIR}/tests/godot/playfab"
              "${CMAKE_SOURCE_DIR}/tests/godot/gameinput"
)
```

Demo-only projects are intentionally absent. The mirrored copies under `<host>\addons\gut\` are git-ignored. Refresh procedure: bump the submodule pin from the repo root with `cd third_party\Gut && git fetch && git checkout <new-tag>`, run a build (the mirror picks up the new copy via `CONFIGURE_DEPENDS`), and commit the updated submodule pointer in the superproject. Do not vendor copies into the superproject — `git ls-files` does not recurse into submodules, so the parse-gate validator skips submodule contents automatically.

Historical note: GUT was initially vendored under `addons\godot_gdk\tests_support\gut\` (see the `infra-vendor-gut` phase below). That tree was migrated to the `third_party\Gut\` submodule layout to simplify upstream refreshes; the mirroring contract and `godot_addon_mirror_test_support` function are unchanged.

## doctest layout

doctest is pinned to upstream tag `v2.5.2`:

```
https://raw.githubusercontent.com/doctest/doctest/v2.5.2/doctest/doctest.h
```

The single header lives at:

```
tests\cpp\third_party\doctest\doctest.h
```

with the upstream `LICENSE.txt` and a `VERSION.txt` recording the tag, source URL, and date. No git submodule, no `FetchContent`.

The CMake function `godot_addon_doctest_target` in `cmake\GodotExtensionCommon.cmake` produces the `gdk_unit_tests` executable behind the option `GDK_BUILD_TESTS` (default ON in the default preset). One translation unit defines `DOCTEST_CONFIG_IMPLEMENT`; the rest include the header without the implementation macro.

C++ tests cover **pure logic only**. Where a helper is currently embedded in a Godot-aware class, the production code is refactored to extract the helper into a free function or pure helper class (for example `gdk_result_codes.cpp/.h`). The production site then calls the extracted helper, and the doctest target consumes the same code. Logic is never duplicated into the test target just to make it testable.

## C++ test scope rules

doctest covers only code that has no Godot, GDExtension-interface, or GDK dependency. **Crucially this excludes `godot::String`, `godot::Variant`, `godot::PackedByteArray`, and any other Variant-family type** — see the "godot::String constraint" note below.

- HRESULT hex-formatting helpers that operate on a caller-provided `char[]` buffer (currently `gdk_internal::format_hresult_hex` and `playfab_internal::format_hresult_hex` in `addons/godot_gdk/src/gdk_result_codes_internal.{h,cpp}` and `addons/godot_playfab/src/playfab_result_codes_internal.{h,cpp}`).
- Future packaging string helpers, **only if and when** they are extracted from `addons/godot_gdk_packaging/editor/packaging_content_preparer.gd` into a pure C++ module that does not depend on `godot::String` or `godot::RegEx`. As of Wave 2 the packaging helpers are GDScript-only and are tested via GUT — see `wave4-packaging-coverage`.

### godot::String constraint (verified Wave 2, `cpp-result-tests`)

`godot::String` (and every other Variant-family type) cannot be safely instantiated in a free-standing doctest exe. Every constructor dispatches through `godot::gdextension_interface::string_new_with_*` function pointers that the engine populates during the GDExtension entry-point handshake. In a standalone exe those pointers stay null and the first non-empty `String` segfaults inside `String::String(const char *)`.

Implications for any future C++ test work in this repo:

- The "thin forwarders" `format_hresult_string`, `format_hresult_message`, and `code_or_format_hresult` (which return / accept `godot::String`) are **NOT runtime-testable** from `gdk_unit_tests.exe`. They are exercised end-to-end through GUT tests that drive the public `GDKResult` / `PlayFabResult` API from a real Godot process (see `wave4-gdk-coverage::test_result_helpers.gd`).
- A test target that must touch a Variant-family type has only two options: (a) extract a pure char-buffer / `std::string_view` variant of the production helper and test that, or (b) cover the helper via GUT instead. Mocking the gdextension interface is explicitly out of scope.
- Compile-time guarantees on Variant-touching helpers (e.g. `static_assert(noexcept(...))`, buffer-size constants) are still fair game and recommended.

Anything that touches `Object`, `Signal`, `Ref<>`, `ClassDB`, `XAsync`, or `XUser` stays GDScript-side and is exercised through the public API from GUT. Specifically, `gdk_pending_signal`, `playfab_pending_signal`, `gdk_signal_xasync_context`, and `playfab_signal_xasync_context` are out of doctest scope; their behavior is inseparable from the Godot object/signal lifecycle and is observed via GUT tests of the public API. If a candidate test would require constructing or driving a Godot object, the correct response is to extract a pure helper, not to wrap Godot types into doctest.

## GUT runner contract

The verified canonical command, run from a sample-project root after the one-time import described below, is:

```powershell
& $godot --headless -s res://addons/gut/gut_cmdln.gd -gdir=res://tests -gexit
```

Notes verified by the Wave -1 spike against GUT `v9.6.0`:

- `-gexit` exits with the correct status (0 on pass, non-zero on fail). `-gexit_on_success` is not required and is not used.
- Defaults `-gprefix=test_` and `-gsuffix=.gd` are in effect; supply them explicitly only when non-default discovery is needed, and **always quote them in PowerShell** (e.g. `"-gsuffix=.gd"`). Unquoted `-gsuffix=.gd` is mis-parsed by Godot's CLI as `[".gd"]` and the run exits 1 with `Unknown arguments: [".gd"]`.
- `gut_cmdln.gd` will not run until `class_name` registration has been populated; a one-time import must run per host before the first GUT invocation:

```powershell
& $godot --headless --import
```

  When the registration is missing, GUT prints "Some GUT class_names have not been imported" and exits **0** without running anything. The orchestrator therefore asserts `Tests > 0` from GUT's own summary on every host run; trusting the exit code alone makes a misconfigured `-gdir` silently green.

## Env propagation contract

Godot does not support a `--env-file` flag. Child-process environment is set explicitly via `[System.Diagnostics.ProcessStartInfo]`:

```powershell
$psi = [System.Diagnostics.ProcessStartInfo]::new($godotExe)
$psi.UseShellExecute = $false
$psi.RedirectStandardOutput = $true
$psi.RedirectStandardError  = $true
foreach ($k in $childEnv.Keys) {
    $psi.EnvironmentVariables[$k] = "$($childEnv[$k])"
}
$proc = [System.Diagnostics.Process]::Start($psi)
```

GDScript reads these via `OS.get_environment("LIVE_TESTS")`. The `addons\godot_gdk\tests_support\test_env.gd` helper centralizes the lookup.

Two traps are baked into this contract because they were verified by the spike:

- `$env:NAME = "..."` mutated in the parent shell does **not** propagate into a child started with `Process.Start(psi)` and `UseShellExecute = $false`. The only reliable channel is `psi.EnvironmentVariables`.
- Sync `Process.StandardOutput.ReadToEnd()` plus `Process.StandardError.ReadToEnd()` deadlocks. Godot floods stderr with `Condition "!FileAccess::exists(path)"` lines whenever a GDExtension is missing (the canonical clean-tree-error pattern in this project), filling the stderr pipe buffer in seconds. The orchestrator uses `BeginOutputReadLine()` / `BeginErrorReadLine()` (or `ReadToEndAsync()` on both streams concurrently) to drain both pipes without blocking.

## Live tests two-mode contract

Tests have two runtime modes, gated by the environment variable that the orchestrator forwards through `psi.EnvironmentVariables`:

- **Default** — `LIVE_TESTS` is unset. Every live-network test falls through to `pending(...)` when its prerequisites are not met (no signed-in user, no PlayFab title id, etc.). The suite is green on any developer workstation.
- **`LIVE_TESTS=1`** — live tests run strict. Missing prerequisites are real failures (`fail_test()`), not skips. Live calls execute against the configured service, including read-side calls (`add_user_with_ui_async` / sign-in, `get_folder`, `get_folder_size`, `get_leaderboard_async`, `get_leaderboard_around_user_async`, `get_friend_leaderboard_async`) and online-state writes (`submit_score_async`, `set_save_description_async`, `reset_cloud_async`, `set_activity_async` / `delete_activity_async`, `update_achievement_async`). Live write tests require:
  - Each test id is unique-per-run, formatted as `gdkfleet-<datetime>-<rand>`:

    ```gdscript
    var run_id := "gdkfleet-" + Time.get_datetime_string_from_system().replace(":", "-") + "-" + str(randi() % 100000)
    ```

  - Every live write test pairs setup with best-effort cleanup where the public API supports it; cleanup failure logs `pending`, not `fail`.
  - Leaderboard read-back after `submit_score_async` polls `get_leaderboard_async` up to `playfab/tests/leaderboard_settle_msec` (default `30000`) for the per-run id; timeout reports `pending(...)` (eventual-consistency flake), not `fail_test()`. Game Saves polls `get_folder_size` for the expected delta on the same pattern.

Manual sandbox cleanup is documented in `tools\reset_player_data.ps1`. Live write coverage should run only against sandbox titles and test accounts.

## Bootstrap / autoload tests

Project settings such as `gdk/runtime/initialize_on_startup`, `gdk/runtime/auto_add_primary_user`, and `gameinput/runtime/embed_dispatch` are read at SceneTree start, before any GUT test gets a chance to execute. Mutating them inside a running Godot process is not equivalent to having them in place when the autoload runs `_ready()`. Each bootstrap scenario therefore runs in its own Godot child process via a mini-runner script that:

- Lives under `tests\godot\gdk\tests\bootstrap\run_*.gd` (e.g. `run_initialize_true.gd`, `run_initialize_false.gd`, `run_auto_add_user_true.gd`, `run_auto_add_user_false.gd`, `run_skip_when_check_only.gd`), and similarly under `tests\godot\gameinput\tests\bootstrap\` for the `gameinput_bootstrap.gd` autoload scenarios.
- `extends SceneTree`, sets the project setting it needs before invoking the GUT runner, and restores the original value on `_finalize()`.
- Invokes the GUT runner against a single test file (e.g. `test_bootstrap_initialize_true.gd`) so the scenario is asserted by GUT-style assertions and reported through the same summary path.

The orchestrator launches each mini-runner as a fresh `Process.Start(psi)`, asserts the per-scenario exit code, and aggregates the results into the run summary alongside the host GUT runs.

## Packaging editor-vs-headless decision

`--headless` is sufficient for the `@tool` `RefCounted` packaging helpers (`gdk_toolchain.gd` and equivalents): they load and execute correctly under `--headless`, and the spike verified two sample assertions (`get_gdk_version()` parsing the `GameDKCoreLatest` env var, `get_bin_dir()` returning a string) pass cleanly there.

`--editor` is **not** a viable orchestrator stage. The spike verified that `--editor --headless --quit-after 200 -s res://addons/gut/gut_cmdln.gd ...` does not execute the `-s` script at all — the editor takes over the main loop, reaches its quit timer, and exits without running `gut_cmdln.gd`. A separate `--editor` orchestrator stage would not run anything anyway.

Tests that genuinely need a live `EditorInterface` instance or a real `EditorPlugin` registration are the explicit exception, not a budgeted pipeline stage. If and when such a test is required, this spec is updated to document the alternate runner (for example an `EditorPlugin` autoload that drives GUT internally) at that time.

## Headless validator

`tools\check_gd_scripts_headless.ps1` is the parse gate and is already enforced by the pre-commit hook. Vendored or `extends GutTest` paths use a sentinel marker file so the validator skips standalone parsing that cannot load the GUT class registry. New `tests_support/` additions and new GUT-test directories must not require a script edit to remain green.

## Orchestrator contract

`tools\run_all_tests.ps1` is the single command for the full local test pass. Implementation lands in Wave 3 (`infra-orchestrator`).

```powershell
tools\run_all_tests.ps1 [-Live] [-SkipBuild] [-OutDir <dir>]
```

The pipeline runs in this order. Any stage failing exits the orchestrator non-zero and skips downstream stages.

1. **Parse gate**: `tools\check_gd_scripts_headless.ps1`. Fails fast on any parse error.
2. **CMake build**: `cmake --build build --preset debug`. Skipped when `-SkipBuild` is supplied.
3. **C++ doctest**: `build\bin\Debug\gdk_unit_tests.exe --reporters=console --no-colors`. Exit code propagates.
4. **Per-host import**: one-time `& $godot --headless --import` per coverage host (idempotent; covers the GUT class-name registration requirement).
5. **GUT runs**: for each of `tests\godot\gdk`, `tests\godot\playfab`, `tests\godot\gameinput`, in turn:

    ```powershell
    & $godot --headless -s res://addons/gut/gut_cmdln.gd -gdir=res://tests -gexit
    ```

   The orchestrator asserts `Tests > 0` from GUT's own summary; a zero-discovery run is a failure regardless of exit code.
6. **Bootstrap mini-runners**: each mini-runner under `tests\godot\gdk\tests\bootstrap\` and `tests\godot\gameinput\tests\bootstrap\` is launched as a fresh `Process.Start(psi)` with the desired project settings already injected. Per-scenario exit codes feed the aggregate.
7. **Aggregate**: results written to `<OutDir>\run-<timestamp>\run-summary.json` and `run-summary.md`. The Markdown summary is what gets pasted into PR descriptions.

Switches:

- `-Live` sets `LIVE_TESTS=1` in the per-child `psi.EnvironmentVariables`.
- `-SkipBuild` skips stage 2.
- `-OutDir <dir>` overrides the run-summary destination.

Env propagation across every stage uses `psi.EnvironmentVariables` and async stdout/stderr drains as described in the env-propagation contract above.

## Definition of "green" (no CI)

A PR against `tests\coverage-expansion` is green when all of the following hold:

- `tools\run_all_tests.ps1` (default flags) was run on the agent's worktree at the head SHA of the PR branch and exited `0`.
- `run-summary.md` from that run is pasted into the PR description and includes:
  - The head SHA the run executed against.
  - The Godot console executable's `--version` output (the runtime version string, **not** the file name — `Godot_v4.6.1-stable_win64_console.exe` reports `4.6.2.stable.official.71f334935`).
  - Per-host counts from GUT's own summary: `Tests`, `Passing`, `Failing`, `Pending`.
  - C++ doctest counts: test cases (passed/failed/skipped) and assertions (passed/failed).
  - The baseline-parity check: post-migration assertion count is `>=` the count recorded in `tests\baselines\<host>.json`. For a host with no baseline (a brand-new host), the summary states "no baseline" and the PR description records explicit reviewer acknowledgement.
- For Wave 4 todos that touch live behavior, an additional run with `-Live` against the dedicated sandbox is attached to the PR description.

The trigger to re-open the CI question is recorded here: if process-only validation produces three or more PRs with stale or fabricated `run-summary.md` blocks, this spec is updated to require a minimal CI workflow.

## Branch and worktree model

The integration branch `tests/coverage-expansion` carries the entire effort and lands as a single PR to `main`. It is created once off `postmortem_cleanup` and its own PR never lands in the existing `postmortem_cleanup` PR. If `postmortem_cleanup` advances during the fleet's lifetime, `tests/coverage-expansion` is rebased forward; fleet branches never re-target `postmortem_cleanup`.

Each todo executes on `tests/<todo-id>` in `.copilot-worktrees\<todo-id>\`, branched off `origin/tests/coverage-expansion`:

```powershell
git fetch origin tests/coverage-expansion
git worktree add -b tests/<todo-id> .copilot-worktrees\<todo-id> origin/tests/coverage-expansion
cd .copilot-worktrees\<todo-id>
git submodule update --init --recursive
```

Each fleet PR targets `tests/coverage-expansion` (not `postmortem_cleanup`). After merge, the parent session prunes:

```powershell
git worktree remove .copilot-worktrees\<todo-id>
git branch -D tests/<todo-id>
```

PR opening is gated on user review. Agents commit and push to their fleet branch; the user opens the PR.

## Out of scope (re-stated explicitly)

- GitHub Actions / CI workflow.
- Manual hardware-checklist expansion in `docs\gameinput\manual-tests.md`.
- Testing of the historical `sample\multiplayer_pong` (now removed). GUT was intentionally not mirrored into it. The GameInput rumble paths it exercised are covered through `tests\godot\gameinput` and the manual hardware checklist.

## Todo manifest

The complete todo + dependency graph is enumerated below so this document is self-contained and does not depend on session-local SQL state.

| ID | Wave | Title | Depends on | Owns |
| --- | --- | --- | --- | --- |
| `infra-spike` | -1 | Wave -1 de-risking spike | (none) | Throwaway PR proving GUT vendoring + mirror, the canonical GUT CLI flag set with exit-code propagation, env propagation via `ProcessStartInfo.EnvironmentVariables` without `--env-file`, the packaging editor-vs-headless decision, and a trivial doctest exe. Output: `spike-report.md` recording verified flags, packaging/editor decision, env method, and pinned versions. |
| `infra-spec-capture` | 0 | Write `spec/testing-strategy.md` | `infra-spike` | This document. |
| `infra-vendor-gut` | 1 | Vendor GUT + CMake mirror to coverage hosts | `infra-spec-capture` | `addons\godot_gdk\tests_support\gut\` (with `LICENSE.md` and `VERSION.txt`); the `godot_addon_mirror_test_support()` function in `cmake\GodotExtensionCommon.cmake` mirroring the single source into coverage hosts only — explicitly **not** into demo-only projects. Generalises the headless-validator exclusion to `.gdignore` / sentinel marker. |
| `infra-doctest-target` | 1 | Vendored doctest header + CMake | `infra-spec-capture` | `tests\cpp\third_party\doctest\doctest.h` (with `LICENSE.txt`, `VERSION.txt`); `godot_addon_doctest_target()` in `cmake\GodotExtensionCommon.cmake` producing `gdk_unit_tests.exe` behind `GDK_BUILD_TESTS=ON` (default ON in the default preset). One TU defines `DOCTEST_CONFIG_IMPLEMENT`. Includes a smoke `TEST_CASE` proving the wiring. |
| `infra-shared-base` | 2 | Shared GUT base classes + `test_env.gd` | `infra-vendor-gut` | `addons\godot_gdk\tests_support\bases\{gdk,playfab,gameinput}_test_base.gd` absorbing the duplicated helpers from today's three `test_context.gd` files; `addons\godot_gdk\tests_support\test_env.gd` with live-test helpers consulting `LIVE_TESTS` and `gdk/tests/live_required`. CMake mirrors `tests_support/` into the three coverage hosts. |
| `cpp-result-codes-extract` | 2 | Extract pure HRESULT-to-code helpers | `infra-doctest-target` | Refactor `gdk_result.cpp` and `playfab_result.cpp` to extract HRESULT-to-code mapping and message formatting into pure free functions (e.g. `gdk_result_codes.cpp/.h`). Production sites call the extracted helper. **No production behavior change.** |
| `cpp-result-tests` | 2 | doctest: result-code mapping coverage | `cpp-result-codes-extract` | doctest cases for the extracted helpers: every documented HRESULT maps to its expected code string; unknown HRESULTs map to a documented fallback; message formatting is stable. |
| `cpp-packaging-helpers` | 2 | doctest: packaging string helpers | `infra-doctest-target` | doctest cases pinning the `patch_executable_name` / `inject_vc14_dependency` / XML-escape contracts from the C++ side. Mirrors the existing GD packaging suite assertions so a refactor cannot desync the two. |
| `gdk-migrate-suites` | 3 | Migrate GDK suites to GUT | `infra-vendor-gut`, `infra-shared-base` | Rewrite the GDK coverage suite as GUT `test_*.gd` files extending `gdk_test_base`. Map `log_skip` -> `pending()`. Records `tests\baselines\gdk.json`; post-migration assertion count must be `>=`. |
| `playfab-migrate-suites` | 3 | Migrate PlayFab suites to GUT | `infra-vendor-gut`, `infra-shared-base` | Same shape for `tests\godot\playfab`. Records `tests\baselines\playfab.json`. |
| `gameinput-migrate-suites` | 3 | Migrate GameInput suites to GUT | `infra-vendor-gut`, `infra-shared-base` | Same shape for the GameInput coverage suite. Records `tests\baselines\gameinput.json`. |
| `infra-orchestrator` | 3 | `tools\run_all_tests.ps1` | `infra-doctest-target`, `infra-shared-base` | Pipeline: parse gate → cmake build (skippable) → C++ doctest exe → GUT runs for the three coverage hosts → bootstrap mini-runners → aggregate → write `run-summary.{json,md}`. Sets child env via `ProcessStartInfo.EnvironmentVariables` (no `--env-file`). Honors `-Live`, `-SkipBuild`, `-OutDir`. |
| `wave4-gdk-coverage` | 4 | Wave 4 — godot_gdk coverage expansion | `gdk-migrate-suites`, `infra-orchestrator` | `tests\godot\gdk\tests\test_multiplayer_activity.gd`, paired mini-runners under `tests\godot\gdk\tests\bootstrap\`, `tests\godot\gdk\tests\test_result_helpers.gd`, `tests\godot\gdk\tests\test_embed_dispatch.gd`, `tests\godot\gdk\tests\test_runtime_error_signals.gd`, and the test-only project setting `gdk/tests/live_required`. |
| `wave4-playfab-coverage` | 4 | Wave 4 — godot_playfab coverage expansion | `playfab-migrate-suites`, `infra-orchestrator` | `tests\godot\playfab\tests\test_game_saves_live.gd`, `tests\godot\playfab\tests\test_leaderboards_live.gd`, `tests\godot\playfab\tests\test_user_entity_key_live.gd`, `tests\godot\playfab\tests\test_validation_walk.gd`, `tests\godot\playfab\project.godot` (test-only `playfab/tests/custom_id`, `playfab/tests/leaderboard_settle_msec`). |
| `wave4-gameinput-coverage` | 4 | Wave 4 — godot_gameinput coverage expansion | `gameinput-migrate-suites`, `infra-orchestrator` | `GameInputReading` defaults / property-and-method shape; `GameInputDevice` defaults + soft-fail on accessors before init / after shutdown; `GameInputMapper` extensions (multiple mappers, `action_map` hot-swap, `target_device_id == -1` semantics); threading smoke (repeated `get_devices()` / `get_current_reading()` across many frames with no device); `gameinput_bootstrap.gd` autoload behavior via the same separate-mini-runner pattern. |
| `wave4-packaging-coverage` | 4 | Wave 4 — godot_gdk_packaging coverage expansion | `gdk-migrate-suites`, `infra-orchestrator` | Extract pure slide-navigation logic from `gdk_tutorial_wizard.gd` (`tutorial_wizard_state.gd`) and test it; `config_import_plugin.gd` against fixture XML files (success, malformed, missing fields); plugin lifecycle with a stubbed `EditorInterface` double (asserts `add_*` / `remove_*` pairs are balanced and idempotent); `gdk_toolchain.gd` edge cases (malformed/missing `GameDKCoreLatest` paths, path normalization). All run in `--headless` (see "Packaging editor-vs-headless decision"). (The originally orphaned `packaging_panel.gd` / `packaging_panel_logic.gd` and their `test_packaging_panel_logic.gd` helper test were later removed when the wizard was updated to the menu-driven workflow.) |
| `xcut-docs` | 5 | Update docs to describe new test pipeline | `infra-orchestrator`, `wave4-gdk-coverage`, `wave4-playfab-coverage`, `wave4-gameinput-coverage`, `wave4-packaging-coverage`, `cpp-result-tests`, `cpp-packaging-helpers` | `docs\gdk\sample-and-tests.md` and per-addon testing-guide entries documenting `tools\run_all_tests.ps1`, the `LIVE_TESTS` env var and project settings (`gdk/tests/live_required`, `playfab/tests/leaderboard_settle_msec`), eventual-consistency expectations for leaderboards, the GUT layout, the doctest target, the definition of green, and the sandbox-state cleanup pointer to `tools\reset_player_data.ps1`. |

The dependency graph (verbatim from the plan) is:

```
infra-spike                  → infra-spec-capture
infra-spec-capture           → infra-vendor-gut, infra-doctest-target
infra-vendor-gut             → infra-shared-base, gdk-migrate-suites,
                                playfab-migrate-suites,
                                gameinput-migrate-suites
infra-doctest-target         → cpp-result-codes-extract, cpp-packaging-helpers,
                                infra-orchestrator
infra-shared-base            → gdk-migrate-suites, playfab-migrate-suites,
                                gameinput-migrate-suites, infra-orchestrator
cpp-result-codes-extract     → cpp-result-tests
gdk-migrate-suites           → wave4-gdk-coverage, wave4-packaging-coverage
playfab-migrate-suites       → wave4-playfab-coverage
gameinput-migrate-suites     → wave4-gameinput-coverage
infra-orchestrator           → xcut-docs, wave4-gdk-coverage,
                                wave4-playfab-coverage, wave4-gameinput-coverage,
                                wave4-packaging-coverage
wave4-gdk-coverage           → xcut-docs
wave4-playfab-coverage       → xcut-docs
wave4-gameinput-coverage     → xcut-docs
wave4-packaging-coverage     → xcut-docs
cpp-result-tests             → xcut-docs
cpp-packaging-helpers        → xcut-docs
```

## Per-wave acceptance criteria

| Wave | Command(s) | Required exit behavior | Expected skip behavior | Parity artifact |
| --- | --- | --- | --- | --- |
| **-1 (spike)** | (spike's own commands; see `spike-report.md`) | `0` for happy path; `1` for the intentional-failure check. | None expected (the trivial test set has no live deps). | `spike-report.md` in the spike PR description recording verified GUT flags, packaging/editor decision, env propagation method. |
| **0** | `git diff --stat spec/` shows `spec\testing-strategy.md` added. | n/a | n/a | The spec exists and includes every section enumerated in this document. |
| **1** | `cmake --preset default && cmake --build build --preset debug && build\bin\Debug\gdk_unit_tests.exe` | `0`; GUT addon mirrors present in coverage hosts and absent from demo-only projects. | The exe runs an empty test suite cleanly (`Status: SUCCESS`). | Mirror tree output pasted in PR. |
| **2** | `cmake --build build --preset debug && build\bin\Debug\gdk_unit_tests.exe --reporters=console --no-colors` | `0`. | n/a (C++ doctest has no live deps). | Per-source-file assertion counts from doctest output. |
| **3** | `tools\run_all_tests.ps1` (default flags). | `0`; for each migrated host, asserted-test count `>=` the recorded pre-migration count. | Host-environment-dependent (no signed-in user, no PlayFab title id) tests show `pending`, never `fail`. | A per-migration `parity.json` file under `tests\baselines\` listing `{ host, pre_migration_assertions, post_migration_assertions }`. |
| **4 (each addon)** | `tools\run_all_tests.ps1` plus, where a live opt-in is implemented, `tools\run_all_tests.ps1 -Live` against the dedicated sandbox. | `0` for the default run on every machine; `0` for `-Live` runs on a configured workstation. | New live tests show `pending` in default runs. | The default run's `run-summary.md` plus, for live runs, the eventual-consistency wait times observed (logged as `info`). |
| **5** | `markdownlint docs/` (only if already configured; otherwise visual review). | n/a | n/a | The PR description links each updated doc section to the corresponding spec section. |

If a wave's acceptance criterion fails, the responsible agent's PR is held back; the wave does not advance with stragglers.

## Wave 1 surprises absorbed from spike

The Wave -1 spike surfaced seven behaviors that bind future waves. Each is a normative rule of this strategy:

- **Zero-test-discovery exits 0.** `gut_cmdln.gd` returns `exit 0` when discovery finds zero tests, including the canonical class-name-not-imported case. The orchestrator and any future runner asserts `Tests > 0` from GUT's own summary on every host run; the exit code alone is not sufficient.
- **`--headless --import` is required before the first GUT run per host.** Without it `gut_cmdln.gd` prints "Some GUT class_names have not been imported" and exits 0 without running anything. The orchestrator runs `& $godot --headless --import` once per coverage host before its first GUT invocation; this stage is idempotent and cheap.
- **`--editor` ignores `-s` script args.** `--editor --headless --quit-after N -s res://addons/gut/gut_cmdln.gd ...` does not execute the `-s` script in this Godot version. No orchestrator stage runs under `--editor`. Tests that genuinely require a live `EditorInterface` instance are documented as exceptions when (and only when) one appears.
- **Parent-shell `$env:VAR` does not propagate** into Godot launched via `Process.Start(psi)` with `UseShellExecute = $false`. The only reliable channel is `psi.EnvironmentVariables[name] = value`. Setting `$env:LIVE_TESTS = "1"` in the parent shell and trusting Godot to inherit it is forbidden.
- **Sync `ReadToEnd()` deadlocks** on Godot's stderr in clean trees. Missing GDExtensions flood stderr with `Condition "!FileAccess::exists(path)"` lines and fill the pipe buffer in seconds. Both stdout and stderr are drained asynchronously (`BeginOutputReadLine()` / `BeginErrorReadLine()` or concurrent `ReadToEndAsync()`).
- **The headless validator exclusion list is generalised** in Wave 1. Vendored or `extends GutTest` paths skip via `.gdignore` / sentinel marker file rather than a hard-coded array.
- **Version pinning matches `--version` output, not the file name.** `Godot_v4.6.1-stable_win64_console.exe` reports `4.6.2.stable.official.71f334935`. Discovery and version-pinning logic compares against the runtime `--version` string; the file name is unreliable as a version identifier and is not used in `run-summary.md`.
