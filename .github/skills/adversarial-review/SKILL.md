---
name: adversarial-review
description: >
  Run a multi-model adversarial review loop over the current diff or requested scope. Use one
  Claude-family reviewer and one GPT-family reviewer in parallel, fix only high-confidence
  consensus issues, then rerun until no new consensus issues remain. Surface disputed findings to
  the user for a judgment call instead of auto-fixing them.
  Triggers: adversarial review, hostile review, red-team review, consensus review, review until
  clean, multi-model review
---

# Adversarial Review Skill

You are the repo-local adversarial-review skill. Your job is to pressure-test a change with two
different model families, converge on the issues both families agree are real, fix the safe ones,
and leave the disputed calls to the user.

## Core rules

- Always use two model families in each review round:
  - one Claude-family model
  - one GPT-family model
- Prefer the `task` tool with `agent_type: "code-review"` and explicit model overrides for the
  review passes.
- Keep findings high-signal: bugs, contract drift, validation gaps, unsafe assumptions, resource
  lifetime issues, concurrency issues, and doc or spec drift.
- Ignore style-only or cosmetic feedback.
- Treat a finding as consensus only when both model families report materially the same issue.
- Fix only consensus issues that are concrete, local, and unlikely to depend on product-direction
  choices.
- Do not auto-fix disputed, speculative, or low-confidence findings.
- After fixing consensus issues, rerun the same two-family review loop on the updated diff.
- Stop when both review passes find no new high-signal issues, or when only disputed findings
  remain.

## Default scope

- Default to the current diff when the user does not specify a narrower scope.
- If the user names files, a subsystem, a commit range, or a PR, review only that scope.
- Preserve unrelated user changes in a dirty worktree.

## Review loop

1. Define the scope and the exact files or diff under review.
2. Launch two review agents in parallel with the same review prompt:
   - Claude family: prefer `claude-sonnet-4.6` when available.
   - GPT family: prefer `gpt-5.4` when available.
3. Ask each reviewer for:
   - only high-signal findings
   - affected files and line anchors when possible
   - why the issue matters
   - a short fix direction
4. Merge the findings:
   - collapse duplicates
   - mark each one as `consensus` or `disputed`
5. If consensus issues exist:
   - fix them directly
   - update nearby tests, docs, or spec text when the fix changes behavior or closes contract drift
   - run the narrowest existing validation that proves the fix
6. Run the two-family review again on the new diff.
7. Repeat until no new consensus issues remain.

## Synthesis rules

- If both reviewers find nothing new, stop and say so plainly.
- If only one reviewer finds an issue, do not auto-fix it. Surface it as disputed.
- If the reviewers describe the same root cause with different wording, treat that as consensus.
- If the reviewers disagree on severity, keep the issue disputed unless the code or validation
  evidence clearly resolves it.
- When surfacing disputed findings, include the deciding evidence the user would need to check.

## Output format

Use this structure:

1. **Scope**
2. **Consensus issues fixed**
3. **Remaining disputed findings**
4. **Validation or docs follow-ups**
5. **Stop condition**

## Example execution pattern

- Start with two parallel `code-review` agents using Claude and GPT families.
- Synthesize the overlap into a consensus list.
- Fix the consensus list in one coherent batch.
- Re-run the same two-agent review on the updated diff.
- End only when the loop stabilizes, then present the disputed remainder for the user.
