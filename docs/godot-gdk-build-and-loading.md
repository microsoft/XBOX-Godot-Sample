# Godot GDK build and loading

This document explains how the `godot_gdk` addon is laid out, built, packaged, and loaded by Godot.

See also:

- [`godot-gdk-plugin.md`](godot-gdk-plugin.md)
- [`godot-gdk-native-runtime.md`](godot-gdk-native-runtime.md)

## Repository and addon layout

At the repository level, `godot_gdk` is one GDExtension addon inside a repo that also contains a separate `godot_gameinput` addon target.

### Native addon files

`addons\godot_gdk\src\` currently contains:

- `gdk.cpp` / `gdk.h` — root singleton
- `gdk_runtime.cpp` / `gdk_runtime.h` — shared GDK runtime and queue owner
- `gdk_result.cpp` / `gdk_result.h` — normalized result wrapper
- `gdk_async_op.cpp` / `gdk_async_op.h` — one-shot async wrapper
- `gdk_dispatch_op.cpp` / `gdk_dispatch_op.h` — dispatch-backed manager wait wrapper
- `gdk_xasync_context.cpp` / `gdk_xasync_context.h` — reusable `XAsyncBlock` bridge base
- `gdk_xbox_services.cpp` / `gdk_xbox_services.h` — shared Xbox services bootstrap and context cache
- `gdk_user.cpp` / `gdk_user.h` — users service and user wrapper
- `gdk_achievement.cpp` / `gdk_achievement.h` — achievements service and achievement wrapper
- `register_types.cpp` / `register_types.h` — Godot class registration and singleton publication

### Addon metadata

- `plugin.cfg` registers the editor plugin script
- `godot_gdk.gdextension` points Godot at the built DLL

### Editor-side scripts

- `editor\gdk_editor_plugin.gd`
- `editor\gdk_export_platform.gd`
- `editor\gdk_setup_panel.gd`

### Sample project

- `sample\gdk_demo\project.godot`
- `sample\gdk_demo\gdk_bootstrap.gd`
- `sample\gdk_demo\main.gd`
- `sample\gdk_demo\tests\run_tests.gd`

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
5. syncs addon metadata and editor scripts into the sample project

The effective runtime artifact chain is:

```text
native C++ sources
  -> godot_gdk.windows.<config>.x86_64.dll
  -> addons/godot_gdk/bin/
  -> sample/gdk_demo/addons/godot_gdk/bin/
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

## How Godot loads the GDExtension

When Godot loads `godot_gdk.gdextension`, it calls the extension entry symbol, which routes into `register_types.cpp`.

`register_types.cpp` currently:

1. registers the native classes
2. creates one `GDK` object
3. publishes it as the engine singleton `"GDK"`

Gameplay code should therefore treat `GDK` as the only root singleton and reach services from there instead of expecting older flat singletons.

## Why the sample is part of the build flow

The build scripts do not just produce the addon DLL. They also sync the addon metadata and runtime DLL dependencies into `sample\gdk_demo\addons\godot_gdk\`.

That makes the sample project the easiest place to:

- open the addon in the editor
- exercise the runtime/users/achievements/presence/social baseline
- run the current headless regression suite
