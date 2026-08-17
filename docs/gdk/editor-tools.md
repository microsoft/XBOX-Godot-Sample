# Godot Microsoft GDK editor tools

This document explains the current editor-side split between the `godot_gdk`
runtime addon and the separate `godot_gdk_editortools` tooling addon.

See also:

- [`gdk/plugin.md`](plugin.md)
- [`gdk/build-and-loading.md`](build-and-loading.md)

## Current state

`godot_gdk` still ships these editor-side scripts:

- `gdk_editor_plugin.gd`
- `gdk_setup_panel.gd`
- `gdk_export_platform.gd`

The runtime addon's editor plugin owns startup wiring for the runtime addon
**and** registers the custom `XBOX on PC` export platform so it appears
in the editor's `Project > Export… > Add…` dropdown alongside the
built-in platforms.

The `godot_gdk_editortools` addon hosts the wider editor workflow
(top-level **Microsoft GDK** menu, sandbox switcher, `MicrosoftGame.config`
flows, tutorial wizard) and the headless packaging CLI.

## `gdk_editor_plugin.gd`

The current `godot_gdk` editor plugin is intentionally narrow.

Today it:

- installs or updates the addon-owned `XboxBootstrap` autoload on
  `_enable_plugin`
- registers the `XBOX on PC` export platform on `_enter_tree`
- registers `gdk_export_features_plugin.gd`, the `EditorExportPlugin` that
  restores the `gdk` feature tag during a delegated export (see
  *Feature tags under delegation*)
- does **not** dock `gdk_setup_panel.gd`

Both registrations are undone in `_exit_tree`, so disabling the addon leaves
nothing behind.

`gdk_editor_plugin.gd` owns startup wiring for the runtime addon and
the export-platform registration; the rest of the packaging UI lives in
`godot_gdk_editortools`.

## `gdk_setup_panel.gd`

`gdk_setup_panel.gd` remains in the addon and is still synced into the sample
projects, but it is not auto-registered by the current editor plugin.

Its job is still useful as a local configuration helper:

- render Partner Center identity fields
- load and save `res://sample_config.cfg`
- prepopulate values from `MicrosoftGame.config` when present
- push selected values into `export_presets.cfg`
- normalize the title id to 8 hex digits

Treat it as a retained sample/config utility rather than the primary packaging UI.

## `gdk_export_platform.gd`

`gdk_export_platform.gd` implements the custom `XBOX on PC` export
platform. It is registered by `gdk_editor_plugin.gd` on `_enter_tree`,
so the platform appears in the editor's `Project > Export… > Add…`
dropdown alongside Windows Desktop, Linux, etc.

It is **not** the only packaging path: the headless packaging runner in
`godot_gdk_editortools` (`tools/run.gd` / `gdkpkg.cmd`) remains the
canonical entry point for headless package, validate, install, and launch
automation. The export platform exists for editor-driven workflows; the
headless runner exists for CI and scripted automation. Keep both paths
working when the underlying packaging primitives change.

### Export failure diagnostics

The final export step shells out to a GDK tool — `wdapp register` when
**Dev Iteration ▸ Register Loose** is enabled, otherwise `makepkg genmap`
+ `makepkg pack`. These tools do **not** report a meaningful process exit
code (it is usually a small value such as `2` or `3`); the real cause is an
HRESULT (`error = 0x8007xxxx`, a `FACILITY_WIN32` code) that they print
across **both** stdout and stderr, depending on the sub-command.

`gdk_export_platform.gd` captures both streams (via `OS.execute_with_pipe`),
echoes the **full** output to the editor Output panel tagged by stream,
extracts the HRESULT, and surfaces it — with a short hint for the common
codes (`0x80070002` file not found, `0x80070003` path not found,
`0x80070005` access denied, `0x80070057` invalid config) plus the **tail**
of the captured output — in the error shown by the export dialog. The dialog
text is capped to the last handful of lines so a large tool log stays
readable; the full log remains in the Output panel.

A failed tool step returns `FAILED`, **not** `ERR_BUG`. Returning `ERR_BUG`
(Godot `Error` value `47`) is what produced the opaque *"unexpected error
code 47"* reported in issue #123; the packaging step never actually
"hit a bug" — the underlying tool failed for a reportable reason, so the
diagnostic must carry that reason instead of an internal error code.

### Template and GDExtension-DLL prerequisites (issue #134)

Two silent ways an `XBOX on PC` export used to produce a package that failed at
launch with *"GDExtension dynamic library not found"* are now hard errors:

- **No export template for the running Godot version.** The `.exe` and `.pck`
  are produced by Godot's own `EditorExportPlatformWindows` (see
  *Delegating the .exe and .pck to Godot* below), which resolves
  `<export templates dir>/<version>/windows_<config>_x86_64.exe`. That means the
  version directory name (including the patch component, e.g. `4.6.2.stable`,
  and the `.mono` suffix on a .NET editor build), self-contained mode, and a
  relocated editor data directory are all handled by the engine instead of being
  re-derived here. It is also strict: there is no fallback to a near-miss
  version directory, so a 4.6.2 editor never silently packages a 4.6 template.
  There is **no** editor-binary stand-in — that binary makes
  `OS.has_feature("editor")` true at runtime and resolves the GDExtension from
  the dev machine's source tree, so a package built from it fails on any other
  machine. A missing template blocks a **loose** dev-register build too. Install
  export templates via **Editor ▸ Manage Export Templates…** (match your patch
  version). Validation performs the same lookup up front so the failure appears
  as a greyed-out **Export Project…** with a reason, not a failed export.

- **No GDExtension DLL for the requested config.** A **release** export stages
  `godot_gdk.windows.release.x86_64.dll`; a **debug** export stages the debug
  DLL. `cmake --build build --preset debug` (the default) only produces the
  debug DLL, so a release export after a debug-only build would stage zero
  GDExtension DLLs. The exporter now aborts with the exact build commands
  (`cmake --preset default-release` then `cmake --build --preset release`)
  instead of shipping a DLL-less
  package. In the export dialog, **Export With Debug** unchecked selects release.


### Texture-format feature tags (issue #144)

A related — and far more destructive — silent failure: an `XBOX on PC` package
whose `.pck` came out a **fraction** of the size the same project's
`Windows Desktop` export produced, with the game dying at load on
*"Can't load dependency: `res://…`"* for every texture.

Godot resolves an imported resource through the `[remap]` section of its
`.import` file, and a **VRAM-compressed** texture has no plain `path` key at
all — only feature-tagged variants:

```ini
[remap]

importer="texture"
path.s3tc="res://.godot/imported/tex.png-<hash>.s3tc.ctex"
```

When it writes the `.pck`, Godot compares each `path.<feature>` key against the
feature tags the platform reports from `_get_platform_features()` +
`_get_preset_features()`, and **erases** every remap whose feature is missing.
This platform used to report only `["windows", "gdk", "x86_64"]`, so `s3tc`
never matched: every VRAM-compressed texture was dropped from the package,
leaving the `.ctex` its scenes and materials reference unresolvable.

Since the `.pck` is now written by the built-in Windows platform (see
*Delegating the `.exe` and `.pck` to Godot*), the tags Godot actually consults
are its own — derived from the very same preset options below. This platform
derives them identically in `_get_preset_features()`, via two export options
that mirror the built-in Windows preset:

| Option | Default | Feature tags reported |
| --- | --- | --- |
| `texture_format/s3tc_bptc` | `true` | `s3tc`, `bptc` |
| `texture_format/etc2_astc` | `false` | `etc2`, `astc` |

Existing `export_presets.cfg` files pick the defaults up automatically — Godot
seeds a preset from the platform's option defaults before applying the stored
values — so `s3tc`/`bptc` are reported without touching the preset.

Leave **S3TC/BPTC** enabled for any desktop/Xbox target. Turning it off is what
reproduces the empty-package failure above; the option exists only so a project
that also imports mobile texture variants can control which set ships.

### Delegating the `.exe` and `.pck` to Godot (issues #134, #144)

A GDK title on PC is an ordinary Win32 application: it runs the stock Godot
Windows export template on a Windows desktop. The only things that legitimately
differ from a **Windows Desktop** export are MSIXVC packaging via `makepkg`, the
GDK staging steps (`MicrosoftGame.config`, logos, addon support DLLs), and the
`gdk` feature tag.

This platform used to reimplement the whole desktop pipeline anyway — copy a
template, stamp its icon, stage the D3D12 redistributables, write the `.pck`,
place the export plugins' shared objects — and every one of those steps was a
chance to drift from what Godot produces. Issues #134 and #144 were both
instances of that drift:

- a package shipped with **no C# assemblies**, because `export_pack()` discards
  the shared objects the .NET export plugin registers (and double-fires every
  plugin's `_export_begin` / `_export_end`);
- a package shipped with **no VRAM-compressed textures**, because the platform
  reported the wrong feature tags;
- a package shipped with the **stock Godot template icon**, because rcedit was
  never invoked — and then Godot **removed rcedit entirely in 4.5**, replacing
  it with a native `TemplateModifier` that is not exposed to GDScript, so an
  rcedit-based icon path could not work on any version this addon supports.

So `_export_project()` no longer builds the binaries. It hands **its own
preset** to the engine's `EditorExportPlatformWindows`:

```gdscript
var windows: Object = ClassDB.instantiate("EditorExportPlatformWindows")
var err: int = windows.export_project(p_preset, p_debug, exe_path, p_flags)
```

and then layers the GDK-specific steps on the result:

1. `_copy_addon_dlls()` — addon support runtimes and the `addons/<name>/bin/`
   layout the `.gdextension` expects.
2. `_stage_microsoft_game_config()` — identity and shell visuals.
3. `_stage_logos()` — the images `MicrosoftGame.config` references.
4. `_wdapp_register()` or `_makepkg_pack()`.

Everything else now comes from the engine and stays correct on its own:
template resolution, PE icon and `VERSIONINFO` stamping, the console wrapper,
`binary_format/embed_pck`, Authenticode signing, the D3D12 Agility SDK and PIX
runtimes, ANGLE, shader baking, and every export plugin's shared objects
(a C# project's published assemblies among them).

**Passing our own preset is the point.** The user's export filters, encryption
settings, script export mode, custom features and every mirrored option are read
straight off the preset they actually edited — there is no copy step that can
fall out of date. Verified by exporting the same project both ways: the `.exe`
and `.pck` this produces are **byte-identical** (SHA-256) to a built-in
`Windows Desktop` export of the same project.

#### The export option table is load-bearing

Delegation only works because `_get_export_options()` declares **every** option
name the built-in exporter reads — all 38: `EditorExportPlatformWindows`'
31 plus its `EditorExportPlatformPC` base's 7, which are identical on Godot 4.5
and 4.6. A missing name reads back as `null` *inside Godot*, and the failure is
neither obvious nor local: omitting `custom_template/debug` aborts the export
with *"Mismatching custom export template executable architecture: found
'invalid'"*.

Only 18 of those options are discoverable at runtime from a preset — Godot
filters advanced-only options out of `get_property_list()`, and
`advanced_options` cannot be set from GDScript — so the list is written out
explicitly and pinned by `test_export_platform_errors.gd`. If a future Godot
version adds a Windows export option, that test fails instead of an export.
Regenerate the expected set by grepping `PropertyInfo(Variant::` out of
`platform/windows/export/export_plugin.cpp` and
`editor/export/editor_export_platform_pc.cpp`.

Option **visibility** mirrors Godot's grouping too: the executable-resource
fields collapse behind **Application ▸ Modify Resources** (the renderer options
`export_angle` / `export_d3d12` / `d3d12_agility_sdk_multiarch` stay visible, as
in Godot), the signing fields behind **Codesign ▸ Enable**, and the SSH fields
behind **SSH Remote Deploy ▸ Enabled**. Hiding is cosmetic — every option stays
declared and readable by the delegate.

#### Feature tags under delegation

Export feature tags come from the platform *performing* the export. Under
delegation that is the Windows platform, so it reports `pc` / `windows` — the
same list this platform reports — but `gdk` would silently disappear from every
packaged build.

`gdk_export_features_plugin.gd` restores it. An `EditorExportPlugin`'s
`_get_export_features()` is consulted whichever platform is running, so the
plugin returns `["gdk"]` while `XboxExportPlatform.exporting_for_gdk` is set —
which is only for the duration of a delegated GDK export, leaving a plain
`Windows Desktop` export in the same editor session untouched. The plugin is
registered and unregistered alongside the platform in `gdk_editor_plugin.gd`.

`_get_platform_features()` reports `pc`, `windows` and `gdk`, mirroring
`EditorExportPlatformPC` plus the one tag genuinely specific to this platform.
It previously also reported `xbox` and `d3d12`, which Godot defines for no
platform. Because feature tags drive `OS.has_feature()`, `ProjectSettings`
`setting.<tag>` overrides, **and** which `path.<feature>` remaps survive into
the `.pck`, those invented tags made the same project resolve differently
depending on whether it was packaged here or through `gdkpkg` (which drives the
built-in Windows Desktop preset). The architecture tag is reported by
`_get_preset_features()`, which is where Godot puts it.

#### Diagnostics from the delegate

The delegate is a bare instance, so its message log would die with it. Every
message it produced is copied onto this platform via `_forward_messages()`, so
the export dialog attributes the whole export to a single run. A delegated
export that returns `OK` is still checked against a real executable on disk
before packaging proceeds.

### What is still implemented here

**Export dialog diagnostics.** Export failures are reported through
`add_message()` as well as `push_error()`/`push_warning()`, so they appear in the
export dialog's own message log with a severity instead of only in the Output
panel, which the dialog does not surface. `_export_project()` calls
`clear_messages()` first so each run's log reflects only that run. The dialog
renders the category (`GDK Export`) beside each message, so the console-facing
`GDK Export: ` prefix is stripped from the dialog copy only. Validation
failures continue to use `set_config_error()` — see below.

**Validation.** `has_valid_export_configuration()` is not exposed to GDScript,
so it cannot be delegated. This platform keeps its own pre-flight check —
GDK installed, `MicrosoftGame.config` present, export template resolvable — so
the export dialog can grey out the button *with a reason* before anything runs.

**The executable name.** Derived from `<Executable Name="…">` in the project's
`MicrosoftGame.config` rather than from a preset field, so the packaged
identity and the staged binary cannot disagree.

### Verified against a real export

The behaviour above was confirmed end to end on a Windows machine with the
Microsoft GDK installed, exporting a probe project through the `XBOX on PC`
preset and inspecting the staging folder:

- A control experiment exported the same project twice — once through this
  platform, once through a built-in `Windows Desktop` preset — and the `.exe`
  and `.pck` were **byte-identical** (matching SHA-256) both times.
- The icon and `VERSIONINFO` strings were stamped natively by Godot: the colour
  extracted from the packaged `.exe`'s PE icon resource matched the source icon,
  and `CompanyName` / `FileVersion` matched the preset.
- A VRAM-compressed texture whose `.import` carried only a `path.s3tc` entry was
  present in the `.pck`, which is the fix for issue #144 observed directly.
- The D3D12 Agility SDK DLLs were staged into `x86_64\` with the architecture
  suffix stripped, matching the built-in exporter's layout.
- `wdapp register` completed against the staged folder, and the probe app was
  unregistered afterwards.


### Why "Export Project…" is greyed out

Godot enables the export dialog's **Export Project…** / **Export All…** buttons
purely from what the platform's validation callbacks return, and displays *no*
explanation unless the platform supplies one via `set_config_error()`. A bare
`return false` therefore left the user staring at a dead button.

`_has_valid_export_configuration()` now collects **every** blocker in one pass
and reports them together:

| Blocker | What the dialog says |
| --- | --- |
| Microsoft GDK not installed / no `makepkg.exe` + `wdapp.exe` | install command (`winget install Microsoft.Gaming.GDK`) and "restart the editor" |
| `MicrosoftGame.config` missing from the configured game-config directory | the exact path probed, the **GDK ▸ Create Game Config…** menu entry, the `.template` fallback, and the `gdk/packaging/game_config_dir` Project Setting |
| No Windows export template for the running Godot version | the failing config (`debug`/`release`), the Godot version, and **Editor ▸ Manage Export Templates…** |
| No Windows **.NET/Mono** export template, on a project containing C# | the same, plus that the .NET template *package* is required rather than the standard one |
| A configured `custom_template/debug`\|`release` that does not exist | the failing path and the option that names it — never silently swapped for an installed template |

The template check also calls `set_config_missing_templates(true)` so Godot
shows its built-in **Manage Export Templates** shortcut. It applies to **every**
export, including **Dev ▸ Register Loose**: the `.exe` is produced by Godot's
own Windows exporter, so there is no editor-binary stand-in that a loose build
could fall back to. `_has_valid_project_configuration()` reports the
missing-GDK reason through the same message helper.

Each blocker is reported independently, so a project missing both the GDK and
`MicrosoftGame.config` sees both lines instead of only the first.

### .NET export templates for C# projects (".NET assemblies not found")

Godot appends its module config to the export-template directory name, so a
**.NET editor build** installs its templates under `4.7.1.stable.mono`, not
`4.7.1.stable`. A hand-rolled probe that looked only at the plain name found
nothing on a .NET editor, and the export used to fall back to
`OS.get_executable_path()` — the Godot **editor** binary.

That fallback was fatal for a C# game. The editor binary is built with
`TOOLS_ENABLED`, so it resolves .NET assemblies from `<exe_dir>/GodotSharp/Api/`
instead of the exported `data_<assembly>_windows_x86_64/` directory, and the
game dies on startup with:

> Unable to find the .NET assemblies directory. Make sure the
> `…/GodotSharp/Api/Debug` directory exists and contains the .NET assemblies.

Both halves of this are closed:

- The `.mono` suffix is handled by the engine's own resolver, which uses the
  running editor's version config — so a .NET editor only ever resolves a .NET
  template, and a non-.NET template (which has no assembly loader and could
  never host a C# game) is never offered as a fallback.
- The editor-binary fallback is gone entirely. The `.exe` comes from
  `EditorExportPlatformWindows`, which requires a real template for every
  export, loose dev-register included.

`_is_dotnet_project()` (a .NET editor *and* a non-empty
`dotnet/project/assembly_name`) survives only to make the validation message
name the .NET template package specifically, so the user is not sent back to the
same instruction that just failed them.

A C# project's published assemblies now reach the package the same way they
reach a `Windows Desktop` export: the .NET export plugin registers them as
shared objects and the built-in exporter places them in
`data_<assembly>_windows_x86_64/`. Projects that enable **Embed Build Outputs**
are unaffected either way — in that mode the plugin writes the assemblies into
the `.pck` instead.

### GDK detection lifecycle

The platform locates the Microsoft GDK (install root + `makepkg.exe` /
`wdapp.exe`) once and caches the result in `_gdk_found`; the export
configuration checks (`_has_valid_export_configuration`,
`_has_valid_project_configuration`) and `_export_project` all gate on it.

Detection runs **lazily**, guarded by a one-shot `_ensure_detected()` helper,
rather than depending on the engine calling the platform's `_initialize()`.
Godot only began calling `EditorExportPlatform::initialize()` from
`add_export_platform()` in **4.6**; on the supported **4.5.x** line
`_initialize()` never fires, so eager-only detection left `_gdk_found` stuck at
`false` and the `XBOX on PC` platform refused to export (issue #127).
`_ensure_detected()` triggers detection from `_initialize()` (eager, on 4.6+)
**and** from the first export/validation callback (lazy, on 4.5.x), while the
guard keeps the repeatedly-polled validation callbacks from re-scanning the
filesystem or re-emitting "GDK not found" warnings.

## `godot_gdk_editortools`

The active editor tooling for Microsoft GDK packaging now lives in
`addons\godot_gdk_editortools\editor\`.

Treat the root `addons\godot_gdk_editortools\` tree as the source of truth. The
repo build syncs that addon into the sample mirrors under
`sample\...\addons\godot_gdk_editortools\`, so contributors should edit the root
addon rather than patching sample copies directly.

That addon owns:

- the top-level **Microsoft GDK** editor menu
- headless package, validate, install, and launch actions
- tutorial/help surfaces
- `MicrosoftGame.config` helper flows

When discussing the current editor workflow, treat `godot_gdk` and
`godot_gdk_editortools` as separate layers:

- `godot_gdk` owns the runtime/services addon
- `godot_gdk_editortools` owns the supported packaging/editor workflow

## Relationship to the runtime addon

The Microsoft GDK runtime addon still matters to the editor story because samples and
packaging flows need title identity and synced addon payloads, but the runtime
addon is no longer the main place to look for packaging UI behavior.

That means editor-side documentation should describe:

- the retained `godot_gdk` helper scripts accurately
- the active packaging workflow under `godot_gdk_editortools`
- the separation between runtime/services behavior and editor tooling
