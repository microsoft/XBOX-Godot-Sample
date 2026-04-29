# GodotGDK

A repository of Godot 4.x GDExtension addons for the Microsoft public GDK
(Game Development Kit).

## Addons

| Addon | Description | Status |
|-------|-------------|--------|
| [`godot_gdk`](addons/godot_gdk/) | GDK runtime, Xbox user identity, Xbox achievements, Xbox presence, Xbox social graph, Xbox multiplayer activity | Runtime/users/achievements/presence/social/multiplayer-activity baseline |
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
.\sample\gdk_demo\launch_editor.bat
```

> **Note:** Building and opening the sample works immediately. Xbox Live
> features require Partner Center setup —
> see [Sample Project Setup](docs/godot-gdk-sample-setup.md).

The repo also includes a ShamWow-inspired scenario shell sample at
`sample\shamwow\`. Launch it with:

```powershell
.\sample\shamwow\launch_editor.bat
```

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
sample/                   # Sample projects
  gdk_demo/              #   GDK addon demo and tests
  multiplayer_pong/      #   Multiplayer pong (from godot-demo-projects)
  shamwow/               #   ShamWow-inspired scenario shell sample
spec/                     # Design spec documents
tools/                    # CLI helper scripts
```

## Sample projects

- `sample\gdk_demo\` — baseline runtime/users/achievements demo
- `sample\shamwow\` — scenario-driven shell inspired by ShamWow, built around grouped runtime/users/achievements/multiplayer-activity actions and an event log
- `sample\multiplayer_pong\` — multiplayer pong with Xbox identity and single player mode

## Documentation

Full documentation lives in [`docs/`](docs/README.md):

- [**Getting Started**](docs/getting-started.md) — prerequisites, building,
  VS Code setup, development workflow
- [**GDScript API Reference**](docs/godot-gdk-api-reference.md) — `GDK`,
  `GDK.users`, `GDK.achievements`, `GDK.presence`, `GDK.social`
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

## Testing Achievements

Achievements must be configured in [Partner Center](https://partner.microsoft.com/) and published
to your development sandbox before they can be unlocked.

To **reset** achievements for re-testing, use the included helper script:

```powershell
.\tools\reset_player_data.ps1
```

This signs into Partner Center via `XblDevAccount.exe`, then calls `XblPlayerDataReset.exe`
to wipe achievements, stats, and leaderboards for the specified test account. You'll need:

- **Service Config ID (SCID)** — from Partner Center → Xbox Live → Xbox Live Setup
- **Sandbox ID** — the development sandbox your test account is signed into
- **XUID** — the Xbox User ID of the test account to reset

> **Note:** Resets only work on Xbox test accounts in a development sandbox, not retail accounts.
> Restart the game after resetting for changes to take effect.

