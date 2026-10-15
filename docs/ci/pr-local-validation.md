# PR-local live validation

GitHub Actions cannot prove the thing this repo most needs proven. Hosted runners have no
GDK, no Xbox sandbox, and cannot form the PlayFab Party P2P mesh, so the hosted PR gates
run the **offline** tier only. A fully green PR is therefore not evidence that a GDK bump,
a PlayFab change, or an engine bump actually works against live services.

`tools\validate_pr_local.ps1` closes that gap. It pulls a pull request down to a fresh,
private checkout on a self-hosted Windows machine, builds it, runs the canonical
`tools\run_all_tests.ps1` live-write suite once per supported Godot version, builds
Release, and posts an honest result back on the PR.

## Contract

One invocation validates **one exact commit, one actual SDK and the selected Godot versions**.
By default every supported engine is selected; `-GodotVersion` selects a smaller scope.
`PASS` means that selected work succeeded, not that the entire PR or support matrix is approved.
Unselected engines and other SDKs are scope notes, not failures. `INCOMPLETE` means requested
work lacked evidence, not that the operator deliberately requested less.

The wrapper runs the existing test orchestrator and retains its results and logs. It does not
infer required coverage from changed files, certify every support-list entry, coordinate
concurrent runs or provide a security sandbox. Later commits require their own evidence.
The report's rerun command repeats the selection, resolving the PR head again.

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
| `-GodotVersion <v>[,<v>]` | Select engines; pass multiple versions as one comma-separated value for `pwsh -File`. Each must appear in the candidate's `.github/godot-versions.json` `supported` list. Selection alone does not prevent `PASS` or exit 0. |
| `-GdkVersion <v>` | Pin the GDK. The value must appear in the candidate's `.github/gdk-versions.json` `supported` list. Writes an `ms-gdk` `overrides` entry into the checkout's `vcpkg.json` before configuring. See [Choosing the GDK](#choosing-the-gdk). |
| `-NoComment` | Produce the artifacts but do not post to the PR. |
| `-WorkRoot <path>` | Override the run-directory root. Defaults to `%LOCALAPPDATA%\godot-gdk-pr-validation`. |

### Narrowing the engine matrix

By default the script runs every version in the candidate's
`.github/godot-versions.json` `supported` list, sequentially, at roughly 25-30
minutes per engine. The report always names the selected scope.

Each run builds exactly **one** GDK. There is no GDK matrix, and adding one is
not a cheap change: a second SDK means a full reconfigure plus the entire Godot
matrix again, so the wall-clock cost multiplies rather than adds.

For a GDK-only bump, the native GDK and PlayFab code does not branch on engine
version, so one engine usually buys most of the signal:

```powershell
pwsh -NoLogo -NoProfile -ExecutionPolicy Bypass -File .\tools\validate_pr_local.ps1 `
    -PullRequest 202 -AllowLiveWrites -GodotVersion 4.6.1-stable
```

A narrowed run can **pass with exit 0** when the selected work succeeds.
The report names the selected engines and lists unselected versions under
"Not covered by this run." It makes no full-matrix claim.

### Choosing the GDK

Nothing in the local build path reads `.github/gdk-versions.json` — that file
is consumed by CI, which injects an `ms-gdk` override into `vcpkg.json` before
configuring. Locally, `cmake --preset default` resolves `ms-gdk` from whatever
`vcpkg.json` and the `vcpkg-configuration.json` registry baseline happen to
select. Without a pin the actual restored SDK defines the scope; the declared
default is shown for information and does not affect the verdict.

Use `-GdkVersion` when validating a particular SDK. It writes the same
`ms-gdk` override CI uses into the checkout's `vcpkg.json` before configuring:

  ```powershell
  pwsh -NoLogo -NoProfile -ExecutionPolicy Bypass -File .\tools\validate_pr_local.ps1 `
      -PullRequest 202 -AllowLiveWrites -GdkVersion 2604.2.7849
  ```

  The value must be in the candidate's `supported` list. If vcpkg still
  restores something else, the run **fails hard** rather than attributing the
  results to an SDK that was never built — we asked for a specific SDK, so a
  silent substitution invalidates everything downstream.

Whether or not you pin, the run cross-checks the resolved `_GRDK_EDITION`
against the edition the candidate's manifest maps to that version, **whenever
the SDK that got restored is the one the manifest selected**. That catches a
wrong edition number in the PR itself, which is a defect on the unpinned path
too. Only the *substitution* check is pin-specific: an unpinned run resolving a
different version is the ordinary registry-baseline case, reported as the
actual SDK scope rather than a failure.

Pinning edits the checkout, so the run no longer matches the PR byte for byte;
the script warns when it does this, and the posted comment says `pinned by
request` instead of `inherited from the candidate checkout`.

Other supported SDKs are outside this invocation. The wrapper does not inspect
changed files or decide how many SDKs a support-list edit should validate.

### Prerequisites

- **A same-repository pull request.** The script refuses fork PRs outright, and there is no
  override. See [Trust boundary](#trust-boundary).
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

### Trust boundary

A run executes the candidate's `CMakeLists.txt`, `tools\run_all_tests.ps1`, freshly compiled
native binaries, and GDScript — all with your account, your `gh` credentials, and your
signed-in Xbox identity. The fresh clone is **workspace isolation, not a security sandbox**:
it stops one run clobbering another, and nothing more.

Two consequences:

- **Fork PRs are refused before anything is fetched.** `gh pr view` reports
  `isCrossRepository`, and the script throws on it. If you genuinely need to validate a fork
  contribution, review the diff first and push it to a branch in this repository.
- **Even a same-repository PR must be code you are willing to run.** Read the build and
  tooling changes before you start. Every candidate-controlled child process — both `cmake`
  invocations for Debug, each `run_all_tests.ps1` leg, and both `cmake` invocations for
  Release — is launched with `PLAYFAB_DEVELOPER_SECRET_KEY`, `GITHUB_TOKEN`/`GH_TOKEN`,
  their enterprise variants, and the ambient `LIVE_TESTS`/`LIVE_WRITE_TESTS` flags removed
  from its environment, and the variables are restored afterwards. The trusted `git`,
  `XblPCSandbox.exe`, and `gh` calls keep their credentials, because the publication step
  needs them. This is hygiene, not containment: the candidate still runs as you and can
  read the same `gh` config, credential manager, and PlayFab files you can.

Keep the shell **non-elevated** unless a sandbox switch forces otherwise (see above);
running the candidate's build as Administrator only widens the blast radius.

## What a run actually does

1. **Preflight** — tool availability, `gh` auth, and (only if a sandbox switch is needed)
   elevation.
2. **Resolve the PR** — `gh pr view` for the title, state, base, and head SHA.
3. **Checkout** — fetch `refs/pull/<n>/head` into a brand-new run directory and
   `git checkout --detach <sha>`, then init submodules and verify `HEAD` matches the SHA.
   The **head commit** is validated, never the merge result, so the evidence corresponds to
   a commit that actually exists on the branch.
4. **Build Debug** — `cmake --preset default` then `cmake --build --preset debug`.
5. **Prove the SDK identity** — read `_GRDK_EDITION` out of
   `build\vcpkg_installed\x64-windows\include\grdk.h` and the `ms-gdk` `versionInfo` out of
   that tree's `share\ms-gdk\vcpkg.spdx.json`. This is what vcpkg *restored*, as opposed to
   what `.github\gdk-versions.json` *claims*.
6. **Acquire engines** — read `supported` from the **candidate checkout's**
   `.github\godot-versions.json` and download each version through
   `tools\ci\get_godot.ps1`, verifying the pinned SHA-512 before extraction. Engines are
   installed run-private, so whatever Godot is installed on the machine is irrelevant.
7. **Switch the Xbox sandbox** — capture the current sandbox first and write
   `SANDBOX-RESTORE.txt` before changing anything.
8. **Run the live matrix** — for each engine, with `GODOT`/`GODOT_BIN`/`GODOT_CONSOLE`
   pointed at that engine and the working directory inside the candidate checkout:

   ```powershell
   pwsh -File tools\run_all_tests.ps1 -SkipBuild -Live -AllowLiveWrites `
     -PlayFabTitleId 10D176 -PlayFabCustomId godot-gdk-ext-live-smoke `
     -PlayFabMatchmakingQueue godot_gdk_ext_live_smoke_queue -OutDir <results>\<version>
   ```

   The C++ doctest binary is engine-independent, so it runs on the first leg only;
   subsequent legs add `-SkipDoctest`. Each summary must report the requested engine version;
   missing or mismatched identity makes that leg an error.
9. **Restore the sandbox** in a `finally`. If restoration fails, `SANDBOX-RESTORE.txt`
    stays behind with the command to run by hand, the manifest records
    `sandbox_restored: false` plus the error, the report says the machine is still in the
    test sandbox, and the run verdict is forced to `error`. The failure is deliberately
    *not* thrown: by this point the live writes have already landed, and losing the
    manifest, the report and the comment would discard hours of real evidence over a
    machine-state problem that says nothing about the candidate's code.
10. **Build Release** — `cmake --preset default-release` then
    `cmake --build --preset release`. This runs *after* the live matrix because builds
    mirror binaries into shared addon and test-host directories.
11. **Report** — write `validation-manifest.json` and `report.md`, then
    `gh pr comment --body-file` unless `-NoComment`.

If Debug configuration or compilation fails, the same reporting path publishes
the failure: the manifest records the failed Debug step, live tests and Release
as not run, and unresolved SDK identity as null (rendered as `unavailable`).
The Xbox sandbox is not changed, and no test coverage is claimed. A nonzero
CMake exit is `fail`; inability to execute the build is `error`. Build details
remain in `logs\build-debug.log`.

Exceptions after Debug also reach reporting with an `error` verdict: completed legs are
retained, uncompleted selected engines are named, and the failing step is recorded in
`logs\execution-errors.log`. Reproduction uses the selected inputs even if no tests ran or
the SDK could not be resolved. This applies once build execution begins; preflight and
checkout errors still stop before a validation report is available.

## Why a green exit code is not enough

`run_all_tests.ps1` can exit 0 while covering nothing that matters:

- An unfiltered PlayFab Multiplayer orchestrator run is allowed to pass with zero scenarios.
- A live GUT host can report every test as pending when the live gate is never reached.
- A leg invoked without `-Live`/`-AllowLiveWrites` would exit 0 having validated none of
  what this tool claims to validate.
- `overall_status` is derived from *failures only*, so a skipped `cpp-doctest` stage, or a
  host whose `tests\bootstrap\` directory is empty or missing, leaves the run green while
  this tool's report claims both were exercised.

`Get-EngineLegVerdict` in `tools\ci\PrLocalValidation.psm1` therefore grades each leg from
its `run-summary.json` rather than trusting the exit code:

| Verdict | Exit code | Trigger |
| --- | --- | --- |
| `pass` | 0 | Live writes on, every required stage green, every required stage covered real tests. |
| `incomplete` | 2 | A required GUT host, the parse gate, the native `cpp-doctest` stage, or a host's bootstrap runners were missing or skipped; a host discovered no tests; a host passed none, reported no passing count at all, or reported no failing count at all; or the orchestrator passed no scenarios. |
| `fail` | 1 | `overall_status` was not `pass`, a required stage failed — including a parse gate that ran and did not pass — a required GUT host reported a nonzero `failing` count, or `run_all_tests.ps1` itself exited nonzero. |
| `error` | 1 | No summary, an unreadable summary, a leg that did not actually run live writes, or a required GUT host reporting a negative `failing` count. |

The strictness is deliberate in four places that would otherwise read as green:

- `live` and `live_writes` must be real JSON booleans. PowerShell casts every non-empty
  string to `$true`, so a summary carrying `"live": "false"` would otherwise satisfy exactly
  the check that exists to demand live evidence. Anything else is an `error` leg.
- A GUT host that discovered tests but reported *no* passing count is `incomplete`, not
  `pass`. An absent tally is as unconfirmed as a zero one, and the report would print a zero
  under a `PASS` heading.
- The parse gate is graded from its own stage status, not only from `overall_status`. Those
  are written by the same run, so a stale or inconsistent summary must not be able to leave a
  failed parse gate inside a passing leg.
- A GUT host's `failing` count fails the leg on its own, independently of its stage status.
  The status field and the tallies beside it are separate evidence from the same file, and
  where they disagree the count is the harder of the two — without this, a summary carrying
  `"status": "pass"` next to `"failing": 1` would put a `PASS` heading above a nonzero
  `Failed` column. The reason text names the host and says the status disagreed. The same
  suspicion runs the other way: a host that discovered tests and then reported *no* `failing`
  count is `incomplete`, because an absent tally is as unconfirmed as a zero one and would
  otherwise print as a clean `0`. A negative count is `error` — it is not a result this tool
  can read, and it is not greater than zero, so a bare "did anything fail?" check would wave
  it through.

"Unreadable" covers more than a missing or truncated file. A summary that is valid JSON can
still be uninterpretable — a counter that is not a number, a `stages` value that is not a
list of records — and grading happens *after* the live writes have already landed, so an
exception there would skip the manifest, the report, and the PR comment for a run that
already touched the shared title. `Get-EngineLegVerdict` therefore never throws: any failure
to read the summary is preserved as an `error` leg naming the field it could not read, which
outranks every other status and fails the run loudly.

The summary is the primary evidence, but it is not the *only* evidence: the orchestrator's
own exit code is the one signal the file cannot carry — an orchestrator that dies after
writing a green summary leaves no trace in it. The wrapper passes that exit code to
`Get-EngineLegVerdict -OrchestratorExitCode`, and anything nonzero fails the leg outright.

The worst selected leg wins. `Get-RunVerdict` also accounts for the builds,
execution errors and sandbox restoration:

| Run-level condition | Effect |
| --- | --- |
| Debug build failed | `fail` (or `error` if it could not execute) |
| Release build failed | `fail` |
| A later execution step threw | `error`, retaining completed results |
| Release was not run after a successful Debug build | `incomplete` unless an error already explains it |
| The Xbox sandbox could not be restored | `error` |
| No engine legs ran at all | `error` |

`incomplete` exits 2 and concerns missing work *inside* the selected scope. A deliberately
unselected engine, a different unpinned registry SDK, or a later PR commit does not change
the result for the named commit and actual configuration. The script does not re-read the
PR head after testing or infer required coverage from a diff.

One thing a host-wide passing tally cannot prove is that the machine was signed in to Xbox.
`test_game_save.gd` and `test_presence.gd` mark a missing Xbox identity *pending* and still
pass their contract tests, and the GDK bootstrap exits 0 when sign-in does not succeed, so a
signed-out PC produces a leg that looks exactly like a signed-in one. The leg verdict
therefore reads an **optional** `xbox_signed_in` boolean from `run-summary.json`: `true`
records the leg as `signed-in` and changes nothing, `false` floors the leg to `incomplete`
with a gap naming the missing identity, and anything that is not a real JSON boolean is an
`error` leg for the same reason `live` and `live_writes` are. When the field is absent the
leg is recorded as `unreported` and the verdict is left alone — the summary is written by the
*candidate's* orchestrator, so grading its absence would permanently mark every branch that
predates the field as incomplete. Unreported legs are instead named in the report's "Not
covered by this run" list, so the gap is stated rather than assumed away.

`cpp-doctest` is asserted on the **first** engine leg only, because the wrapper deliberately
appends `-SkipDoctest` to later legs — the native suite does not depend on the engine.

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

- **Nothing stops two runs from overlapping.** There is no lock. Live writes go to a single
  shared PlayFab title and the sandbox switch is machine-wide, so a second local run, or the
  nightly `playfab-live` workflow, will interleave with this one and neither result is
  trustworthy afterwards. A machine-local mutex was deliberately not kept: it could not
  serialize against the nightly or another developer's machine anyway, so it would have
  implied a guarantee it could not make. Confirm the nightly is not mid-run, start one run
  at a time, and if a previous run was killed rather than allowed to exit, check the Xbox
  sandbox and that run's in-flight PlayFab state before starting another.
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
