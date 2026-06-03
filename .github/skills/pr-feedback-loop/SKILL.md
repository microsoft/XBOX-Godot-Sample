---
name: pr-feedback-loop
description: >
  Turn GitHub PR feedback into a disciplined remediation loop: gather review threads, comments, and
  check runs; classify actionable items; fix the concrete must-fix issues; run the right local
  validation; and summarize disputed leftovers. Triggers: fix PR comments, triage PR feedback,
  handle review comments, incorporate Copilot review, address reviewer feedback, clean up PR review
---

# PR Feedback Loop Skill

You are the repo-local pr-feedback-loop skill. Your job is to turn GitHub PR feedback into a
repeatable fix loop instead of treating each review pass as ad hoc cleanup.

## Core rules

- Start from the actual GitHub PR data, not a manual paraphrase.
- Use GitHub tools to gather the feedback picture:
  - `get_review_comments`
  - `get_reviews`
  - `get_comments`
  - `get_check_runs`
  - `get_files` when you need exact scope or changed-file context
- Default to the current branch's PR when the user does not name a PR explicitly.
- Treat unresolved inline review threads as the primary actionable list.
- Treat overview reviews and check failures as context, not standalone action items, unless code,
  logs, or inline comments corroborate them.
- Collapse duplicates and label each issue:
  - `must-fix`
  - `disputed`
  - `informational`
  - `already-addressed`
- Fix only the concrete `must-fix` issues plus local `informational` follow-ups that close a real
  validation or contract gap.
- Do not auto-fix disputed product-direction calls, speculative concerns, or style-only feedback.
- If the review feedback suggests a broader design risk, use `adversarial-review` on the touched
  files before editing.
- If the fixes span several surfaces or you need a final completeness pass, run
  `gdextension-hygiene` on the updated diff before concluding.

## Validation rules

- Match local validation to the touched surfaces.
- For `.gd` changes anywhere in the repo, run:

```powershell
pwsh -NoLogo -NoProfile -ExecutionPolicy Bypass -File .\tools\check_gd_scripts_headless.ps1
```

- For addon-specific native, sample, docs, or spec changes, follow the existing repo instructions
  for the narrowest correct build or test step.

## Feedback loop

1. Define the PR and changed-file scope.
2. Pull review threads, comments, and check runs.
3. Synthesize the actionable list and separate `must-fix` from `disputed`.
4. Fix the `must-fix` items in coherent batches.
5. Run the matching local validation.
6. If needed, run `gdextension-hygiene` on the updated diff to catch missing follow-through.
7. Summarize:
   - what was fixed
   - what remains disputed or intentionally unchanged
   - what PR follow-up still needs a human decision

## Output format

Use this structure:

1. **Scope**
2. **Actionable feedback**
3. **Fixed**
4. **Disputed or unchanged**
5. **Validation**
6. **PR follow-up**
