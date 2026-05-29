# PlayFab Multiplayer Test Automation — Scenario Authoring

## Overview

This document specifies how to write a scenario file for the orchestrator. Each scenario is a single `.gd` file in `tests\godot\mp_orchestrator\scenarios\`. The orchestrator discovers files at startup, validates metadata, and runs them sequentially against the connected test clients.

The companion documents are:

- `0-architecture.md` — system overview and design rationale.
- `3-harness-spec.md` — wire protocol between orchestrator and test clients.

## Scenario contract

A scenario file is a GDScript file extending `MpScenarioBase`. It declares metadata as constants, optionally overrides `setup()` / `cleanup()`, and implements `run()`.

```gdscript
# tests/godot/mp_orchestrator/scenarios/lobby/lobby_create_public_smoke.gd
extends "res://scenarios/_base/mp_scenario_base.gd"

const SCENARIO_ID := "lobby.create.public.smoke"
const SCENARIO_NAME := "Public lobby create snapshot"
const CATEGORY := "functional"
const PRIORITY := "P0"
const REQUIRED_ROLES := ["host"]
const REQUIRED_CAPABILITIES := ["playfab_multiplayer_available"]
const TIMEOUT_SEC := 60

func run(orch: TestOrchestrator) -> Dictionary:
    var host := orch.client("host")

    var create_result := await host.send("create_lobby", {
        "as": "smoke_lobby",
        "max_players": 4,
        "access_policy": "public",
        "search_properties": { "string_key1": "casual" },
    })
    if not create_result.ok:
        return fail("create_lobby failed", { "error": create_result.error })

    var lobby_id: String = create_result.result.lobby_id
    if lobby_id.is_empty():
        return fail("lobby_id was empty")

    var snapshot := await host.send("get_lobby_snapshot", { "handle": "smoke_lobby" })
    if not snapshot.ok:
        return fail("get_lobby_snapshot failed", { "error": snapshot.error })

    if int(snapshot.result.member_count) != 1:
        return fail("expected member_count=1, got %d" % snapshot.result.member_count)

    return ok({ "lobby_id": lobby_id })
```

## Metadata constants

| Constant | Type | Required | Meaning |
| --- | --- | --- | --- |
| `SCENARIO_ID` | `String` | yes | Globally unique, dotted identifier. Must match `^[a-z0-9_]+(\.[a-z0-9_]+)*$`. Convention: `<service>.<group>.<name>.<variant>`. A leading-underscore service segment (`_smoke.*`) is reserved for harness self-tests and sorts before all real scenarios. |
| `SCENARIO_NAME` | `String` | yes | Human-readable, suitable for the results report. |
| `CATEGORY` | `String` | yes | Free-form string label, recorded verbatim in the results report. In-repo convention pairs the test-pyramid label (`functional`, `boundary`, `negative`, `integration`, `chaos`, `state_transitions`, `e2e`) for the smoke + cross-service tier with the service-namespaced label (`lobby`, `match`, `party`, `cross_service`) for the per-service tier. New scenarios should match an existing label rather than coin a fresh one. |
| `PRIORITY` | `String` | yes | One of `P0`, `P1`, `P2`, `P3`. |
| `REQUIRED_ROLES` | `Array[String]` | yes | The role names the scenario needs. The orchestrator skips the scenario with reason `missing_roles` if any required role is not connected. |
| `REQUIRED_CAPABILITIES` | `Array[String]` | no | Capability names from the client handshake's `capabilities` block. Default: `["playfab_multiplayer_available"]`. |
| `TIMEOUT_SEC` | `int` | no | Per-scenario wall-clock timeout. Default: 60. |
| `QUARANTINED` | `bool` | no | If true, the scenario runs but a failure does not fail the orchestrator exit code. Default: false. Use during diagnosis of a flaky scenario; track in a follow-up issue. |
| `TAGS` | `Array[String]` | no | Free-form labels for filtering, e.g. `["smoke", "rate_limited"]`. |

Metadata is validated at discovery time. Any missing required constant or invalid value causes the scenario to be reported as `invalid_metadata` and excluded from the run.

## Lifecycle methods

```gdscript
# Optional. Called once before run(). Can return a failure to skip the scenario.
func setup(orch: TestOrchestrator) -> Dictionary:
    return ok()

# Required. The scenario body. Must return ok() or fail().
func run(orch: TestOrchestrator) -> Dictionary:
    ...

# Optional. Called after run(), regardless of outcome. Best-effort.
# Use for scenario-owned cleanup beyond reset_client (e.g., title-side cleanup
# via a PlayFab admin call). Do not rely on this for handle cleanup — the
# orchestrator's mandatory reset_client handles handle/state teardown.
func cleanup(orch: TestOrchestrator) -> void:
    pass
```

The orchestrator always sends `reset_client` to every connected client after `cleanup()` returns (or after `run()` if `cleanup()` is not defined), regardless of pass/fail. Scenarios do not need to call leave_lobby/leave_party manually for cleanup; that's the harness's job.

## Helpers from `MpScenarioBase`

```gdscript
# Wrap a successful result.
func ok(details: Dictionary = {}) -> Dictionary

# Wrap a failure result.
func fail(reason: String, details: Dictionary = {}) -> Dictionary

# Mark the scenario as deliberately skipped at runtime. Counts as skipped,
# not failed. Use when a precondition discovered at runtime makes the
# scenario inapplicable (e.g., title not configured for matchmaking).
func skip(reason: String) -> Dictionary

# Convenience: assert helpers that build the failure message with line info.
# All return a Dictionary; if the assertion fails the Dictionary is the
# failure result and run() should `return` it immediately.
func assert_eq(actual: Variant, expected: Variant, message: String = "") -> Dictionary
func assert_true(condition: bool, message: String = "") -> Dictionary
func assert_false(condition: bool, message: String = "") -> Dictionary
func assert_has(dict: Dictionary, key: String, message: String = "") -> Dictionary
func assert_ok(response: Dictionary, message: String = "") -> Dictionary  # asserts response.ok is true
```

The assertion helpers return `null` on success and a failure Dictionary on failure. Typical pattern:

```gdscript
var err := assert_eq(snapshot.result.member_count, 1, "expected single member after create")
if err: return err
```

## Orchestrator API surface

The `TestOrchestrator` passed to `run()` is a thin facade over the harness. Scenarios should treat it as a god-object intentionally — keeping orchestrator API on one object simplifies scenario authoring.

### Client addressing

```gdscript
# Get the client proxy for a connected role.
func client(role: String) -> ClientProxy
```

Returns a `ClientProxy` for the named role. If the role is not connected (which shouldn't happen if `REQUIRED_ROLES` is honest), the proxy methods all fail with `client_not_connected`.

```gdscript
# Get the set of currently connected role names.
func connected_roles() -> Array[String]
```

### Sending commands

Scenarios send commands via the client proxy:

```gdscript
var host := orch.client("host")

# Send a command, await its response. Returns Dictionary:
#   { ok: bool, duration_ms: int, result: Dictionary, error: Dictionary }
var response := await host.send(command_name: String, params: Dictionary, timeout_ms: int = 30000)
```

The `host.send(...)` call returns a coroutine that resolves when the client's `response` frame arrives or the timeout elapses. On timeout, the returned Dictionary has `ok = false, error = { code: "timeout", ... }` and the orchestrator marks the client lost for the next reset.

### Waiting for events

The event-ordering pattern is "subscribe before triggering":

```gdscript
var host := orch.client("host")
var client := orch.client("guest")

# Subscribe BEFORE the trigger. expect_event returns a waiter object;
# do not await it yet.
var member_added := client.expect_event("lobby.member_added", { "handle": "shared_lobby" })

# Trigger the action that should produce the event.
var join_result := await client.send("join_lobby", {
    "connection_string": create_result.result.connection_string,
    "as": "shared_lobby",
})
if not join_result.ok: return fail("join failed", { "error": join_result.error })

# Now await the waiter. Returns Dictionary { ok, event, timed_out }.
var event_result := await member_added.wait(timeout_ms = 10000)
if event_result.timed_out:
    return fail("lobby.member_added did not fire on guest within 10s")
```

The waiter is created in `client.expect_event(...)` and immediately starts buffering matching events. By the time the scenario `await`s the waiter, any event fired between subscribe and await is already in the waiter's queue.

Filters in `expect_event(event_type, filter)`:

- `event_type` is the dotted event type (e.g., `lobby.member_added`).
- `filter` is an optional Dictionary; the event matches only if every key in `filter` is present in the event's `payload` (or top-level fields) with the matching value. Filters are equality-only; use a custom `expect_event_where(event_type, predicate: Callable)` for complex matching.

### Parallel commands

```gdscript
var host := orch.client("host")
var guest := orch.client("guest")

# Fan out: both clients perform an action in parallel.
var host_op := host.send("create_lobby", { "as": "host_lobby" })
var guest_op := guest.send("search_lobbies", { "filter": "..." })

var host_result := await host_op
var guest_result := await guest_op
```

Coroutines are independent because they target different clients. Within a single client, only one outstanding request at a time is permitted by the wire protocol.

### Convenience helpers

```gdscript
# Wait for a predicate against a client's snapshot to become true.
# Polls every poll_ms; returns ok or timeout.
await orch.wait_until(client("guest"), func(state): return state.lobby_count == 1, timeout_ms = 5000, poll_ms = 100)

# Capture all events of a type for the duration of a block.
var events := await orch.capture_events("guest", "chat.text_message_received", func():
    await host.send("send_chat", { "handle": "host_lobby", "text": "hello" })
)
# events is Array[Dictionary] of all chat.text_message_received events on guest
# fired between capture start and the inner coroutine returning.
```

### Logging from scenarios

```gdscript
orch.log("info", "Host created lobby", { "lobby_id": create_result.result.lobby_id })
orch.log("warn", "Guest snapshot stale, retrying")
orch.log("error", "Unexpected owner after migration")
```

Scenario logs go to the run's per-scenario log file and (at debug level) to stderr. They are not part of the assertion path.

## Worked examples

### 1. Two-client join with explicit handles and event ordering

```gdscript
# scenarios/lobby/lobby_join_by_connection_string.gd
extends "res://scenarios/_base/mp_scenario_base.gd"

const SCENARIO_ID := "lobby.join.by_connection_string"
const SCENARIO_NAME := "Client joins by connection string"
const CATEGORY := "functional"
const PRIORITY := "P0"
const REQUIRED_ROLES := ["host", "guest"]

func run(orch: TestOrchestrator) -> Dictionary:
    var host := orch.client("host")
    var guest := orch.client("guest")

    var create := await host.send("create_lobby", {
        "as": "shared_lobby",
        "max_players": 4,
        "access_policy": "public",
    })
    var err := assert_ok(create, "host create_lobby")
    if err: return err

    var member_added := guest.expect_event("lobby.member_added", { "handle": "shared_lobby" })

    var join := await guest.send("join_lobby", {
        "connection_string": create.result.connection_string,
        "as": "shared_lobby",
    })
    err = assert_ok(join, "guest join_lobby")
    if err: return err

    var evt := await member_added.wait(10000)
    if evt.timed_out:
        return fail("guest did not observe its own lobby.member_added")

    var host_snapshot := await host.send("get_lobby_snapshot", { "handle": "shared_lobby" })
    err = assert_eq(host_snapshot.result.member_count, 2, "host should see 2 members")
    if err: return err

    return ok()
```

### 2. Negative scenario: invalid connection string

```gdscript
# scenarios/lobby/lobby_join_invalid_connection_string.gd
extends "res://scenarios/_base/mp_scenario_base.gd"

const SCENARIO_ID := "lobby.join.invalid_connection_string"
const SCENARIO_NAME := "Join with invalid connection string fails predictably"
const CATEGORY := "negative"
const PRIORITY := "P1"
const REQUIRED_ROLES := ["guest"]

func run(orch: TestOrchestrator) -> Dictionary:
    var guest := orch.client("guest")

    var join := await guest.send("join_lobby", {
        "connection_string": "not-a-real-string",
        "as": "would_be_lobby",
    })
    if join.ok:
        return fail("expected join to fail; got success")

    var err := assert_eq(join.error.code, "playfab_error", "wrong error code shape")
    if err: return err
    err = assert_true(
        join.error.playfab_result.hresult.begins_with("0x"),
        "expected an HRESULT-shaped error"
    )
    if err: return err

    return ok({ "observed_hresult": join.error.playfab_result.hresult })
```

### 3. Match ticket create-and-cancel

```gdscript
# scenarios/match/match_ticket_create_and_cancel.gd
extends "res://scenarios/_base/mp_scenario_base.gd"

const SCENARIO_ID := "match.ticket.create_and_cancel"
const SCENARIO_NAME := "Match ticket create and cancel"
const CATEGORY := "functional"
const PRIORITY := "P0"
const REQUIRED_ROLES := ["host"]
const REQUIRED_CAPABILITIES := ["playfab_multiplayer_available", "matchmaking_queue_configured"]

func run(orch: TestOrchestrator) -> Dictionary:
    var host := orch.client("host")

    var create := await host.send("create_match_ticket", {
        "as": "smoke_ticket",
        "queue_name": orch.env("PLAYFAB_MULTIPLAYER_MATCH_QUEUE"),
        "timeout_seconds": 60,
        "attributes": { "skill": 1 },
    })
    var err := assert_ok(create, "create_match_ticket")
    if err: return err

    var cancelled := host.expect_event("match.status_changed", {
        "handle": "smoke_ticket",
        "status": "cancelled",
    })

    var cancel := await host.send("cancel_match_ticket", { "handle": "smoke_ticket" })
    err = assert_ok(cancel, "cancel_match_ticket")
    if err: return err

    var evt := await cancelled.wait(10000)
    if evt.timed_out:
        return fail("did not observe match.status_changed=cancelled within 10s")

    return ok({ "ticket_id": create.result.ticket_id })
```

### 4. Party RPC round-trip (regression coverage for PR #132 / issue #133)

```gdscript
# scenarios/party/party_rpc_round_trip.gd
extends "res://scenarios/_base/mp_scenario_base.gd"

const SCENARIO_ID := "party.rpc.round_trip.post_join_first_message"
const SCENARIO_NAME := "First RPC after client join arrives at host (PR #132 regression)"
const CATEGORY := "integration"
const PRIORITY := "P0"
const REQUIRED_ROLES := ["host", "guest"]
const REQUIRED_CAPABILITIES := ["playfab_party_available"]

func run(orch: TestOrchestrator) -> Dictionary:
    var host := orch.client("host")
    var guest := orch.client("guest")

    var party := await host.send("party_create_network", {
        "as": "party",
        "invitation_id": "auto",
        "enable_text_chat": false,
    })
    var err := assert_ok(party, "host party_create_network")
    if err: return err

    var peer_connected := host.expect_event("party.peer_connected", { "handle": "party" })

    var join := await guest.send("party_join_network", {
        "as": "party",
        "descriptor": party.result.network_descriptor,
        "invitation_id": party.result.invitation_id,
    })
    err = assert_ok(join, "guest party_join_network")
    if err: return err

    var connected := await peer_connected.wait(15000)
    if connected.timed_out:
        return fail("host did not observe party.peer_connected within 15s")

    # Regression: first RPC after join must arrive at host.
    var ping_received := host.expect_event("party.rpc.ping_received", { "handle": "party" })

    var send := await guest.send("party_send_rpc_ping", { "handle": "party" })
    err = assert_ok(send, "guest party_send_rpc_ping")
    if err: return err

    var arrived := await ping_received.wait(5000)
    if arrived.timed_out:
        return fail("host did not receive ping RPC within 5s (PR #132 regression)")

    return ok({ "ping_payload": arrived.event.payload })
```

## Scenario discovery and ordering

The orchestrator scans `scenarios/` recursively for `.gd` files. Files whose top-level script omits `SCENARIO_ID` / `SCENARIO_NAME` are silently treated as shared helpers (e.g., the base class under `_base/`, future helpers under `_helpers/`) and not enrolled as scenarios. Each remaining file:

1. Is loaded via `load(path)`.
2. Is instantiated via `.new()`.
3. Is checked for the required metadata constants.
4. Is added to the run queue if metadata is valid; otherwise reported as `invalid_metadata` and skipped.

Run ordering:

- Within a single run, scenarios execute in priority order (P0 → P1 → P2 → P3).
- Within a priority band, scenarios execute in `SCENARIO_ID` lexicographic order. Stable, reproducible.
- Scenarios with `--filter <regex>` matching `SCENARIO_ID` are the only ones queued.
- `--list` prints the queue without running anything.

## Results format

After the run, the orchestrator writes two files in `--results-dir`:

- `mp-test-results.json` — machine-readable, full detail.
- `mp-test-results.md` — human-readable summary plus per-scenario detail.

The JSON shape is documented in the orchestrator's source as the schema is small and stable; the headline fields are `total`, `passed`, `failed`, `skipped`, `quarantined_failures`, and a per-scenario array with `id`, `name`, `status`, `duration_ms`, `failure_reason`, and `details`.

## Common pitfalls

- **`wait_for_signal` then trigger** — always wrong. Always subscribe via `expect_event` *before* sending the triggering command. The wire protocol is documented to support this; using it the other way around races every time.
- **Implicit "primary" state on the client** — always pass `as: "name"` on create commands and `handle: "name"` on follow-up commands. Implicit globals work for smoke tests and break for multi-lobby coverage.
- **Forgetting to await** — every `client.send(...)` returns a coroutine. Forgetting `await` does not error at parse time but races the response into the next scenario.
- **Long unbounded waits** — every `wait()` and `wait_until()` requires a `timeout_ms`. The default `TIMEOUT_SEC = 60` only catches catastrophic hangs; per-wait timeouts catch the cases where the scenario "almost works".
- **Mutating fixture state** — scenarios run sequentially and `reset_client` runs between them, but anything stored title-side (PlayFab profiles, statistics, leaderboards) persists. Scenarios that mutate title-side state must clean up in their `cleanup()` hook, and live-write coverage stays gated behind `LIVE_WRITE_TESTS=1` (orchestrator surfaces this as a capability gate `live_write_allowed`).

## Style

- One scenario per file. Multiple scenarios per file are not discovered.
- File name should match `SCENARIO_ID` with dots replaced by underscores, in the appropriate `<service>/` subdirectory. Example: `lobby.create.public.smoke` → `scenarios/lobby/lobby_create_public_smoke.gd`.
- Keep scenarios under ~80 lines. If a scenario needs more, extract helpers into `scenarios/_helpers/`.
- Comments are sparse; the scenario name and step shape should be self-describing. A short comment is fine to explain a regression citation (e.g., `# Regression for PR #132`).
- No `print` statements; use `orch.log(level, message, context)`.
