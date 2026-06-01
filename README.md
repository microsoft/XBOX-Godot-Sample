# Godot XBOX

> [!IMPORTANT]
> **This is a source-only sample, not a product.** The repository is MIT-licensed at the wrapper layer; the Microsoft GDK and PlayFab dependencies still require their own installs and license acceptance, consistent with other XBOX samples. There is no specified update cadence for support or maintenance. We'll watch the repo, monitor issues, and iterate where it makes sense, but this isn't a commercial release. We are excited to hear your feedback, and see any community PRs, as we evolve this together.
>
> **This is a sample specific to XBOX on PC.** There is no specific support for XBOX Series X\|S or XBOX One. Please talk with your Microsoft representative if you'd like to learn more about support on those platforms.

A working source-only reference for building a Godot extension that wraps the Microsoft **GDK**, **XBOX Services**, and **PlayFab**, and lets you build your title for XBOX on PC — without leaving the engine you already love.

The sample covers roughly **85–95% of the surface area** a Godot developer needs to ship for XBOX on PC, across:

- GDK platform services and XBOX services (identity, achievements, presence, social, profile, privacy, multiplayer activity, stats, leaderboards, title storage, package metadata + DLC, XStore commerce, GameUI, accessibility, capture, launcher, error reporting)
- PlayFab Core + Services (accounts, catalog, cloud script, entity data, experimentation, friends, groups, inventory, localization, player data, statistics, title data)
- PlayFab Multiplayer (Lobby, Matchmaking, Party)
- PlayFab Game Saves
- Microsoft GameInput v3 controller support — devices, polling, rumble, and an action bridge into Godot's `Input` / `InputMap`

The **PlayFab extension sample code does not have a specific dependency on the Microsoft GDK extension sample code**, so the two can be adopted modularly — use either on its own, or compose them (e.g. sign in to PlayFab with the XBOX user provided by the GDK side).

The sample is intended to give you insights and re-usable integration code that you can leverage in your own game. The sample is currently compatible with the **April 2026 Microsoft GDK** out of the box.

This is the **first step** in our Godot for XBOX on PC integration journey. We plan to evolve it over time based on what the community tells us is most valuable.

## Addons

The addons are designed to be dropped into any Godot 4.5+ project. This repository is where the addons are authored, built, tested, and demonstrated through the tutorial sample projects under `sample/tutorial_app/` and `sample/tutorial_gameinput/`. Build the addons from source per [Getting started](docs/getting-started.md), then drop the addon folders into your project.

| Addon | Description |
|-------|-------------|
| [`godot_gdk`](addons/godot_gdk/) | GDK runtime + PC-supported XBOX services: users, achievements, presence, social, profile, privacy, multiplayer activity, stats, leaderboards, title storage, string verification, package metadata + DLC, XStore commerce, GameUI, accessibility, capture, launcher, error reporting, system metadata |
| [`godot_playfab`](addons/godot_playfab/) | PlayFab runtime, XBOX- and custom-ID sign-in, Game Saves, leaderboards, Multiplayer (lobby + matchmaking), Party, and client-safe service wrappers (accounts, catalog, cloud script, entity data, experimentation, friends, groups, inventory, localization, player data, statistics, title data) |
| [`godot_gameinput`](addons/godot_gameinput/) | Native GameInput v3 controller support — devices, polling, vibration, and an action bridge into Godot's InputMap |
| [`godot_gdk_packaging`](addons/godot_gdk_packaging/) | Pure-GDScript editor plugin for PC MSIXVC packaging via `makepkg.exe`, plus the in-editor Package Manager dialog |

## Documentation

Full documentation lives in [`docs/`](docs/README.md).

Start here:

- [**Documentation index**](docs/README.md) — full doc tree
- [**Getting started**](docs/getting-started.md) — clone, build, install the addons in your own Godot project, and sign in
- [**Addons quickstart**](docs/addon-getting-started.md) — drop the addons into an existing Godot project
- [**Tutorials**](docs/tutorials/README.md) — task-oriented walkthroughs (sign-in, achievements, leaderboards, Game Saves, lobbies, Multiplayer Activity, PlayFab Party, integration tech demo) plus a standalone GameInput track
- [**Troubleshooting**](docs/troubleshooting.md) — common build, runtime, and test issues

Per-addon documentation:

- [`godot_gdk`](docs/gdk/plugin.md) — runtime, services, async system, build, editor tooling
- [`godot_playfab`](docs/playfab/plugin.md) — runtime configuration, user sessions, Game Saves, leaderboards, client services
- [`godot_gameinput`](docs/gameinput/plugin.md) — devices, polling, vibration, action bridge
- [`godot_gdk_packaging`](docs/packaging/plugin.md) — headless packaging runner; see also the [editor `GDK` menu](docs/packaging/editor-menu.md)

Platform setup:

- [XBOX sandbox and test accounts](docs/platform/Xbox-sandbox-and-test-accounts.md)

Design specs live in [`spec/`](spec/) — design intent that is not always reflective of the current implementation.

## Support and contributing

- [`SUPPORT.md`](SUPPORT.md) — how to file issues
- [`CONTRIBUTING.md`](CONTRIBUTING.md) — CLA and Code of Conduct
- [`SECURITY.md`](SECURITY.md) — security vulnerability reporting (MSRC; please do **not** file security issues via GitHub)
- [`CODE_OF_CONDUCT.md`](CODE_OF_CONDUCT.md) — Microsoft Open Source Code of Conduct
