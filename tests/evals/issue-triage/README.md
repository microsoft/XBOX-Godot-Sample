# Issue triage evals

Evals for the [`issue-triage` skill](../../../.github/skills/issue-triage/SKILL.md),
the prompt that both `/triage` and local Copilot CLI use. Each case is a real
issue pinned to the commit it was filed against. A person scores each run
against [`rubric.md`](rubric.md). No model grades another model.

A valid report is **not** the same as a good report. The harness checks the
schema and citations automatically. A run counts as a quality pass only after
a reviewer scores it.

The eval workflow uses the same network allow list and `web-fetch` tool as
`/triage`, so the agent can read `devdocs.xbox.com` and `learn.microsoft.com`.
Those pages are live, not pinned to the case commit, so a rerun of the same
case can see different documentation. Reviewers should check that any
`doc_references` actually support the assessment.

## Layout

```text
tests/evals/issue-triage/
  rubric.md                      scoring rules
  cases/<case-id>/
    case.json                    target_sha, justification, fixture_digest
    issue.json                   frozen issue title, body, and comments
    expectations.json            gold data: acceptable kinds, evidence, notes
```

Gold data (`expectations.json`, `rubric.md`) is never staged for the model.
`prepare` writes only the pinned source snapshot and the issue context. The
Actions eval workflow deletes `tests/evals` from its workspace before the agent
starts.

The harness is `tools/ci/issue_triage_eval.cjs`. Run it with no arguments to
print usage.

## Running a case locally

This repo's local clone may name its remote something other than `origin`.
Pass `--remote <name>` if so.

1. Prepare a run directory. The directory must be empty or not exist yet.

   ```powershell
   node tools\ci\issue_triage_eval.cjs prepare --case <case-id> --out $env:TEMP\tri-eval\<case-id> --model <model>
   ```

   This writes `source\` (the repo at `target_sha`, plus the candidate
   `SKILL.md`), `context\context.md`, `prompt.txt`, and `run.json`.

2. Start Copilot CLI with `source\` as the working directory and allow read
   access to `context\`. Send the contents of `prompt.txt` as the prompt.
   Copy the reply into `report.md` in the run directory. The harness accepts
   raw JSON or a reply that ends with a fenced `json` block.

3. Check the report, then create a scorecard:

   ```powershell
   node tools\ci\issue_triage_eval.cjs validate-report --run $env:TEMP\tri-eval\<case-id>
   node tools\ci\issue_triage_eval.cjs scorecard --run $env:TEMP\tri-eval\<case-id>
   ```

4. Read the case's `expectations.json` and `rubric.md`, then fill in
   `scorecard.json`: reviewer, one 0–2 score per dimension, and
   `critical_failure` (`null` or `{ "reason": "..." }`).

5. Score it. Pass several `--run` options to get a suite verdict once every
   case is covered. `prepare` records the case's `target_sha` and
   `fixture_digest` in `run.json`, plus digests of `expectations.json` and
   `rubric.md`. `score` rejects a run when any of these no longer match the
   checked-in case, so an old report is never graded against changed
   criteria; re-run `prepare` after changing a case or the rubric. It exits 0
   only when every case has a `quality-pass` run, so scoring a single case
   exits 1 even when that case passes; read the per-run status for
   single-case checks.

   ```powershell
   node tools\ci\issue_triage_eval.cjs score --run $env:TEMP\tri-eval\<case-id>
   ```

   Each run gets a status of `quality-pass`, `quality-fail`,
   `pending-human-review`, `invalid-report`, or `infra-or-model-failure`, which is
   written to `result.json`.

## Running a case in Actions

Dispatch **Issue Triage Eval** (`.github/workflows/issue-triage-eval.md`) from
the Actions tab and pick a case. It runs the same skill in Actions mode against
the pinned snapshot. It never comments on issues. The run uploads an
`issue-triage-eval-<case-id>` artifact containing `run.json`, `report.json`,
and `context/`. Its `run.json` `model` field is the agent job's model selector
(`GH_AW_MODEL_AGENT_COPILOT`, then `GH_AW_DEFAULT_MODEL_COPILOT`, then `auto`).

To score it, download and extract the artifact, then run steps 3–5 above
against the extracted directory. `validate-report` and `score` rebuild
`source\` from `target_sha` when it is missing.

gh-aw generates a `conclusion` job with `issues: write` in every agentic
workflow, including this one. The eval agent declares no GitHub tools, and its
only safe output writes to the job summary. Enabling `web-fetch` does leave
Copilot's built-in `github-mcp-server` reachable (see the note in
`docs\ci\issue-triage.md`), but the eval job's own token is read-only.

## Adding a case

1. Pick a closed or well-understood issue. Choose the commit the reporter
   was on (or the closest one before the fix) and record why in
   `sha_justification`.
2. Create `cases/<case-id>/` with `issue.json`, `expectations.json`, and a
   `case.json` whose `fixture_digest` is empty. Evidence locations must exist
   at `target_sha`.
3. Run `validate-fixtures --case <case-id>`. The error shows the `issue.json`
   digest. Copy it into `fixture_digest` and run it again until it passes.
4. Add the case id to the `case` choice list in
   `.github/workflows/issue-triage-eval.md`, then recompile:

   ```powershell
   gh aw compile issue-triage-eval --validate --strict
   ```

`issue-triage-checks.yml` runs `validate-fixtures` for every case on each
relevant pull request.
