# Godot Microsoft GDK editor tools

This document explains the current editor-side split between the `godot_gdk`
runtime addon and the separate `godot_gdk_packaging` tooling addon.

See also:

- [`gdk/plugin.md`](plugin.md)
- [`gdk/build-and-loading.md`](build-and-loading.md)

## Current state

`godot_gdk` still ships these editor-side scripts:

- `gdk_editor_plugin.gd`
- `gdk_setup_panel.gd`
- `gdk_export_platform.gd`

The runtime addon's editor plugin owns startup wiring for the runtime addon
**and** registers the custom `Xbox GDK (PC)` export platform so it appears
in the editor's `Project > Export… > Add…` dropdown alongside the
built-in platforms.

The `godot_gdk_packaging` addon hosts the wider editor workflow
(top-level **Microsoft GDK** menu, sandbox switcher, `MicrosoftGame.config`
flows, tutorial wizard) and the headless packaging CLI.

## `gdk_editor_plugin.gd`

The current `godot_gdk` editor plugin is intentionally narrow.

Today it:

- installs or updates the addon-owned `GDKBootstrap` autoload on
  `_enable_plugin`
- registers the `Xbox GDK (PC)` export platform on `_enter_tree`
- does **not** dock `gdk_setup_panel.gd`

`gdk_editor_plugin.gd` owns startup wiring for the runtime addon and
the export-platform registration; the rest of the packaging UI lives in
`godot_gdk_packaging`.

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

`gdk_export_platform.gd` implements the custom `Xbox GDK (PC)` export
platform. It is registered by `gdk_editor_plugin.gd` on `_enter_tree`,
so the platform appears in the editor's `Project > Export… > Add…`
dropdown alongside Windows Desktop, Linux, etc.

It is **not** the only packaging path: the headless packaging runner in
`godot_gdk_packaging` (`tools/run.gd` / `gdkpkg.cmd`) remains the
canonical entry point for headless package, validate, install, and launch
automation. The export platform exists for editor-driven workflows; the
headless runner exists for CI and scripted automation. Keep both paths
working when the underlying packaging primitives change.

## `godot_gdk_packaging`

The active editor tooling for Microsoft GDK packaging now lives in
`addons\godot_gdk_packaging\editor\`.

Treat the root `addons\godot_gdk_packaging\` tree as the source of truth. The
repo build syncs that addon into the sample mirrors under
`sample\...\addons\godot_gdk_packaging\`, so contributors should edit the root
addon rather than patching sample copies directly.

That addon owns:

- the top-level **Microsoft GDK** editor menu
- headless package, validate, install, and launch actions
- tutorial/help surfaces
- `MicrosoftGame.config` helper flows

When discussing the current editor workflow, treat `godot_gdk` and
`godot_gdk_packaging` as separate layers:

- `godot_gdk` owns the runtime/services addon
- `godot_gdk_packaging` owns the supported packaging/editor workflow

## Relationship to the runtime addon

The Microsoft GDK runtime addon still matters to the editor story because samples and
packaging flows need title identity and synced addon payloads, but the runtime
addon is no longer the main place to look for packaging UI behavior.

That means editor-side documentation should describe:

- the retained `godot_gdk` helper scripts accurately
- the active packaging workflow under `godot_gdk_packaging`
- the separation between runtime/services behavior and editor tooling
