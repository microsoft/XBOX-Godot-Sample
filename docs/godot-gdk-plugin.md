# Godot GDK plugin

This is the landing page for the `godot_gdk` docs set.

`godot_gdk` is the primary GDExtension addon in this repository. It currently implements the new runtime/users/achievements baseline and also ships editor-side setup and export tooling for the broader GDK workflow.

## Current implementation status

### Implemented now

- root singleton registration through `GDK`
- shared GDK runtime lifecycle
- shared async bridge
- shared Xbox services scaffold
- users service
- achievements service
- dispatch-backed manager wait ops
- sample bootstrap for dispatch
- sample demo for runtime/users/achievements
- headless runtime/users/achievements tests
- editor setup/export scripts shipped with the addon

### Not implemented yet in native runtime

- save service
- stats service
- leaderboards service
- presence service
- social service

## Document map

### Plugin overview

- [`godot-gdk-build-and-loading.md`](godot-gdk-build-and-loading.md)  
  How the addon is laid out, built, packaged, and loaded by Godot.

- [`godot-gdk-native-runtime.md`](godot-gdk-native-runtime.md)  
  The native runtime architecture: `GDK`, `GDKRuntime`, async wrappers, users service, and extension points.

- [`godot-gdk-editor-tools.md`](godot-gdk-editor-tools.md)  
  The editor plugin, setup panel, and export platform.

- [`godot-gdk-sample-and-tests.md`](godot-gdk-sample-and-tests.md)  
  How the sample project uses the plugin and what the headless test harness verifies.

### Subsystem deep dive

- [`godot-gdk-async-system.md`](godot-gdk-async-system.md)  
  Lower-level explanation of the shared async bridge and its current runtime/users/achievements implementation.

## Recommended reading order

1. Start here for scope.
2. Read [`godot-gdk-build-and-loading.md`](godot-gdk-build-and-loading.md) for the addon lifecycle.
3. Read [`godot-gdk-native-runtime.md`](godot-gdk-native-runtime.md) for the current runtime architecture.
4. Use [`godot-gdk-async-system.md`](godot-gdk-async-system.md) when you need the lower-level async mechanics.
5. Read [`godot-gdk-editor-tools.md`](godot-gdk-editor-tools.md) and [`godot-gdk-sample-and-tests.md`](godot-gdk-sample-and-tests.md) when working on tooling or validation.
