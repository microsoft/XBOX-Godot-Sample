# PlayFab Multiplayer Test Automation — Detailed Scenarios

## Overview

This document gives step-by-step API sequences for the top 28 P0/P1 scenarios from `1-test-matrix.md`. Each entry is detailed enough that an author can mechanically translate it into a scenario `.gd` file as described in `4-scenario-authoring.md`.

The remaining P0/P1 entries from the matrix (not detailed here) are intentionally left to authors during C5 mass production. They follow the same patterns as the detailed entries, and the matrix already names them, their roles, and their capability gates.

## Scenario format

Each detailed scenario follows this layout:

- **ID**, **name**, **priority**, **category**, **roles**, **capabilities** — copied from the matrix.
- **Goal** — one-sentence statement of what the scenario proves.
- **Preconditions** — assumptions beyond clients being signed in.
- **Steps** — numbered orchestrator-side actions in order. Each step is either:
  - `send(role, command, params)` — fire a command and await the response.
  - `expect_event(role, event_type, filter)` — subscribe to an event before triggering it.
  - `await waiter` — block on a previously-subscribed waiter.
  - `assert ...` — orchestrator-side assertion (typically about response or event payload).
- **Cleanup** — anything beyond `reset_client`.
- **Notes** — extra context: error code, timing, regression citation, etc.

The command vocabulary used in the steps is the working test-client command surface defined in `3-harness-spec.md` and exemplified in `4-scenario-authoring.md`. Commands roll out incrementally: `sign_in` and `_smoke.ping` land in C3; lobby commands (`create_lobby`, `join_lobby`, `set_lobby_properties`, `set_member_properties`, `get_lobby_snapshot`, `leave_lobby`, `search_lobbies`) in C4b; match-ticket commands (`create_match_ticket`, `get_match_ticket_snapshot`, `cancel_match_ticket`) in C4c; Party commands in C4d. Until the matching block ships, the client returns `unknown_command` and any scenario depending on the missing surface either skips or fails fast — that is the expected behavior, not a bug.

## Command vocabulary (working set)

These are the commands a scenario can send to a client. The test client maintains a handle → resource map (lobby_id, ticket_id, network) and resolves handles per command.

### Identity

| Command | Params | Returns |
| --- | --- | --- |
| `sign_in` | `{ custom_id?: String, custom_id_suffix?: String, create_account?: bool }` | `{ entity_id, entity_type, custom_id }` |

`sign_in` is idempotent at the (process, custom_id) level: if a scenario calls `sign_in` again with the same custom_id, the test client returns the cached payload without a PlayFab call. If a scenario calls `sign_in` with a **different** custom_id — which is the expected path between scenarios because of the rotation pool described below — the test client transparently drops its local user reference and per-op rate-limit history, then issues a fresh `LoginWithCustomID`. The PlayFab SDK supports multiple resident user tokens; only the most recent one is referenced by subsequent per-user calls (`create_lobby_async(user, …)` etc.).

Custom-id resolution (in priority order):

1. Explicit `params.custom_id` — used as-is.
2. `params.custom_id_suffix` — appended to the per-process prefix.
3. Role-derived default with **rotation-pool slot** — the test client looks up the role-to-account map (`host → host`, `guest → client`, `guest2 → client2`, `observer → observer`), then appends the current rotation slot (1..ROTATION_POOL_SIZE) to form `<prefix>-<account>-<slot>`. The slot advances every `reset_client` (i.e. between scenarios) so successive scenarios sign in as the next account in the pool.

The per-process prefix comes from `PLAYFAB_MULTIPLAYER_CUSTOM_ID_PREFIX` (if set) or `<PLAYFAB_CUSTOM_ID>-multiplayer`, last-resort `godot-gdk-ext-live-smoke-multiplayer`. `tools/configure_playfab_test_title.ps1::Ensure-MultiplayerWorkerAccounts` provisions both naming styles per title:

- **Legacy unsuffixed** (one per role): `<prefix>-host`, `<prefix>-client`, `<prefix>-client2`, `<prefix>-observer`. Used by the retiring `tools/run_playfab_multiplayer_live.ps1` runner.
- **Rotation pool** (POOL_SIZE per role, default 4): `<prefix>-host-1` … `<prefix>-host-4`, plus the same `-1..4` suffixes for client / client2 / observer. Total: 4 roles × POOL_SIZE = 16 pooled accounts. Used by `tests/godot/mp_test_client/scripts/test_client.gd::_derive_custom_id_for_role`.

#### Sign-in pool rotation — why

PlayFab Multiplayer service quotas are scoped per `title_player_account`. The most restrictive lobby limit is `create_lobby = 6 calls / 120s per account`; with a single account hosting every scenario sequentially, a suite of ~11 lobby scenarios saturates the window and forces ~60s of waiting before each subsequent create. Rotating the host role across a 4-account pool multiplies the per-window create budget by 4× (24 creates / 120s) — comfortably above the actual scenario rate without any waiting.

Each test-client process tracks its own `_rotation_index` (zero at startup, incremented by `_handle_reset_client` after lobby/match/party state is cleared). The pool slot is `(_rotation_index % ROTATION_POOL_SIZE) + 1`. Roles are independent: the host process rotates host-1 → host-2 → … → host-4 → host-1; the guest process rotates client-1 → client-2 → … in parallel.

As a backstop, the test client also runs a proactive per-(account, endpoint) sliding-window pacer (`playfab_runtime.gd::_reserve_call_slot`, sourced from `RATE_BUDGETS`). On the rare path where rotation alone isn't enough (the pool exhausts under sustained load, or an SDK-internal call hides under a known endpoint), the pacer sleeps until the budget recovers. If both layers miss, the existing 8-attempt reactive retry on `0x892354DD`/HTTP 429 still catches it.

Default `create_account` is `false` — production sandboxes typically disable on-the-fly account creation (`E_PF_PLAYER_CREATION_DISABLED`); scenarios assume the configure script has pre-provisioned all 16 pooled accounts. Set `create_account: true` only for negative-tests that intentionally exercise the creation path.

Scenarios normally call `await client.send("sign_in", {}, 60_000)` with empty params and let the test client derive the custom_id from the role + current rotation slot — the same pattern used by `_smoke.signin` and every C4/C5 lobby/match/party scenario.

### Lobby

| Command | Params | Returns |
| --- | --- | --- |
| `create_lobby` | `{ as: String, max_players?: int, access_policy?: String, owner_migration_policy?: String, lobby_properties?: Dictionary, member_properties?: Dictionary, search_properties?: Dictionary }` | `{ handle, lobby_id, connection_string }` |
| `join_lobby` | `{ as: String, connection_string: String, member_properties?: Dictionary }` | `{ handle, lobby_id }` |
| `join_arranged_lobby` | `{ as: String, connection_string: String, member_properties?: Dictionary }` | `{ handle, lobby_id }` |
| `set_lobby_properties` | `{ handle: String, properties: Dictionary }` | `{}` |
| `set_member_properties` | `{ handle: String, properties: Dictionary }` | `{}` |
| `get_lobby_snapshot` | `{ handle: String }` | `{ lobby_id, owner_entity_key, member_count, members, properties, search_properties }` |
| `leave_lobby` | `{ handle: String }` | `{}` |
| `search_lobbies` | `{ filter?: String, order_by?: String, max_results?: int }` | `{ lobbies: Array }` |

### Match

| Command | Params | Returns |
| --- | --- | --- |
| `create_match_ticket` | `{ as: String, queue_name: String, timeout_seconds?: int, attributes?: Dictionary }` | `{ handle, ticket_id }` |
| `get_match_ticket_snapshot` | `{ handle: String }` | `{ ticket_id, status, members, match? }` |
| `cancel_match_ticket` | `{ handle?: String, ticket_id?: String }` | `{}` |

### Party

| Command | Params | Returns |
| --- | --- | --- |
| `party_create_network` | `{ as: String, max_players?: int, direct_peer_connectivity?: int, invitation_id?: String, enable_text_chat?: bool, enable_voice_chat?: bool }` | `{ handle, network_descriptor, invitation_id, local_peer_id }` |
| `party_join_network` | `{ as: String, descriptor: String, invitation_id?: String }` | `{ handle, local_peer_id }` |
| `party_leave_network` | `{ handle: String }` | `{}` |
| `party_send_rpc_ping` | `{ handle: String, payload?: Dictionary }` | `{ correlation_id }` |
| `party_send_chat_text` | `{ handle: String, text: String, target_peer_ids?: Array }` | `{}` |
| `party_get_network_snapshot` | `{ handle: String }` | `{ peer_count, peers, local_peer_id }` |

### Events surfaced by clients

| Event type | Payload keys | Fired by |
| --- | --- | --- |
| `lobby.created` | `handle, lobby_id` | client that created |
| `lobby.member_added` | `handle, lobby_id, member_entity_key` | every member |
| `lobby.member_removed` | `handle, lobby_id, member_entity_key, reason` | every surviving member, and the leaver once |
| `lobby.member_updated` | `handle, lobby_id, member_entity_key, properties` | every member |
| `lobby.properties_updated` | `handle, lobby_id, properties` | every member |
| `lobby.owner_changed` | `handle, lobby_id, old_owner, new_owner` | every surviving member |
| `lobby.disconnected` | `handle, lobby_id, reason` | every member |
| `match.status_changed` | `handle, ticket_id, status, match?` | the ticket owner |
| `party.network_ready` | `handle, network_descriptor, local_peer_id` | every connecting client |
| `party.peer_connected` | `handle, peer_id, entity_key` | every existing peer |
| `party.peer_disconnected` | `handle, peer_id, entity_key, reason` | every surviving peer |
| `party.network_destroyed` | `handle, reason` | every surviving peer |
| `party.rpc.ping_received` | `handle, sender_peer_id, payload, correlation_id` | receiver of `party_send_rpc_ping` |
| `party.rpc.pong_received` | `handle, sender_peer_id, payload, correlation_id` | original ping sender, after receiver returns pong |
| `party.chat.text_message_received` | `handle, sender_peer_id, text` | every recipient |

## Lobby — P0 detailed scenarios

### `lobby.create.public.smoke`

- **Roles**: host
- **Goal**: Public-access lobby created with default options exposes a connection string and a single-member snapshot.
- **Steps**:
  1. `send(host, create_lobby, { as: "smoke_lobby", max_players: 4, access_policy: "public" })`.
  2. Assert `response.ok` and `response.result.lobby_id != ""`.
  3. Assert `response.result.connection_string != ""`.
  4. `send(host, get_lobby_snapshot, { handle: "smoke_lobby" })`.
  5. Assert `snapshot.result.member_count == 1`.
  6. Assert `snapshot.result.members[0].entity_key.id == orch.client(host).entity_id`.
- **Notes**: Port of legacy `public lobby create snapshot`.

### `lobby.join.by_connection_string`

- **Roles**: host, guest
- **Goal**: Guest joins via connection string; both sides observe `lobby.member_added`.
- **Steps**:
  1. `send(host, create_lobby, { as: "shared_lobby", max_players: 4, access_policy: "public" })`.
  2. `host_added = expect_event(host, lobby.member_added, { handle: "shared_lobby" })` — subscribe before guest join.
  3. `guest_added = expect_event(guest, lobby.member_added, { handle: "shared_lobby" })`.
  4. `send(guest, join_lobby, { as: "shared_lobby", connection_string: create.result.connection_string })`.
  5. `await host_added.wait(10000)`; assert not timed out.
  6. `await guest_added.wait(10000)`; assert not timed out.
  7. `send(host, get_lobby_snapshot, { handle: "shared_lobby" })`; assert `member_count == 2`.
- **Notes**: Port of legacy `client join by connection string`.

### `lobby.join.three_clients`

- **Roles**: host, guest, guest2
- **Goal**: All three clients see each other; snapshots converge to 3 members.
- **Steps**:
  1. `send(host, create_lobby, { as: "tri_lobby", max_players: 4, access_policy: "public" })`.
  2. Subscribe `host_added` and `guest_added` for `member_added`; trigger `send(guest, join_lobby, ...)`; await both.
  3. Subscribe `host_added2`, `guest_added2`, `guest2_added2` for `member_added` on the second join; trigger `send(guest2, join_lobby, ...)`; await all three.
  4. `send(host, get_lobby_snapshot, { handle: "tri_lobby" })`; assert `member_count == 3`.
  5. `send(guest, get_lobby_snapshot, { handle: "tri_lobby" })`; assert `member_count == 3`.
  6. `send(guest2, get_lobby_snapshot, { handle: "tri_lobby" })`; assert `member_count == 3`.
- **Notes**: Port of legacy `three-client membership snapshots`. Member ordering is not asserted; only entity-key membership.

### `lobby.search.public.by_string_key`

- **Roles**: host, observer
- **Goal**: A public lobby with a unique `string_key1` is found by exact-match OData search.
- **Steps**:
  1. Generate a unique tag: `tag = "smoke-" + orch.run_id + "-" + scenario_id`.
  2. `send(host, create_lobby, { as: "tagged_lobby", access_policy: "public", search_properties: { "string_key1": tag } })`.
  3. `send(observer, search_lobbies, { filter: "string_key1 eq '" + tag + "'", max_results: 10 })`.
  4. Assert `response.result.lobbies.size() == 1` and `lobbies[0].lobby_id == create.result.lobby_id`.
- **Notes**: Port of legacy `public lobby search by string key`. Uses `string_key1` because the typed string bucket is searchable.

### `lobby.search.private.not_searchable`

- **Roles**: host, observer
- **Goal**: A private lobby with the same tag is not returned by search.
- **Steps**:
  1. Generate tag as above.
  2. `send(host, create_lobby, { as: "private_lobby", access_policy: "private", search_properties: { "string_key1": tag } })`.
  3. `send(observer, search_lobbies, { filter: "string_key1 eq '" + tag + "'" })`.
  4. Assert `response.result.lobbies.size() == 0`.
- **Notes**: Port of legacy `private lobby not searchable`.

### `lobby.properties.lobby.propagation`

- **Roles**: host, guest, guest2
- **Goal**: Host-set lobby property propagates to all members via `lobby.properties_updated`.
- **Steps**:
  1. Create lobby, join guest, join guest2 (per join.three_clients pattern through step 3).
  2. Subscribe `host_props`, `guest_props`, `guest2_props` for `lobby.properties_updated` with `{ handle: "tri_lobby" }`.
  3. `send(host, set_lobby_properties, { handle: "tri_lobby", properties: { "round": "1" } })`.
  4. `await host_props`, `guest_props`, `guest2_props`, each with 10s timeout.
  5. For each role, assert `event.payload.properties.round == "1"`.
  6. `send(guest, get_lobby_snapshot, { handle: "tri_lobby" })`; assert `properties.round == "1"`.
- **Notes**: Port of legacy `lobby property propagation`.

### `lobby.properties.member.propagation`

- **Roles**: host, guest, guest2
- **Goal**: Guest's own `set_member_properties` propagates to host and guest2 via `lobby.member_updated`.
- **Steps**:
  1. Create + join three clients as above.
  2. Subscribe `host_mu`, `guest2_mu` for `lobby.member_updated` filtered by `{ handle: "tri_lobby", member_entity_key.id: guest.entity_id }`.
  3. `send(guest, set_member_properties, { handle: "tri_lobby", properties: { "ready": "true" } })`.
  4. Await both; assert each event has `properties.ready == "true"`.
- **Notes**: Port of legacy `member property propagation`.

### `lobby.leave.client`

- **Roles**: host, guest
- **Goal**: Guest leave fires `lobby.member_removed` on host.
- **Steps**:
  1. Create + guest joins.
  2. Subscribe `host_removed` for `lobby.member_removed` with `{ handle: "shared_lobby", member_entity_key.id: guest.entity_id }`.
  3. `send(guest, leave_lobby, { handle: "shared_lobby" })`.
  4. `await host_removed.wait(10000)`; assert not timed out.
  5. `send(host, get_lobby_snapshot, { handle: "shared_lobby" })`; assert `member_count == 1`.
- **Notes**: Port of legacy `client leave propagation`.

### `lobby.leave.third_member`

- **Roles**: host, guest, guest2
- **Goal**: `guest2` leave fires `lobby.member_removed` on host and guest.
- **Steps**:
  1. Create + join three clients.
  2. Subscribe `host_removed`, `guest_removed` filtered by `{ member_entity_key.id: guest2.entity_id }`.
  3. `send(guest2, leave_lobby, { handle: "tri_lobby" })`.
  4. Await both; snapshot from host: `member_count == 2`.
- **Notes**: Port of legacy `third member leave propagation`.

### `lobby.leave.rejoin_after_leave`

- **Roles**: host, guest
- **Goal**: Guest can rejoin the same lobby after leaving.
- **Steps**:
  1. Create + guest joins; await `host_removed` after `guest leave_lobby`.
  2. Subscribe `host_added2` for `lobby.member_added` filtered by `{ member_entity_key.id: guest.entity_id }`.
  3. `send(guest, join_lobby, { as: "shared_lobby_b", connection_string: create.result.connection_string })`.
  4. Assert join response ok; await `host_added2`.
- **Notes**: Port of legacy `rejoin after leave`. The new handle `"shared_lobby_b"` is intentional; reusing the old handle is not contractually supported.

### `lobby.leave.host.owner_migration`

- **Roles**: host, guest
- **Goal**: Host leaves; ownership migrates to guest.
- **Steps**:
  1. Create + guest joins.
  2. Subscribe `guest_owner_changed` for `lobby.owner_changed` filtered by `{ handle: "shared_lobby", new_owner.id: guest.entity_id }`.
  3. Subscribe `guest_removed_host` for `lobby.member_removed` filtered by `{ member_entity_key.id: host.entity_id }`.
  4. `send(host, leave_lobby, { handle: "shared_lobby" })`.
  5. Await both with 30s timeout — owner migration can take time depending on PFLobby semantics.
  6. `send(guest, get_lobby_snapshot, { handle: "shared_lobby" })`; assert `owner_entity_key.id == guest.entity_id`.
- **Notes**: Port of legacy `owner migration after host leave`. PlayFab's `owner_migration_policy` default is `automatic`.

### `lobby.join.invalid_connection_string`

- **Roles**: guest
- **Goal**: Join with garbage connection string fails with a typed PlayFab error.
- **Steps**:
  1. `send(guest, join_lobby, { as: "would_be", connection_string: "not-a-real-string" })`.
  2. Assert `response.ok == false`.
  3. Assert `response.error.code == "playfab_error"`.
  4. Assert `response.error.playfab_result.hresult.begins_with("0x")`.
- **Notes**: Port of legacy `invalid connection string typed failure`. The legacy runner does not assert a specific HRESULT; this scenario follows suit because PlayFab's specific error may change with service revisions.

## Lobby — P1 detailed scenarios

### `lobby.create.with_initial_search_properties`

- **Roles**: host
- **Goal**: `search_properties` provided at create-time appear on the immediate snapshot and a search returns the lobby on the first try.
- **Steps**:
  1. Build search properties with `string_key1, string_key2, number_key1`.
  2. `send(host, create_lobby, { as: "search_init", access_policy: "public", search_properties: {...} })`.
  3. `send(host, get_lobby_snapshot, { handle: "search_init" })`.
  4. Assert every supplied key/value present in `snapshot.search_properties`.
  5. `send(host, search_lobbies, { filter: "string_key1 eq '<value>'" })`.
  6. Assert `response.result.lobbies` contains `create.result.lobby_id`.

### `lobby.state.owner_migration_event_ordering`

- **Roles**: host, guest
- **Goal**: After host leaves, the guest receives `lobby.member_removed` and `lobby.owner_changed` in the documented order.
- **Steps**:
  1. Create + guest joins.
  2. Use a `capture_events` block on guest for `lobby.*` for the duration of `host leave_lobby` + 15s.
  3. From the captured event list, assert: there exists a `lobby.member_removed` event for host, followed (strictly later) by a `lobby.owner_changed` event with `new_owner.id == guest.entity_id`.
- **Notes**: This guards an event-ordering invariant: `member_removed` precedes `owner_changed` for the same triggering event. If the addon ever emits in the other order, snapshots may transiently show a removed owner that still owns the lobby.

## Match — P0 detailed scenarios

### `match.ticket.create_and_cancel`

- **Roles**: host
- **Capabilities**: `matchmaking_queue_configured`
- **Goal**: Create a ticket, cancel it, observe `cancelled` status.
- **Steps**:
  1. `send(host, create_match_ticket, { as: "ticket", queue_name: orch.env("PLAYFAB_MULTIPLAYER_MATCH_QUEUE"), timeout_seconds: 60, attributes: { skill: 1 } })`.
  2. Assert `response.result.ticket.ticket_id != ""`.
  3. Subscribe `cancelled = expect_event(host, match.status_changed, { handle: "ticket", status: "cancelled" })`.
  4. `send(host, cancel_match_ticket, { ticket_id: response.result.ticket.ticket_id })`.
  5. `await cancelled.wait(15000)`; assert not timed out.
- **Notes**: Port of legacy `match ticket create and cancel`.

### `match.ticket.two_player_match.complete`

- **Roles**: host, guest
- **Goal**: Both clients in the same queue both reach `matched`.
- **Steps**:
  1. `host_matched = expect_event(host, match.status_changed, { handle: "host_ticket", status: "matched" })`.
  2. `guest_matched = expect_event(guest, match.status_changed, { handle: "guest_ticket", status: "matched" })`.
  3. `send(host, create_match_ticket, { as: "host_ticket", queue_name: ..., timeout_seconds: 60 })`.
  4. `send(guest, create_match_ticket, { as: "guest_ticket", queue_name: ..., timeout_seconds: 60 })`.
  5. `await host_matched.wait(60000)`; `await guest_matched.wait(60000)`.
  6. Assert both events have a `match.id` matching each other.
- **Notes**: Port of legacy `two-player match completion`. Subscribe before create — once a queue has enough players, status can transition to matched before the second `create_match_ticket` returns.

### `match.ticket.completion.metadata_present`

- **Roles**: host, guest
- **Goal**: The `matched` event carries the expected metadata fields.
- **Steps**:
  1. Drive both clients to `matched` per the above pattern.
  2. Assert each `matched` event has non-empty `match.id`, `match.arranged_lobby_connection_string`, and a `match.members` array of length 2.
  3. Assert each member entry has an `entity_key` with non-empty id.
- **Notes**: Guards completion-payload shape. If the addon ever drops a field, this fails before integration scenarios that depend on it.

### `match.integration.arranged_lobby_join`

- **Roles**: host, guest
- **Capabilities**: `matchmaking_queue_configured`
- **Goal**: After match, both players join the arranged lobby and observe each other as members.
- **Steps**:
  1. Drive both clients to `matched`; capture `connection_string = host_matched.event.payload.match.arranged_lobby_connection_string`.
  2. `host_added = expect_event(host, lobby.member_added, { handle: "arranged" })`.
  3. `guest_added = expect_event(guest, lobby.member_added, { handle: "arranged" })`.
  4. `send(host, join_arranged_lobby, { as: "arranged", connection_string: connection_string })`.
  5. `send(guest, join_arranged_lobby, { as: "arranged", connection_string: connection_string })`.
  6. `await host_added`; `await guest_added`.
  7. `send(host, get_lobby_snapshot, { handle: "arranged" })`; assert `member_count == 2`.
- **Notes**: Port of legacy `explicit arranged-lobby join`.

### `match.integration.arranged_lobby_cleanup`

- **Roles**: host, guest
- **Goal**: Both players leave the arranged lobby cleanly; no leaks observed via post-leave snapshot fetches.
- **Steps**:
  1. Run `match.integration.arranged_lobby_join` through step 7.
  2. Subscribe `host_removed` for `lobby.member_removed` filtered by guest entity.
  3. `send(guest, leave_lobby, { handle: "arranged" })`; `await host_removed`.
  4. `send(host, leave_lobby, { handle: "arranged" })`.
  5. `send(host, get_lobby_snapshot, { handle: "arranged" })`; assert `response.ok == false` and `response.error.code == "unknown_handle"`.
- **Notes**: Port of legacy `arranged-lobby cleanup`. After leave, the client must no longer recognize the handle (the in-process handle map drops it).

## Party — P0 detailed scenarios

### `party.network.create.smoke`

- **Roles**: host
- **Capabilities**: `playfab_party_available`
- **Goal**: Host creates a Party network and obtains a finalized descriptor + invitation id + local peer id.
- **Steps**:
  1. `send(host, party_create_network, { as: "party", max_players: 4, invitation_id: "auto", enable_text_chat: true, enable_voice_chat: false })`.
  2. Assert `response.ok`.
  3. Assert `response.result.network_descriptor != ""` and is a valid base64-looking string.
  4. Assert `response.result.invitation_id != ""`.
  5. Assert `response.result.local_peer_id == 1` (Party host peer id convention).

### `party.network.join.smoke`

- **Roles**: host, guest
- **Capabilities**: `playfab_party_available`
- **Goal**: Guest joins host's network; both observe the join.
- **Steps**:
  1. Host creates network as above; capture descriptor + invitation_id.
  2. `host_peer_connected = expect_event(host, party.peer_connected, { handle: "party" })`.
  3. `guest_ready = expect_event(guest, party.network_ready, { handle: "party" })`.
  4. `send(guest, party_join_network, { as: "party", descriptor: descriptor, invitation_id: invitation_id })`.
  5. `await guest_ready.wait(15000)`; `await host_peer_connected.wait(15000)`.
  6. Assert `guest_ready.event.payload.local_peer_id > 1`.
  7. Assert `host_peer_connected.event.payload.peer_id == guest_ready.event.payload.local_peer_id`.
- **Notes**: First Party scenario the orchestrator must prove. Same-host two-process Party is the working assumption (per `0-architecture.md`). If this fails on every retry in C4, re-open the multi-machine question with the user.

### `party.network.leave.smoke`

- **Roles**: host, guest
- **Goal**: Guest leaves; host observes `peer_disconnected`.
- **Steps**:
  1. Host creates + guest joins per above.
  2. `host_peer_disconnected = expect_event(host, party.peer_disconnected, { handle: "party" })`.
  3. `send(guest, party_leave_network, { handle: "party" })`.
  4. `await host_peer_disconnected.wait(15000)`.
  5. `send(host, party_get_network_snapshot, { handle: "party" })`; assert `peer_count == 1`.

### `party.rpc.round_trip.post_join_first_message`

- **Roles**: host, guest
- **Goal**: First RPC sent by guest immediately after `network_ready` arrives at host. Regression for PR #132.
- **Steps**:
  1. Host creates + guest joins (full sequence to step 7 of `party.network.join.smoke`).
  2. `host_ping_received = expect_event(host, party.rpc.ping_received, { handle: "party" })`.
  3. `send(guest, party_send_rpc_ping, { handle: "party", payload: { seq: 1 } })`.
  4. `await host_ping_received.wait(5000)`.
  5. Assert `host_ping_received.event.payload.payload.seq == 1`.
  6. Assert `host_ping_received.event.payload.sender_peer_id == guest_local_peer_id`.
- **Notes**: This is the regression the broader effort is justified by. If this passes, the C0 design assumption (same-host Party works after PR #132) is validated and the matrix's Party rows are honest.

### `party.rpc.bidirectional`

- **Roles**: host, guest
- **Goal**: Host → guest → host RPC sequence is preserved with matching `correlation_id`.
- **Steps**:
  1. Host creates + guest joins.
  2. `guest_ping_received = expect_event(guest, party.rpc.ping_received, { handle: "party" })`.
  3. `host_pong_received = expect_event(host, party.rpc.pong_received, { handle: "party" })`.
  4. `send(host, party_send_rpc_ping, { handle: "party", payload: { from: "host" } })`. Capture `correlation_id` from response.
  5. `await guest_ping_received.wait(5000)`; assert `guest_ping_received.event.payload.correlation_id == correlation_id`.
  6. Client implementation: on receiving `ping`, the test client automatically sends a `pong` back with the same correlation_id (built-in echo behavior, not scenario-driven).
  7. `await host_pong_received.wait(5000)`; assert `host_pong_received.event.payload.correlation_id == correlation_id`.

### `party.chat.text.round_trip`

- **Roles**: host, guest
- **Goal**: Guest sends chat text; host receives it via `party.chat.text_message_received`.
- **Steps**:
  1. Host creates with `enable_text_chat: true`; guest joins.
  2. `host_chat = expect_event(host, party.chat.text_message_received, { handle: "party" })`.
  3. `send(guest, party_send_chat_text, { handle: "party", text: "hello" })`.
  4. `await host_chat.wait(10000)`.
  5. Assert `host_chat.event.payload.text == "hello"` and `sender_peer_id == guest_local_peer_id`.

### `party.join.invalid_descriptor`

- **Roles**: guest
- **Goal**: Joining with a malformed descriptor fails synchronously with `party_invalid_descriptor` or equivalent typed error.
- **Steps**:
  1. `send(guest, party_join_network, { as: "junk", descriptor: "not-base64-anything", invitation_id: "junk" })`.
  2. Assert `response.ok == false`.
  3. Assert `response.error.code in [ "party_invalid_descriptor", "party_invalid_options", "playfab_error" ]`. The exact code depends on how the addon classifies decode failures; the scenario tolerates the documented set.
- **Notes**: This is one of the few scenarios where we permit a small set of acceptable error codes. The acceptable set is fixed at scenario-write time; if PlayFab adds a new code we update the scenario explicitly.

## Party — P1 detailed scenarios

### `party.descriptor.round_trip`

- **Roles**: host, guest
- **Goal**: Descriptor returned to scenario by host is byte-identical to descriptor used by guest, and the join produces a connected network.
- **Steps**:
  1. Host create network; capture `descriptor_a = response.result.network_descriptor`.
  2. `send(host, party_get_network_snapshot, { handle: "party" })`; capture `descriptor_b` from snapshot (the snapshot also surfaces the descriptor).
  3. Assert `descriptor_a == descriptor_b`.
  4. Guest joins with `descriptor_a`; assert join completes with non-zero `local_peer_id`.
- **Notes**: Guards descriptor stability across calls — a regression would mean the addon is regenerating descriptors and breaking session-sharing flows.

## Cross-service — P1 detailed scenarios

### `party_lobby.descriptor_via_lobby_property`

- **Roles**: host, guest
- **Capabilities**: `playfab_party_available`
- **Goal**: Host stores a Party descriptor in a public lobby property; guest joins the lobby, reads the property, joins the Party network.
- **Steps**:
  1. `send(host, create_lobby, { as: "session", access_policy: "public", search_properties: { "string_key1": run_tag } })`.
  2. `send(host, party_create_network, { as: "party", invitation_id: "auto" })`.
  3. `send(host, set_lobby_properties, { handle: "session", properties: { "party_descriptor": party.result.network_descriptor, "invitation_id": party.result.invitation_id } })`.
  4. `send(guest, join_lobby, { as: "session", connection_string: create.result.connection_string })`.
  5. `send(guest, get_lobby_snapshot, { handle: "session" })`; assert both property keys present.
  6. `send(guest, party_join_network, { as: "party", descriptor: snapshot.properties.party_descriptor, invitation_id: snapshot.properties.invitation_id })`.
  7. Assert join response ok and `local_peer_id > 1`.
- **Notes**: This is the canonical "Party + Lobby" composition pattern from `spec\gdext-playfab-party.md`. If `party.network.join.smoke` works, this should work — but the property carry-over is a frequently broken layer in titles, so it earns its own scenario.

## Notes on omitted scenarios

The following P0/P1 entries from `1-test-matrix.md` are deliberately not detailed here:

- **Three-client member_added/leave_removed for guest2**: covered by templates above; mechanical repetition.
- **`lobby.create.private.smoke`**: same as `public.smoke` with `access_policy: "private"`.
- **`lobby.create.with_initial_lobby_properties` / `with_initial_member_properties`**: same shape as `with_initial_search_properties`.
- **`lobby.search.no_results.isolation` / `multiple_lobbies`**: same as `search.public.by_string_key` with different filter shapes.
- **`lobby.properties.set.unjoined_lobby` / `unjoined_member` / `lobby.create.unsigned_in_user` / `lobby.search.invalid_filter_string`**: negative variants of detailed scenarios; assert `response.ok == false` with a documented error code per the addon's binding.
- **`match.ticket.invalid_queue_name` / `cancel_already_cancelled` / `create_without_init` / `cancel_unknown_handle`**: follow the same negative pattern as `lobby.join.invalid_connection_string`.
- **`match.state.full_match_event_sequence`**: same shape as `lobby.state.owner_migration_event_ordering` but with `match.status_changed` events.
- **`match.integration.arranged_lobby_property_round_trip`**: combination of `arranged_lobby_join` and `lobby.properties.lobby.propagation`.
- **All Party state_transitions / chaos / lifecycle entries**: same pattern as `party.network.{create,join,leave}.smoke` plus a kill or sequence assertion.
- **`party.transport.peer_id_assignment`**: a 3-line scenario asserting `host.local_peer_id == 1` and `guest.local_peer_id > 1` after join — covered implicitly by `party.network.join.smoke` already.
- **All chat permission / mute / multi-client chat scenarios**: derivatives of `party.chat.text.round_trip` with extra clients or a `set_chat_permissions` command.
- **All cross-service e2e**: linear compositions of detailed scenarios. C5 authors them by sequencing already-proven steps.

The C5 author begins by reading this file and the matrix together, then mass-produces the remaining 32 P1 scenario files mechanically using the templates here.
