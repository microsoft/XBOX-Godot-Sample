# GDK release watch

Microsoft publishes new GDK releases to [microsoft/GDK][gdk-releases] on its own
cadence. This automation notices them, asks a read-only AI agent what the release
means for this repository, and files a tracking issue holding that assessment and
a brief describing the work it implies.

The assessment is **advice, not a patch**. It names the files to change and the
questions to answer; it never derives exact values, never opens a branch, and
never opens a pull request. Assign the tracking issue to GitHub Copilot or pick
it up yourself, re-check the report against current `main`, and produce a draft
pull request for a human to validate against a real SDK.

## Why a schedule and not a release trigger

A `release` event fires in the repository that published the release. A workflow
in this repository cannot subscribe to events in `microsoft/GDK`, so the watcher
polls instead: weekly on Monday, plus manual dispatch whenever you want an
answer sooner.

## Files

| File | Role |
| ---- | ---- |
| `.github/workflows/gdk-release-watch.yml` | Scheduled + manually dispatched watcher. Finds unsupported releases, opens or updates tracking issues, and dispatches the assessor. |
| `.github/workflows/gdk-release-assess.md` | [GitHub Agentic Workflow](https://github.github.com/gh-aw/) source for the assessor: trigger, permissions, tools, the agent instructions, and the publisher job. |
| `.github/workflows/gdk-release-assess.lock.yml` | Compiled workflow that Actions actually runs. Generated; never hand-edit. |
| `tools/ci/gdk_release_watch.cjs` | Deterministic discovery: release parsing, edition math, support state, backlog selection, and the tracking-issue ledger. |
| `tools/ci/gdk_release_assess.cjs` | Deterministic assessment half: evidence bundle, report validation, consistency rules, rendering, and publishing. |
| `tools/ci/tests/gdk_release_*.test.cjs` | `node:test` suites for both helpers. |
| `.github/workflows/pr-gates.yml` (`gdk-watch` job) | PR/push checks: helper tests and lock-file drift, selected by `tools/ci/pr_gate_scope.cjs` and aggregated into the required `PR gates` check. |

The agent is read-only. It cannot edit files, open issues, or open pull
requests; it emits one JSON report and the helpers above decide what is
published. The release notes it reads are untrusted text, fenced and labelled as
data in the evidence bundle.

Its egress allowlist is exactly `learn.microsoft.com` and `devdocs.xbox.com`.
Unlike `issue-triage.md`, this workflow deliberately omits gh-aw's `defaults`
host bundle: that bundle adds roughly three dozen apt, snap, package-registry
and certificate-revocation hosts, and this agent has no shell and installs
nothing, so those hosts would only widen the exfiltration surface available to a
prompt-injection payload hidden in upstream release notes. `doc_references`
hosts are validated a second time in `tools/ci/gdk_release_assess.cjs`, so a
cited URL outside those two hosts fails publication even if the fetch somehow
succeeded.

## What counts as a release worth tracking

A release is queued only when all of these hold:

- The tag parses as a GDK version (`...-v<YYMM>.<update>.<build>`), and the
  release is published and not a pre-release.
- Its edition is at or above the minimum edition in
  `tools/ci/gdk_release_watch.cjs`. **The October 2025 (`2510`) family is
  deliberately below the floor**: those editions lack GDK features the addons
  depend on, so adding one is a real porting job rather than a supported-list
  change. Editions already pinned in the supported lists keep working; the
  watcher just never proposes a new `2510` edition.
- Its edition is not already in `GDK_SUPPORTED_VERSIONS` in
  `cmake/GDKDependencies.cmake`, which is the definition of "supported".

Everything else is skipped with a reason recorded in the run summary.

## What the watcher does

1. Lists every release in `microsoft/GDK`, paginating — the list is **not**
   ordered by version, so `/releases/latest` and watermarks are both unsafe.
2. Partitions them into supported, unsupported, and ignored against this
   repository's checked-in support state.
3. Opens a tracking issue, labelled `gdk-release`, for **every** unsupported
   release that does not have one yet, holding the release identity and the
   current support configuration. The label is created first if the repository
   does not have it yet — the queue is found *by* that label, so an unlabelled
   issue would be re-created on every later run.
4. Dispatches `gdk-release-assess.lock.yml` for the oldest release that has no
   assessment yet.

Issues are cheap, so the watcher files all of them; assessments cost a model run,
so it queues **one** per run. A backlog therefore drains one assessment per week
unless you dispatch it manually with `drain_backlog`.

Tracking issues are the durable queue: a bot-authored issue carries the release
id and the last assessment status in bot-authored state comments.

### Reports are snapshots

A published report describes this repository as it stood at one commit, named in
the report. It is never refreshed automatically — not when `main` moves, not when
the upstream release notes are edited, not on the next weekly run. A release that
already has a terminal status (`changes-required`, `tests-only`, `needs-review`,
`assessment-failed`) is skipped by every later run.

That is deliberate. A report is read once, by whoever picks the issue up, and
the brief tells them to re-check it against current `main` before acting. Paying
for a fresh model run every time an unrelated file changes would buy a fresher
timestamp and no new decision.

When you *do* want a fresh read — the repository has moved materially, or the
earlier report was wrong — ask for one explicitly: run **GDK Release Watch** with
`release_tag` set to that release and `retry` enabled. The new report is posted
alongside the old one, so the history of what was decided and when stays on the
issue. Closing the tracking issue is how you tell the watcher to drop a release
for good: a closed issue is never reassessed, retry or not.

### When an assessment does not come back

If an assessor run dies without posting a report — a rejected dispatch, a crashed
agent, a blocked safe output — its ledger entry would otherwise read
`assessment-dispatched` forever. The watcher treats such an entry as in flight
for six hours; past that it writes an `assessment-failed` state closing the dead
attempt out, and raises a warning on the run naming the exact retry to run.

It does **not** re-queue the release. An attempt that died on this release would
keep dying on it, unattended, every week, and each death costs a model run.
Recovery is a deliberate act: re-run **GDK Release Watch** with `release_tag` and
`retry: true`. Both the warning and the `assessment-failed` comment spell that
command out, so no internal identifier has to be copied anywhere.

A dispatch that fails outright is recorded the same way before the error is
re-raised, so a failed run is always visible in both the Actions log and the
tracking issue.

### Attempt ids

Each dispatch carries an **attempt id** — the watcher run *and run attempt* that
queued it — as a workflow input. Both halves matter: re-running a watcher
workflow preserves `GITHUB_RUN_ID` and only increments `GITHUB_RUN_ATTEMPT`, so a
run-id-only key would hand the re-run the previous attempt's identity. The
assessor never re-derives that id from the ledger, because a retry re-queues the
same release under a new watcher run: a slow assessor reading the ledger at
publish time would otherwise adopt the retry's id and settle it with an older
report. An assessor that finds a different attempt in flight, or finds the
release already settled, refuses to publish and lets the retry win. Both terminal
states preserve the queuing watcher run and record the assessor run separately as
`assessorRunUrl`, so re-running a finished assessor recognises its own report
instead of posting a duplicate.

Because the attempt id cannot be reconstructed, dispatching the assessor
*directly* requires copying the `attempt` value out of the in-flight state
comment on the tracking issue; a run that omits it renders its staged preview and
then refuses to post. Going through the watcher — the supported path — never
requires that, and never puts two attempts in flight on one release: a retry
aimed at a release that is still being assessed is refused with a warning naming
the live run. GitHub comments have no compare-and-swap, so two concurrent
attempts would race between the final ledger read and the write; keeping one
attempt in flight at a time is what makes that race unreachable. The six-hour
staleness sweep above is what releases the next attempt when one dies.

### Manual dispatch

Run **GDK Release Watch** from the Actions tab:

| Input | Effect |
| ----- | ------ |
| `release_tag` | Act on exactly this tag instead of the oldest release awaiting assessment. |
| `retry` | Assess again even though the release already has a report. Reports are snapshots, so this is the only way to refresh one. **Requires `release_tag`**: a retry spends a model run and overrides the terminal-state guard, so it has to name one release. The run fails fast if it is set alone. A retry is refused while an assessment is still in flight — wait for it, or for the six-hour staleness sweep to close it out. |
| `drain_backlog` | Dispatch an assessment for every release awaiting one, not just the oldest. |
| `preview` | Report only. No issue, comment, or dispatch is written. |

`preview` is the safe way to see what the watcher currently thinks; the summary
table lands in the run summary.

## What the assessor decides

The agent reads a prepared evidence bundle: the release identity, this
repository's current support configuration, the release notes delta against the
closest supported release, and the full notes. It reviews the addon surfaces,
the CMake/vcpkg wiring, and the packaging tooling, and returns one of:

| Classification | Meaning | What the brief asks for |
| -------------- | ------- | ----------------------- |
| `changes_required` | A concrete call site, build setting, or packaging flow has to change. Every required change cites a real file and line range. | Re-verify each cited location against current `main`, make the change, then add the edition to the support lists. |
| `tests_only` | Nothing in this repository needs to change; the release only needs to be added to the supported lists and validated. | Add the edition to the support lists only. |
| `needs_review` | The notes are ambiguous or the evidence is incomplete. | Close the named evidence gaps by hand before any support-list edit. |

Every verdict produces the same thing: one assessment comment on the tracking
issue, carrying the report and an implementation brief. No verdict produces a
branch, a diff, or a pull request.

`tests_only` is the weakest claim to make from release notes alone — it asserts
that nothing anywhere in the repository is affected — so it carries the strictest
bar. A report claiming `tests_only` is downgraded to
`needs_review` unless it has `high` confidence, zero required changes, zero
evidence gaps, at least three *distinct* reviewed areas (repeating one area
three times does not count), and at least one validation task.
The downgrade and its reason are shown in the comment — a model cannot talk its
way past it.

Truncation is enforced the same way, but from the other side. The context
builder records what it actually had to cut — the release notes, the
release-note delta, or the whole evidence bundle — into `context.json`, and the
publisher downgrades `tests_only` on that record alone. Asking the model to
report the gap would be asking the wrong witness: a breaking change in the tail
it never received is exactly the change it cannot warn about. A missing
comparison baseline is tracked separately and does **not** downgrade, because it
widens the delta rather than shortening it.

## The implementation brief

This repository does not allow GitHub Actions to create pull requests, and the
automation does not try to work around that. Every assessment comment ends with
**How to pick this up** — a self-contained brief, written to stand alone because
a coding agent may see nothing but the issue. Assign the tracking issue to GitHub
Copilot, or take it yourself.

The brief always says the same five things:

1. Re-read the upstream release and re-check the report against current `main`.
   The report names the commit it describes; if the repository has moved, derive
   the change from what is there now.
2. Do the work the verdict implies (the right-hand column of the table above).
3. Add the edition to the supported-version lists, keeping every existing entry:
   - `cmake/GDKDependencies.cmake` — the allowlist of editions an installed GDK
     may satisfy.
   - `.github/gdk-versions.json` — the hosted vcpkg matrix CI restores from.
   - `vcpkg-configuration.json` — the registry baseline that pins which port
     versions are resolvable.
   - `vcpkg.json` — the `ms-gdk` version override.

   The vcpkg files only change if `ms-gdk <version>` is actually published in the
   public vcpkg registry. The brief says to check; the automation does not look,
   because the answer at pick-up time is the only one that matters.
4. Open a **draft** pull request filling in `.github/PULL_REQUEST_TEMPLATE.md`,
   stating under **Validation** that nothing has been run.
5. Hand it to a maintainer to build and test against the real SDK.

The brief is deliberately a list of files and questions rather than a diff. Exact
values — which baseline, which override, whether the port exists — depend on the
repository at pick-up time, not on the snapshot the report describes. A generated
patch would look authoritative and be wrong whenever `main` had moved, and the
assignee has to open those files anyway.

It also tells the assignee, in as many words, that they **cannot** validate the
change: proving support means building on Windows against an installed GDK, which
no hosted agent can do. **Nothing in this automation ever runs a build or a
test.**

## Posting and staged mode

`GDK_ASSESS_MODE` in the publisher step of `gdk-release-assess.md` controls
writes. `post` (the default) comments on the tracking issue. Set it to `staged`
to render the assessment to the job summary and write nothing. The watcher's
equivalent is the `preview` dispatch input.

The assessor is independently dispatchable, so the publisher re-checks the
trusted context the watcher checks — target repository, `refs/heads/main` — and
stages instead of posting anywhere else. A fork or feature-branch run therefore
produces a job summary and nothing else, whatever `GDK_ASSESS_MODE` says.

The publisher needs `issues: write` and nothing more. Do not add `contents:
write`, `pull-requests: write`, a personal access token, or a GitHub App to make
the automation open the pull request itself. The change exists to be reviewed by
a human, and a token with more authority does not change that.

## Changing the supported floor

The minimum edition lives in `tools/ci/gdk_release_watch.cjs`. Raising it means
the watcher stops proposing older editions; it does not remove anything from
`GDK_SUPPORTED_VERSIONS`. Lowering it is only meaningful if the addons actually
work on the older family. Update this page in the same change.

## Local checks

```powershell
node --test tools/ci/tests/gdk_release_watch.test.cjs tools/ci/tests/gdk_release_assess.test.cjs
gh aw compile gdk-release-assess --strict
```

The compile step needs the pinned gh-aw version recorded in the `gdk-watch` job
of `.github/workflows/pr-gates.yml`; a mismatch shows up as lock-file drift in
CI.

[gdk-releases]: https://github.com/microsoft/GDK/releases
