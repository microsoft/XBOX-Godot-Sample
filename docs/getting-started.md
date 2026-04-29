# Getting Started

This guide covers prerequisites, building from source, editor setup, and the
development workflow for the GodotGDK repository.

## Prerequisites

- Windows 10 (18362+) or Windows 11
- [Microsoft GDK](https://github.com/microsoft/GDK/releases) — install via
  `winget install Microsoft.Gaming.GDK`
- [Godot 4.3+](https://godotengine.org/download) (stable, Windows 64-bit)
- Visual Studio 2022+ with the **C++ Desktop** workload
- CMake 3.20+

> **Note:** The Debug build of the GDK addon requires Visual Studio to be
> installed on the machine that *runs* the sample, not just the machine that
> builds it. See [Troubleshooting](troubleshooting.md) for details.

## Clone with submodules

```powershell
git clone --recurse-submodules https://github.com/gaming-microsoft/godot-public-gdk-ext.git
cd godot-public-gdk-ext
```

If you've already cloned without submodules:

```powershell
git submodule update --init --recursive
```

## Build

```powershell
# Configure all addons
cmake --preset default

# Build debug
cmake --build build --preset debug

# Build release
cmake --build build --preset release
```

The build:

- Outputs addon DLLs to `addons/<addon>/bin/`
- Copies built DLLs and runtime dependencies into each sample's `addons/<addon>/bin/`
- Syncs addon metadata and editor scripts into the sample project

### Selective builds

```powershell
# GDK addon only
cmake --preset gdk-only
cmake --build --preset debug-gdk

# GameInput addon only
cmake --preset gameinput-only
cmake --build --preset debug-gameinput
```

### CMake auto-detection

CMake automatically detects the GDK Windows layout:

1. `GameDKCoreLatest` environment variable (preferred)
2. `GameDKLatest` environment variable (fallback)
3. Latest edition under `C:/Program Files (x86)/Microsoft GDK/<edition>/windows`

To override manually:

```powershell
cmake --preset default -DGDK_WINDOWS="C:/Program Files (x86)/Microsoft GDK/260400/windows"
```

## Run the samples

**You must build before launching any sample** — the build step syncs addon
DLLs and runtime dependencies into every sample project. Without building,
Godot will fail with "GDExtension dynamic library not found" errors.

```powershell
# Build first (required — populates addon DLLs in all samples)
cmake --build build --preset debug

# Launch any sample
.\sample\gdk_demo\launch_editor.bat            # GDK addon demo
.\sample\shamwow\launch_editor.bat             # ShamWow scenario shell
.\sample\multiplayer_pong\launch_editor.bat    # Multiplayer pong
```

## VS Code setup

After building, VS Code IntelliSense should work automatically with the
included `.vscode/c_cpp_properties.json`. If you see red squiggles on
`#include` directives:

1. Ensure you've **built at least once** — godot-cpp headers are generated
   during the first build into `build/godot-cpp/gen/include/`
2. Ensure the `GameDKCoreLatest` environment variable is available (or update
   the GDK include path in `.vscode/c_cpp_properties.json`)
3. Reload VS Code (`Ctrl+Shift+P` → "C/C++: Reset IntelliSense Database")

The config defines `_GAMING_DESKTOP`, which is required for XSAPI/libHttpClient
platform detection.

## Repository layout

```
addons/godot_gdk/         # GDK addon: metadata, editor scripts, native sources
addons/godot_gameinput/   # GameInput addon: metadata, native sources
cmake/                    # Shared CMake helpers
docs/                     # Documentation
godot-cpp/                # godot-cpp submodule
sample/                   # Sample projects
  gdk_demo/              #   GDK addon demo and tests
  multiplayer_pong/      #   Multiplayer pong (from godot-demo-projects)
spec/                     # Design spec documents
tools/                    # CLI helper scripts
```

## Development workflow

### After changing native code

```powershell
cmake --build build --preset debug
```

This rebuilds the DLL and syncs it (plus addon metadata) into the sample
project.

### Running headless tests

```powershell
cd sample/gdk_demo
.\Godot_v4.6.1-stable_win64.exe --headless --script res://tests/run_tests.gd
```

### After changing editor scripts or addon metadata

Rebuild so the sample copy is refreshed:

```powershell
cmake --build build --preset debug
```

### Validating changes

1. Rebuild the addon
2. Run the headless test suite
3. Open the sample in the editor and verify the GDK Setup panel loads
4. If Xbox Live features changed, test with a sandbox and test account

### Optional pre-commit hook

Enable the repo-managed pre-commit hook to run headless GDScript validation before each commit:

```powershell
git config core.hooksPath .githooks
```
