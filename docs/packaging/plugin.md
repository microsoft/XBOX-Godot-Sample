# Godot GDK Packaging — User Reference

`addons\godot_gdk_packaging\` exposes Microsoft GDK PC packaging from
Godot — Microsoft Game Config, makepkg (genmap / pack / validate), wdapp
(register / install / launch / terminate / uninstall), XblPCSandbox, and
the GameConfigEditor / Store Association wizard. It works as a **headless**
runner you can drive from scripts and CI, with a small editor menu for
Game Config and documentation shortcuts.

This page is the headless-surface reference.

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
4. Repo-local `sample\Godot*_console.exe` (highest version wins; for dev hosts)
5. `where godot` / `which godot`

Both forwarders accept `--path <project_dir>` to override the project
root (otherwise the current working directory is used). Pass `--godot
<exe>` to override discovery. The Windows forwarder preserves each
forwarded argument as a separate token, so Godot paths and packaging flags
may contain spaces (for example under `C:\Program Files (x86)\...`).

## Verbs

The runner dispatches one verb per invocation. Common runner flags accepted
on every verb: `--help` (`-h`), `--no-json`, `--config <path>`,
`--verbose` (`-v`).

| Verb               | Required flags                       | Description                                                       |
|--------------------|--------------------------------------|-------------------------------------------------------------------|
| `pack`             | `--source-dir`, `--output-dir`       | makepkg pack. Auto-genmap when `--map-file` is omitted.           |
| `genmap`           | `--source-dir`, `--map-file`         | makepkg genmap.                                                   |
| `validate`         | `--source-dir`, `--map-file`         | makepkg validate; optional `--output-dir` selects `/pd`.          |
| `prepare_content`  | `--content-dir`                      | Copies MicrosoftGame.config + logos into a content directory.     |
| `export`           | `--preset`, `--output-dir`           | Godot Windows-Desktop export; `--release`; optional `--no-prepare`. |
| `register_loose`   | `--content-dir`                      | wdapp register (loose-files build).                               |
| `install`          | `--package`                          | wdapp install on an .msixvc file.                                 |
| `uninstall`        | `--package-name`                     | wdapp uninstall by package full name.                             |
| `launch`           | `--package-name` or `--aumid`        | wdapp launch. Resolves AUMID from PFN when needed.                |
| `terminate`        | `--package-name`                     | wdapp terminate; taskkill fallback is limited to the build's config-named `.exe`. |
| `sandbox`          | `--action {get,set,retail}`          | `set` additionally requires `--sandbox-id`.                       |
| `config_template`  | (none)                               | Writes a starter MicrosoftGame.config; optional `--output`, `--overwrite`. |
| `config_editor`    | (none)                               | Detached GameConfigEditor.exe launch on the current config.       |
| `store_wizard`     | (none)                               | Detached GameConfigEditor.exe `/StoreAssociation`.                |

`pack`-specific flags:

| Flag              | Default  | Description                                                            |
|-------------------|----------|------------------------------------------------------------------------|
| `--content-id`    | `product_id` | Override the `/contentid` value.                                   |
| `--product-id`    |          | Override the `/productid` value (otherwise MicrosoftGame.config wins).|
| `--encrypt`       | `none`   | `none`, `license`, or `key:<ekb-path>`.                                |
| `--encrypt-key`   |          | EKB path (required when `--encrypt=key`).                              |
| `--updcompat`     | `3`      | makepkg `/updcompat` level (1, 2, or 3).                               |
| `--no-prepare`    | off      | Skip the content-prep step before pack.                                |

Additional verb-specific flags:

- `validate --output-dir <dir>` chooses the makepkg validation output `/pd`
  directory (otherwise a `validate-out` sibling is created).
- `export --no-prepare` skips the post-export content preparation step.
- `config_template --output <path>` writes the template at that path;
  `--overwrite` replaces that same resolved output file when it already
  exists.

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
2. **`res://.gdk_packaging.cfg`** — written by the dock. Empty strings
   here do not blow away values from lower layers.
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

- `.github\instructions\godot-gdk-packaging.instructions.md` — repo-wide
  rules for contributors editing the addon.
- `docs\gdk\editor-tools.md` — broader editor-tooling notes.
