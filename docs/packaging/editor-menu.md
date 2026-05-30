# Godot GDK Packaging — Editor `GDK` Menu

The `godot_gdk_packaging` addon adds a top-level **GDK** menu to the
Godot editor. The menu is editor tooling only: it does not add a runtime
autoload or engine singleton, and it is separate from the headless
`gdkpkg` runner documented in [Packaging Plugin](plugin.md).

The menu is assembled by
`addons\godot_gdk_packaging\editor\gdk_packaging_plugin.gd`. It contains
11 user-visible entries:

1. Getting Started
2. Create MicrosoftGame.config / Edit MicrosoftGame.config
3. Change Sandbox…
4. Package Manager…
5. PC Packaging Overview
6. makepkg Reference
7. GameConfigEditor Reference
8. Achievements Guide
9. PlayFab Game Manager
10. PlayFab IDs from Xbox Live
11. PlayFab + GDK Quickstart

## Prerequisites shared by all menu entries

- Enable the `godot_gdk_packaging` addon in the Godot editor.
- Open the project whose packaging state you want to inspect or change.
- Install the Microsoft GDK when you want to run local tools such as
  `GameConfigEditor.exe`, `XblPCSandbox.exe`, `wdapp.exe`, or `makepkg.exe`.
  The addon discovers the GDK from `GDK_BIN` first, then from the default
  `C:\Program Files (x86)\Microsoft GDK\bin` location.

Documentation links also require network access and whatever Microsoft,
Partner Center, or PlayFab account permissions the linked site requires.

## Menu entries

### Getting Started

**What it does:** Opens the in-editor tutorial wizard. The wizard walks
through the packaging workflow at a high level: MicrosoftGame.config,
sandboxes, export and package steps, installing and launching builds,
achievements, and PlayFab setup.

**When to use it:** Use this first when enabling the addon in a project, or
when you need a quick reminder of the packaging workflow without leaving the
editor.

**Required prerequisites:** The addon must be enabled and the Godot editor
must have a base editor window. The wizard itself does not require the GDK to
be installed because it is informational.

**Expected output / side effects:** A modal wizard window opens. Navigating or
closing it does not change project files or machine state.

**Deeper docs:** Continue with [Packaging Plugin](plugin.md) for the headless
commands that perform the build and packaging work.

### Create MicrosoftGame.config / Edit MicrosoftGame.config

**What it does:** The label changes based on whether the project already has a
MicrosoftGame.config. When the file is missing, the menu creates a starter
`MicrosoftGame.config`, creates placeholder images under `storelogos\`, asks
Godot's file system to rescan, then launches `GameConfigEditor.exe`. When the
file already exists, it launches `GameConfigEditor.exe` against the current
config.

**When to use it:** Use it when bootstrapping a GDK project identity, editing
store logos, or updating the executable, identity, product, or Xbox Live fields
inside the config.

**Required prerequisites:** Install the Microsoft GDK so
`GameConfigEditor.exe` is available. The project directory must be writable
when creating the initial template. Partner Center sign-in is not required just
to create or edit the local file, but store association and Xbox Live IDs must
come from your Partner Center title when you are preparing a real package.

**Expected output / side effects:** Template creation writes
`MicrosoftGame.config` and a `storelogos\` folder in the project if they do not
exist. Editing through GameConfigEditor may rewrite the config and logo files.
If the editor executable is missing or fails to launch, the Godot output panel
shows an error.

**Deeper docs:** See [Packaging Plugin](plugin.md#verbs) for the
`config_template` and `config_editor` headless verbs, and Microsoft's
GameConfigEditor documentation from the **GameConfigEditor Reference** menu
entry.

### Change Sandbox…

**What it does:** Opens the GDK Sandbox Switcher dialog. The dialog reads the
current PC sandbox with `XblPCSandbox.exe /get`, can switch to a target
sandbox with `/set <sandbox-id> /noApps`, and can switch back to RETAIL with
`/retail /noApps`.

**When to use it:** Use it before testing Xbox Live services with development
sandboxes, or before returning the PC to retail Xbox Live services.

**Required prerequisites:** Install the Microsoft GDK so
`XblPCSandbox.exe` is available. Switching sandboxes is machine-wide and
requires administrator privileges. You also need the target sandbox ID from a
Partner Center title that your developer account can access.

**Expected output / side effects:** The dialog displays the current sandbox,
confirms any switch, and reports success or failure. A successful switch changes
the PC's machine-wide Xbox sandbox for every user, Xbox tool, and Xbox-aware
app on the machine. Switch back to RETAIL before using retail Xbox Live
services.

**Deeper docs:** See [Xbox sandbox and test accounts](../platform/xbox-sandbox-and-test-accounts.md)
for account and sandbox setup. The headless equivalent is the `sandbox` verb in
[Packaging Plugin](plugin.md#verbs).

### Package Manager…

**What it does:** Opens a machine-wide package manager backed by `wdapp.exe`.
It lists registered packages and AUMIDs, installs a selected `.msixvc`,
uninstalls the selected package, refreshes the package list, and can open
Godot's standard **Project > Export…** dialog to build a new package.

**When to use it:** Use it when validating what packages are installed on your
PC, installing a package built elsewhere, removing stale packages, or jumping to
Godot's export dialog before a packaging pass.

**Required prerequisites:** Install the Microsoft GDK so `wdapp.exe` is
available. Installing requires a built `.msixvc` package. Uninstalling requires
selecting a package from the list. Exporting requires a configured Godot export
preset for your project.

**Expected output / side effects:** The dialog refreshes asynchronously while
`wdapp` runs. Installing or uninstalling changes the machine-wide registered
package state, not just the current project. Opening **Project > Export…** does
not build by itself; the export dialog owns that workflow.

**Deeper docs:** See the `install`, `uninstall`, `register_loose`, `launch`,
and `terminate` verbs in [Packaging Plugin](plugin.md#verbs), plus the
**PC Packaging Overview** and **makepkg Reference** menu links.

### PC Packaging Overview

**What it does:** Opens Microsoft's PC packaging getting-started documentation
in the system browser.

**When to use it:** Use it when you need the Microsoft overview for PC package
layout, packaging terms, or the broader flow around building MSIXVC packages.

**Required prerequisites:** Network access. Running the documented steps later
requires the Microsoft GDK and an appropriately configured title.

**Expected output / side effects:** The browser opens a Microsoft Learn page.
No project files or machine packaging state change.

**Deeper docs:** Pair it with [Packaging Plugin](plugin.md) for this addon's
headless command reference.

### makepkg Reference

**What it does:** Opens Microsoft's `makepkg.exe` package-creation reference in
the system browser.

**When to use it:** Use it when troubleshooting `genmap`, `pack`, or
`validate`, or when you need to understand the flags the addon passes through
to `makepkg.exe`.

**Required prerequisites:** Network access. To run `makepkg.exe` locally, the
Microsoft GDK must be installed.

**Expected output / side effects:** The browser opens a Microsoft Learn page.
No local files change.

**Deeper docs:** See the `pack`, `genmap`, and `validate` verb descriptions in
[Packaging Plugin](plugin.md#verbs).

### GameConfigEditor Reference

**What it does:** Opens Microsoft's GameConfigEditor documentation in the
system browser.

**When to use it:** Use it when editing MicrosoftGame.config fields, store
logos, identity values, or store association details.

**Required prerequisites:** Network access. To launch GameConfigEditor from the
addon, the Microsoft GDK must be installed.

**Expected output / side effects:** The browser opens a Microsoft Learn page.
No local files change until you separately run GameConfigEditor and save a
config.

**Deeper docs:** See **Create MicrosoftGame.config / Edit MicrosoftGame.config**
on this page and the `config_template` / `config_editor` verbs in
[Packaging Plugin](plugin.md#verbs).

### Achievements Guide

**What it does:** Opens Microsoft's PC end-to-end achievements guide in the
system browser.

**When to use it:** Use it when configuring achievements for a Partner Center
title, publishing them to a development sandbox, or validating achievement
unlock flows with test accounts.

**Required prerequisites:** Network access. Following the guide requires a
Partner Center title, access to Xbox Live configuration for that title, a
development sandbox, and a matching test account.

**Expected output / side effects:** The browser opens a Microsoft Learn page.
No local project files change.

**Deeper docs:** See [Xbox sandbox and test accounts](../platform/xbox-sandbox-and-test-accounts.md)
for the local sandbox and test-account prerequisites.

### PlayFab Game Manager

**What it does:** Opens the PlayFab Game Manager sign-in page in the system
browser.

**When to use it:** Use it to manage PlayFab title settings, API keys,
statistics, leaderboards, multiplayer feature switches, and test data needed by
PlayFab-backed samples or tutorials.

**Required prerequisites:** Network access and a PlayFab account with access to
the target title.

**Expected output / side effects:** The browser opens PlayFab Game Manager.
Changes happen only if you modify title data in the portal.

**Deeper docs:** See [PlayFab title prerequisites](../playfab/prerequisites.md)
for the title configuration expected by this repository's PlayFab tutorials.

### PlayFab IDs from Xbox Live

**What it does:** Opens the PlayFab REST API documentation for mapping Xbox Live
IDs to PlayFab IDs.

**When to use it:** Use it when reconciling Xbox identities with PlayFab player
records or debugging cross-service account linking.

**Required prerequisites:** Network access. Calling the documented API requires
PlayFab credentials and the appropriate Xbox Live identity data.

**Expected output / side effects:** The browser opens a Microsoft Learn API
reference. No local files change.

**Deeper docs:** See [PlayFab title prerequisites](../playfab/prerequisites.md)
and the PlayFab user-session docs in [PlayFab Plugin](../playfab/plugin.md).

### PlayFab + GDK Quickstart

**What it does:** Opens the PlayFab SDK for GDK quickstart in the system
browser.

**When to use it:** Use it when you need Microsoft's native PlayFab + GDK
integration context while comparing this repository's Godot addon layer to the
underlying platform SDK guidance.

**Required prerequisites:** Network access. Running native GDK SDK samples also
requires the Microsoft GDK and a PlayFab title.

**Expected output / side effects:** The browser opens Microsoft Learn. No local
project files or machine state change.

**Deeper docs:** See [PlayFab Plugin](../playfab/plugin.md) for this repo's
Godot-facing PlayFab surface.
