# Godot GDK editor tools

This document explains the editor-side pieces that ship with the `godot_gdk` addon.

See also:

- [`godot-gdk-plugin.md`](godot-gdk-plugin.md)
- [`godot-gdk-build-and-loading.md`](godot-gdk-build-and-loading.md)

## Editor plugin overview

The addon still ships an editor plugin even though the native runtime implementation is still at an early runtime/users/achievements/presence/social baseline.

The editor-side pieces are:

- `gdk_editor_plugin.gd`
- `gdk_setup_panel.gd`
- `gdk_export_platform.gd`

Together they provide:

- a custom export platform for GDK packaging
- a setup dock for local configuration
- editor integration around sample/export settings

## `gdk_editor_plugin.gd`

This is the editor entry point.

On load it:

1. creates and registers the custom export platform
2. creates and docks the setup panel

On unload it removes both.

So this script is the glue between the addon metadata in `plugin.cfg` and the actual tool scripts used inside the editor.

## `gdk_setup_panel.gd`

This script builds a dock UI for local Partner Center configuration.

Its job is to:

- render a form of GDK/Xbox Live identity fields
- load existing values from `res://sample_config.cfg`
- save the form back to `sample_config.cfg`
- push selected values into `export_presets.cfg`
- normalize the Title ID to 8 hex digits and reject invalid values before saving/applying

The SCID field should be treated as an override. The runtime/services path now derives the current-title SCID from the title id, so the setup panel no longer needs a manually entered SCID for the normal sample flow.

In practice, this is the editor-side configuration bridge between:

- local sample runtime settings
- export preset settings

## `gdk_export_platform.gd`

This script defines the custom export platform for GDK packaging.

At a high level it:

1. detects the local GDK tools
2. reads defaults from `sample_config.cfg`
3. exposes export options in the export dialog
4. stages a Godot export into a temporary directory
5. copies the addon DLL into the staged build
6. generates `MicrosoftGame.config`
7. either:
   - registers the loose build with `wdapp`, or
   - packages with `makepkg`

When no explicit SCID is configured, the export tooling derives the default Game OS SCID from the title id using the same null-GUID convention as the runtime.

So even though the gameplay-side native feature set is still early, the addon already includes editor-side configuration and packaging support for the broader GDK workflow.

## Relationship to the current native scope

The native runtime currently implements the runtime/users/achievements/presence/social baseline.

The editor tools are therefore broader than the currently implemented runtime APIs. They remain part of the shipped addon because they support:

- project configuration
- sample setup
- export packaging flow

That means the editor side should be thought of as addon infrastructure, not just a thin wrapper around whatever native services are currently implemented.
