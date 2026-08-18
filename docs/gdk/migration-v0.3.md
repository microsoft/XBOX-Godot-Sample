# Migrating to v0.3.0 — `GDK*` types are now `Xbox*`

v0.3.0 renames every script-visible type in the `godot_gdk` addon from the `GDK`
prefix to the `Xbox` prefix. **No deprecated `GDK*` aliases are provided**, so any
project that names these types has to be updated.

## Why this rename happened

This rename exists to let the addon **enable console support inside compatible
Godot forks**. Console-capable forks supply the console platform layer and
register their own types, and the `GDK*` prefix collided with names those forks
already use. Godot registers extension classes in a single flat `ClassDB`
namespace, so two providers exposing the same name cannot coexist in one project.

Moving this addon's script-visible types to the `Xbox*` prefix keeps them
distinct, so the addon can be dropped into such a fork and light up console
scenarios there rather than being blocked by a name conflict.

Godot itself is unchanged by this: the engine's own console support comes from
those forks, not from this repository. Within this repo, the rename is a naming
change only — no runtime behavior changed.

**A single binary covers both PC and console.** You do not compile the addon a
second time for console, and there is no console-specific preset or conditional
define. `godot_gdk.gdextension` declares only the `windows.*.x86_64` libraries,
and a console-capable fork loads those same binaries — so the only thing you
need to update for v0.3.0 is your type names.

A codemod is shipped alongside this guide:

```powershell
.\tools\migrate_gdk_to_xbox.ps1 -Path C:\path\to\your\godot\project -WhatIf
.\tools\migrate_gdk_to_xbox.ps1 -Path C:\path\to\your\godot\project
```

## The one thing that is easy to get wrong

The ClassDB class name and the engine singleton name are now **different strings**.

- The singleton is still registered as **`GDK`**. `GDK.initialize()`, `GDK.users`,
  `GDK.game_save` and every other call through the global keep working untouched.
- The *type* behind it is now **`Xbox`**, not `GDK`.

So anything that class-checks the singleton must compare against `"Xbox"`:

```gdscript
# Before
if GDK.get_class() == "GDK":

# After — the singleton is still named GDK, but its class is Xbox
if GDK.get_class() == "Xbox":
```

The same applies to `is` checks and type annotations:

```gdscript
var g: Xbox = GDK          # type is Xbox, global is still GDK
if some_object is Xbox:
```

The codemod deliberately **does not** rewrite a bare `GDK` token, because in almost
every real project it is the singleton (which must not change). It reports the
ambiguous occurrences it finds so you can review them by hand.

> The singleton name remains configurable via the Project Setting added in v0.2.0
> (`gdk/runtime/singleton_name`). Renaming the types did not change that.

## What did *not* change

- The `addons/godot_gdk` folder, `godot_gdk.gdextension`, and the `gdk_addon_init`
  entry symbol.
- The `gdk/runtime/*` and `gdk/packaging/*` Project Settings.
- The default singleton name `"GDK"`, and the `PlayFab` / `GameInput` singletons.
- `PlayFab*` and `GameInput*` types — this rename is scoped to the GDK addon.
- The `gdk` export feature tag and the "XBOX on PC" export platform.
- `GodotGdkCSharp.csproj` / the `GodotGdkCSharp` assembly name.
- `cmake/GDKDependencies.cmake`, `GDK_VERSION`, `gdk_edition.h`, and prose that
  refers to the Microsoft GDK product itself.

## GDScript type mapping

| Before | After |
| --- | --- |
| `GDK` (type only — the global stays `GDK`) | `Xbox` |
| `GDKAccessibility` | `XboxAccessibility` |
| `GDKAchievement` | `XboxAchievement` |
| `GDKAchievements` | `XboxAchievements` |
| `GDKActivation` | `XboxActivation` |
| `GDKCapture` | `XboxCapture` |
| `GDKCaptureMetaData` | `XboxCaptureMetaData` |
| `GDKClosedCaptionProperties` | `XboxClosedCaptionProperties` |
| `GDKDisplay` | `XboxDisplay` |
| `GDKDisplayTimeoutDeferral` | `XboxDisplayTimeoutDeferral` |
| `GDKErrorReporting` | `XboxErrorReporting` |
| `GDKEvents` | `XboxEvents` |
| `GDKGameChat` | `XboxGameChat` |
| `GDKGameSave` | `XboxGameSave` |
| `GDKGameUI` | `XboxGameUI` |
| `GDKLauncher` | `XboxLauncher` |
| `GDKLeaderboard` | `XboxLeaderboard` |
| `GDKLeaderboardColumn` | `XboxLeaderboardColumn` |
| `GDKLeaderboardRow` | `XboxLeaderboardRow` |
| `GDKLeaderboards` | `XboxLeaderboards` |
| `GDKMultiplayerActivity` | `XboxMultiplayerActivity` |
| `GDKMultiplayerActivityInfo` | `XboxMultiplayerActivityInfo` |
| `GDKPackage` | `XboxPackage` |
| `GDKPackageMount` | `XboxPackageMount` |
| `GDKPackageResourcePack` | `XboxPackageResourcePack` |
| `GDKPresence` | `XboxPresence` |
| `GDKPresenceRecord` | `XboxPresenceRecord` |
| `GDKPrivacy` | `XboxPrivacy` |
| `GDKProfile` | `XboxProfile` |
| `GDKResult` | `XboxResult` |
| `GDKSocial` | `XboxSocial` |
| `GDKSocialFilter` | `XboxSocialFilter` |
| `GDKSocialGroup` | `XboxSocialGroup` |
| `GDKSocialUser` | `XboxSocialUser` |
| `GDKSpeechSynthesizer` | `XboxSpeechSynthesizer` |
| `GDKStats` | `XboxStats` |
| `GDKStore` | `XboxStore` |
| `GDKStoreLicenseStatus` | `XboxStoreLicenseStatus` |
| `GDKStringVerify` | `XboxStringVerify` |
| `GDKSystem` | `XboxSystem` |
| `GDKTitleStorage` | `XboxTitleStorage` |
| `GDKTitleStorageBlobMetadata` | `XboxTitleStorageBlobMetadata` |
| `GDKTitleStorageBlobMetadataResult` | `XboxTitleStorageBlobMetadataResult` |
| `GDKUser` | `XboxUser` |
| `GDKUserProfile` | `XboxUserProfile` |
| `GDKUserSignOutDeferral` | `XboxUserSignOutDeferral` |
| `GDKUsers` | `XboxUsers` |

Before:

```gdscript
var init: GDKResult = GDK.initialize()
if not init.success:
    push_error(init.message)
    return

var res: GDKResult = await GDK.users.add_default_user_async()
var user: GDKUser = res.data
print(user.gamertag)

GDK.runtime_error.connect(func(r: GDKResult) -> void: push_error(r.message))
```

After:

```gdscript
var init: XboxResult = GDK.initialize()
if not init.success:
    push_error(init.message)
    return

var res: XboxResult = await GDK.users.add_default_user_async()
var user: XboxUser = res.data
print(user.gamertag)

GDK.runtime_error.connect(func(r: XboxResult) -> void: push_error(r.message))
```

## C# mapping

The C# facade moves with the GDScript types — every `Gdk`-prefixed identifier
becomes `Xbox`-prefixed, including the static entry point.

| Before | After |
| --- | --- |
| namespace `GodotGdk` | namespace `GodotXbox` |
| static class `Gdk` | static class `Xbox` |
| `GdkResult`, `GdkUser`, `GdkObject`, `GdkServiceBase`, … | `XboxResult`, `XboxUser`, `XboxObject`, `XboxServiceBase`, … |
| `Gdk*.cs` source files | `Xbox*.cs` |

Unlike GDScript, the bare C# identifier `Gdk` **is** renamed to `Xbox` — it is a
static class, not the engine singleton name, so there is no ambiguity.

Before:

```csharp
using GodotGdk;

GdkResult init = Gdk.Initialize();
GdkResult res = await Gdk.Users.AddDefaultUserAsync();
GdkUser user = res.DataAs<GdkUser>();
GD.Print(user.Gamertag);
```

After:

```csharp
using GodotXbox;

XboxResult init = Xbox.Initialize();
XboxResult res = await Xbox.Users.AddDefaultUserAsync();
XboxUser user = res.DataAs<XboxUser>();
GD.Print(user.Gamertag);
```

`GodotGdkCSharp.csproj` and the `GodotGdkCSharp` assembly name are unchanged, so
existing `<ProjectReference>` entries keep resolving.

## Autoloads

The bootstrap autoload is renamed. In `project.godot`:

```ini
# Before
GDKBootstrap="*res://addons/godot_gdk/runtime/gdk_bootstrap.gd"

# After
XboxBootstrap="*res://addons/godot_gdk/runtime/gdk_bootstrap.gd"
```

The C# tutorial tracks additionally rename the autoload script itself from
`res://Autoload/GdkBootstrap.cs` to `res://Autoload/XboxBootstrap.cs`.

If your own code refers to the autoload by name (`GDKBootstrap.something`), update
those references too. The codemod handles the `project.godot` entry and plain
identifier references, but it will not rename your script *files* unless you pass
`-RenameFiles`.

## Packaging forwarder environment variables

Only relevant if you drive the MSIXVC packaging forwarder directly:

| Before | After |
| --- | --- |
| `GDKPKG_GODOT_EXE` | `XBOXPKG_GODOT_EXE` |
| `GDKPKG_PROJECT_PATH` | `XBOXPKG_PROJECT_PATH` |
| `GDKPKG_ARG_COUNT` | `XBOXPKG_ARG_COUNT` |
| `GDKPKG_ARG_<n>` | `XBOXPKG_ARG_<n>` |

## Suggested migration order

1. Back up / commit your project first — the codemod rewrites files in place.
2. Drop in the v0.3.0 addons.
3. Run `.\tools\migrate_gdk_to_xbox.ps1 -Path <project> -WhatIf` and read the report.
4. Run it for real, then review the "ambiguous bare `GDK`" list it prints.
5. Open the project in the editor. Scene and resource files reference script classes
   by name, so a missed rename shows up as a **load** error rather than a parse
   error — check the editor output, not just the script editor.
6. For C# projects, `dotnet build` will surface anything the codemod missed.
