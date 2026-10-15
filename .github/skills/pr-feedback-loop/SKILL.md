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
- A `must-fix` needs an agreed requirement, a reachable failure under the supported usage or
  trust model, and code or test evidence of the consequence. Reviewer severity and reviewer
  agreement are not evidence by themselves.
- Fix only accepted `must-fix` issues. Do not bundle informational improvements into remediation.
- Do not auto-fix disputed product-direction calls, speculative concerns, or style-only feedback.
- Prefer deleting an unnecessary claim or simplifying the contract over adding machinery to
  defend it. Do not weaken an agreed safety requirement.
- A fix that introduces coordination, a new abstraction, a second grading layer, or a stronger
  trust guarantee is a design decision: explain the simpler alternative and obtain user approval.
- Do not launch another reviewer merely because a finding sounds serious. Use
  `adversarial-review` only when requested or a specific unresolved risk needs it.
- Default to one triage batch and one verification pass over the fixes. New unrelated findings
  go into a follow-up list, not another automatic fix/review cycle. Reopen a deferred item only
  with new evidence or user direction.
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
3. State the agreed contract. For each finding, name the violated requirement, reachable failure,
   evidence, and smallest proportionate fix; otherwise classify it as disputed or informational.
4. Fix the `must-fix` items in coherent batches.
5. Run the matching local validation.
6. Verify the accepted fixes and their regressions. If needed, run `gdextension-hygiene` for
   completeness; do not turn verification into a new open-ended review.
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
