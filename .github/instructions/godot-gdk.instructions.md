---
description: Godot GDK addon architecture, async model, script conventions, and sample workflow
applyTo: "addons/godot_gdk/**, sample/addons/godot_gdk/**, sample/gdk_bootstrap.gd, sample/main.gd, sample/main.tscn, sample/MicrosoftGame.config, sample/project.godot, sample/sample_config.cfg.template, sample/tests/**, docs/godot-gdk-*.md, spec/gdext-gdk.md, tools/setup_sample.ps1"
---

# Godot GDK Addon Instructions

## Public Architecture

- `GDK` is the only engine singleton registered by the addon.
- Service surfaces such as `GDK.users` and `GDK.achievements` are `RefCounted` objects returned from the root singleton, not separate engine singletons.
- Script-visible wrapper types such as `GDKUser`, `GDKAchievement`, `GDKResult`, `GDKAsyncOp`, and `GDKDispatchOp` are part of the public Godot-facing contract.
- When adding new feature areas, prefer adding a service namespace under `GDK` instead of introducing new global singleton names.

## Async Model

- `GDKRuntime` owns one shared `XTaskQueue` with:
  - `ThreadPool` work dispatch
  - `Manual` completion dispatch
- Games and the sample are expected to call `GDK.dispatch()` regularly. This pumps both `XAsync` completions and manager-driven state.
- Use `GDKAsyncOp` only for true `XAsyncBlock`-backed requests.
- Use `GDKDispatchOp` for manager/event-driven one-shot waits such as Achievements Manager and future Social Manager flows.
- Immediate failures should still return an already-completed op of the appropriate type.
- Update service caches and emit service signals before completing the one-shot op.

## XAsync Wrapping Rules

- `GDKXAsyncContext` owns shared mechanics only: queue binding, lifetime, and best-effort cancellation plumbing.
- Do **not** treat `XAsyncGetStatus()` as a generic result-decoding layer.
- Concrete finalizers must call the operation-specific `*Result()` / `*ResultSize()` APIs required by the native GDK function.
- Keep result extraction in the concrete wrapper instead of pushing API-specific decoding into `GDKXAsyncContext`.

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

## GDScript and Editor Script Rules

- In the GDK-owned `sample\` scripts and in `addons\godot_gdk\editor\`, avoid `:=` when the right-hand side comes from Variant-returning engine APIs. Prefer explicit `: Type = ...` or plain `=`.
- Avoid reserved identifiers such as `class_name` for local variables or loop variables.
- Keep GDScript public method names in snake_case to match Godot conventions.

## Sample, Docs, and Tooling Workflow

- The GDK-owned sample surfaces under `sample\` are part of the addon contract. Update them when public `godot_gdk` behavior changes.
- The headless test entry point for this addon is:

```powershell
cd sample
godot --headless --script res://tests/run_tests.gd
```

- After changing synced addon files under `addons\godot_gdk\` (for example editor scripts or addon metadata), run:

```powershell
cmake --build build --preset debug
```

  so the `sample\addons\godot_gdk\` copy is refreshed.
- Keep `docs\godot-gdk-*.md`, `spec\gdext-gdk.md`, and `tools\setup_sample.ps1` aligned with the current addon architecture and sample workflow when those surfaces change.
