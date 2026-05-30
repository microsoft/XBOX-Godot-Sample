# PlayFab Multiplayer Test Automation — Test Matrix

## Overview

This document is the canonical scenario inventory for the PlayFab Multiplayer test automation effort. It enumerates every scenario the harness intends to exercise across PlayFab Lobby, PlayFab Matchmaking, and PlayFab Party, with priority, category, role requirements, and capability gates.

C1 lands the **what**. C2 (`2-detailed-scenarios.md`) lands the **how** — step-by-step API sequences — for the top P0/P1 scenarios. C3+ lands the harness code and the scenario files themselves.

Companions:

- `0-architecture.md` — system rationale.
- `3-harness-spec.md` — wire protocol.
- `4-scenario-authoring.md` — scenario contract and `TestOrchestrator` API.

## Priorities

| Priority | Definition |
| --- | --- |
| **P0** | Must pass on every `run_all_tests.ps1 -Live` invocation. Blocks a release if red. A failure here means PlayFab Multiplayer or Party is non-functional for a major flow. |
| **P1** | Should pass on every `-Live` invocation. Blocks a release after one re-run if still red. A failure here means a real feature is broken but a major flow may still work. |
| **P2** | Runs on `-Live`, surfaces in reports, does not block release on its own. Edge cases, boundary conditions, and non-critical features. |
| **P3** | Opt-in via `--include-p3` (or a separate `-LiveSoak` switch in `run_all_tests.ps1`). Long, expensive, or quarantine-prone scenarios. |

## Categories

| Category | Meaning |
| --- | --- |
| `functional` | Normal-path API exercises with single-call validation. |
| `boundary` | Value extremes, size limits, near-overflow, empty inputs, max-member counts. |
| `negative` | Invalid input must produce a typed error of the expected shape. |
| `state_transitions` | Multi-step state machine coverage where the assertion is the transition sequence. |
| `integration` | Two services composed (e.g., matchmaking-then-arranged-lobby, lobby property carrying a Party descriptor). |
| `chaos` | Disconnect, kill, timeout, or otherwise disrupt a participant; assert the survivors converge. |
| `e2e` | Long, multi-action scripted scenario that mimics a real session. |

## Roles

The matrix uses these role names. Roles are GDScript test client processes; each holds one signed-in PlayFab user with a stable custom-id for the run.

| Role | Use |
| --- | --- |
| `host` | The lobby/network creator and (most often) owner. |
| `guest` | The primary joiner. |
| `guest2` | The third participant for multi-member coverage. |
| `observer` | A signed-in user that exercises search/find paths without joining. |

A scenario lists the roles it requires. The orchestrator only schedules a scenario when every required role is connected; otherwise it reports `skipped` with reason `missing_roles`.

## Capability gates

Capabilities are announced by clients in the handshake (`3-harness-spec.md`). The orchestrator skips a scenario whose `REQUIRED_CAPABILITIES` include a capability the assigned client did not announce.

| Capability | Meaning | Defaulted by |
| --- | --- | --- |
| `playfab_multiplayer_available` | Client linked and initialized PlayFab Multiplayer. | Always required by Lobby + Match scenarios. |
| `playfab_party_available` | Client linked and initialized PlayFab Party. | Always required by Party scenarios. |
| `matchmaking_queue_configured` | `PLAYFAB_MULTIPLAYER_MATCH_QUEUE` is set and resolves to a real queue. | Match scenarios. |
| `live_write_allowed` | Orchestrator launched with `-AllowLiveWrites`. | Scenarios that persist title-side state (statistics, leaderboard entries, etc.). |
| `multi_host_processes` | At least two distinct test client processes are connected. | Two-role scenarios; orchestrator can spawn two clients on the same machine to satisfy this. |
| `multi_machine_eligible` | Two clients connected from distinct hosts. Currently only set when run via a future `--remote-clients` mode. | None today. Reserved. |

The working assumption (per the C0 `0-architecture.md` decision log) is that Party same-host scenarios are viable after PR #132; we do **not** add a `multi_machine_eligible` gate to Party scenarios speculatively. If C4 re-validation shows same-host Party still fails, that gate becomes the documented escape.

## Per-service state-transition tables

These tables anchor the `state_transitions` scenarios. Each cell is a transition the harness asserts is reached and observed via the corresponding event.

### Lobby state transitions

| From | Trigger | To | Observable event |
| --- | --- | --- | --- |
| `none` | `create_lobby_async(host)` | `joined(host)` | `lobby.created` on host |
| `joined(host)` | `join_lobby_async(guest, conn_str)` | `joined(host, guest)` | `lobby.member_added` on both host and guest |
| `joined(host, guest)` | `join_lobby_async(guest2, conn_str)` | `joined(host, guest, guest2)` | `lobby.member_added` on host, guest, and guest2 |
| `joined(host, guest)` | `set_member_properties_async(guest)` | `joined(host, guest)` w/ updated member properties | `lobby.member_updated` on host and guest |
| `joined(host, guest)` | `set_properties_async(host)` | `joined(host, guest)` w/ updated lobby properties | `lobby.properties_updated` on host and guest |
| `joined(host, guest)` | `leave_async(guest)` | `joined(host)` | `lobby.member_removed` on host and guest |
| `joined(host, guest)` | `leave_async(host)` | `joined(guest)` w/ owner migrated to guest | `lobby.owner_changed` on guest, `lobby.member_removed` on guest |
| `joined(host, guest)` | host process killed | `joined(guest)` w/ owner migrated to guest after grace | `lobby.disconnected` on host (last) + `lobby.owner_changed` on guest |
| `joined(host)` | `leave_async(host)` | `none` | `lobby.disconnected` on host |

### Match ticket state transitions

| From | Trigger | To | Observable event |
| --- | --- | --- | --- |
| `none` | `create_match_ticket_async(host, queue)` | `waiting_for_players` | `match.status_changed = waiting_for_players` on host |
| `waiting_for_players` | second player ticket created in same queue | `waiting_for_match` → `matched` | `match.status_changed = matched` on both |
| `matched` | host calls `cancel_match_ticket_async` | `cancelled` (idempotent if already terminal) | `match.status_changed = cancelled` on host |
| `waiting_for_players` | host calls `cancel_match_ticket_async` | `cancelled` | `match.status_changed = cancelled` on host |
| `waiting_for_players` | ticket `timeout_seconds` elapses | `failed` w/ timeout cause | `match.status_changed = failed` on host |
| `matched` | host calls `join_arranged_lobby_async` w/ match conn_str | `joined(arranged_lobby)` | `lobby.created` on host |

### Party network state transitions

| From | Trigger | To | Observable event |
| --- | --- | --- | --- |
| `none` | `create_and_join_network_async(host)` | `connected(host)` w/ descriptor | `party.network_ready` on host |
| `connected(host)` | `join_network_async(guest, descriptor)` | `connected(host, guest)` | `party.peer_connected(guest)` on host, `party.network_ready` on guest |
| `connected(host, guest)` | host sends RPC ping over `MultiplayerPeer` | host_observes ping response | `party.rpc.ping_received` on guest, `party.rpc.pong_received` on host |
| `connected(host, guest)` | guest sends chat text | host receives text | `party.chat.text_message_received` on host |
| `connected(host, guest)` | `leave_network_async(guest)` | `connected(host)` | `party.peer_disconnected(guest)` on host |
| `connected(host, guest)` | host process killed | `disconnected(guest)` after Party detects | `party.network_destroyed` on guest |

## Scenario matrix

The `ID` column maps directly to `SCENARIO_ID` in the scenario file (`4-scenario-authoring.md`). The `Source` column distinguishes scenarios ported from the legacy PS runner (P) from new ones (N).

### Lobby — functional

| ID | Name | Priority | Roles | Caps | Source |
| --- | --- | --- | --- | --- | --- |
| `lobby.create.public.smoke` | Public lobby create snapshot | P0 | host | — | P |
| `lobby.create.private.smoke` | Private lobby create snapshot | P1 | host | — | N |
| `lobby.create.with_initial_lobby_properties` | Create with non-empty initial lobby properties | P1 | host | — | N |
| `lobby.create.with_initial_member_properties` | Create with non-empty initial member properties | P1 | host | — | N |
| `lobby.create.with_initial_search_properties` | Create with non-empty initial search properties | P1 | host | — | N |
| `lobby.join.by_connection_string` | Client joins by connection string | P0 | host, guest | — | P |
| `lobby.join.three_clients` | Three-client membership snapshots | P0 | host, guest, guest2 | — | P |
| `lobby.search.public.by_string_key` | Public lobby search by string key | P0 | host, observer | — | P |
| `lobby.search.no_results.isolation` | Search returns 0 results when filter excludes all created lobbies | P1 | host, observer | — | P |
| `lobby.search.multiple_lobbies` | Multiple-lobby search returns all matching lobbies | P1 | host, observer | — | P |
| `lobby.search.private.not_searchable` | Private lobby is not returned by search | P0 | host, observer | — | P |
| `lobby.properties.lobby.propagation` | Lobby property update propagates to all members | P0 | host, guest, guest2 | — | P |
| `lobby.properties.member.propagation` | Member property update propagates to all members | P0 | host, guest, guest2 | — | P |
| `lobby.leave.client` | Client leave propagates `member_removed` to remaining members | P0 | host, guest | — | P |
| `lobby.leave.third_member` | Third member leave propagates to remaining two | P0 | host, guest, guest2 | — | P |
| `lobby.leave.rejoin_after_leave` | Client can rejoin same lobby after leaving | P0 | host, guest | — | P |
| `lobby.leave.host.owner_migration` | Owner migration after host leave | P0 | host, guest | — | P |

### Lobby — boundary

| ID | Name | Priority | Roles | Caps | Source |
| --- | --- | --- | --- | --- | --- |
| `lobby.create.max_players.one` | `max_players=1` creates a single-member lobby | P2 | host | — | N |
| `lobby.create.max_players.max` | `max_players` at the addon/PFLobby ceiling succeeds | P2 | host | — | N |
| `lobby.create.max_players.over_limit` | `max_players` over the ceiling fails synchronously | P2 | host | — | N |
| `lobby.properties.lobby.max_keys` | Set the maximum allowed number of lobby properties | P2 | host | — | N |
| `lobby.properties.member.max_keys` | Set the maximum allowed number of member properties | P2 | host | — | N |
| `lobby.properties.large_value` | Property value at max size limit roundtrips | P2 | host | — | N |
| `lobby.search.large_continuation_set` | Search returning > one page exposes continuation token | P2 | host, observer | — | N |
| `lobby.create.search_properties.all_typed_keys` | All `string_keyN`, `number_keyN` typed buckets are honored | P2 | host, observer | — | N |

### Lobby — negative

| ID | Name | Priority | Roles | Caps | Source |
| --- | --- | --- | --- | --- | --- |
| `lobby.join.invalid_connection_string` | Join with invalid connection string fails predictably | P0 | guest | — | P |
| `lobby.join.empty_connection_string` | Join with empty connection string fails synchronously | P1 | guest | — | N |
| `lobby.create.invalid_max_players.zero` | `max_players=0` fails synchronously | P1 | host | — | N |
| `lobby.create.invalid_max_players.negative` | `max_players<0` fails synchronously | P2 | host | — | N |
| `lobby.properties.set.unjoined_lobby` | Setting properties on a lobby the client left is rejected | P1 | host, guest | — | N |
| `lobby.properties.member.set.unjoined_member` | Setting member properties without being in the lobby is rejected | P1 | guest | — | N |
| `lobby.create.unsigned_in_user` | Calling create with a user whose entity handle is gone is rejected synchronously | P1 | host | — | N |
| `lobby.leave.double_leave` | Calling leave twice on the same lobby produces a typed already-left error | P2 | host | — | N |
| `lobby.search.invalid_filter_string` | Malformed OData filter produces a typed error, not a hang | P1 | observer | — | N |

### Lobby — state_transitions

| ID | Name | Priority | Roles | Caps | Source |
| --- | --- | --- | --- | --- | --- |
| `lobby.state.create_join_leave_full_cycle` | Full create→join→leave cycle observed via every state-change kind | P1 | host, guest | — | N |
| `lobby.state.owner_migration_event_ordering` | `member_removed` and `owner_changed` arrive in the documented order on the survivor | P1 | host, guest | — | N |
| `lobby.state.disconnected_after_double_kill` | Killing all members produces `disconnected` on all surviving observers | P2 | host, guest | — | N |

### Lobby — chaos

| ID | Name | Priority | Roles | Caps | Source |
| --- | --- | --- | --- | --- | --- |
| `lobby.chaos.host_kill.owner_migration` | Host process killed → owner migrates to guest within the grace window | P1 | host, guest | — | N |
| `lobby.chaos.client_kill.member_removed` | Guest process killed → host observes `member_removed` after grace | P1 | host, guest | — | N |
| `lobby.chaos.tcp_pause.no_state_drift` | Pausing the wire TCP for ≤ heartbeat tolerance does not cause state divergence | P3 | host, guest | — | N |

### Lobby — multiple

| ID | Name | Priority | Roles | Caps | Source |
| --- | --- | --- | --- | --- | --- |
| `lobby.tracking.multiple_lobbies_per_host` | Single host tracks multiple lobbies and returns the right snapshot per handle | P1 | host | — | P |

### Match — functional

| ID | Name | Priority | Roles | Caps | Source |
| --- | --- | --- | --- | --- | --- |
| `match.ticket.create_and_cancel` | Create ticket, cancel ticket, status reaches cancelled | P0 | host | `matchmaking_queue_configured` | P |
| `match.ticket.two_player_match.complete` | Two players in same queue both reach matched | P0 | host, guest | `matchmaking_queue_configured` | P |
| `match.ticket.completion.metadata_present` | Completed ticket exposes match.id, arranged-lobby conn_str, members | P0 | host, guest | `matchmaking_queue_configured` | N |

### Match — negative

| ID | Name | Priority | Roles | Caps | Source |
| --- | --- | --- | --- | --- | --- |
| `match.ticket.invalid_queue_name` | Unknown queue name fails predictably | P1 | host | `matchmaking_queue_configured` | N |
| `match.ticket.cancel_already_cancelled` | Cancel on already-cancelled ticket is idempotent (typed already-terminal error or no-op) | P2 | host | `matchmaking_queue_configured` | N |
| `match.ticket.create_without_init` | Create ticket before `PlayFab.multiplayer.initialize_async` is rejected | P1 | host | `matchmaking_queue_configured` | N |
| `match.ticket.cancel_unknown_handle` | Cancel against an unknown handle is rejected with a typed error | P2 | host | `matchmaking_queue_configured` | N |

### Match — boundary

| ID | Name | Priority | Roles | Caps | Source |
| --- | --- | --- | --- | --- | --- |
| `match.ticket.timeout.min_seconds` | Ticket with minimum allowed `timeout_seconds` works | P2 | host | `matchmaking_queue_configured` | N |
| `match.ticket.timeout.elapses` | Ticket with short timeout elapses and reports `failed` | P2 | host | `matchmaking_queue_configured` | N |
| `match.ticket.attributes.empty` | Empty attributes is accepted (queue allowing) | P2 | host | `matchmaking_queue_configured` | N |
| `match.ticket.attributes.complex` | Nested/complex attribute Dictionary survives roundtrip | P2 | host | `matchmaking_queue_configured` | N |

### Match — state_transitions

| ID | Name | Priority | Roles | Caps | Source |
| --- | --- | --- | --- | --- | --- |
| `match.state.full_match_event_sequence` | All status transitions observed in order on both players | P1 | host, guest | `matchmaking_queue_configured` | N |

### Match — integration (with Lobby)

| ID | Name | Priority | Roles | Caps | Source |
| --- | --- | --- | --- | --- | --- |
| `match.integration.arranged_lobby_join` | After match, both clients join the arranged lobby | P0 | host, guest | `matchmaking_queue_configured` | P |
| `match.integration.arranged_lobby_cleanup` | Leaving the arranged lobby releases handles cleanly | P0 | host, guest | `matchmaking_queue_configured` | P |
| `match.integration.arranged_lobby_property_round_trip` | Arranged-lobby members can set + read lobby properties | P1 | host, guest | `matchmaking_queue_configured` | N |

### Party — functional

| ID | Name | Priority | Roles | Caps | Source |
| --- | --- | --- | --- | --- | --- |
| `party.network.create.smoke` | Host creates a network and finalizes descriptor | P0 | host | `playfab_party_available` | N |
| `party.network.join.smoke` | Guest joins via host descriptor; both observe `network_ready` | P0 | host, guest | `playfab_party_available` | N |
| `party.network.leave.smoke` | Guest leaves via `leave_network_async`; host observes peer_disconnected | P0 | host, guest | `playfab_party_available` | N |
| `party.descriptor.round_trip` | Descriptor serialized by host deserializes and authenticates on guest | P1 | host, guest | `playfab_party_available` | N |
| `party.lifecycle.host_create_join_destroy` | Host creates, guest joins, host destroys; both clean | P1 | host, guest | `playfab_party_available` | N |

### Party — RPC / transport

| ID | Name | Priority | Roles | Caps | Source |
| --- | --- | --- | --- | --- | --- |
| `party.rpc.round_trip.post_join_first_message` | First RPC from guest after join arrives at host (PR #132 regression) | P0 | host, guest | `playfab_party_available` | N |
| `party.rpc.bidirectional` | Host → guest → host RPC sequence preserved | P0 | host, guest | `playfab_party_available` | N |
| `party.rpc.large_payload` | RPC payload at the max recommended size roundtrips | P2 | host, guest | `playfab_party_available` | N |
| `party.rpc.burst` | 100 RPCs in 1s do not lose any frames | P2 | host, guest | `playfab_party_available` | N |
| `party.transport.peer_id_assignment` | Host is peer 1; guest receives a positive, stable peer id | P1 | host, guest | `playfab_party_available` | N |

### Party — chat

| ID | Name | Priority | Roles | Caps | Source |
| --- | --- | --- | --- | --- | --- |
| `party.chat.text.round_trip` | Guest sends chat text, host receives it via `text_message_received` | P0 | host, guest | `playfab_party_available` | N |
| `party.chat.text.three_clients` | Three clients see each other's chat messages | P1 | host, guest, guest2 | `playfab_party_available` | N |
| `party.chat.mute.peer` | Muting peer suppresses their chat on the muting client | P1 | host, guest | `playfab_party_available` | N |
| `party.chat.permissions.send_only` | Setting send-only chat permission denies receive | P2 | host, guest | `playfab_party_available` | N |

### Party — negative

| ID | Name | Priority | Roles | Caps | Source |
| --- | --- | --- | --- | --- | --- |
| `party.join.invalid_descriptor` | Join with malformed descriptor fails synchronously | P0 | guest | `playfab_party_available` | N |
| `party.join.expired_descriptor` | Join with stale descriptor whose network was destroyed fails with a typed error | P1 | host, guest | `playfab_party_available` | N |
| `party.create.invalid_direct_peer_connectivity` | Setting incompatible `direct_peer_connectivity` flags fails synchronously with `party_invalid_options` | P1 | host | `playfab_party_available` | N |
| `party.create.unsigned_in_user` | Create with a user without an entity handle is rejected synchronously | P1 | host | `playfab_party_available` | N |
| `party.send.before_network_ready` | Sending RPC before `network_ready` is rejected (or queued and asserted to land post-ready) | P2 | host | `playfab_party_available` | N |

### Party — boundary

| ID | Name | Priority | Roles | Caps | Source |
| --- | --- | --- | --- | --- | --- |
| `party.network.max_players.three_join_to_three` | Three clients fill a `max_players=3` network exactly | P2 | host, guest, guest2 | `playfab_party_available` | N |
| `party.network.max_players.over_limit` | Fourth client join into `max_players=3` rejected with typed error | P2 | host, guest, guest2 | `playfab_party_available`, `multi_host_processes` | N |
| `party.descriptor.long_metadata` | Metadata Dictionary at the addon's max size roundtrips | P3 | host | `playfab_party_available` | N |

### Party — state_transitions

| ID | Name | Priority | Roles | Caps | Source |
| --- | --- | --- | --- | --- | --- |
| `party.state.create_join_leave_full_cycle` | All `network_ready`, `peer_connected`, `peer_disconnected`, `network_destroyed` events observed in expected order | P1 | host, guest | `playfab_party_available` | N |
| `party.state.host_leaves_network_destroyed_on_guest` | Host leaves → guest observes `network_destroyed` | P1 | host, guest | `playfab_party_available` | N |

### Party — chaos

| ID | Name | Priority | Roles | Caps | Source |
| --- | --- | --- | --- | --- | --- |
| `party.chaos.host_kill.network_destroyed` | Host process killed → guest observes `network_destroyed` within Party's detection window | P1 | host, guest | `playfab_party_available` | N |
| `party.chaos.guest_kill.peer_disconnected` | Guest process killed → host observes `peer_disconnected` within Party's detection window | P1 | host, guest | `playfab_party_available` | N |
| `party.chaos.guest_reconnect_with_new_endpoint` | Killed guest rejoins; receives a new peer id; existing host state intact | P2 | host, guest | `playfab_party_available` | N |

### Party + Lobby — integration

| ID | Name | Priority | Roles | Caps | Source |
| --- | --- | --- | --- | --- | --- |
| `party_lobby.descriptor_via_lobby_property` | Host stores Party descriptor in lobby property; guest reads it and joins Party | P1 | host, guest | `playfab_party_available` | N |
| `party_lobby.descriptor_via_member_property` | Host stores Party descriptor in member property; guest reads + joins | P2 | host, guest | `playfab_party_available` | N |
| `party_lobby.invite_chain` | Host invites guest to lobby; guest joins lobby; reads Party descriptor; joins Party network | P2 | host, guest | `playfab_party_available` | N |

### Party + Match — integration

| ID | Name | Priority | Roles | Caps | Source |
| --- | --- | --- | --- | --- | --- |
| `party_match.descriptor_via_arranged_lobby_property` | Matched players join arranged lobby; host publishes Party descriptor as lobby property; guest joins Party | P1 | host, guest | `playfab_party_available`, `matchmaking_queue_configured` | N |

### Cross-service — e2e

| ID | Name | Priority | Roles | Caps | Source |
| --- | --- | --- | --- | --- | --- |
| `e2e.full_session.match_then_party_play` | Match → arranged lobby → Party network → bidirectional RPC + chat → orderly leave | P1 | host, guest | `playfab_party_available`, `matchmaking_queue_configured` | N |
| `e2e.full_session.three_player_lobby_party_play` | 3-player public lobby → Party from lobby property → chat + RPC → owner migration after host leaves | P2 | host, guest, guest2 | `playfab_party_available` | N |

## Summary counts

| Service | P0 | P1 | P2 | P3 | Total |
| --- | --- | --- | --- | --- | --- |
| Lobby | 12 | 17 | 11 | 1 | 41 |
| Match | 5 | 4 | 6 | 0 | 15 |
| Party | 7 | 12 | 7 | 1 | 27 |
| Cross-service | 0 | 3 | 3 | 0 | 6 |
| **Total** | **24** | **36** | **27** | **2** | **89** |

C5 ships P0 + P1 (60 scenarios). C2 details the top 25-30 P0/P1 with API sequences. P2 and P3 are queued for follow-up after the harness has shipped its first green run.

## Coverage vs legacy PS runner

The retired 18-scenario PowerShell runner mapped 1:1 onto the matrix entries marked `Source = P`. C5/C6 now carry the full P0/P1 set through `tests/godot/mp_orchestrator/scenarios/`; `tools/run_all_tests.ps1` selects those P0/P1 scenario files for the canonical live MP stage.

## Out-of-scope (deferred follow-ups)

- Service-side leaderboards, statistics, and Game Saves — covered by the GUT suites, not by this harness.
- Voice chat audio path verification — requires audio capture/playback fixtures that this harness does not own.
- PartyXbl (Xbox-platform Party identity bridging) — not initialized by the addon; see `spec\gdext-playfab-party.md`.
- Multi-machine remote-client orchestration — design exists in `0-architecture.md`, implementation deferred until a real same-host gap is identified.
- Long soak (1-hour+) reliability runs — `P3` placeholder, opt-in via `-LiveSoak` once the harness is steady.
