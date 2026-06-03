---
description: Godot GDK addon architecture, async model, script conventions, and sample workflow
applyTo: "addons/godot_gdk/**, sample/gdk_demo/addons/godot_gdk/**, sample/gdk_demo/main.gd, sample/gdk_demo/main.tscn, sample/gdk_demo/MicrosoftGame.config, sample/gdk_demo/project.godot, sample/gdk_demo/sample_config.cfg.template, sample/gdk_demo/tests/**, sample/gdk_launch_point/project.godot, sample/multiplayer_pong/project.godot, sample/playfab_demo/project.godot, docs/godot-gdk-*.md, spec/gdext-gdk.md, tools/setup_sample.ps1"
---

# Godot GDK Addon Instructions

## Public Architecture

- `GDK` is the only engine singleton registered by the addon.
- Service surfaces such as `GDK.users` and `GDK.achievements` are `RefCounted` objects returned from the root singleton, not separate engine singletons.
- Script-visible wrapper types such as `GDKUser`, `GDKAchievement`, and `GDKResult` are part of the public Godot-facing contract.
- When adding new feature areas, prefer adding a service namespace under `GDK` instead of introducing new global singleton names.

## Async Model

- `GDKRuntime` owns one shared `XTaskQueue` with:
  - `ThreadPool` work dispatch
  - `Manual` completion dispatch
- `gdk/runtime/embed_dispatch` defaults to `true`. On Godot 4.5+ builds, the addon auto-pumps `GDK.dispatch()` from Godot's main thread each process frame via the engine frame callback path.
- On Godot 4.3/4.4 builds, auto-pumping is not available through `embed_dispatch`; games, samples, and tests must keep calling `GDK.dispatch()` manually each frame. Manual pumping is also the required path whenever `embed_dispatch` is disabled or deterministic control is needed. This pump still covers both `XAsync` completions and manager-driven state.
- One-shot public APIs return completion `Signal` values that callers await directly.
- Use `GDKPendingSignal` as the internal one-shot request helper for both `XAsyncBlock`-backed work and manager/event-driven waits.
- Immediate failures should still return an already-completed completion signal of the appropriate type.
- Update service caches and emit service signals before resolving the one-shot completion signal.

## XAsync Wrapping Rules

- `GDKSignalXAsyncContext` owns shared mechanics only: queue binding, lifetime, and best-effort cancellation plumbing.
- Do **not** treat `XAsyncGetStatus()` as a generic result-decoding layer.
- Concrete finalizers must call the operation-specific `*Result()` / `*ResultSize()` APIs required by the native GDK function.
- Keep result extraction in the concrete wrapper instead of pushing API-specific decoding into `GDKSignalXAsyncContext`.

## Xbox Services Scaffolding

- Shared Xbox services bootstrap belongs in `GDKXboxServices`.
- Default SCID is derived from `XGameGetXboxTitleId()` as a null GUID with the title id in the last 8 hex digits.
- Treat explicit SCID values as overrides for advanced scenarios; the normal path should use the derived current-title SCID.
- Reuse the shared Xbox services/context layer for achievements, stats, leaderboards, presence, and social instead of rebuilding per-service bootstrap code.

## C++ and Registration Conventions

- Every header that includes Windows or GDK APIs must define `WIN32_LEAN_AND_MEAN` and include `<windows.h>` before Godot or GDK headers.
- Register new native classes in `addons\godot_gdk\src\register_types.cpp`.
- Add new implementation files to `addons\godot_gdk\CMakeLists.txt`.
- When exposing object-returning properties from C++, set the `PropertyInfo` class name (for example `GDKUsers` or `GDKAchievements`) so Godot does not instantiate anonymous object defaults.

## Public API Documentation Contract

- Public Godot-facing classes in `godot_gdk` should have matching documentation under `addons\godot_gdk\doc_classes\`.
- When public methods, properties, signals, enums, or behavior change, update the relevant `doc_classes\*.xml` in the same change.
- The addon CMake already collects `doc_classes\*.xml` automatically for editor and debug-template builds; keep the XML set aligned with the script-visible surface instead of treating it as optional follow-up documentation.

## GDScript and Editor Script Rules

- In the GDK-owned `sample\` scripts and in `addons\godot_gdk\editor\`, avoid `:=` when the right-hand side comes from Variant-returning engine APIs. Prefer explicit `: Type = ...` or plain `=`.
- Avoid reserved identifiers such as `class_name` for local variables or loop variables.
- Keep GDScript public method names in snake_case to match Godot conventions.

## Sample, Docs, and Tooling Workflow

- The GDK-owned sample surfaces under `sample\` are part of the addon contract. Update them when public `godot_gdk` behavior changes.
- When public `godot_gdk` API or behavior changes, keep `doc_classes`, `docs\godot-gdk-*.md`, `spec\gdext-gdk.md`, the sample, and tests aligned in the same change.
- For `.gd` changes in the GDK-owned sample or editor/plugin scripts, run the repo headless validator:

```powershell
pwsh -NoLogo -NoProfile -ExecutionPolicy Bypass -File .\tools\check_gd_scripts_headless.ps1
```

- The headless test entry point for this addon is:

```powershell
cd sample/gdk_demo
godot --headless --script res://tests/run_tests.gd
```

- After changing synced addon files under `addons\godot_gdk\` (for example editor scripts or addon metadata), run:

```powershell
cmake --build build --preset debug
```

  so the `sample\gdk_demo\addons\godot_gdk\` copy is refreshed.
- Keep `docs\godot-gdk-*.md`, `spec\gdext-gdk.md`, and `tools\setup_sample.ps1` aligned with the current addon architecture and sample workflow when those surfaces change.
