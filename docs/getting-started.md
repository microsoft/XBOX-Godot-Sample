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
- Syncs addon metadata, editor scripts, GUT, and shared test support into the sample projects

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

## Clean ignored local artifacts

Use the repo cleanup helper to preview or remove ignored local files such as
`build\`, addon/sample `bin\`, sample `.godot\`, local sample configs or Godot
editor copies under `sample\`, and generated packaging output.

```powershell
# Preview what would be removed
.\tools\clean_repo.ps1

# Remove ignored local artifacts
.\tools\clean_repo.ps1 -Apply
```

The script wraps `git clean` in ignored-files-only mode, so tracked repository
files stay intact. Preview first if you want to keep local sample configs or
Godot executable copies in your worktree.

## Run the samples

**You must build before launching any sample** — the build step syncs addon
DLLs and runtime dependencies into every sample project. Without building,
Godot will fail with "GDExtension dynamic library not found" errors.

```powershell
# Build first (required — populates addon DLLs in all samples)
cmake --build build --preset debug

# Launch any sample
.\sample\gdk_demo\launch_editor.bat            # GDK addon demo
.\sample\gdk_launch_point\launch_editor.bat    # GDK Launch Point scenario shell
.\sample\multiplayer_pong\launch_editor.bat    # Multiplayer pong
```

## Run the tests

Use the repo-wide orchestrator as the canonical local test command:

```powershell
pwsh -NoLogo -NoProfile -ExecutionPolicy Bypass -File .\tools\run_all_tests.ps1
```

The orchestrator runs the GDScript parse gate, debug CMake build, C++ doctest executable, GUT host suites, bootstrap mini-runners, and aggregate summary. Results are written to `build\test-results\run-summary.json` and `build\test-results\run-summary.md`. See [Godot GDK sample and tests](godot-gdk-sample-and-tests.md) for the full pipeline, live switch, and troubleshooting links.

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
addons/godot_playfab/     # PlayFab addon: metadata, native sources
cmake/                    # Shared CMake helpers
docs/                     # Documentation
godot-cpp/                # godot-cpp submodule
sample/                   # Sample projects
  gdk_demo/               # GDK addon demo
  gdk_launch_point/       # GDK Launch Point scenario shell
  multiplayer_pong/       # Multiplayer pong demo, not a test host
  playfab_demo/           # PlayFab demo
spec/                     # Design spec documents
tests/                    # Baselines, C++ doctest sources, and Godot test hosts
  godot/gdk/              # GDK and GDK packaging test host
  godot/playfab/          # PlayFab test host
  godot/gameinput/        # GameInput test host
tools/                    # CLI helper scripts
```

## Development workflow

### After changing native code

```powershell
cmake --build build --preset debug
```

This rebuilds the DLL and syncs it, addon metadata, and generated test support into the sample projects.

### After changing editor scripts or addon metadata

Rebuild so the sample copy is refreshed:

```powershell
cmake --build build --preset debug
```

### Validating changes

1. Rebuild the addon or run the full orchestrator.
2. Run `pwsh -NoLogo -NoProfile -ExecutionPolicy Bypass -File .\tools\run_all_tests.ps1`.
3. Open the relevant sample in the editor and verify the user-facing flow still loads.
4. If Xbox Live or PlayFab live features changed, test with `-Live` against a sandbox title and test account.

### Optional pre-commit hook

Enable the repo-managed pre-commit hook to run headless GDScript validation before each commit:

```powershell
git config core.hooksPath .githooks
```
