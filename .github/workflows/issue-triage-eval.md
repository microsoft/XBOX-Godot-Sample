---
name: Issue Triage Eval
description: >-
  Runs the issue-triage skill against one pinned eval case and uploads the report
  as an artifact for human scoring. Never writes to issues. See
  tests/evals/issue-triage/README.md.
on:
  workflow_dispatch:
    inputs:
      case:
        description: Eval case to run (tests/evals/issue-triage/cases).
        required: true
        type: choice
        options:
          - arranged-lobby-config-177
          - gameinput-stuck-actions-parse-178
          - matchmaking-cancel-hresult-191
          - virtual-keyboard-events-190
  reaction: none
  status-comment: false
permissions:
  contents: read
  copilot-requests: write
engine:
  id: copilot
# Keep in sync with DOC_HOSTS in tools/ci/issue_triage.cjs.
network:
  allowed:
    - defaults
    - devdocs.xbox.com
    - learn.microsoft.com
strict: true
concurrency:
  group: issue-triage-eval-${{ github.ref }}-${{ inputs.case }}
  cancel-in-progress: false
  job-discriminator: ${{ github.run_id }}
tools:
  github: false
  bash: false
  cli-proxy: false
  edit: false
  # Mirrors issue-triage.md: web-fetch makes gh-aw drop --disable-builtin-mcps,
  # leaving built-in github-mcp-server reachable. Accepted for the same reason
  # (read-only job token plus the gh-aw firewall).
  web-fetch: {}
max-turns: 30
max-ai-credits: 200
timeout-minutes: 20
steps:
  - name: Stage eval snapshot
    env:
      EVAL_CASE: ${{ inputs.case }}
      # Mirror the agent job's model selector so run.json records the model, not the engine.
      EVAL_MODEL: ${{ vars.GH_AW_MODEL_AGENT_COPILOT || vars.GH_AW_DEFAULT_MODEL_COPILOT || 'auto' }}
    run: |
      node tools/ci/issue_triage_eval.cjs prepare \
        --case "$EVAL_CASE" \
        --out /tmp/gh-aw/agent/eval \
        --model "$EVAL_MODEL" \
        --run-id "$GITHUB_RUN_ID"
      # Gold expectations must never be readable by the agent.
      rm -rf tests/evals
post-steps:
  - name: Collect and validate eval report
    if: always()
    run: |
      node tools/ci/issue_triage_eval.cjs collect \
        --run /tmp/gh-aw/agent/eval \
        --agent-output /tmp/gh-aw/agent_output.json \
        --item-type submit_triage_eval_report || true
      node tools/ci/issue_triage_eval.cjs validate-report --run /tmp/gh-aw/agent/eval --no-fetch || true
  - name: Upload eval run
    if: always()
    uses: actions/upload-artifact@043fb46d1a93c77aae656e7c1c64a875d1fc6a0a # v7.0.1
    with:
      name: issue-triage-eval-${{ inputs.case }}
      path: |
        /tmp/gh-aw/agent/eval/run.json
        /tmp/gh-aw/agent/eval/report.json
        /tmp/gh-aw/agent/eval/context/
      if-no-files-found: warn
      retention-days: 30
safe-outputs:
  report-failure-as-issue: false
  noop: false
  missing-tool: false
  missing-data: false
  report-incomplete: false
  activation-comments: false
  jobs:
    submit-triage-eval-report:
      description: >-
        Submit the single triage report for this eval case. Call exactly once. The
        `report` input is a JSON string matching the schema in the instructions.
      permissions:
        contents: read
      inputs:
        report:
          description: The triage report as a JSON string.
          required: true
          type: string
      steps:
        - name: Record submission
          run: |
            echo "Eval report submitted. Score it from the issue-triage-eval-* artifact of the agent job; see tests/evals/issue-triage/README.md." >> "$GITHUB_STEP_SUMMARY"
---

# Issue triage eval

Use Actions mode of the issue-triage skill below.

- Context file: `/tmp/gh-aw/agent/eval/context/context.md`. It holds the issue
  labels, title, body, and earlier comments. Read it first.
- Code: the read-only snapshot at `/tmp/gh-aw/agent/eval/source`, extracted from
  the pinned commit named in the context file. Treat that directory as the
  repository root: search and read only there, and give every citation path
  relative to it. Ignore the job workspace; it is not the snapshot under
  evaluation.
- Output: call the `submit_triage_eval_report` tool exactly once. Its `report`
  input is the report JSON object from the skill's report contract, serialized
  as a JSON string.

{{#runtime-import .github/skills/issue-triage/SKILL.md}}
