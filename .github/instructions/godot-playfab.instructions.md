---
description: Godot PlayFab addon architecture, runtime model, and sample workflow
applyTo: "addons/godot_playfab/**, tests/godot/playfab/**, sample/tutorial_app/**, docs/playfab/**, spec/gdext-playfab.md"
---

# Godot PlayFab Addon Instructions

## Public Architecture

- `PlayFab` is the only engine singleton registered by this addon.
- Service surfaces such as `PlayFab.users`, `PlayFab.game_saves`, and `PlayFab.leaderboards` are `RefCounted` objects returned from the root singleton, not separate engine singletons.
- Script-visible wrapper types such as `PlayFabUser` and `PlayFabResult` are part of the public Godot-facing contract.
- When adding new feature areas, prefer adding a service namespace under `PlayFab` instead of introducing additional global singleton names.

## Runtime and Async Model

- `PlayFabRuntime` owns the shared PlayFab bootstrap: `XGameRuntimeInitialize`, `PFInitialize`, `PFServicesInitialize`, `PFGameSaveFilesInitialize`, and the shared `XTaskQueue`.
- `playfab/runtime/embed_dispatch` defaults to `true`. On Godot 4.5+ builds, the addon auto-pumps `PlayFab.dispatch()` from Godot's main thread each process frame via the extension frame callback path.
- One-shot public APIs return completion `Signal` values awaited directly.
- Completion payloads are always delivered through `PlayFabResult`.
- Immediate failures should still return an already-completed completion signal instead of failing silently or returning inconsistent shapes.

## User and Service Model

- PlayFab sign-in is an explicit gameplay action keyed by a `GDKUser` object or a title-defined custom id; do not expose raw local Xbox user ids for XUser-backed sign-in.
- `PlayFabUser` represents one signed-in PlayFab session associated with either one Xbox-backed user object flow or one custom id.
- Game Saves requires an Xbox-backed `PlayFabUser` with a local user handle; custom-ID sessions must fail with `xbox_user_required` instead of surfacing a low-level handle error.
- Higher-level service calls should require a `PlayFabUser` rather than raw ids or loosely typed user variants whenever the session is required.
- Keep Xbox-facing identity details on the GDK side; the PlayFab wrapper should expose the PlayFab-facing session data higher-level services need.

## Project Settings and Registration

- Runtime configuration belongs in Project Settings keys:
  - `playfab/runtime/title_id`
  - `playfab/runtime/endpoint`
  - `playfab/runtime/embed_dispatch`
- Register new public classes in `addons\godot_playfab\src\register_types.cpp`.
- Add new implementation files to `addons\godot_playfab\CMakeLists.txt`.
- When exposing object-returning properties from C++, set the `PropertyInfo` class name so Godot does not instantiate anonymous object defaults.
- Preserve idempotent registration behavior when the same DLL is reachable through synced addon copies in multiple samples.

## Sample and Workflow

- No PlayFab sample currently ships in `sample/`. The legacy
  `sample\playfab_demo\` (canonical PlayFab smoke-test sample,
  depending on the GDK sample bootstrap for Xbox runtime
  initialization and user sign-in) has been removed; the
  tutorial-driven sample revamp's PR 3 will reintroduce PlayFab
  coverage inside `sample/tutorial_app/` (T1 sign-in autoload
  + per-tutorial scenes for Game Saves, leaderboards, lobby,
  Party).
- PlayFab GUT suites live under `tests\godot\playfab\tests\` and `extends "res://addons/godot_gdk_tests/playfab_test_base.gd"` (the base is at `addons\godot_gdk\tests_support\bases\playfab_test_base.gd` and is mirrored into the host by CMake). Use custom-ID helpers for default PlayFab sign-in coverage; reserve `ensure_gdk_primary_user_for_playfab()` for optional Xbox-backed compatibility flows. The root CMake option `GODOT_PLAYFAB_TEST_HOST_WITH_GDK` controls whether `godot_gdk` is mirrored into the PlayFab host for those optional flows.
- When public `godot_playfab` behavior changes, update the docs, spec, and tests in the same change rather than leaving automation follow-up for later. (Sample updates resume once PR 3 lands `sample/tutorial_app/`.)
- After changing synced addon files under `addons\godot_playfab\`, run:

```powershell
cmake --build build --preset debug
```

  so the test host addon copies are refreshed.
- For `.gd` changes in the PlayFab synced addon scripts, run the repo headless validator:

```powershell
pwsh -NoLogo -NoProfile -ExecutionPolicy Bypass -File .\tools\check_gd_scripts_headless.ps1
```

- The headless test entry point for this addon is the repo-root orchestrator:

```powershell
pwsh -NoLogo -NoProfile -ExecutionPolicy Bypass -File .\tools\run_all_tests.ps1
```

  PlayFab tests run in the `gut:tests/godot/playfab` stage. To iterate on the PlayFab host alone:

```powershell
cd tests\godot\playfab
..\..\..\Godot_v4.6.1-stable_win64_console.exe --headless -s res://addons/gut/gut_cmdln.gd -gdir=res://tests -ginclude_subdirs -gexit
```

- Live PlayFab coverage is opt-in. Pass `-Live` to the orchestrator to expose `LIVE_TESTS=1` for tests that talk to PlayFab, including tests that write online state. Prefer `-PlayFabTitleId "<title-id>" -PlayFabCustomId "<existing-custom-id>"` for live custom-ID tests; the runner forwards those values as `PLAYFAB_TITLE_ID` and `PLAYFAB_CUSTOM_ID` only to Godot child processes. Custom-ID live sign-in uses `create_account=false`, so the account must already exist. Use a dedicated sandbox PlayFab title for live write coverage and never point live write tests at a shared or production title.

## Documentation Contract

- Public Godot-facing class docs live in `addons\godot_playfab\doc_classes\`.
- `docs\playfab\plugin.md` is the user-facing addon overview. Update it when the public surface or sample workflow changes.
- `spec\gdext-playfab.md` is the source of truth for design direction and deferred work. Mark shipped sections or note deviations there when scope changes.

## Build / Binding Gotchas

Lessons that have cost rework in past sessions. Apply them as starting assumptions:

- **No `Ref<GDKUser>` in PlayFab public bindings.** `godot_playfab` and `godot_gdk` ship as separate GDExtension DLLs and cannot exchange typed `Ref<>` values directly. Public PlayFab methods that need a GDK user accept `Object *` and duck-type via `has_method` / `call`. See `addons\godot_playfab\src\playfab_users.h` and `playfab_users.cpp` for the established pattern.
- **PlayFab `sign_in_with_xuser_async` accepts a `GDKUser` object only — never a raw local Xbox user id.** When the supplied user object is null or invalid the call returns `invalid_xuser`; do not re-introduce a `local_id` parameter.
- **Party SDK typedefs (`PartyError`, `PartyString`, `PartyBool`) live in the global namespace.** They are defined in `<playfab/party/PartyTypes.h>`, not inside `namespace Party`. Writing `Party::PartyError` will not compile.
- **`<playfab/party/PartyImpl.h>` must be included in exactly one `.cpp`.** Party.lib only exports the C interface; the C++ wrappers (`Party::PartyManager`, …) require `PartyImpl.h` for inline definitions. The current home is `addons\godot_playfab\src\playfab_party.cpp`; do not duplicate the include elsewhere.
- **PlayFab Lobby `search_properties` must use service keys.** Use `string_key1` … `string_keyN` and `number_key1` … `number_keyN`. Custom key names cause live create/search to fail. See `docs\playfab\plugin.md` and `spec\gdext-playfab-lobby-matchmaking.md` for the canonical list.
- **Lobby constants/mutators live on `PlayFabLobby`. Match-ticket constants and ticket ops live on `PlayFabMatchTicket`.** Do not put either set on `PlayFabMultiplayer` — that surface stays focused on factory/service-level entry points. This was corrected multiple times in PR #109 and is now the established convention.
