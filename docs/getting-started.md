# Getting Started

This guide walks you through using the Godot XBOX addons in your own Godot
project — what to copy, how to enable it, how to configure project settings,
and how to reach **a signed-in user** for both XBOX Live (`godot_gdk`) and
PlayFab (`godot_playfab`).

It also covers building the addon binaries from source, which is the current
way to obtain them.

## Who this is for

- **Godot developers** building a Windows game that needs to ship with
  Microsoft Store / Microsoft GDK identity, XBOX Live services, PlayFab backend
  features, GameInput, or MSIXVC packaging.
- **Comfortable with Godot 4.x** as an everyday tool: scenes, autoloads,
  signals, `await`, and the in-editor F1 documentation.
- **Have at least a sandbox Partner Center title** (or plan to) — the
  XBOX-services side of the addons cannot meaningfully be exercised
  without a title id, SCID, and a sandbox.

You do **not** need prior Microsoft GDK or PlayFab experience to follow
the [tutorials](tutorials/README.md). You **do** need a working Godot
project, a Windows PC, and (for live tests) a sandbox-provisioned test
account.

## You should already know

- Godot 4.x editor workflow (creating scenes, attaching scripts,
  using autoloads, reading the Output panel).
- GDScript basics: `func`, `var` with type hints, `await`, `signal`,
  `match`. The tutorial snippets are dense; you should be able to read
  one without looking up GDScript syntax.
- One-shot async with `await Signal` — this is how every long-running
  call in the addons resolves. The dedicated
  [Async patterns](async-patterns.md) page is the one-page primer the
  tutorials assume you've read.

## You need

- Windows 10 (build 18362+) or Windows 11
- [Godot 4.5+](https://godotengine.org/download) (stable, Windows
  64-bit)
- [Microsoft GDK](https://github.com/microsoft/GDK/releases) installed
  (`winget install Microsoft.Gaming.GDK`)
- A built copy of each addon you want to use — built from source per
  **Step 1** below
- For XBOX-services tutorials (T1+): a Partner Center title + SCID +
  sandbox test account
- For PlayFab tutorials (T1+): a PlayFab title id
- A scratch Godot project to copy the addons into (the tutorials build
  one from scratch — you do not need to clone this repo as a
  Godot project)

> **TL;DR**
> 1. Build the addon binaries from source (Step 1 below).
> 2. Copy `addons/<addon>/` into your project, including the `bin/` folder.
> 3. Enable the editor plugin in **Project Settings → Plugins**.
> 4. Set the `gdk/runtime/*` and `playfab/*` project settings.
> 5. Subscribe to `GDK.users.user_changed` (XBOX) and call
>    `PlayFab.users.sign_in_with_xuser_async(...)` (PlayFab).

## What's in this repo

| Addon | Purpose |
|-------|---------|
| `addons/godot_gdk` | Microsoft GDK runtime + PC-supported XBOX services (users, achievements, presence, social, leaderboards, multiplayer activity, store, system, …). Installs a `GDKBootstrap` autoload. |
| `addons/godot_playfab` | PlayFab runtime, sign-in (XBOX-backed or custom id), Game Saves, leaderboards, Lobby + Matchmaking, Party, and REST service wrappers. Installs a `PlayFabBootstrap` autoload (auto-init is opt-in). |
| `addons/godot_gameinput` | GameInput controller integration (devices, polling, vibration, action-bridge into Godot's `InputMap`). Installs a `GameInputBootstrap` autoload. |
| `addons/godot_gdk_packaging` | GDScript-only editor plugin for PC MSIXVC packaging via `makepkg.exe`. **Editor-only**, no runtime. |

The four addons are independent — ship one, several, or all of them.

When something goes wrong while you follow the tutorials, the
[Troubleshooting](troubleshooting.md) page collects the failures we see
most often (DLL load 126, SCID mismatch, sandbox mismatch, schema
errors in `MicrosoftGame.config`, etc.).

---

## Prerequisites

### To run the addons in your game

- Windows 10 (build 18362+) or Windows 11
- [Godot 4.5+](https://godotengine.org/download) (stable, Windows 64-bit)
- [Microsoft GDK](https://github.com/microsoft/GDK/releases) installed on
  every machine that runs the game (the XBOX runtime DLLs the addons depend
  on resolve from the Microsoft GDK install). `winget install Microsoft.Gaming.GDK`.

### To make XBOX sign-in actually work

- A title in [Partner Center](https://partner.microsoft.com/dashboard)
  (Title ID, SCID, Sandbox ID at minimum)
- At least one XBOX **test account** provisioned in your sandbox
- Your dev PC switched into that sandbox

> **First time?** You need an XBOX publisher account before you can
> register a Partner Center title. Start with the
> [ID@XBOX program overview](https://www.xbox.com/en-us/developers/id) /
> [application form](https://www.xbox.com/en-us/developers/id/apply)
> and the [Microsoft Game Stack publisher hub](https://developer.microsoft.com/en-us/games/publish/).
> Then follow [Microsoft GDK — Get started](https://learn.microsoft.com/en-us/gaming/gdk/docs/gdk-dev/get-started/get-started-home),
> [Configuring XBOX services (Title ID + SCID)](https://learn.microsoft.com/en-us/gaming/gdk/docs/services/fundamentals/portal-config/live-service-config-ids-mp),
> and [Setting up sandboxes](https://learn.microsoft.com/en-us/gaming/gdk/docs/services/fundamentals/sandboxes/live-setup-sandbox)
> for the Microsoft-side walkthrough that produces the IDs the rest of
> this guide refers to.

See [Sample project setup](gdk/sample-setup.md) and
[XBOX sandbox and test-account setup](platform/XBOX-sandbox-and-test-accounts.md)
for the canonical addon-side walk-through.

### To make PlayFab sign-in work

- A PlayFab title (you'll need its title id)
- For the XBOX-backed `sign_in_with_xuser_async` flow, the same XBOX
  prerequisites above
- For `sign_in_with_custom_id_async`, no XBOX setup is required

> **First time?** You need a PlayFab title before you can fill in
> `playfab/runtime/title_id`. Sign up at the
> [PlayFab developer portal](https://developer.playfab.com/) and follow
> [PlayFab — Game Manager quickstart](https://learn.microsoft.com/en-us/gaming/playfab/gamemanager/quickstart)
> to create your account, studio, and first title. The Title ID lives in
> Game Manager under **your title → Settings → API features**. The
> [PlayFab Learn hub](https://learn.microsoft.com/en-us/gaming/playfab/)
> is the entry point for everything else.

### To build the addons from source

- Visual Studio 2022+ with the **C++ Desktop** workload
- CMake 3.25+ (required by the `CMakePresets.json` schema version)
- [vcpkg](https://github.com/microsoft/vcpkg) in manifest mode. The build
  reads `vcpkg.json` + `vcpkg-configuration.json` at the repo root and
  resolves the `ms-gdk[playfab]` and `gameinput` ports for you, so you
  don't need a separate Microsoft GDK install just to compile the addons.
  Set the `VCPKG_ROOT` environment variable to your vcpkg clone (the
  CMake preset reads it via `$env{VCPKG_ROOT}`).

> **Note:** The vcpkg manifest only provides the **build-time headers and
> import libs** for Microsoft GDK + GameInput. You still need a full Microsoft GDK
> install on any machine where you intend to run `makepkg.exe`, `wdapp.exe`,
> or the Game Config Editor (see [Editor tools](gdk/editor-tools.md) and
> [Packaging plugin](packaging/plugin.md)).

> **Note:** The Debug build of the Microsoft GDK addon requires Visual Studio to be
> installed on the machine that *runs* the game, not just the machine that
> builds it. Use the Release build for distribution. See
> [Troubleshooting](troubleshooting.md) for details.

---

## 1. Build the addon binaries

You need a built copy of each addon you want to use. Build the
drop-in layout from source: clone with submodules and run the package
helper.

```powershell
git clone --recurse-submodules https://github.com/microsoft/Godot-XBOX.git
cd Godot-XBOX

.\tools\package_addons.ps1
```

The helper configures the `addon-package` CMake preset, builds Debug and
Release native addon DLLs by default, stages the drop-in addon files under
`build\dist\godot-gdk-addons\addons\`, and writes
`build\dist\godot-gdk-addons-debug-release.zip`. Copy the addon folders
you need from `build\dist\godot-gdk-addons\addons\` into your Godot
project's `addons/` folder (or extract the zip there).

The package build:

- Outputs each addon's DLLs and PDBs into `addons/<addon>/bin/`
- Copies the runtime dependency DLLs (`libHttpClient.dll`,
  `Microsoft.XBOX.Services.C.Thunks.dll` for Release or
  `Microsoft.XBOX.Services.C.Thunks.Debug.dll` for Debug, `PlayFabCore.dll`,
  `PlayFabServices.dll`, `PlayFabGameSave.dll`, `PlayFabMultiplayer.dll`,
  `Party.dll`) into the same `bin/` folder so they ship side-by-side
- Syncs the addon directories into every sample project under `sample/`

If you only need one addon during development, use the targeted presets
(`gdk-only`, `playfab-only`, `gameinput-only`). See the
[repo README](../README.md#build-presets) for the full preset table.

For deeper notes on the build pipeline (CMake auto-detection of the Microsoft GDK
install, selective builds, VS Code IntelliSense, ignored-artifact
cleanup), jump to [Building from source](#building-from-source) at the
end of this guide.

---

## 2. Copy the addons into your project

For each addon you want, copy the **entire** `addons/<addon>/` directory
into your project's `addons/` folder. The shape that needs to land in
your project is:

```
your_project/
└── addons/
    └── godot_gdk/
        ├── bin/                       # native DLLs + runtime deps
        │   ├── godot_gdk.windows.debug.x86_64.dll
        │   ├── godot_gdk.windows.release.x86_64.dll
        │   ├── libHttpClient.dll
        │   ├── Microsoft.XBOX.Services.C.Thunks.Debug.dll
        │   └── Microsoft.XBOX.Services.C.Thunks.dll
        ├── doc_classes/               # in-editor F1 documentation
        ├── editor/                    # editor plugin scripts
        ├── runtime/                   # GDKBootstrap autoload
        ├── godot_gdk.gdextension      # extension manifest
        └── plugin.cfg                 # editor plugin manifest
```

Repeat for `godot_playfab/` and `godot_gameinput/` if you want them. The
`bin/` folder of each addon already contains every native runtime
dependency that addon needs — copy the directory recursively and you're
done.

> **Don't strip `bin/`.** Without the DLLs Godot logs `GDExtension
> dynamic library not found` and the addon's singletons never register.
> 64-bit Windows is the only target the addons currently build for.

> ⚠️ **Copy from the packaged output, not a raw dev build.** The
> `build\dist\godot-gdk-addons\addons\` layout produced by
> [`tools\package_addons.ps1`](#1-build-the-addon-binaries) is already
> consumer-ready. A raw in-tree `addons\<addon>\` directory from a
> `cmake --build` dev build *also* contains `src\`, `tests_support\`, and
> `CMakeLists.txt`. The `tests_support\` scripts extend `GutTest` (from
> the GUT testing framework, which your project won't have), so copying
> them in makes Godot throw parse errors like
> `Could not find base class "GutTest"` on load. If you must copy from a
> dev build, exclude `src\`, `tests_support\`, and `CMakeLists.txt` —
> keep only `bin\`, `runtime\`, `editor\`, `doc_classes\`, the
> `.gdextension` manifest, and `plugin.cfg`.

---

## 3. Enable the editor plugin

Open your project in Godot once so it discovers the new `plugin.cfg`
files, then go to **Project → Project Settings → Plugins** and enable:

| Plugin name in the Plugins tab | What enabling it does |
|--------------------------------|------------------------|
| `GodotGDK` | Installs the `GDKBootstrap` autoload at `res://addons/godot_gdk/runtime/gdk_bootstrap.gd`. |
| `GodotPlayFab` | Installs the `PlayFabBootstrap` autoload at `res://addons/godot_playfab/runtime/playfab_bootstrap.gd`. The autoload only calls `PlayFab.initialize()` automatically when `playfab/runtime/initialize_on_startup` is `true` — sign-in stays in your code. |
| `Godot GameInput` | Installs the `GameInputBootstrap` autoload at `res://addons/godot_gameinput/runtime/gameinput_bootstrap.gd`. |
| `GDK Packaging` | (Optional, editor-only) registers MSIXVC packaging tooling under the editor's tools menu. |

Disabling a plugin removes its autoload again — there's no orphaned
state.

### Enabling plugins from `project.godot` (CI / automated setup)

The Plugins tab just writes an `[editor_plugins]` section to
`project.godot`. For headless setup, CI, onboarding scripts, or if you
prefer editing config directly, add the section yourself (list only the
addons you actually copied in):

```ini
[editor_plugins]

enabled=PackedStringArray("res://addons/godot_gdk/plugin.cfg", "res://addons/godot_playfab/plugin.cfg", "res://addons/godot_gameinput/plugin.cfg", "res://addons/godot_gdk_packaging/plugin.cfg")
```

Without this section Godot opens with every plugin disabled and prints
no error — the autoloads and the **GDK** editor menu simply never appear.

### Verify the addons load (headless)

To confirm the addons resolve without parse or load errors — useful for
agents and CI that can't visually check the editor — open the project
once in headless mode and inspect the output:

```powershell
godot --headless --path path\to\your_project --quit 2>&1 |
  Select-String -Pattern "Parse Error", "GDExtension dynamic library not found"
```

No matching lines means the addons loaded cleanly. (You can point
`--path` at the in-repo `sample\tutorial_app` project once its addon
binaries have been built and synced.) This is a quick load smoke test —
the repo's own GDScript validator
(`tools\check_gd_scripts_headless.ps1`) remains the authoritative parse
gate for contributors.

---

## 4. Configure project settings

The bootstrap autoloads consume a handful of `ProjectSettings` entries.
You can edit them in **Project → Project Settings → General** (toggle
**Advanced Settings**) or write them straight into `project.godot`.

### `godot_gdk`

```ini
[gdk]

runtime/initialize_on_startup=true   ; bootstrap calls GDK.initialize() at startup
runtime/auto_add_primary_user=true   ; bootstrap calls add_default_user_async() after init
runtime/embed_dispatch=true          ; pump async completions in _process (default)
```

Leave `embed_dispatch` on unless you want to drive `GDK.dispatch()`
yourself for deterministic frame control.

### `godot_playfab`

```ini
[playfab]

title_id=""                          ; REQUIRED: your PlayFab title id
endpoint=""                          ; optional: blank derives https://<titleid>.playfabapi.com
runtime/initialize_on_startup=true   ; bootstrap calls PlayFab.initialize() at startup (default false)
runtime/embed_dispatch=true          ; pump async completions in _process (default)
```

When `initialize_on_startup` is `true`, the `PlayFabBootstrap` autoload
calls `PlayFab.initialize()` during `_ready` — the same shape as the Microsoft GDK
bootstrap. Sign-in is still in your code because PlayFab needs a
per-player key (a `GDKUser` or a custom id).

### `godot_gameinput`

Example override (the registered default for `runtime/initialize_on_startup` is `false`):

```ini
[game_input]

runtime/initialize_on_startup=true   ; example override: bootstrap calls GameInput.initialize() at startup
runtime/auto_poll=true               ; bootstrap calls GameInput.poll() in _process (default)
mapper/default_action_map=""         ; optional path to a GameInputActionMap .tres
```

See [GameInput addon](gameinput/plugin.md) for the action-bridge details.

---

## 5. Walkthrough — get to a signed-in user

This is the smallest thing that proves the addons are wired up
correctly: an XBOX sign-in (godot_gdk) followed by a PlayFab sign-in
that reuses the XBOX user (godot_playfab).

### 5a. XBOX sign-in (`godot_gdk`)

If you set `gdk/runtime/initialize_on_startup` and
`gdk/runtime/auto_add_primary_user` to `true`, the `GDKBootstrap`
autoload does this for you on `_ready` — your code just needs to react
to the result.

```gdscript
extends Node
## Drop this on a node in your first scene.

func _ready() -> void:
    if not Engine.has_singleton("GDK"):
        push_error("godot_gdk extension is not loaded — check addons/godot_gdk/bin/")
        return

    GDK.users.user_changed.connect(_on_user_changed)

    # The bootstrap may already have signed someone in by the time _ready runs.
    var user: GDKUser = GDK.users.get_primary_user()
    if user != null:
        _show_user(user)

func _on_user_changed(user: GDKUser, change_kind: String) -> void:
    if change_kind == "added" and user == GDK.users.get_primary_user():
        _show_user(user)
    elif change_kind == "removed":
        print("[GDK] user removed: %d" % user.local_id)

func _show_user(user: GDKUser) -> void:
    print("[GDK] signed in as %s (XUID %s)" % [user.gamertag, user.xuid])
```

If you want full control instead, leave the two `gdk/runtime/*` flags
off and call the lifecycle yourself:

```gdscript
func _ready() -> void:
    var init_result: GDKResult = GDK.initialize()
    if not init_result.ok:
        push_warning("GDK.initialize failed: %s" % init_result.message)
        return

    var sign_in_result: GDKResult = await GDK.users.add_default_user_async()
    if not sign_in_result.ok:
        push_warning("Silent sign-in failed: %s" % sign_in_result.message)
        return

    var user: GDKUser = sign_in_result.data
    print("[GDK] signed in as %s" % user.gamertag)

func _exit_tree() -> void:
    if GDK.is_initialized():
        GDK.shutdown()
```

`add_default_user_async()` is the XBOX **silent** sign-in. If no user is
already signed into the XBOX app on the PC, it returns a non-ok result
(commonly `no_default_user`). Use `add_user_with_ui_async()` to put the
system account picker on screen instead.

For the full method/signal table see
[Microsoft GDK API reference → `GDK.users`](gdk/api-reference.md#users-service-gdkusers).

> Real XBOX Live sign-in needs Partner Center configuration, the right
> sandbox set on the PC, and a test account signed into the XBOX app —
> see [Sample project setup](gdk/sample-setup.md) and
> [XBOX sandbox and test-account setup](platform/XBOX-sandbox-and-test-accounts.md).
> Without those, sign-in will report a clear error and the rest of the
> game keeps running fine.

### 5b. PlayFab sign-in with the XBOX user (`godot_playfab`)

If you set `playfab/runtime/initialize_on_startup` to `true`, the
`PlayFabBootstrap` autoload calls `PlayFab.initialize()` for you. Sign-in
still goes through your code because PlayFab needs a per-player key —
either a `GDKUser` or a custom id.

```gdscript
extends Node

func _ready() -> void:
    await _sign_in_to_playfab()

func _sign_in_to_playfab() -> void:
    if not Engine.has_singleton("PlayFab"):
        push_error("godot_playfab extension is not loaded")
        return
    if not Engine.has_singleton("GDK"):
        push_error("godot_gdk extension is not loaded")
        return

    # Make sure GDK is up and we have an XBOX user.
    if not GDK.is_initialized():
        var init: GDKResult = GDK.initialize()
        if not init.ok:
            push_warning("GDK init failed: %s" % init.message); return

    var XBOX_user: GDKUser = GDK.users.get_primary_user()
    if XBOX_user == null or not XBOX_user.signed_in:
        var XBOX_result: GDKResult = await GDK.users.add_default_user_async()
        if not XBOX_result.ok:
            push_warning("XBOX sign-in failed: %s" % XBOX_result.message); return
        XBOX_user = XBOX_result.data

    # Initialize PlayFab once. The PlayFabBootstrap autoload may have
    # already done this when playfab/runtime/initialize_on_startup is true;
    # the call here is a defensive fallback for projects that disable it.
    if not PlayFab.is_initialized():
        var pf_init: PlayFabResult = PlayFab.initialize()
        if not pf_init.ok:
            push_warning("PlayFab init failed: %s" % pf_init.message); return

    # Sign the XBOX user into PlayFab.
    var pf_result: PlayFabResult = await PlayFab.users.sign_in_with_xuser_async(XBOX_user)
    if not pf_result.ok:
        push_warning("PlayFab sign-in failed: %s" % pf_result.message); return

    var pf_user: PlayFabUser = pf_result.data
    var key: Dictionary = pf_user.entity_key
    print("[PlayFab] signed in: %s:%s" % [key.get("type", ""), key.get("id", "")])
```

If you don't have an XBOX account yet (or are testing on a machine
without Microsoft GDK Live setup), use the no-XBOX custom-ID path:

```gdscript
var pf_result: PlayFabResult = await PlayFab.users.sign_in_with_custom_id_async(
        "your-title-defined-id", false)  # false = don't auto-create
```

`sign_in_with_xuser_async` returns `invalid_xuser` if you pass a null or
signed-out Microsoft GDK user, so always confirm `XBOX_user.signed_in` first.
PlayFab Game Saves additionally require an XBOX-backed PlayFab session
(custom-id users will get `XBOX_user_required` from `PlayFab.game_saves`
methods).

For the full PlayFab service surface see the
[PlayFab plugin overview](playfab/plugin.md).

### 5c. Verify

Run your project. You should see, in order:

```
[GDK] Bootstrap: GDK.initialize() succeeded.
[GDK] Runtime initialized
[GDK] User added: <gamertag>
[GDK] signed in as <gamertag> (XUID <xuid>)
[PlayFab] signed in: title_player_account:<entity-id>
```

If sign-in fails before that point, the `result.message` you printed
tells you which step is broken — see
[Troubleshooting → sign-in](troubleshooting.md) and the sandbox /
test-account guide.

---

## Common pitfalls

| Symptom | Likely cause | Fix |
|---------|--------------|-----|
| `GDExtension dynamic library not found` | The `bin/` folder didn't make it into the project copy | Copy `addons/<addon>/` recursively, including `bin/` |
| `[GDK] Bootstrap: 'GDK' singleton not registered` | Extension failed to load (wrong Windows arch, missing Microsoft GDK install, missing `libHttpClient.dll`) | Check that the addon copy preserved `bin/` and that the Microsoft GDK is installed on the machine that runs the game |
| Silent sign-in returns `no_default_user` | No test account signed in to the XBOX app on the PC, or the PC sandbox doesn't match Partner Center | Set the sandbox with `XblPCSandbox.exe` and sign a test account into the XBOX app — see [XBOX sandbox and test-account setup](platform/XBOX-sandbox-and-test-accounts.md) |
| `PlayFab.initialize()` fails immediately | `playfab/runtime/title_id` is empty | Set `playfab/runtime/title_id` in Project Settings (or `project.godot` `[playfab] runtime/title_id="..."`) |
| `sign_in_with_xuser_async` returns `invalid_xuser` | Passing a null / signed-out Microsoft GDK user | Verify `XBOX_user != null and XBOX_user.signed_in` before calling |
| `PlayFab.game_saves` returns `XBOX_user_required` | The PlayFab session was created with a custom id | Use `sign_in_with_xuser_async` for any flow that touches Game Saves |

---

## Where to go next

- [**Tutorials**](tutorials/README.md) — task-oriented walkthroughs
  for sign-in, achievements, leaderboards, Game Saves, lobbies, and
  GameInput. Recommended next stop once you have signed in.
- [Microsoft GDK API reference](gdk/api-reference.md) — full list of services
  (`GDK.users`, `GDK.achievements`, `GDK.leaderboards`,
  `GDK.multiplayer_activity`, `GDK.store`, `GDK.system`, …)
- [PlayFab plugin overview](playfab/plugin.md) — `PlayFab.users`,
  `PlayFab.game_saves`, `PlayFab.leaderboards`, `PlayFab.multiplayer`,
  `PlayFab.party`, `PlayFab.events`, and the REST service wrappers
- [GameInput addon](gameinput/plugin.md) — devices, polling, vibration,
  and the action-bridge into Godot's `InputMap`
- [Sample project setup](gdk/sample-setup.md) — Partner Center
  configuration, sandboxes, test accounts
- [Troubleshooting](troubleshooting.md) — common build, runtime, and
  test issues

---

## Building from source

Reference for contributors and anyone reproducing the addon binaries
themselves.

### Clone with submodules

```powershell
git clone --recurse-submodules https://github.com/microsoft/Godot-XBOX.git
cd Godot-XBOX
```

If you've already cloned without submodules:

```powershell
git submodule update --init --recursive
```

### Build

For a packaged drop-in build (stages addons under `build\dist\` and
produces an addons zip):

```powershell
.\tools\package_addons.ps1
```

For local development builds:

```powershell
# Configure all addons
cmake --preset default

# Build debug
cmake --build build --preset debug

# Build release
cmake --build build --preset release
```

The build:

- Outputs addon DLLs to `addons/<addon>/bin/`
- Copies built DLLs and runtime dependencies into each sample's `addons/<addon>/bin/`
- Syncs addon metadata, editor scripts, GUT, and shared test support into the sample projects

### Selective builds

```powershell
# GDK addon only
cmake --preset gdk-only
cmake --build --preset debug-gdk

# PlayFab addon only
cmake --preset playfab-only
cmake --build --preset debug-playfab

# GameInput addon only
cmake --preset gameinput-only
cmake --build --preset debug-gameinput
```

### Source for the Microsoft GDK dependency

**Which preset should I use?**

- `VCPKG_ROOT` is set (or you're happy to set it) → **`cmake --preset default`**.
  Resolves the Microsoft GDK + GameInput from vcpkg; no machine-wide GDK
  install needed at build time.
- No vcpkg, but the Microsoft GDK is installed on disk (auto-detected via
  `%GRDKLatest%` / `%GameDK%`, or set explicitly with
  `-DGDK_INSTALL_DIR=<path>`) → **`cmake --preset installed-gdk`**. No vcpkg
  checkout or `VCPKG_ROOT` required.
- Neither → install [vcpkg](https://github.com/microsoft/vcpkg), set
  `VCPKG_ROOT`, then use `cmake --preset default`.

By default, the build resolves the Microsoft GDK headers and import libs through
the `ms-gdk[playfab]` vcpkg port (`default` preset). This requires only
a vcpkg checkout — no machine-wide Microsoft GDK install is needed at build time.

If you already have a Microsoft GDK installed on disk (most developers
do, for `makepkg.exe` / `wdapp.exe` / Game Config Editor), the
`installed-gdk` preset consumes it directly and **does not require vcpkg
at all** — no `VCPKG_ROOT`, no vcpkg checkout, no port restore:

```powershell
# Consume an installed Microsoft GDK (auto-detected via %GRDKLatest% or %GameDK%)
cmake --preset installed-gdk
cmake --build --preset debug-installed-gdk

# Override auto-detection with an explicit path
cmake --preset installed-gdk -DGDK_INSTALL_DIR="C:/Program Files (x86)/Microsoft GDK/260400"
```

> **Note:** the `installed-gdk` preset is the only supported way to
> consume an installed Microsoft GDK. Setting `-DGDK_DEPENDENCY_SOURCE=installed`
> on the `default` preset does **not** work — the vcpkg toolchain
> processes manifest features (and restores `ms-gdk[playfab]`) before
> the Microsoft GDK source-selection logic runs. The `installed-gdk` preset
> sidesteps this by not loading the vcpkg toolchain at all.

Installed mode consumes the modern `windows\` subdirectory layout of the
Microsoft GDK (`<install>/windows/include`, `<install>/windows/lib/x64`,
`<install>/windows/bin/x64`) that ships in Microsoft GDK **260400 / April 2026 and
later**. The legacy `GRDK\` peer layout is not supported; use the
`default` preset (vcpkg) for older Microsoft GDK versions.

**GameInput in installed mode** — the installed Microsoft GDK ships GameInput v1
only, but the `godot_gameinput` addon targets v3. The `installed-gdk`
preset solves this by setting `GAMEINPUT_SOURCE=nuget`, which fetches
the `Microsoft.GameInput` NuGet package directly from nuget.org via
`file(DOWNLOAD)` (cached under `build/installed-gdk/_deps/`). No vcpkg
toolchain is involved; the first configure needs network access, and
subsequent configures reuse the cached extract. This is the same
upstream archive vcpkg's `gameinput` port wraps, so behavior is
identical at runtime. Target machines still need `GameInputRedist.msi`
installed (extracted to `build/installed-gdk/_deps/gameinput-nuget-<version>/redist/`).

Source-selection options:

| `GDK_DEPENDENCY_SOURCE` | Behavior |
|---|---|
| `vcpkg` (default) | Use the `ms-gdk[playfab]` vcpkg port. Set automatically by the `default` preset. Requires `VCPKG_ROOT`. |
| `installed` | Use a Microsoft GDK install on disk. Set automatically by the `installed-gdk` preset (which also drops the vcpkg toolchain — no `VCPKG_ROOT` required). Setting this alone on the `default` preset has no effect on the vcpkg restore. |

| `GAMEINPUT_SOURCE` | Behavior |
|---|---|
| `vcpkg` (default) | Use the `gameinput` vcpkg port. Requires `VCPKG_ROOT`. |
| `nuget` | Fetch the `Microsoft.GameInput` NuGet package directly from nuget.org (no vcpkg required). Set automatically by the `installed-gdk` preset. The pinned version + SHA512 are tracked in `cmake/GDKDependencies.cmake` (override with `-DGDK_GAMEINPUT_NUGET_VERSION=<version> -DGDK_GAMEINPUT_NUGET_SHA512=<hash>`). |

### CMake auto-detection

The build resolves its native dependencies via vcpkg manifest mode. CMake
reads `vcpkg.json` + `vcpkg-configuration.json` at the repo root and pulls
in the `ms-gdk[playfab]` and `gameinput` ports for you. There is no
machine-wide Microsoft GDK install required for compilation — set `VCPKG_ROOT` and
run the `default` preset.

If you ever need to point at a specific vcpkg toolchain file (for example,
to share an installed cache between repos), override it on the CMake
command line:

```powershell
cmake --preset default -DCMAKE_TOOLCHAIN_FILE="C:/path/to/vcpkg/scripts/buildsystems/vcpkg.cmake"
```

### Clean ignored local artifacts

Use the repo cleanup helper to preview or remove ignored local files such as
`build\`, addon/sample `bin\`, sample `.godot\`, local sample configs or Godot
editor copies under `sample\`, and generated packaging output.

```powershell
# Preview what would be removed
.\tools\clean_repo.ps1

# Remove ignored local artifacts
.\tools\clean_repo.ps1 -Apply
```

The script wraps `git clean` in ignored-files-only mode, so tracked repository
files stay intact.

### Run the bundled samples

> **No sample projects currently.** The repository is mid-revamp;
> samples are returning in PR 3 of the tutorial-driven sample
> series (`sample/tutorial_app/` and `sample/tutorial_gameinput/`).
> Until then, follow [the tutorials](tutorials/README.md) in your
> own Godot project.

### Run the tests

Use the repo-wide orchestrator as the canonical local test command:

```powershell
pwsh -NoLogo -NoProfile -ExecutionPolicy Bypass -File .\tools\run_all_tests.ps1
```

The orchestrator runs the GDScript parse gate, debug CMake build, C++
doctest executable, GUT host suites, bootstrap mini-runners, and the
aggregate summary. Results are written to
`build\test-results\run-summary.json` and `.md`. See
[Sample and tests](gdk/sample-and-tests.md) for the full pipeline,
the live switch, and troubleshooting links.

### VS Code setup

After building, VS Code IntelliSense should work automatically with the
included `.vscode/c_cpp_properties.json`. If you see red squiggles on
`#include` directives:

1. Ensure you've **built at least once** — godot-cpp headers are generated
   during the first build into `build/godot-cpp/gen/include/`, and vcpkg
   stages the Microsoft GDK + GameInput headers into `build/vcpkg_installed/`.
2. Ensure `VCPKG_ROOT` is set in your environment so the IntelliSense
   include paths resolve. If you point IntelliSense at a different vcpkg
   install than the build uses, update `.vscode/c_cpp_properties.json` to
   match.
3. Reload VS Code (`Ctrl+Shift+P` → "C/C++: Reset IntelliSense Database").

The config defines `_GAMING_DESKTOP`, which is required for XSAPI /
libHttpClient platform detection.

### Repository layout

```
addons/godot_gdk/         # GDK addon: metadata, editor scripts, native sources
addons/godot_gameinput/   # GameInput addon: metadata, native sources
addons/godot_playfab/     # PlayFab addon: metadata, native sources
addons/godot_gdk_packaging/   # GDScript-only packaging tools (editor-only)
cmake/                    # Shared CMake helpers
docs/                     # Documentation
godot-cpp/                # godot-cpp submodule
sample/                   # Sample projects (returning in PR 3 of the
                          # tutorial-driven sample revamp:
                          #   tutorial_app/        — integrated chain
                          #   tutorial_gameinput/  — standalone GameInput
                          # ; currently empty)
tests/                    # Baselines, C++ doctest sources, and Godot test hosts
  godot/gdk/              # GDK and GDK packaging test host
  godot/playfab/          # PlayFab test host
  godot/gameinput/        # GameInput test host
third_party/Gut/          # GUT (bitwes/Gut) submodule — mirrored into test hosts
tools/                    # CLI helper scripts
```

### Development workflow

#### After changing native code

```powershell
cmake --build build --preset debug
```

This rebuilds the DLL and syncs it, addon metadata, and generated test
support into the sample projects.

#### After changing editor scripts or addon metadata

Rebuild so the sample copy is refreshed:

```powershell
cmake --build build --preset debug
```

#### Validating changes

1. Rebuild the addon or run the full orchestrator.
2. Run `pwsh -NoLogo -NoProfile -ExecutionPolicy Bypass -File .\tools\run_all_tests.ps1`.
3. Open the relevant sample in the editor and verify the user-facing flow still loads.
4. If XBOX Live or PlayFab live features changed, test with `-Live` against a sandbox title and test account.

#### Optional pre-commit hook

Enable the repo-managed pre-commit hook to run headless GDScript validation before each commit:

```powershell
git config core.hooksPath .githooks
```
