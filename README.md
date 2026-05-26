# GodotGDK

A repository of Godot 4.x GDExtension addons for Microsoft gaming on Windows,
covering both the Microsoft public **GDK** (Game Development Kit) and
**PlayFab** live services. The two stacks compose: the GDK addons handle Xbox
identity, services, GameInput, and PC MSIXVC packaging, while the PlayFab
addon handles cross-platform live services such as Game Saves, leaderboards,
and multiplayer — typically signed in with the Xbox user provided by the
GDK side. Each addon can also be used on its own.

The addons are designed to be dropped into any Godot 4.5+ project — most
developers will consume them as a prebuilt addon zip. This repository is where
the addons are authored, built, tested, and (once the tutorial-driven sample
revamp's PR 3 lands) demonstrated through a small set of tutorial sample
projects. Sample projects are temporarily absent while that revamp lands;
follow the [tutorials](docs/tutorials/README.md) in your own project in the
meantime.

## Addons

| Addon | Description |
|-------|-------------|
| [`godot_gdk`](addons/godot_gdk/) | GDK runtime + PC-supported Xbox services: users, achievements, presence, social, profile, privacy, multiplayer activity, stats, leaderboards, title storage, string verification, package metadata + DLC, XStore commerce, GameUI, accessibility, capture, launcher, error reporting, system metadata |
| [`godot_playfab`](addons/godot_playfab/) | PlayFab runtime, Xbox- and custom-ID sign-in, Game Saves, leaderboards, Multiplayer (lobby + matchmaking), and client-safe service wrappers (accounts, catalog, cloud script, entity data, experimentation, friends, groups, inventory, localization, player data, statistics, title data) |
| [`godot_gameinput`](addons/godot_gameinput/) | Native GameInput controller support — devices, polling, vibration, and an action bridge into Godot's InputMap |
| [`godot_gdk_packaging`](addons/godot_gdk_packaging/) | Pure-GDScript editor plugin for PC MSIXVC packaging via `makepkg.exe`, plus the in-editor Package Manager dialog |

## Documentation

Full documentation lives in [`docs/`](docs/README.md).

Start here:

- [**Documentation index**](docs/README.md) — full doc tree
- [**Getting started**](docs/getting-started.md) — clone, build, install the addons in your own Godot project, and sign in
- [**Addons quickstart**](docs/addon-getting-started.md) — drop-in addon zip in an existing project
- [**Tutorials**](docs/tutorials/README.md) — task-oriented walkthroughs (sign-in, achievements, leaderboards, Game Saves, lobbies, GameInput)
- [**Troubleshooting**](docs/troubleshooting.md) — common build, runtime, and test issues

Per-addon documentation:

- [`godot_gdk`](docs/gdk/plugin.md) — runtime, services, async system, build, editor tooling
- [`godot_playfab`](docs/playfab/plugin.md) — runtime configuration, user sessions, Game Saves, leaderboards, client services
- [`godot_gameinput`](docs/gameinput/plugin.md) — devices, polling, vibration, action bridge
- [`godot_gdk_packaging`](docs/packaging/plugin.md) — Create Game Config, Sandbox dialog, export platform, Package Manager

Platform setup:

- [Xbox sandbox and test accounts](docs/platform/xbox-sandbox-and-test-accounts.md)

Design specs live in [`spec/`](spec/) — design intent that is not always reflective of the current implementation.
