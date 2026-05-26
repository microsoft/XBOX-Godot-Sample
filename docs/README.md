# GodotGDK Documentation

Welcome to the documentation for the GodotGDK repository — a collection of
Godot 4.x GDExtension addons for Windows gaming integrations around the
Microsoft public GDK.

## Folder structure

```
docs/
├── README.md                                — this index
├── getting-started.md                       — onboarding (repo build + first sign-in)
├── addon-getting-started.md                 — addon-zip quickstart
├── troubleshooting.md                       — common build, runtime, and test issues
├── tutorials/                               — task-oriented walkthroughs
├── gdk/                                     — godot_gdk addon
├── playfab/                                 — godot_playfab addon
├── gameinput/                               — godot_gameinput addon
├── packaging/                               — godot_gdk_packaging addon
└── platform/                                — Xbox sandbox + test-account setup
```

## Getting started

- [**Getting Started**](getting-started.md) — adding the addons to your
  project, configuring project settings, getting to user sign-in (Xbox
  + PlayFab), and building from source
- [**Addons getting started**](addon-getting-started.md) — short
  quickstart shipped inside the addon zip (enable, set PlayFab title
  id, create game config, switch sandbox, sign in)
- [**Tutorials**](tutorials/README.md) — task-oriented walkthroughs
  for sign-in, achievements, leaderboards, Game Saves, lobbies, and
  GameInput
- [**Test Pipeline**](gdk/sample-and-tests.md) — repo-wide orchestrator,
  GUT hosts, C++ doctest, live switch, and test baselines

## Sample projects

Four sample projects live under `sample/`:

| Sample | Description |
|--------|-------------|
| [`gdk_demo`](gdk/sample-and-tests.md) | GDK addon demo |
| [`gdk_launch_point`](gdk/sample-and-tests.md) | GDK Launch Point scenario shell |
| `multiplayer_pong` | Multiplayer pong with Xbox identity and GameInput rumble; not a test host |
| [`playfab_demo`](playfab/plugin.md) | PlayFab sign-in, Game Saves, and leaderboard sample |

All samples share the same GDK setup. See [Sample Project Setup](gdk/sample-setup.md)
for Partner Center configuration.

## Godot GDK addon (`godot_gdk`)

### User guides

- [**Sample Project Setup**](gdk/sample-setup.md) — Partner Center
  configuration, sandbox setup, test accounts, and the config flow
- [**GDScript API Reference**](gdk/api-reference.md) — public API
  surface for `GDK`, `GDK.system`, `GDK.users`, `GDK.game_ui`,
  `GDK.accessibility`, `GDK.achievements`, `GDK.package`, `GDK.stats`,
  `GDK.leaderboards`, `GDK.privacy`, `GDK.presence`, `GDK.social`,
  `GDK.profile`, `GDK.string_verify`, `GDK.title_storage`,
  `GDK.error_reporting`, `GDK.multiplayer_activity`, `GDK.capture`,
  `GDK.launcher`, and `GDK.store`

### Architecture

- [**Plugin Overview**](gdk/plugin.md) — implementation status and
  document map
- [**Build and Loading**](gdk/build-and-loading.md) — addon layout,
  build flow, and how Godot loads the extension
- [**Native Runtime**](gdk/native-runtime.md) — runtime structure,
  services, and request flow
- [**Async System**](gdk/async-system.md) — shared queue, pending-signal
  helpers, XAsync bridge, and cancellation
- [**Editor Tools**](gdk/editor-tools.md) — editor plugin, setup panel,
  and export platform
- [**Sample and Tests**](gdk/sample-and-tests.md) — sample architecture,
  coverage hosts, and the repo-wide test pipeline

## GameInput addon (`godot_gameinput`)

- [**GameInput Addon**](gameinput/plugin.md) — devices, polling, vibration,
  action bridge, project settings, sample integration, and testing
- [**GameInput Manual Tests**](gameinput/manual-tests.md) — hardware
  checklist for things headless tests can't cover (real controllers, rumble,
  hot-plug)

## PlayFab addon (`godot_playfab`)

- [**PlayFab Plugin Overview**](playfab/plugin.md) — runtime
  configuration, public surface (`PlayFab`, `PlayFab.users`,
  `PlayFab.game_saves`, `PlayFab.leaderboards`, and the client-safe service
  wrappers under `PlayFab.accounts`, `PlayFab.catalog`, `PlayFab.cloud_script`,
  `PlayFab.entity_data`, `PlayFab.experimentation`, `PlayFab.friends`,
  `PlayFab.groups`, `PlayFab.inventory`, `PlayFab.localization`,
  `PlayFab.player_data`, `PlayFab.statistics`, `PlayFab.title_data`), sample
  usage, and testing

## Packaging addon (`godot_gdk_packaging`)

- [**Packaging Plugin**](packaging/plugin.md) — editor tooling for PC
  packaging: Create Game Config, Sandbox dialog, export platform, and
  the Package Manager (install/uninstall/run loose builds)

## Platform setup

- [**Xbox Sandbox and Test Accounts**](platform/xbox-sandbox-and-test-accounts.md)
  — switching the PC into a Partner Center sandbox, signing in the
  Xbox app with a test account, and the error-code lookup table

## Design specs

Design intent documents live in [`spec/`](../spec/). These describe the planned
API surface and are not guaranteed to match the current implementation.

- [`gdext-gdk.md`](../spec/gdext-gdk.md) — GDK addon design spec
- [`gdext-gameinput.md`](../spec/gdext-gameinput.md) — GameInput addon design spec
- [`gdext-playfab.md`](../spec/gdext-playfab.md) — PlayFab addon design spec
- [`testing-strategy.md`](../spec/testing-strategy.md) — durable test strategy spec

## Troubleshooting

- [**Troubleshooting**](troubleshooting.md) — common build, runtime, and test issues
