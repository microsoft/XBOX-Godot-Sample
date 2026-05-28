# Troubleshooting

Common build and runtime issues for the GodotGDK addons.

## DLL load failure: Error 126

**Symptom:**

```
ERROR: Can't open dynamic library: .../godot_gdk.windows.debug.x86_64.dll.
Error: Error 126: The specified module could not be found.
```

**Cause:** Error 126 means a **transitive dependency** of the DLL cannot be
found — not the DLL itself.

The Debug build of `godot_gdk` links against
`Microsoft.Xbox.Services.C.Thunks.Debug.dll`, which depends on **debug C
runtime** DLLs (`MSVCP140D.dll`, `VCRUNTIME140D.dll`, `ucrtbased.dll`). These
debug CRT DLLs are only available when Visual Studio is installed.

**Dependency chain:**

```
godot_gdk.windows.debug.x86_64.dll
  → Microsoft.Xbox.Services.C.Thunks.Debug.dll     (in bin/)
      → MSVCP140D.dll, VCRUNTIME140D.dll            (needs Visual Studio)
      → libHttpClient.dll                            (in bin/)
```

**Fixes:**

- **Install Visual Studio** (any edition — Community is free) on the machine
  running the sample. The Godot editor always loads the `debug` variant of the
  extension.
- **Use a Release build** if Visual Studio cannot be installed. Release builds
  use redistributable CRT DLLs (`MSVCP140.dll`) that are widely available:
  ```powershell
  cmake --build build --config Release
  ```

## GDK headers or import libs not found during CMake configure

**Symptom:**

```
CMake Error: Could not find a package configuration file provided by "ms-gdk"
CMake Error: Could not find a package configuration file provided by "gameinput"
```

…or any error mentioning XSAPI / libHttpClient / GameInput headers being
unresolvable during CMake configure.

**Cause:** vcpkg manifest mode could not resolve the `ms-gdk[playfab]` or
`gameinput` ports defined in the repo's `vcpkg.json`.

**Fixes:**

1. Ensure `VCPKG_ROOT` is set to a valid vcpkg clone:
   ```powershell
   $env:VCPKG_ROOT = "C:\path\to\vcpkg"
   ```
   The default CMake preset reads this when it injects the vcpkg toolchain
   file. If you prefer to point at a specific toolchain, override on the
   command line:
   ```powershell
   cmake --preset default -DCMAKE_TOOLCHAIN_FILE="C:/path/to/vcpkg/scripts/buildsystems/vcpkg.cmake"
   ```
2. From a fresh shell at the repo root, re-run the configure step so vcpkg
   restores the manifest packages into `build\vcpkg_installed\`:
   ```powershell
   cmake --preset default
   ```
   The first run downloads the GDK and GameInput NuGet packages from
   Microsoft's public feed and can take several minutes; subsequent runs
   reuse the cached install.
3. If the configure still fails, delete `build\vcpkg_installed\` and try
   again — partial manifest installs from an interrupted previous run can
   leave the cache in an inconsistent state.

> **Note:** vcpkg covers everything you need to **build** the addons. You
> still need a full Microsoft GDK install on machines that need to **run**
> packaging tools (`makepkg.exe`, `wdapp.exe`, Game Config Editor).

## GDK packaging tools not found at runtime

**Symptom:** The packaging plugin reports that `makepkg.exe`, `wdapp.exe`,
or `GameConfigEditor.exe` cannot be located.

**Cause:** The vcpkg-based build does not install the GDK tools (only the
headers and import libs). End users need the full GDK install for the
tooling surface.

**Fix:** Install the Microsoft GDK on the machine that runs the packaging
plugin:

```powershell
winget install Microsoft.Gaming.GDK
```

You can also point `GDK_BIN` at a non-default tools directory — see
[Packaging plugin](packaging/plugin.md) for the full precedence list.

## Godot editor cannot find the executable

**Symptom:**

```
Windows cannot find 'Godot_v4.6.1-stable_win64.exe'.
```

**Cause:** Older sample `launch_editor.bat` scripts (now removed)
expected the Godot executable to be placed in the sample directory
with that exact filename. The sample-driven tutorial revamp in
progress replaces those launchers with the standard Godot editor
workflow (open `project.godot` directly).

**Fix:** Open the project the standard way — launch your Godot 4.x
editor and use **Project → Open** to pick `project.godot` in
whichever Godot project you are working in. No bundled launcher
script is required.

## Visual Studio version mismatch

**Symptom:**

```
CMake Error: Generator "Visual Studio 17 2022" could not find any instance of Visual Studio.
```

**Cause:** The repository CMake presets target Visual Studio 2022. This
error usually means Visual Studio 2022 is not installed, or only a different
Visual Studio version is installed.

**Fix:** Either install Visual Studio 2022 to match the presets, or configure
manually with the generator for the Visual Studio version you have installed:

```powershell
# Replace the generator string with your installed Visual Studio version,
# for example: "Visual Studio 17 2022" or "Visual Studio 18 2026"
cmake -G "<your installed Visual Studio generator>" -A x64 -B build -DCMAKE_CXX_STANDARD=17 -DBUILD_GODOT_GDK=ON -DBUILD_GODOT_GAMEINPUT=ON
```

## godot-cpp submodule not found

**Symptom:**

```
CMake Error: godot-cpp submodule not found. Run: git submodule update --init --recursive
```

**Fix:**

```powershell
git submodule update --init --recursive
```

## Xbox sign-in fails at runtime

**Possible causes:**

1. **Wrong sandbox** — Your PC must be in the same sandbox as your test account:
   ```powershell
   & "C:\Program Files (x86)\Microsoft GDK\bin\XblPCSandbox.exe"
   ```
   See [Microsoft GDK — Setting up sandboxes](https://learn.microsoft.com/en-us/gaming/gdk/docs/services/fundamentals/sandboxes/live-setup-sandbox)
   and
   [PC Sandbox Switcher (XblPCSandbox.exe)](https://learn.microsoft.com/en-us/gaming/gdk/docs/tools/tools-services/live-pc-sandbox-switcher).
2. **Using personal account** — The sample requires Xbox test accounts, not
   personal Microsoft accounts.
3. **Title not configured** — Ensure your title is set up in
   [Partner Center](https://partner.microsoft.com/dashboard) with Xbox Live
   enabled. See
   [Microsoft GDK — Configuring Xbox services (Title ID + SCID)](https://learn.microsoft.com/en-us/gaming/gdk/docs/services/fundamentals/portal-config/live-service-config-ids-mp)
   for the canonical Microsoft-side walkthrough.

See [Sample Project Setup](gdk/sample-setup.md) for the full
configuration guide.

## SCID does not match between `MicrosoftGame.config` and Partner Center

**Symptom:**

```
[GDK] add_default_user_async failed: code=auth_invalid_scid, message=...
```

Or sign-in succeeds but every Xbox services call (achievements,
leaderboards, MPA, presence) fails with `404` / `not_found` / a similar
"unknown SCID" diagnostic.

**Cause:** The `<ExtendedAttributeList>` `Scid` value inside
`MicrosoftGame.config` does not match the SCID assigned to your title in
Partner Center, or matches an SCID from a different sandbox.

**Fix:**

1. In [Partner Center](https://partner.microsoft.com/dashboard) → your title
   → **Xbox services → Service configuration → IDs**, copy the
   **Service Configuration ID** value. See
   [Microsoft GDK — Configuring Xbox services (Title ID + SCID)](https://learn.microsoft.com/en-us/gaming/gdk/docs/services/fundamentals/portal-config/live-service-config-ids-mp)
   for where this lives.
2. Open your project's `MicrosoftGame.config` and confirm the `Scid`
   attribute under
   `ExtendedAttributeList/ExtendedAttribute Name="Xbox.Services.Configuration"`
   matches **exactly** (uppercase, no surrounding whitespace).
3. Repackage and reinstall (`makepkg pack` → `wdapp register`); the
   running game keeps the SCID it was packaged with.

If the SCID is right but sign-in still fails with an SCID-shaped
error, the game's **sandbox** does not match the SCID's sandbox. See
the next entry.

## Sandbox mismatch between PC and test account

**Symptom:**

- Sign-in succeeds in **RETAIL** but returns no friend list / no
  achievements / wrong gamerscore.
- Sign-in fails immediately with `auth_no_account` when the account
  works fine on a different machine.
- The Game Bar identity badge shows a different gamertag than the one
  Xbox services returns.

**Cause:** The PC's active sandbox does not match the sandbox the test
account is provisioned in (or the SCID is published into).

**Fix:**

1. Switch the PC's sandbox to the one your test account lives in:
   ```powershell
   & "C:\Program Files (x86)\Microsoft GDK\bin\XblPCSandbox.exe" YOURSANDBOX.0
   ```
   See [Microsoft GDK — Setting up sandboxes](https://learn.microsoft.com/en-us/gaming/gdk/docs/services/fundamentals/sandboxes/live-setup-sandbox)
   for the underlying model and
   [PC Sandbox Switcher (XblPCSandbox.exe)](https://learn.microsoft.com/en-us/gaming/gdk/docs/tools/tools-services/live-pc-sandbox-switcher)
   for the tool reference.
2. Sign out of the Microsoft Store / Xbox app, then sign back in with
   the test account.
3. Re-run the game.

Use `RETAIL` only for live builds. While developing, stay on a
non-retail sandbox tied to your title's Partner Center configuration.

## `MicrosoftGame.config` schema errors during `makepkg pack`

**Symptom:**

```
MakePkg : error 0xc00ce169 : 'GodotGDK Sample' violates pattern constraint of '[^_ ]+'
MakePkg : error: <Identity Name="..."> contains an illegal character
```

…or any `makepkg.exe` error citing `MicrosoftGame.config` and an
attribute name (`Identity/@Name`, `Identity/@Publisher`, etc.).

**Cause:** `MicrosoftGame.config` is validated against a strict XML
schema. Two common slip-ups:

- `Identity/@Name` and `Identity/@Publisher` reject spaces and
  underscores. `DisplayName` and `PublisherDisplayName` allow them.
- `Identity/@Publisher` must start with `CN=` (e.g. `CN=Contoso`).

**Fix:**

- Edit `MicrosoftGame.config` so `Identity/@Name` is alphanumeric +
  hyphens only (`GodotGDK-Sample`, not `GodotGDK Sample`).
- Keep `Identity/@Publisher` as `CN=<your publisher name>`.
- If you generated the config from `GDKPackagingConfig`, re-run the
  generator after fixing the source name; the generator sanitizes the
  Identity name automatically.

See [Packaging plugin](packaging/plugin.md) for the full config
schema.

## `PlayFab.initialize()` fails with `title_id_required`

**Symptom:**

```
[PlayFab] initialize failed: code=title_id_required, message=Set playfab/runtime/title_id in Project Settings.
```

**Cause:** The PlayFab runtime needs a title id at initialize time.
The `playfab/runtime/title_id` project setting is empty or missing.

**Fix:**

1. Open **Project → Project Settings → PlayFab → Runtime** (with the
   `GodotPlayFab` plugin enabled, the section appears automatically).
2. Set **Title Id** to your PlayFab title id (a 4–6 character
   hexadecimal string from [PlayFab Game Manager](https://developer.playfab.com/) →
   your title → **Settings → API features**). If you don't have a title
   yet, see
   [PlayFab — Game Manager quickstart](https://learn.microsoft.com/en-us/gaming/playfab/gamemanager/quickstart)
   for the create-account-and-title walkthrough.
3. Save the project.

If you initialize PlayFab from script before the project settings have
been loaded (e.g. from a `_init` rather than `_ready`), `title_id`
reads as empty even when it is set in the project file. Initialize
PlayFab from an autoload's `_ready` instead — the
`GodotPlayFab` editor plugin's `PlayFabBootstrap` autoload does this
for you when you flip `playfab/runtime/initialize_on_startup` to
`true`.

## `PlayFab.users.sign_in_with_xuser_async` fails with `invalid_xuser` or `xuser_not_found`

**Symptom:**

```
[PlayFab] sign-in failed: code=invalid_xuser, message=The provided GDK user is not signed in.
```

…or:

```
[PlayFab] sign-in failed: code=xuser_not_found, message=Failed to find an active XUserHandle for the provided GDK user.
```

**Cause:** `PlayFab.users.sign_in_with_xuser_async` takes a typed
`GDKUser` object and reads its `local_id` to find the matching
`XUserHandle`. Two failure paths show up most often:

- `invalid_xuser` — the GDK user you passed in is `null`, has not
  finished sign-in (`signed_in == false`), or does not expose a
  non-zero `local_id`. Most common cause: handing `PlayFab` a stale
  `Auth.xbox_user` from before sign-in completed.
- `xuser_not_found` — the `local_id` was valid at one point but the
  underlying `XUserHandle` has been freed or signed out. Most common
  cause: signing out of Xbox between `add_default_user_async` and
  `sign_in_with_xuser_async`, or holding a stored user across a
  long-running test session.

**Fix:**

1. Confirm Xbox sign-in completed and the user is still live before
   passing it to PlayFab:
   ```gdscript
   var xbox_result := await GDK.users.add_default_user_async()
   assert(xbox_result.ok, "Xbox sign-in failed before PlayFab")
   ```
2. Pass the `xbox_result.data` (a `GDKUser`) into PlayFab sign-in:
   ```gdscript
   var pf_result := await PlayFab.users.sign_in_with_xuser_async(xbox_result.data)
   ```
   Do not pass an `xuid` or a raw local id directly — `sign_in_with_xuser_async`
   takes the typed `GDKUser` object.
3. If the Xbox sign-in succeeded but PlayFab still fails with
   `xuser_not_found`, re-run `GDK.users.add_default_user_async()` to
   obtain a fresh `GDKUser` and immediately retry — the previous
   handle was released.
4. If the failure is consistent across fresh sign-ins, your PlayFab
   title may not have Xbox Live linked. In
   [PlayFab Game Manager](https://developer.playfab.com/) →
   your title → **Add-ons → Xbox Live**, install the add-on and
   configure it with your title's SCID.

## PlayFab leaderboard submit fails with `E_PF_API_NOT_ENABLED_FOR_GAME_CLIENT_ACCESS` (0x89235472)

**Symptom:** `PlayFab.leaderboards.submit_score_async` returns a result
with `ok == false` and a message like:

```
[Lead] Submit failed: Failed to update the PlayFab leaderboard entry. (HRESULT 0x89235472)
```

The HRESULT `0x89235472` decodes (via `<playfab/core/PFErrors.h>`) to
`E_PF_API_NOT_ENABLED_FOR_GAME_CLIENT_ACCESS`. The verbatim service
error body is:

```json
{
  "code": 400,
  "status": "BadRequest",
  "error": "APINotEnabledForGameClientAccess",
  "errorCode": 1082,
  "errorMessage": "This API must be enabled for client access in the Game Manager API Features settings"
}
```

**Cause:** PlayFab's `LeaderboardsV2/UpdateLeaderboardEntries` endpoint
(the one the addon's `submit_score_async` calls via
`PFLeaderboardsUpdateLeaderboardEntriesAsync`) is, by default,
**server-only**, and the current PlayFab Game Manager UI does not
expose a per-leaderboard or per-title toggle to grant client write
access to it. The block lives at the LeaderboardsV2 service layer and
is not relaxed by anything the public PlayFab admin REST API exposes:

- It is **not** controlled by `ApiPolicy` / `Admin/UpdatePolicy`. The
  default `ApiPolicy` already has an `Allow * *` statement on
  `pfrn:api--/Leaderboard/*`, and adding the analogous statement on
  `pfrn:api--/LeaderboardsV2/*` does **not** unblock the call — the
  service still returns errorCode 1082.
- It is **not** an entry in `Admin/GetTitleData` /
  `Admin/GetTitleInternalData` either.
- `Admin/GetPolicy` only accepts `PolicyName: "ApiPolicy"` — no other
  policy name exists.
- No `Admin/GetAPIFeatures` / `Admin/GetTitleAPIFeatures` /
  `Admin/GetClientAPISettings` endpoint exists (all return 404).

**Fix:** switch the client write path from
`PlayFab.leaderboards.submit_score_async` to
`PlayFab.statistics.update_statistics_async`, configure the
leaderboard in Game Manager to source its rankings from a statistic,
and enable the title's **Allow client to post player stats** setting
so the statistic write itself reaches the service. The leaderboard
ranks statistic values, so the read paths
(`get_leaderboard_async`, `get_leaderboard_around_user_async`,
`get_friend_leaderboard_async`) stay unchanged.

1. Open [PlayFab Game Manager](https://developer.playfab.com/) and
   select your title.
2. Navigate to **Statistics → New statistic**. Create a statistic with
   entity type `title_player_account` and one column (a name like
   `"high_score"` matches the T3 sample).
3. Navigate to **LeaderboardsV2** → create or edit the target
   leaderboard and configure its source to be the statistic created
   in step 2. The leaderboard and statistic may share the same name.
4. Navigate to **Title settings → API Features** and enable
   **Allow client to post player stats**. This title-wide setting
   gates both the legacy `Client/UpdatePlayerStatistics` endpoint and
   the V2 `Statistic/UpdateStatistics` endpoint that
   `update_statistics_async` calls. The same `errorCode 1082`
   returned by `submit_score_async` is returned by
   `update_statistics_async` when this setting is disabled.
5. In code, replace any call shaped like
   `PlayFab.leaderboards.submit_score_async(user, name, score)` with:

   ```gdscript
   await PlayFab.statistics.update_statistics_async(user, {
       "statistics": [
           {"name": STATISTIC_NAME, "scores": [str(score)]},
       ],
   })
   ```

   `scores` is a `PackedStringArray` of decimal-encoded values, one
   per statistic column. A single-column statistic takes a
   single-element array.

After the leaderboard is sourced from the statistic and the write
path is `update_statistics_async`, the T3 sample succeeds and
subsequent `get_leaderboard_async` calls return the recorded value
once statistic-to-leaderboard propagation completes (typically a few
seconds). The
[Tutorial 3 walkthrough](tutorials/03-playfab-leaderboard.md) and
[PlayFab title prerequisites — §2 Leaderboards](playfab/prerequisites.md#leaderboards-t3-t8)
cover the full pattern.

> **Security note.** The statistic-backed pattern lets any signed-in
> client write any value to the statistic. Fine for tutorial /
> sandbox titles; for a production title that requires validated
> writes, keep client writes off and call
> `LeaderboardsV2/UpdateLeaderboardEntries` or
> `Statistics/UpdateStatistics` from CloudScript / Azure Functions /
> your own trusted backend with the developer secret key, and
> validate scores before writing.

## Tests

### Orchestrator says all green but my new test wasn't discovered

Check that the host run includes `-ginclude_subdirs`. GUT v9.6.0's `-gdir` is non-recursive, so `-gdir=res://tests` only selects the directory tree when `-ginclude_subdirs` is also present. Also confirm the directory you pass to `-gdir` is the one that contains your test files.

### GUT exits 0 but says `Nothing was run.`

This is a discovery filter mismatch, usually the wrong `-gdir`, a missing `-ginclude_subdirs`, or files that do not match GUT's default `test_*.gd` pattern. `tools\run_all_tests.ps1` detects this by parsing GUT's summary and fails the host, but manual invocations may look green if you check only the exit code.

### Pre-commit hook fails with UID warnings on test files

Drop an empty `.gut_skip_validation` sentinel at the tests root that contains GUT-extending files. The headless validator treats a directory containing that sentinel, and its descendants, as not standalone-parseable.

### C++ doctest crashes inside `godot::String`

This is expected in a free-standing executable. `godot::String` and other Variant-family types require the GDExtension function table that Godot initializes when it loads an addon. Move that case into a GUT test, or extract a pure helper that does not instantiate Godot Variant-family types.

### Leaderboard test marked pending after submit

PlayFab leaderboard writes are eventually consistent. If your sandbox is slow, set the test-host-only `playfab/tests/leaderboard_settle_msec` key in `tests\godot\playfab\project.godot` so the test polls longer before marking the read-after-write check pending.

### Bootstrap runner exit code is 0 but I never saw `BOOTSTRAP_OK:`

Check that the script prints the literal success prefix before it exits and ends with `quit(0)`. `tools\run_all_tests.ps1` gates the bootstrap stage on process exit code, while the `BOOTSTRAP_OK:` and `BOOTSTRAP_FAIL:` prefixes are the log contract reviewers and manual runs use to understand what happened.
