# Godot GDK sample and tests

This document explains how the sample project uses the addon and what the current headless test harness validates.

See also:

- [`godot-gdk-plugin.md`](godot-gdk-plugin.md)
- [`godot-gdk-native-runtime.md`](godot-gdk-native-runtime.md)

## Sample project role

The sample projects are the easiest place to see how the plugin is expected to be used in Godot.

It currently exercises the runtime/users/achievements/presence/social baseline and acts as the main integration target for the addon's synced binaries and editor metadata.

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

`sample\main.gd` is a minimal runtime/users/achievements/presence/social demo layered onto the existing sample scene.

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
- root API shape plus lifecycle/reset behavior
- users API shape plus real default-user flow when a local user is available
- achievements API shape plus live query/update validation against a signed-in user when services are available
- presence API shape plus caller-context and input-validation behavior
- social API shape plus graph/group behavior against a signed-in user when services are available
- signal connectivity
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
