---
name: Issue Triage
description: >-
  First-pass AI evaluation of an issue against the default branch, requested by a
  maintainer commenting `/triage`. See docs/ci/issue-triage.md.
on:
  issue_comment:
    types: [created]
  roles: [admin, maintainer, write]
  reaction: none
  status-comment: false
  permissions:
    contents: read
    issues: read
  steps:
    - name: Check out triage helper
      uses: actions/checkout@93cb6efe18208431cddfb8368fd83d5badbf9bfd # v5.0.1
      with:
        persist-credentials: false
        sparse-checkout: tools/ci
        sparse-checkout-cone-mode: true
    - name: Check triage eligibility
      id: gate
      uses: actions/github-script@3a2844b7e9c422d3c10d287c895573f7108da1b3 # v9.0.0
      with:
        script: |
          const triage = require(`${process.env.GITHUB_WORKSPACE}/tools/ci/issue_triage.cjs`);
          await triage.runGate({ github, context, core });
jobs:
  pre-activation:
    outputs:
      eligible: ${{ steps.gate.outputs.eligible }}
if: needs.pre_activation.outputs.eligible == 'true'
permissions:
  contents: read
  issues: read
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
  group: issue-triage-${{ github.repository_id }}-${{ github.event.issue.number }}-${{ github.event.comment.id }}
  cancel-in-progress: false
tools:
  github: false
  bash: false
  cli-proxy: false
  edit: false
  web-fetch: true
max-turns: 30
max-ai-credits: 200
timeout-minutes: 20
steps:
  - name: Prepare triage context
    uses: actions/github-script@3a2844b7e9c422d3c10d287c895573f7108da1b3 # v9.0.0
    with:
      script: |
        const triage = require(`${process.env.GITHUB_WORKSPACE}/tools/ci/issue_triage.cjs`);
        await triage.prepareContext({
          github,
          context,
          core,
          outDir: '/tmp/gh-aw/agent/triage',
          sha: process.env.GITHUB_SHA,
        });
post-steps:
  - name: Upload triage context metadata
    if: always()
    uses: actions/upload-artifact@043fb46d1a93c77aae656e7c1c64a875d1fc6a0a # v7.0.1
    with:
      name: issue-triage-context
      path: /tmp/gh-aw/agent/triage/context.json
      if-no-files-found: ignore
      retention-days: 7
  - name: Validate triage report
    uses: actions/github-script@3a2844b7e9c422d3c10d287c895573f7108da1b3 # v9.0.0
    with:
      script: |
        const triage = require(`${process.env.GITHUB_WORKSPACE}/tools/ci/issue_triage.cjs`);
        triage.validateAgentOutput({
          core,
          agentOutputPath: '/tmp/gh-aw/agent_output.json',
          root: process.env.GITHUB_WORKSPACE,
        });
safe-outputs:
  report-failure-as-issue: false
  noop: false
  missing-tool: false
  missing-data: false
  report-incomplete: false
  activation-comments: false
  jobs:
    post-triage-report:
      description: >-
        Submit the single triage report for this issue. Call exactly once. The
        `report` input is a JSON string matching the schema in the instructions.
      if: (!cancelled()) && needs.agent.result == 'success' && needs.detection.outputs.detection_success == 'true'
      permissions:
        contents: read
        issues: write
      inputs:
        report:
          description: The triage report as a JSON string.
          required: true
          type: string
      steps:
        - name: Check out analyzed commit
          uses: actions/checkout@93cb6efe18208431cddfb8368fd83d5badbf9bfd # v5.0.1
          with:
            ref: ${{ github.sha }}
            persist-credentials: false
        - name: Download triage context metadata
          uses: actions/download-artifact@3e5f45b2cfb9172054b4087a40e8e0b5a5461e7c # v8.0.1
          with:
            name: issue-triage-context
            path: ${{ runner.temp }}/issue-triage-context
        - name: Publish triage report
          uses: actions/github-script@3a2844b7e9c422d3c10d287c895573f7108da1b3 # v9.0.0
          env:
            # `post` comments on the issue. Set to `staged` to render the report to
            # the job summary only (docs/ci/issue-triage.md).
            TRIAGE_MODE: post
            TRIAGE_CONTEXT_PATH: ${{ runner.temp }}/issue-triage-context/context.json
          with:
            script: |
              const triage = require(`${process.env.GITHUB_WORKSPACE}/tools/ci/issue_triage.cjs`);
              await triage.publish({
                github,
                context,
                core,
                env: process.env,
                root: process.env.GITHUB_WORKSPACE,
                agentOutputPath: process.env.GH_AW_AGENT_OUTPUT,
                contextPath: process.env.TRIAGE_CONTEXT_PATH,
              });
---

# First-pass issue triage

Use Actions mode of the issue-triage skill below.

- Context file: `/tmp/gh-aw/agent/triage/context.md`. It holds the issue title,
  body, labels, and earlier comments. Read it first.
- Code: the current workspace, checked out at the default-branch commit named in
  the context file.
- Output: call the `post_triage_report` tool exactly once. Its `report` input is
  the report JSON object from the skill's report contract, serialized as a JSON
  string.

{{#runtime-import .github/skills/issue-triage/SKILL.md}}
