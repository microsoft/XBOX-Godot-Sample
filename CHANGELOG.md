# Changelog

All notable changes to this project are documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.1.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).
While the version is below `1.0.0`, breaking changes may land in a minor release.

## [Unreleased]

## [0.3.0]

### Changed — BREAKING

- **Every script-visible `GDK*` type is renamed to `Xbox*`.** `GDKUser` →
  `XboxUser`, `GDKResult` → `XboxResult`, `GDKAchievement` → `XboxAchievement`,
  and so on across 46 registered classes, plus the abstract root `GDK` → `Xbox`.
  **No deprecated `GDK*` aliases are provided.**
- The engine singleton is **still registered as `GDK`**, so `GDK.initialize()`,
  `GDK.users`, and every other call through the global keep working. Only type
  names changed. Because the ClassDB name and the singleton name are now
  different strings, code that class-checks the singleton must compare against
  `"Xbox"` rather than `"GDK"`.
- C# facade: namespace `GodotGdk` → `GodotXbox`, static class `Gdk` → `Xbox`,
  and every `Gdk*` type → `Xbox*`. The `GodotGdkCSharp` assembly and project
  file names are unchanged, so existing `<ProjectReference>` entries still resolve.
- Bootstrap autoload `GDKBootstrap` → `XboxBootstrap`.
- Packaging forwarder environment variables `GDKPKG_*` → `XBOXPKG_*`.

Unchanged on purpose: the `addons/godot_gdk` folder, `godot_gdk.gdextension`,
the `gdk_addon_init` entry symbol, the `gdk/runtime/*` and `gdk/packaging/*`
Project Settings, the `gdk` export feature tag, `cmake/GDKDependencies.cmake`,
`gdk_edition.h`, the `PlayFab` and `GameInput` addons, and prose referring to
the Microsoft GDK product.

See [docs/gdk/migration-v0.3.md](docs/gdk/migration-v0.3.md) for the full
old → new table and the `tools/migrate_gdk_to_xbox.ps1` codemod.

### Added

- `tools/migrate_gdk_to_xbox.ps1` — codemod that rewrites `GDK*`/`Gdk*` API
  references in a consumer project, with `-WhatIf` support. It deliberately
  leaves bare `GDK` alone (that is still the singleton) and reports the
  genuinely ambiguous type positions and class checks for manual review.
- `docs/gdk/migration-v0.3.md` — migration guide for the rename.
- PlayFab Party chat indicators, voice controls, text-to-speech, and network
  diagnostics (#154).
- The missing XUser API wrappers, enabling XR-112 (#147).
- The engine singleton name is now a Project Setting, so the `GDK`, `PlayFab`,
  and `GameInput` globals can be renamed per project (#142).
- This changelog.

### Fixed

- Release builds no longer silently link the debug `godot-cpp` (#155).
- The game-save quota query no longer runs on the main thread (#145, #152).
- The "XBOX on PC" export now delegates its `.exe` and `.pck` to Godot's Windows
  exporter, and ships C# assemblies correctly (#144, #146, #151).
- Broken documentation anchors and H1 titles (#149).
- GDK Tutorial 3 no longer performs a Title Storage upload (#136, #140).

### Documentation

- README quick start, badges, and copy cleanup (#148, #153).

### Internal

- PR gates are skipped for documentation-only changes (#150).
- Addon versions moved from `0.1.0` / `0.1.0-dev` to `0.3.0`, matching the
  repository release.

## [0.2.0]

Community fixes plus Godot .NET support.

- Support for Godot .NET (C#) via managed facade addons and C# tutorial tracks.
- Support for a broader range of GDK and Godot versions, including the October
  2025 GDK editions and Godot 4.7.1-stable.
- Samples restructured into modular GDK / PlayFab / Integrated tracks.
- GDK API reference coverage raised to 100%.
- Token-based PlayFab sign-in (Steam, OpenID Connect, Battle.net).
- `godot_gdk_packaging` renamed to `godot_gdk_editortools`.
- CI: PR gates, nightly PlayFab live tests, libFuzzer infrastructure, and
  per-edition addon builds cached across Godot versions.
- Fixes for community-reported issues around `MicrosoftGame.config`
  initialization and build export.

## [0.1.0]

Initial XBOX Godot Sample release.

[Unreleased]: https://github.com/microsoft/XBOX-Godot-Sample/compare/v0.3.0...HEAD
[0.3.0]: https://github.com/microsoft/XBOX-Godot-Sample/compare/v0.2.0...v0.3.0
[0.2.0]: https://github.com/microsoft/XBOX-Godot-Sample/compare/v0.1.0...v0.2.0
[0.1.0]: https://github.com/microsoft/XBOX-Godot-Sample/releases/tag/v0.1.0
