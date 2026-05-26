# Dedicated Godot Test Hosts

> **Historical spec.** This document describes the dedicated
> test-host conversion that originally landed alongside the legacy
> sample projects (`gdk_demo`, `playfab_demo`, `gdk_launch_point`,
> `multiplayer_pong`). Those samples have since been removed; the
> tutorial-driven sample revamp (PR 3) will reintroduce
> `sample\tutorial_app\` and `sample\tutorial_gameinput\`. The
> `tests\godot\*` host contract documented below is unchanged and
> still authoritative — the goals/non-goals references to the
> historical samples are kept for context.

## Purpose

Automated Godot coverage must run from dedicated test projects instead of demo samples. Demo projects should remain examples for developers and manual scenarios, while `tests\godot\*` owns default automated GUT and bootstrap coverage.

## Goals

- Move default GUT coverage from `sample\gdk_demo`, `sample\playfab_demo`, and `sample\gdk_launch_point` to dedicated hosts under `tests\godot\`.
- Preserve or improve the existing default baselines: GDK 98 tests / 521 assertions / 12 pending, PlayFab 40 tests / 207 assertions / 11 pending, GameInput 25 tests / 167 assertions / 0 pending, GDK bootstrap 5 scripts, GameInput bootstrap 2 scripts, and C++ doctest result-code coverage.
- Keep Launch Point as a manual scenario sample with no default automated test directory, GUT mirror, shared-test-base mirror, baseline, or bootstrap runner after GameInput coverage moves.
- Make PlayFab default live sign-in coverage independent of a local Xbox user by adding custom-ID login.
- Keep docs, specs, baselines, CMake, and path-scoped instructions aligned with the runner.

## Non-goals

- Do not remove sample projects.
- Do not make `sample\multiplayer_pong` a GUT coverage host.
- Do not make `godot_gameinput` depend on `godot_gdk`.
- Do not run PlayFab live tests by default.
- Do not add CI in the host-conversion change; CI is a future track.

## Host layout

| Host | Owns |
| --- | --- |
| `tests\godot\gdk` | GDK GUT suites, GDK packaging GUT suites, and GDK bootstrap mini-runners |
| `tests\godot\playfab` | PlayFab GUT suites and PlayFab custom-ID sign-in coverage |
| `tests\godot\gameinput` | GameInput GUT suites and GameInput bootstrap mini-runners |

Each host is a standalone Godot project with its own `project.godot`, local `tests\` tree, mirrored addon files, mirrored vendored GUT, and mirrored shared test bases.

## Runner contract

`tools\run_all_tests.ps1` is the canonical default validation command. After the migration, its default GUT hosts must be exactly:

- `tests\godot\gdk`
- `tests\godot\playfab`
- `tests\godot\gameinput`

The runner must keep the explicit `Tests > 0` assertion for every GUT host because GUT can exit successfully when no tests are discovered. GUT discovery must include subdirectories.

Bootstrap mini-runners remain fresh Godot `--script` processes. They must not be converted into ordinary GUT tests because they validate startup-only project settings and autoload behavior.

## CMake contract

The root superproject distinguishes actual samples from coverage hosts:

- `GODOT_SAMPLE_DIRS` lists demo/sample projects.
- `GODOT_TEST_HOST_DIRS` lists dedicated test projects.
- Addon-local build wiring copies DLLs, runtime DLLs, and addon metadata to the project roots that need that addon.
- Vendored GUT and shared test bases are mirrored to coverage hosts, not to every sample.
- `GODOT_PLAYFAB_TEST_HOST_WITH_GDK` controls whether `godot_gdk` is mirrored into `tests\godot\playfab` for optional Xbox-backed PlayFab compatibility coverage. Default PlayFab coverage must still use custom-ID sign-in and remain valid when that option is disabled.

During migration, current sample coverage hosts may temporarily continue receiving GUT and shared test-base mirrors so the existing suites stay green while coverage moves. Once a suite has moved, remove that sample host from the mirror list and remove stale generated mirrors from the sample.

## PlayFab custom-ID login contract

PlayFab must expose custom-ID login so tests can sign in without an Xbox local user:

- `PlayFab.users.sign_in_with_custom_id_async(custom_id: String, create_account := true)`
- `PlayFab.users.get_user_by_custom_id(custom_id)`

`PlayFabUser` must represent both Xbox-backed and custom-ID-backed sessions. Xbox-backed sessions have a local user id, entity key, entity handle, and `PFLocalUserHandle`. Custom-ID sessions have a custom id, entity key, and entity handle, but no `PFLocalUserHandle`.

Game Saves currently requires `PFLocalUserHandle`; custom-ID-only users must fail with a clear high-level error such as `local_user_required` or `xbox_user_required`. Leaderboards operate from entity handles and should accept both Xbox-backed and custom-ID-backed users.

Live custom-ID tests should use a configured pre-existing custom id with `create_account=false`. Generated account creation with `create_account=true` mutates title state and must be limited to sandbox test titles.

## Migration order

1. Create dedicated host projects and CMake mirror infrastructure.
2. Move GameInput coverage from Launch Point to `tests\godot\gameinput`.
3. Move and split GDK suites from `sample\gdk_demo` to `tests\godot\gdk`.
4. Add PlayFab custom-ID login, then move and split PlayFab suites from `sample\playfab_demo` to `tests\godot\playfab`.
5. Consolidate shared test support only where repeated post-move code justifies it.
6. Update docs, specs, baselines, and instructions to match the final runner.
7. Run full local validation and record the new baselines.

## Future CI track

CI should be added after local host conversion is stable. The CI matrix should cover the current supported Godot version plus the two previous supported versions (`current`, `current - 1`, `current - 2`), bounded by the repository's minimum supported Godot version. Each matrix entry should use a pinned Godot console executable and record the executable path plus `--version` in the run summary.

CI should run the default non-live validation only: CMake configure/build, C++ doctest, and the default GUT hosts. It should not run `-Live` by default.

## Acceptance criteria

- Default validation no longer runs demo projects as GUT coverage hosts.
- Dedicated test hosts under `tests\godot\` are the default GUT hosts.
- Launch Point remains manual-only and has no automated test directory, GUT mirror, shared-test-base mirror, baseline, or bootstrap runner in the default pipeline.
- GameInput, GDK, PlayFab, and C++ doctest baselines are preserved or intentionally improved.
- PlayFab live sign-in coverage no longer requires a local Xbox user.
- Pending counts do not increase.
- Baseline files use the new host names and counts.
- Documentation and path-scoped instructions name the same hosts that the runner executes.
