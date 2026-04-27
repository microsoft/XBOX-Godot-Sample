# Copilot Instructions — GodotGDK

## Project Overview

GodotGDK is a repository of GDExtension addons (C++17) centered on the **Microsoft public GDK** (not GDKX) for Godot 4.x. The primary addon is `godot_gdk`, and `godot_gameinput` is a separate addon target with its own `CMakeLists.txt`. The repo targets PC/Xbox app via the GRDK (Gaming Runtime Development Kit) only — Xbox console (ERA/GDKX) is explicitly out of scope. The addons ship as Windows-only DLLs loaded through the `.gdextension` system.

## Build Commands

```powershell
# Configure (requires Microsoft GDK + Visual Studio 2022 with C++ Desktop workload)
cmake --preset default

# Build debug
cmake --build build --preset debug

# Build release
cmake --build build --preset release

# Optional: build only the GameInput addon
cmake --preset gameinput-only
cmake --build --preset debug-gameinput
```

The root `CMakeLists.txt` is a thin superproject. Addon-local build logic lives in `addons/godot_gdk/CMakeLists.txt` and `addons/godot_gameinput/CMakeLists.txt`, with shared helpers in `cmake/`.

Output DLLs land in `addons/godot_gdk/bin/` and `addons/godot_gameinput/bin/`, and are auto-copied to the matching `sample/addons/.../bin/` folders. The GDK addon also copies its PDB (debug), editor assets, and runtime DLLs.

The GDK addon build auto-detects the GDK via the `GRDKLatest` environment variable (set by the GDK installer), falling back to the standard install path. It also auto-detects **Xbox Services API (XSAPI)** and **libHttpClient** from `ExtensionLibraries/` relative to the GDK. Override with `-DGDK_GAMEKIT=<path>`, `-DXSAPI_ROOT=<path>`, or `-DLIBHTTPCLIENT_ROOT=<path>` if needed.

## Test Commands

Tests run inside Godot's headless mode using the sample project's custom test harness:

```powershell
# Run full test suite (from repo root)
cd sample
godot --headless --script res://tests/run_tests.gd

# Exit code: 0 = pass, 1 = failures
```

The test harness (`sample/tests/run_tests.gd`) extends `SceneTree` with built-in assertion helpers (`_assert_true`, `_assert_eq`, `_assert_has_method`, `_assert_has_signal`). Tests are organized into groups that run sequentially in `_initialize()`: singleton availability, class registration, per-module API surface, signal connectivity, and addon structure. There is no single-test filter — all groups always run. Tests that depend on GDK runtime services gracefully skip when those aren't available.

## Architecture

### GDExtension Singleton Pattern

Every module follows the same pattern — a C++ class registered as a Godot engine singleton:

- **`GDKCore`** → singleton name `GDK` — GDK runtime lifecycle + XTaskQueue for async dispatch
- **`GDKUserManager`** → singleton name `GDKUser` — Xbox Live sign-in (silent + UI, with auto-fallback)
- **`GDKInput`** → singleton name `GDKInput` — GameInput polling for Xbox controllers + rumble
- **`GDKAchievements`** → singleton name `GDKAchievements` — Xbox Live achievement unlock, progress, and status queries

Each singleton class has:
- A `static ClassName *singleton` pointer (initialized to `nullptr`, set in constructor, cleared in destructor)
- `GDCLASS(ClassName, Object)` macro for Godot reflection
- `ERR_FAIL_COND(singleton != nullptr)` guard in the constructor
- A `static get_singleton()` accessor

Singletons are created/registered in `register_types.cpp` at `MODULE_INITIALIZATION_LEVEL_SCENE` and torn down in reverse order. GDScript accesses them directly by name (e.g., `GDK.initialize()`).

`GDKUserInfo` is the exception — it extends `RefCounted` (not `Object`) and is not a singleton. It wraps `XUserHandle` and is passed by reference through signals.

### Async Model

GDK async operations use `XTaskQueue` with ThreadPool dispatch for work and Manual dispatch for completions. `GDKCore::tick()` drains the completion port on the main thread — **games must call `GDK.tick()` every frame** (typically from an autoload `_process`).

Async callbacks follow a context-struct pattern:
1. Allocate a context struct on the heap containing a pointer back to the singleton and any state
2. Set `XAsyncBlock.context` to the struct and `XAsyncBlock.callback` to a static C function
3. In the callback, cast `async->context` back to the struct, extract results, call a member function, then `delete` the struct
4. Use `call_deferred("emit_signal", ...)` to safely emit signals from callback context

### Editor Integration

`addons/godot_gdk/editor/` contains:
- **`gdk_editor_plugin.gd`** — registers the export platform and a setup dock panel
- **`gdk_export_platform.gd`** — `EditorExportPlatformExtension` that generates `MicrosoftGame.config`, stages PCK + DLL, and packages via `makepkg` or registers loose via `wdapp`
- **`gdk_setup_panel.gd`** — dock panel UI for entering Partner Center credentials; writes `sample_config.cfg`

### Sample Project

`sample/` is a self-contained Godot project with:
- `gdk_bootstrap.gd` — autoload that initializes GDK, GameInput, Xbox Live services, and triggers silent sign-in; calls `GDK.tick()` and `GDKInput.process()` every frame
- `main.gd` — demo UI with live gamepad visualizer and haptics testing
- `sample_config.cfg` — INI config (gitignored) with Partner Center credentials, generated by `tools/setup_sample.ps1` or the editor setup panel
- `tests/run_tests.gd` — headless test suite

### Tools

- **`tools/setup_sample.ps1`** — interactive wizard that collects Partner Center credentials and generates `sample_config.cfg`, `MicrosoftGame.config`, and updates `export_presets.cfg`
- **`tools/reset_player_data.ps1`** — resets achievements/stats for a test account via `XblPlayerDataReset.exe`

## Conventions

### Adding a New GDK Module

1. Create `addons/godot_gdk/src/gdk_<module>.h` and `addons/godot_gdk/src/gdk_<module>.cpp`
2. Class extends `Object`, uses `GDCLASS` macro, has a static singleton pointer with `get_singleton()` accessor
3. Bind methods in `_bind_methods()` via `ClassDB::bind_method(D_METHOD("name", "param1", "param2"), &Class::method)`
4. Bind signals via `ADD_SIGNAL(MethodInfo("name", PropertyInfo(Variant::TYPE, "param")))`
5. Register the class and create/register the singleton in `addons/godot_gdk/src/register_types.cpp`
6. Add the `.cpp` file to `addons/godot_gdk/CMakeLists.txt`'s `add_library` source list
7. Add cleanup in `uninitialize_gdk_extension` in reverse order

### Windows Header Ordering

Every header that includes GDK/Windows APIs must follow this order:

```cpp
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>     // Must come first

// Godot headers
#include <godot_cpp/classes/object.hpp>

// GDK headers
#include <XGameRuntimeInit.h>
```

The `_GAMING_DESKTOP` compile definition (set in `addons/godot_gdk/CMakeLists.txt`) is required for XSAPI/libHttpClient platform detection.

### Error Handling

GDK `HRESULT` failures are formatted as hex strings (`0x%08X` via `snprintf`) and reported through both `UtilityFunctions::push_error()` and signal emission (`error_occurred` / `sign_in_failed`). Methods that can fail return Godot's `Error` enum. Some HRESULTs have special handling (e.g., `HTTP_E_STATUS_NOT_MODIFIED` means "already unlocked" for achievements).

### Signal-Driven API

All async results are communicated through signals, not return values. GDScript consumers connect signals and `await` them. Methods that start async work (like `sign_in()`) return `void`.

### GDScript API Naming

C++ singletons use PascalCase class names, but GDScript accesses them through short names: `GDK`, `GDKUser`, `GDKInput`, `GDKAchievements`. Methods use snake_case per Godot convention.
