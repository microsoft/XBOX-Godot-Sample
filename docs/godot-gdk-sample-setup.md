# Godot GDK sample project setup

The sample project needs your **Partner Center** credentials to work with
Xbox Live services. You can configure everything through the in-editor
**GDK Setup panel** or via a CLI script.

## Prerequisites

1. **Register your title** in
   [Partner Center](https://partner.microsoft.com/)
2. **Create test accounts** in Partner Center → Account Settings → Xbox Live
   → Test Accounts
3. **Configure achievements** (optional) in Partner Center → Xbox Live →
   Achievements, then publish to your sandbox
4. Gather these values from Partner Center → Xbox Live → Xbox Live Setup:

| Value | Where to find it | Example |
|-------|-------------------|---------|
| Title ID | Xbox Live Setup | `6718942c` |
| MSA App ID | Xbox Live Setup | `93900f42-4313-...` |
| Store ID | Product identity page | `9XXXXXXXXX` |
| SCID | Xbox Live Setup | `00000000-0000-0000-0000-000067...` |
| Sandbox ID | Xbox Live Setup | `XDKS.1` |
| Publisher CN | Product identity page | `CN=XXXXXXXX-XXXX-...` |

## Option A: Configure in the Godot editor (recommended)

1. Build the addon and open the sample in the editor:
   ```powershell
   cmake --build build --preset debug
   .\sample\launch_editor.bat
   ```
2. Find the **GDK Setup** panel in the bottom-right dock
3. Enter your Partner Center values
4. Click **Save Configuration** — this writes `sample_config.cfg` (used at
   runtime by the sample's GDScript)
5. Click **Apply to Export Preset** — this pushes the same values into the
   export preset (used when packaging for distribution)

The config file is gitignored, so your credentials stay local.

## Option B: Configure via CLI

```powershell
.\tools\setup_sample.ps1
```

This prompts for each value and generates `sample_config.cfg`,
`MicrosoftGame.config`, and updates `export_presets.cfg` in one step.

## Set your PC sandbox

Your PC must be in the same sandbox as your test account:

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

> **Tip:** If sign-in fails, verify the sandbox with `XblPCSandbox.exe` and
> check that your test account exists in Partner Center → Account Settings →
> Xbox Live → Test Accounts.

## How configuration flows

```
sample_config.cfg (single source of truth)
  ├─► gdk_bootstrap.gd    reads SCID at runtime → initializes Xbox Live
  ├─► main.gd             reads achievement ID at runtime → unlock button
  ├─► export preset        auto-populates defaults → used during export
  └─► MicrosoftGame.config generated at export time from preset values
```

The **GDK Setup panel** and **export dialog** both read from
`sample_config.cfg`. If a value is set in the export preset, it takes priority.
If it's blank, the config file value is used as a fallback.

## Testing achievements

Achievements must be configured in
[Partner Center](https://partner.microsoft.com/) and published to your
development sandbox before they can be unlocked.

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
