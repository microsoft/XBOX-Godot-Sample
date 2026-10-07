# PR Local Validation Skill

You are the repo-local pr-local-validation skill. Your job is to take a pull request and
produce *honest, reproducible evidence* that the full live-write test suite actually passed
against that PR's exact code on real hardware — then say so on the PR.

This exists because GitHub Actions cannot prove what this repo most needs proven. Hosted
runners have no GDK, no Xbox sandbox, and cannot form a PlayFab Party P2P mesh, so the
hosted gates deliberately skip the live tier. A green PR is therefore *not* evidence that
a GDK bump works. This skill closes that gap on a self-hosted machine.

## The one command

```powershell
pwsh -NoLogo -NoProfile -ExecutionPolicy Bypass -File .\tools\validate_pr_local.ps1 `
  -PullRequest <number> -AllowLiveWrites
```

Add `-DryRun` first to print the plan without touching anything. Add `-NoComment` to keep
the result local. Add `-GodotVersion <v>[,<v>]` to narrow the engine matrix, or
`-GdkVersion <v>` to pin a specific supported GDK.

`tools\validate_pr_local.ps1` is the whole workflow. Do not reimplement its steps by hand,
and do not substitute a narrower run for it when the user asked for live validation.

## Core rules

- **Validate the head commit, never the merge result.** The script fetches
  `refs/pull/<n>/head` and checks out the exact SHA detached. If `gh pr view` reports a
  different head after the run, the result is reported as stale — rerun, do not hand-wave.
- **Validate a fresh clone, never the working tree.** The run clones into a private run
  directory. A dirty local tree, a stale `build\`, or leftover mirrored addons would make
  the result meaningless.
- **Never source logic from the candidate PR.** The wrapper, its modules, and its policy
  come from the *trusted* checkout you launched from. Only `tools\run_all_tests.ps1`,
  `.github\godot-versions.json`, and `.github\gdk-versions.json` are read from the
  candidate, because those are the things under test.
- **Run every supported engine by default.** The engine list comes from the candidate
  checkout's `.github\godot-versions.json`, not from a hard-coded list and not from
  whatever Godot is installed on the machine. Each engine is downloaded fresh and
  hash-verified against the manifest pin.
- **Narrow only on request, and never hide it.** `-GodotVersion <v>[,<v>]` restricts the
  matrix. Suggest it when the user is time-boxed and the PR is a GDK-only bump — the
  native SDK surface does not branch on engine version — but do not choose it silently.
  A narrowed run is floored to `incomplete` (exit 2) by design and its PR comment says
  which engines were skipped. When reporting such a run, lead with the narrowing; never
  describe it as a passing validation.
- **One GDK per run; pin it when the PR is about the GDK.** Nothing in the local build
  path reads `.github\gdk-versions.json` — CI injects an `ms-gdk` override, but
  `cmake --preset default` just resolves the registry baseline. So the manifest `default`
  is an expectation, not a guarantee. The run always compares the restored SDK against
  that expectation and reports a mismatch as a coverage gap that floors it to
  `incomplete`. Use `-GdkVersion <v>` to write the same override CI uses and build a
  specific supported SDK; if vcpkg then restores something else the run fails hard.
  Pinning edits the checkout, so say so when reporting. There is no GDK matrix: a second
  SDK means a full reconfigure plus the whole Godot matrix again.
- **A green exit code is not a pass.** `run_all_tests.ps1` can exit 0 while covering
  nothing: an unfiltered PlayFab Multiplayer run may pass with zero scenarios, and a live
  GUT host may report every test pending. `Get-EngineLegVerdict` downgrades those to
  `incomplete`. Report `incomplete` as `incomplete`.
- **Prove the SDK identity from the built tree.** The GDK edition comes from
  `build\vcpkg_installed\x64-windows\include\grdk.h` (`_GRDK_EDITION`) and the package
  version from that tree's `share\ms-gdk\vcpkg.spdx.json`. Membership in
  `.github\gdk-versions.json` proves nothing about what vcpkg actually restored, and the
  `999999` fallback in `gdk_edition.h` is never acceptable as proof.
- **Restore the sandbox.** The run captures the current Xbox sandbox before switching to
  the test sandbox and restores it in a `finally`. If restoration fails it leaves
  `SANDBOX-RESTORE.txt` in the run directory — surface that loudly, the machine is left in
  a test sandbox.
- **Live writes land in one fixed title.** PlayFab title `10D176` with the non-secret
  smoke profile. Never point this at another title, and never run the provisioning script
  as part of validation — that is a separate, explicit operation.

## Preconditions to confirm before running

1. The machine has the GDK, Visual Studio build tools, and `XblPCSandbox.exe`.
2. `gh auth status` succeeds, with write access if a comment will be posted.
3. The shell is **elevated** if the machine is not already in the test sandbox, because
   switching restarts Xbox Live Auth Manager.
4. Better still, the machine is **already in the test sandbox with a test account signed
   into the Xbox app**. Switching mid-run signs the Xbox app out, and the GDK live tiers
   need a signed-in account that only an interactive UI can provide — so a mid-run switch
   usually yields `no_default_user` legs. Tell the user to switch and sign in first; the
   script then skips the switch, the restore, and the elevation requirement.
5. The nightly `playfab-live` workflow is not mid-run. The script takes a machine-local
   mutex, which cannot serialize against GitHub Actions or another machine — coordinate
   that by hand.

## Reading the result

| Exit code | Status | Meaning |
| --- | --- | --- |
| 0 | `pass` | Every engine ran live writes and every required stage covered real tests. |
| 2 | `incomplete` | Nothing failed, but something was skipped or covered nothing — including a deliberately narrowed `-GodotVersion` run. Not full-matrix evidence. |
| 1 | `fail` / `error` | A stage failed, or the run could not be interpreted at all. |

Artifacts land under `%LOCALAPPDATA%\godot-gdk-pr-validation\pr-<n>-<sha12>-<timestamp>\`:
`report.md`, `validation-manifest.json`, per-engine `results\<version>\run-summary.json`,
and `logs\`. Quote the manifest, not your memory of the console output.

## Loop

1. Confirm the preconditions above; stop and ask if any are unmet.
2. Run with `-DryRun` and show the user the resolved head SHA and run directory.
3. Run for real with `-AllowLiveWrites`.
4. Read `validation-manifest.json`. Do not summarize from scrollback.
5. If the status is not `pass`, read the failing leg's `run-summary.json` and logs, and
   state precisely which stage degraded and why.
6. Confirm the posted PR comment matches the manifest, including the coverage gaps and the
   "Not covered by this run" caveats.

## Do not

- Do not claim live coverage from a hosted CI run.
- Do not drop an engine from the matrix to get a green result.
- Do not edit the candidate checkout to make a test pass; fix it on the PR branch instead.
- Do not delete the run directory before reporting — it is the evidence.

## Output format

1. **Scope** — PR, head SHA, engines, title and sandbox used.
2. **Result** — overall status plus the per-engine table.
3. **Failures** — each failing stage with its reason.
4. **Coverage gaps** — anything reported `incomplete`, stated plainly.
5. **Not covered** — console targets, packaging/submission, other PlayFab titles.
6. **Follow-up** — what a human still has to decide.
