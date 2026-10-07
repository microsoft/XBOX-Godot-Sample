# PR-local live validation

GitHub Actions cannot prove the thing this repo most needs proven. Hosted runners have no
GDK, no Xbox sandbox, and cannot form the PlayFab Party P2P mesh, so the hosted PR gates
run the **offline** tier only. A fully green PR is therefore not evidence that a GDK bump,
a PlayFab change, or an engine bump actually works against live services.

`tools\validate_pr_local.ps1` closes that gap. It pulls a pull request down to a fresh,
private checkout on a self-hosted Windows machine, builds it, runs the canonical
`tools\run_all_tests.ps1` live-write suite once per supported Godot version, builds
Release, and posts an honest result back on the PR.

> Related: [PR gates](pr-gates.md) describes the hosted, scoped gate set.
> [`tests/godot/README.md`](../../tests/godot/README.md) defines the test tiers this run
> exercises.

## Usage

```powershell
# Print the plan. Touches nothing.
pwsh -NoLogo -NoProfile -ExecutionPolicy Bypass -File .\tools\validate_pr_local.ps1 `
  -PullRequest 202 -DryRun

# Run it for real.
pwsh -NoLogo -NoProfile -ExecutionPolicy Bypass -File .\tools\validate_pr_local.ps1 `
  -PullRequest 202 -AllowLiveWrites
```

| Parameter | Purpose |
| --- | --- |
| `-PullRequest <n>` | Required. The PR number in `microsoft/XBOX-Godot-Sample`. |
| `-AllowLiveWrites` | Required to execute. Without it the script refuses, because every run writes to a live PlayFab title. |
| `-DryRun` | Resolve the PR and print the plan, then exit 0. Also reports whether `XblPCSandbox.exe` was found and whether the shell is elevated, so you can tell up front if the machine can run it. |
| `-NoComment` | Produce the artifacts but do not post to the PR. |
| `-WorkRoot <path>` | Override the run-directory root. Defaults to `%LOCALAPPDATA%\godot-gdk-pr-validation`. |

### Prerequisites

- Windows, with the GDK and Visual Studio build tools installed (the same machine that can
  already run `cmake --preset default`).
- `git`, `gh`, `cmake`, and `pwsh` on `PATH`, and `gh auth status` succeeding.
- `XblPCSandbox.exe` available for sandbox switching.
- An **elevated** shell if the machine is not already in the test sandbox. Switching
  sandboxes restarts Xbox Live Auth Manager and requires elevation.

> **Strongly recommended: put the machine in the test sandbox *before* you start, and sign
> in.** Switching the sandbox restarts Xbox Live Auth Manager, which signs the Xbox app out,
> and the GDK live tiers need a signed-in test account. This script cannot sign one in for
> you — that is an interactive system UI. If it has to switch mid-run, the GDK legs will most
> likely come back `no_default_user`. Switch first, sign a test account for that sandbox into
> the Xbox app, and then run: the script sees it is already in the right sandbox, skips both
> the switch and the restore, and no longer needs elevation.

## What a run actually does

1. **Preflight** — tool availability, `gh` auth, and (only if a sandbox switch is needed)
   elevation.
2. **Resolve the PR** — `gh pr view` for the title, state, base, and head SHA.
3. **Serialize** — take a machine-local named mutex so two runs cannot share the PlayFab
   title. See the limitation below.
4. **Checkout** — fetch `refs/pull/<n>/head` into a brand-new run directory and
   `git checkout --detach <sha>`, then init submodules and verify `HEAD` matches the SHA.
   The **head commit** is validated, never the merge result, so the evidence corresponds to
   a commit that actually exists on the branch.
5. **Build Debug** — `cmake --preset default` then `cmake --build --preset debug`.
6. **Prove the SDK identity** — read `_GRDK_EDITION` out of
   `build\vcpkg_installed\x64-windows\include\grdk.h` and the `ms-gdk` `versionInfo` out of
   that tree's `share\ms-gdk\vcpkg.spdx.json`. This is what vcpkg *restored*, as opposed to
   what `.github\gdk-versions.json` *claims*.
7. **Acquire engines** — read `supported` from the **candidate checkout's**
   `.github\godot-versions.json` and download each version through
   `tools\ci\get_godot.ps1`, verifying the pinned SHA-512 before extraction. Engines are
   installed run-private, so whatever Godot is installed on the machine is irrelevant.
8. **Switch the Xbox sandbox** — capture the current sandbox first and write
   `SANDBOX-RESTORE.txt` before changing anything.
9. **Run the live matrix** — for each engine, with `GODOT`/`GODOT_BIN`/`GODOT_CONSOLE`
   pointed at that engine and the working directory inside the candidate checkout:

   ```powershell
   pwsh -File tools\run_all_tests.ps1 -SkipBuild -Live -AllowLiveWrites `
     -PlayFabTitleId 10D176 -PlayFabCustomId godot-gdk-ext-live-smoke `
     -PlayFabMatchmakingQueue godot_gdk_ext_live_smoke_queue -OutDir <results>\<version>
   ```

   The C++ doctest binary is engine-independent, so it runs on the first leg only;
   subsequent legs add `-SkipDoctest`.
10. **Restore the sandbox** in a `finally`. If restoration fails, `SANDBOX-RESTORE.txt`
    stays behind with the command to run by hand.
11. **Build Release** — `cmake --preset default-release` then
    `cmake --build --preset release`. This runs *after* the live matrix because builds
    mirror binaries into shared addon and test-host directories.
12. **Re-check the head SHA** and flag drift if the PR moved during the run.
13. **Report** — write `validation-manifest.json` and `report.md`, then
    `gh pr comment --body-file` unless `-NoComment`.

## Why a green exit code is not enough

`run_all_tests.ps1` can exit 0 while covering nothing that matters:

- An unfiltered PlayFab Multiplayer orchestrator run is allowed to pass with zero scenarios.
- A live GUT host can report every test as pending when the live gate is never reached.
- A leg invoked without `-Live`/`-AllowLiveWrites` would exit 0 having validated none of
  what this tool claims to validate.

`Get-EngineLegVerdict` in `tools\ci\PrLocalValidation.psm1` therefore grades each leg from
its `run-summary.json` rather than trusting the exit code:

| Verdict | Exit code | Trigger |
| --- | --- | --- |
| `pass` | 0 | Live writes on, every required stage green, every required stage covered real tests. |
| `incomplete` | 2 | A required GUT host or the parse gate was missing or skipped, a host discovered no tests, a host passed none, or the orchestrator passed no scenarios. |
| `fail` | 1 | `overall_status` was not `pass`, or a required stage failed. |
| `error` | 1 | No summary, an unreadable summary, or a leg that did not actually run live writes. |

The worst leg wins. `incomplete` is reported as `incomplete`, never rounded up to a pass.

## Artifacts

```
%LOCALAPPDATA%\godot-gdk-pr-validation\pr-<n>-<sha12>-<timestampZ>\
  checkout\                         fresh detached clone at the PR head
  engines\<version>\                hash-verified, run-private Godot
  results\<version>\run-summary.json
  results\<version>\mp-orchestrator\
  logs\                             per-step transcripts
  validation-manifest.json          machine-readable result
  report.md                         the body posted to the PR
  SANDBOX-RESTORE.txt               present only if restoration failed
```

Report from `validation-manifest.json`, not from console scrollback.

## Fixed configuration

These are deliberately constants in the script rather than parameters, so a run cannot be
quietly pointed somewhere else:

| Setting | Value |
| --- | --- |
| Repository | `microsoft/XBOX-Godot-Sample` |
| PlayFab title | `10D176` (custom id `godot-gdk-ext-live-smoke`, queue `godot_gdk_ext_live_smoke_queue`) |
| Xbox sandbox | `lykhvw.0`, restored to whatever was there before |

The PlayFab profile is the same non-secret test profile `tools\configure_playfab_test_title.ps1`
uses. Provisioning that title remains a **separate, explicit** operation — validation never
provisions, and never needs `PLAYFAB_DEVELOPER_SECRET_KEY`.

## Limitations

- **The mutex is machine-local.** It cannot serialize against the nightly `playfab-live`
  workflow or another developer's machine. Confirm the nightly is not mid-run before
  starting.
- **This does not cover console targets, packaging, or submission.** The report says so
  explicitly under "Not covered by this run"; keep that section honest.
- **Only title `10D176` is exercised.** Nothing here says anything about any other title.
- **Sandbox switching is disruptive.** It restarts Xbox Live Auth Manager, so expect
  Xbox-signed-in apps on the machine to drop.

## Shared pieces

| File | Role |
| --- | --- |
| `tools\validate_pr_local.ps1` | The wrapper. Local-only; never runs in CI. |
| `tools\ci\PrLocalValidation.psm1` | Pure verdict, Markdown-safety, and report logic. |
| `tools\ci\GodotAcquisition.psm1` | Pin/verify/extract policy shared with the `setup-godot` action. |
| `tools\ci\get_godot.ps1` | `-Mode Pin\|Install\|Resolve` CLI over the acquisition module. |
| `tools\ci\tests\pr_local_validation.test.ps1` | Offline contract tests, run by the `ci-lint` job. |

Because `GodotAcquisition.psm1` and `get_godot.ps1` back the `setup-godot` action, the PR
gate selector routes them to every Godot consumer. The validation wrapper, its module, and
its tests are exempt from the addon gates — the always-on `ci-lint` job parses them and
runs their offline tests.
