# Copilot Instructions — GodotGDK Repo

## Repository Overview

GodotGDK is a repository of Godot 4.x GDExtension addons for Windows gaming integrations around the **Microsoft public GDK**.

Current addon targets:

- `addons\godot_gdk` — GDK runtime and Xbox services integration
- `addons\godot_playfab` — PlayFab runtime, users, Game Saves, and leaderboards
- `addons\godot_gameinput` — GameInput integration
- `addons\godot_gdk_packaging` — GDScript editor tooling for PC packaging

Each addon owns its own `CMakeLists.txt`. The root `CMakeLists.txt` is a thin superproject that wires the addon-local targets together.

## Build Commands

```powershell
# Configure the default repo build
cmake --preset default

# Build debug
cmake --build build --preset debug

# Build release
cmake --build build --preset release

# Optional: build only the GameInput addon
cmake --preset gameinput-only
cmake --build --preset debug-gameinput
```

Output binaries land in each addon's `bin\` folder. Addon-local build logic may also sync files into `sample\addons\...\`.

## Repo-Wide Conventions

- Keep addon-specific architecture, API, and workflow guidance in path-scoped instruction files under `.github\instructions\`.
- Do **not** assume `godot_gdk` conventions automatically apply to `godot_gameinput`, or vice versa.
- Treat `godot_gdk` and `godot_playfab` as **service/runtime addons**: prefer one root singleton with typed service and wrapper surfaces beneath it.
- Treat `godot_gameinput` as an **input integration addon**: optimize for compatibility with Godot's `Input` / `InputMap` flow and additive device functionality rather than forcing it into the same public API shape as the service/runtime addons.
- Treat `godot_gdk_packaging` as **editor tooling**, not as a runtime service addon.
- When behavior changes, update the related addon-local docs, sample content, and tooling that define or demonstrate that behavior.
- Prefer narrow, addon-local changes over repo-wide edits unless the task truly spans multiple addons.

## Path-Scoped Instructions

- `addons\godot_gdk\`, the GDK-owned sample files, `docs\gdk\`, `spec\gdext-gdk.md`, and `tools\setup_sample.ps1` are covered by `.github\instructions\godot-gdk.instructions.md`.
- `addons\godot_playfab\`, `tests\godot\playfab\`, `docs\playfab\`, and `spec\gdext-playfab.md` are covered by `.github\instructions\godot-playfab.instructions.md`. (Sample-path coverage will return when PR 3 of the tutorial-driven sample revamp adds `sample\tutorial_app\` and `sample\tutorial_gameinput\`.)
- `addons\godot_gameinput\`, `tests\godot\gameinput\`, the GameInput-touching sample files (the pong logic scripts that pulse rumble or wire hot-plug), `docs\gameinput\`, and `spec\gdext-gameinput.md` are covered by `.github\instructions\godot-gameinput.instructions.md`.
- If future addon-specific guidance is needed, add another scoped instruction file instead of expanding this top-level file with rules that only apply to one addon.

## Workflow Quality Gates

- Use repo-local skills when they match the task:
  - `adversarial-review` for risky or cross-cutting diffs that need pressure testing
  - `pr-feedback-loop` when the user mentions PR feedback, review comments, or check results
  - `gdextension-hygiene` for a finish pass that checks validation, documentation, sample/test sync, and build wiring
- When the task touches `.gd` files anywhere in the repo, run the repo-managed headless validator:

```powershell
pwsh -NoLogo -NoProfile -ExecutionPolicy Bypass -File .\tools\check_gd_scripts_headless.ps1
```

- Do not assume GitHub checks exercised the relevant local build, sample, or validation paths; run the matching local validation yourself.
- Use the specific Godot project root when invoking Godot commands (a host under `tests\godot\`, or whichever project you are running against — the legacy `sample\gdk_demo`, `sample\gdk_launch_point`, `sample\multiplayer_pong`, and `sample\playfab_demo` projects have been removed and will be replaced by the tutorial-driven samples in PR 3).
- For end-to-end test coverage, run the repo-root orchestrator. It is the canonical way to validate "tests pass" across all addons:

```powershell
pwsh -NoLogo -NoProfile -ExecutionPolicy Bypass -File .\tools\run_all_tests.ps1
```

  The orchestrator runs (in order): the parse gate, `cmake --build --preset debug`, the C++ doctest exe (`gdk_unit_tests.exe`), GUT for each coverage host (`tests\godot\gdk`, `tests\godot\playfab`, `tests\godot\gameinput`), and the bootstrap mini-runners under each host's `tests\bootstrap\` directory. Per-stage results land in `build\test-results\run-summary.{json,md}`. Use `-Live` to opt in to live-service tests (`LIVE_TESTS=1`). Writes that persist in the live title (create lobby, post leaderboard entry, save Game Save, …) will be gated behind a separate forthcoming `-AllowLiveWrites` switch (`LIVE_WRITE_TESTS=1`) added in the companion tooling PR; until that switch lands, treat any live write coverage as a conscious sandbox-only decision and never point live write tests at a shared or production title. See `tests\godot\README.md` for the test-tier contract.
- New GUT suites live under each host's `tests\` directory as `test_*.gd` files (auto-discovered via `-gdir res://tests -ginclude_subdirs`) and should `extends` the appropriate shared base from `addons\godot_gdk_tests\` (`gdk_test_base.gd`, `playfab_test_base.gd`, or `gameinput_test_base.gd`). The bases live in `addons\godot_gdk\tests_support\bases\` and CMake mirrors them into each host. Vendored GUT under `tests_support\gut\` and the mirrored `addons\gut\` copies are intentionally untouched — never edit them; refresh from upstream instead.
- When public addon behavior changes, keep the corresponding docs, samples, tests, and path-scoped instructions aligned in the same change.

## Before Reporting Completion

A task is not done until the following are satisfied. Walk through this checklist explicitly before claiming completion or opening a PR:

- **Parse gate ran.** If any `.gd` file was touched (anywhere in the repo, including synced sample copies under `sample\<host>\addons\`), run `tools\check_gd_scripts_headless.ps1` and confirm it exits clean. If no `.gd` was touched, say so out loud — do not skip silently.
- **Test orchestrator ran or was intentionally narrowed.** The canonical green bar is `tools\run_all_tests.ps1`. Running a narrower subset (single GUT host, single bootstrap script, `cmake --build` only) is allowed, but state which subset and why. Never report "tests pass" based on GitHub Actions output alone.
- **Live coverage decision is explicit.** State whether live tests were skipped (default) or run (`-Live`, with a sandbox title id named). For tests that write online state, state whether the (forthcoming) `-AllowLiveWrites` switch was used; until the companion tooling PR adds that switch, any live write coverage is a conscious sandbox-only decision and must be called out in the PR description. See `tests\godot\README.md` for the test-tier contract.
- **Public-API drift was reconciled in the same change.** When public addon behavior changed, the matching `doc_classes\*.xml`, `spec\gdext-*.md`, `docs\<addon>\*.md`, sample content, and tests were updated in this change — not deferred to follow-up.
- **PR description carries usage examples.** When public API was added or renamed, the PR description (or the change commit body if no PR yet) shows a short GDScript usage snippet for each new or renamed surface. Reviewers should not have to reverse-engineer the new API from the diff.

## Worktree Lifecycle

This repo regularly uses multiple worktrees for parallel feature branches (`.copilot-worktrees/<branch>` and `R:\repos\godot-public-gdk-ext-<feature>` are both seen in practice). Treat each worktree as expendable:

- **Name the worktree after its feature branch.** `R:\repos\godot-public-gdk-ext-<feature>` is preferred for human-driven work; `.copilot-worktrees/<branch>` is acceptable for agent-driven slices.
- **Rebase or merge `origin/main` before "make the PR".** A worktree that has drifted from main is not ready to be reviewed. Run `git fetch origin && git merge --ff-only origin/main` (or `git rebase origin/main`) before opening or refreshing a PR — do not ask the user to retry merges.
- **Delete the worktree after its PR merges.** Once the corresponding PR is merged into `origin/main` (or explicitly abandoned), remove the worktree with `git worktree remove --force <path>` and `git worktree prune`. Stale worktrees serve as a foothold for outdated `.github/copilot-instructions.md` content and confuse later sessions.
- **A standalone helper to list stale worktrees** (`tools\list_stale_worktrees.ps1`) is forthcoming in the companion tooling PR. Until it lands, run `git worktree list` manually and check whether each worktree's branch is fully merged into `origin/main` before pruning.

## Long-Lived Feature Plans

For multi-session work (multi-week features such as PlayFab Multiplayer, PlayFab Party, GDK packaging tooling), do **not** rely on `~\.copilot\session-state\<id>\plan.md` as the persistent plan — those files are scoped to a single session and the next session cannot find them.

Instead, extend the existing `spec\gdext-<feature>.md` with two checked-in sections:

- **`## Plan`** — the phased execution plan that survives across sessions. Each phase calls out the public API surface it lands, the test tier it targets, and any sample/doc updates it must carry.
- **`## Progress`** — the running ledger of what has shipped (`Phase 1: ✅ shipped in #109`), what is in-flight, and what is intentionally deferred. New sessions read this section first to know where to pick up.

Session-state `plan.md` remains the right place for one-off tasks (single PR scope, a few hours of work). It is **not** the right place for anything you expect to outlive the session.

Do not introduce a parallel `docs\plans\` tree. The spec is the single source of truth for both design and progress.

## Anti-Patterns

Recurring detours observed in past sessions. Recognize them early and steer back to the established conventions instead:

- **No code-generation or manifest pipelines for binding surfaces.** Hand-written C++ bindings and the GDScript test matrix (`tests\godot\<addon>\tests\test_api_services.gd` and siblings) are the source of truth. Do not introduce JSON manifests, generators, or "generated/" directories — past attempts have all been removed.
- **No new global engine singletons.** Each addon registers exactly one root singleton (`GDK`, `PlayFab`, `GameInput`). New feature areas attach as service namespaces under the existing singleton (`GDK.users`, `PlayFab.party`, …), not as new top-level names.
- **No `Ref<>` parameters across addon DLL boundaries.** `godot_playfab` cannot accept a `Ref<GDKUser>` directly because `godot_gdk` and `godot_playfab` are separate GDExtension DLLs. Use `Object *` and duck-type via `has_method` / `call`. Same rule for any future cross-addon API.
- **No silent "tests pass" claims after GitHub Actions runs.** Local validation must be run explicitly per the **Before Reporting Completion** checklist.
- **No raw local Xbox user ids in PlayFab sign-in.** `PlayFab.users.sign_in_with_xuser_async` accepts a `GDKUser` object only. Plumbing local handles through the higher-level API is a step backwards — keep Xbox-facing identity inside the GDK side.
