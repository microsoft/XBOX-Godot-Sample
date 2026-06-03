# Godot GDK editor tools

This document explains the current editor-side split between the `godot_gdk`
runtime addon and the separate `godot_gdk_packaging` tooling addon.

See also:

- [`godot-gdk-plugin.md`](godot-gdk-plugin.md)
- [`godot-gdk-build-and-loading.md`](godot-gdk-build-and-loading.md)

## Current state

`godot_gdk` still ships these editor-side scripts:

- `gdk_editor_plugin.gd`
- `gdk_setup_panel.gd`
- `gdk_export_platform.gd`

However, the current editor workflow is no longer centered on a custom GDK
export platform. The active export/package/install UI now lives in the
separate `godot_gdk_packaging` addon.

## `gdk_editor_plugin.gd`

The current `godot_gdk` editor plugin is intentionally narrow.

Today it:

- installs or updates the addon-owned `GDKBootstrap` autoload
- does **not** register the legacy custom export platform
- does **not** dock `gdk_setup_panel.gd`

In other words, `gdk_editor_plugin.gd` owns startup wiring for the runtime
addon, but it is no longer the owner of the repo's main packaging workflow.
That responsibility moved to `godot_gdk_packaging`.

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

`gdk_export_platform.gd` is a retained implementation of the older custom
export-platform path.

It is **not** the default packaging path today. The repo now favors the
packaging dock in `godot_gdk_packaging`, which owns the supported export,
package, validate, install, and launch flow.

Keep docs and samples aligned with that newer flow unless the repo explicitly
reintroduces the custom export-platform path.

## `godot_gdk_packaging`

The active editor tooling for GDK packaging now lives in
`addons\godot_gdk_packaging\editor\`.

Treat the root `addons\godot_gdk_packaging\` tree as the source of truth. The
repo build syncs that addon into the sample mirrors under
`sample\...\addons\godot_gdk_packaging\`, so contributors should edit the root
addon rather than patching sample copies directly.

That addon owns:

- the top-level **GDK** editor menu
- the packaging dock panel
- export + package actions
- install + launch actions
- tutorial/help surfaces
- `MicrosoftGame.config` helper flows

When discussing the current editor workflow, treat `godot_gdk` and
`godot_gdk_packaging` as separate layers:

- `godot_gdk` owns the runtime/services addon
- `godot_gdk_packaging` owns the supported packaging/editor workflow

## Relationship to the runtime addon

The GDK runtime addon still matters to the editor story because samples and
packaging flows need title identity and synced addon payloads, but the runtime
addon is no longer the main place to look for packaging UI behavior.

That means editor-side documentation should describe:

- the retained `godot_gdk` helper scripts accurately
- the active packaging workflow under `godot_gdk_packaging`
- the separation between runtime/services behavior and editor tooling
