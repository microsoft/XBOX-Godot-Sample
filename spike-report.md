# Wave -1 De-Risking Spike Report

Branch: `tests/infra-spike` (worktree: `.copilot-worktrees/infra-spike/`)
Host: Windows + PowerShell 7, MSVC 19.44, Godot 4.6.2-stable (`Godot_v4.6.1-stable_win64_console.exe` discovered via `GODOT_CONSOLE` env var).

All five items below are verified end-to-end with the exact commands and (trimmed) outputs below. Any "Wave 1 should reconsider" notes are at the bottom under **Wave 1 inputs**.

---

## 1. GUT vendored + mirrored (experimental)

**Pinned upstream:** GUT `v9.6.0` (Godot 4 line).

* Tarball: <https://github.com/bitwes/Gut/archive/refs/tags/v9.6.0.tar.gz>
* SHA-256: `3C6C87DDE4C79E8EF109A1C9419B6E654971BA11E7F5AC1731E927EAA0FD1159`
* Vendored at: `addons\godot_gdk\tests_support\gut\`
* `LICENSE.md` (MIT, Tom "Butch" Wesley) is preserved in-place from the upstream `addons/gut/` tree.
* `addons\godot_gdk\tests_support\gut\VERSION.txt` records tag, URL, SHA, date.

**CMake function added:** `godot_addon_mirror_test_support` in `cmake\GodotExtensionCommon.cmake` (configure-time `file(COPY)`, marked EXPERIMENTAL — Wave 1 will replace with a build-time tracked-output target).

**Wired in root** `CMakeLists.txt`:

```cmake
godot_addon_mirror_test_support(
    SOURCE_DIR  "${CMAKE_SOURCE_DIR}/addons/godot_gdk/tests_support/gut"
    DEST_SUBDIR "gut"
    SAMPLE_DIRS "${CMAKE_SOURCE_DIR}/sample/gdk_demo"
)
```

`sample\multiplayer_pong\` is intentionally **not** in `SAMPLE_DIRS`.

**Verification:**

```powershell
PS> cmake --preset default
...
-- Mirrored test-support 'gut' -> R:/.../sample/gdk_demo/addons/gut
-- Configuring done (8.4s)
-- Generating done (0.4s)

PS> Test-Path "sample\gdk_demo\addons\gut\gut_cmdln.gd"
True
PS> Test-Path "sample\multiplayer_pong\addons\gut"
False
PS> Test-Path "sample\gdk_launch_point\addons\gut"
False
PS> Test-Path "sample\playfab_demo\addons\gut"
False
```

Mirrored copies are git-ignored via `.gitignore` entry `sample/*/addons/gut/`.

---

## 2. Verified GUT CLI flags (exit-code propagation)

Test file: `sample\gdk_demo\tests\spike\test_spike.gd` (extends `GutTest`, two assertions).

GUT will **not** run until `--headless --import` has been executed once in the project (the `class_name` registry must be populated). After that:

**Canonical passing command** (run from `sample\gdk_demo\`):

```powershell
& $godot --headless -s res://addons/gut/gut_cmdln.gd -gdir=res://tests/spike -gexit
```

Output (trimmed):

```
res://tests/spike/test_spike.gd
* test_arithmetic_passes
* test_live_tests_env_var_present
2/2 passed.
Tests                 2
Passing Tests         2
Asserts               2
---- All tests passed! ----
EXIT=0
```

The explicit-flag form `... -gdir=res://tests/spike "-gprefix=test_" "-gsuffix=.gd" -gexit` is also accepted, but **PowerShell quoting matters**: unquoted `-gsuffix=.gd` was misparsed as `[".gd"]` by Godot's CLI parser (`Unknown arguments: [".gd"]`, exit 1). Defaults (`-gprefix=test_`, `-gsuffix=.gd`) work without specifying them, which is what the canonical command relies on.

`-gexit_on_success` was not needed — `-gexit` exits with the correct status (0 on pass, non-zero on fail) and does not leave the process open.

**Failing case** — flipping the assertion to `assert_eq(1, 2)`:

```powershell
& $godot --headless -s res://addons/gut/gut_cmdln.gd -gdir=res://tests/spike -gexit
```

Output (trimmed):

```
* test_arithmetic_passes
    [Failed]:  [1] expected to equal [2]: ...
1/2 passed.
Passing Tests         1
Failing Tests         1
Asserts             1/2
---- 1 failing tests ----
EXIT=1
```

Restored to passing before commit.

---

## 3. Env propagation without `--env-file`

Same project, same `test_spike.gd`. The second test asserts `OS.get_environment("LIVE_TESTS") == "1"`.

Stub script: `tests\cpp\spike\run_env_check.ps1`. It uses
`[System.Diagnostics.ProcessStartInfo]::new($godotExe)` and sets
`$psi.EnvironmentVariables["LIVE_TESTS"] = "1"` (no `$env:` mutation in the
parent shell, no `--env-file` flag). It also reads stdout/stderr async — a
sync `ReadToEnd()` deadlocked on a full stderr pipe (Godot prints many
GDExtension-not-found ERROR lines in a clean tree; this is worth
remembering for Wave 1's orchestrator).

**Run** (with the env var set):

```
==== with LIVE_TESTS=1 (LIVE_TESTS=True) ====
* test_arithmetic_passes
* test_live_tests_env_var_present
2/2 passed.
Tests                 2
Passing Tests         2
Asserts               2
---- All tests passed! ----
EXIT=0
```

**Run** (without the env var):

```
==== without LIVE_TESTS (LIVE_TESTS=False) ====
* test_arithmetic_passes
* test_live_tests_env_var_present
    [Failed]:  [""] expected to equal ["1"]:  expected LIVE_TESTS=1 in env (set by ProcessStartInfo)
1/2 passed.
Passing Tests         1
Failing Tests         1
Asserts             1/2
---- 1 failing tests ----
EXIT=1
```

Final summary line from the wrapper:

```
  with LIVE_TESTS=1   : exit=0 (expect 0)
  without LIVE_TESTS  : exit=1 (expect non-zero)
FINAL_EXIT=0
```

`ProcessStartInfo.EnvironmentVariables` propagation is verified end-to-end.

---

## 4. Packaging editor-vs-headless decision (--editor vs --headless)

Test file: `sample\gdk_demo\tests\packaging_spike\test_packaging_spike.gd`.

It preloads `addons/godot_gdk_packaging/editor/gdk_toolchain.gd` (a `@tool` `RefCounted`) and exercises the pure helpers `get_gdk_version()` (parses the `GameDKCoreLatest` env var) and `get_bin_dir()`.

**Headless run** (from `sample\gdk_demo\`):

```powershell
& $godot --headless -s res://addons/gut/gut_cmdln.gd -gdir=res://tests/packaging_spike -gexit
```

Output (trimmed):

```
res://tests/packaging_spike/test_packaging_spike.gd
* test_get_gdk_version_returns_string
* test_get_bin_dir_is_string
Tests                 2
Passing Tests         2
Asserts               4
---- All tests passed! ----
HEADLESS_EXIT=0
```

**Editor run**:

```powershell
& $godot --editor --headless --quit-after 200 -s res://addons/gut/gut_cmdln.gd -gdir=res://tests/packaging_spike -gexit
```

Output (trimmed — note the absence of any GUT `Tests`/`Asserts`/`passed` lines):

```
[GDK Packaging] Editor plugin loaded
... (GDExtension load errors, expected for clean tree) ...
[GDK Packaging] Editor plugin unloaded
ERROR: 10 RID allocations of type 'N16RendererViewport8ViewportE' were leaked at exit.
ERROR: 654 RID allocations of type '...ShapedTextDataAdvanced...' were leaked at exit.
WARNING: ObjectDB instances leaked at exit (run with --verbose for details).
EDITOR_EXIT=0
```

Without `--quit-after` the editor stays open indefinitely; with it the editor quits before GUT's `-s` script ever runs. Net: **`--editor` does not execute `-s gut_cmdln.gd`** — the editor takes over the main loop and ignores the `-s` script in this Godot version.

**Decision (Wave 4 input):**
**Run all packaging tests in `--headless` mode in the same orchestrator stage as everything else.** `gdk_toolchain.gd` and other `@tool` scripts that don't touch `EditorInterface`/`EditorPlugin` instances load and execute fine under `--headless`. If/when a packaging test ever needs a real `EditorPlugin` instance or `EditorInterface`, it should be tagged and skipped (or driven via a different runner) rather than triggering a separate `--editor` orchestrator stage — `--editor` simply will not cooperate with `gut_cmdln.gd` here.

---

## 5. Trivial doctest exe builds and runs

Pinned `doctest` `v2.5.2`.

* Header: `tests\cpp\third_party\doctest\doctest.h` (single file)
* License: `tests\cpp\third_party\doctest\LICENSE.txt`
* `tests\cpp\third_party\doctest\VERSION.txt` records tag, URLs, date.

CMake:

* `tests\cpp\CMakeLists.txt` defines an `add_executable(gdk_unit_tests spike_main.cpp)`.
* Root `CMakeLists.txt` adds `option(GDK_BUILD_TESTS "..." ON)` and `add_subdirectory(tests/cpp)` guarded by it.
* TU: `tests\cpp\spike_main.cpp` defines `DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN` and contains `TEST_CASE("spike: arithmetic") { CHECK(1 + 1 == 2); }`.

**Build + run (passing):**

```powershell
PS> cmake --build build --preset debug --target gdk_unit_tests
  spike_main.cpp
  gdk_unit_tests.vcxproj -> ...\build\bin\Debug\gdk_unit_tests.exe
PS> & .\build\bin\Debug\gdk_unit_tests.exe
[doctest] doctest version is "2.5.2"
[doctest] test cases: 1 | 1 passed | 0 failed | 0 skipped
[doctest] assertions: 1 | 1 passed | 0 failed |
[doctest] Status: SUCCESS!
EXIT=0
```

**Build + run (intentional failure, then restored):**

```powershell
# After adding `CHECK(1 == 2);` to the TEST_CASE
PS> & .\build\bin\Debug\gdk_unit_tests.exe
[doctest] doctest version is "2.5.2"
spike_main.cpp(8): ERROR: CHECK( 1 == 2 ) is NOT correct!
[doctest] test cases: 1 | 0 passed | 1 failed | 0 skipped
[doctest] assertions: 2 | 1 passed | 1 failed |
[doctest] Status: FAILURE!
EXIT=1
```

Restored to single passing assertion before commit.

---

## Validation gate

```powershell
PS> pwsh -NoLogo -NoProfile -ExecutionPolicy Bypass -File .\tools\check_gd_scripts_headless.ps1
Checking addons (23 scripts)
Checking sample\gdk_demo (33 scripts)
Checking sample\gdk_launch_point (29 scripts)
Checking sample\multiplayer_pong (28 scripts)
Checking sample\playfab_demo (30 scripts)
Headless GDScript validation passed.
EXIT=0
```

The validator was modified narrowly to skip three vendored / GUT-extending paths
that cannot parse standalone (`addons/godot_gdk/tests_support/`,
`sample/gdk_demo/tests/spike/`, `sample/gdk_demo/tests/packaging_spike/`).
Wave 1 should generalise this to any `tests/gut/` or `tests/**/test_*.gd`
that `extends GutTest`, and ideally fold the exclusion into a
machine-readable config rather than a hard-coded array.

---

## Wave 1 inputs

* **Verified GUT CLI flag set (canonical line, run from a sample project root after one-time `--headless --import`):**
  ```
  godot --headless -s res://addons/gut/gut_cmdln.gd -gdir=res://tests/<dir> -gexit
  ```
  Add `"-gprefix=test_" "-gsuffix=.gd"` only if non-default discovery is needed — and **always quote them** when invoked from PowerShell, otherwise `.gd` is mis-parsed by Godot's CLI parser. `-gexit_on_success` is not required; `-gexit` propagates the failure exit code correctly.

* **Pinned GUT:** tag `v9.6.0`, tarball `https://github.com/bitwes/Gut/archive/refs/tags/v9.6.0.tar.gz`, SHA-256 `3C6C87DDE4C79E8EF109A1C9419B6E654971BA11E7F5AC1731E927EAA0FD1159`.

* **Pinned doctest:** tag `v2.5.2`, header `https://raw.githubusercontent.com/doctest/doctest/v2.5.2/doctest/doctest.h`.

* **Packaging editor-or-headless decision:** Run packaging tests in `--headless` mode in the same orchestrator stage as the rest — `gdk_toolchain.gd`-style pure helpers load fine, and `--editor` refuses to execute `-s gut_cmdln.gd` so a separate editor stage would not run anything anyway.

* **Env propagation method:** Use `[System.Diagnostics.ProcessStartInfo]::new(godotExe)` with `psi.EnvironmentVariables[name] = value` (and `psi.UseShellExecute = $false`); never `$env:NAME = ...` in the parent shell, never `--env-file`. See `tests\cpp\spike\run_env_check.ps1` for the reference pattern (note the async stdout/stderr reads — sync `ReadToEnd()` deadlocks Godot).

---

## Surprises / things Wave 1 should reconsider

1. **GUT class_name registration requires an explicit `--headless --import` step** before `gut_cmdln.gd` will run. A first-run-of-the-day orchestrator must do this once per sample-project that hosts GUT tests, otherwise the runner emits *"Some GUT class_names have not been imported"* and exits 0 without running anything. Worth surfacing in a clear "first run" gate.

2. **`gut_cmdln.gd` returns `exit 0` even when discovery finds zero tests.** The class-name-not-imported case above is the canonical example. The orchestrator must assert "Tests > 0" from GUT's own summary, not just trust the exit code, otherwise a misconfigured `-gdir` is silently green.

3. **`--editor` ignores `-s` script args.** Wave 4's packaging plan should not budget for a separate `--editor` orchestrator stage. Tests that genuinely need a live `EditorInterface` will require a different mechanism (e.g. an `EditorPlugin` autoload that invokes GUT internally) and should be the exception, not the rule.

4. **Parent-shell `$env:VAR` does NOT propagate** into Godot when launched via `Process.Start(psi)` with `UseShellExecute=$false` and no manual env-vars set — the child gets the *current process* env at that snapshot. Setting `psi.EnvironmentVariables[...]` is the only reliable path. The Wave 1 orchestrator must avoid the trap of relying on shell-level `$env:` for sub-process env.

5. **Sync `ReadToEnd()` on both stdout and stderr deadlocks** when Godot is missing GDExtensions and floods stderr (the `Condition "!FileAccess::exists(path)"` torrent fills the pipe buffer in seconds). Use `ReadToEndAsync()` or `BeginOutputReadLine()`/`BeginErrorReadLine()` patterns. This is a generic trap, but very easy to hit with this project's clean-tree-error pattern.

6. **The repo headless validator is sensitive to vendored GDScript that can't parse standalone.** Wave 1 should generalise the exclusion list (or honour `.gdignore` / a sentinel marker file) so future `tests_support/` additions don't require a script edit.

7. **`Godot_v4.6.1-stable_win64_console.exe` is named `Godot_v4.6.1` but reports `4.6.2.stable.official.71f334935`.** Cosmetic, but Wave 1's discovery / version-pinning logic should match against the runtime `--version` output, not the file name.
