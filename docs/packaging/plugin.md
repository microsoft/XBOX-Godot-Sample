# Godot GDK Packaging — User Reference

`addons\godot_gdk_packaging\` exposes Microsoft GDK PC packaging from
Godot — Microsoft Game Config, makepkg (genmap / pack / validate), wdapp
(register / install / launch / terminate / uninstall), XblPCSandbox, and
the GameConfigEditor / Store Association wizard. It works as a **headless**
runner you can drive from scripts and CI, with a top-level editor `GDK` menu
for Game Config, sandbox/package management, and documentation shortcuts.

This page is the headless-surface reference. For editor UI coverage, see
[Godot GDK Packaging — Editor `GDK` Menu](editor-menu.md).

## Invocation

Three equivalent invocations. Pick whichever fits your environment.

### A — Short script path (recommended)

Works in every host, including before a `--import` pass:

```pwsh
godot --headless -s res://addons/godot_gdk_packaging/run.gd -- `
    pack --source-dir Build\content --output-dir Build\out
```

### B — `--main-loop` class name

Shortest official form. Requires a prior `--headless --import` against the
host project so the `class_name GdkPackagingRunner` registration is in
place:

```pwsh
godot --headless --import        # one-time per host
godot --headless --main-loop GdkPackagingRunner -- `
    pack --source-dir Build\content --output-dir Build\out
```

### C — Shell forwarders

The addon ships its own per-OS forwarders that auto-discover Godot and
default the project path to the current working directory:

```pwsh
# Windows (cmd / PowerShell)
addons\godot_gdk_packaging\gdkpkg.cmd pack --source-dir Build\content --output-dir Build\out

# POSIX (bash / zsh)
addons/godot_gdk_packaging/gdkpkg.sh pack --source-dir Build/content --output-dir Build/out
```

Godot discovery order in the forwarders:

1. `GODOT_CONSOLE` env var (full path to a console-enabled Godot binary)
2. `GODOT_BIN` env var
3. `GODOT` env var
4. Repo-local `sample\Godot*_console.exe` / `sample\Godot*.exe` candidates
5. Current-working-directory `Godot*_console.exe` / `Godot*.exe` candidates
6. `where godot` / `where godot4` on Windows, or `command -v godot` /
   `command -v godot4` on POSIX shells (`gdkpkg.sh` uses `command -v` for
   PATH lookup, not `which`)

Both forwarders accept `--path <project_dir>` to override the project
root (otherwise the current working directory is used). Pass `--godot
<exe>` to override discovery. The Windows forwarder preserves each
forwarded argument as a separate token, so Godot paths and packaging flags
may contain spaces (for example under `C:\Program Files (x86)\...`).

## Verbs

The runner dispatches one verb per invocation. Common runner flags accepted
on every verb: `--help` (`-h`), `--no-json`, `--config <path>`,
`--verbose` (`-v`).

### Export prerequisites

The `export` verb uses Godot's Windows Desktop exporter, so install the Godot
4.6.1 export templates under `%APPDATA%\Godot\export_templates\` first.
MSIXVC and loose-registration flows also need the public GDK toolchain on
`PATH` (`makepkg.exe`, `wdapp.exe`, `XblPCSandbox.exe`), or `GDK_BIN` pointing
at the GDK `bin\` directory.

| Verb               | Required flags                 | Optional flags                                                                 | Description                                                       |
|--------------------|--------------------------------|---------------------------------------------------------------------------------|-------------------------------------------------------------------|
| `pack`             | `--source-dir`, `--output-dir` | `--map-file`, `--content-id`, `--product-id`, `--encrypt`, `--encrypt-key`, `--updcompat`, `--no-prepare` | makepkg pack. Auto-generates a map file when `--map-file` is omitted; `--no-prepare` only skips content prep. |
| `genmap`           | `--source-dir`, `--map-file`   | (none)                                                                          | makepkg genmap.                                                   |
| `validate`         | `--source-dir`, `--map-file`   | `--output-dir`                                                                  | makepkg validate; `--output-dir` selects `/pd`, otherwise a `validate-out` sibling is created. |
| `prepare_content`  | `--content-dir`                | (none)                                                                          | Copies MicrosoftGame.config + logos into a content directory.     |
| `export`           | `--preset`, `--output-dir`     | `--release`, `--no-prepare`                                                     | Godot Windows-Desktop export; `--no-prepare` skips post-export content preparation. |
| `register_loose`   | `--content-dir`                | (none)                                                                          | wdapp register (loose-files build).                               |
| `install`          | `--package`                    | (none)                                                                          | wdapp install on an .msixvc file.                                 |
| `uninstall`        | `--package-name`               | (none)                                                                          | wdapp uninstall by package full name.                             |
| `launch`           | `--package-name` or `--aumid`  | (none)                                                                          | wdapp launch. `run_launch()` uses `--aumid` directly when present; otherwise it resolves the AUMID from `wdapp list` by matching `--package-name`. |
| `terminate`        | `--package-name`               | (none)                                                                          | wdapp terminate; taskkill fallback is limited to the build's config-named `.exe`. |
| `sandbox`          | (none)                         | `--action {get,set,retail}`, `--sandbox-id`                                     | Defaults to `get`; `--action set` also requires `--sandbox-id`.   |
| `config_template`  | (none)                         | `--output`, `--overwrite`                                                       | Writes a starter MicrosoftGame.config; `--output` redirects the template path (relative paths resolve under the project root) and `--overwrite` replaces that same file. |
| `config_editor`    | (none)                         | (none)                                                                          | Detached GameConfigEditor.exe launch on the current config.       |
| `store_wizard`     | (none)                         | (none)                                                                          | Detached GameConfigEditor.exe `/StoreAssociation`.                |

`--encrypt=key` without a non-empty `--encrypt-key` fails with `EXIT_CONFIG`;
the pack verb never silently downgrades a key-encrypted build to unencrypted.

Per-verb help is always reachable as `<verb> --help`:

```pwsh
addons\godot_gdk_packaging\gdkpkg.cmd pack --help
```

## Output format

Every invocation prints a one-line human summary plus (by default) a
single line of JSON prefixed with `PACKAGING_RESULT_JSON:` so callers can
grep one canonical marker out of a log:

```
[packaging] pack ok in 6213ms: Packed C:\Build\content -> C:\Build\out
PACKAGING_RESULT_JSON:{"verb":"pack","exit_code":0,"ok":true,"message":"...","details":{...},"stdout":"...","stderr":"","duration_ms":6213}
```

Pass `--no-json` to suppress the marker line for terminal use. The
process exit code mirrors the `exit_code` field. Underlying tool diagnostics
are reported in `stderr` instead of being merged into `stdout`.

### Exit-code reference

| Code | Constant            | Meaning                                        |
|------|---------------------|------------------------------------------------|
|   0  | `EXIT_OK`           | Success.                                       |
|   1  | `EXIT_FAIL`         | Generic verb-ran-but-failed.                   |
|   2  | `EXIT_USAGE`        | Bad CLI arguments / unknown verb or flag.      |
|   3  | `EXIT_CONFIG`       | Required config / env / file missing.          |
|   4  | `EXIT_TOOL`         | Underlying tool (makepkg, wdapp, ...) failed.  |
|   5  | `EXIT_UNIMPLEMENTED`| Verb known but not yet implemented.            |

## Settings precedence

`packaging_config.gd` collapses every source into a single flat dict
(highest precedence wins):

1. **CLI flags** (kebab-case). E.g. `--source-dir C:\Build\content`.
2. **`res://.gdk_packaging.cfg`** — persisted editor settings. Empty strings
   here do not blow away values from lower layers. The settings file contains:
   - `[packaging]`: `source_dir`, `map_file`, `auto_genmap`, `output_dir`,
     `content_id`, `product_id`, `encrypt_option`, `encrypt_key`,
     `updcompat_option`
   - `[sandbox]`: `sandbox_id`, `test_account`
   - `[export]`: `preset_name`, `clean_build`
3. **`MicrosoftGame.config`** at `<project>\MicrosoftGame.config` (override
   with the runner's `--config <path>` flag) — provides `product_id`,
   identity fields, and the executable name.
4. **`project.godot`** — only `application/config/name` and
   `application/config/version`.
5. Built-in defaults (`encrypt="none"`, `updcompat=3`, `action="get"`).

Derived rules:

- `content_id` falls back to `product_id` when neither CLI nor settings
  supplied one.
- `--encrypt=key:<ekb>` is shorthand for `--encrypt key --encrypt-key <ekb>`.
- `--config <path>` is honored by the resolver and by content preparation, so
  `prepare_content`, `pack`, and post-export prep stage the selected config
  instead of implicitly reading `res://MicrosoftGame.config`.
- `config_template --output <path>` writes to that path; relative paths are
  resolved under the project root, and `--overwrite` removes and recreates that
  same resolved file.
- `auto_genmap`, `test_account`, and `clean_build` are persisted editor UI
  state. They are listed here for completeness, but the headless resolver does
  not treat them as CLI-equivalent overrides.

## Content-directory safety

`prepare_content`, `pack`, and `export` validate every logo destination path
read from MicrosoftGame.config before copying logo bytes. Relative paths such
as `storelogos\Square150x150Logo.png` are staged under the content directory;
absolute paths or paths that simplify outside the content directory (for
example `..\..\outside.png`) are refused with an error before staging begins.

## Environment variables

| Variable                          | Purpose                                                                       |
|-----------------------------------|-------------------------------------------------------------------------------|
| `GODOT_CONSOLE` / `GODOT_BIN` / `GODOT` | Shell-forwarder Godot discovery (see Invocation).                       |
| `GDK_BIN`                         | Override the GDK install bin directory (otherwise `C:\Program Files (x86)\Microsoft GDK\bin`). |
| `GameDKCoreLatest`                | Standard GDK install marker; used to detect the GDK version when present.     |

## Examples

```pwsh
# Drop a starter MicrosoftGame.config in the current project, or redirect it.
addons\godot_gdk_packaging\gdkpkg.cmd config_template
addons\godot_gdk_packaging\gdkpkg.cmd config_template --output Configs\Alt.config

# Get the current sandbox.
addons\godot_gdk_packaging\gdkpkg.cmd sandbox --action get

# Set the sandbox without ousting installed apps.
addons\godot_gdk_packaging\gdkpkg.cmd sandbox --action set --sandbox-id XDKS.1

# Export, prepare, and pack in three steps.
addons\godot_gdk_packaging\gdkpkg.cmd export --preset "Windows Desktop" --output-dir Build\content
addons\godot_gdk_packaging\gdkpkg.cmd prepare_content --content-dir Build\content --config Configs\Alt.config
addons\godot_gdk_packaging\gdkpkg.cmd pack --source-dir Build\content --output-dir Build\out --config Configs\Alt.config

# Register a loose-files build, launch, then terminate.
addons\godot_gdk_packaging\gdkpkg.cmd register_loose --content-dir Build\content
addons\godot_gdk_packaging\gdkpkg.cmd launch --package-name MyPublisher.MyGame_1.0.0.0_x64__abc123
addons\godot_gdk_packaging\gdkpkg.cmd terminate --package-name MyPublisher.MyGame_1.0.0.0_x64__abc123
```

## Troubleshooting

- **`gdkpkg.cmd` can't find Godot.** Set one of `GODOT_CONSOLE`,
  `GODOT_BIN`, or `GODOT` to a full path. The forwarder prints which
  candidate it tried; pass `--godot <path>` to short-circuit discovery.
- **`EXIT_CONFIG: wdapp.exe not found`.** Set `GDK_BIN` to point at your
  GDK install's `bin\` directory, or install the GDK to the default
  location.
- **`terminate` falls back to `wdapp` failure instead of `taskkill`.** The
  fallback only targets the exact executable named by `MicrosoftGame.config`
  and only when that file exists in the build directory. The config value
  must be a bare `.exe` file name (no path separators, drive prefixes,
  quotes, or wildcards); extra `.exe` files are intentionally ignored.
- **`PACKAGING_RESULT_JSON:` line missing.** You passed `--no-json`. Drop
  it to re-enable the marker.
- **Form B (`--main-loop GdkPackagingRunner`) reports unknown class.** The
  host project hasn't been imported yet. Run `godot --headless --import`
  once against the host, or use form A (`-s ...`) — it doesn't require
  prior import.
- **GDK runtime warnings on every invocation** (e.g. silent sign-in
  cancellation). Those come from the host project's autoloaded
  `GDKBootstrap`. They are unrelated to the packaging verb result and
  can be ignored when inspecting `PACKAGING_RESULT_JSON:`.

## See also

- [Godot GDK Packaging — Editor `GDK` Menu](editor-menu.md) — what each
  editor menu item does and what prerequisites it needs.
- `.github\instructions\godot-gdk-packaging.instructions.md` — repo-wide
  rules for contributors editing the addon.
- `docs\gdk\editor-tools.md` — broader editor-tooling notes.
