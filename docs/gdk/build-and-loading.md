# Godot Microsoft GDK build and loading

This document explains how the `godot_gdk` addon is laid out, built, packaged, and loaded by Godot.

See also:

- [`gdk/plugin.md`](plugin.md)
- [`gdk/native-runtime.md`](native-runtime.md)

## Repository and addon layout

At the repository level, `godot_gdk` is one addon inside a repo that also
contains:

- `godot_playfab` — PlayFab runtime/services addon
- `godot_gameinput` — GameInput integration addon
- `godot_gdk_editortools` — editor-side packaging tooling

### Native addon files

`addons\godot_gdk\src\` currently contains:

- `xbox.cpp` / `xbox.h` — root singleton
- `xbox_runtime.cpp` / `xbox_runtime.h` — shared Microsoft GDK runtime and queue owner
- `xbox_result.cpp` / `xbox_result.h` — normalized result wrapper
- `xbox_pending_signal.cpp` / `xbox_pending_signal.h` — retained one-shot completion signal helper
- `xbox_signal_xasync_context.cpp` / `xbox_signal_xasync_context.h` — reusable `XAsyncBlock` bridge base for signal-returning requests
- `xbox_services.cpp` / `xbox_services.h` — shared XBOX services bootstrap and context cache
- `xbox_user.cpp` / `xbox_user.h` — users service and user wrapper
- `xbox_achievement.cpp` / `xbox_achievement.h` — achievements service and achievement wrapper
- `xbox_package.cpp` / `xbox_package.h` — package metadata and install-state service wrappers
- `xbox_presence.cpp` / `xbox_presence.h` — presence service and wrapper types
- `xbox_social.cpp` / `xbox_social.h` — social graph service and wrapper types
- `xbox_activation.cpp` / `xbox_activation.h` — single native activation registration owner and activation event fan-out
- `xbox_multiplayer_activity.cpp` / `xbox_multiplayer_activity.h` — multiplayer activity service and wrapper types
- `xbox_system.cpp` / `xbox_system.h` — system/runtime metadata service
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

### Renaming the singleton

`gdk/runtime/singleton_name` (default `GDK`) controls the name the extension
registers its Engine singleton under, for titles that need to avoid a collision
with an existing global or prefer a project-specific name.

The extension reads the setting **once, at load time**, before any project
script runs — so it must already be in `project.godot` (or an `override.cfg`)
when Godot starts. Changing it at runtime has no effect until the next launch.

A configured name is rejected, with a warning, when it is blank, is not a valid
ASCII identifier, or collides with an already-registered singleton. In those
cases the extension stays registered as `GDK` so the addon never becomes
unreachable.

Once renamed, the `GDK` global no longer resolves in GDScript. Look the
singleton up by name instead:

```gdscript
var singleton_name: String = ProjectSettings.get_setting(
        "gdk/runtime/singleton_name", "GDK")
var gdk: Object = Engine.get_singleton(singleton_name)
var result = gdk.initialize()
```

The bundled `XboxBootstrap` autoload, the C# `Xbox` facade, and the shared GUT
test base all resolve the singleton this way, so they keep working across a
rename with no further changes. They also verify the resolved object is really
a `GDK` instance, so a configured name that happens to match an unrelated
engine singleton (`Input`, say) falls back to the real runtime instead of
silently binding to the wrong object.

### Editor-side scripts

- `editor\gdk_editor_plugin.gd`
- `editor\gdk_export_platform.gd`
- `editor\gdk_setup_panel.gd`

These files are still shipped and synced. The current `gdk_editor_plugin.gd`
registers the custom `XBOX on PC` export platform and keeps the runtime
autoload installed, but it no longer docks `gdk_setup_panel.gd`. The repo's
broader packaging UI lives in the separate `godot_gdk_editortools` addon.

### Sample projects

The repository currently ships four tutorial-driven sample projects:

- `sample\tutorial_gdk\` — GDK-only tutorial track (Microsoft GDK runtime/sign-in,
  achievements, title storage & stats, Multiplayer Activity). Receives
  `godot_gdk` and `godot_gdk_editortools` mirrors from the CMake build (no PlayFab).
- `sample\tutorial_playfab\` — PlayFab-only tutorial track. Receives only the
  `godot_playfab` mirror (no GDK).
- `sample\tutorial_integrated\` — integrated tutorial track (Microsoft GDK
  runtime/sign-in, PlayFab-backed flows, Multiplayer Activity, Party, and the
  final integration scene). This project receives `godot_gdk`, `godot_playfab`,
  and `godot_gdk_editortools` mirrors from the CMake build.
- `sample\tutorial_gameinput\` — standalone GameInput tutorial sample. It is
  wired for the GameInput addon and does not consume the Microsoft GDK runtime addon.

Related Microsoft GDK runtime/test surfaces:

- `addons\godot_gdk\runtime\gdk_bootstrap.gd`
- `tests\godot\gdk\tests\`

## Build and packaging flow

The native addon is built by `addons\godot_gdk\CMakeLists.txt`.

That target currently:

1. resolves the Microsoft GDK headers and import libs via the shared
   `cmake/GDKDependencies.cmake` helper, which dispatches to either the
   `ms-gdk[playfab]` vcpkg port (default) or an installed Microsoft GDK
   on disk (opt-in via the `installed-gdk` preset — see
   [Source for the Microsoft GDK dependency](../getting-started.md#source-for-the-microsoft-gdk-dependency))
2. builds `godot_gdk` as a shared library
3. links against:
   - `godot::cpp`
   - `Xbox::GameRuntime`
   - `Xbox::XSAPI` (XSAPI thunks)
   - `Xbox::HTTPClient` (libHttpClient)
4. copies runtime DLL dependencies (libHttpClient, the per-config XSAPI
   Thunks DLL, and XCurl) into the addon output via `$<TARGET_FILE:...>`
   genexps so the addon-local `bin/` is self-contained
5. syncs addon metadata, the runtime bootstrap, and editor scripts into the sample projects listed by the root CMake configuration

The effective runtime artifact chain is:

```text
native C++ sources
  -> godot_gdk.windows.<config>.x86_64.dll
  -> addons\godot_gdk\bin\
  -> sample\tutorial_gdk\addons\godot_gdk\bin\
  -> sample\tutorial_integrated\addons\godot_gdk\bin\
```

> **Note:** vcpkg only provides the build-time dependencies. Consumers who
> want to **run** `makepkg.exe`, `wdapp.exe`, or the Game Config Editor
> still need the full Microsoft GDK installed on their machine — see
> [Editor tools](editor-tools.md) and [Editor Tools plugin](../editortools/plugin.md).
> The same install can also be used as the source of build-time headers
> and libs in place of vcpkg via the `installed-gdk` preset (which then
> needs no vcpkg checkout at all) —
> [Source for the Microsoft GDK dependency](../getting-started.md#source-for-the-microsoft-gdk-dependency).

## Runtime loading path

Godot loads the native library through:

- `addons\godot_gdk\godot_gdk.gdextension`

That file maps platform/config combinations to the built DLL path inside `addons\godot_gdk\bin\`. At runtime, `GDK.is_available()` simply reflects whether this build was compiled with `_GAMING_DESKTOP`; non-Gaming-Desktop builds return `false`.

## Editor loading path

Godot loads the editor plugin through:

- `addons\godot_gdk\plugin.cfg`

That file points to:

- `editor\gdk_editor_plugin.gd`

So the addon has two Godot-facing entry points:

- a **runtime** path through GDExtension
- an **editor** path through the normal editor plugin system

The editor plugin also keeps the `XboxBootstrap` autoload pointed at
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

- `tests\godot\gdk\` is the canonical Microsoft GDK test harness
- `tests\godot\playfab\` receives `godot_gdk` when `GODOT_PLAYFAB_TEST_HOST_WITH_GDK=ON` so optional XBOX-backed PlayFab compatibility tests can run; turn the option off to keep the PlayFab host custom-ID-only
- `sample\tutorial_gdk\` and `sample\tutorial_integrated\` receive the Microsoft GDK runtime addon; `tutorial_gdk` is the GDK-only tutorial sample and `tutorial_integrated` is the integrated Microsoft GDK + PlayFab tutorial sample (`sample\tutorial_playfab\` is PlayFab-only and is not a Microsoft GDK runtime consumer)
- `sample\tutorial_gameinput\` is a standalone GameInput sample and is not a Microsoft GDK runtime consumer
