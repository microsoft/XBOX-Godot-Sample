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

- `addons\godot_gdk\`, the GDK-owned sample files, `docs\godot-gdk-*.md`, `spec\gdext-gdk.md`, and `tools\setup_sample.ps1` are covered by `.github\instructions\godot-gdk.instructions.md`.
- `addons\godot_playfab\`, `tests\godot\playfab\`, the PlayFab addon copies synced into samples, `sample\playfab_demo\`, `docs\godot-playfab*.md`, and `spec\gdext-playfab.md` are covered by `.github\instructions\godot-playfab.instructions.md`.
- `addons\godot_gameinput\`, `tests\godot\gameinput\`, the GameInput-touching sample files (the pong logic scripts that pulse rumble or wire hot-plug), `docs\godot-gameinput*.md`, and `spec\gdext-gameinput.md` are covered by `.github\instructions\godot-gameinput.instructions.md`.
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
- Use the specific Godot project root when invoking Godot commands (`sample\gdk_demo`, `sample\gdk_launch_point`, `sample\multiplayer_pong`, `sample\playfab_demo`, or a host under `tests\godot\`).
- For end-to-end test coverage, run the repo-root orchestrator. It is the canonical way to validate "tests pass" across all addons:

```powershell
pwsh -NoLogo -NoProfile -ExecutionPolicy Bypass -File .\tools\run_all_tests.ps1
```

  The orchestrator runs (in order): the parse gate, `cmake --build --preset debug`, the C++ doctest exe (`gdk_unit_tests.exe`), GUT for each coverage host (`tests\godot\gdk`, `tests\godot\playfab`, `tests\godot\gameinput` — `multiplayer_pong` is intentionally **not** a test host), and the bootstrap mini-runners under each host's `tests\bootstrap\` directory. Per-stage results land in `build\test-results\run-summary.{json,md}`. Use `-Live` to opt in to live-service tests (`LIVE_TESTS=1`), including tests that write online state; use a dedicated sandbox PlayFab title for live write coverage.
- New GUT suites live under each host's `tests\` directory as `test_*.gd` files (auto-discovered via `-gdir res://tests -ginclude_subdirs`) and should `extends` the appropriate shared base from `addons\godot_gdk_tests\` (`gdk_test_base.gd`, `playfab_test_base.gd`, or `gameinput_test_base.gd`). The bases live in `addons\godot_gdk\tests_support\bases\` and CMake mirrors them into each host. Vendored GUT under `tests_support\gut\` and the mirrored `addons\gut\` copies are intentionally untouched — never edit them; refresh from upstream instead.
- When public addon behavior changes, keep the corresponding docs, samples, tests, and path-scoped instructions aligned in the same change.
