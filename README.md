# GodotGDK

A repository of Godot 4.x GDExtension addons for the Microsoft public GDK
(Game Development Kit).

## Addons

| Addon | Description | Status |
|-------|-------------|--------|
| [`godot_gdk`](addons/godot_gdk/) | GDK runtime + PC-supported Xbox services: users, achievements, presence, social graph, profile, privacy, multiplayer activity, stats, leaderboards, title storage, string verification, package metadata + DLC, XStore commerce, GameUI, accessibility, capture, launcher, error reporting, system metadata | Runtime + PC Xbox services baseline |
| [`godot_playfab`](addons/godot_playfab/) | PlayFab runtime bootstrap, Xbox- and custom-ID sign-in, Game Saves, leaderboards, and client-safe service wrappers (accounts, catalog, cloud script, entity data, experimentation, friends, groups, inventory, localization, player data, statistics, title data) | Root runtime/users/game-saves/leaderboards + client services baseline |
| [`godot_gameinput`](addons/godot_gameinput/) | Native GameInput controller support — devices, polling, vibration, action bridge | v1: Devices, Polling, Vibration, Action Bridge |
| [`godot_gdk_packaging`](addons/godot_gdk_packaging/) | GDScript editor plugin for PC MSIXVC packaging via `makepkg.exe` | Editor plugin (no C++ build) |

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

The repo also includes the GDK Launch Point scenario shell sample at
`sample\gdk_launch_point\`. Launch it with:

```powershell
.\sample\gdk_launch_point\launch_editor.bat
```

To enable the repo-managed pre-commit hook that runs headless GDScript validation, run:

```powershell
git config core.hooksPath .githooks
```

To preview or remove ignored local artifacts from your worktree (build output,
sample `.godot\`, local Godot executables/configs, generated packaging files),
run:

```powershell
.\tools\clean_repo.ps1
.\tools\clean_repo.ps1 -Apply
```

The script removes ignored files only, so tracked repository files stay intact.

### Requirements

- Windows 10 (18362+) or Windows 11
- [Microsoft GDK](https://github.com/microsoft/GDK/releases)
  (`winget install Microsoft.Gaming.GDK`)
- [Godot 4.5+](https://godotengine.org/download)
- Visual Studio 2022+ with the C++ Desktop workload
- CMake 3.25+

## Build presets

`CMakePresets.json` defines a `default` preset that builds every addon and
per-addon presets for selective builds. Each configure preset has matching
`debug-*` / `release-*` build presets.

| Configure preset | Builds | Build presets |
|------------------|--------|---------------|
| `default` | `godot_gdk`, `godot_playfab`, `godot_gameinput` | `debug`, `release` |
| `gdk-only` | `godot_gdk` only (PlayFab and GameInput disabled) | `debug-gdk`, `release-gdk` |
| `playfab-only` | `godot_playfab` only (GDK and GameInput disabled) | `debug-playfab`, `release-playfab` |
| `gameinput-only` | `godot_gameinput` only (GDK and PlayFab disabled) | `debug-gameinput`, `release-gameinput` |
| `addon-package` | Native addons for drop-in zip packaging (tests disabled) | `debug-addon-package`, `release-addon-package` |

Use one of:

```powershell
cmake --preset default          ; cmake --build build --preset debug
cmake --preset gdk-only         ; cmake --build --preset debug-gdk
cmake --preset playfab-only     ; cmake --build --preset debug-playfab
cmake --preset gameinput-only   ; cmake --build --preset debug-gameinput
```

Per-addon `BUILD_GODOT_*` cache variables (`BUILD_GODOT_GDK`,
`BUILD_GODOT_PLAYFAB`, `BUILD_GODOT_GAMEINPUT`, `BUILD_GODOT_GDK_PACKAGING`)
can also be flipped manually on the `default` preset if you need a
combination the dedicated presets do not cover.

To create a zip that can be extracted into a Godot project root, run:

```powershell
.\tools\package_addons.ps1
```

The default package builds Debug and Release native DLLs, stages the drop-in
addon files under `build\dist\godot-gdk-addons\addons\`, and writes
`build\dist\godot-gdk-addons-debug-release.zip`.

## Repository layout

```
addons/godot_gdk/         # GDK addon: metadata, editor scripts, native sources
addons/godot_playfab/     # PlayFab addon: metadata, native sources
addons/godot_gameinput/   # GameInput addon: metadata, native sources
addons/godot_gdk_packaging/ # Pure GDScript editor plugin for PC packaging
cmake/                    # Shared CMake helpers
docs/                     # Documentation
godot-cpp/                # godot-cpp submodule
sample/                   # Sample projects
  gdk_demo/              #   GDK addon demo
  multiplayer_pong/      #   Multiplayer pong (from godot-demo-projects)
  playfab_demo/          #   PlayFab init/sign-in smoke-test sample
  gdk_launch_point/      #   GDK Launch Point scenario shell sample
spec/                     # Design spec documents
tests/godot/              # Dedicated Godot GUT coverage hosts
tools/                    # CLI helper scripts
```

## Sample projects

- `sample\gdk_demo\` — baseline runtime/users/achievements demo
- `sample\gdk_launch_point\` — scenario-driven launch point built around grouped runtime/users/achievements/multiplayer-activity actions and an event log
- `sample\multiplayer_pong\` — multiplayer pong with Xbox identity and single player mode
- `sample\playfab_demo\` — PlayFab root singleton smoke test with manual sign-in

## Documentation

Full documentation lives in [`docs/`](docs/README.md):

- [**Getting Started**](docs/getting-started.md) — prerequisites, building,
  VS Code setup, development workflow
- [**GDScript API Reference**](docs/godot-gdk-api-reference.md) — public surface
  for `GDK`, `GDK.system`, `GDK.users`, `GDK.game_ui`, `GDK.accessibility`,
  `GDK.achievements`, `GDK.package`, `GDK.stats`, `GDK.leaderboards`,
  `GDK.privacy`, `GDK.presence`, `GDK.social`, `GDK.profile`,
  `GDK.string_verify`, `GDK.title_storage`, `GDK.error_reporting`,
  `GDK.multiplayer_activity`, `GDK.capture`, `GDK.launcher`, and `GDK.store`
- [**Sample Project Setup**](docs/godot-gdk-sample-setup.md) — Partner Center
  config, sandbox, test accounts
- [**GameInput Addon**](docs/godot-gameinput.md) — devices, polling,
  vibration, action bridge, sample integration
- [**PlayFab Plugin Overview**](docs/godot-playfab-plugin.md) — PlayFab
  runtime, project settings, user sessions, Game Saves, leaderboards, and
  client-safe service wrappers (`PlayFab.accounts`, `PlayFab.catalog`,
  `PlayFab.cloud_script`, `PlayFab.entity_data`, `PlayFab.experimentation`,
  `PlayFab.friends`, `PlayFab.groups`, `PlayFab.inventory`,
  `PlayFab.localization`, `PlayFab.player_data`, `PlayFab.statistics`,
  `PlayFab.title_data`)
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

## GDK Packaging Addon

The `godot_gdk_packaging` addon is a pure GDScript editor plugin (no C++ build required) that
wraps Microsoft GDK PC packaging tools into the Godot Editor.

### Features

- **GDK editor menu** with quick access to Game Config actions and documentation
- **Headless MSIXVC Package Creation** — drive makepkg from scripts or CI
- **Mapping File Generation** — auto-generate layout.xml via `makepkg genmap`
- **Package Validation** — dry-run validation before building
- **MicrosoftGame.config Management** — create templates, parse identity, launch GameConfigEditor
- **Documentation Links** — direct links to MS Learn docs for PC packaging, makepkg, and GameConfigEditor

### Setup

1. Copy `addons/godot_gdk_packaging/` into your Godot project's `addons/` folder
2. In the Godot Editor, go to **Project → Project Settings → Plugins** and enable **GDK Packaging**
3. The GDK tools are discovered automatically from `C:\Program Files (x86)\Microsoft GDK\bin\`
   (override with the `GDK_BIN` environment variable if needed)

> In this repo, `cmake --build build --preset debug` also refreshes the synced
> sample mirrors under `sample\...\addons\godot_gdk_packaging\`. Edit
> `addons\godot_gdk_packaging\` as the source of truth.

### Usage

- Use the **GDK** editor menu to create `MicrosoftGame.config` when it is missing
  or edit it with GameConfigEditor when it exists.
- Use the headless runner (`addons\godot_gdk_packaging\gdkpkg.cmd`) for package,
  map, validate, install, and launch automation.

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

