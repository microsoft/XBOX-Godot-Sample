# CI: PR Gates

This repo runs three CI gates on GitHub Actions, all on GitHub-hosted
`windows-latest` runners. They are split across two workflows by trigger model.

| Gate | Workflow | Trigger | Runner |
| --- | --- | --- | --- |
| GDScript parse | `.github/workflows/pr-gates.yml` (`parse-gate`) | `pull_request` / `push` to `main`, `workflow_dispatch` | hosted, **matrixed over Godot versions** |
| Fuzz replay | `.github/workflows/pr-gates.yml` (`fuzz-replay`) | same as above | hosted, single |
| PlayFab live (read + write) | `.github/workflows/playfab-live-nightly.yml` | nightly `schedule` + `workflow_dispatch` | hosted, sandbox title |

## Godot version range

The set of Godot versions CI exercises is data-driven in
[`.github/godot-versions.json`](../../.github/godot-versions.json):

```json
{
  "default": "4.6.1-stable",
  "supported": ["4.6.1-stable"]
}
```

- `default` — used by single-version jobs (the live tier, and the
  `workflow_dispatch` default for it).
- `supported` — the matrix the **parse gate** fans out over (one leg per
  version, `fail-fast: false` so each reports independently).

**To add a Godot version to the range**, append its version string to
`supported`. The only requirement is that `godotengine/godot-builds` publishes a
release tagged `<version>` with the asset `Godot_v<version>_win64.exe.zip` (the
zip bundles the `_console.exe` the gates use). Verified-available examples you
can extend with today: `4.5.1-stable`, `4.4.1-stable`, `4.3-stable`. No workflow
edits are needed — the `versions` job reads the manifest and the matrix updates
automatically.

The `setup-godot` composite action (`.github/actions/setup-godot/`) downloads,
caches, and exposes any requested version via `GODOT` / `GODOT_CONSOLE` /
`GODOT_BIN`, matching the discovery order in `tools/check_gd_scripts_headless.ps1`
and `tools/run_all_tests.ps1`. The repo tooling does not pin or enforce a Godot
version, so widening the range is purely additive.

## Parse gate

Runs `tools/check_gd_scripts_headless.ps1` (headless `--check-only` over every
`.gd` file). One matrix leg per supported Godot version.

## Fuzz replay

Builds the libFuzzer targets with the `fuzz` preset (ClangCL + ASan, static CRT;
**no Microsoft GDK required**) and replays each target's committed corpus once:

```powershell
cmake --preset fuzz
cmake --build build/fuzz --preset debug-fuzz
pwsh -File tools/run_fuzz_replay.ps1
```

This is **regression replay, not active fuzzing** — `run_fuzz_replay.ps1` passes
the individual corpus files to each target, so libFuzzer runs every input exactly
once and exits non-zero on the first crash/leak/timeout. (Passing a corpus
*directory* would make libFuzzer fuzz indefinitely; the script deliberately
passes files.) Corpora live at `tests/cpp/fuzz/corpus/<target>/`. A target with
no corpus is still smoke-run with `-runs=0`. On failure, the workflow uploads any
`crash-*` / `leak-*` / `timeout-*` artifacts.

To add a regression input, drop the bytes into the matching
`tests/cpp/fuzz/corpus/<target>/` directory and commit it.

## PlayFab live tests (nightly)

`playfab-live-nightly.yml` runs the live tier against a **dedicated sandbox
PlayFab title**:

```powershell
tools/run_all_tests.ps1 -Live -AllowLiveWrites -PlayFabTitleId <sandbox> ...
```

It runs only on a nightly schedule and manual `workflow_dispatch` (never on
`pull_request`), so title secrets are never reachable from fork PRs. The
`workflow_dispatch` form accepts an optional `godot_version` (defaults to the
manifest `default`).

### Required secrets (GitHub Environment `playfab-sandbox`)

Configure these as **environment** secrets on an environment named
`playfab-sandbox` (Settings → Environments). The job refuses to run if
`PLAYFAB_TITLE_ID` is unset.

| Secret | Required | Purpose |
| --- | --- | --- |
| `PLAYFAB_TITLE_ID` | yes | The sandbox title id. **Must be a dedicated sandbox title** — the live tier writes persistent state. |
| `PLAYFAB_CUSTOM_ID` | optional | Pre-existing custom id for `LoginWithCustomID`. |
| `PLAYFAB_MULTIPLAYER_MATCH_QUEUE` | optional | Matchmaking queue name for Multiplayer coverage. |

> **No developer secret key.** `PLAYFAB_DEVELOPER_SECRET_KEY` is **not** used by
> this gate. `run_all_tests.ps1` scrubs it from child processes, and a
> pre-provisioned title only needs client-side sign-in. That high-privilege
> secret belongs only to a separate, manual title-provisioning step
> (`tools/configure_playfab_test_title.ps1`), not to the per-run live gate.

## Known limitations

- **Microsoft GDK is out of scope on the runner (for now).** The live job
  configures the `default` preset, which builds the PlayFab addon and needs the
  PlayFab SDK provisioned on the hosted runner. Until that provisioning lands,
  the configure step is the expected first point of failure — it fails loudly
  rather than reporting a false green for live tests it never ran.
- **Fuzz infra dependency.** The `fuzz-replay` job assumes the fuzz harness /
  `fuzz` preset (originally on `infra/fuzz-testing`) is present on `main`.
