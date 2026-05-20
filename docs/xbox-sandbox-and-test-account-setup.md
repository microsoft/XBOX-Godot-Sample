# Xbox sandbox and test-account setup

This guide is the canonical, addon-agnostic walk-through for getting a PC
into the right state to test Xbox Live services from any of the GodotGDK
samples (`gdk_demo`, `multiplayer_pong`, `gdk_launch_point`, `playfab_demo`).

If you are configuring a sample's Partner Center IDs (Title ID, SCID, etc.),
see [`godot-gdk-sample-setup.md`](godot-gdk-sample-setup.md). This document
covers what your **PC** and **test account** need on top of that.

---

## Prerequisites

| Requirement | How to check |
|---|---|
| Microsoft GDK installed | `Get-ChildItem "C:\Program Files (x86)\Microsoft GDK\bin\XblPCSandbox.exe"` returns a path |
| Xbox app installed | `Get-AppxPackage Microsoft.GamingApp` returns a package |
| Windows Developer Mode enabled | `wdapp list` runs without printing `Developer mode is not enabled.` (see [Enabling Windows Developer Mode](#enabling-windows-developer-mode)) |
| Godot Windows export templates installed | Godot editor → **Project → Manage Export Templates… → Download and Install** finds a 4.x build matching your editor (one-time setup; required for the loose-layout workflow) |
| Partner Center title (with at least one published Service Configuration) | You can read the SCID + Sandbox ID off Partner Center → Xbox Live → Xbox Live Setup |
| At least one **Xbox test account** provisioned in your sandbox | Partner Center → Account Settings → Xbox Live → Test Accounts |

> Xbox Live calls fail (HRESULT `0x80070490` and friends) if any one of
> these is missing. The samples log that warning at boot but stay playable
> in offline mode — useful for local UI work, but cloud features won't fire.

---

## 1. Set your PC sandbox

Your PC has to be in the same sandbox as the test account you plan to sign
into. The sandbox ID comes straight from Partner Center → Xbox Live → Xbox
Live Setup (e.g. `XDKS.1` or `MYTEAM.0`).

```powershell
# Set sandbox (admin PowerShell required).
& "C:\Program Files (x86)\Microsoft GDK\bin\XblPCSandbox.exe" XDKS.1

# Read it back to confirm.
& "C:\Program Files (x86)\Microsoft GDK\bin\XblPCSandbox.exe"
```

The sandbox switch is per-PC and persists across reboots. You only need to
re-run it when you move between titles or testing environments.

> **Returning to retail.** Use `RETAIL` as the sandbox ID to put the
> machine back into the public Xbox Live environment.

---

## 2. Sign in a test account

Xbox Live differentiates "real" Microsoft Accounts (MSAs) from **test
accounts** that exist only inside Partner Center. The samples expect a test
account because:

- They run inside a development sandbox (your retail account is not in it).
- Test accounts can be reset, granted achievements, and have their stats
  wiped via `XblPlayerDataReset.exe` without affecting any real player.

There are two ways to sign in:

### Option A — via the Xbox app (recommended for interactive testing)

1. Make sure the PC is in the right sandbox (step 1 above).
2. Open the **Xbox app** (`Microsoft.GamingApp`).
3. Sign out of any signed-in account.
4. Sign back in with the **test account email + password** from
   Partner Center → Account Settings → Xbox Live → Test Accounts.
5. The Xbox app should now show the test account's gamertag.

When you launch a GodotGDK sample after that, the bootstrap autoload calls
`GDK.users.add_default_user_async()` (silent sign-in). If that succeeds the
title screen HUD shows **GDK READY · SIGNED IN** with the test account's
gamertag and avatar. **If silent sign-in fails** — common when running
unpackaged from the editor or right after a sandbox switch — the bootstrap
automatically falls back to `GDK.users.add_user_with_ui_async()`, which
pops the system identity picker. Pick the test account once and the HUD
flips to signed-in for the rest of the session. (The fallback requires
[Windows Developer Mode to be enabled](#enabling-windows-developer-mode);
without it the picker silently no-ops.)

### Option B — via `XblDevAccount.exe` (scripted / CI-friendly)

```powershell
# Sign the dev account into the active sandbox.
& "C:\Program Files (x86)\Microsoft GDK\bin\XblDevAccount.exe" `
    signin --user testaccount@xboxtest.com
```

This is the same flow `tools\reset_player_data.ps1` uses internally.

---

## 3. Verify

Three quick checks confirm the PC is wired up:

1. **Sandbox** — `XblPCSandbox.exe` (no args) prints the sandbox ID.
2. **Account** — Xbox app shows the test account's gamertag.
3. **In sample** — launch e.g. `sample\multiplayer_pong` and check the
   title screen HUD:

   | HUD readout | Meaning |
   |---|---|
   | `GDK OFFLINE` (red ring) | Extension didn't load, or `is_initialized()` returned false. Almost always a missing Title ID / SCID in `sample_config.cfg`. |
   | `GDK INIT…` (yellow ring) | Extension loaded but the runtime didn't reach the `initialized` signal yet. Should resolve in <1 s. Stuck here = check the `[GDK]` warnings in stdout. |
   | `GDK READY · SIGNED OUT` | Runtime up, but `add_default_user_async` couldn't bind a user. **The most common cause is running the title from the Godot editor (F5).** PC GDK requires the running process to be a registered package or loose layout; an unpackaged `godot.exe` has no Title ID context, so Xbox services silently degrade. See [Run the title as a registered loose layout](#run-the-title-as-a-registered-loose-layout) below. |
   | `GDK READY · SIGNED IN` + gamertag + avatar | Everything is wired up correctly. |

If the HUD says `SIGNED OUT`, the **first** thing to check is whether you
launched via the editor's play button. F5 from the editor cannot bind a
signed-in Xbox user — that's a hard PC GDK constraint, not a sample bug.
Use [Run the title as a registered loose layout](#run-the-title-as-a-registered-loose-layout)
instead. After that, work through the
[Troubleshooting](#troubleshooting) checklist below if it's still wrong.

---

## Run the title as a registered loose layout

PC GDK requires the running process to live inside a **registered package
or loose-layout build** before it can bind a signed-in user. Launching a
sample with the Godot editor's play button (F5) runs `godot.exe`
unpackaged, so `XGameGetXboxTitleId` returns failure inside the GDK
extension and Xbox services silently degrade — the runtime still emits
its `initialized` signal (so the HUD reads `GDK READY`) but no user can
ever bind (`SIGNED OUT`).

The supported dev workflow is to export the sample into a `Build/` folder
and register that folder with `wdapp`:

1. Open the sample's project in the Godot editor (e.g.
   `sample\multiplayer_pong\project.godot`).
2. **(One-time)** If you've never installed Godot's Windows export
   templates: **Project → Manage Export Templates… → Download and Install**.
3. Export and prepare the loose layout with the packaging runner:

   ```powershell
   .\addons\godot_gdk_packaging\gdkpkg.cmd export --preset "Windows Desktop" --output-dir Build
   ```

4. Register the prepared layout:

   ```powershell
   .\addons\godot_gdk_packaging\gdkpkg.cmd register_loose --content-dir Build
   ```

5. Launch with `wdapp launch` from PowerShell using the package alias printed
   during registration.

The launched process inherits the registered Title ID context, so silent
sign-in succeeds (or the bootstrap's UI fallback pops the system identity
picker) and the HUD switches to `GDK READY · SIGNED IN`.

> **The first launch after a sandbox switch may still show `SIGNED OUT`.**
> The bootstrap automatically falls back to the system identity picker on
> the next launch — pick the test account once and the rest of the session
> stays signed in.

> **`wdapp list` is the source of truth.** If it reports
> `No developer packages were found.`, nothing is registered yet and no
> sign-in path can succeed. Re-run **Export and Register**.

> **Need to iterate fast?** The Godot CLI can also be driven from
> PowerShell:
>
> ```powershell
> $godot = "<path to Godot editor exe>"
> $proj  = "<absolute path to sample project>"
> & $godot --headless --path $proj --export-debug "Windows Desktop" "$proj\Build\<game>.exe"
> & "C:\Program Files (x86)\Microsoft GDK\bin\wdapp.exe" register "$proj\Build"
> & "C:\Program Files (x86)\Microsoft GDK\bin\wdapp.exe" launch <PackageFamily>!Game
> ```
>
> The package alias (`<PackageFamily>!Game`) comes from `MicrosoftGame.config`'s
> `<Identity Name="…"/>` and `<Executable Id="…"/>`.

---

## 4. Reset player data between test runs

Achievements, stats, and leaderboard entries persist per-account in the
sandbox. To wipe them between runs (e.g. so an unlock flow runs from
scratch), use the bundled helper:

```powershell
.\tools\reset_player_data.ps1
```

This signs in via `XblDevAccount.exe`, then calls `XblPlayerDataReset.exe`
to wipe data for a given XUID + sandbox + SCID. You'll be prompted for:

- **SCID** — Partner Center → Xbox Live → Xbox Live Setup
- **Sandbox ID** — same one you set in step 1
- **XUID** — the test account's Xbox User ID

> **Resets only work on test accounts in a development sandbox.** Retail
> accounts are immutable. Restart the game after a reset for the new state
> to take effect.

---

## Troubleshooting

| Symptom | Likely cause | Fix |
|---|---|---|
| `[GDK] Xbox title ID is unavailable; Xbox services were not initialized.` (`0x80070490`) | `sample_config.cfg` is empty / missing Title ID / SCID. | Run `tools\setup_sample.ps1` or use the **GDK Setup** editor panel ([`godot-gdk-sample-setup.md`](godot-gdk-sample-setup.md)). |
| HUD shows `GDK READY · SIGNED OUT` even though the Xbox app is signed in to the test account | The title is running unpackaged (e.g. via Godot editor F5). PC GDK rejects sign-in attempts when `XGameGetXboxTitleId` can't bind to a registered package; the runtime layer still initializes ("READY") but Xbox services internally degrade. The Xbox app's own broker tokens are unrelated — they only apply to the Xbox app process. | Register a loose layout via `addons\godot_gdk_packaging\gdkpkg.cmd register_loose` and launch through `wdapp` — see [Run the title as a registered loose layout](#run-the-title-as-a-registered-loose-layout) below. The bootstrap also falls back to the system identity picker if silent sign-in fails inside a registered title; use that to switch accounts mid-session. |
| HUD shows `GDK READY · SIGNED OUT` from a *registered* loose layout, and the system identity picker never appears | Windows Developer Mode is disabled. PC GDK tooling (including `wdapp` and the user broker hooks) refuses to operate, and the bootstrap's UI fallback fires but the system swallows the picker. The classic symptom is `wdapp list` printing `Developer mode is not enabled. To use this tool, please enable developer mode in the Windows Settings app.` | Turn on Developer Mode (Settings → For Developers → **Developer Mode**), or run the equivalent registry edit from an elevated PowerShell — see [Enabling Windows Developer Mode](#enabling-windows-developer-mode) below. Then re-register and relaunch the sample. |
| `XblPCSandbox.exe` returns `Error: Gaming Services must be updated. Update from https://aka.ms/gamingservices.` even though the Store shows Gaming Services as **Installed** | `XblPCSandbox.exe` is paired one-to-one with the `Microsoft.GamingServices` build that shipped in the same GDK edition (e.g. GDK 260400 ships `XblPCSandbox 1.0.2603.20002` + `GamingServices 35.112.20003.0`). The Microsoft Store ships a different consumer build (e.g. `35.112.23002.0`) that omits the developer/sandbox surface, so the GDK tool refuses to talk to it regardless of how new the Store version is. The bundled `InstallGamingServicesBundle.ps1` script is broken — it ships with literal `%GAMING_SERVICES_VERSION%` placeholders and bails out before doing anything useful. | Replace the consumer package with the GDK-shipped bundle directly. Open an **elevated** PowerShell and run the snippet under [Replacing Gaming Services with the GDK-shipped build](#replacing-gaming-services-with-the-gdk-shipped-build) below. After the script finishes, `XblPCSandbox.exe` will work and you can switch sandboxes normally. |
| `XblPCSandbox.exe : Access is denied.` | PowerShell is not elevated. | Re-launch PowerShell as Administrator before running `XblPCSandbox.exe`. |
| Sandbox flips back to `RETAIL` after reboot | Newer Windows builds need an explicit re-set after major OS updates. | Re-run step 1. |
| Test account does not appear in Partner Center | Sandbox membership wasn't granted. | In Partner Center → Account Settings → Xbox Live → Test Accounts → click the account → **Sandbox membership** → add the sandbox you're using. |
| `XblDevAccount.exe signin` fails with `0x87DD0006` | Test account password reset on Partner Center but local cache is stale. | `XblDevAccount.exe signout`, then sign back in. |
| Multiplayer activity warning at boot (`0x80070032`) | `MultiplayerActivity` requires a properly registered MSA App ID + protocol activation. | Safe to ignore for non-multiplayer samples. For real multiplayer testing, follow `godot-gdk-sample-setup.md`. |

---

## Enabling Windows Developer Mode

PC GDK tooling refuses to operate when Windows Developer Mode is disabled.
The most visible symptom is `wdapp list` printing:

```
Developer mode is not enabled. To use this tool, please enable developer mode in the Windows Settings app.
```

Less obvious is that the in-process **identity picker** the sample bootstrap
falls back to when silent sign-in fails will silently no-op without
Developer Mode, leaving the HUD stuck on `GDK READY · SIGNED OUT`.

**Easy way:** Settings → System → For Developers → flip **Developer Mode**
on, accept the prompt.

**Scripted way (elevated PowerShell):**

```powershell
$key = 'HKLM:\SOFTWARE\Microsoft\Windows\CurrentVersion\AppModelUnlock'
if (-not (Test-Path $key)) { New-Item -Path $key -Force | Out-Null }
New-ItemProperty -Path $key -Name AllowDevelopmentWithoutDevLicense -Value 1 -PropertyType DWord -Force | Out-Null
New-ItemProperty -Path $key -Name AllowAllTrustedApps -Value 1 -PropertyType DWord -Force | Out-Null

# Verify wdapp now responds.
& 'C:\Program Files (x86)\Microsoft GDK\bin\wdapp.exe' list
```

No reboot required. Relaunch the sample and the identity picker will appear
the first time silent sign-in fails.

---

## Replacing Gaming Services with the GDK-shipped build

If `XblPCSandbox.exe` reports `Error: Gaming Services must be updated.` (see
the troubleshooting table for why this happens even when the Store shows the
package as installed), the Store-distributed `Microsoft.GamingServices`
package needs to be replaced with the build that shipped in your installed
GDK edition.

> **Heads-up.** The `InstallGamingServicesBundle.ps1` script in
> `...\windows\redist\` is shipped broken (literal `%GAMING_SERVICES_VERSION%`
> placeholders that throw `Cannot convert value "%GAMING_SERVICES_VERSION%"
> to type "System.Version"`). Don't bother running it. The snippet below
> uses `Add-AppxPackage` directly and is what actually works.

Open an **elevated** PowerShell prompt and run:

```powershell
# Adjust to whichever GDK edition you have installed (highest-numbered folder
# under "C:\Program Files (x86)\Microsoft GDK\"). 260400 = April 2026 GDK.
$bundle = 'C:\Program Files (x86)\Microsoft GDK\260400\windows\redist\GamingServices.appxbundle'

# 1. Stop the running services so the package isn't locked.
Stop-Service GamingServices, GamingServicesNet -Force -ErrorAction Continue

# 2. Remove the existing (consumer Store) package for all users.
Get-AppxPackage -AllUsers Microsoft.GamingServices |
    ForEach-Object { Remove-AppxPackage -AllUsers -Package $_.PackageFullName }

# 3. Install the GDK-shipped bundle.
Add-AppxPackage -Path $bundle -ForceApplicationShutdown

# 4. Provision so it survives new user profiles.
Add-AppxProvisionedPackage -Online -PackagePath $bundle -SkipLicense | Out-Null

# 5. Restart services and verify.
Start-Service GamingServices, GamingServicesNet
Get-AppxPackage Microsoft.GamingServices | Select-Object Name, Version
& 'C:\Program Files (x86)\Microsoft GDK\bin\XblPCSandbox.exe'
```

The `Get-AppxPackage` line should now print the GDK-paired version (e.g.
`35.112.20003.0` for GDK 260400) instead of the consumer build, and
`XblPCSandbox.exe` should print `Current Sandbox: RETAIL` instead of the
"Gaming Services must be updated" error. You can then switch sandboxes:

```powershell
& 'C:\Program Files (x86)\Microsoft GDK\bin\XblPCSandbox.exe' XDKS.1
```

> **Store auto-update.** The Microsoft Store may eventually re-push the
> newer consumer build of `Microsoft.GamingServices`, which would put you
> back in the broken state. If that happens, just re-run the snippet above.
> A future GDK edition is expected to ship a matching newer build.

---

## Related docs

- [`godot-gdk-sample-setup.md`](godot-gdk-sample-setup.md) — per-sample
  Partner Center configuration (Title ID, SCID, MSA App ID, etc.).
- [`godot-gdk-async-system.md`](godot-gdk-async-system.md) — how the
  bootstrap silent sign-in op completes.
- [`godot-gdk-api-reference.md`](godot-gdk-api-reference.md) — full GDK
  surface (`GDKUsers`, `GDKUser`, `get_gamer_picture_async`, …).
