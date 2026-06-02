# `godot_gdk_packaging` — Headless Packaging Surface

Status: **Headless workflow implemented**. The editor plugin exposes a
top-level `GDK` menu and modal dialogs; it does not register an editor dock.

## Overview

`addons\godot_gdk_packaging\` is the editor tooling for PC GDK packaging:
Microsoft Game Config, makepkg.exe (genmap / pack / validate), wdapp.exe
(register / install / launch / terminate / uninstall), XblPCSandbox.exe, and
the GameConfigEditor / Store Association wizard launches.

Editor access is through the top-level `GDK` menu and modal dialogs under
`editor\`. This spec defines a headless-first surface so automation
(`tools\run_all_tests.ps1`, sample-export CI, cloud agents) can drive every
verb without a human clicking through editor UI.

The headless surface is intentionally **GDScript-only** and lives entirely
under `addons\godot_gdk_packaging\`. There is no new PowerShell wrapper
under `tools\`; the addon ships its own shell forwarders for convenience.

## Architecture

```
addons\godot_gdk_packaging\
  plugin.cfg
  run.gd                                 # class_name GdkPackagingRunner; extends SceneTree
  gdkpkg.cmd, gdkpkg.sh                  # shell forwarders (option C)
  core\                                  # headless-safe; no EditorInterface, no Control
    packaging_cli.gd                     # argv -> {verb, options, help, error}
    packaging_config.gd                  # settings resolver (CLI > .cfg > MSGame.config > project)
    packaging_service.gd                 # verb facade (one run_* method per verb)
    packaging_result.gd                  # typed Dictionary builder + exit-code constants
    gdk_toolchain.gd                     # real GDK discovery + execute_tool/launch_detached
    makepkg_executor.gd                  # wraps makepkg pack/genmap/validate
    game_config_manager.gd               # parses/writes MicrosoftGame.config
    packaging_content_preparer.gd        # copies config + logos into content dir
    wdapp_manager.gd                     # wraps wdapp register/install/launch/terminate/uninstall
    packaging_settings_store.gd          # reads/writes res://.gdk_packaging.cfg
    export_preset_catalog.gd             # enumerates Windows export presets
  editor\                                # UI-only — preloads from core\
    gdk_packaging_plugin.gd              # EditorPlugin entry; top-level GDK menu
    gdk_tutorial_wizard.gd, tutorial_wizard_state.gd
    gdk_sandbox_dialog.gd                # GDK Sandbox Switcher popup
    gdk_package_manager_dialog.gd        # machine-wide Package Manager popup
    config_import_plugin.gd              # MicrosoftGame.config import behaviour
```

Layer summary:

- `core\packaging_cli.gd` is **pure**: argv -> structured dict. No file or
  process IO. Owns the declarative `VERBS` table that is the single source
  of truth for the verb-flag matrix.
- `core\packaging_config.gd` collapses CLI overrides + the .gdk_packaging.cfg
  + MicrosoftGame.config + project.godot + built-in defaults into a flat
  dict the service consumes (precedence: CLI > .cfg > config > project).
- `core\packaging_service.gd` is the verb **facade**: `run_pack`, `run_genmap`,
  `run_validate`, `run_prepare_content`, `run_export`, `run_register_loose`,
  `run_install`, `run_uninstall`, `run_launch`, `run_terminate`,
  `run_sandbox`, `run_config_template`, `run_config_editor`,
  `run_store_wizard`, plus `dispatch(verb, resolved)` and
  `method_for_verb(verb)`. Each `run_*` orchestrates the helpers and
  returns a `PackagingResult`.
- `run.gd` is `class_name GdkPackagingRunner` extending `SceneTree`. It
  reads `OS.get_cmdline_user_args()`, calls
  `PackagingCli.parse() -> PackagingConfig.resolve() -> service.dispatch()`,
  prints a one-line summary plus a `PACKAGING_RESULT_JSON:` line (unless
  `--no-json` is supplied), then `quit(exit_code)`.
- `gdkpkg.cmd` / `gdkpkg.sh` locate Godot via env vars
  (`GODOT_CONSOLE` -> `GODOT_BIN` -> `GODOT`), fall back to repo-local
  `sample\Godot*_console.exe` / `sample\Godot*.exe` candidates, then the
  current working directory's `Godot*_console.exe` / `Godot*.exe` candidates,
  then `where godot` / `where godot4` on Windows or `command -v godot` /
  `command -v godot4` on POSIX shells (`gdkpkg.sh` uses `command -v`, not
  `which`, for PATH lookup). They forward all remaining args using form
  A (`-s ...`) so they work even
  before a `--import` pass has populated the class registry. The forwarders
  preserve argument boundaries for paths with spaces; `gdkpkg.sh` uses Bash
  arrays and `gdkpkg.cmd` invokes Godot through a PowerShell argument array.

## Verb contract

Every verb returns a `PackagingResult` dictionary:

```gdscript
{
    "verb":        "pack",     # the verb name
    "exit_code":   0,          # mirrored by the runner as the process exit code
    "ok":          true,       # exit_code == 0
    "message":     "Packed ... -> ...",
    "details":     {...},      # verb-specific (artifact paths, parsed config, etc.)
    "stdout":      "...",      # forwarded from underlying tool stdout, may be ""
    "stderr":      "...",      # forwarded from underlying tool stderr, may be ""
    "duration_ms": 1234
}
```

Exit-code categories live in `core/packaging_result.gd`:

| Constant            | Code | Meaning                                        |
|---------------------|------|------------------------------------------------|
| `EXIT_OK`           |   0  | Success.                                       |
| `EXIT_FAIL`         |   1  | Generic verb-ran-but-failed.                   |
| `EXIT_USAGE`        |   2  | Bad CLI arguments / unknown verb or flag.      |
| `EXIT_CONFIG`       |   3  | Required config / env / file missing.          |
| `EXIT_TOOL`         |   4  | Underlying tool (makepkg, wdapp, ...) failed.  |
| `EXIT_UNIMPLEMENTED`|   5  | Verb known to the dispatcher but not handled.  |

## CLI shape

Three equivalent surfaces (callers pick whichever is most convenient):

```
# A — short script path (works without a prior --import; canonical form)
godot --headless -s res://addons/godot_gdk_packaging/run.gd -- \
    <verb> [--key=value] [--key value] [--flag] [-- positional ...]

# B — class_name main loop (shortest official form; needs a prior --import
#     so the class registry is populated)
godot --headless --main-loop GdkPackagingRunner -- \
    <verb> [--key=value] [--key value] [--flag] [-- positional ...]

# C — addon-local shell forwarders (Godot auto-discovered)
addons\godot_gdk_packaging\gdkpkg.cmd <verb> [...]
addons/godot_gdk_packaging/gdkpkg.sh  <verb> [...]
```

The runner emits a one-line summary on every invocation:

```
[packaging] pack ok in 6213ms: Packed C:\Build\content -> C:\Build\out
PACKAGING_RESULT_JSON:{"verb":"pack","exit_code":0,"ok":true,...}
```

Callers parse the marker line out of stdout. `--no-json` suppresses the
marker for interactive use; the summary line and the exit code are always
emitted.

Common runner flags (accepted by every verb): `--help` (`-h`), `--no-json`,
`--config <path>`, `--verbose` (`-v`). There is no CLI `--settings` flag:
`settings_path` exists only on `PackagingConfig.resolve(...)` as a resolver/test
hook, and normal runner invocations always load `res://.gdk_packaging.cfg`.

Verb list (14):

| Verb               | Required flags                | Notes                                                  |
|--------------------|-------------------------------|--------------------------------------------------------|
| `pack`             | `--source-dir`, `--output-dir`| Auto-genmaps if `--map-file` omitted; honours `--no-prepare`. |
| `genmap`           | `--source-dir`, `--map-file`  |                                                        |
| `validate`         | `--source-dir`, `--map-file`  | Optional `--output-dir` selects makepkg `/pd`.          |
| `prepare_content`  | `--content-dir`               | Standalone content-prep step; honors `--config`.        |
| `export`           | `--preset`, `--output-dir`    | `--release`; optional `--no-prepare`.                  |
| `register_loose`   | `--content-dir`               | wdapp register.                                        |
| `install`          | `--package`                   | wdapp install.                                         |
| `uninstall`        | `--package-name`              | wdapp uninstall.                                       |
| `launch`           | `--package-name` or `--aumid` | Resolves AUMID via `wdapp list` when only PFN given.   |
| `terminate`        | `--package-name`              | Falls back to taskkill only for the exact bare `.exe` basename named by MicrosoftGame.config, with path/wildcard/quote characters rejected and the file required in the build dir. |
| `sandbox`          | (none)                        | Defaults to `get`; only `--action set` requires `--sandbox-id`. |
| `config_template`  | (none)                        | Writes a starter MicrosoftGame.config; optional `--output`, `--overwrite`. |
| `config_editor`    | (none)                        | Detached GameConfigEditor.exe launch.                  |
| `store_wizard`     | (none)                        | Detached GameConfigEditor.exe `/StoreAssociation`.     |

## Settings resolver

`packaging_config.resolve(cli_options, project_root="", settings_path=..., config_path_override="")`
collapses every source into a single flat dict. Precedence (highest wins):

1. CLI flags (kebab-case in argv -> snake_case in the resolved dict via
   `_CLI_KEY_REMAP`).
2. `res://.gdk_packaging.cfg` (via `PackagingSettingsStore`). Empty strings
   here do **not** clobber values pulled from lower layers.
3. `MicrosoftGame.config` (identity / product / executable fields). The
   common `--config <path>` flag selects this config file and is carried into
   `prepare_content`, `pack`, and post-export prep.
4. `project.godot` (only `application/config/name` and `version`).
5. Built-in defaults (`encrypt="none"`, `updcompat=3`, `action="get"`, ...).

Derived rules:

- `content_id` falls back to `product_id` if neither CLI nor settings file
  supplied one.
- `--encrypt=key:<ekb>` is split into `encrypt="key"` + `encrypt_key="<ekb>"`
  unless `--encrypt-key` is also passed (CLI wins). A resolved `encrypt="key"`
  with no non-empty `encrypt_key` fails with `EXIT_CONFIG`; the service never
  silently drops the requested encryption mode.
- `config_template --output <path>` writes to the requested path; `--overwrite`
  removes and recreates that same resolved output file.

## Content preparation safety

`PackagingContentPreparer.ensure_content_dir_ready()` validates logo
destination attributes from MicrosoftGame.config after path simplification and
before staging. Relative paths inside the content directory are allowed;
absolute paths, Godot URI paths, and paths that resolve outside the content
directory are rejected with a content-dir safety error before any config or
logo bytes are written to the staging directory.

## Plan

Headless workflows are the supported automation contract. Editor access stays
explicit through the top-level `GDK` menu and modal dialogs; this spec does not
plan a separate editor-panel rewrite.

### Shipped — headless workflows

1. Move 8 headless-safe helpers from `editor\` to `core\`; update preloads.
2. Add `core\packaging_result.gd` (typed dict + exit-code constants).
3. Add `core\packaging_cli.gd` (argv parser + declarative `VERBS` table).
4. Add `core\packaging_config.gd` (settings resolver with precedence chain).
5. Add `core\packaging_service.gd` (verb facade; one method per verb).
6. Add `addons\godot_gdk_packaging\run.gd` (`class_name GdkPackagingRunner`).
7. Add `gdkpkg.cmd` / `gdkpkg.sh` shell forwarders.
8. Add GUT coverage under `tests\godot\gdk\tests\packaging\` for the CLI
   parser, config resolver, and result builder plus the helper suites that
   target the moved `core\` paths.
9. Keep the spec, user-facing reference (`docs\packaging\plugin.md`), editor
   menu reference (`docs\packaging\editor-menu.md`), and path-scoped
   instructions aligned with the live implementation.

### Future — service hardening and editor UX

10. Add targeted service and runner coverage for behaviours that require the
    real GDK toolchain, gated on `is_gdk_available()`.
11. Keep editor entry points menu/dialog based unless a future design explicitly
    changes the editor model.
12. Optional: add a thin `tools\run_packaging.ps1` only if usage patterns demand
    it; the addon-local forwarders remain the supported wrapper today.

## Progress

- **Headless workflow**: shipped in branch `packaging-headless-revamp`. All
  eight pre-existing source modules moved into `core\`; new
  `packaging_result.gd`, `packaging_cli.gd`, `packaging_config.gd`,
  `packaging_service.gd`, `run.gd`, `gdkpkg.cmd`, and `gdkpkg.sh` shipped with
  that work. GUT coverage under `tests\godot\gdk\tests\packaging\` covers the
  resolver, CLI parser, result builder, and helper suites for the moved
  `core\` paths. Verb-level service coverage continues to expand alongside
  real-toolchain validation.
- **Known orchestrator flake (not blocking)**: `tools\run_all_tests.ps1`
  occasionally crashes the `tests\godot\gdk` GUT host with signal 11 after
  ~194/205 passing tests and 0 failures. The backtrace points at
  `_on_startup_user_completed` in `addons\godot_gdk\runtime\gdk_bootstrap.gd`
  during `after_each` runtime resets — pre-existing infrastructure, not
  packaging code. Reproduced on `origin/main` (`11aba6c`) with packaging
  changes stashed. Direct GUT invocations and isolated host runs are clean;
  re-running the orchestrator typically passes on retry. Tracking
  separately; do not gate packaging work on this flake.
- **Editor UX / service hardening**: ongoing. The current editor surface is the
  top-level `GDK` menu plus dialogs; no dock is registered.
