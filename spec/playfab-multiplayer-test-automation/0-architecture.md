# PlayFab Multiplayer Test Automation — Architecture

## Overview

This document specifies the architecture of a pure-Godot, scenario-driven live test harness for PlayFab Lobby, Matchmaking, and Party. It replaces the existing `tools\run_playfab_multiplayer_live.ps1` + `tests\godot\playfab_multiplayer_worker\worker.gd` PowerShell + file-IPC harness with two Godot --headless projects communicating over raw TCP, plus a directory of plain GDScript scenario files.

The harness is patterned on the [`ai-test-orchestrator` replication playbook](https://github.com/gaming-microsoft/ai-test-orchestrator/blob/main/guide/3-replication-playbook.md), adapted for a Godot/GDExtension codebase. Where the playbook reference uses C# .NET + WebSocket + YAML + C++ test apps, this design uses Godot --headless + raw TCP + GDScript scenario files + Godot --headless test clients. The rationale for each deviation is documented in this file.

The companion files in this directory are:

- `1-test-matrix.md` — full Lobby + Match + Party scenario matrix, P0-P3 with state-transition tables.
- `2-detailed-scenarios.md` — step-by-step blueprints for the top P0/P1 scenarios.
- `3-harness-spec.md` — wire protocol, framing, handshake, request/response/event model, error handling, connection lifecycle.
- `4-scenario-authoring.md` — scenario file contract, base class API, orchestrator API surface, worked examples.

## Design goals

1. **Single language top-to-bottom** — both orchestrator and test client are Godot --headless GDScript projects. No C++ in the orchestrator, no .NET runtime, no YAML parser. Scenario authors write GDScript and only GDScript.
2. **Runtime-loaded scenarios** — scenario files are plain `.gd` files discovered at orchestrator startup. Adding or editing scenarios does not require rebuilding any binary; scenario iteration is `edit file → re-run orchestrator`.
3. **Multi-client coordination** — the orchestrator owns scenario sequencing across N connected test clients (`PlayerA`, `PlayerB`, …). Scenarios direct each client through named role addressing; the orchestrator routes commands and aggregates responses.
4. **Event ordering correctness** — the harness explicitly supports the "subscribe before triggering" pattern. Scenarios that wait for signals (e.g. `lobby.member_added` on `PlayerB` after `PlayerA` invites) register an `expect_event` waiter before sending the trigger; the test client maintains an event log keyed by subscription so signals fired between subscribe and await are not lost.
5. **Explicit handle ownership** — scenarios pass user-chosen handles (`"as": "host_lobby"`) on create commands and reference them by name in subsequent calls. The test client does not maintain implicit "primary lobby" or "primary ticket" globals. This makes multi-lobby and arranged-lobby scenarios first-class.
6. **Mandatory per-scenario reset** — after every scenario (pass or fail), the orchestrator sends `reset_client` to every connected client; on timeout or crash, the affected client is killed and respawned. Scenarios cannot leak state across runs.
7. **Stable wire protocol with capability negotiation** — clients announce protocol version, addon version, Party availability, and platform during handshake. The orchestrator rejects mismatches loudly rather than silently degrading.
8. **Fresh process per CI run** — the orchestrator and all spawned clients are short-lived (one orchestrator run = one scenario batch). This sidesteps Godot's script cache for runtime-loaded scenarios and keeps test isolation simple.
9. **Side-by-side with the legacy harness during transition** — the new harness ships alongside the existing PowerShell + file-IPC runner. `tools\run_all_tests.ps1 -Live` runs both during C4 and C5 for parity verification; the legacy harness is deleted only in C6 once the new harness has soaked clean.

## Component overview

```
┌──────────────────────────────────────────────────────────────────────────┐
│  Orchestrator   (tests\godot\mp_orchestrator\, Godot --headless)          │
│  ─────────────                                                            │
│  - TCPServer on configurable port (default 18765)                         │
│  - Per-client connection state machine (handshake, heartbeat, framing)    │
│  - Scenario discovery (scan scenarios\*.gd)                               │
│  - Scenario runner (one scenario at a time, sequential)                   │
│  - Result aggregator (writes mp-test-results.{json,md} to OutDir)         │
│  - Optional client auto-spawn (--auto-spawn N)                            │
└─────────────────────────────────┬────────────────────────────────────────┘
                                  │ raw TCP, length-prefixed JSON
                                  │ (default 127.0.0.1:18765, configurable)
                  ┌───────────────┼───────────────┐
                  │               │               │
       ┌──────────▼─────┐  ┌──────▼─────┐  ┌──────▼─────┐
       │  mp_test_client │  │ mp_test_   │  │ mp_test_   │
       │  (PlayerA)      │  │ client     │  │ client     │
       │                 │  │ (PlayerB)  │  │ (Observer) │
       │  - StreamPeerTCP│  │            │  │            │
       │  - command      │  │            │  │            │
       │    dispatch     │  │            │  │            │
       │  - PlayFab      │  │            │  │            │
       │    addon        │  │            │  │            │
       │  - event log    │  │            │  │            │
       └─────────────────┘  └────────────┘  └────────────┘
```

Both projects are Godot --headless. The orchestrator does not import `godot_playfab`. The test clients import `godot_playfab` (CMake mirrors the addon into `tests\godot\mp_test_client\addons\godot_playfab\` the same way it does for `playfab_multiplayer_worker`).

## Why pure Godot

The playbook reference uses C# .NET 8 for the controller because (a) C# has mature WebSocket and YamlDotNet libraries and (b) the target test app is a C++ Xbox console binary that already has Visual Studio toolchain. Both reasons evaporate in our context:

- The test app is already a Godot project. Adding a non-Godot orchestrator means a second toolchain (.NET SDK or a C++ build target) just to talk to it.
- The only thing YAML offered the playbook was "scenarios as data so you can add tests without recompiling the controller". In our context, `.gd` files are already loaded at runtime by Godot — adding or editing a scenario doesn't recompile anything. GDScript scenarios are as data-driven as YAML scenarios, with the bonus that they're a full programming language and can express loops, conditionals, parameterization, and helper extraction without inventing a DSL.
- Splitting "orchestrator logic in C++/.NET, scenarios in GDScript" adds a serialization hop between the scenario runtime and the orchestrator routing layer for no clear benefit.

The single-language design has one tradeoff: Godot --headless is an unusual host for a TCP server-style application. The harness layer is the entirety of `tests\godot\mp_orchestrator\`, and the `TCPServer` lifecycle is documented in detail in `3-harness-spec.md` so the unfamiliarity is contained.

## Why raw TCP (not WebSocket)

The playbook uses WebSocket because the reference controller has a WinForms UI that might also serve a web panel. Our orchestrator is headless-only; we have no browser interop, no HTTP upgrade requirement, and no streaming-from-the-browser scenarios. Raw TCP avoids:

- The HTTP upgrade handshake and the JS-style framing requirements.
- WebSocket close-frame protocol handling.
- A dependency on Godot's `WebSocketPeer` (which has its own quirks vs the lower-level `StreamPeerTCP` API).

Length-prefixed JSON over `StreamPeerTCP` is roughly 100 lines of GDScript per side and gives us bidirectional binary-clean message framing with no schema or protocol surprises. The full framing format and connection lifecycle are specified in `3-harness-spec.md`.

If the orchestrator ever needs to accept browser clients (it shouldn't), the wire-protocol layer is replaceable by adding a `WebSocketPeer` adapter beside the `StreamPeerTCP` listener without touching the scenario runtime.

## Why plain GDScript scenarios (not GUT)

GUT is excellent for unit-suite-style coverage with `before_each` / `after_each` / `test_*` discovery and rich assertion reporting. It's a poor fit for the harness here because:

- Scenarios are not single-process tests; each scenario coordinates 2-3 long-lived test client processes through the orchestrator. GUT's test runner has no concept of "send this command to PlayerA, wait for that signal on PlayerB, send a follow-up to PlayerA". Forcing scenarios into GUT classes means writing a parallel coordination layer inside GUT, which buys nothing.
- GUT's reporting model is per-method pass/fail; we want per-scenario pass/fail with multi-line failure detail (which client failed, which step, what was expected, what actually happened).
- GUT scenarios in our existing hosts are deliberately scoped to *single-process* coverage. Mixing single-process GUT tests and multi-process orchestration tests in the same framework would be a category confusion.

The scenario base class (`mp_scenario_base.gd`, specified in `4-scenario-authoring.md`) provides what scenarios actually need: metadata constants, `skip()` / `fail()` / `allow_failure()`, a default `TIMEOUT_SEC`, and assertion helpers. The orchestrator runner provides discovery and reporting. Both pieces are small, focused, and built for the multi-client coordination use case.

GUT-based per-host suites (`tests\godot\gdk\`, `tests\godot\playfab\`, `tests\godot\gameinput\`) remain unchanged. The new harness is purely additive for live multi-client coverage.

## Connection and process lifecycle

A typical orchestrator invocation:

1. Orchestrator process starts: parses CLI args (`--port`, `--scenarios-dir`, `--auto-spawn N`, `--results-dir`, `--filter <regex>`, `--list`).
2. Orchestrator binds the listen port and begins accepting connections.
3. If `--auto-spawn N` was passed, the orchestrator spawns N test client processes locally via `OS.create_process`, passing `--orchestrator-host`, `--orchestrator-port`, `--client-id PlayerA` (et al.). Otherwise, the operator launches the test client processes manually (potentially on different machines).
4. Each test client connects to the orchestrator, performs handshake (protocol version, capabilities, client-id), and enters the command dispatch loop.
5. The orchestrator scans `scenarios\*.gd`, instantiates each, checks `REQUIRED_ROLES` against connected clients, and queues scenarios for execution.
6. For each scenario in order:
   a. Orchestrator emits `scenario_started` event to each client involved.
   b. Orchestrator calls `await scenario.run(orchestrator)`.
   c. Scenario sends commands, awaits responses, awaits expected events, and returns `{ ok, failure_reason, details }`.
   d. Orchestrator sends `reset_client` to every involved client.
   e. Orchestrator records the result.
   f. On timeout or unhandled exception: kill the involved clients, respawn them, mark the scenario failed.
7. After all scenarios complete: orchestrator writes `mp-test-results.json` and `mp-test-results.md`, sends shutdown to all clients, exits with code 0 (all passed or all skip-eligible) or 1 (any failure).
8. Test clients receive shutdown, perform graceful PlayFab shutdown (multiplayer leave-all + Party network leave + addon shutdown), and exit.

## Launch modes

| Mode | Invocation | Use case |
| --- | --- | --- |
| Single-host auto-spawn | `mp_orchestrator --auto-spawn 3 --scenarios-dir <path>` | Default CI and local dev. Orchestrator spawns 3 clients, runs full suite, exits. |
| Single-host manual launch | `mp_orchestrator --port 18765` then launch 3 clients separately | Debugging a single client under a debugger or with extra logging. |
| Multi-host | `mp_orchestrator --port 18765 --bind-address 0.0.0.0` on host machine; clients launched on other machines with `mp_test_client --orchestrator-host <host-ip> --orchestrator-port 18765 --client-id PlayerB` | Cross-machine validation, including any future scenario that genuinely needs distinct hosts. |
| Discovery dry-run | `mp_orchestrator --list` | Print discovered scenarios + their metadata; do not connect or run. |
| Filtered run | `mp_orchestrator --filter "lobby.*" --auto-spawn 3` | Run a subset by regex. |

Multi-host mode is supported by the protocol but is **not** currently required by any scenario. PR #134 fixed the same-host UDP-bind collision in the addon (`PlayFabParty::_ensure_initialized` now calls `PartyManager::SetOption(LocalUdpSocketBindAddress, port=0)` so each process binds an ephemeral port), so same-host Party scenarios are first-class. Multi-host mode is preserved for future use and for diagnosing harness-vs-SDK distinctions.

## Decisions (locked)

The full list of decisions and the alternatives ruled out is in `<session-state>\plan.md`. Key items relevant to this document:

- **Pure Godot** — no C++ orchestrator, no .NET, no YAML. Confirmed by rubber-duck review and four rounds of user pushback.
- **Plain GDScript scenarios** — `func run(orch: TestOrchestrator) -> Dictionary` plus metadata constants. Not GUT classes.
- **Raw TCP with length-prefixed JSON** — `<uint32-BE length><utf8 JSON bytes>`, max payload 4 MiB, protocol version in handshake.
- **Capability negotiation** — handshake includes protocol version, addon version, Party availability, platform.
- **Explicit handles** — `{ "as": "host_lobby" }` and `{ "handle": "host_lobby" }`. No implicit `_primary_lobby` globals.
- **Mandatory reset + respawn** — `reset_client` after every scenario; kill + respawn on timeout or crash.
- **Rotating sign-in pool** — each test-client process holds a
  rotation index that advances on every `reset_client`, so successive
  scenarios sign in as the next account in a per-role pool
  (`{prefix}-host-1`..`-host-4`, `-client-1..-4`, etc.). This
  spreads the per-(title_player_account) PlayFab rate limits
  (`createlobby` = 6/120s/account, `joinlobby` = 6/120s/account,
  `findlobbies` = 12/120s/account, …) across the pool so a single
  process can run many scenarios without burning the account's
  quota. The pool size and accounts are defined by
  `tools/configure_playfab_test_title.ps1` and surfaced in the
  title-data marker. See
  [2-detailed-scenarios.md "Sign-in pool rotation"](2-detailed-scenarios.md#sign-in-pool-rotation--why).
- **Single PR, sequential commits C0-C6** — user-confirmed. Each commit ships a complete and testable artifact.
- **Party same-host now works (re-validated + fixed in C4)** — PlayFab Party uses entity tokens (not XUser), so two same-host processes with distinct custom-id sign-ins are valid distinct Party devices. C4d live validation discovered the SDK's default `LocalUdpSocketBindAddress` was binding a fixed port that collided between processes (`PartyNetworkDestroyed: failed to bind or connect the UDP socket because the address is already in local use`). Fixed in PR #134 by setting `port=0` (ephemeral) + `ExcludeGameCorePreferredUdpMultiplayerPort` in `PlayFabParty::_ensure_initialized` (`addons/godot_playfab/src/playfab_party.cpp`). Single-host canary + multi-process host+guest now both pass on transport-only Party. Same-host chat-control allocation still collides (audio device probing + shared OS chat resources) and is reserved for a future scenario gated on `multi_machine_eligible` once the `--remote-clients` launch mode lands.
- **Issue #133** — to be closed only by C6, and only if Party coverage is real (not gated skipware).

## Out of scope

- C++ orchestrator. Rejected because GDScript scenarios give the no-recompile property YAML would have provided; splitting languages adds a serialization hop with no payoff. If a future workload requires native CPU perf in the orchestrator (it shouldn't — orchestration is I/O-bound), it can be replaced behind the same TCP protocol.
- YAML scenarios. Same rationale.
- WinForms or web UI. The orchestrator is headless. Reporting is JSON + Markdown files, consumable by both humans and CI.
- WebSocket transport. Raw TCP is sufficient; the adapter can be added later if browser interop is ever needed.
- AI mass-scenario-generation inside the orchestrator. AI assists scenario authoring at design time (this session and follow-ups); the orchestrator itself does not invoke any LLM at runtime.
- Multi-host CI deployment. The protocol supports it; CI workflow changes to fan out across runners are deferred to a follow-up.

## Replaces / alongside

This harness will replace, in C6:

- `tools\run_playfab_multiplayer_live.ps1`
- `tests\godot\playfab_multiplayer_worker\`
- The inline 18 lobby + match scenarios encoded in the PowerShell.

Until C6 ships, both harnesses run side-by-side. `tools\run_all_tests.ps1 -Live` invokes both; parity is required before retirement.

## Open questions

- Whether the orchestrator integrates with `tools\run_all_tests.ps1` via a thin PowerShell wrapper (likely yes, to match the existing test stage invocation pattern) or is invoked directly. Resolved in C6 prep.
- Whether the test client uses `OS.create_process` or `OS.execute` for any sub-processes (it shouldn't need to; PlayFab + Party are in-process). Tracked here in case a future scenario needs a sub-process helper.
