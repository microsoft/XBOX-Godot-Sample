# GodotGDK addons — getting started

A quickstart for the addon zip. This walks through enabling the addons,
setting the PlayFab title id, creating the GDK game config, switching
the Xbox sandbox, and signing a user in — first into Xbox Live, then
into PlayFab.

For the full repo guide (building from source, samples, deeper API
notes), see `docs/getting-started.md` in the source repo.

## What's in the zip

```
addons/
  godot_gdk/             Microsoft GDK runtime + Xbox services
  godot_playfab/         PlayFab runtime, sign-in, Game Saves, leaderboards, multiplayer
  godot_gameinput/       GameInput controller integration (optional)
  godot_gdk_packaging/   Editor-only: game config, sandbox, packaging, package manager
```

Each addon is independent — copy in only the ones you need. Always copy
the whole `addons/<addon>/` directory (including its `bin/` folder) into
your Godot project's `addons/` directory.

## Prerequisites

- Windows 10 (build 18362+) or Windows 11, 64-bit
- Godot 4.5+ stable, Windows 64-bit
- [Microsoft GDK](https://github.com/microsoft/GDK/releases) installed
  on every machine that runs the game (`winget install
  Microsoft.Gaming.GDK`). The Xbox runtime DLLs the addons depend on
  resolve from the GDK install.
- A PlayFab title (you'll need its title id) for the `godot_playfab`
  addon. Sign up at the [PlayFab portal](https://playfab.com/).
- A **Partner Center title** with Xbox Live configured, plus at least one
  Xbox **test account** provisioned in the title's sandbox.

> **First time setting up a Partner Center title?** You need an Xbox
> publisher account before you can register a title. Start here:
>
> - [ID@Xbox program overview](https://www.xbox.com/en-us/developers/id)
>   and [application form](https://www.xbox.com/en-us/developers/id/apply)
>   — Microsoft's on-ramp for independent developers.
> - [Microsoft Game Stack — Publish](https://developer.microsoft.com/en-us/games/publish/)
>   — top-level publisher resources.
> - [Partner Center dashboard](https://partner.microsoft.com/dashboard)
>   — where you register the title and read the Title ID, SCID, MSA App
>   ID, Store ID, and Sandbox ID values this guide refers to.
> - [Microsoft GDK — Get started](https://learn.microsoft.com/en-us/gaming/gdk/docs/gdk-dev/get-started/get-started-home)
>   and [Configuring Xbox services (Title ID + SCID)](https://learn.microsoft.com/en-us/gaming/gdk/docs/services/fundamentals/portal-config/live-service-config-ids-mp)
>   for the canonical Microsoft walkthrough of the IDs and configuration
>   surfaces this guide assumes you already have.
> - [Microsoft GDK — Setting up sandboxes](https://learn.microsoft.com/en-us/gaming/gdk/docs/services/fundamentals/sandboxes/live-setup-sandbox)
>   for the sandbox model (test accounts only authenticate against the
>   sandbox they were created in).

## 1. Enable the addons

1. Drop the `addons/` directory from this zip into your Godot project
   root (so you end up with `your_project/addons/godot_gdk/...`).
2. Open the project in Godot once so it discovers the new
   `plugin.cfg` files.
3. Go to **Project — Project Settings — Plugins** and tick the box for
   each addon you copied:
   - **GodotGDK** — installs the `GDKBootstrap` autoload.
   - **GodotPlayFab** — installs the `PlayFabBootstrap` autoload.
   - **Godot GameInput** *(optional)* — installs the
     `GameInputBootstrap` autoload.
   - **GDK Packaging** — registers the **GDK** menu in the editor
     menu bar (game config, sandbox switcher, package manager,
     packaging).
4. Restart the editor when prompted.

If a plugin fails to load, check that the `bin/` directory of that
addon made it into your project — the native DLLs live there.

## 2. Set the PlayFab title id

`godot_playfab` reads the title id from Project Settings. Open
**Project — Project Settings — General**, enable **Advanced Settings**
in the top-right, and set:

| Setting | Value |
|--|--|
| `playfab/runtime/title_id` | your PlayFab title id (e.g. `A1B2C`) |
| `playfab/runtime/initialize_on_startup` | `true` (recommended) — `PlayFabBootstrap` calls `PlayFab.initialize()` for you on `_ready` |

Or edit `project.godot` directly:

```ini
[playfab]

runtime/title_id="A1B2C"
runtime/initialize_on_startup=true
```

The endpoint is derived as `https://<titleid>.playfabapi.com` when
`playfab/runtime/endpoint` is left blank.

While you're in Project Settings, the equivalent GDK toggles are worth
turning on too so the bootstrap brings the GDK runtime up automatically:

```ini
[gdk]

runtime/initialize_on_startup=true
```

> Do **not** set `gdk/runtime/auto_add_primary_user=true` if you plan
> to drive sign-in yourself (next section). The auto-add flag runs the
> silent path on startup and does not fall back to the UI path, so the
> bootstrap and your code can race.

## 3. Create the game config

`MicrosoftGame.config` is required for Xbox-backed sign-in and for
packaging. The `godot_gdk_packaging` addon ships a one-click action:

1. From the editor menu bar, open **GDK — Create MicrosoftGame.config**
   (the item is labeled **Edit MicrosoftGame.config** if a config
   already exists in the project).
2. A template `MicrosoftGame.config` is written to the project root, and
   the Microsoft `GameConfigEditor.exe` opens on it.
3. Fill in at minimum:
   - **Identity / Name** — package family name (e.g. `Studio.Game`;
     no spaces or underscores).
   - **Title Id** — the Xbox title id from
     [Partner Center → your title → Xbox services → Setup](https://partner.microsoft.com/dashboard).
     See [Configuring Xbox services (Title ID + SCID)](https://learn.microsoft.com/en-us/gaming/gdk/docs/services/fundamentals/portal-config/live-service-config-ids-mp).
   - **Store Id (SCID)** — the service configuration id from Partner
     Center, same page as above.
   - **Version** — `1.0.0.0` is a fine starting value.
4. Save and close GameConfigEditor.

The file lives next to `project.godot`. Commit it to source control so
the whole team uses the same identity and Partner Center mapping.

> Without a valid `MicrosoftGame.config`, Xbox Live sign-in returns a
> registration error and packaging fails.

## 4. Change the Xbox sandbox

Test accounts only authenticate against the sandbox they were created
in. To point the PC at a development sandbox:

1. Open **GDK — Change Sandbox…** from the editor menu bar.
2. The dialog shows the current sandbox (e.g. `RETAIL`).
3. Enter your target sandbox id (typically of the form `XDKS.1` or
   `studioname.1`) and click **Set Sandbox**, or click **Switch to
   RETAIL** to go back to consumer.
4. Approve the UAC prompt — sandbox changes need administrator
   privileges and apply machine-wide.

You can also do this from a terminal with
`XblPCSandbox.exe /set XDKS.1 /noApps` if you prefer a script.

## 5. Sign in a user

The recommended flow is **GDK first, PlayFab second**. PlayFab needs an
authenticated Xbox user as input.

Sign-in is **not** driven from `GDK.users.user_changed` — that signal
fires for every user lifecycle event and is not the right hook for
session bootstrap. Instead, run an explicit sign-in routine once on
startup with a check — silent — UI fallback.

### 5a. Xbox sign-in (godot_gdk)

The recommended pattern is:

1. **Check for a primary user.** If one already exists (the bootstrap
   or a prior call already signed someone in), use them.
2. **If not, try silent sign-in.** `add_default_user_async()` picks up
   the user already signed into the Xbox app on the PC without
   surfacing any UI.
3. **If silent fails, fall back to UI.** `add_user_with_ui_async()`
   shows the system account picker so the user can pick or add an
   account interactively.

```gdscript
extends Node

func _ready() -> void:
    var xbox_user: GDKUser = await _ensure_xbox_user()
    if xbox_user == null:
        push_warning("Xbox sign-in failed — playing offline.")
        return

    await _ensure_playfab_user(xbox_user)

func _ensure_xbox_user() -> GDKUser:
    if not Engine.has_singleton("GDK"):
        push_error("godot_gdk extension is not loaded")
        return null

    if not GDK.is_initialized():
        var init: GDKResult = GDK.initialize()
        if not init.ok:
            push_warning("GDK.initialize failed: %s" % init.message)
            return null

    # 1. Already have a primary user? Use it.
    var primary: GDKUser = GDK.users.get_primary_user()
    if primary != null and primary.signed_in:
        return primary

    # 2. Try silent sign-in.
    var silent: GDKResult = await GDK.users.add_default_user_async()
    if silent.ok and silent.data != null and silent.data.signed_in:
        return silent.data

    print("[GDK] Silent sign-in failed (%s) — falling back to UI." % silent.message)

    # 3. Fall back to the system account picker.
    var ui: GDKResult = await GDK.users.add_user_with_ui_async()
    if ui.ok and ui.data != null and ui.data.signed_in:
        return ui.data

    push_warning("[GDK] UI sign-in failed: %s" % ui.message)
    return null
```

`add_default_user_async()` typically returns `no_default_user` when the
PC has no Xbox account signed in — that is the cue to escalate to the
UI path.

### 5b. PlayFab sign-in (godot_playfab)

Once a primary Xbox user exists, hand it to PlayFab. There is no
PlayFab equivalent of "silent vs UI" — `sign_in_with_xuser_async`
authenticates the GDK user directly, no extra prompts.

```gdscript
func _ensure_playfab_user(xbox_user: GDKUser) -> PlayFabUser:
    if not Engine.has_singleton("PlayFab"):
        push_error("godot_playfab extension is not loaded")
        return null

    # PlayFabBootstrap will already have done this when
    # playfab/runtime/initialize_on_startup is true; this is the
    # defensive fallback for projects that disable the auto-init.
    if not PlayFab.is_initialized():
        var init: PlayFabResult = PlayFab.initialize()
        if not init.ok:
            push_warning("PlayFab.initialize failed: %s" % init.message)
            return null

    var result: PlayFabResult = await PlayFab.users.sign_in_with_xuser_async(xbox_user)
    if not result.ok:
        push_warning("PlayFab sign-in failed: %s" % result.message)
        return null

    var pf_user: PlayFabUser = result.data
    var key: Dictionary = pf_user.entity_key
    print("[PlayFab] signed in: %s:%s" % [key.get("type", ""), key.get("id", "")])
    return pf_user
```

`sign_in_with_xuser_async` returns:

- `invalid_xuser` if `xbox_user` is null or signed out — guard with
  `xbox_user != null and xbox_user.signed_in` before calling.
- `title_id_required` if `playfab/runtime/title_id` is empty — set it
  in Project Settings (step 2).

> Pass the `GDKUser` object itself, not the raw user handle.
> `godot_playfab` cannot accept Xbox-side `Ref<>` types directly across
> the addon DLL boundary, so the API takes the `Object *` and reads
> what it needs internally.

## Expected log output

A successful first run prints, in order:

```
[GDK] Bootstrap: GDK.initialize() succeeded.
[GDK] Runtime initialized
[GDK] User added: <gamertag>
[PlayFab] Bootstrap: PlayFab.initialize() succeeded.
[PlayFab] Runtime initialized
[PlayFab] signed in: title_player_account:<entity-id>
```

If you stop earlier than that, the last `push_warning` printed by the
snippets above tells you which step failed.

## Common pitfalls

| Symptom | Likely cause | Fix |
|--|--|--|
| `GDExtension dynamic library not found` | The `bin/` folder didn't make it into the project copy. | Copy `addons/<addon>/` recursively, including `bin/`. |
| `GDK singleton not registered` | Native DLL failed to load (wrong arch, missing GDK install, missing `libHttpClient.dll`). | Install the Microsoft GDK on the machine that runs the game. |
| Silent sign-in returns `no_default_user` | No test account signed into the Xbox app on the PC, or PC sandbox doesn't match Partner Center. | Sign in a test account via the Xbox app after switching to the right sandbox (step 4). The fallback `add_user_with_ui_async()` will surface the picker. |
| Xbox Live calls fail with a registration error | `MicrosoftGame.config` is missing, malformed, or has placeholder Title Id / SCID. | Re-run **GDK — Edit MicrosoftGame.config** and fill in real Partner Center values. |
| `PlayFab.initialize()` fails immediately | `playfab/runtime/title_id` is empty. | Set it in Project Settings (step 2). |
| `sign_in_with_xuser_async` returns `invalid_xuser` | Passing a null or signed-out GDK user. | Confirm `xbox_user != null and xbox_user.signed_in` first. |

## Where to go from here

- [**Tutorials**](tutorials/README.md) — the next step after this
  quickstart. Six task-oriented walkthroughs that build on the
  sign-in flow above: unlocking an achievement, posting and querying
  a leaderboard, saving the player's progress, creating and joining
  a lobby, and bridging a controller through GameInput.
- Full API reference for `GDK.users`, `GDK.achievements`,
  `GDK.leaderboards`, etc. ships in the editor — press **F1** on any
  GDK class name.
- The `godot_gdk_packaging` addon also provides **GDK — Package
  Manager…** (list / install / uninstall registered packages) and
  **Project — Export…** integration for building MSIXVC or loose
  packages.
- For the full source-level guide, see the repo at
  <https://github.com/gaming-microsoft/godot-public-gdk-ext>.
