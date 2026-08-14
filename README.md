# XBOX Godot Sample

[![Latest release][badge-release]][link-release]
[![License: MIT][badge-license]][link-license]
[![Godot 4.5+][badge-godot]][link-godot]
[![Microsoft GDK supported][badge-gdk]][link-gdk]
[![PlayFab supported][badge-playfab]][link-playfab]
[![GameInput supported][badge-gameinput]][link-gameinput]
[![.NET supported][badge-dotnet]][link-dotnet]
[![Documentation on Microsoft Learn][badge-docs]][link-docs]
[![Visit our Blog][badge-blog]][link-blog]
[![Join us on Discord][badge-discord]][link-discord]
[![PRs welcome][badge-prs]][link-prs]

**Learn to build your Godot 4 game for XBOX on PC.** Reference GDExtension addons binding the Microsoft **GDK**, **XBOX Services**, **PlayFab**, and **GameInput** into Godot - usable from both GDScript and C#/.NET.

> [!WARNING]
> **Breaking change: `GDK*` classes are now `Xbox*`.** Every script-visible type was renamed from the `GDK` prefix to the `Xbox` prefix — `GDKUser` → `XboxUser`, `GDKResult` → `XboxResult`, `GDKAchievement` → `XboxAchievement`, and so on. The C# facade moved with them: namespace `GodotGdk` → `GodotXbox`, static class `Gdk` → `Xbox`. No deprecated `GDK*` aliases are provided, so type references must be updated.
>
> **The singleton is still called `GDK`.** Calls like `GDK.initialize()` and `GDK.users.add_default_user_async()` keep working unchanged — only the *type* names moved. The `addons/godot_gdk` folder, the `gdk/runtime/*` Project Settings, and the `godot_gdk.gdextension` entry point are all unchanged as well.
>
> ```gdscript
> # before
> var r: GDKResult = GDK.initialize()
> var u: GDKUser = r.data
>
> # after
> var r: XboxResult = GDK.initialize()
> var u: XboxUser = r.data
> ```
>
> One consequence worth calling out: the ClassDB class name (`Xbox`) and the singleton name (`GDK`) are now different strings. If your code class-checks the singleton, compare against `"Xbox"` — `is_class("GDK")` no longer matches.


> [!IMPORTANT]
> **This is a source-only sample, not a product.** The repository is MIT-licensed at the wrapper layer; the Microsoft GDK and PlayFab dependencies still require their own installs and license acceptance, consistent with other XBOX samples. There is no specified update cadence for support or maintenance. We'll watch the repo, monitor issues, and iterate where it makes sense, but this isn't a commercial release. We are excited to hear your feedback, and see any community PRs, as we evolve this together.
>
> **This is a sample specific to XBOX on PC.** There is no specific support for XBOX Series X\|S or XBOX One. Please talk with your Microsoft representative if you'd like to learn more about support on those platforms.
<img width="1920" height="1080" alt="XBOXGodot_HERO1" src="https://github.com/user-attachments/assets/59dfb9dc-8eb6-4bc0-b15e-c55b90057020" />

## Quick start

| I want to… | Start here |
|---|---|
| Drop the addons into an existing Godot project | [**Addons quickstart**](docs/addon-getting-started.md) |
| Clone and build the sample from source | [**Getting started**](docs/getting-started.md) |
| Follow a guided, task-oriented walkthrough | [**Tutorials**](docs/tutorials/README.md) |
| Set up an XBOX sandbox and test accounts | [**Sandbox and test accounts**](docs/platform/xbox-sandbox-and-test-accounts.md) |
| Fix a build or runtime error | [**Troubleshooting**](docs/troubleshooting.md) |
| Browse the full documentation tree | [**Documentation index**](docs/README.md) |

## Overview

A working source-only reference for building a Godot extension that wraps the Microsoft **GDK**, **XBOX Services**, and **PlayFab**, and lets you build your title for XBOX on PC - without leaving the engine you already love.

The sample covers roughly **85–95% of the surface area** a Godot developer needs to ship for XBOX on PC, across:

- GDK platform services and XBOX services (identity, achievements, presence, social, profile, privacy, multiplayer activity, stats, leaderboards, title storage, package metadata + DLC, XStore commerce, GameUI, accessibility, capture, launcher, error reporting)
- PlayFab Core + Services (accounts, catalog, cloud script, entity data, experimentation, friends, groups, inventory, localization, player data, statistics, title data)
- PlayFab Multiplayer (Lobby, Matchmaking, Party)
- PlayFab Game Saves
- Microsoft GameInput v3 controller support - devices, polling, rumble, and an action bridge into Godot's `Input` / `InputMap`

The **PlayFab extension sample code does not have a specific dependency on the Microsoft GDK extension sample code**, so the two can be adopted modularly - use either on its own, or compose them (e.g. sign in to PlayFab with the XBOX user provided by the GDK side).

The sample is intended to give you insights and re-usable integration code that you can leverage in your own game. The default (vcpkg) build targets the **April 2026 Microsoft GDK** out of the box; the sample also builds against an installed **October 2025 Microsoft GDK (edition `251001`)** or later via the `installed-gdk` preset and `-DGDK_VERSION=<edition>` - see [Source for the Microsoft GDK dependency](docs/getting-started.md#source-for-the-microsoft-gdk-dependency).

This is the **first step** in our XBOX Godot Sample integration journey. We plan to evolve it over time based on what the community tells us is most valuable.

## Addons

The addons are designed to be dropped into any Godot 4.5+ project. This repository is where the addons are authored, built, tested, and demonstrated through the tutorial sample projects under `sample/tutorial_gdk/`, `sample/tutorial_playfab/`, `sample/tutorial_integrated/`, and `sample/tutorial_gameinput/`. Build the addons from source per [Getting started](docs/getting-started.md), then drop the addon folders into your project.

| Addon | Description |
|-------|-------------|
| [`godot_gdk`](addons/godot_gdk/) | GDK runtime + PC-supported XBOX services: users, achievements, presence, social, profile, privacy, multiplayer activity, stats, leaderboards, title storage, string verification, package metadata + DLC, XStore commerce, GameUI, accessibility, capture, launcher, error reporting, system metadata |
| [`godot_playfab`](addons/godot_playfab/) | PlayFab runtime, XBOX- and custom-ID sign-in, Game Saves, leaderboards, Multiplayer (lobby + matchmaking), Party, and client-safe service wrappers (accounts, catalog, cloud script, entity data, experimentation, friends, groups, inventory, localization, player data, statistics, title data) |
| [`godot_gameinput`](addons/godot_gameinput/) | Native GameInput v3 controller support - devices, polling, vibration, and an action bridge into Godot's InputMap |
| [`godot_gdk_editortools`](addons/godot_gdk_editortools/) | Pure-GDScript editor plugin for PC MSIXVC packaging via `makepkg.exe`, plus the in-editor Package Manager dialog |

## Documentation

Full documentation lives in [`docs/`](docs/README.md).

Start here:

- [**Documentation index**](docs/README.md) - full doc tree
- [**Getting started**](docs/getting-started.md) - clone, build, install the addons in your own Godot project, and sign in
- [**Addons quickstart**](docs/addon-getting-started.md) - drop the addons into an existing Godot project
- [**Tutorials**](docs/tutorials/README.md) - task-oriented walkthroughs (sign-in, achievements, leaderboards, Game Saves, lobbies, Multiplayer Activity, PlayFab Party, integration tech demo) plus a standalone GameInput track
- [**Troubleshooting**](docs/troubleshooting.md) - common build, runtime, and test issues

Per-addon documentation:

- [`godot_gdk`](docs/gdk/plugin.md) - runtime, services, async system, build, editor tooling
- [`godot_playfab`](docs/playfab/plugin.md) - runtime configuration, user sessions, Game Saves, leaderboards, client services
- [`godot_gameinput`](docs/gameinput/plugin.md) - devices, polling, vibration, action bridge
- [`godot_gdk_editortools`](docs/editortools/plugin.md) - headless packaging runner; see also the [editor `GDK` menu](docs/editortools/editor-menu.md)

Platform setup:

- [XBOX sandbox and test accounts](docs/platform/xbox-sandbox-and-test-accounts.md)

Design specs live in [`spec/`](spec/) - design intent that is not always reflective of the current implementation.

## Additional Documentation

- [**Microsoft GDK**](https://github.com/microsoft/GDK) - Microsoft GDK product details
- [**PlayFab Unified SDK**](https://learn.microsoft.com/en-us/gaming/playfab/sdks/unified-sdk/overview) - PlayFab Unified SDK product details
- [**GameInput**](https://aka.ms/GameInput) - GameInput product details
- [**Docs**](https://aka.ms/XBOXGodotDocs) - documentation on XBOX development with Godot on Microsoft GDK Learn website
- [**Issues**](https://aka.ms/XBOXGodotIssues) - list of issues & feedback related to XBOX Godot sample
- [**Godot C# Essentials**](https://github.com/microsoft/godot-csharp-essentials) - learning content provided by Microsoft on using Godot with C#

## Support and contributing

- [**Report a bug**](https://github.com/microsoft/XBOX-Godot-Sample/issues/new?template=bug_report.yml) - file a bug report with repro steps, Godot engine version, and GDK version
- [**Open an issue**](https://github.com/microsoft/XBOX-Godot-Sample/issues/new/choose) - pick from the available issue templates
- [`SUPPORT.md`](SUPPORT.md) - how to file issues
- [`CONTRIBUTING.md`](CONTRIBUTING.md) - CLA and Code of Conduct
- [`SECURITY.md`](SECURITY.md) - security vulnerability reporting (MSRC; please do **not** file security issues via GitHub)
- [`CODE_OF_CONDUCT.md`](CODE_OF_CONDUCT.md) - Microsoft Open Source Code of Conduct

<!-- Badge image and link definitions -->
[badge-release]: https://img.shields.io/github/v/release/microsoft/XBOX-Godot-Sample?label=release&color=107C10
[link-release]: https://github.com/microsoft/XBOX-Godot-Sample/releases
[badge-license]: https://img.shields.io/github/license/microsoft/XBOX-Godot-Sample?color=107C10
[link-license]: LICENSE
[badge-godot]: https://img.shields.io/badge/Godot-4.5%2B-478CBF?logo=godotengine&logoColor=white
[link-godot]: https://godotengine.org/
[badge-gdk]: https://img.shields.io/badge/Microsoft%20GDK-%E2%9C%93-107C10
[link-gdk]: https://github.com/microsoft/GDK/releases
[badge-playfab]: https://img.shields.io/badge/PlayFab-%E2%9C%93-107C10?logo=data:image/svg%2Bxml;base64,PHN2ZyB4bWxucz0iaHR0cDovL3d3dy53My5vcmcvMjAwMC9zdmciIHZpZXdCb3g9IjAgMCAyNCAyNCI%2BPHBhdGggZmlsbD0iI2ZmZiIgZD0iTTYuNiAxOS42YTQuNiA0LjYgMCAwIDEtLjQzLTkuMTggNi42NSA2LjY1IDAgMCAxIDEyLjYtLjk4IDQuNTcgNC41NyAwIDAgMS0uODcgOS4xNkg2LjZaIi8%2BPC9zdmc%2BDQo%3D
[link-playfab]: docs/playfab/plugin.md
[badge-gameinput]: https://img.shields.io/badge/GameInput-%E2%9C%93-0078D4?logo=data:image/svg%2Bxml;base64,PHN2ZyB4bWxucz0naHR0cDovL3d3dy53My5vcmcvMjAwMC9zdmcnIHZpZXdCb3g9JzAgMCAyNCAyNCcgZmlsbD0nI2ZmZicgZmlsbC1ydWxlPSdldmVub2RkJz48cGF0aCBkPSdNNyA3aDEwYTUgNSAwIDAgMSA1IDQuNmwuOCA1LjJBMy4yIDMuMiAwIDAgMSAxNyAxOC42TDE1LjQgMTZIOC42TDcgMTguNmEzLjIgMy4yIDAgMCAxLTUuOC0xLjhMMiAxMS42QTUgNSAwIDAgMSA3IDdabS0xIDN2MS41SDQuNXYxLjdINlYxNWgxLjd2LTEuOGgxLjV2LTEuN0g3LjdWMTBabTEwLjMuNGExLjIgMS4yIDAgMSAwIDAgMi40IDEuMiAxLjIgMCAwIDAgMC0yLjRabS0yLjYgMi42YTEuMiAxLjIgMCAxIDAgMCAyLjQgMS4yIDEuMiAwIDAgMCAwLTIuNFonLz48L3N2Zz4%3D
[link-gameinput]: docs/gameinput/plugin.md
[badge-dotnet]: https://img.shields.io/badge/.NET-%E2%9C%93-512BD4?logo=dotnet&logoColor=white
[link-dotnet]: docs/getting-started.md
[badge-docs]: https://img.shields.io/badge/docs-Microsoft%20Learn-0078D4
[link-docs]: https://aka.ms/XBOXGodotDocs
[badge-blog]: https://img.shields.io/badge/Visit%20our-Blog-FFA500?logo=rss&logoColor=white
[link-blog]: https://developer.microsoft.com/en-us/games/articles/
[badge-discord]: https://img.shields.io/badge/Join%20us%20on-Discord-7289DA?logo=discord&logoColor=white
[link-discord]: https://aka.ms/msftgamedevdiscord
[badge-prs]: https://img.shields.io/badge/PRs-welcome-d6336c
[link-prs]: CONTRIBUTING.md
