# CI: PR Gates

This repo runs three CI gates on GitHub Actions, on GitHub-hosted runners. Each
workflow first reads the Godot manifest in a tiny `ubuntu-latest` job, then runs
the actual gate on a Windows runner. They are split across two workflows by
trigger model.

| Gate | Workflow | Trigger | Runner |
| --- | --- | --- | --- |
| GDScript parse | `.github/workflows/pr-gates.yml` (`parse-gate`) | `pull_request` / `push` to `main`, `workflow_dispatch` | `windows-latest`, **matrixed over Godot versions** |
| Fuzz replay | `.github/workflows/pr-gates.yml` (`fuzz-replay`) | same as above | `windows-2022` (pinned — see below) |
| PlayFab live (read + write) | `.github/workflows/playfab-live-nightly.yml` | nightly `schedule` + `workflow_dispatch` | `windows-2022`, sandbox title (GDK via vcpkg `ms-gdk`) |

> The matrix-resolve / version-resolve steps (`versions`, `resolve-version`) run
> on `ubuntu-latest`; the Windows runners above do the actual gate work.

## Godot version range

The set of Godot versions CI exercises is data-driven in
[`.github/godot-versions.json`](../../.github/godot-versions.json). Policy:
**support Godot 4.5 and newer** (latest patch of each minor line):

```json
{
  "default": "4.6.1-stable",
  "supported": ["4.6.1-stable", "4.5.1-stable"],
  "sha512": {
    "4.6.1-stable": "67c63291…",
    "4.5.1-stable": "ab84df90…"
  }
}
```

- `default` — used by single-version jobs (the live tier, and the
  `workflow_dispatch` default for it).
- `supported` — the matrix the **parse gate** fans out over (one leg per
  version, `fail-fast: false` so each reports independently).
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
`.gd` file). One matrix leg per supported Godot version.

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
`tests/cpp/fuzz/corpus/<target>/` directory and commit it.

## PlayFab live tests (nightly)

`playfab-live-nightly.yml` runs the live tier against a **dedicated sandbox
PlayFab title**:

```powershell
tools/run_all_tests.ps1 -Live -AllowLiveWrites -PlayFabTitleId <sandbox> ...
```

It runs only on a nightly schedule and manual `workflow_dispatch` (never on
`pull_request`), so title secrets are never reachable from fork PRs. The
`workflow_dispatch` form accepts an optional `godot_version` (defaults to the
manifest `default`).

### Runner provisioning (GDK runs on the hosted runner)

The live job runs on a **hosted Windows Server 2022 runner** — no self-hosted
runner is required. Three things make the Microsoft GDK runtime work there:

1. **`VCPKG_ROOT` is exported** from the image's `VCPKG_INSTALLATION_ROOT` (the
   `default` preset reads `VCPKG_ROOT`). Configuring the preset restores the
   PlayFab SDK + GDK redist via the vcpkg `ms-gdk` port.
2. **The runner is pinned to `windows-2022`** because the `default` preset
   targets the `Visual Studio 17 2022` generator (`windows-2025` ships VS2026).
3. **The GDK redist DLLs are staged next to `Godot.exe`** after configure
   (`build/vcpkg_installed/x64-windows/bin/*.dll`). Without this, `xgameruntime.dll`
   loads but its init returns `E_GAMERUNTIME_DLL_NOT_FOUND` (`0x89240101`) because
   the thunk chain is not on the Godot **process** search path.

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
  `actions/upload-artifact`). Re-tagging a release upstream cannot change what runs.
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
- **Fuzz infra dependency.** The `fuzz-replay` job assumes the fuzz harness /
  `fuzz` preset (originally on `infra/fuzz-testing`) is present on `main`.
