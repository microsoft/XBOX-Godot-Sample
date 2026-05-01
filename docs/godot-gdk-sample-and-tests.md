# Godot GDK sample and tests

This document explains how the sample project uses the addon and what the current headless test harness validates.

See also:

- [`godot-gdk-plugin.md`](godot-gdk-plugin.md)
- [`godot-gdk-native-runtime.md`](godot-gdk-native-runtime.md)

## Sample project role

The `sample/` directory contains multiple sample projects that demonstrate
how to use the GDK addon:

| Sample | Description |
|--------|-------------|
| `sample/gdk_demo/` | Baseline runtime/users/achievements/presence/social demo and headless test suite |
| `sample/gdk_launch_point/` | GDK Launch Point scenario shell with grouped actions and event log |
| `sample/multiplayer_pong/` | Multiplayer pong with Xbox identity, single player AI, and visual effects |

All samples share the same GDK addon setup: `gdk_bootstrap.gd` autoload,
`plugin.cfg` editor plugin, and addon DLLs synced by the CMake build system.

## Autoload bootstrap

Each sample's `project.godot` autoloads its own copy of `gdk_bootstrap.gd`.

The `gdk_demo` bootstrap currently:

1. skips itself during the headless test run
2. connects to root and users signals
3. calls `GDK.initialize()`
4. starts `GDK.users.add_default_user_async()` when initialization succeeds
5. relies on native auto-dispatch with `gdk/runtime/embed_dispatch=true`
6. shuts the runtime down when leaving the tree

That means the GDK demo sample expects native auto-dispatch to stay enabled and
does not provide a manual pump path in the sample code.

`sample\gdk_launch_point\gdk_bootstrap.gd` also autoloads in the GDK Launch Point sample, but that bootstrap only keeps the extension loaded while the shell drives runtime scenarios. The sample projects set `gdk/runtime/embed_dispatch=true` explicitly in `project.godot` and do not expose manual dispatch UI.

## Demo scenes

### GDK Demo (`sample\gdk_demo\main.gd`)

A minimal runtime/users/achievements/presence/social demo.

It currently:

- reflects runtime state
- shows the primary user's gamertag and XUID
- retries the silent sign-in flow through `GDK.users.add_default_user_async()`
- queries the achievements cache for achievement `1`
- advances achievement `1` in 25% steps through `GDK.achievements.update_achievement_async()`
- shows the current cached progress for achievement `1`
- queries and shows the primary user's cached presence state
- starts the Social Manager graph for the primary user
- requests the default friends group through `GDK.social.get_friends_async()`
- shows the current tracked friend count for that group

The older controller widgets are still present in the scene tree, but the script now reuses one of the previously hidden text areas for the presence/social summary because controller-native functionality is still not part of the current baseline.

### GDK Launch Point (`sample\gdk_launch_point\main.gd`)

`sample\gdk_launch_point\main.gd` builds a scenario catalog with grouped runtime, users, achievements, and multiplayer activity actions. It serves as the repo's GDK Launch Point through:

- grouped scenarios
- nested "up one level" navigation
- a tile-style menu
- a persistent event log
- a side panel that reflects the currently selected scenario and live GDK state

### Multiplayer Pong (`sample\multiplayer_pong\`)

A multiplayer pong game imported from [godot-demo-projects](https://github.com/godotengine/godot-demo-projects) and extended with:

- Single player mode with AI opponent
- Xbox identity integration (sign-in required for multiplayer, optional for single player)
- Visual effects: ball trail, screen shake, score pop animations, neon color scheme

## Headless tests

`sample\gdk_demo\tests\run_tests.gd` is the current regression harness for the plugin.

It checks:

- singleton availability
- class registration
- root API shape plus lifecycle/reset behavior
- users API shape plus real default-user flow when a local user is available
- achievements API shape plus live query/update validation against a signed-in user when services are available
- presence API shape plus caller-context and input-validation behavior
- social API shape plus graph/group behavior against a signed-in user when services are available
- signal connectivity
- embed-dispatch startup behavior and sample bootstrap compatibility
- addon structure

The suite is now split into focused modules with a shared test context so it can exercise real runtime behavior, async completion, and signed-in user flows when the environment supports them, while still checking deterministic validation/error paths on machines that cannot complete Xbox sign-in.

## Why the sample and tests matter

The sample and tests are the closest thing the addon has to an end-to-end integration surface right now.

They validate that:

- the addon loads correctly in Godot
- the `GDK` singleton is present
- the runtime/users/achievements/presence/social baseline is script-usable
- the dispatch model is compatible with a normal Godot frame pump
- deterministic validation and unavailable-environment paths still surface stable Godot-facing results
