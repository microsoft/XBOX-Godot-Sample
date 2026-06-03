# GodotGDK Documentation

Welcome to the documentation for the GodotGDK repository — a collection of
Godot 4.x GDExtension addons for Windows gaming integrations around the
Microsoft public GDK.

## Getting started

- [**Getting Started**](getting-started.md) — prerequisites, building,
  VS Code setup, and development workflow

## Sample projects

Four sample projects live under `sample/`:

| Sample | Description |
|--------|-------------|
| [`gdk_demo`](godot-gdk-sample-and-tests.md) | GDK addon demo + headless test suite |
| [`gdk_launch_point`](godot-gdk-sample-and-tests.md) | GDK Launch Point scenario shell |
| `multiplayer_pong` | Multiplayer pong with Xbox identity and single player AI |
| `playfab_demo` | PlayFab sign-in smoke-test sample wired to the `PlayFab` root singleton |

All samples share the same GDK setup. See [Sample Project Setup](godot-gdk-sample-setup.md)
for Partner Center configuration.

## Godot GDK addon (`godot_gdk`)

### User guides

- [**Sample Project Setup**](godot-gdk-sample-setup.md) — Partner Center
  configuration, sandbox setup, test accounts, and the config flow
- [**GDScript API Reference**](godot-gdk-api-reference.md) — public API
  surface for `GDK`, `GDK.users`, and `GDK.achievements`

### Architecture

- [**Plugin Overview**](godot-gdk-plugin.md) — implementation status and
  document map
- [**Build and Loading**](godot-gdk-build-and-loading.md) — addon layout,
  build flow, and how Godot loads the extension
- [**Native Runtime**](godot-gdk-native-runtime.md) — runtime structure,
  services, and request flow
- [**Async System**](godot-gdk-async-system.md) — shared queue, pending-signal
  helpers, XAsync bridge, and cancellation
- [**Editor Tools**](godot-gdk-editor-tools.md) — editor plugin, setup panel,
  and export platform
- [**Sample and Tests**](godot-gdk-sample-and-tests.md) — sample architecture
  and headless test harness

## GameInput addon (`godot_gameinput`)

- [**GameInput Addon**](godot-gameinput.md) — devices, polling, vibration,
  action bridge, project settings, sample integration
- [**GameInput Manual Tests**](godot-gameinput-manual-tests.md) — hardware
  checklist for things headless tests can't cover (real controllers, rumble,
  battery, hot-plug)

## PlayFab addon (`godot_playfab`)

- [**PlayFab Plugin Overview**](godot-playfab-plugin.md) — runtime configuration,
  current implementation status, and sample usage

## Design specs

Design intent documents live in [`spec/`](../spec/). These describe the planned
API surface and are not guaranteed to match the current implementation.

- [`gdext-gdk.md`](../spec/gdext-gdk.md) — GDK addon design spec
- [`gdext-gameinput.md`](../spec/gdext-gameinput.md) — GameInput addon design spec
- [`gdext-playfab.md`](../spec/gdext-playfab.md) — PlayFab addon design spec

## Troubleshooting

- [**Troubleshooting**](troubleshooting.md) — common build and runtime issues
