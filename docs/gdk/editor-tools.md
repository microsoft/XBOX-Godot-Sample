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

- installs or updates the addon-owned `GDKBootstrap` autoload on
  `_enable_plugin`
- registers the `XBOX on PC` export platform on `_enter_tree`
- does **not** dock `gdk_setup_panel.gd`

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

- **No export template for the running Godot version.** The exporter looks for
  `%APPDATA%\Godot\export_templates\<version>\windows_<config>_x86_64.exe` and
  now tries the patch-qualified directory first (`4.6.2.stable`), then the
  patch-less form (`4.6.stable`, used by `x.y.0` releases). If no template is
  found it **refuses to package** rather than substituting the Godot editor
  binary — that binary makes `OS.has_feature("editor")` true at runtime and
  resolves the GDExtension from the dev machine's source tree, so a package
  built from it fails on any other machine. A **loose** dev-register build still
  tolerates the editor binary, with a loud warning. Install export templates via
  **Editor ▸ Manage Export Templates…** (match your patch version).

- **No GDExtension DLL for the requested config.** A **release** export stages
  `godot_gdk.windows.release.x86_64.dll`; a **debug** export stages the debug
  DLL. `cmake --build build --preset debug` (the default) only produces the
  debug DLL, so a release export after a debug-only build would stage zero
  GDExtension DLLs. The exporter now aborts with the exact build command
  (`cmake --build build --preset release`) instead of shipping a DLL-less
  package. In the export dialog, **Export With Debug** unchecked selects release.

### Export-plugin shared objects / C# assemblies (issue #144)

A third silent failure mode was that an `XBOX on PC` export produced a package
containing **no C# assemblies at all** — the `data_<assembly>_windows_x86_64`
folder that a Godot .NET build normally places next to the `.exe` was simply
absent, so a C# project failed at launch.

Godot hands an export platform two separate things: the resource `.pck`, and a
list of **shared objects** that export plugins registered via
`add_shared_object()` during `_export_begin`. The C# export plugin publishes the
managed assemblies to a temp directory and registers **every published file** as
a shared object targeted at `data_<assembly>_windows_x86_64/`. That list is the
*only* channel through which those assemblies reach a package.

The platform used to build the `.pck` with `EditorExportPlatform.export_pack()`,
which is the wrong primitive for a custom platform:

- it calls `save_pack()` with a **null shared-object sink**, discarding every
  registered shared object, and
- it opens a **second** `ExportNotifier` inside the one
  `EditorExportPlatformExtension` already opens around `_export_project()`, so
  each export plugin's `_export_begin` / `_export_end` runs **twice** (the C#
  publish ran twice) and the inner `_export_end` deleted the C# publish temp
  directory before the platform could copy anything out of it.

`_export_pck()` now calls `EditorExportPlatform.save_pack()` — available since
Godot 4.4 — which returns `{"result": Error, "so_files": Array}`. Each entry is
`{path, tags, target_folder}`; `_stage_shared_objects()` copies it into the
staging directory, placing an entry with an empty `target_folder` next to the
`.exe` and everything else inside that folder, exactly as Godot's built-in
desktop exporter does. Targets that would escape the staging root are rejected.

Shared objects living directly in `addons/<name>/bin/` are skipped, because
`_copy_addon_dlls()` already stages those with debug/release filtering and the
`addons/` layout the `.gdextension` expects; staging them twice would also leave
an unfiltered duplicate in the package root. Shared objects from anywhere else
(the C# publish temp dir, a GDExtension that does not use the `bin/` layout) are
staged by `_stage_shared_objects()`.

Projects that enable **Embed Build Outputs** are unaffected either way: in that
mode the C# plugin writes the assemblies into the `.pck` via `add_file()`
instead of registering shared objects.

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
| No Windows export template for the running Godot version (packaged export only) | the failing config (`debug`/`release`), the Godot version, and **Editor ▸ Manage Export Templates…** |
| No Windows **.NET/Mono** export template, on a project containing C# | the same, plus that the .NET template package is required and that loose dev-register cannot substitute for it |

The template check also calls `set_config_missing_templates(true)` so Godot
shows its built-in **Manage Export Templates** shortcut. It is skipped when
**Dev ▸ Register Loose** is on, because a loose dev-register build does not
need a template — *except* on a C# project, which always needs a real .NET
template (see below). `_has_valid_project_configuration()` reports the
missing-GDK reason through the same message helper.

Each blocker is reported independently, so a project missing both the GDK and
`MicrosoftGame.config` sees both lines instead of only the first.

### .NET export templates for C# projects (".NET assemblies not found")

`_find_windows_template()` probes
`%APPDATA%\Godot\export_templates\<version>\windows_<config>_x86_64.exe`. Godot
appends its module config to that directory name, so a **.NET editor build**
installs its templates under `4.7.1.stable.mono`, not `4.7.1.stable`. Probing
only the plain name found nothing on a .NET editor, and the export silently fell
back to `OS.get_executable_path()` — the Godot **editor** binary.

That fallback is fatal for a C# game. The editor binary is built with
`TOOLS_ENABLED`, so it resolves .NET assemblies from `<exe_dir>/GodotSharp/Api/`
instead of the exported `data_<assembly>_windows_x86_64/` directory, and the
game dies on startup with:

> Unable to find the .NET assemblies directory. Make sure the
> `…/GodotSharp/Api/Debug` directory exists and contains the .NET assemblies.

Two changes close this:

- `_template_version_dirs()` takes a `p_dotnet` flag. When the running editor is
  a .NET build (`_is_dotnet_editor()`, i.e. `ClassDB.class_exists("CSharpScript")`)
  every candidate directory gets a `.mono` suffix, and the plain names are
  **dropped** rather than offered as a fallback — a non-.NET template has no
  assembly loader and could never host a C# game.
- `_is_dotnet_project()` (a .NET editor *and* a non-empty
  `dotnet/project/assembly_name`) makes a real template mandatory. Unlike a
  GDScript project, a C# project does **not** get the loose dev-register
  editor-binary fallback; both `_has_valid_export_configuration()` and
  `_export_project()` hard-fail with a message naming the .NET template package.

A GDScript project on a standard (non-.NET) editor build is unaffected: the
candidate list and the loose fallback behave exactly as before.

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
