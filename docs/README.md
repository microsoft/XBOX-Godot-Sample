# XBOX Godot Sample Documentation

Welcome to the documentation for the XBOX Godot Sample repository — a collection of
Godot 4.x GDExtension addons for Windows gaming integrations around the
Microsoft public GDK.

## Folder structure

```
docs/
├── README.md                                — this index
├── getting-started.md                       — onboarding (repo build + first sign-in)
├── addon-getting-started.md                 — drop-in addons quickstart
├── async-patterns.md                        — one-page async-system primer
├── troubleshooting.md                       — common build, runtime, and test issues
├── tutorials/                               — task-oriented walkthroughs
├── gdk/                                     — godot_gdk addon
├── playfab/                                 — godot_playfab addon
│   └── async-system.md                      — PlayFab completion/dispatch contract
├── gameinput/                               — godot_gameinput addon
├── packaging/                               — godot_gdk_packaging addon
└── platform/                                — XBOX Sandbox + test-account setup
```

## Getting started

- [**Getting Started**](getting-started.md) — adding the addons to your
  project, configuring project settings, getting to user sign-in (XBOX
  + PlayFab), and building from source
- [**Addons getting started**](addon-getting-started.md) — short
  quickstart for dropping the addons into an existing Godot project
  (enable, set PlayFab title id, create game config, switch sandbox,
  sign in)
- [**Tutorials**](tutorials/README.md) — task-oriented walkthroughs
  for sign-in, achievements, leaderboards, Game Saves, lobbies,
  Multiplayer Activity, PlayFab Party, the integration tech demo
  capstone, plus a standalone GameInput track
- [**Async patterns**](async-patterns.md) — one-page primer on the
  `_async` naming convention, `await`-on-coroutine, Result objects,
  and service-level runtime errors
- [**PlayFab async system**](playfab/async-system.md) — PlayFab-specific
  fire-once completion signals, dispatch ownership, and shutdown/cancellation
  behavior
- [**Test Pipeline**](gdk/sample-and-tests.md) — repo-wide orchestrator,
  GUT hosts, C++ doctest, live switch, and test baselines

## Sample projects

> Current committed sample hosts:
>
> - `sample/tutorial_gdk/` — GDK-only tutorial track (sign-in,
>   achievements, title storage & stats, Multiplayer Activity)
> - `sample/tutorial_playfab/` — PlayFab-only tutorial track (sign-in,
>   leaderboards, lobby, Party)
> - `sample/tutorial_integrated/` — integrated GDK + PlayFab track
>   (Xbox→PlayFab sign-in, integration tech demo)
> - `sample/tutorial_gameinput/` — standalone GameInput demo
>
> [The tutorials](../docs/tutorials/README.md) walk through each
> surface, and the test hosts under `tests/godot/` exercise the
> addons end-to-end.

## Godot Microsoft GDK addon (`godot_gdk`)

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
- [**PlayFab title prerequisites**](playfab/prerequisites.md) —
  PlayFab title configuration required before any PlayFab tutorial or
  sample runs (Title ID, per-tutorial Game Manager fixtures including
  the statistic + leaderboard pair backing T3, the Lobby and Party
  feature switches, and the `configure_playfab_test_title.ps1` helper)

## Packaging addon (`godot_gdk_packaging`)

- [**Packaging Plugin**](packaging/plugin.md) — headless runner and CLI
  reference for PC packaging verbs
- [**Editor `GDK` menu**](packaging/editor-menu.md) — Create/Edit
  MicrosoftGame.config, sandbox switching, Package Manager, and documentation
  shortcuts

## Platform setup

- [**XBOX Sandbox and Test Accounts**](platform/xbox-sandbox-and-test-accounts.md)
  — switching the PC into a Partner Center sandbox, signing in the
  XBOX app with a test account, and the error-code lookup table

## Troubleshooting

- [**Troubleshooting**](troubleshooting.md) — common build, runtime, and test issues
