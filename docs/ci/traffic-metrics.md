# Traffic metrics

[`.github/workflows/track-traffic.yml`](../../.github/workflows/track-traffic.yml)
snapshots this repository's GitHub traffic data daily and commits it to the
orphan `metrics` branch.

## Why

GitHub retains repository traffic (clones, views, referrers, popular paths) for
a rolling **14 days**, then discards it permanently. There is no backfill API.
Any day not captured inside that window is lost for good. This workflow exists
solely to capture it before that happens.

## Why not release download counts

`download_count` is a field on **release assets** — files explicitly uploaded to
a release. It does not exist for:

- the auto-generated `Source code (zip/tar.gz)` links (synthesized from the git
  tag on request; not stored objects, so there is nothing to count),
- `git clone`,
- CDN or mirror copies.

This repository currently publishes **no** uploaded release assets, so
`.../releases` reports `assets: []` and a
`img.shields.io/github/downloads/.../total` badge renders a red `0`. Adding
download tracking is only worthwhile once a release workflow starts attaching
real artifacts. At that point it can reuse the same merge script and storage
model — the schema is per-key upsert, not traffic-specific.

## Setup: the traffic GitHub App

**The built-in `GITHUB_TOKEN` cannot read the traffic API.** These endpoints
require write/admin access, which maps to the fine-grained **`Administration:
Read`** permission — and `administration` is not an assignable scope in a
workflow's `permissions:` block. There is no configuration of `GITHUB_TOKEN`
that works; it returns `403 Must have push access to repository`. A separate
credential is mandatory.

We use a **GitHub App** rather than a PAT. A PAT is bound to one person: it
expires on a fixed date, and it dies if that person changes roles or leaves.
Either way the snapshots stop — and once they have been stopped for 14 days,
the missed data is unrecoverable. An App is owned by the org, has no expiring
credential to rotate, and mints a fresh repo-scoped token per run that is
revoked when the job ends.

### 1. Register the App

**Settings → Developer settings → GitHub Apps → New GitHub App**

| Field | Value |
| --- | --- |
| Name | `xbox-godot-sample-traffic` (must be globally unique) |
| Homepage URL | the repo URL — unused, but required |
| Webhook | **uncheck Active** — this App never receives events |

**Repository permissions** — grant exactly these two, nothing else:

| Permission | Level | Why |
| --- | --- | --- |
| `Administration` | **Read-only** | read the traffic endpoints |
| `Contents` | **Read and write** | push snapshots to the `metrics` branch |

Under "Where can this App be installed?", pick **Only on this account**.

> Registering the App under the `microsoft` org (rather than a personal
> account) is what makes it outlive any individual. If org policy blocks
> creating org-owned Apps, an org owner has to register it.

### 2. Record the App ID and generate a private key

On the App's settings page after creation:

- Copy the **App ID** (a number; not sensitive).
- **Generate a private key** — this downloads a `.pem` **once**. Losing it means
  generating a new one; leaking it lets anyone act as the App, so delete the
  local copy after step 4.

### 3. Install the App on the repository

On the App's page → **Install App** → the `microsoft` org → **Only select
repositories** → `XBOX-Godot-Sample`.

The App can do nothing until it is installed. A correctly configured but
*uninstalled* App fails at token minting — an easy step to skip.

### 4. Store the credentials

App ID as a repository **variable** (not sensitive; being visible makes
debugging easier), private key as a **secret**. Both require repo admin:

```powershell
gh auth switch --user <account-with-admin>
gh variable set TRAFFIC_APP_ID --repo microsoft/XBOX-Godot-Sample --body "<app-id>"
gh secret   set TRAFFIC_APP_PRIVATE_KEY --repo microsoft/XBOX-Godot-Sample < path\to\key.pem
```

Pipe the `.pem` from the file rather than pasting it — the key must keep its
newlines, and a single-line paste fails to parse. Delete the local `.pem`
afterwards.

### 5. Verify

Once the workflow is on the default branch:

```powershell
gh workflow run track-traffic.yml --repo microsoft/XBOX-Godot-Sample
gh run list --workflow track-traffic.yml --repo microsoft/XBOX-Godot-Sample --limit 1
git fetch origin metrics && git show origin/metrics:data/clones.csv
```

### Fallback: fine-grained PAT

If org policy blocks App registration, a fine-grained PAT works with the same
`Administration: Read` permission, stored as a secret and referenced in place of
the minted token. **It must be created by an account that already has admin on
this repo** — a token can never grant more access than its owner has. Accept the
expiry and set a rotation reminder, because an unnoticed expiry silently
destroys data after 14 days.

## Storage model

Data lands on the orphan `metrics` branch, not `main`, so daily commits do not
churn `main`'s history or retrigger the PR gates.

Rows are **upserted** on a key (date, or date + name) rather than appended,
because consecutive snapshots overlap by 13 of their 14 days. Appending would
duplicate every day roughly 14 times. Last-write-wins also lets the current
(partial) UTC day self-correct on the following run, and makes re-runs
idempotent.

See the [`metrics` branch README](https://github.com/microsoft/XBOX-Godot-Sample/blob/metrics/README.md)
for the per-file schemas and the analysis caveats — in particular, the
referrer/path rollups are trailing-14-day totals and **must not** be summed
across snapshot dates.

## Local development

The merge helper is plain bash and has a standalone test script:

```bash
bash .github/scripts/test_merge_traffic_csv.sh
```

The git sequence that maintains the orphan branch can also be exercised offline,
against a throwaway local bare repo — no token or network required. It replays
three consecutive "days" to cover bootstrap, shallow-fetch-then-push, and the
no-change path:

```bash
bash .github/scripts/dryrun_metrics_branch.sh
```

To dry-run the API calls yourself, you need a token with `Administration: Read`
(your own account credentials work if you have admin on the repo):

```bash
gh api "repos/microsoft/XBOX-Godot-Sample/traffic/clones?per=day" \
  --jq '.clones[] | [(.timestamp[0:10]), (.count|tostring), (.uniques|tostring)] | join(",")'
```

## Failure modes worth knowing

| Symptom | Cause |
| --- | --- |
| `403 Must have push access` | App lacks `Administration: Read`, or is not installed on this repo |
| Token minting step fails | Wrong `TRAFFIC_APP_ID`, malformed private key (newlines lost), or App not installed |
| Push rejected on `metrics` | App lacks `Contents: Write` |
| Workflow silently stops running | GitHub disables schedules after 60 days of repo inactivity |
| Gap in the date series | Workflow was broken/disabled; unrecoverable after 14 days |
| Newest row looks low | Current UTC day is partial; settles on the next run |
