# CI: PR Gates

This repo runs its CI on GitHub Actions, on GitHub-hosted runners, split across
two workflows by trigger model. Both share one **build-once / test-across-Godot**
model (see [below](#build-once-per-gdk-test-across-godot-versions)): the native
addon DLLs are built once per Microsoft GDK edition, the build output is cached +
handed off as an artifact, and the test tiers consume that output without
rebuilding.

`pr-gates.yml` is **scoped**: a selector job classifies the changed files and
runs only the gates those files can affect (see
[Scoped gate selection](#scoped-gate-selection)). Branch protection requires the
single aggregate check **`PR gates`**.

| Gate | Workflow (job) | Runs on PR / push when | Runner |
| --- | --- | --- | --- |
| Gate selection | `pr-gates.yml` (`scope`) | always | `ubuntu-latest` |
| CI lint (selector tests, actionlint, PowerShell parse) | `pr-gates.yml` (`ci-lint`) | always | `ubuntu-latest` |
| GDScript parse | `pr-gates.yml` (`parse-gate`) | any parse scope selected; only those scopes | `windows-latest`, **matrixed over Godot versions** |
| Native build (+ C++ doctest) | `pr-gates.yml` (`build`) | any addon or doctest selected; only those addons | `windows-2022`, **default GDK edition** |
| Offline tier (load/smoke + non-live GUT) | `pr-gates.yml` (`test-offline`) | any coverage host selected; only those hosts | `windows-2022`, **default Godot** |
| Editor tools (native-free) | `pr-gates.yml` (`editortools`) | editor-tool sources/tests changed | `windows-latest`, **matrixed over Godot versions** |
| Fuzz replay | `pr-gates.yml` (`fuzz-replay`) | fuzzed production sources or harnesses changed; only those targets | `windows-2022` (pinned — see below) |
| C# facade parity | `pr-gates.yml` (`csharp`) | C# facades, their tests, or `doc_classes` changed | `windows-latest`, .NET 8 SDK |
| **Aggregate (required check)** | `pr-gates.yml` (`PR gates`) | always | `ubuntu-latest` |
| Native build + C++ doctest | `playfab-live-nightly.yml` (`build`) | nightly `schedule` + `workflow_dispatch` | `windows-2022`, **matrixed over GDK editions** |
| Offline tier (load/smoke + non-live GUT) | `playfab-live-nightly.yml` (`test-offline`) | same | `windows-2022`, **GDK editions × Godot supported** |
| PlayFab live (read + write) | `playfab-live-nightly.yml` (`playfab-live`) | same | `windows-2022`, sandbox title, **GDK editions × default Godot** |

`workflow_dispatch` of `pr-gates.yml` and the nightly always run the **full**
set; only `pull_request` and `push` are scoped.

## Scoped gate selection

The `scope` job runs [`tools/ci/pr_gate_scope.cjs`](../../tools/ci/pr_gate_scope.cjs)
over the PR diff (`merge-base(base, head)..head`, renames count both sides) or
the push range (`before..after`). It writes one job output per gate and a
**PR gate scope** table to the job summary listing what was selected and why.

| Changed area (examples) | Selected gates |
| --- | --- |
| `addons/godot_gdk/**`, `tests/godot/gdk/**`, `sample/tutorial_gdk/**` | GDK build + GDK host, GDK parse scopes; doctest/fuzz only when their production sources change |
| `addons/godot_playfab/**`, `tests/godot/playfab/**`, `sample/tutorial_playfab/**` | PlayFab build + PlayFab host (built **without** GDK), PlayFab parse scopes |
| `addons/godot_gameinput/**`, `tests/godot/gameinput/**`, `sample/tutorial_gameinput/**` | GameInput build + host, GameInput parse scopes |
| `addons/godot_gdk_editortools/**`, `tests/godot/gdk/tests/editortools/**` | `editortools` lane only (no native build) |
| `addons/godot_gdk/tests_support/**` (shared GUT bases) | the hosts that use the changed base (the GDK base also feeds PlayFab, which extends it; `test_env.gd` feeds every host + editor tools) |
| `addons/*_csharp/**`, `tests/csharp/**`, `doc_classes/**` | `csharp` (and the addon's host for `doc_classes`) |
| `tests/cpp/**` | doctest and/or the matching fuzz targets |
| `.github/actions/build-addons/**` | all hosts + doctest |
| `.github/actions/run-offline-tier/**`, `tools/ci/gdextension_load_check.gd` | all hosts |
| `.github/godot-versions.json`, `.github/actions/setup-godot/**` | all hosts + editor tools + full parse |
| `.github/gdk-versions.json`, `cmake/GDKDependencies.cmake` | GDK + PlayFab |
| `pr-gates.yml`, `tools/ci/pr_gate_*`, root `CMakeLists.txt`/presets, `godot-cpp`, `vcpkg*.json`, `.gitmodules` | **full** set |
| `docs/**`, `spec/**`, `*.md`, `.github/instructions/**`, other workflows | nothing beyond `scope` + `ci-lint` |
| **Any path without a rule** | **full** set, with a warning listing the path |

Notes:

- **Unknown paths fail safe.** A path no rule matches selects the full set and is
  listed under a warning in the job summary. Add a rule instead of living with it.
- **Malformed selector output fails the run**; an empty selection is valid.
- **Shared code stays shared.** A change to doctest/fuzz production sources
  (`addons/*/src/**` files they compile) re-runs doctest/fuzz even when only one
  addon changed; the C# facade gate runs whenever any facade or `doc_classes`
  file changes.
- **PlayFab-only builds omit GDK** (`GODOT_PLAYFAB_TEST_HOST_WITH_GDK=OFF`). The
  default PlayFab coverage uses custom-ID sign-in and is required to pass without
  GDK; the optional Xbox-backed flows need a live signed-in user and are covered
  by the full nightly.
- **Push to `main`** with an unresolvable `before` (new branch, force push)
  selects the full set.

### Adding or changing a rule

Rules are an ordered table in `pr_gate_scope.cjs` (`RULES`); each matched path
contributes its selection and a reason. Pin the change with a case in
`tools/ci/tests/pr_gate_scope.test.cjs` and run:

```powershell
node --test tools/ci/tests/pr_gate_scope.test.cjs
# Dry-run a selection for any file list:
git diff --name-only origin/main... > $env:TEMP\files.txt
node tools/ci/pr_gate_scope.cjs --files-from $env:TEMP\files.txt
```

Editing the selector, its tests, or `pr-gates.yml` itself always selects the full set.

### The `PR gates` aggregate and branch protection

Skipped jobs never report a failing status, so requiring individual jobs would
either block scoped PRs (required job skipped → pending forever) or let a failure
slip past (job skipped because its dependency failed). The `PR gates` job runs
with `if: always()`, needs every gate, and passes only when:

- `scope` and `ci-lint` succeeded, and
- every **selected** gate succeeded, and every **unselected** gate was skipped.

**Rollout (repo admin):** in *Settings → Branches* (or the ruleset) for `main`
and `experimental/dotnet`, replace the old required checks (`parse-gate (…)`,
`build`, `test-offline`, `fuzz-replay`, `csharp`) with the single **`PR gates`**
check. Do this right after the change merges. Until then, the old names never
report on new PRs and those PRs wait on them. Because `pr-gates.yml` has no
`paths-ignore`, `PR gates` reports on every PR, including docs-only PRs.

## Build once per GDK, test across Godot versions

The addon `.gdextension` files declare `compatibility_minimum = "4.5"` and bind
through the GDExtension interface bundled in the pinned `godot-cpp` submodule.
A GDExtension is **forward-compatible only**: a DLL built against godot-cpp's
`extension_api.json` for interface version *N* loads on engine *N* and every
newer 4.x, but is rejected by any engine older than *N*. So the `godot-cpp` pin
**must track the oldest supported Godot line** (currently the `4.5` branch, whose
bundled API is `4.5`); a build against a newer line (e.g. `4.6`) would fail to
load on 4.5.1 even though the manifest claims `4.5`. With the pin on 4.5, **one
native build per GDK edition is valid across the whole Godot 4.x test matrix**.
CI exploits this: it never rebuilds the
DLLs per Godot version. Two shared local composite actions implement the model so
`pr-gates.yml` and `playfab-live-nightly.yml` reuse one implementation:

- **`.github/actions/build-addons`** — builds the addons against one GDK edition
  and packages the output. Inputs: `gdk_version` (ms-gdk port version; blank =
  registry baseline), `artifact_name`, `components` (comma-separated subset of
  `gdk,playfab,gameinput`; default all; blank = doctest-only), `doctest`
  (default `true`). It exports `VCPKG_ROOT`, pins the edition via a `vcpkg.json`
  `overrides` entry, restores the per-edition **per-scope** build cache,
  configures `cmake --preset default` with explicit `BUILD_GODOT_*` /
  `VCPKG_MANIFEST_FEATURES` flags for the selected addons, builds with
  `cmake --build build --preset debug`, runs the C++ doctest **once** when
  selected (`build\bin\Debug\gdk_unit_tests.exe`), verifies that exactly the
  selected host DLLs were produced, saves the cache, and uploads the build output
  as an artifact. Output: `artifact_name`, `cache_key`.
- **`.github/actions/run-offline-tier`** — restores a build artifact and runs the
  offline tier against one Godot version. Inputs: `godot_version`,
  `artifact_name`, `hosts` (comma-separated subset of `gdk,playfab,gameinput`;
  default all), `skip_parse` (default `false`; the PR workflow passes `true`
  because `parse-gate` is its own job). It downloads the artifact, **selects the GUT version for the
  engine** (see [below](#per-version-gut-45-vs-46)), sets up Godot via
  `setup-godot`, stages the GDK/PlayFab redist DLLs next to `Godot.exe`, runs the
  **GDExtension load/smoke** (below), then `run_all_tests.ps1 -SkipBuild
  -SkipDoctest -SkipOrchestrator` (non-live GUT + bootstrap), and uploads the run
  summary.

### Caching and artifact handoff

Caching is **hybrid**:

- **Cross-run `actions/cache`** on the whole `build/` tree (which includes
  `build/vcpkg_installed`), keyed on the GDK edition **and** a source fingerprint
  (git object SHAs of `addons/`, `cmake/`, `tests/cpp/`, `CMakeLists.txt`,
  `CMakePresets.json`, the committed `vcpkg.json` + `vcpkg-configuration.json`
  manifest/config, and the `godot-cpp` + `third_party/Gut` + `third_party/Gut-4.5`
  submodule pins). The key also carries the build scope label (e.g.
  `playfab+doctest`, `all+doctest`). `restore-keys` try the same scope
  (`build-<OS>-gdk-<edition>-<scope>-`) first, then any scope of the edition
  (`build-<OS>-gdk-<edition>-`), to seed a warm vcpkg restore + incremental
  objects. Editions never cross-contaminate. The cache is saved only on a
  non-exact restore.
- **Per-run upload artifact** (`build-pr` for PRs, `build-gdk-<edition>` for the
  nightly) for the intra-run handoff to the test jobs. It carries the
  build-generated overlay the test tiers need: the mirrored `tests/godot/*/addons/`
  tree (addon DLLs + **both GUT mirrors** — `gut` (9.6.0) and `gut-4.5` — plus the
  `godot_gdk_tests` bases; the whole tree is `.gitignore`d), the doctest exe, and
  the vcpkg redist DLLs.

The test jobs check out **without submodules** — the GUT mirror arrives via the
artifact — and never run `cmake`.

### GDExtension load/smoke

Before the GUT suites, `run-offline-tier` fail-fasts with a clear per-version
message if a native addon DLL cannot load under the matrix Godot build. It copies
`tools/ci/gdextension_load_check.gd` into each coverage host (`gdk`, `playfab`,
`gameinput`), runs `--import`, then runs the checker (a headless `SceneTree`) and
asserts `GDExtensionManager.is_extension_loaded(<host .gdextension>)`. This turns
"the DLL doesn't load on Godot X" into a precise, early failure instead of a
confusing mid-suite GUT error.

### Per-version GUT (4.5 vs 4.6)

GUT itself has a hard engine floor: **9.6.0** (`required_godot_version = '4.6'` in
`addons/gut/utils.gd`) refuses to run on Godot 4.5.x. But 4.5.1 is a supported
engine, so the 4.5.x offline leg needs a GUT that runs there. There is no tagged
GUT release that both runs on 4.5.x **and** matches 9.6.0's behaviour, so the repo
carries **two** GUT submodules:

- `third_party/Gut` — pinned to upstream tag **v9.6.0**; the default. Used for
  every Godot 4.6+ leg.
- `third_party/Gut-4.5` — pinned to upstream commit **b366b70** (`v9.5.0` + the
  #778 `is_push_warning` fix, still `required_godot_version = '4.5'`). Used for
  Godot 4.5.x legs.

The `is_push_warning` fix matters: before it, GUT classified every `push_warning()`
(including addon `UtilityFunctions::push_warning` on deliberately-tested bad-input
paths) as a failable *engine* error, so the plain v9.5.0 tag reports ~50 spurious
failures. `b366b70` exempts push_warnings exactly as 9.6.0 does, so **both legs
produce identical results** (verified: gdk 225 / playfab 53 / gameinput 46 passing,
0 failures, on both 4.5.1 and 4.6.x).

CMake mirrors *both* trees into every coverage host — `addons/gut` (9.6.0) and
`addons/gut-4.5` — and the artifact packages both. `run-offline-tier`'s
**Select GUT** step swaps `addons/gut-4.5` over `addons/gut` only when
`godot_version` matches `4.5.x`; 4.6+ legs use the default mirror untouched.
Locally, `run_all_tests.ps1` does the same via `Select-GutForGodotVersion`, which
re-establishes the correct GUT from whichever submodule is present (no-op in the
artifact-only CI path). Swapping GUT invalidates Godot's cached `class_name`
globals **and** forces a reimport of GUT's font/resources, so the host import
runs **two** `--import` passes to settle the class cache before GUT starts — and,
because Godot 4.5.x's headless importer can intermittently crash (access
violation) mid-reimport, both the load/smoke import and the orchestrator's import
retry a crashed pass (the reimport is incremental, so a retry resumes it; a
deterministic failure still fails).

### The `-SkipDoctest` flag

`tools/run_all_tests.ps1` takes a `-SkipDoctest` switch (mirrors `-SkipGut` /
`-SkipOrchestrator`): it records the C++ doctest stage as `skip` instead of
running it. The offline matrix passes it because the Godot-independent doctest
already ran once in the `build` job — re-running it on every Godot version would
be pure waste. The live tier also passes it (it reuses the same build artifact).

## Godot version range

The set of Godot versions CI exercises is data-driven in
[`.github/godot-versions.json`](../../.github/godot-versions.json). Policy:
**support Godot 4.5 and newer** (latest patch of each minor line):

```json
{
  "default": "4.7.1-stable",
  "supported": ["4.7.1-stable", "4.6.1-stable", "4.5.1-stable"],
  "sha512": {
    "4.7.1-stable": "a6b02c52…",
    "4.6.1-stable": "67c63291…",
    "4.5.1-stable": "ab84df90…"
  }
}
```

- `default` — used by single-version jobs: the PR `build` + `test-offline`
  jobs, the nightly live tier, and the `workflow_dispatch` default.
- `supported` — the matrix the **parse gate** and the **nightly offline tier**
  fan out over (one leg per version, `fail-fast: false` so each reports
  independently).
- `sha512` — the supply-chain pin: the SHA-512 of each version's
  `Godot_v<version>_win64.exe.zip` (see [Supply-chain hardening](#supply-chain-hardening)).

**To add a Godot version to the range**, append its version string to
`supported` **and** its zip's SHA-512 to `sha512`. The version must be a real
`godotengine/godot-builds` release tag whose asset `Godot_v<version>_win64.exe.zip`
exists (the zip bundles the `_console.exe` the gates use). No workflow edits are
needed — the `versions` job reads the manifest and the matrix updates
automatically. `setup-godot` **refuses** any version missing from `sha512`.

The `setup-godot` composite action (`.github/actions/setup-godot/`) downloads,
caches, and exposes any requested version via `GODOT` / `GODOT_CONSOLE` /
`GODOT_BIN`, matching the discovery order in `tools/check_gd_scripts_headless.ps1`
and `tools/run_all_tests.ps1`. The repo tooling does not pin or enforce a Godot
version, so widening the range is purely additive.

## Parse gate

Runs `tools/check_gd_scripts_headless.ps1` (headless `--check-only` over every
`.gd` file). One matrix leg per supported Godot version. On scoped PRs the job
passes `-Paths <scope,...>` so only the selected projects/directories are
parsed; full selections parse everything. The gate needs no native build (the
coverage hosts are skipped by their `.gut_skip_validation` sentinels and run in
`test-offline`/`editortools` instead). The validator groups
`.gd` files by context: scripts inside a Godot project are checked in place;
repo-root `addons/` and `sample/addons/` scripts are checked against a
lightweight temp project; and **standalone project-less scripts** (e.g. CI
helpers under `tools/`, such as `tools/ci/gdextension_load_check.gd`) are copied
into a shared bare temp project — preserving their repo-relative path — and
checked there.

## Native build + offline tier (PRs)

When the selector picks any addon (or the doctest), the PR builds natively and
runs the offline test tier for **only the selected addons**, via the two shared
composites ([above](#build-once-per-gdk-test-across-godot-versions)):

- **`build`** (`windows-2022`) — uses the **default** GDK edition from
  `gdk-versions.json` and `uses: ./.github/actions/build-addons` with
  `components` / `doctest` from the selector. Produces the `build-pr` artifact +
  the per-scope build cache, and runs the C++ doctest once when selected. (A
  cold cache pays the full ms-gdk + godot-cpp compile; the vcpkg +
  build-output caches make repeat PRs fast.)
- **`test-offline`** (`windows-2022`, single **default** Godot) — `needs:
  build`; `uses: ./.github/actions/run-offline-tier` with the `build-pr`
  artifact, `hosts` = the selected addons, and `skip_parse: true`. Runs the
  GDExtension load/smoke + non-live GUT + bootstrap for those hosts.

## Editor tools lane (native-free)

The `godot_gdk_editortools` suites (packaging, settings, menu) mock the editor
plugin and never load a native DLL. The `editortools` job therefore runs them
without CMake, vcpkg, or the GDK: it fetches only the GUT submodules, runs
[`tools/ci/prepare_editortools_host.ps1`](../../tools/ci/prepare_editortools_host.ps1)
to stage a temp Godot project under `build/editortools-host` (the addon, the
shared test bases, the editor-tool suites, and the GUT build matching the Godot
version), then runs `run_all_tests.ps1 -Hosts build/editortools-host -SkipBuild
-SkipDoctest -SkipParse -SkipOrchestrator`. It fans out over the supported Godot
versions. To reproduce locally:

```powershell
pwsh -File tools/ci/prepare_editortools_host.ps1 -GodotVersion 4.6.2-stable
pwsh -File tools/run_all_tests.ps1 -Hosts build/editortools-host -SkipBuild -SkipDoctest -SkipParse -SkipOrchestrator
```

A full selection still runs these suites inside the GDK host too.

## CI lint

`ci-lint` always runs (seconds on `ubuntu-latest`) so workflow, action, and
tooling PRs get real signal without a native build: the selector unit tests,
`actionlint` (pinned) over the hand-written workflows (the generated gh-aw
`*.lock.yml` files are excluded), and a PowerShell syntax parse of every
`tools/**/*.ps1`/`*.psm1`. Workflow or action changes also select whatever runtime
gates the touched action feeds (table above).

Neither job uses secrets, so both are safe on fork PRs. The PR leg deliberately
uses the single default GDK + default Godot for the fastest signal; the nightly
fans the **same** composites across the full GDK × Godot matrix.

## Fuzz replay

Builds the libFuzzer targets with the `fuzz` preset (ClangCL + ASan, static CRT;
**no Microsoft GDK required**) and replays each target's committed corpus once:

```powershell
cmake --preset fuzz
cmake --build build/fuzz --preset debug-fuzz
pwsh -File tools/run_fuzz_replay.ps1
```

This is **regression replay, not active fuzzing** — `run_fuzz_replay.ps1` passes
the individual corpus files to each target, so libFuzzer runs every input exactly
once and exits non-zero on the first crash/leak/timeout. (Passing a corpus
*directory* would make libFuzzer fuzz indefinitely; the script deliberately
passes files.) Corpora live at `tests/cpp/fuzz/corpus/<target>/`. A target with
no corpus is still smoke-run with `-runs=0`, which executes the initial inputs
(the implicit empty input) once with **zero** additional mutations and exits —
a finite, deterministic load/link/ASan-init check. (libFuzzer's "run forever"
sentinel is `-runs=-1`, not `0`.) On failure, the workflow uploads any
`crash-*` / `leak-*` / `timeout-*` artifacts.

The `fuzz-replay` job is pinned to **`windows-2022`** because the `fuzz` preset
uses the `Visual Studio 17 2022` generator + `ClangCL` toolset; `windows-latest`
has moved to a VS2026 image with no VS2022 instance, so the preset can't
configure there.

To add a regression input, drop the bytes into the matching
`tests/cpp/fuzz/corpus/<target>/` directory and commit it. A corpus directory
or `tests/cpp/fuzz/*.cpp` file that doesn't match a known target is treated as an
unknown path: it selects the full gate set and fails the `ci-lint` route-coverage
test until the target is registered in `tools/ci/pr_gate_scope.cjs`.

On scoped PRs the job builds and replays only the targets the selector picked
(changed harness, corpus, or production source the target compiles):

```powershell
cmake --build build/fuzz --preset debug-fuzz --target gdk_fuzz_result_formatting playfab_fuzz_key_lookup
pwsh -File tools/run_fuzz_replay.ps1 -Targets gdk_fuzz_result_formatting,playfab_fuzz_key_lookup
```

`-Targets` fails on an unknown or unbuilt target instead of silently skipping it.

## C# facade parity gate (`experimental/dotnet`)

The `experimental/dotnet` branch adds a managed C# facade layer over the native
addons: `addons/godot_gdk_csharp/`, `addons/godot_playfab_csharp/`, and
`addons/godot_gameinput_csharp/` wrap the same GDExtension singletons the
GDScript API uses. The facades do **not** re-bind anything — they call straight
through the native singletons (`GodotObject.Call` / `Get`), so the native DLL
stays the single binding surface. The `csharp` job guards that layer:

```powershell
pwsh -File tools/run_csharp_tests.ps1
```

which `dotnet build`s the three facade class libraries and then runs the
`tests/csharp/FacadeParity.Tests` xUnit suite. `FacadeParity.Tests` reflects over
the built facade assemblies and asserts that **every** method / member / signal
documented in each native `doc_classes/*.xml` has a matching managed member — so
when the native surface gains an API (e.g. a new bound method), the C# facade
must grow a wrapper in the same change or the gate fails.

It is pure managed metadata: `net8.0` + the `Godot.NET.Sdk` NuGet SDK, with **no
Godot binary, no native/CMake build, no submodules, and no secrets**. It runs on
a bare `windows-latest` with only the .NET 8 SDK provisioned by
`actions/setup-dotnet`, so it is fast and fork-safe.

**Trigger scoping.** The `csharp` job runs on `pull_request` / `push` to both
`main` and `experimental/dotnet` when the selector picks it: any change under
`addons/*_csharp/`, `tests/csharp/`, `tools/run_csharp_tests.ps1`, or an addon's
`doc_classes/` (the parity source of truth).

## Nightly: build → offline → live (`playfab-live-nightly.yml`)

The nightly is decomposed into `resolve` → `build` → (`test-offline`,
`playfab-live`) so the addons are built **once per GDK edition** and that output
is tested across Godot versions and against the live sandbox without per-leg
rebuilds:

- **`resolve`** (`ubuntu-latest`) — emits the GDK edition list, the single
  default Godot (live tier), and the Godot `supported` matrix (offline tier).
- **`build`** (`windows-2022`, matrix = GDK editions) — `uses:
  ./.github/actions/build-addons` per edition; uploads `build-gdk-<edition>` +
  the per-edition build cache; runs the C++ doctest once.
- **`test-offline`** (`windows-2022`, matrix = GDK editions × Godot `supported`)
  — `needs: build`; `uses: ./.github/actions/run-offline-tier` per
  edition×version, restoring `build-gdk-<edition>`. This is the build-once /
  test-many payoff: the load/smoke + non-live GUT run on every supported engine
  version with **no rebuild**. Each leg uploads
  `offline-run-summary-gdk-<edition>-godot-<version>`. Gated on
  `!cancelled() && needs.resolve.result == 'success'` (not plain `success()`),
  so a single failed `build` edition does **not** skip the rest of the matrix —
  the affected legs fail loudly at artifact download while the others run.
- **`playfab-live`** (`windows-2022`, matrix = GDK editions × default Godot) —
  the live (read + write) tier (below). It also `needs: build` and **restores
  the per-edition build artifact instead of rebuilding**. Uses the same
  `!cancelled() && needs.resolve.result == 'success'` gate as `test-offline` to
  preserve per-edition signal when one build edition fails.

### Live tier

`playfab-live` runs the live tier against a **dedicated sandbox PlayFab title**,
reusing the cached build:

```powershell
# After restoring the build-gdk-<edition> artifact + staging redist DLLs:
tools/run_all_tests.ps1 -SkipBuild -SkipDoctest -SkipOrchestrator -Live -AllowLiveWrites -PlayFabTitleId <sandbox> ...
```

`-SkipBuild` reuses the artifact and `-SkipDoctest` skips the doctest (it already
ran in `build`). `-SkipOrchestrator` disables the multi-client PlayFab Multiplayer
orchestrator (stage 5) on the hosted runner: hosted Windows Server runners can't
reliably form the real-time Party P2P mesh between spawned client processes, and
the live Lobby/Match service calls intermittently time out (host `create_lobby`
hangs ~60-70s), whose respawn cascade also fails otherwise-healthy scenarios. The
single-process GUT live suites (gdk/playfab/gameinput) still run and gate the job;
a self-hosted Win11 runner should omit `-SkipOrchestrator` to exercise the full
orchestrator. The live tier runs only on a nightly schedule and manual
`workflow_dispatch` (never on `pull_request`), so title secrets are never
reachable from fork PRs.
The `workflow_dispatch` form accepts an optional `godot_version` (defaults to the
manifest `default`) and an optional `gdk_version` (see the GDK edition matrix
below).

### GDK edition matrix

The Microsoft GDK edition the live tier builds against is **not** fixed: the set
of editions is data-driven in
[`.github/gdk-versions.json`](../../.github/gdk-versions.json). Each entry is an
`ms-gdk` vcpkg port version (`YYMM.N.<build>`, mapping to the 6-digit edition
`YYMM0N`). Policy: **support edition 251001 and newer**, limited to editions
published to the **public** vcpkg registry and reachable from the baseline in
`vcpkg-configuration.json`:

```json
{
  "default": "2604.1.7839",
  "supported": [
    { "version": "2604.1.7839", "edition": "260401", "release": "April 2026" },
    { "version": "2510.2.6247", "edition": "251002", "release": "October 2025" },
    { "version": "2510.1.6224", "edition": "251001", "release": "October 2025" }
  ]
}
```

The Microsoft GDK edition the addons build against is **not** fixed: the set
of editions is data-driven in
[`.github/gdk-versions.json`](../../.github/gdk-versions.json). Each entry is an
`ms-gdk` vcpkg port version (`YYMM.N.<build>`, mapping to the 6-digit edition
`YYMM0N`). Policy: **support edition 251001 and newer**, limited to editions
published to the **public** vcpkg registry and reachable from the baseline in
`vcpkg-configuration.json`:

```json
{
  "default": "2604.1.7839",
  "supported": [
    { "version": "2604.1.7839", "edition": "260401", "release": "April 2026" },
    { "version": "2510.2.6247", "edition": "251002", "release": "October 2025" },
    { "version": "2510.1.6224", "edition": "251001", "release": "October 2025" }
  ]
}
```

The edition list drives the nightly `build`, `test-offline`, and `playfab-live`
matrices. The `build` job (via the `build-addons` composite) injects the selected
`ms-gdk` version as a top-level `overrides` entry in `vcpkg.json` before
`cmake --preset default`, so vcpkg restores exactly that edition (this automates
the manual pin documented in
[getting-started.md](../getting-started.md#switching-gdk-editions-on-the-vcpkg-path)).
`test-offline` and `playfab-live` then consume that edition's build artifact —
they do not re-pin or rebuild.

- **`default`** — the single edition the **nightly `schedule`** runs (keeps
  nightly sandbox load low), and the edition the PR `build` job uses.
- **`supported`** — the **full matrix**, run on manual `workflow_dispatch`. Pass
  a specific `gdk_version` input (e.g. `2510.2.6247`) to run just one edition.
- The `playfab-live` matrix uses `max-parallel: 1` (the legs are live **writes**
  against the shared sandbox title, so they must serialize) and `fail-fast:
  false` (one edition failing still reports the others). Each leg uploads its run
  summary as `playfab-live-run-summary-gdk-<edition>`.
- Editions outside the public registry (e.g. `260400`, `260402`) are reachable
  only via the `installed-gdk` path, not on a hosted runner, so they are not in
  this matrix. To widen coverage, add an entry whose `version` exists in
  `microsoft/vcpkg` `versions/m-/ms-gdk.json` reachable from the baseline — no
  workflow edits are required.

### Runner provisioning (GDK runs on the hosted runner)

The native jobs run on **hosted Windows Server 2022 runners** — no self-hosted
runner is required. Three things make the Microsoft GDK runtime work there:

1. **`VCPKG_ROOT` is exported** from the image's `VCPKG_INSTALLATION_ROOT` (the
   `default` preset reads `VCPKG_ROOT`). The `build` job's configure restores the
   PlayFab SDK + GDK redist via the vcpkg `ms-gdk` port.
2. **The runners are pinned to `windows-2022`** because the `default` preset
   targets the `Visual Studio 17 2022` generator (`windows-2025` ships VS2026).
   The build runner ABI-matches the test runners so the native DLLs are
   compatible end-to-end.
3. **The GDK redist DLLs are staged next to `Godot.exe`** in every test job
   (`build/vcpkg_installed/x64-windows/bin/*.dll`, restored from the build
   artifact). Without this, `xgameruntime.dll` loads but its init returns
   `E_GAMERUNTIME_DLL_NOT_FOUND` (`0x89240101`) because the thunk chain is not on
   the Godot **process** search path.

Game Saves is the one live surface that stays unavailable on an unpackaged
hosted runner: `PFGameSaveFilesInitialize` needs a packaged GDK title identity.
`PlayFab.initialize()` treats that failure as **non-fatal** (logs a warning,
keeps Core/Services up), so the rest of the live tier runs. The custom-ID Game
Saves tests still execute and assert `xbox_user_required`.

### Required secrets (GitHub Environment `playfab-sandbox`)

Configure these as **environment** secrets on an environment named
`playfab-sandbox` (Settings → Environments). The job refuses to run if
`PLAYFAB_TITLE_ID` is unset.

| Secret | Required | Purpose |
| --- | --- | --- |
| `PLAYFAB_TITLE_ID` | yes | The sandbox title id. **Must be a dedicated sandbox title** — the live tier writes persistent state. |
| `PLAYFAB_CUSTOM_ID` | optional | Pre-existing custom id for `LoginWithCustomID`. |
| `PLAYFAB_MULTIPLAYER_MATCH_QUEUE` | optional | Matchmaking queue name for Multiplayer coverage. |

> **No developer secret key.** `PLAYFAB_DEVELOPER_SECRET_KEY` is **not** used by
> this gate. `run_all_tests.ps1` scrubs it from child processes, and a
> pre-provisioned title only needs client-side sign-in. That high-privilege
> secret belongs only to a separate, manual title-provisioning step
> (`tools/configure_playfab_test_title.ps1`), not to the per-run live gate.

## Supply-chain hardening

Both workflows are hardened so a compromised upstream (a third-party action, or
the Godot download) cannot silently inject code into a CI run:

- **Third-party actions are pinned to full commit SHAs**, not floating tags. Each
  `uses:` references an immutable 40-char commit with a trailing `# vX.Y.Z`
  comment for readability (`actions/checkout`, `actions/cache`,
  `actions/upload-artifact`, `actions/download-artifact`). Re-tagging a release
  upstream cannot change what runs.
- **The Godot download is verified against a pinned SHA-512** committed in
  `.github/godot-versions.json` (`sha512` map). `setup-godot` downloads the zip
  into a staging directory **outside** the cached path, verifies the hash, and
  only extracts into the cache **after** it matches — so a tampered or corrupt
  download can never poison the per-version cache. A version with no pinned hash
  is refused outright (no network fetch).
- **Least privilege:** both workflows declare `permissions: contents: read`, and
  every `actions/checkout` sets `persist-credentials: false` so the `GITHUB_TOKEN`
  is not left in the local git config after checkout.

When bumping a pinned action, resolve the new tag to its commit SHA (e.g.
`gh api repos/actions/checkout/commits/v5.0.1 --jq .sha`) and update both the SHA
and the `# vX.Y.Z` comment. When adding a Godot version, compute the zip hash with
`(Get-FileHash -Algorithm SHA512 <zip>).Hash.ToLower()` (optionally cross-checked
against the release's `SHA512-SUMS.txt`) and add it to the `sha512` map.

## Known limitations

- **Game Saves is unavailable on the unpackaged hosted runner.** `PFGameSaveFilesInitialize`
  requires a packaged GDK title identity, which a hosted runner running a custom-ID
  session does not have. `PlayFab.initialize()` degrades gracefully (warns, keeps
  Core/Services up) so the rest of the live tier runs; the custom-ID Game Saves tests
  assert the `xbox_user_required` rejection rather than exercising real cloud saves.
  Full Game Saves coverage requires a packaged title run (out of scope for the hosted gate).
- **The live Multiplayer orchestrator is skipped on hosted runners.** The
  `playfab-live` job passes `-SkipOrchestrator`, so stage 5 (the multi-client
  PlayFab Multiplayer/Party orchestrator) records as `skip`. Hosted Windows Server
  runners can't reliably form the real-time Party P2P mesh between spawned client
  processes, and live Lobby/Match service calls intermittently time out (host
  `create_lobby` hangs ~60-70s -> `proxy_closed_request_timeout_host_create_lobby`),
  whose respawn cascade also fails otherwise-healthy scenarios. The single-process
  GUT live suites (gdk/playfab/gameinput) still run and gate the job. Full
  orchestrator coverage requires a self-hosted Win11 runner (which omits
  `-SkipOrchestrator`).
- **No interactive Xbox identity on the hosted runner.** GDK silent sign-in
  (`GDK.users.add_default_user_async()`) has no signed-in Xbox account to bind to
  on a hosted custom-id session, so it stays pending and yields no primary user.
  The `run_auto_add_primary_user_true` bootstrap mini-runner therefore asserts
  only that the startup autoload *kicked off* the auto-add (a still-pending
  attempt passes), not that a user actually signs in. A real signed-in user is
  only reachable on a self-hosted Win11 runner or a developer machine.
- **Fuzz infra dependency.** The `fuzz-replay` job assumes the fuzz harness /
  `fuzz` preset (originally on `infra/fuzz-testing`) is present on `main`.
