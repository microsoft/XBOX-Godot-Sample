# PlayFab Party GDExtension Spec

## Overview

This document defines the planned Godot-facing PlayFab Party surface for the `godot_playfab` addon.

`PlayFab.party` owns PlayFab Party initialization, Party network host/join flows, descriptor serialization, the Godot high-level multiplayer transport, and Party chat-control integration. The normal title-facing object is `PlayFabPartyPeer`: it is assigned to `multiplayer.multiplayer_peer` for RPC/gameplay traffic and also exposes peer-id-based helpers for common Party text chat, mute, and permission operations.

PlayFab Multiplayer lobbies and matchmaking remain separate and are described in `spec\gdext-playfab-lobby-matchmaking.md`. Title code composes the two systems by sharing finalized Party descriptors through title-owned lobby properties, invites, matchmaking follow-up flows, or backend services.

## Design goals

1. **Single root singleton** - expose Party through `PlayFab.party`, not a second engine singleton.
2. **Godot-native transport** - make `PlayFabPartyPeer` usable anywhere a Godot `MultiplayerPeer` would be assigned.
3. **One title-facing Party object** - titles should normally keep `PlayFabPartyPeer` for gameplay packets, chat, mute, permissions, descriptor access, and close/leave behavior.
4. **Entity-handle APIs only** - public calls accept a signed-in `PlayFabUser`, and native calls use that user's internal `PFEntityHandle`.
5. **Hide native choreography** - titles should not manually call create local user, create/connect network, authenticate user, create endpoint, create chat control, or connect chat control.
6. **Final descriptors only** - never expose the provisional immediate descriptor from `CreateNewNetwork(...)`; expose only the finalized base64 serialized `PartyNetworkDescriptor`.
7. **Separate Party from Multiplayer** - Party does not create lobbies, matchmaking tickets, or arranged-lobby joins.
8. **Chat is not packet transport** - text, transcription, mute, and permissions use Party chat-control APIs internally and are surfaced through peer-id helpers, not through Godot RPC packets.

## Scope

| Domain | Included | Notes |
| --- | --- | --- |
| Party runtime | Yes | `PartyManager::Initialize`, state-change pumping, cleanup |
| Party host/join | Yes | convenience async operations return ready networks |
| Descriptor serialization | Yes | finalized descriptor only, base64 string |
| Godot high-level transport | Yes | `PlayFabPartyPeer` implements `MultiplayerPeerExtension` semantics |
| Peer-id mapping | Yes | host is peer `1`; clients receive positive ids through transport handshake |
| Data endpoints | Yes | internal only; gameplay uses Godot peer ids |
| Text chat / transcription | Yes | exposed primarily through `PlayFabPartyPeer` peer-id helpers/signals |
| Mute / chat permissions | Yes | exposed primarily through `PlayFabPartyPeer` peer-id helpers/signals |
| Advanced chat-control access | Yes | `PlayFab.party.chat` / `PlayFabPartyChatControl` remain diagnostic escape hatches |
| PartyXbl | No | not initialized, pumped, or exposed in the first pass |
| Platform privacy/privilege policy | No | title code or later GDK/PartyXbl integration applies policy through peer permissions |
| PlayFab Multiplayer lobbies/matchmaking | No | see `spec\gdext-playfab-lobby-matchmaking.md` |

## Public API summary

### Root service

`PlayFab.party` returns a `PlayFabParty` service object under the existing `PlayFab` singleton.

```gdscript
class_name PlayFabParty
extends RefCounted

signal party_error(result: PlayFabResult)

func is_initialized() -> bool
func initialize_async(config: PlayFabPartyConfig = null) -> Signal
func shutdown_async() -> Signal

func create_and_join_network_async(user: PlayFabUser, config: PlayFabPartyConfig = null) -> Signal
func join_network_async(user: PlayFabUser, descriptor: String, config: PlayFabPartyConfig = null) -> Signal
func leave_network_async(network: PlayFabPartyNetwork) -> Signal

func get_chat() -> PlayFabPartyChat
func get_networks() -> Array[PlayFabPartyNetwork]
```

### Completion payloads

| Method | `PlayFabResult.data` |
| --- | --- |
| `initialize_async()` / `shutdown_async()` | `null` |
| `create_and_join_network_async()` | `PlayFabPartyNetwork` |
| `join_network_async()` | `PlayFabPartyNetwork` |
| `leave_network_async()` | `null` |

Immediate validation failures still return an already-completed `Signal` containing a failed `PlayFabResult`.

## Native operation model

All user-owned calls validate `PlayFabUser::get_entity_handle()` and use newer entity-handle Party APIs. A missing entity handle is an invalid-user error; the implementation must not fall back to entity-key/entity-token auth paths for user-owned Party operations.

`PlayFab.party.create_and_join_network_async(user, config)` hides this native state machine:

1. Validate the signed-in `PlayFabUser` and its `PFEntityHandle`.
2. Initialize Party lazily if needed.
3. Create or reuse the local Party user for that entity handle.
4. Create the Party network.
5. Connect to the Party network.
6. Authenticate the local Party user to the connected network.
7. Optionally create/connect the local chat control when chat is enabled.
8. Create the local data endpoint.
9. Capture the finalized network descriptor from the completed/connected network state, serialize it, and expose it as a base64 string.
10. Create a ready `PlayFabPartyNetwork` with a local `PlayFabPartyPeer`.

`PlayFab.party.join_network_async(user, descriptor, config)` hides this native state machine:

1. Validate the signed-in `PlayFabUser` and its `PFEntityHandle`.
2. Initialize Party lazily if needed.
3. Decode and deserialize the finalized base64 Party descriptor.
4. Create or reuse the local Party user for that entity handle.
5. Connect to the Party network.
6. Authenticate the local Party user to the connected network.
7. Optionally create/connect the local chat control when chat is enabled.
8. Create the local data endpoint.
9. Run the peer-id handshake with the host.
10. Create a ready `PlayFabPartyNetwork` with a local `PlayFabPartyPeer`.

The lower-level native steps remain implementation details. Public host/join signals resolve only when every resource required for normal use is complete and usable.

## Runtime and dispatch ownership

`PlayFab.party` either extends `PlayFabRuntime` or owns a `PlayFabPartyRuntime` that shares the root PlayFab lifecycle.

Rules:

1. Base PlayFab runtime initializes first.
2. `PlayFab.party` initializes `PartyManager` lazily on first Party operation or explicitly through `initialize_async()`.
3. `PartyManager::StartProcessingStateChanges` / `FinishProcessingStateChanges` are pumped from the PlayFab runtime dispatch path, not from every peer.
4. Auto dispatch may update internal state and complete one-shot operations.
5. Network-facing emissions are queued until the owning `PlayFabPartyPeer::_poll()` flushes them through Godot's normal multiplayer polling path.
6. Shutdown closes active Party peers, networks, endpoints, and chat controls before PlayFab Multiplayer, PlayFab Services, and the core runtime are uninitialized.
7. Shutdown emits cancelled completion results from a snapshot of pending operations, rejects new Party work while the service is shutting down, defers native cleanup until any active Party state-change batch has unwound, and defers freeing native async context storage until after `PartyManager::Cleanup()` returns.

## Config wrappers

```gdscript
class_name PlayFabPartyConfig
extends RefCounted

var max_players: int = 8
var direct_peer_connectivity: int = PlayFabParty.DIRECT_PEER_CONNECTIVITY_NONE
var invitation_id: String = "" # generated if empty
var enable_voice_chat: bool = true
var enable_text_chat: bool = true
var enable_transcription: bool = false
var enable_translation: bool = false
var audio_input: String = ""  # platform default when empty
var audio_output: String = "" # platform default when empty
var metadata: Dictionary = {}
```

`enable_voice_chat`, `enable_text_chat`, transcription, translation, and audio fields are addon policy flags. They decide whether and how the addon creates/connects Party chat controls; they are not native Party network settings.

### `direct_peer_connectivity` flag rules

`direct_peer_connectivity` is a bitmask of `PlayFabParty.DirectPeerConnectivity` flags. The valid shapes that match the GDK Party SDK contract are:

- `0` (`DIRECT_PEER_CONNECTIVITY_NONE`, default): no direct connections; all traffic relays through PlayFab Party.
- One or both of (`SAME_PLATFORM_TYPE`, `DIFFERENT_PLATFORM_TYPE`) **combined with** one or both of (`SAME_ENTITY_LOGIN_PROVIDER`, `DIFFERENT_ENTITY_LOGIN_PROVIDER`). `DIRECT_PEER_CONNECTIVITY_ANY` (= `ANY_PLATFORM_TYPE | ANY_ENTITY_LOGIN_PROVIDER`) is the recommended "broadest direct-connection" preset.
- `DIRECT_PEER_CONNECTIVITY_ONLY_SERVERS` by itself; it cannot be combined with the platform-type or entity-login-provider flags in the network configuration.

Setting platform-type flags without a login-provider flag (or vice versa) — or combining `ONLY_SERVERS` with anything else — fails synchronously inside `PlayFabParty.create_and_join_network_async()` with code `party_invalid_options` instead of being forwarded to `PartyManager::CreateNewNetwork` (which would return a generic "invalid network configuration struct" `PartyError`).

```gdscript
class_name PlayFabPartyTextMessageConfig
extends RefCounted

var language_code: String = ""
var translate_to_languages: PackedStringArray = PackedStringArray()
var metadata: Dictionary = {}
```

## Party network and descriptor model

```gdscript
class_name PlayFabPartyNetwork
extends RefCounted

signal state_changed(change: PlayFabPartyNetworkStateChange)

var network_id: String
var descriptor: String
var state: int
var local_user: PlayFabUser
var local_peer: PlayFabPartyPeer
var local_chat_control: PlayFabPartyChatControl
var is_host: bool

func get_network_id() -> String
func get_descriptor() -> String
func get_state() -> int
func get_local_user() -> PlayFabUser
func get_local_peer() -> PlayFabPartyPeer
func get_local_chat_control() -> PlayFabPartyChatControl
func is_host_network() -> bool
func leave_async() -> Signal
```

`PlayFabPartyNetwork` is the typed setup result and lifecycle owner. `PlayFab.party` keeps a strong reference to active networks until `leave_network_async()`, `PlayFabPartyNetwork.leave_async()`, `PlayFabPartyPeer.close_with_reason()`, or shutdown settles. Dropping a GDScript reference or replacing `multiplayer.multiplayer_peer` must not silently destroy the Party network.

Descriptor rules:

1. The immediate `CreateNewNetwork(...)` descriptor is an internal bootstrap value.
2. `PlayFabPartyNetwork.descriptor` and descriptor getters remain empty/unavailable until the finalized descriptor is available.
3. Public descriptors are base64-encoded serialized `PartyNetworkDescriptor` values.
4. Descriptors are title-owned sharing data. The addon does not decide whether they move through lobbies, invites, matchmaking, or a backend.

## `PlayFabPartyPeer`

`PlayFabPartyPeer` is the object assigned to `multiplayer.multiplayer_peer` and the normal object titles keep for a Party network.

```gdscript
class_name PlayFabPartyPeer
extends MultiplayerPeerExtension

signal connection_state_changed(status: int)
signal network_error(result: PlayFabResult)
signal chat_control_added(peer_id: int, chat_control: PlayFabPartyChatControl)
signal chat_control_removed(peer_id: int)
signal text_message_received(peer_id: int, message: PlayFabPartyChatMessage)
signal transcription_received(peer_id: int, message: PlayFabPartyChatMessage)
signal chat_permissions_changed(peer_id: int, permissions: int)
signal peer_muted_changed(peer_id: int, muted: bool)

func get_network() -> PlayFabPartyNetwork
func get_local_user() -> PlayFabUser
func get_descriptor() -> String
func get_peer_entity_key(peer_id: int) -> PlayFabEntityKey
func get_peer_member(peer_id: int) -> PlayFabPartyMember
func get_peers() -> Array[int]
func get_local_chat_control() -> PlayFabPartyChatControl
func get_peer_chat_control(peer_id: int) -> PlayFabPartyChatControl
func send_text_async(message: String, target_peer_ids: PackedInt32Array = PackedInt32Array(), config: PlayFabPartyTextMessageConfig = null) -> Signal
func set_peer_chat_permissions_async(peer_id: int, permissions: int) -> Signal
func set_peer_muted_async(peer_id: int, muted: bool) -> Signal
func close_with_reason(reason: String = "") -> void
```

`get_local_chat_control()` and `get_peer_chat_control(peer_id)` are advanced escape hatches. Normal title code should prefer the peer-id helper methods so it does not maintain Party chat-control identity itself.

## Transport semantics

`PlayFabPartyPeer` implements `MultiplayerPeerExtension` semantics:

1. `_poll()` drains queues populated by Party auto dispatch. It does not call the Party state-change pump.
2. `_put_packet()` sends the current packet to the current target peer using internal Party endpoint routing.
3. `_get_packet()`, `_get_available_packet_count()`, `_get_packet_peer()`, `_get_packet_mode()`, and `_get_packet_channel()` expose queued gameplay packets.
4. `_set_target_peer()` supports broadcast, server, specific positive peer ids, and negative peer ids interpreted as broadcast-except-target.
5. `_set_transfer_mode()` maps Godot reliable/unreliable/unreliable-ordered modes to the closest Party send options. Any downgrade from ENet parity must be documented.
6. `_set_transfer_channel()` maps Godot channels where possible; otherwise channel metadata is preserved in the packet envelope.
7. `_get_unique_id()` returns `1` for the host/server and a positive client id for each joined peer.
8. `_get_connection_status()` reflects Party network, auth, endpoint, and handshake readiness, not just SDK call success.
9. `_is_server()` is true for the peer that created the Party network.
10. `_close()` and `_disconnect_peer()` disconnect internal routing/network resources and emit Godot-consistent state transitions.

Peer-id handshake:

1. Host always starts as Godot peer id `1`.
2. After a client authenticates and its endpoint is ready, it sends a reserved transport-control packet with its `PlayFabEntityKey` and a random join nonce to the host endpoint.
3. Host allocates the next positive peer id, records `{peer_id, PlayFabEntityKey, Party endpoint}`, and replies with a reserved assignment packet.
4. Client stores the assigned peer id, transitions to connected, and includes the assigned source peer id in future gameplay packet envelopes.
5. If assignment does not complete before timeout, join fails with `party_peer_not_connected` and the network closes.

Reserved handshake/control packets are filtered out of `_get_packet()` so Godot RPC code only sees gameplay packets.

## Chat, mute, and permissions

Text chat, transcription, mute, and permissions use Party chat-control APIs internally. They are not sent through `_put_packet()`, `_get_packet()`, or Godot RPCs.

Peer-facing rules:

1. `send_text_async(message, target_peer_ids, config)` resolves target peer ids to known Party chat controls.
2. An empty `target_peer_ids` array sends to all currently known remote chat controls.
3. Specific positive peer ids target those peers.
4. Unknown or invalid peer ids return a failed `PlayFabResult`; messages must not be silently dropped.
5. `set_peer_chat_permissions_async(peer_id, permissions)` and `set_peer_muted_async(peer_id, muted)` wrap the local chat-control relationship APIs for the target peer.
6. Native relationship outcomes update the peer permission/mute cache and emit `chat_permissions_changed` / `peer_muted_changed`.
7. Platform privacy and privilege policy is not automatic in the first pass because PartyXbl is out of scope. Title code can resolve policy through existing GDK/Xbox services and apply the result through peer permission helpers.

Recommended permission constants:

```gdscript
PlayFabParty.CHAT_PERMISSION_NONE
PlayFabParty.CHAT_PERMISSION_SEND_AUDIO
PlayFabParty.CHAT_PERMISSION_RECEIVE_AUDIO
PlayFabParty.CHAT_PERMISSION_RECEIVE_TEXT
```

> **Text-chat permission model:** PlayFab Party does not expose a separate `SEND_TEXT` permission flag. The local chat control may send text chat to any peer; the recipients are determined by the `targets` argument of `send_text_async()`. `RECEIVE_TEXT` controls whether the local chat control will receive text chat from a particular peer.

Advanced wrappers remain available:

```gdscript
class_name PlayFabPartyChat
extends RefCounted

signal state_changed(change: PlayFabPartyChatStateChange)

func get_local_chat_control(user: PlayFabUser) -> PlayFabPartyChatControl
func get_chat_controls() -> Array[PlayFabPartyChatControl]
```

```gdscript
class_name PlayFabPartyChatControl
extends RefCounted

signal state_changed(change: PlayFabPartyChatStateChange)
signal message_received(message: PlayFabPartyChatMessage)
signal transcription_received(message: PlayFabPartyChatMessage)

var id: String
var user: PlayFabUser
var is_voice_enabled: bool
var is_text_enabled: bool
var is_transcription_enabled: bool

func get_id() -> String
func get_user() -> PlayFabUser
func send_text_async(targets: Array[PlayFabPartyChatControl], message: String, config: PlayFabPartyTextMessageConfig = null) -> Signal
func set_permissions_async(target: PlayFabPartyChatControl, permissions: int) -> Signal
func set_muted_async(target: PlayFabPartyChatControl, muted: bool) -> Signal
func destroy_async() -> Signal
```

```gdscript
class_name PlayFabPartyChatMessage
extends RefCounted

var sender: PlayFabPartyChatControl
var sender_entity_key: PlayFabEntityKey
var targets: Array[PlayFabPartyChatControl]
var text: String
var language_code: String
var translated_text: String
var is_transcription: bool
var timestamp: int
var metadata: Dictionary
```

## Member and state-change wrappers

```gdscript
class_name PlayFabPartyMember
extends RefCounted

var peer_id: int
var entity_key: PlayFabEntityKey
var user: PlayFabUser
var is_local: bool

func get_peer_id() -> int
func get_entity_key() -> PlayFabEntityKey
func get_user() -> PlayFabUser
func is_local_member() -> bool
```

```gdscript
class_name PlayFabPartyNetworkStateChange
extends RefCounted

var kind: int
var network: PlayFabPartyNetwork
var result: PlayFabResult
var user: PlayFabUser
var peer_id: int
var state: int
var reason: String
```

Recommended stable constants:

```gdscript
PlayFabParty.NETWORK_STATE_CREATING
PlayFabParty.NETWORK_STATE_CONNECTING
PlayFabParty.NETWORK_STATE_AUTHENTICATING
PlayFabParty.NETWORK_STATE_CONNECTED
PlayFabParty.NETWORK_STATE_DISCONNECTING
PlayFabParty.NETWORK_STATE_DISCONNECTED
PlayFabParty.NETWORK_STATE_FAILED
```

## Example usage

### Create and join a Party network

```gdscript
func host_party_game(playfab_user: PlayFabUser) -> PlayFabPartyPeer:
    var config := PlayFabPartyConfig.new()
    config.max_players = 4
    config.enable_voice_chat = true
    config.enable_text_chat = true

    var result = await PlayFab.party.create_and_join_network_async(playfab_user, config)
    if not result.ok:
        push_warning(result.message)
        return null

    var network: PlayFabPartyNetwork = result.data
    var peer: PlayFabPartyPeer = network.get_local_peer()
    peer.connection_state_changed.connect(_on_party_peer_state_changed)
    peer.text_message_received.connect(_on_party_text_message)
    peer.peer_muted_changed.connect(_on_party_peer_muted_changed)

    multiplayer.multiplayer_peer = peer

    # Share this through a lobby property, invite, matchmaking follow-up, or backend.
    print("Party descriptor: ", network.get_descriptor())
    return peer
```

### Join a Party network

```gdscript
func join_party_game(playfab_user: PlayFabUser, descriptor: String) -> PlayFabPartyPeer:
    var config := PlayFabPartyConfig.new()
    config.enable_voice_chat = true
    config.enable_text_chat = true

    var result = await PlayFab.party.join_network_async(playfab_user, descriptor, config)
    if not result.ok:
        push_warning(result.message)
        return null

    var network: PlayFabPartyNetwork = result.data
    var peer: PlayFabPartyPeer = network.get_local_peer()
    peer.connection_state_changed.connect(_on_party_peer_state_changed)
    peer.text_message_received.connect(_on_party_text_message)

    multiplayer.multiplayer_peer = peer
    return peer
```

### Use Godot RPCs for gameplay

```gdscript
@rpc("any_peer", "reliable")
func submit_ready_state(is_ready: bool) -> void:
    print("Peer ready: ", multiplayer.get_remote_sender_id(), " -> ", is_ready)

func send_ready() -> void:
    submit_ready_state.rpc(true)
```

### Send text and mute by peer id

```gdscript
func send_party_text(peer: PlayFabPartyPeer, target_peer_id: int, message: String) -> void:
    var result = await peer.send_text_async(message, PackedInt32Array([target_peer_id]))
    if not result.ok:
        push_warning(result.message)

func mute_peer(peer: PlayFabPartyPeer, target_peer_id: int) -> void:
    var result = await peer.set_peer_muted_async(target_peer_id, true)
    if not result.ok:
        push_warning(result.message)
```

### Apply title/platform communication policy

```gdscript
func apply_text_only_policy(peer: PlayFabPartyPeer, target_peer_id: int) -> void:
    # Receive text from the peer; do not receive their voice audio and do not
    # send our voice audio to them. Text-send is implicit (per-call recipients
    # via send_text_async()), so there is no SEND_TEXT flag.
    var permissions = PlayFabParty.CHAT_PERMISSION_RECEIVE_TEXT

    var result = await peer.set_peer_chat_permissions_async(target_peer_id, permissions)
    if not result.ok:
        push_warning(result.message)
```

### Leave a Party network

```gdscript
func leave_party(network: PlayFabPartyNetwork) -> void:
    if network == null:
        return

    var result = await PlayFab.party.leave_network_async(network)
    if not result.ok:
        push_warning(result.message)
        return

    multiplayer.multiplayer_peer = null
```

## Composition with PlayFab Multiplayer

Party does not know about lobbies or matchmaking. To use lobby discovery, title code creates and joins a Party network first, then stores `network.descriptor` in a lobby property such as `party_descriptor`. Joiners read that property from `PlayFabLobby` and explicitly call `PlayFab.party.join_network_async(...)`.

Matchmaking completion is also passive. A completed `PlayFabMatchTicket` can report an arranged-lobby connection string; title code decides whether to call `PlayFab.multiplayer.join_arranged_lobby_async(...)`, inspect lobby properties, and then join Party.

## Error/result conventions

Use stable error codes so GDScript callers can branch:

```gdscript
"party_not_initialized"
"party_already_initialized"
"party_invalid_user"
"party_invalid_options"
"party_network_already_active"
"party_network_create_failed"
"party_network_connect_failed"
"party_descriptor_invalid"
"party_transport_create_failed"
"party_peer_not_connected"
"party_resource_not_ready"
"party_chat_control_create_failed"
"party_chat_permission_failed"
```

All validation failures must update `PlayFab.last_error` consistently with existing PlayFab services and return a completed `Signal` with a failed `PlayFabResult`.

## Testing expectations

Add GUT coverage under `tests\godot\playfab\tests\` for:

- public class and service registration;
- invalid or missing `PlayFabUser` entity handles;
- invalid Party config and invalid descriptors;
- one active Party network per local PlayFab entity handle;
- finalized-descriptor-only exposure;
- immediate failure result shapes;
- resource-not-ready errors for provisional network, endpoint, peer, and chat-control wrappers;
- peer-id mapping helpers and host id `1`;
- negative target peer behavior;
- packet queue behavior and reserved control-packet filtering;
- chat helper validation, including unknown peer ids and missing chat controls;
- mute/permission helper result shapes;
- shutdown cleanup for active networks, endpoints, and chat controls.

Live Party tests must stay opt-in behind the repository's `LIVE_TESTS=1` / `-Live` path and use a sandbox PlayFab title.
