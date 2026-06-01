# PlayFab Lobby and Matchmaking GDExtension Spec

## Overview

This document defines the planned Godot-facing PlayFab Multiplayer lobby and matchmaking surface for the `godot_playfab` addon.

The design intentionally keeps PlayFab Multiplayer separate from PlayFab Party. `PlayFab.multiplayer` owns lobby discovery, lobby membership, lobby properties, matchmaking tickets, arranged-lobby joins, and PlayFab Multiplayer handle cleanup. Party transport remains owned by `PlayFab.party` / `PlayFabPartyPeer` as described in `spec\gdext-playfab-party.md`; title code composes the two by storing or reading title-owned lobby properties such as a Party descriptor.

## Design goals

1. **Single root singleton** - expose lobby and matchmaking through `PlayFab.multiplayer`, not a second engine singleton.
2. **Entity-handle APIs only** - public calls accept a signed-in `PlayFabUser`, and native calls use that user's internal `PFEntityHandle`. Do not rebuild operations from public entity-key snapshots or entity tokens.
3. **Separation from Party** - lobby and matchmaking APIs never create Party networks, Party peers, or Party chat controls.
4. **Passive matchmaking completion** - completed match tickets report match metadata and the arranged-lobby connection string; the addon must not auto-join the arranged lobby.
5. **Godot-native async and state** - one-shot requests return completion `Signal` values with `PlayFabResult`; ongoing lobby and ticket updates are connectable state signals on persistent wrapper objects.
6. **Stable snapshots** - wrapper getters return cached Godot-friendly snapshots that are updated before state-change signals are emitted.

## Scope

| Domain | Included | Notes |
| --- | --- | --- |
| PlayFab Multiplayer runtime | Yes | `PFMultiplayerInitialize`, dispatch registration, shutdown cleanup |
| Lobby create/join/search/leave | Yes | `PlayFabLobby` wrappers and stable snapshots |
| Lobby property/member updates | Yes | async updates with visible failures |
| Lobby invites | Yes | surfaced as `PlayFabLobbyInvite` and service-level notifications |
| Matchmaking tickets | Yes | create, status updates, cancel, completed match metadata |
| Arranged lobby join | Yes | explicit title-requested call using a connection string from a completed ticket |
| Party transport | No | see `spec\gdext-playfab-party.md` |
| Cross-service Party-lobby helpers | No | no `create_party_lobby_async()` or `join_party_lobby_async()` helper |
| Auto-join after match | No | completed ticket does not join or create the arranged lobby |

## Public API summary

### Root service

`PlayFab.multiplayer` returns a `PlayFabMultiplayer` service object under the existing `PlayFab` singleton.

```gdscript
class_name PlayFabMultiplayer
extends RefCounted

signal state_changed(change: PlayFabMultiplayerStateChange)
signal invite_received(invite: PlayFabLobbyInvite)
signal multiplayer_error(result: PlayFabResult)

func is_initialized() -> bool
func initialize_async(config: PlayFabMultiplayerConfig = null) -> Signal
func shutdown_async() -> Signal

func create_lobby_async(user: PlayFabUser, config: PlayFabLobbyConfig = null) -> Signal
func join_lobby_async(user: PlayFabUser, connection_string: String, config: PlayFabLobbyJoinConfig = null) -> Signal
func join_arranged_lobby_async(user: PlayFabUser, connection_string: String, config: PlayFabLobbyJoinConfig = null) -> Signal
func find_lobbies_async(user: PlayFabUser, search: PlayFabLobbySearchConfig = null) -> Signal

func create_match_ticket_async(user: PlayFabUser, config: PlayFabMatchmakingTicketConfig) -> Signal

func get_lobbies() -> Array[PlayFabLobby]
func get_lobby(lobby_id: String) -> PlayFabLobby
func get_match_tickets() -> Array[PlayFabMatchTicket]
```

### Completion payloads

| Method | `PlayFabResult.data` |
| --- | --- |
| `initialize_async()` / `shutdown_async()` | `null` |
| `create_lobby_async()` / `join_lobby_async()` / `join_arranged_lobby_async()` | `PlayFabLobby` |
| `find_lobbies_async()` | `PlayFabLobbySearchResult` |
| `PlayFabLobby.set_properties_async()` / `PlayFabLobby.set_member_properties_async()` | `null`, unless implementation chooses to return a refreshed `PlayFabLobby` |
| `create_match_ticket_async()` / `PlayFabMatchTicket.refresh_async()` | `PlayFabMatchTicket` |
| `PlayFabMatchTicket.cancel_async()` | `null` |

Immediate validation failures still return an already-completed `Signal` containing a failed `PlayFabResult`.

## Native API mapping

All user-owned calls validate `PlayFabUser::get_entity_handle()` and use the newer `PFEntityHandle` overloads. A missing entity handle is an invalid-user error; the implementation must not fall back to `PFEntityKey` plus entity-token overloads.

| GDScript API | Native ownership / calls | Completion gate |
| --- | --- | --- |
| `initialize_async(config)` | `PFMultiplayerInitialize(...)`; attach lobby/matchmaking state processing to PlayFab runtime dispatch | PlayFab Multiplayer initialized and ready to process state changes |
| `shutdown_async()` | Cancel tracked tickets, leave tracked lobbies, reject new Multiplayer work, release handles, `PFMultiplayerUninitialize(...)`, then free deferred async contexts | Tracked resources settle or shutdown generation closes |
| `create_lobby_async(user, config)` | `PFMultiplayerCreateAndJoinLobby(...)` entity-handle overload | create/join completed state; `PlayFabLobby` snapshot populated |
| `join_lobby_async(user, connection_string, config)` | `PFMultiplayerJoinLobbyWithEntityHandle(...)` using a lobby connection string | join completed state; `PlayFabLobby` snapshot populated |
| `join_arranged_lobby_async(user, connection_string, config)` | `PFMultiplayerJoinArrangedLobby(...)` entity-handle overload using caller-provided arranged-lobby connection string | arranged-lobby join completed state; `PlayFabLobby` snapshot populated |
| `find_lobbies_async(user, search)` | `PFMultiplayerFindLobbies(...)` entity-handle overload | find completed state; stable search summaries populated |
| `PlayFabLobby.set_properties_async(properties)` | PFLobby update/post-update API for lobby properties | lobby update completed state; cached lobby snapshot refreshed |
| `PlayFabLobby.set_member_properties_async(properties)` | PFLobby update/post-update API for the local member associated with the lobby's entity handle | member update completed state; cached member snapshot refreshed |
| `create_match_ticket_async(user, config)` | `PFMultiplayerCreateMatchmakingTicketWithEntityHandles(...)` using local requester and configured members | native handle returned and tracked; subsequent progress is pushed by matchmaking state changes |
| `PlayFabMatchTicket.refresh_async()` | `PFMatchmakingTicketGetStatus(...)` / `PFMatchmakingTicketGetMatch(...)` snapshot refresh | diagnostic refresh only; normal progress is push-driven |
| `PlayFabMatchTicket.cancel_async()` | `PFMatchmakingTicketCancel(...)` | completion signal settles when the ticket reaches a cancelled or failed terminal state |

Native handles returned synchronously by create/join/find/ticket calls are provisional implementation details. Public wrappers may be allocated for bookkeeping, but externally visible state remains creating, joining, searching, matching, or equivalent until the corresponding completed state change succeeds.

Shutdown is cancellation-first: pending lobby and ticket signals complete with cancelled `PlayFabResult`s from a stable snapshot, re-entrant handlers cannot start new Multiplayer SDK work, native teardown is deferred until any active Multiplayer state-change batch has unwound, and the native async context storage is not freed until after `PFMultiplayerUninitialize(...)` returns.

## Lobby model

`PlayFabLobby` wraps a `PFLobby` handle and exposes stable snapshots.

```gdscript
class_name PlayFabLobby
extends RefCounted

signal state_changed(change: PlayFabLobbyStateChange)

var lobby_id: String
var connection_string: String
var owner_entity_key: Dictionary
var max_member_count: int
var member_count: int
var properties: Dictionary
var search_properties: Dictionary
var members: Array[PlayFabLobbyMember]

func get_lobby_id() -> String
func get_connection_string() -> String
func get_owner_entity_key() -> Dictionary
func get_members() -> Array[PlayFabLobbyMember]
func get_properties() -> Dictionary
func get_search_properties() -> Dictionary
func is_owner(user: PlayFabUser) -> bool
func set_properties_async(properties: Dictionary) -> Signal
func set_member_properties_async(properties: Dictionary) -> Signal
func leave_async() -> Signal
```

Lobby updates are object-scoped. When a lobby state change arrives, the addon updates the cached snapshot before emitting `PlayFabLobby.state_changed(change)`. The MLP also emits `PlayFab.multiplayer.state_changed(change)` as an aggregate signal for titles that prefer one service-level subscription. `PlayFabLobby.set_member_properties_async()` is local-member-only; after a successful local write, the addon eagerly patches that local member's snapshot before settling the completion signal and emitting `MEMBER_UPDATED`, because the native SDK may only report remote member-property changes through SDK-driven update callbacks.

### Lobby configs

```gdscript
class_name PlayFabLobbyConfig
extends RefCounted

var max_players: int = 8
var access_policy: PlayFabLobbyConfig.AccessPolicy = PlayFabLobbyConfig.ACCESS_POLICY_PRIVATE
var owner_migration_policy: PlayFabLobbyConfig.OwnerMigrationPolicy = PlayFabLobbyConfig.OWNER_MIGRATION_AUTOMATIC
var search_properties: Dictionary = {}
var lobby_properties: Dictionary = {}
var member_properties: Dictionary = {}
```

```gdscript
class_name PlayFabLobbyJoinConfig
extends RefCounted

var member_properties: Dictionary = {}
```

```gdscript
class_name PlayFabLobbySearchConfig
extends RefCounted

var filter: String = ""
var order_by: String = ""
var max_results: int = 10
```

### Lobby payload wrappers

```gdscript
class_name PlayFabLobbyMember
extends RefCounted

var user_id: String
var entity_key: Dictionary
var properties: Dictionary
var is_local: bool

func get_user_id() -> String
func get_entity_key() -> Dictionary
func get_properties() -> Dictionary
func is_local_member() -> bool
```

```gdscript
class_name PlayFabLobbyInvite
extends RefCounted

var lobby_id: String
var connection_string: String
var sender_user_id: String
var sender_entity_key: Dictionary
var invite_uri: String
var properties: Dictionary

func get_lobby_id() -> String
func get_connection_string() -> String
func get_sender_user_id() -> String
func get_sender_entity_key() -> Dictionary
func get_invite_uri() -> String
func get_properties() -> Dictionary
```

```gdscript
class_name PlayFabLobbySearchResult
extends RefCounted

var lobbies: Array[PlayFabLobbySummary]
var continuation_token: String

func get_lobbies() -> Array[PlayFabLobbySummary]
func get_continuation_token() -> String
```

```gdscript
class_name PlayFabLobbySummary
extends RefCounted

var lobby_id: String
var connection_string: String
var owner_entity_key: Dictionary
var max_member_count: int
var member_count: int
var search_properties: Dictionary
var lobby_properties: Dictionary

func get_lobby_id() -> String
func get_connection_string() -> String
func get_owner_entity_key() -> Dictionary
func get_search_properties() -> Dictionary
func get_lobby_properties() -> Dictionary
```

### Lobby state changes

```gdscript
class_name PlayFabLobbyStateChange
extends RefCounted

var kind: int
var lobby: PlayFabLobby
var result: PlayFabResult
var member: PlayFabLobbyMember
var invite: PlayFabLobbyInvite
var user: PlayFabUser
var properties: Dictionary
```

Recommended stable constants:

```gdscript
PlayFabLobby.MEMBER_ADDED
PlayFabLobby.MEMBER_REMOVED
PlayFabLobby.MEMBER_UPDATED
PlayFabLobby.PROPERTIES_UPDATED
PlayFabLobby.OWNER_CHANGED
PlayFabLobby.DISCONNECTED
```

## Matchmaking model

`PlayFabMatchTicket` wraps a PlayFab matchmaking ticket and exposes cached ticket and completed-match metadata.

```gdscript
class_name PlayFabMatchTicket
extends RefCounted

signal state_changed(change: PlayFabMatchTicketStateChange)

var ticket_id: String
var queue_name: String
var status: int
var members: Array[PlayFabUser]
var match_id: String
var arranged_lobby_connection_string: String
var properties: Dictionary

func get_ticket_id() -> String
func get_queue_name() -> String
func get_status() -> int
func get_members() -> Array[PlayFabUser]
func get_match_id() -> String
func get_arranged_lobby_connection_string() -> String
func get_properties() -> Dictionary
func is_complete() -> bool
func is_cancelled() -> bool
func refresh_async() -> Signal
func cancel_async() -> Signal
```

The `arranged_lobby_connection_string` is copied from native `PFMatchmakingMatchDetails.lobbyArrangementString`. It is reported to title code as optional follow-up data. The addon must not call `join_arranged_lobby_async(...)` from ticket-completion handling.

### Matchmaking configs

```gdscript
class_name PlayFabMatchmakingTicketConfig
extends RefCounted

var queue_name: String = ""
var timeout_seconds: int = 120
var members: Array[PlayFabMatchmakingMember] = []
```

```gdscript
class_name PlayFabMatchmakingMember
extends RefCounted

var user: PlayFabUser
var attributes: Dictionary = {}
```

`create_match_ticket_async(user, config)` uses the `user` argument as the local ticket owner/requester. If `config.members` is empty, the implementation creates a single matchmaking member from that user's `PFEntityHandle` with empty attributes. If members are supplied, each configured member supplies its own `PlayFabUser`, which is converted to its internal `PFEntityHandle`, plus per-member attributes. The returned completion signal resolves only after the SDK assigns a non-empty `ticket_id`; until then, the half-created native handle remains internal and is not returned by `get_match_tickets()`.

### Match ticket state changes

```gdscript
class_name PlayFabMatchTicketStateChange
extends RefCounted

var kind: int
var ticket: PlayFabMatchTicket
var result: PlayFabResult
var status: int
var match_id: String
var arranged_lobby_connection_string: String
```

Recommended stable constants:

```gdscript
PlayFabMatchTicket.CREATED
PlayFabMatchTicket.STATUS_CHANGED
PlayFabMatchTicket.COMPLETED
PlayFabMatchTicket.CANCELLED
PlayFabMatchTicket.FAILED
```

## Example usage and Party composition

Lobby and matchmaking APIs should remain generic PlayFab Multiplayer surfaces. Title code can compose them with Party by storing a finalized Party descriptor in lobby properties, but the addon should not hide that composition behind a cross-service helper.

### Create a lobby and receive updates

```gdscript
func create_lobby(playfab_user: PlayFabUser) -> PlayFabLobby:
    var config := PlayFabLobbyConfig.new()
    config.max_players = 4
    config.access_policy = PlayFabLobbyConfig.ACCESS_POLICY_PUBLIC
    config.search_properties = {
        "string_key1": "duos"
    }
    config.lobby_properties = {
        "map": "arena"
    }
    config.member_properties = {
        "display_name": "Host"
    }

    var result = await PlayFab.multiplayer.create_lobby_async(playfab_user, config)
    if not result.ok:
        push_warning(result.message)
        return null

    var lobby: PlayFabLobby = result.data
    lobby.state_changed.connect(_on_lobby_state_changed)
    return lobby

func _on_lobby_state_changed(change: PlayFabLobbyStateChange) -> void:
    match change.kind:
        PlayFabLobby.MEMBER_ADDED:
            print("Member joined: ", change.member.get_entity_key().get_id())
        PlayFabLobby.MEMBER_REMOVED:
            print("Member left: ", change.member.get_entity_key().get_id())
        PlayFabLobby.PROPERTIES_UPDATED:
            print("Lobby properties: ", change.lobby.get_properties())
        PlayFabLobby.DISCONNECTED:
            push_warning(change.result.message)
```

### Search and join a lobby

```gdscript
func find_and_join_lobby(playfab_user: PlayFabUser) -> PlayFabLobby:
    var search := PlayFabLobbySearchConfig.new()
    search.filter = "string_key1 eq 'duos'"
    search.max_results = 10

    var search_result = await PlayFab.multiplayer.find_lobbies_async(playfab_user, search)
    if not search_result.ok:
        push_warning(search_result.message)
        return null

    var summaries = search_result.data.get_lobbies()
    if summaries.is_empty():
        push_warning("No matching lobbies found.")
        return null

    var join_result = await PlayFab.multiplayer.join_lobby_async(
        playfab_user,
        summaries[0].get_connection_string()
    )
    if not join_result.ok:
        push_warning(join_result.message)
        return null

    var lobby: PlayFabLobby = join_result.data
    lobby.state_changed.connect(_on_lobby_state_changed)
    return lobby
```

### Start matchmaking and receive updates

```gdscript
func start_matchmaking(playfab_user: PlayFabUser) -> PlayFabMatchTicket:
    var config := PlayFabMatchmakingTicketConfig.new()
    config.queue_name = "default"
    config.timeout_seconds = 120

    var member := PlayFabMatchmakingMember.new()
    member.user = playfab_user
    member.attributes = {
        "skill": 12,
        "region": "westus"
    }
    config.members = [member]

    var result = await PlayFab.multiplayer.create_match_ticket_async(playfab_user, config)
    if not result.ok:
        push_warning(result.message)
        return null

    var ticket: PlayFabMatchTicket = result.data
    ticket.state_changed.connect(_on_match_ticket_state_changed)
    return ticket
```

Recommended conventional title-owned lobby property keys:

```gdscript
"party_descriptor"
"party_protocol"
"party_host_entity"
```

### Host with lobby discovery

```gdscript
func create_party_lobby(playfab_user: PlayFabUser) -> PlayFabLobby:
    var party_result = await PlayFab.party.create_and_join_network_async(playfab_user)
    if not party_result.ok:
        push_warning(party_result.message)
        return null

    var network: PlayFabPartyNetwork = party_result.data
    var descriptor := network.get_descriptor()
    if descriptor.is_empty():
        push_warning("Party descriptor is not ready yet.")
        return null

    var lobby_config = PlayFabLobbyConfig.new()
    lobby_config.max_players = 4
    lobby_config.access_policy = PlayFabLobbyConfig.ACCESS_POLICY_PUBLIC
    lobby_config.lobby_properties["party_descriptor"] = descriptor
    lobby_config.lobby_properties["party_protocol"] = "playfab_party_v1"
    lobby_config.member_properties["display_name"] = "Host"

    var result = await PlayFab.multiplayer.create_lobby_async(playfab_user, lobby_config)
    if not result.ok:
        push_warning(result.message)
        return null

    var lobby: PlayFabLobby = result.data
    lobby.state_changed.connect(_on_lobby_state_changed)
    return lobby
```

### Join lobby, then explicitly join Party

```gdscript
func join_lobby_party(playfab_user: PlayFabUser, connection_string: String) -> void:
    var lobby_result = await PlayFab.multiplayer.join_lobby_async(playfab_user, connection_string)
    if not lobby_result.ok:
        push_warning(lobby_result.message)
        return

    var lobby: PlayFabLobby = lobby_result.data
    var descriptor: String = lobby.get_properties().get("party_descriptor", "")
    if descriptor.is_empty():
        push_warning("Lobby did not include a Party descriptor.")
        return

    var party_result = await PlayFab.party.join_network_async(playfab_user, descriptor)
    if not party_result.ok:
        push_warning(party_result.message)
        return

    var network: PlayFabPartyNetwork = party_result.data
    multiplayer.multiplayer_peer = network.get_local_peer()
```

### Match completion, then explicit title choice

```gdscript
func _on_match_ticket_state_changed(change: PlayFabMatchTicketStateChange) -> void:
    match change.kind:
        PlayFabMatchTicket.COMPLETED:
            print("Matched: ", change.match_id)
            print("Arranged lobby connection: ", change.arranged_lobby_connection_string)
            _show_match_ready_ui(change.ticket, change.arranged_lobby_connection_string)
        PlayFabMatchTicket.CANCELLED:
            print("Matchmaking cancelled.")
        PlayFabMatchTicket.FAILED:
            push_warning(change.result.message)
```

```gdscript
func join_arranged_match_when_title_decides(playfab_user: PlayFabUser, ticket: PlayFabMatchTicket) -> void:
    var connection_string := ticket.get_arranged_lobby_connection_string()
    if connection_string.is_empty():
        push_warning("Match completed without an arranged lobby connection string.")
        return

    var join_result = await PlayFab.multiplayer.join_arranged_lobby_async(playfab_user, connection_string)
    if not join_result.ok:
        push_warning(join_result.message)
        return

    var lobby: PlayFabLobby = join_result.data
    var descriptor: String = lobby.get_properties().get("party_descriptor", "")
    if not descriptor.is_empty():
        var party_result = await PlayFab.party.join_network_async(playfab_user, descriptor)
        if party_result.ok:
            var network: PlayFabPartyNetwork = party_result.data
            multiplayer.multiplayer_peer = network.get_local_peer()
```

## Error/result conventions

Use stable error codes so GDScript callers can branch:

```gdscript
"not_initialized"
"invalid_user"
"invalid_connection_string"
"invalid_arranged_lobby_connection_string"
"invalid_properties"
"invalid_search"
"invalid_lobby"
"invalid_match_ticket_config"
"invalid_match_ticket_member"
"invalid_match_ticket"
"lobby_create_failed"
"lobby_join_failed"
"arranged_lobby_join_failed"
"lobby_search_failed"
"lobby_update_failed"
"match_ticket_failed"
"match_ticket_completed_failed"
```

All validation failures must return an already-completed `Signal` with a failed `PlayFabResult`.

## Testing expectations

Add GUT coverage under `tests\godot\playfab\tests\` for:

- public class and service registration;
- invalid or missing `PlayFabUser` entity handles;
- invalid lobby configs, join identifiers, arranged-lobby connection strings, searches, and ticket configs;
- result shapes for immediate failures and completed async operations;
- lobby snapshot update ordering before `PlayFabLobby.state_changed`;
- lobby/member property update validation;
- ticket status, completed, cancelled, and failed state-change wrappers;
- completed tickets reporting `arranged_lobby_connection_string` without automatically joining an arranged lobby;
- shutdown cleanup for tracked lobbies and tickets.

Live PlayFab Multiplayer tests must stay opt-in behind the repository's `LIVE_TESTS=1` / `-Live` path and use a sandbox PlayFab title. The live runner covers multi-client lobby flows and, when a configured matchmaking queue is supplied, match ticket create/cancel, two-player match completion, explicit arranged-lobby joins, and arranged-lobby cleanup.
