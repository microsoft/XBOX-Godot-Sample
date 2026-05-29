# PlayFab Multiplayer Test Automation — Harness Spec

## Overview

This document specifies the wire protocol, connection lifecycle, framing, and runtime behaviour of the orchestrator ↔ test client harness. The companion documents are:

- `0-architecture.md` — system overview and design rationale.
- `4-scenario-authoring.md` — scenario file contract and the GDScript-side API that scenarios use.

The protocol is intentionally narrow: enough to drive multi-client scenarios reliably, no more. Anything not specified here is a protocol violation and the receiving side closes the connection.

## Transport

- TCP (no TLS). Loopback by default; bind address configurable for cross-machine use.
- Both sides use Godot's `StreamPeerTCP` (clients) and `TCPServer` (orchestrator).
- Connection lifetime is the lifetime of the test client process. Reconnection is not supported; on any disconnect the orchestrator considers the client lost and respawns it before continuing.
- The orchestrator listens on a single port and accepts multiple clients. Each connected client is identified by the `client_id` it provides during handshake (e.g. `PlayerA`, `PlayerB`, `Observer`).

## Framing

Every message is a length-prefixed JSON frame:

```
+--------+--------+--------+--------+--------+--------+--------+--------+
|              uint32-BE length (bytes of payload, max 4 MiB)           |
+--------+--------+--------+--------+--------+--------+--------+--------+
|                       UTF-8 JSON payload                              |
|                       (length bytes)                                  |
+-----------------------------------------------------------------------+
```

Notes:

- Length is big-endian unsigned 32-bit. `StreamPeerTCP::set_big_endian(true)` is the default; both sides verify.
- Max payload is 4,194,304 bytes (4 MiB). Receiving a frame larger than this is a protocol violation; receiver logs and closes.
- Zero-length payloads are a protocol violation.
- The JSON payload is always an object (`{ ... }`), never a primitive or array at the top level. The presence of the `kind` field (see below) is mandatory.
- No batching, no compression, no message-ID inside the framing layer. Correlation IDs live inside the JSON payload.

## Message kinds

Every message has a top-level `kind` field. The complete set:

| `kind` | Direction | Purpose |
| --- | --- | --- |
| `handshake.hello` | client → orchestrator | First message sent by client after TCP connect. |
| `handshake.welcome` | orchestrator → client | Sent in response to a valid `handshake.hello`. |
| `handshake.reject` | orchestrator → client | Sent in response to an invalid handshake; connection closes immediately after. |
| `request` | orchestrator → client | A command for the client to execute. |
| `response` | client → orchestrator | The result of a command. Correlated to a `request` via `correlation_id`. |
| `event` | client → orchestrator | An asynchronous notification from the client (e.g. a PlayFab signal fired). |
| `ping` | either → either | Heartbeat keepalive. |
| `pong` | either → either | Heartbeat response. |
| `shutdown` | orchestrator → client | Graceful shutdown request. Client should finish in-flight requests, perform addon shutdown, and exit. |
| `log` | client → orchestrator | Forwarded log line (level + text + optional context). Best-effort, not correlated. |

Unknown `kind` values are a protocol violation.

## Handshake

After TCP connect, the client sends `handshake.hello` within 5 seconds. Failure to send any frame in that window causes the orchestrator to close the connection.

```jsonc
// client → orchestrator
{
  "kind": "handshake.hello",
  "protocol_version": 1,
  "client_id": "PlayerA",
  "capabilities": {
    "addon_version": "0.5.0",
    "godot_version": "4.6.2.stable.official",
    "platform": "windows",
    "playfab_party_available": true,
    "playfab_multiplayer_available": true,
    "playfab_game_save_available": false
  }
}
```

On a valid hello, the orchestrator replies:

```jsonc
// orchestrator → client
{
  "kind": "handshake.welcome",
  "protocol_version": 1,
  "orchestrator_version": "1.0.0",
  "session_id": "5fa9b1c8",
  "max_payload_bytes": 4194304,
  "heartbeat_interval_ms": 10000,
  "request_timeout_default_ms": 30000
}
```

On any invalid hello (wrong `protocol_version`, missing `client_id`, duplicate `client_id`, etc.) the orchestrator replies:

```jsonc
// orchestrator → client
{
  "kind": "handshake.reject",
  "reason": "duplicate_client_id",
  "message": "client_id 'PlayerA' is already connected"
}
```

…and closes the connection. The client logs and exits with a non-zero code.

`protocol_version` is the integer wire-protocol revision. Increment by 1 when introducing any breaking change. Clients with a `protocol_version` not equal to the orchestrator's are rejected (no backwards-compat negotiation; both sides ship from the same source tree).

`capabilities` is informational. The orchestrator uses it to skip scenarios whose `REQUIRED_CAPABILITIES` are not met (see `4-scenario-authoring.md`). Capability checks happen scenario-by-scenario, not during handshake; handshake only rejects on protocol mismatch or client_id conflict.

## Heartbeat

After handshake, both sides exchange heartbeats:

```jsonc
{ "kind": "ping", "nonce": "abc123" }
```

```jsonc
{ "kind": "pong", "nonce": "abc123" }
```

Default cadence: orchestrator sends `ping` every `heartbeat_interval_ms` (default 10 000 ms). Client must reply within 5 seconds. Two missed heartbeats and the orchestrator closes the connection and marks the client lost.

Clients may also send `ping` proactively (e.g. during long-running PlayFab operations they want to confirm liveness) and the orchestrator must `pong`.

## Request / response

```jsonc
// orchestrator → client
{
  "kind": "request",
  "correlation_id": "req-0042",
  "command": "create_lobby",
  "params": {
    "as": "host_lobby",
    "max_players": 4,
    "access_policy": "public",
    "search_properties": { "string_key1": "casual" }
  },
  "timeout_ms": 30000
}
```

```jsonc
// client → orchestrator
{
  "kind": "response",
  "correlation_id": "req-0042",
  "ok": true,
  "duration_ms": 612,
  "result": {
    "handle": "host_lobby",
    "lobby_id": "82d05a95-c0fe-4d2b-ac80-28904e57f25a.r-20260323",
    "connection_string": "82d05a95-c0fe-4d2b-ac80-28904e57f25a.r-20260323|...",
    "owner_entity_key": { "id": "...", "type": "title_player_account" }
  }
}
```

Failure response:

```jsonc
{
  "kind": "response",
  "correlation_id": "req-0042",
  "ok": false,
  "duration_ms": 1830,
  "error": {
    "code": "playfab_error",
    "message": "Failed to create lobby",
    "playfab_result": {
      "hresult": "0x892357BA",
      "error_name": "InternalServerError",
      "error_message": "Internal server error"
    }
  }
}
```

Rules:

- `correlation_id` is opaque to the client; the orchestrator picks it. Format `req-NNNN` is a convention, not a requirement.
- One outstanding `request` per client at a time. The orchestrator sends `request`, awaits `response`, then sends the next. Concurrent requests against the same client are a protocol violation by the orchestrator (the runtime guarantees one at a time).
- The orchestrator may have outstanding requests against *different* clients simultaneously (e.g., `request` to PlayerA and PlayerB at the same time). This is how cross-client parallelism is expressed.
- If `timeout_ms` elapses with no `response`, the orchestrator marks the request failed, kills the client, and respawns it. The scenario fails.

## Events

Clients emit `event` messages for any PlayFab signal scenarios may wait on:

```jsonc
// client → orchestrator
{
  "kind": "event",
  "client_id": "PlayerB",
  "event_id": "evt-PlayerB-128",
  "event_type": "lobby.member_added",
  "handle": "host_lobby",
  "timestamp_ms": 1748509231414,
  "payload": {
    "member_count": 2,
    "added_member": {
      "entity_key": { "id": "...", "type": "title_player_account" },
      "is_local": false
    }
  }
}
```

Rules:

- Every event a scenario may wait on **must** be emitted; the client maintains a small fixed-size event log (default 200 most recent events) so scenarios can drain backlog after-the-fact.
- `event_type` is namespaced by service: `lobby.*`, `match.*`, `party.*`, `chat.*`, `client.*`.
- `handle` references the client-side handle that owns the source object (e.g. the lobby that fired `member_added`). May be absent for events not tied to a handle.
- `event_id` is a monotonically increasing identifier per client. Used by the orchestrator for in-order processing and de-duplication on respawn.
- Events are fire-and-forget. The orchestrator never acknowledges them.

The event-ordering pattern (subscribe before triggering) is specified in `4-scenario-authoring.md`. On the wire, all events are simply pushed by the client; the orchestrator's runtime maintains the per-`expect_event` subscription state.

## Reset

The orchestrator sends `reset_client` as a `request` after every scenario, regardless of outcome:

```jsonc
{
  "kind": "request",
  "correlation_id": "req-reset-PlayerA-0042",
  "command": "reset_client",
  "params": {},
  "timeout_ms": 15000
}
```

The client's `reset_client` handler:

1. Leaves any joined lobbies (best-effort).
2. Cancels any active match tickets (best-effort).
3. Leaves any Party networks (best-effort).
4. Disconnects any chat controls.
5. Clears the handle map.
6. Clears the event log.
7. Advances the sign-in rotation index by one. The local PlayFab
   user reference is **not** dropped here — it is dropped lazily
   on the next `sign_in` call when it detects that the requested
   custom_id differs from the cached one. See
   [`2-detailed-scenarios.md` "Sign-in pool rotation"](2-detailed-scenarios.md#sign-in-pool-rotation--why).
8. Returns `{ ok: <all subsystems reset cleanly>, result: { lobby: <lobby reset result>, match: <match reset result>, party: <party reset result>, rotation_index: <new index> } }`.

Reset does **not** itself issue a `LoginWithCustomID`; the next
scenario's first `sign_in` does that under the rotation pool's
new slot. The PlayFab entity token for the previous account is
released when the local user reference drops; the PlayFab SDK
holds the previous account's token until then.

If `reset_client` fails or times out, the orchestrator kills and respawns the client before the next scenario.

## Shutdown

```jsonc
{ "kind": "shutdown", "reason": "scenarios_complete" }
```

On receipt the client:

1. Aborts any in-flight request (sending a `response` with `ok: false, error: { code: "shutting_down" }` for whatever it had pending).
2. Performs PlayFab Multiplayer leave-all + Party leave-all + addon shutdown.
3. Closes the TCP connection.
4. Exits with code 0.

If the orchestrator does not see the connection close within 10 seconds it logs a warning and proceeds.

## Logs

```jsonc
{
  "kind": "log",
  "client_id": "PlayerA",
  "level": "info",
  "message": "create_lobby_async resolved",
  "context": { "lobby_id": "...", "duration_ms": 612 }
}
```

Log messages are written to the orchestrator's per-client log file and (at `--log-level=debug`) to stderr. They are not part of any scenario assertion path.

## Connection state machine (orchestrator side)

```
                       new TCP accept
                              │
                              ▼
                       ┌──────────────┐
                       │ awaiting_hello│  (5 s deadline)
                       └──────┬───────┘
                              │ valid hello
                              ▼
                       ┌──────────────┐
                       │ connected    │  (heartbeat active)
                       └──┬───┬───────┘
              ┌───────────┘   └──────────┐
              │ request                  │ shutdown
              ▼                          ▼
        ┌────────────┐            ┌──────────────┐
        │ in_request │            │ shutting_down│
        └─────┬──────┘            └──────┬───────┘
              │ response or timeout      │ tcp close or 10 s deadline
              ▼                          ▼
        ┌────────────┐            ┌──────────────┐
        │ connected  │            │ closed       │
        └────────────┘            └──────────────┘
```

Transitions not on this diagram (e.g. `connected → closed` on disconnect, `in_request → closed` on timeout) all mark the client as lost and trigger respawn before any further scenarios run.

## Connection state machine (client side)

```
                       process start
                              │
                              ▼
                       ┌──────────────┐
                       │ connecting   │  (retry every 1 s for 30 s)
                       └──────┬───────┘
                              │ tcp connect
                              ▼
                       ┌──────────────┐
                       │ awaiting_welc│  (5 s deadline)
                       └──────┬───────┘
                              │ welcome
                              ▼
                       ┌──────────────┐
                       │ ready        │  (command loop, heartbeat)
                       └──────┬───────┘
                              │ shutdown
                              ▼
                       ┌──────────────┐
                       │ shutting_down│
                       └──────┬───────┘
                              │ addon shutdown done
                              ▼
                       ┌──────────────┐
                       │ exit         │
                       └──────────────┘
```

## Buffering and partial reads

Each side maintains a read buffer per connection. The framing parser:

1. Reads available bytes into the buffer (`StreamPeerTCP::get_partial_data`).
2. If the buffer has fewer than 4 bytes, returns.
3. Peeks at the first 4 bytes for the uint32-BE length.
4. If length > `max_payload_bytes`, logs and closes the connection.
5. If the buffer has fewer than `4 + length` bytes, returns.
6. Slices the JSON payload out of the buffer.
7. Parses the JSON. If parse fails, logs and closes the connection.
8. Dispatches by `kind`.
9. Loops back to step 2 to drain any remaining frames in the buffer.

Each side maintains a write queue. Writes are non-blocking; the orchestrator's main loop drains the write queue on each frame. Backpressure: if a client's write queue exceeds 1024 unsent frames, the orchestrator logs and closes the connection (treated as a hung client).

The orchestrator never blocks on a slow client. Head-of-line blocking is avoided by:

- Per-client request slot (the runtime tracks "outstanding request" per client and only sends the next when the previous response arrives).
- Independent processing of events (events do not occupy the request slot).
- Per-client read/write buffers (no shared mutexes between client connections).

## Error codes (response envelope)

`error.code` is one of:

| Code | Meaning |
| --- | --- |
| `playfab_error` | A PlayFab API call returned a non-OK `PlayFabResult`. The original `PlayFabResult` is in `error.playfab_result`. Rate-limit cases (`0x892354DD` / HTTP 429) surface here only after the test-client's reactive 8-attempt retry budget (`playfab_runtime.gd::await_completion_with_rate_limit_retry`) is exhausted; the proactive sliding-window pacer (`_reserve_call_slot`) and the per-scenario account rotation pool keep this path rare. Repeated rate-limit `playfab_error`s usually mean the per-role account pool was exhausted within the same 120s window. |
| `convergence_timeout` | A lobby property write (`set_lobby_properties` / `set_member_properties`) was ACK'd by the service but the local SDK lobby cache did not converge on the new values within `playfab_lobby_ops.gd::CONVERGENCE_TIMEOUT_MS` (default 10s). For member-property writes the helper polls the local member entry via `PlayFabLobbyMember.is_local_member()`; if a future regression hides the local member from `get_members()` again this code surfaces with `last=<null>`. Diagnostic only; the service-side write succeeded. |
| `match_ticket_id_timeout` | `create_match_ticket` returned a tracked `PlayFabMatchTicket` whose `ticket_id` did not populate from the SDK's first `TicketStatusChanged` dispatch within `playfab_match_ops.gd::TICKET_ID_POPULATE_TIMEOUT_MS` (default 20s). The half-formed ticket is rolled back via `cancel_async` before the error is returned so `reset_client` never sees an id-less handle. |
| `not_signed_in` | The command requires a signed-in user; no `sign_in` has been performed in this process. |
| `not_initialized` | The command requires PlayFab Multiplayer or Party to be initialized; `initialize` has not been called. |
| `unknown_handle` | The command referenced a `handle` that does not exist in the client's handle map. |
| `invalid_params` | The command's `params` failed validation (missing required field, wrong type, out of range). `error.details` includes the failing field name. |
| `unknown_command` | The client does not recognize the `command` name. |
| `shutting_down` | The client received `shutdown` while this request was in-flight. |
| `internal_error` | Unhandled exception in the command handler. `error.details.stack_trace` includes the GDScript stack. |

## Versioning

`protocol_version` increments by 1 on any breaking change. There is no backwards compatibility on the wire — orchestrator and client always ship from the same source tree, so version skew is a bug.

Both sides hardcode the current `protocol_version` in a single constant (`orchestrator/wire/protocol.gd` and `mp_test_client/wire/protocol.gd`). The constant lives in shared code copied/mirrored between the two projects via the same CMake mirror pattern used for `gut` and addons.

## Notes for implementation

- `StreamPeerTCP::set_no_delay(true)` is set on every accepted connection and every client connection to disable Nagle's algorithm. Scenarios send small frequent messages; latency matters more than bandwidth.
- The orchestrator's main loop runs at 60 Hz (`SceneTree.process` or equivalent). Within each tick: drain read buffers, dispatch frames, drain write queues, advance scenario runner state machine.
- The orchestrator should never call `await` inside the main loop without a corresponding timeout. All `await` paths have a `timeout_ms` parameter; on timeout the path takes its failure branch.
- For `--auto-spawn N`, the orchestrator uses `OS.create_process` to launch test client processes. The orchestrator does **not** monitor the child PIDs after spawn; the TCP connection lifecycle is the only signal of client health. On client crash, the TCP socket closes, which triggers respawn.
