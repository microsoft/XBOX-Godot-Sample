# Issue triage (`/triage`)

A maintainer can ask for an AI first-pass evaluation of an issue by commenting
`/triage` on it. An agentic workflow reads the issue, analyzes it against the
default-branch code, and posts one comment that summarizes the issue, points at
the relevant code, and lists open questions and suggested next steps. See
[Posting and staged mode](#posting-and-staged-mode) to switch to a
job-summary-only preview.

The report is a **starting point for a human**, not a decision. It is static
analysis only: the agent never builds, runs, or reproduces anything, and it
does not label, assign, close, or edit the issue.

## Files

| File | Role |
| ---- | ---- |
| `.github/skills/issue-triage/SKILL.md` | **Canonical triage prompt.** Analysis procedure, report contract, and the rules for Actions and local modes. The workflows import it at run time. |
| `.github/workflows/issue-triage.md` | [GitHub Agentic Workflow](https://github.github.com/gh-aw/) source: trigger, permissions, tools, a thin Actions adapter that imports the skill, and the publisher job. |
| `.github/workflows/issue-triage.lock.yml` | Compiled workflow that Actions actually runs. Generated; never hand-edit. |
| `.github/workflows/issue-triage-eval.md` / `.lock.yml` | Manually dispatched eval workflow. It runs the skill against one pinned eval case and never comments. |
| `.github/aw/actions-lock.json` | Action pins recorded by the gh-aw compiler. Generated. |
| `tools/ci/issue_triage.cjs` | Deterministic helper: eligibility gate, context preparation, report validation, citation checks, Markdown rendering, and publishing. |
| `tools/ci/issue_triage_eval.cjs` | Eval harness: fixture validation, run preparation, report validation, and scoring. |
| `tools/ci/tests/issue_triage*.test.cjs` | `node:test` suites for the helper and the harness. |
| `tests/evals/issue-triage/` | Eval cases, rubric, and [eval instructions](../../tests/evals/issue-triage/README.md). |
| `.github/workflows/issue-triage-checks.yml` | PR/push checks: tests, eval fixture validation, and lock-file drift for both workflows. |

## Using it

Comment exactly `/triage` (nothing else in the comment) on an open issue. The
workflow runs only when **all** of these hold:

- The comment was just created (edits do not trigger), on an issue rather than
  a pull request.
- The commenter has `admin`, `maintain`, or `write` permission on the repo and
  is not a bot. The permission is re-checked live, not taken from the event.
- The issue is open, is not a JIT access request (`jit` label or a title
  starting with `JIT request`), and carries none of the labels `security`,
  `security-sensitive`, or `vulnerability`.
- The trigger comment still exists, unchanged, when the workflow starts.

Anything else is skipped quietly. gh-aw's built-in role check may skip the run
before the gate starts. Otherwise, the gate writes the skip reason as a notice
annotation on the run.
To re-run triage after the issue changes, post a new `/triage` comment. Each
request comment produces at most one report.

## Running triage locally

The same skill works in an interactive Copilot CLI session in this repo. Ask
for it by name, for example "triage issue #123" or "use the issue-triage
skill on this issue", and paste the issue text if the session cannot fetch it.
If the session was already open when the skill changed, run `/skills reload`.

Local mode is display-only. The skill never posts, labels, or edits anything.
Its output differs from the workflow's report:

- It analyzes the **current checkout**, not the default branch, and states the
  branch, SHA, and whether the tree has uncommitted changes.
- Citations of modified or untracked files are labeled "local only" and get no
  permalink.
- The workflow's validator does not run, so the report is unvalidated.

## How it works

1. **Gate** (pre-activation job, read-only token). `runGate` re-reads the
   issue, the trigger comment, and the commenter's permission, and emits
   `eligible`. Unexpected API errors fail the run instead of skipping.
2. **Context** (agent job). `prepareContext` re-reads the issue and fails the
   run before writing anything if it has since closed or gained a JIT or
   security label, so that content never reaches the agent. Otherwise it writes
   `/tmp/gh-aw/agent/triage/context.md`. It fences the labels, title, body, and earlier
   comments as untrusted text and keeps them within size budgets. It also writes
   `context.json`, which records the analyzed SHA, the issue and comment ids, and
   a digest of the issue content.
3. **Agent**. The Copilot engine reads the context and the checked-out
   workspace, following the skill's Actions mode. It has no shell, no editing,
   and no declared GitHub tools, and runs behind the gh-aw network firewall. Its
   only declared network tool is `web-fetch`, and the firewall allows only the
   gh-aw `defaults` set plus `devdocs.xbox.com` and `learn.microsoft.com`, so it
   can read GDK and PlayFab documentation. The skill forbids fetching URLs taken
   from the issue. The workflow's `network.allowed` list and `DOC_HOSTS` in
   `tools/ci/issue_triage.cjs` must stay in sync. It must call `post_triage_report` exactly once
   with a JSON report that matches the schema in the prompt.

   Note that enabling `web-fetch` makes gh-aw omit `--disable-builtin-mcps` from
   the compiled harness, because Copilot CLI serves `web_fetch` from its built-in
   tool schema. That same flag also gates the built-in `github-mcp-server`, so the
   agent can reach GitHub's API even though the workflow declares `github: false`.
   This is accepted rather than fixed: the job's token is read-only
   (`contents: read`, `issues: read`), the agent cannot write to the repository,
   and every report still passes through the validation and publish gates below.
   The alternative, gh-aw's `copilot-sdk` engine mode, restores the flag but moves
   the agent onto a newer execution path that this workflow has not exercised.
4. **Validation** (agent post-step). `validateAgentOutput` fails the run if the
   report is missing, malformed, flagged `security_sensitive`, cites a path
   or line range that does not exist at the analyzed commit, or lists a
   `doc_references` URL that is not plain HTTPS on a `DOC_HOSTS` host (no
   query string, credentials, or port).
5. **Threat detection**. This is the standard gh-aw detection job.
6. **Publish** (`post-triage-report` job, the only job that posts the report).
   `publish` validates everything again and re-checks eligibility. It confirms
   that the issue content digest and SHA still match `context.json`, then
   renders the report. All model text is escaped: no HTML, links, mentions,
   issue references, or headings. Citations become permalinks pinned to the
   analyzed SHA. Validated `doc_references` are the only other links; they are
   listed under "Documentation". Before posting, it looks for an existing report for the same
   request and skips if it finds one.

gh-aw also generates a `conclusion` job with `issues: write` in every agentic
workflow. It is compiler-owned and only handles gh-aw run bookkeeping; this
workflow disables its failure issues and status comments (see
[Failure behavior](#failure-behavior)). So two jobs hold `issues: write`:
`post-triage-report` and `conclusion`. The agent job has only `issues: read`.

Each report carries a hidden marker:

```text
<!-- xbox-godot-issue-triage request=<repo-id>:<issue>:<comment-id> sha=<sha> -->
```

Deduplication trusts only markers in comments authored by
`github-actions[bot]`. Set the `TRIAGE_BOT_LOGIN` env var on the publish step
if the workflow ever posts under a different identity.

## Failure behavior

The workflow fails closed. If anything is wrong, the run fails visibly in the
Actions tab and **nothing is posted**. That includes an invalid report, a bad
citation, a security flag, a digest mismatch, the issue being edited or closed
mid-run, or the trigger comment being edited or deleted. gh-aw's automatic
failure issues and status comments are disabled, so failures do not create
noise on the issue. The requesting maintainer can post `/triage` again.

## Security-sensitive issues

Vulnerabilities must be reported privately. See [`SECURITY.md`](../../SECURITY.md).
Issues labeled as security-sensitive are never triaged. If the agent itself
concludes an issue looks security-sensitive, the run fails and nothing is
posted. A maintainer should then follow the `SECURITY.md` process by hand.

## Posting and staged mode

The workflow runs with `TRIAGE_MODE: post` on the "Publish triage report" step
of `.github/workflows/issue-triage.md`, so each valid report is posted as an
issue comment. Any value other than exactly `post` (for example `staged`)
renders the report into the run's job summary and never comments on the issue.

To pause posting, set the value to `staged`, then recompile and commit both
files:

```powershell
gh aw compile issue-triage --validate --strict
```

To turn the workflow off entirely, disable **Issue Triage** in the Actions tab.

## Prerequisites

- **Copilot billing.** The agent job authenticates with the workflow's
  `GITHUB_TOKEN` using the `copilot-requests: write` permission. The
  organization must allow Copilot usage from GitHub Actions for this repo.
  Otherwise the agent step fails at startup.
- **gh-aw CLI** (maintainers editing the workflow only).
  `gh extension install github/gh-aw --pin <version>`. Use the version recorded
  in the header of `issue-triage.lock.yml`, which is also `GH_AW_VERSION` in
  `issue-triage-checks.yml`. When you upgrade, update both and recompile.

## Editing and validating

Change triage behavior in `.github/skills/issue-triage/SKILL.md`. Edit the
workflow `.md` files only for Actions-specific wiring. The workflows import the
skill at run time, so a skill-only change needs no recompile. It does change
what both Actions and local sessions do, so run the evals first.

```powershell
# Helper and harness unit tests
node --test .\tools\ci\tests\issue_triage.test.cjs .\tools\ci\tests\issue_triage_eval.test.cjs

# Eval fixtures (add --remote <name> if your remote is not origin)
node .\tools\ci\issue_triage_eval.cjs validate-fixtures

# Recompile after editing either workflow .md
gh aw compile issue-triage --validate --strict
gh aw compile issue-triage-eval --validate --strict
```

`issue-triage-checks.yml` runs the tests and fixture validation, and fails if
either committed lock file differs from a fresh compile. Never hand-edit a lock
file: change the `.md` and recompile.

## Evals

Prompt changes are judged against real issues pinned to specific commits.
Run them locally or dispatch **Issue Triage Eval** from the Actions tab, then
score each run by hand against the rubric. CI checks only the deterministic
parts, never model quality. A report that passes the validator is not
necessarily a good one; only a scored run can be a quality pass. See
[`tests/evals/issue-triage/README.md`](../../tests/evals/issue-triage/README.md).

## Pilot checklist

Confirm the following after deploying or changing the workflow:

- [ ] A `/triage` from a maintainer posts one report comment with working
      permalinks and sensible findings. Re-running that run does not post a
      duplicate; a new `/triage` comment posts a new report.
- [ ] `/triage` from a non-collaborator, on a pull request, on a closed issue,
      on a JIT request, and on a security-labeled issue is skipped, with the
      reason in the run's notice
      annotation or the gh-aw role check.
- [ ] Editing or deleting the trigger comment right after posting it causes a
      skip or a visible failure, never a report.
- [ ] The permission lookup (`getCollaboratorPermissionLevel`) succeeds with
      the workflow token.
- [ ] The agent can read and search files with shell and editing disabled.
- [ ] Copilot requests are billed as expected, and the run stays within the
      configured `max-ai-credits` and `timeout-minutes`.
- [ ] A deliberately broken report (for example, a bad citation) fails the run
      without posting.
