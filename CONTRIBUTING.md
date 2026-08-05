# Contributing

This project welcomes contributions and suggestions. Most contributions require you to
agree to a Contributor License Agreement (CLA) declaring that you have the right to,
and actually do, grant us the rights to use your contribution. For details, visit
https://cla.microsoft.com.

When you submit a pull request, a CLA-bot will automatically determine whether you need
to provide a CLA and decorate the PR appropriately (e.g., label, comment). Simply follow the
instructions provided by the bot. You will only need to do this once across all repositories using our CLA.

This project has adopted the [Microsoft Open Source Code of Conduct](https://opensource.microsoft.com/codeofconduct/).
For more information see the [Code of Conduct FAQ](https://opensource.microsoft.com/codeofconduct/faq/)
or contact [opencode@microsoft.com](mailto:opencode@microsoft.com) with any additional questions or comments.

## CI for contributions from forks

Most PR gates — the GDScript parse gate, the native build, the fuzz replay, and
the C# facade parity check — use no secrets and run normally on your pull
request.

The `test-tier` job is different. This repo's test suite has **no offline
mode**: the live and live-write tiers always run against a dedicated sandbox
PlayFab title, and the orchestrator refuses to start without that
configuration. GitHub does not expose repository or environment secrets to pull
requests from forks, so `test-tier` fails immediately on a fork PR with a
message pointing here. **This is expected and is not something you did wrong.**

To get a full validation run, a maintainer pushes your PR head to a `pr/**`
branch inside this repository, which triggers a complete run with the sandbox
credentials available. The maintainer links that run back on your PR. You do not
need to do anything except mention in the PR description that your change needs
a maintainer re-run, and re-request one after any substantive push.

If you can provision your own sandbox PlayFab title, you can also validate
locally before opening the PR:

```powershell
pwsh -NoLogo -NoProfile -ExecutionPolicy Bypass -File .\tools\run_all_tests.ps1 `
    -PlayFabTitleId <your-sandbox-title> `
    -PlayFabCustomId <custom-id> `
    -PlayFabMatchmakingQueue <queue>
```

Use `tools\configure_playfab_test_title.ps1` to provision the accounts, queue,
and leaderboard the suite expects. Never point this at a shared or production
title — the live-write tier mutates it. See `tests\godot\README.md` for the
test-tier contract and `docs\ci\pr-gates.md` for the full CI model.
