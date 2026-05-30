---
applyTo: "addons/godot_gdk_packaging/**,tests/godot/gdk/tests/packaging/**,sample/tutorial_app/**,docs/packaging/**,spec/gdext-packaging.md"
description: "Godot GDK Packaging addon architecture, headless runner, settings precedence, and editor menu"
---

# Godot GDK Packaging addon — instructions

`addons/godot_gdk_packaging/` is the GDK PC packaging tooling (Microsoft
Game Config, makepkg, wdapp, XblPCSandbox, GameConfigEditor, Store
Association). It is **editor tooling**, not a runtime service addon. Treat
it differently from `godot_gdk` / `godot_playfab`.

## Layered shape

```
addons/godot_gdk_packaging/
  plugin.cfg
  run.gd                     # class_name GdkPackagingRunner; extends SceneTree
  gdkpkg.cmd, gdkpkg.sh      # shell forwarders (option C)
  core/                      # headless-safe only; no EditorInterface, no Control, no @onready
    packaging_cli.gd         # pure argv -> {verb, options, help, error}; owns VERBS dict
    packaging_config.gd      # precedence chain resolver (CLI > .cfg > MSGame.config > project)
    packaging_service.gd     # verb facade (one run_* method per CLI verb)
    packaging_result.gd      # typed Dictionary builder + EXIT_* constants
    gdk_toolchain.gd, makepkg_executor.gd, game_config_manager.gd,
    packaging_content_preparer.gd, wdapp_manager.gd,
    packaging_settings_store.gd, export_preset_catalog.gd,
    packaging_panel_logic.gd
  editor/                    # EditorPlugin menu / import plugin; preloads from core/
```

Two non-negotiable invariants:

- **Anything under `core/` must be headless-safe.** No `EditorInterface`,
  no `Control`, no `@onready`. Production runs of `run.gd` and the GUT
  suite under `tests/godot/gdk/tests/packaging/` both instantiate these
  modules in a `--headless` Godot child process and assume nothing
  editor-only loads.
- **`editor/config_import_plugin.gd` and `editor/gdk_packaging_plugin.gd`
  stay in `editor/`.** They extend `EditorImportPlugin` / `EditorPlugin`
  and only work inside the editor process. Test files that already pin
  the `editor/` path for those two scripts are correct as-is — do not
  move them to `core/`.

## CLI surface

`run.gd` is the single headless entry point. Three equivalent invocations
are supported and must stay equivalent:

```
# A — short script path; always works
godot --headless -s res://addons/godot_gdk_packaging/run.gd -- <verb> [...]

# B — class_name main loop; needs a prior --import per host
godot --headless --main-loop GdkPackagingRunner -- <verb> [...]

# C — addon-local shell forwarders (auto-discover Godot)
addons\godot_gdk_packaging\gdkpkg.cmd <verb> [...]
addons/godot_gdk_packaging/gdkpkg.sh  <verb> [...]
```

The 14-verb matrix lives in `core/packaging_cli.gd::VERBS` and is the
single source of truth that docs (`docs/packaging/plugin.md`), the
spec (`spec/gdext-packaging.md`), and the GUT suite all pin against. Add
new verbs there first; never duplicate verb metadata in another file.

## Verb result contract

Every `run_*` method on `core/packaging_service.gd` returns a
`PackagingResult` dictionary (see `core/packaging_result.gd::make()`). The
runner mirrors `exit_code` as the process exit code and emits the full
dict as a single `PACKAGING_RESULT_JSON:<json>` line on stdout unless
`--no-json` is passed.

Exit-code categories: `EXIT_OK=0`, `EXIT_FAIL=1`, `EXIT_USAGE=2`,
`EXIT_CONFIG=3`, `EXIT_TOOL=4`, `EXIT_UNIMPLEMENTED=5`. Pick the most
specific category in new verbs.

## Settings resolver

`core/packaging_config.gd::resolve(cli_options, project_root="", settings_path=..., config_path_override="")`
collapses every source into one flat dict with precedence:

1. CLI flags (kebab-case keys; remapped to snake_case via `_CLI_KEY_REMAP`).
2. `res://.gdk_packaging.cfg` (empty strings do NOT clobber lower layers).
3. `MicrosoftGame.config`.
4. `project.godot` (only `application/config/name` / `version`).
5. Built-in defaults.

If you add a new CLI flag whose key differs from the resolver field name,
add the kebab -> snake mapping to `_CLI_KEY_REMAP`. Do not rename
existing CLI keys without updating the docs and the dock alike — they
were chosen to match dock field names verbatim.

## Where to put new behaviour

- Pure parsing / data-shape work: `core/packaging_cli.gd`,
  `core/packaging_config.gd`, `core/packaging_result.gd`.
- New verb: add a `run_<verb>(resolved)` method on
  `core/packaging_service.gd`, register the verb in
  `core/packaging_cli.gd::VERBS`, add the kebab->snake remap if needed,
  and an entry in the doc verb table.
- New underlying-tool wrapper: live in `core/<tool>_manager.gd` or
  `core/<tool>_executor.gd`. Mirror `gdk_toolchain.gd`'s style — accept
  the toolchain in the constructor, surface `execute_tool` / `launch_detached`
  results directly.
- Editor-only UI: `editor/`. Use `preload("res://addons/godot_gdk_packaging/core/...")`
  for any shared helper. Do not duplicate helper logic across `editor/` and
  `core/`.

## Editor menu and headless cohabitation

The primary automation surface is `run.gd` plus the addon-local shell
forwarders. The editor plugin intentionally does **not** register a dock tab;
it only adds the top-level GDK menu for Game Config and documentation
shortcuts.

- If you change a helper's public method signature, update both editor menu
  call-sites and the service in the same change.

## Sample mirrors

`addons/godot_gdk_packaging/` is mirrored into the GDK test host
via `godot_addon_sync_directory` in the root CMakeLists. (Sample
mirror targets will return when PR 3 of the tutorial-driven
sample revamp adds `sample/tutorial_app/`.) The sync uses
`copy_if_different` — **it does not delete stale mirror files.**
When you remove or rename a file in `addons/godot_gdk_packaging/`,
run `cmake --build build --preset debug` once, then manually
delete the stale mirror files under
`tests/godot/gdk/addons/godot_gdk_packaging/` (and, once
samples land, under each `sample/*/addons/godot_gdk_packaging/`)
before committing.

## Tests

- `tests/godot/gdk/tests/packaging/test_config_import_plugin.gd` — import
  plugin file classification.
- `tests/godot/gdk/tests/packaging/test_game_config_xml_rewriting.gd` —
  MicrosoftGame.config logo rewriting and encryption-key safety pins.
- `tests/godot/gdk/tests/packaging/test_gdk_toolchain.gd` — GDK discovery,
  execute_tool result shape, and stdout/stderr capture.
- `tests/godot/gdk/tests/packaging/test_packaging_cli.gd` — argv parsing
  and verb-flag matrix.
- `tests/godot/gdk/tests/packaging/test_gdkpkg_forwarder.gd` — shell
  forwarder argument-preservation regressions.
- `tests/godot/gdk/tests/packaging/test_packaging_config_resolver.gd` —
  precedence chain + key remap + encrypt key:<path> split.
- `tests/godot/gdk/tests/packaging/test_packaging_content_preparer.gd` —
  content-prep XML helpers and runtime DLL refresh behavior.
- `tests/godot/gdk/tests/packaging/test_packaging_panel_logic.gd` — dock
  presenter helper behavior.
- `tests/godot/gdk/tests/packaging/test_packaging_plugin_lifecycle.gd` —
  editor plugin enter/exit lifecycle.
- `tests/godot/gdk/tests/packaging/test_packaging_result.gd` — result
  builder shape, exit-code constants, JSON round trip.
- `tests/godot/gdk/tests/packaging/test_packaging_service.gd` — verb-facade
  regressions for headless-only behaviours.
- `tests/godot/gdk/tests/packaging/test_tutorial_wizard_state.gd` — tutorial
  wizard state transitions.
- `tests/godot/gdk/tests/packaging/test_wdapp_manager.gd` — wdapp
  install/uninstall verb wiring + cancellation/early-return guards.
- Pre-existing helper suites (`test_packaging.gd`,
  `test_packaging_panel_logic.gd`, `test_packaging_content_preparer.gd`,
  `test_gdk_toolchain.gd`, `test_packaging_settings_store.gd`,
  `test_export_preset_catalog.gd`, `test_game_config_manager.gd`,
  `test_makepkg_executor.gd`, `test_wdapp_manager.gd`) — preserved as-is,
  preloads updated to `core/`.

GUT base class for packaging tests: just `extends GutTest`. The shared
service bases (`gdk_test_base.gd`, …) are for runtime addons; do not use
them here.

## Anti-patterns specific to this addon

- **No new top-level singleton.** This addon never registers an autoload.
  All entry points are explicit (the dock for editor mode; `run.gd` for
  headless mode).
- **No verb-side IO in `packaging_cli.gd`.** It is pure. Coercion and
  schema enforcement only; reading files belongs in
  `packaging_config.gd`.
- **No `Object *` cross-addon references in the public API.** This addon
  does not bind GDExtension C++ — every public surface is plain GDScript.
- **No PowerShell wrapper under `tools/`.** Headless callers use form A,
  B, or C from above. (If user demand changes, add it under
  `tools/run_packaging.ps1`; do not split the addon's own shell
  forwarders into `tools/`.)
