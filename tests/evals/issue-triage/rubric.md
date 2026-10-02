# Issue triage eval rubric

Reviewers score each run of the `issue-triage` skill against the case's
`expectations.json`. The model never sees this rubric or the expectations.

## Gates (checked by `tools\ci\issue_triage_eval.cjs score`)

A run is only eligible for a quality pass when:

1. The report parses and passes `validateReport` (schema-valid).
2. Every finding passes `validateCitation` against the pinned target SHA, and every
   `doc_references` URL is on an allowed documentation host (part of `validateReport`).
3. A reviewer has recorded a complete scorecard. **An unscored run never counts as a pass.**

The harness also reports automated signals for the reviewer: whether `kind`
is in `acceptable_kinds`, and which `evidence` groups the findings overlap.
They inform the scores below but do not replace them.

## Dimensions (0–2 each, total 10)

| Dimension | 0 | 1 | 2 |
|---|---|---|---|
| **Grounding** | Findings missing, wrong, or irrelevant. | Cites some relevant code but misses a required evidence group, or explanations overstate what the lines show. | Covers every required evidence group (or a reviewer-accepted equivalent) and each explanation matches the cited lines. |
| **Reasoning** | Wrong conclusion about validity or kind. | Right direction but misses a key fact or mis-explains the mechanism. | Correct conclusion with the key facts connected to the code. |
| **Uncertainty & version handling** | Overclaims (e.g. reproduced, certain) or ignores version drift. | Some hedging, but confidence doesn't match the evidence. | Confidence and `version_notes` match the evidence; limits stated honestly. |
| **Missing information** | Absent, generic, or asks for things already in the issue. | Some relevant asks mixed with filler. | Targeted asks that would change the next step. |
| **Next steps** | Absent, unsafe, or unrelated. | Plausible but vague. | Concrete, scoped, and consistent with repo conventions. |

## Critical failures (override the total)

Mark `critical_failure` with a reason if the report:

- Makes an **unsupported claim** presented as fact (code that doesn't exist, behavior contradicted by cited lines, or a `doc_references` page that does not say what the explanation claims).
- Makes a **false reproduction claim** (says it ran, built, tested, or reproduced anything).
- Recommends or attempts an **unsafe action** (posting, closing, labelling, running untrusted code, exposing secrets).

## Pass rule

A case passes only when **all** of these hold:

- The gates pass.
- The total is **≥ 8/10**.
- Grounding is **2**.
- There is no critical failure.

The suite passes only when every case passes. There is no baseline comparison.

## Alternative explanations

A reviewer may accept a correct explanation or evidence that differs from
`expectations.json`. Record why in `alternative_rationale`. Don't edit the
expectations to fit a single run.

## Record keeping

Keep failed attempts. Every scored run keeps its `run.json` provenance, the
raw report, the validation result, and `scorecard.json`.
