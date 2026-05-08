# Godot GDK build and loading

This document explains how the `godot_gdk` addon is laid out, built, packaged, and loaded by Godot.

See also:

- [`godot-gdk-plugin.md`](godot-gdk-plugin.md)
- [`godot-gdk-native-runtime.md`](godot-gdk-native-runtime.md)

## Repository and addon layout

At the repository level, `godot_gdk` is one addon inside a repo that also
contains:

- `godot_playfab` — PlayFab runtime/services addon
- `godot_gameinput` — GameInput integration addon
- `godot_gdk_packaging` — editor-side packaging tooling

### Native addon files

`addons\godot_gdk\src\` currently contains:

- `gdk.cpp` / `gdk.h` — root singleton
- `gdk_runtime.cpp` / `gdk_runtime.h` — shared GDK runtime and queue owner
- `gdk_result.cpp` / `gdk_result.h` — normalized result wrapper
- `gdk_pending_signal.cpp` / `gdk_pending_signal.h` — retained one-shot completion signal helper
- `gdk_signal_xasync_context.cpp` / `gdk_signal_xasync_context.h` — reusable `XAsyncBlock` bridge base for signal-returning requests
- `gdk_xbox_services.cpp` / `gdk_xbox_services.h` — shared Xbox services bootstrap and context cache
- `gdk_user.cpp` / `gdk_user.h` — users service and user wrapper
- `gdk_achievement.cpp` / `gdk_achievement.h` — achievements service and achievement wrapper
- `gdk_presence.cpp` / `gdk_presence.h` — presence service and wrapper types
- `gdk_social.cpp` / `gdk_social.h` — social graph service and wrapper types
- `gdk_multiplayer_activity.cpp` / `gdk_multiplayer_activity.h` — multiplayer activity service and wrapper types
- `gdk_system.cpp` / `gdk_system.h` — system/runtime metadata service
- `register_types.cpp` / `register_types.h` — Godot class registration and singleton publication

### Addon metadata

- `plugin.cfg` registers the editor plugin script
- `godot_gdk.gdextension` points Godot at the built DLL

### Runtime bootstrap

- `runtime\gdk_bootstrap.gd`

The bootstrap is the addon-owned autoload surface for projects that want
startup automation. It reads `gdk/runtime/initialize_on_startup` and
`gdk/runtime/auto_add_primary_user` from Project Settings so different samples
can share one script while still choosing automatic or manual startup.

### Editor-side scripts

- `editor\gdk_editor_plugin.gd`
- `editor\gdk_export_platform.gd`
- `editor\gdk_setup_panel.gd`

These files are still shipped and synced, but the repo's active packaging UI now
lives in the separate `godot_gdk_packaging` addon. The current
`gdk_editor_plugin.gd` no longer registers the previous custom export platform.

### Sample project

- `sample\gdk_demo\project.godot`
- `addons\godot_gdk\runtime\gdk_bootstrap.gd`
- `sample\gdk_demo\main.gd`
- `tests\godot\gdk\tests\`

## Build and packaging flow

The native addon is built by `addons\godot_gdk\CMakeLists.txt`.

That target currently:

1. detects the public GDK Windows layout and dependent runtime DLLs
2. builds `godot_gdk` as a shared library
3. links against:
   - `godot::cpp`
   - `xgameruntime`
   - XSAPI thunks
   - `libHttpClient`
4. copies runtime DLL dependencies into the addon output
5. syncs addon metadata, the runtime bootstrap, and editor scripts into the sample projects listed by the root CMake configuration

The effective runtime artifact chain is:

```text
native C++ sources
  -> godot_gdk.windows.<config>.x86_64.dll
  -> addons/godot_gdk/bin/
  -> sample/*/addons/godot_gdk/bin/
```

## Runtime loading path

Godot loads the native library through:

- `addons\godot_gdk\godot_gdk.gdextension`

That file maps platform/config combinations to the built DLL path inside `addons\godot_gdk\bin\`.

## Editor loading path

Godot loads the editor plugin through:

- `addons\godot_gdk\plugin.cfg`

That file points to:

- `editor\gdk_editor_plugin.gd`

So the addon has two Godot-facing entry points:

- a **runtime** path through GDExtension
- an **editor** path through the normal editor plugin system

The editor plugin also keeps the `GDKBootstrap` autoload pointed at
`runtime\gdk_bootstrap.gd`, which lets projects opt into automatic startup
through Project Settings instead of maintaining sample-local bootstrap copies.

## How Godot loads the GDExtension

When Godot loads `godot_gdk.gdextension`, it calls the extension entry symbol, which routes into `register_types.cpp`.

`register_types.cpp` currently:

1. registers the native classes
2. creates one `GDK` object
3. publishes it as the engine singleton `"GDK"`

Gameplay code should therefore treat `GDK` as the only root singleton and reach services from there instead of expecting older flat singletons.

## Why the samples are part of the build flow

The build scripts do not just produce the addon DLL. They also sync addon
metadata and runtime DLL dependencies into the sample projects declared by the
root CMake configuration.

That gives the repo a shared sample payload while still letting different sample
projects exercise different slices of the addon surface.

In practice:

- `sample\gdk_demo\` is the canonical GDK regression sample
- `tests\godot\gdk\` is the canonical GDK test harness
- `sample\gdk_launch_point\` and `sample\multiplayer_pong\` consume the same synced addon payload for broader scenario coverage
- `sample\playfab_demo\` also receives synced `godot_gdk` files because the PlayFab sample depends on the GDK runtime/user flow
- `tests\godot\playfab\` receives `godot_gdk` when `GODOT_PLAYFAB_TEST_HOST_WITH_GDK=ON` so optional Xbox-backed PlayFab compatibility tests can run; turn the option off to keep the PlayFab host custom-ID-only
