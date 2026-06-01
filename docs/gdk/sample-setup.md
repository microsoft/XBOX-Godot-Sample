# Godot GDK sample project setup

The integrated tutorial sample needs your **Partner Center** credentials to
work with Xbox Live services. The supported setup paths are the repo CLI script
or manually editing the local config files and Project Settings. The
`godot_gdk` editor plugin no longer docks a **GDK Setup** panel; it keeps the
runtime autoload installed and registers the `Xbox GDK (PC)` export platform.

## Prerequisites

1. **Register your title** in the
   [Partner Center dashboard](https://partner.microsoft.com/dashboard).
   If you don't have a publisher account yet, start with the
   [ID@Xbox program](https://www.xbox.com/en-us/developers/id) and the
   [Microsoft Game Stack publisher hub](https://developer.microsoft.com/en-us/games/publish/).
2. **Create test accounts** in Partner Center → Account Settings → Xbox Live
   → Test Accounts
3. **Configure achievements** (optional) in Partner Center → Xbox Live →
   Achievements, then publish to your sandbox. See
   [Microsoft GDK — Achievements](https://learn.microsoft.com/en-us/gaming/gdk/docs/services/player-data/achievements/live-achievements-nav)
   for the authoring walkthrough.
4. Gather these values from Partner Center → Xbox Live → Xbox Live Setup
   (see
   [Configuring Xbox services (Title ID + SCID)](https://learn.microsoft.com/en-us/gaming/gdk/docs/services/fundamentals/portal-config/live-service-config-ids-mp)
   and
   [Setting up sandboxes](https://learn.microsoft.com/en-us/gaming/gdk/docs/services/fundamentals/sandboxes/live-setup-sandbox)):

| Value | Where to find it | Example |
|-------|-------------------|---------|
| Title ID | Xbox Live Setup | `6718942c` |
| MSA App ID | Xbox Live Setup | `93900f42-4313-...` |
| Store ID | Product identity page | `9XXXXXXXXX` |
| SCID | Xbox Live Setup | `00000000-0000-0000-0000-000067...` |
| Sandbox ID | Xbox Live Setup | `XDKS.1` |
| Publisher CN | Product identity page | `CN=XXXXXXXX-XXXX-...` |

## Title-owned values checklist

Keep these title-specific values in one place before wiring the sample or your
own game code:

- **Stats and leaderboard identifiers:** the exact stat names your title writes
  with `GDK.stats`, plus any leaderboard/stat relationships your gameplay uses.
- **Rich presence values:** the state/string IDs and token values your title
  configured for `GDK.presence` or multiplayer activity surfaces.
- **Store product IDs:** the product IDs you pass to `GDK.store` queries and
  purchase UI flows.
- **DLC package layout:** each content package's expected `pack_relative_path`
  to the `.pck`/`.zip` you load with `GDK.package.load_resource_pack_async()`.
  The runtime-discovered `package_identifier` comes from
  `enumerate_packages()`/`find_package_by_identifier()`, but the pack-relative
  path is title-owned.
- **Sandbox + test accounts:** the active sandbox ID, the test accounts
  provisioned into that sandbox, and the PC sandbox currently selected on the
  development machine.
- **Peer-XUID prerequisites:** additional signed-in test accounts/XUIDs for
  invites, recent-player updates, profile-card UI, reputation feedback, and any
  other peer-targeted Xbox service flow.

## Option A: Configure via CLI (recommended)

From the repository root, build the mirrored sample addons and run the setup
script:

```powershell
cmake --build build --preset debug
pwsh -NoLogo -NoProfile -ExecutionPolicy Bypass -File .\tools\setup_sample.ps1
```

The script prompts for each Partner Center value, derives the current-title
SCID, then writes `sample\tutorial_app\sample_config.cfg` and
`sample\tutorial_app\MicrosoftGame.config`. If `export_presets.cfg` already
exists in that project, the script also updates the matching export-preset
fields. The script does not create the preset from scratch; start from the
sample's checked-in base preset.

The generated files are gitignored, so your credentials stay local.

## Option B: Configure manually in the project

1. Copy `sample\tutorial_app\sample_config.cfg.template` to
   `sample\tutorial_app\sample_config.cfg` and fill in the Partner Center
   values used by the tutorial scripts.
2. Copy `sample\tutorial_app\MicrosoftGame.config.template` to
   `sample\tutorial_app\MicrosoftGame.config` and replace the Title ID,
   MSA App ID, Store ID, identity, publisher, and visual placeholders.
3. Open `sample\tutorial_app\project.godot` in Godot. The committed project
   already enables the `godot_gdk`, `godot_playfab`, and `godot_gdk_packaging`
   plugins after the CMake build mirrors them into `sample\tutorial_app\addons\`.
4. Review **Project → Project Settings** for `gdk/runtime/*` startup settings
   and any PlayFab runtime values needed by the tutorial scenes.
5. For editor-driven packaging, use **Project → Export… → Add… → Xbox GDK (PC)**.
   For scripted packaging and sandbox actions, use the separate
   `godot_gdk_packaging` addon (`addons\godot_gdk_packaging\gdkpkg.cmd` or its
   top-level **GDK** editor menu).

## Set your PC sandbox

Setting the PC sandbox and signing into a test account is covered in its
own document — see [`platform/xbox-sandbox-and-test-accounts.md`](../platform/xbox-sandbox-and-test-accounts.md).
The short version:

```powershell
# Set sandbox (requires admin)
& "C:\Program Files (x86)\Microsoft GDK\bin\XblPCSandbox.exe" YOUR_SANDBOX_ID

# Verify
& "C:\Program Files (x86)\Microsoft GDK\bin\XblPCSandbox.exe"
```

## Test account sign-in

The sample uses **Xbox test accounts**, not personal Microsoft accounts:

1. Ensure your PC sandbox matches the sandbox in Partner Center
2. Launch the sample — it will attempt silent sign-in automatically
3. If prompted, sign in with your **test account** credentials (not your
   personal account)
4. Your test account must be provisioned in Partner Center under the same
   sandbox

> **Tip:** If sign-in fails, see the troubleshooting table in
> [`platform/xbox-sandbox-and-test-accounts.md`](../platform/xbox-sandbox-and-test-accounts.md).

## How configuration flows

```
project.godot
  └─► addons/godot_gdk/runtime/gdk_bootstrap.gd
         reads gdk/runtime/* startup flags → initializes GDK / silent sign-in

sample_config.cfg (local tutorial values)
  └─► tutorial scenes read achievement and sandbox-related sample settings

MicrosoftGame.config
  └─► packaging/export flows read title identity and shell visuals
```

The CLI script writes both generated files from the same prompts. When you edit
manually, keep `sample_config.cfg`, `MicrosoftGame.config`, and any export
preset values aligned yourself; there is no docked `GDK Setup` panel in the
current runtime addon.

## Testing achievements

Achievements must be configured in
[Partner Center](https://partner.microsoft.com/dashboard) and published to
your development sandbox before they can be unlocked. See
[Microsoft GDK — Achievements](https://learn.microsoft.com/en-us/gaming/gdk/docs/services/player-data/achievements/live-achievements-nav)
for the authoring walkthrough.

To **reset** achievements for re-testing, use the included helper script:

```powershell
.\tools\reset_player_data.ps1
```

This signs into Partner Center via `XblDevAccount.exe`, then calls
`XblPlayerDataReset.exe` to wipe achievements, stats, and leaderboards for
the specified test account. You'll need:

- **Service Config ID (SCID)** — from Partner Center → Xbox Live → Xbox Live
  Setup
- **Sandbox ID** — the development sandbox your test account is signed into
- **XUID** — the Xbox User ID of the test account to reset

> **Note:** Resets only work on Xbox test accounts in a development sandbox,
> not retail accounts. Restart the game after resetting for changes to take
> effect.
