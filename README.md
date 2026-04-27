# GodotGDK

A repository of Godot GDExtension addons for the Microsoft public GDK
(Game Development Kit). The primary addon is `godot_gdk`, with a second
`godot_gameinput` addon scaffolded as a separate build target.

## Features (POC)

- **GDK Runtime Lifecycle** — Initialize/shutdown the GDK runtime with proper async task queue
- **Xbox User Identity** — Sign in with Xbox Live (silent + UI), get gamertag/XUID/gamer picture
- **Xbox Achievements** — Unlock achievements, check status, update progress via Xbox Live
- **GameInput Controllers** — Native GameInput support for Xbox controllers with rumble
- **GDScript API** — Clean singleton-based API (`GDK`, `GDKUser`, `GDKInput`, `GDKAchievements`)

## Requirements

- Windows 10 (18362+) or Windows 11
- [Microsoft GDK](https://developer.microsoft.com/en-us/games/xbox/partner/resources-gdk) with Xbox Extensions (install via `winget install Microsoft.Gaming.GDK`)
- [Godot 4.3+](https://godotengine.org/download)
- Visual Studio 2022 with C++ Desktop workload
- CMake 3.20+

## Building

### 1. Clone with submodules

```powershell
git clone --recurse-submodules https://github.com/your-org/godot-gdk.git
cd godot-gdk
```

If you've already cloned without submodules:

```powershell
git submodule update --init --recursive
```

### 2. Configure and build

```powershell
# Configure all addons
cmake --preset default

# Build debug
cmake --build build --preset debug

# Build release
cmake --build build --preset release
```

The build:
- Outputs addon DLLs to `addons/godot_gdk/bin/` and `addons/godot_gameinput/bin/`
- Copies built addon DLLs into `sample/addons/godot_gdk/bin/` and `sample/addons/godot_gameinput/bin/`
- Copies required GDK runtime DLLs (XSAPI, libHttpClient) for `godot_gdk` to both locations

Selective configure/build flows are also available:

```powershell
# Configure and build only the GDK addon
cmake --preset gdk-only
cmake --build --preset debug-gdk

# Configure and build only the GameInput addon
cmake --preset gameinput-only
cmake --build --preset debug-gameinput
```

### 3. Repository layout

- `addons/godot_gdk/` — addon assets, addon-local `CMakeLists.txt`, and native sources under `src/`
- `addons/godot_gameinput/` — second addon scaffold with its own `CMakeLists.txt` and `src/`
- `cmake/` — shared CMake helpers for addon output naming, sample sync, and GDK dependency discovery
- `sample/` — shared Godot sample project populated by each addon's build steps

### 4. CMake auto-detection

CMake automatically detects the GDK dependencies needed by `godot_gdk`:
- **GDK GameKit** — via the `GRDKLatest` env var or `C:/Program Files (x86)/Microsoft GDK/`
- **Xbox Services API (XSAPI)** — from `ExtensionLibraries/Xbox.Services.API.C`
- **libHttpClient** — from `ExtensionLibraries/Xbox.LibHttpClient`

If auto-detection fails, override manually:

```powershell
cmake --preset default `
    -DGDK_GAMEKIT="C:/path/to/GameKit" `
    -DXSAPI_ROOT="C:/path/to/Xbox.Services.API.C" `
    -DLIBHTTPCLIENT_ROOT="C:/path/to/Xbox.LibHttpClient"
```

## VS Code Setup

After building, VS Code IntelliSense should work automatically with the included
`.vscode/c_cpp_properties.json`. If you see red squiggles on `#include` directives:

1. Ensure you've **built at least once** — godot-cpp headers are generated during the first build
   into `build/godot-cpp/gen/include/`
2. If your GDK is installed at a non-default path, update the paths in
   `.vscode/c_cpp_properties.json`
3. Reload VS Code (`Ctrl+Shift+P` → "C/C++: Reset IntelliSense Database")

The config defines `_GAMING_DESKTOP` which is required for XSAPI/libHttpClient platform detection.

## Sample Project Setup

The sample project needs your **Partner Center** credentials to work with Xbox Live services.
You can configure everything through the **in-editor GDK Setup panel** or via a CLI script.

### Prerequisites

1. **Register your title** in [Partner Center](https://partner.microsoft.com/)
2. **Create test accounts** in Partner Center → Account Settings → Xbox Live → Test Accounts
3. **Configure achievements** (optional) in Partner Center → Xbox Live → Achievements, then
   publish to your sandbox
4. Gather these values from Partner Center → Xbox Live → Xbox Live Setup:

| Value | Where to find it | Example |
|-------|-------------------|---------|
| Title ID | Xbox Live Setup | `6718942c` |
| MSA App ID | Xbox Live Setup | `93900f42-4313-...` |
| Store ID | Product identity page | `9XXXXXXXXX` |
| SCID | Xbox Live Setup | `00000000-0000-0000-0000-000067...` |
| Sandbox ID | Xbox Live Setup | `XDKS.1` |
| Publisher CN | Product identity page | `CN=XXXXXXXX-XXXX-...` |

### Option A: Configure in the Godot editor (recommended)

1. Build the addon and open the sample in the editor:
   ```powershell
   cmake --build build --preset debug
   .\sample\launch_editor.bat
   ```
2. Find the **GDK Setup** panel in the bottom-right dock
3. Enter your Partner Center values
4. Click **Save Configuration** — this writes `sample_config.cfg` (used at runtime by the
   sample's GDScript)
5. Click **Apply to Export Preset** — this pushes the same values into the export preset
   (used when packaging for distribution)

The config file is gitignored, so your credentials stay local.

### Option B: Configure via CLI

```powershell
.\tools\setup_sample.ps1
```

This prompts for each value and generates `sample_config.cfg`, `MicrosoftGame.config`,
and updates `export_presets.cfg` in one step.

### Set your PC sandbox

Your PC must be in the same sandbox as your test account:

```powershell
# Set sandbox (requires admin)
& "C:\Program Files (x86)\Microsoft GDK\bin\XblPCSandbox.exe" YOUR_SANDBOX_ID

# Verify
& "C:\Program Files (x86)\Microsoft GDK\bin\XblPCSandbox.exe" 
```

### Test account sign-in

The sample uses **Xbox test accounts**, not personal Microsoft accounts:

1. Ensure your PC sandbox matches the sandbox in Partner Center
2. Launch the sample — it will attempt silent sign-in automatically
3. If prompted, sign in with your **test account** credentials (not your personal account)
4. Your test account must be provisioned in Partner Center under the same sandbox

> **Tip:** If sign-in fails, verify the sandbox with `XblPCSandbox.exe` and check that your
> test account exists in Partner Center → Account Settings → Xbox Live → Test Accounts.

### How configuration flows

```
sample_config.cfg (single source of truth)
  ├─► gdk_bootstrap.gd    reads SCID at runtime → initializes Xbox Live
  ├─► main.gd             reads achievement ID at runtime → unlock button
  ├─► export preset        auto-populates defaults → used during export
  └─► MicrosoftGame.config generated at export time from preset values
```

The **GDK Setup panel** and **export dialog** both read from `sample_config.cfg`. If a value
is set in the export preset, it takes priority. If it's blank, the config file value is used
as a fallback.

### Quick start (after setup)

```powershell
# Build
cmake --build build --preset debug

# Launch editor
.\sample\launch_editor.bat
```

## Usage

1. Copy `addons/godot_gdk/` into your Godot project
2. Enable the addon in Project → Project Settings → Plugins
3. Use from GDScript:

```gdscript
func _ready():
    GDK.initialize()
    var user = await GDKUser.user_signed_in
    print("Hello, ", user.gamertag)
```

## GDScript API

### GDK (GDKCore singleton)
- `initialize() → Error` — Start the GDK runtime
- `shutdown()` — Clean up
- `tick()` — Dispatch async callbacks (call in `_process`)
- `get_version() → String`
- Signals: `initialized`, `shutdown_completed`, `error_occurred(message)`

### GDKUser (GDKUserManager singleton)
- `sign_in()` — Xbox sign-in with UI
- `sign_in_silently()` — Silent sign-in (falls back to UI)
- `get_current_user() → GDKUser`
- `get_gamer_picture()` — Fetch user's profile picture (async)
- `is_signed_in() → bool`
- Signals: `user_signed_in(user)`, `sign_in_failed(error)`, `gamer_picture_loaded(texture)`

### GDKInput singleton
- `initialize() → Error` — Start GameInput
- `shutdown()` — Clean up
- `process()` — Poll controllers (call in `_process`)
- `get_connected_device_count() → int`
- Signals: `device_connected(joy_id)`, `device_disconnected(joy_id)`

### GDKAchievements singleton
- `initialize(scid: String) → Error` — Initialize Xbox Live services with your SCID
- `shutdown()` — Clean up
- `unlock(achievement_id: String)` — Unlock an achievement (100% progress)
- `update_progress(achievement_id: String, percent: int)` — Set achievement progress (1-100)
- `check_achievement(achievement_id: String)` — Query whether an achievement is already unlocked
- `is_initialized() → bool`
- Signals: `achievement_unlocked(id)`, `achievement_update_failed(id, error)`, `achievement_checked(id, is_unlocked)`

## Testing Achievements

Achievements must be configured in [Partner Center](https://partner.microsoft.com/) and published
to your development sandbox before they can be unlocked.

To **reset** achievements for re-testing, use the included helper script:

```powershell
.\tools\reset_player_data.ps1
```

This signs into Partner Center via `XblDevAccount.exe`, then calls `XblPlayerDataReset.exe`
to wipe achievements, stats, and leaderboards for the specified test account. You'll need:

- **Service Config ID (SCID)** — from Partner Center → Xbox Live → Xbox Live Setup
- **Sandbox ID** — the development sandbox your test account is signed into
- **XUID** — the Xbox User ID of the test account to reset

> **Note:** Resets only work on Xbox test accounts in a development sandbox, not retail accounts.
> Restart the game after resetting for changes to take effect.

