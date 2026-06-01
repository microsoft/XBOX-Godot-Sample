# Godot Microsoft GDK plugin

This is the landing page for the `godot_gdk` docs set.

`godot_gdk` is the primary GDExtension addon in this repository. It implements a public Microsoft GDK service surface — runtime, users, achievements, presence, social, profile, privacy, multiplayer activity, stats, leaderboards, title storage, string verification, package metadata + DLC content access, XStore commerce, GameUI, accessibility, capture, launcher, error reporting, and system metadata — and also ships editor-side setup and export tooling for the broader Microsoft GDK workflow.

## Current implementation status

### Implemented now

- root singleton registration through `GDK`
- shared Microsoft GDK runtime lifecycle
- shared async bridge
- shared XBOX services scaffold
- users service (`GDK.users`)
- system / runtime metadata service (`GDK.system`)
- game UI service (`GDK.game_ui`)
- accessibility service (`GDK.accessibility`)
- achievements service (`GDK.achievements`)
- package metadata + DLC content service (`GDK.package`)
- stats service (`GDK.stats`)
- leaderboards service (`GDK.leaderboards`)
- privacy service (`GDK.privacy`)
- presence service (`GDK.presence`)
- social graph service (`GDK.social`)
- profile service (`GDK.profile`)
- string verification service (`GDK.string_verify`)
- title storage service (`GDK.title_storage`)
- error reporting service (`GDK.error_reporting`)
- multiplayer activity service (`GDK.multiplayer_activity`)
- capture metadata + capture-state service (`GDK.capture`)
- launcher service (`GDK.launcher`) — `XLaunchUri` only
- display service (`GDK.display`) — `XDisplay.h` HDR mode + idle-timeout deferrals
- activation service (`GDK.activation`) — `XGameActivation.h` activation events (modern replacement for the deprecated `XGameProtocol.h`)
- XStore commerce service (`GDK.store`)
- dispatch-backed manager wait ops
- sample bootstrap for dispatch
- sample demo for runtime/users/achievements/presence/social and the launch-point scenario shell
- GUT coverage under `tests\godot\gdk\tests\`
- editor setup/export scripts shipped with the addon

### Not implemented yet in native runtime

- Game Saves are intentionally not part of `godot_gdk`; they live in
  `godot_playfab` under `PlayFab.game_saves` because the PlayFab Game Saves
  C API drives the XBOX-backed save flow.
- Server / admin / private Microsoft GDK surfaces remain out of scope for the public
  PC client wrappers.

## Testing this addon

`godot_gdk` is exercised by the `tests\godot\gdk\` host. Coverage lives under `tests\godot\gdk\tests\` and includes files such as `test_core.gd`, `test_users.gd`, `test_achievements.gd`, `test_presence.gd`, `test_social.gd`, `test_multiplayer_activity.gd`, `test_system.gd`, `test_result_helpers.gd`, and `test_embed_dispatch.gd`. Startup-only behavior is covered by `tests\godot\gdk\tests\bootstrap\run_*.gd`, and packaging/editor-helper logic is covered under `tests\godot\gdk\tests\packaging\`.

Most deterministic coverage runs in the default orchestrator pass. Live Microsoft GDK flows are gated by `LIVE_TESTS=1` through `-Live`; any live write coverage should use a test sandbox.

Run the standard pipeline from the repository root:

```powershell
pwsh -NoLogo -NoProfile -ExecutionPolicy Bypass -File .\tools\run_all_tests.ps1
```

See [`gdk/sample-and-tests.md`](sample-and-tests.md) for the orchestrator stages, GUT layout, live switch, baselines, and troubleshooting pointers.

## Document map

### User guides

- [`getting-started.md`](../getting-started.md)  
  Adding the addons to your project, configuring project settings,
  getting to user sign-in (XBOX + PlayFab), and building from source.

- [`gdk/sample-setup.md`](sample-setup.md)  
  Partner Center configuration, sandbox setup, test accounts, and configuration flow.

- [`gdk/api-reference.md`](api-reference.md)  
  Public GDScript API surface for `GDK`, `GDK.system`, `GDK.users`,
  `GDK.game_ui`, `GDK.accessibility`, `GDK.achievements`, `GDK.package`,
  `GDK.stats`, `GDK.leaderboards`, `GDK.privacy`, `GDK.presence`,
  `GDK.social`, `GDK.profile`, `GDK.string_verify`, `GDK.title_storage`,
  `GDK.error_reporting`, `GDK.multiplayer_activity`, `GDK.capture`,
  `GDK.launcher`, `GDK.display`, `GDK.activation`, and `GDK.store`.

### Architecture

- [`gdk/build-and-loading.md`](build-and-loading.md)  
  How the addon is laid out, built, packaged, and loaded by Godot.

- [`gdk/native-runtime.md`](native-runtime.md)  
  The native runtime architecture: `GDK`, `GDKRuntime`, async wrappers, users service, and extension points.

- [`gdk/editor-tools.md`](editor-tools.md)  
  The editor plugin, setup panel, and export platform.

- [`gdk/sample-and-tests.md`](sample-and-tests.md)  
  Sample roles and the repo-wide test pipeline.

### Subsystem deep dive

- [`gdk/async-system.md`](async-system.md)  
  Lower-level explanation of the shared async bridge and how the Microsoft GDK service
  wrappers participate in it.

### Reference

- [`troubleshooting.md`](../troubleshooting.md)  
  Common build, runtime, and test issues.

## Recommended reading order

1. Start with [`getting-started.md`](../getting-started.md) to build and run.
2. Read this page for current scope.
3. Read [`gdk/api-reference.md`](api-reference.md) for the public API.
4. Read [`gdk/build-and-loading.md`](build-and-loading.md) for the addon lifecycle.
5. Read [`gdk/native-runtime.md`](native-runtime.md) for the current runtime architecture.
6. Use [`gdk/async-system.md`](async-system.md) when you need the lower-level async mechanics.
7. Read [`gdk/editor-tools.md`](editor-tools.md) and [`gdk/sample-and-tests.md`](sample-and-tests.md) when working on tooling or validation.
