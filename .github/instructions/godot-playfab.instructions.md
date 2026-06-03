---
description: Godot PlayFab addon architecture, runtime model, and sample workflow
applyTo: "addons/godot_playfab/**, sample/playfab_demo/**, sample/gdk_demo/addons/godot_playfab/**, sample/gdk_launch_point/addons/godot_playfab/**, sample/multiplayer_pong/addons/godot_playfab/**, docs/godot-playfab*.md, spec/gdext-playfab.md"
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

- PlayFab sign-in is an explicit gameplay action keyed by a local Xbox user id or a `GDKUser`.
- `PlayFabUser` represents one signed-in PlayFab session associated with one local Xbox user id.
- Higher-level service calls should require a `PlayFabUser` rather than raw ids or loosely typed user variants whenever the session is required.
- Keep Xbox-facing identity details on the GDK side; the PlayFab wrapper should expose the PlayFab-facing session data higher-level services need.

## Project Settings and Registration

- Runtime configuration belongs in Project Settings keys:
  - `playfab/titleid`
  - `playfab/endpoint`
  - `playfab/runtime/embed_dispatch`
- Register new public classes in `addons\godot_playfab\src\register_types.cpp`.
- Add new implementation files to `addons\godot_playfab\CMakeLists.txt`.
- When exposing object-returning properties from C++, set the `PropertyInfo` class name so Godot does not instantiate anonymous object defaults.
- Preserve idempotent registration behavior when the same DLL is reachable through synced addon copies in multiple samples.

## Sample and Workflow

- `sample\playfab_demo\` is the canonical PlayFab smoke-test sample.
- The PlayFab demo depends on the GDK sample bootstrap to provide Xbox runtime initialization and user sign-in before PlayFab sign-in.
- `sample\playfab_demo\tests\run_tests.gd` is the PlayFab headless contract suite.
- When public `godot_playfab` behavior changes, update the sample, docs, spec, and tests in the same change rather than leaving automation follow-up for later.
- After changing synced addon files under `addons\godot_playfab\`, run:

```powershell
cmake --build build --preset debug
```

  so the sample addon copies are refreshed.
- For `.gd` changes in the PlayFab sample or synced addon scripts, run the repo headless validator:

```powershell
pwsh -NoLogo -NoProfile -ExecutionPolicy Bypass -File .\tools\check_gd_scripts_headless.ps1
```

- The headless test entry point for this addon is:

```powershell
cd sample/playfab_demo
godot --headless --script res://tests/run_tests.gd
```

## Documentation Contract

- Public Godot-facing class docs live in `addons\godot_playfab\doc_classes\`.
- `docs\godot-playfab-plugin.md` is the user-facing addon overview. Update it when the public surface or sample workflow changes.
- `spec\gdext-playfab.md` is the source of truth for design direction and deferred work. Mark shipped sections or note deviations there when scope changes.
