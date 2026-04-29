# GodotGDK

A repository of Godot 4.x GDExtension addons for the Microsoft public GDK
(Game Development Kit).

## Addons

| Addon | Description | Status |
|-------|-------------|--------|
| [`godot_gdk`](addons/godot_gdk/) | GDK runtime, Xbox user identity, Xbox achievements | Runtime/users/achievements baseline |
| [`godot_gameinput`](addons/godot_gameinput/) | Native GameInput controller support | Scaffold (build pipeline verified) |

## Quick start

```powershell
# Clone with submodules
git clone --recurse-submodules https://github.com/gaming-microsoft/godot-public-gdk-ext.git
cd godot-public-gdk-ext

# Build
cmake --preset default
cmake --build build --preset debug

# Launch the sample
.\sample\launch_editor.bat
```

> **Note:** Building and opening the sample works immediately. Xbox Live
> features require Partner Center setup —
> see [Sample Project Setup](docs/godot-gdk-sample-setup.md).

### Requirements

- Windows 10 (18362+) or Windows 11
- [Microsoft GDK](https://github.com/microsoft/GDK/releases)
  (`winget install Microsoft.Gaming.GDK`)
- [Godot 4.3+](https://godotengine.org/download)
- Visual Studio 2022+ with the C++ Desktop workload
- CMake 3.20+

## Repository layout

```
addons/godot_gdk/         # GDK addon: metadata, editor scripts, native sources
addons/godot_gameinput/   # GameInput addon: metadata, native sources
cmake/                    # Shared CMake helpers
docs/                     # Documentation
godot-cpp/                # godot-cpp submodule
sample/                   # Shared Godot sample project
spec/                     # Design spec documents
tools/                    # CLI helper scripts
```

## Documentation

Full documentation lives in [`docs/`](docs/README.md):

- [**Getting Started**](docs/getting-started.md) — prerequisites, building,
  VS Code setup, development workflow
- [**GDScript API Reference**](docs/godot-gdk-api-reference.md) — `GDK`,
  `GDK.users`, `GDK.achievements`
- [**Sample Project Setup**](docs/godot-gdk-sample-setup.md) — Partner Center
  config, sandbox, test accounts
- [**GameInput Addon**](docs/godot-gameinput.md) — status and planned API
- [**Troubleshooting**](docs/troubleshooting.md) — common build and runtime
  issues

Architecture and internals:

- [Plugin Overview](docs/godot-gdk-plugin.md) ·
  [Build & Loading](docs/godot-gdk-build-and-loading.md) ·
  [Native Runtime](docs/godot-gdk-native-runtime.md) ·
  [Async System](docs/godot-gdk-async-system.md) ·
  [Editor Tools](docs/godot-gdk-editor-tools.md) ·
  [Sample & Tests](docs/godot-gdk-sample-and-tests.md)

## Usage

1. Copy `addons/godot_gdk/` into your Godot project
2. Enable the addon in Project → Project Settings → Plugins
3. Use from GDScript:

```gdscript
func _ready():
    GDK.initialize()

func _process(_delta):
    GDK.dispatch()

func _exit_tree():
    GDK.shutdown()
```

See the [API Reference](docs/godot-gdk-api-reference.md) for the full surface.

