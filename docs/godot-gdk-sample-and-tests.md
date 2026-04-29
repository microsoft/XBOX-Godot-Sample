# Godot GDK sample and tests

This document explains how the sample project uses the addon and what the current headless test harness validates.

See also:

- [`godot-gdk-plugin.md`](godot-gdk-plugin.md)
- [`godot-gdk-native-runtime.md`](godot-gdk-native-runtime.md)

## Sample project role

The sample projects are the easiest place to see how the plugin is expected to be used in Godot.

`sample\` remains the baseline runtime/users/achievements demo, while `sample_shamwow\` is a ShamWow-inspired scenario browser that groups runtime, users, achievements, and multiplayer activity actions behind nested navigation and a persistent event log.

Both act as integration targets for the addon's synced binaries and editor metadata.

## Autoload bootstrap

`sample\project.godot` autoloads `sample\gdk_bootstrap.gd`.

That bootstrap currently:

1. skips itself during the headless test run
2. connects to root and users signals
3. calls `GDK.initialize()`
4. starts `GDK.users.add_default_user_async()` when initialization succeeds
5. calls `GDK.dispatch()` every frame
6. shuts the runtime down when leaving the tree

That means the sample currently treats `GDK.dispatch()` as a per-frame pump managed by an autoload.

`sample_shamwow\project.godot` also autoloads `sample_shamwow\gdk_bootstrap.gd`, but that bootstrap only keeps the extension loaded and pumps dispatch when the runtime is already initialized. Runtime initialization itself is left to explicit scenarios in the shell.

## Demo scene

`sample\main.gd` is a minimal runtime/users/achievements demo layered onto the existing sample scene.

It currently:

- reflects runtime state
- shows the primary user's gamertag and XUID
- retries the silent sign-in flow through `GDK.users.add_default_user_async()`
- queries the achievements cache for achievement `1`
- advances achievement `1` in 25% steps through `GDK.achievements.update_achievement_async()`
- shows the current cached progress for achievement `1`

The older controller widgets are still present in the scene tree, but the script hides them because those native subsystems are not part of the current baseline.

`sample_shamwow\main.gd` instead builds a scenario catalog with grouped runtime, users, achievements, and multiplayer activity actions. It mirrors ShamWow conceptually through:

- grouped scenarios
- nested "up one level" navigation
- a tile-style menu
- a persistent event log
- a side panel that reflects the currently selected scenario and live GDK state

## Headless tests

`sample\tests\run_tests.gd` is the current regression harness for the plugin.

It checks:

- singleton availability
- class registration
- root API shape
- users API shape
- achievements API shape
- signal connectivity
- addon structure

It also verifies an important runtime behavior:

- if users methods are called before initialization, they still return a `GDKAsyncOp`
- if achievements methods are called before initialization, they still return a `GDKDispatchOp`
- that op is already completed with an error result

This keeps the public async shape stable even when the runtime is unavailable.

## Why the sample and tests matter

The sample and tests are the closest thing the addon has to an end-to-end integration surface right now.

They validate that:

- the addon loads correctly in Godot
- the `GDK` singleton is present
- the runtime/users/achievements baseline is script-usable
- the dispatch model is compatible with a normal Godot frame pump
- immediate error paths still preserve the async API contract
