---
applyTo: "addons/godot_gdk_editortools/**,tests/godot/gdk/tests/editortools/**,sample/tutorial_gdk/**,sample/tutorial_integrated/**,docs/editortools/**,spec/gdext-editortools.md"
description: "Godot GDK Editor Tools addon architecture, headless runner, settings precedence, and editor menu"
---

# Godot GDK Editor Tools addon — instructions

`addons/godot_gdk_editortools/` is the GDK PC packaging tooling (Microsoft
Game Config, makepkg, wdapp, XblPCSandbox, GameConfigEditor, Store
Association). It is **editor tooling**, not a runtime service addon. Treat
it differently from `godot_gdk` / `godot_playfab`.

## Layered shape

```
addons/godot_gdk_editortools/
  plugin.cfg
  run.gd                     # class_name GdkEditorToolsRunner; extends SceneTree
  gdkpkg.cmd, gdkpkg.sh      # shell forwarders (option C)
  core/                      # headless-safe only; no EditorInterface, no Control, no @onready
    editortools_cli.gd         # pure argv -> {verb, options, help, error}; owns VERBS dict
    editortools_config.gd      # precedence chain resolver (CLI > .cfg > MSGame.config > project)
    editortools_service.gd     # verb facade (one run_* method per CLI verb)
    editortools_result.gd      # typed Dictionary builder + EXIT_* constants
    gdk_toolchain.gd, makepkg_executor.gd, game_config_manager.gd,
    editortools_content_preparer.gd, wdapp_manager.gd,
    editortools_settings_store.gd, export_preset_catalog.gd
  editor/                    # EditorPlugin menu / import plugin; preloads from core/
```

Two non-negotiable invariants:

- **Anything under `core/` must be headless-safe.** No `EditorInterface`,
  no `Control`, no `@onready`. Production runs of `run.gd` and the GUT
  suite under `tests/godot/gdk/tests/editortools/` both instantiate these
  modules in a `--headless` Godot child process and assume nothing
  editor-only loads.
- **`editor/config_import_plugin.gd` and `editor/gdk_editortools_plugin.gd`
  stay in `editor/`.** They extend `EditorImportPlugin` / `EditorPlugin`
  and only work inside the editor process. Test files that already pin
  the `editor/` path for those two scripts are correct as-is — do not
  move them to `core/`.

## CLI surface

`run.gd` is the single headless entry point. Three equivalent invocations
are supported and must stay equivalent:

```
# A — short script path; always works
godot --headless -s res://addons/godot_gdk_editortools/run.gd -- <verb> [...]

# B — class_name main loop; needs a prior --import per host
godot --headless --main-loop GdkEditorToolsRunner -- <verb> [...]

# C — addon-local shell forwarders (auto-discover Godot)
addons\godot_gdk_editortools\gdkpkg.cmd <verb> [...]
addons/godot_gdk_editortools/gdkpkg.sh  <verb> [...]
```

The 14-verb matrix lives in `core/editortools_cli.gd::VERBS` and is the
single source of truth that docs (`docs/editortools/plugin.md`), the
spec (`spec/gdext-editortools.md`), and the GUT suite all pin against. Add
new verbs there first; never duplicate verb metadata in another file.

## Verb result contract

Every `run_*` method on `core/editortools_service.gd` returns a
`EditorToolsResult` dictionary (see `core/editortools_result.gd::make()`). The
runner mirrors `exit_code` as the process exit code and emits the full
dict as a single `PACKAGING_RESULT_JSON:<json>` line on stdout unless
`--no-json` is passed.

Exit-code categories: `EXIT_OK=0`, `EXIT_FAIL=1`, `EXIT_USAGE=2`,
`EXIT_CONFIG=3`, `EXIT_TOOL=4`, `EXIT_UNIMPLEMENTED=5`. Pick the most
specific category in new verbs.

## Settings resolver

`core/editortools_config.gd::resolve(cli_options, project_root="", settings_path=..., config_path_override="")`
collapses every source into one flat dict with precedence:

1. CLI flags (kebab-case keys; remapped to snake_case via `_CLI_KEY_REMAP`).
2. `res://.gdk_packaging.cfg` (empty strings do NOT clobber lower layers).
3. `MicrosoftGame.config`.
4. `project.godot` (only `application/config/name` / `version`).
5. Built-in defaults.

If you add a new CLI flag whose key differs from the resolver field name,
add the kebab -> snake mapping to `_CLI_KEY_REMAP`. Do not rename
existing CLI keys without updating the docs and editor UI alike — they
were chosen to match editor field names verbatim.

## Where to put new behaviour

- Pure parsing / data-shape work: `core/editortools_cli.gd`,
  `core/editortools_config.gd`, `core/editortools_result.gd`.
- New verb: add a `run_<verb>(resolved)` method on
  `core/editortools_service.gd`, register the verb in
  `core/editortools_cli.gd::VERBS`, add the kebab->snake remap if needed,
  and an entry in the doc verb table.
- New underlying-tool wrapper: live in `core/<tool>_manager.gd` or
  `core/<tool>_executor.gd`. Mirror `gdk_toolchain.gd`'s style — accept
  the toolchain in the constructor, surface `execute_tool` / `launch_detached`
  results directly.
- Editor-only UI: `editor/`. Use `preload("res://addons/godot_gdk_editortools/core/...")`
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

`addons/godot_gdk_editortools/` is mirrored into the GDK test host and the
packaging-using sample tracks (`sample/tutorial_gdk/` and
`sample/tutorial_integrated/`) via `godot_addon_sync_directory` in the root
CMakeLists. The sync uses `copy_if_different` — **it does not delete
stale mirror files.** When you remove or rename a file in
`addons/godot_gdk_editortools/`, run `cmake --build build --preset debug`
once, then manually delete the stale mirror files under
`tests/godot/gdk/addons/godot_gdk_editortools/`,
`sample/tutorial_gdk/addons/godot_gdk_editortools/`, and
`sample/tutorial_integrated/addons/godot_gdk_editortools/` before committing.

## Tests

- `tests/godot/gdk/tests/editortools/test_config_import_plugin.gd` — import
  plugin file classification and fixture XML sanity checks.
- `tests/godot/gdk/tests/editortools/test_export_preset_catalog.gd` — export
  preset parsing and Windows Desktop preset filtering.
- `tests/godot/gdk/tests/editortools/test_game_config_manager.gd` —
  MicrosoftGame.config detection/path helpers plus fixture-driven
  `parse_config()` shape coverage.
- `tests/godot/gdk/tests/editortools/test_game_config_xml_rewriting.gd` —
  MicrosoftGame.config logo rewriting and encryption-key safety pins.
- `tests/godot/gdk/tests/editortools/test_gdk_toolchain.gd` — GDK discovery,
  execute_tool result shape, and stdout/stderr capture.
- `tests/godot/gdk/tests/editortools/test_gdkpkg_forwarder.gd` — shell
  forwarder discovery order and argument-preservation regressions.
- `tests/godot/gdk/tests/editortools/test_headless_runner.gd` — `run.gd`
  parse → resolve → dispatch pipeline, JSON suppression, and exit-code
  propagation.
- `tests/godot/gdk/tests/editortools/test_makepkg_executor.gd` — makepkg argv
  construction and tool-result propagation.
- `tests/godot/gdk/tests/editortools/test_editortools.gd` — packaging helper-suite
  inventory and CLI/service public-surface contract.
- `tests/godot/gdk/tests/editortools/test_editortools_cli.gd` — argv parsing
  and verb-flag matrix.
- `tests/godot/gdk/tests/editortools/test_editortools_config_resolver.gd` —
  precedence chain + key remap + encrypt key:<path> split.
- `tests/godot/gdk/tests/editortools/test_editortools_content_preparer.gd` —
  content-prep XML helpers, `ensure_content_dir_ready()`, and runtime DLL
  refresh behavior.
- `tests/godot/gdk/tests/editortools/test_editortools_plugin_lifecycle.gd` —
  editor plugin enter/exit lifecycle.
- `tests/godot/gdk/tests/editortools/test_editortools_result.gd` — result
  builder shape, exit-code constants, JSON round trip.
- `tests/godot/gdk/tests/editortools/test_editortools_service.gd` — all CLI
  verbs, `dispatch()`, `method_for_verb()`, and failure-normalization
  coverage using a fake toolchain.
- `tests/godot/gdk/tests/editortools/test_editortools_settings_store.gd` — dock
  settings defaults plus ConfigFile save/load round trips.
- `tests/godot/gdk/tests/editortools/test_tutorial_wizard_state.gd` — tutorial
  wizard state transitions.
- `tests/godot/gdk/tests/editortools/test_wdapp_manager.gd` — wdapp
  availability, direct verb argv, list parsing, termination, and async
  cancellation/early-return guards.

GUT base class for packaging tests: just `extends GutTest`. The shared
service bases (`gdk_test_base.gd`, …) are for runtime addons; do not use
them here.

## Anti-patterns specific to this addon

- **No new top-level singleton.** This addon never registers an autoload.
  All entry points are explicit (the `GDK` menu/dialogs for editor mode;
  `run.gd` for headless mode).
- **No verb-side IO in `editortools_cli.gd`.** It is pure. Coercion and
  schema enforcement only; reading files belongs in
  `editortools_config.gd`.
- **No `Object *` cross-addon references in the public API.** This addon
  does not bind GDExtension C++ — every public surface is plain GDScript.
- **No PowerShell wrapper under `tools/`.** Headless callers use form A,
  B, or C from above. (If user demand changes, add it under
  `tools/run_editortools.ps1`; do not split the addon's own shell
  forwarders into `tools/`.)
