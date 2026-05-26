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

**Cause:** `sample/gdk_demo/launch_editor.bat` expects the Godot executable to be
placed in the `sample/gdk_demo/` directory with that exact filename.

**Fix:** Download Godot 4.6.1 stable from
[godotengine.org](https://godotengine.org/download) and place the executable
in the `sample/gdk_demo/` directory. The `.exe` filename must match what's in
`launch_editor.bat`.

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
2. **Using personal account** — The sample requires Xbox test accounts, not
   personal Microsoft accounts.
3. **Title not configured** — Ensure your title is set up in Partner Center
   with Xbox Live enabled.

See [Sample Project Setup](gdk/sample-setup.md) for the full
configuration guide.

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
