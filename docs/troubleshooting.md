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

## GDK not found during CMake configure

**Symptom:**

```
CMake Error: Microsoft GDK not found.
Install via: winget install Microsoft.Gaming.GDK
```

**Cause:** The GDK is not installed, or CMake cannot find it.

**Fixes:**

1. Install the GDK:
   ```powershell
   winget install Microsoft.Gaming.GDK
   ```
2. If installed but not detected, set the path manually:
   ```powershell
   cmake --preset default -DGDK_WINDOWS="C:/Program Files (x86)/Microsoft GDK/<edition>/windows"
   ```
3. Check the `GameDKCoreLatest` or `GameDKLatest` environment variables point
   to a valid GDK installation.

## XSAPI or libHttpClient headers not found

**Symptom:**

```
CMake Error: Xbox Services API (XSAPI) headers not found
CMake Error: libHttpClient headers not found
```

**Cause:** The GDK installation does not include Xbox Extensions headers.

**Fix:** Ensure you've installed the full GDK (not a partial installation).
Re-install with:

```powershell
winget install Microsoft.Gaming.GDK
```

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

See [Sample Project Setup](godot-gdk-sample-setup.md) for the full
configuration guide.
