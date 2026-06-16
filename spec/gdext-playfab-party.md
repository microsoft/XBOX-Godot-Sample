# PlayFab Party GDExtension Spec

## Overview

This document defines the planned Godot-facing PlayFab Party surface for the `godot_playfab` addon.

`PlayFab.party` owns PlayFab Party initialization, Party network host/join flows, descriptor serialization, the Godot high-level multiplayer transport, and Party chat-control integration. The normal title-facing object is `PlayFabPartyPeer`: it is assigned to `multiplayer.multiplayer_peer` for RPC/gameplay traffic and also exposes peer-id-based helpers for common Party text chat, mute, and permission operations.

PlayFab Multiplayer lobbies and matchmaking remain separate and are described in `spec\gdext-playfab-lobby-matchmaking.md`. Title code composes the two systems by sharing finalized Party descriptors through title-owned lobby properties, invites, matchmaking follow-up flows, or backend services.

## Design goals

1. **Single root singleton** - expose Party through `PlayFab.party`, not a second engine singleton.
2. **Godot-native transport** - make `PlayFabPartyPeer` usable anywhere a Godot `MultiplayerPeer` would be assigned.
3. **One title-facing Party object** - titles should normally keep `PlayFabPartyPeer` for gameplay packets, descriptor access, and close/leave behavior; chat lives on the separate `PlayFab.party.chat` surface.
4. **Entity-handle APIs only** - public calls accept a signed-in `PlayFabUser`, and native calls use that user's internal `PFEntityHandle`.
5. **Hide native choreography** - titles should not manually call create local user, create/connect network, authenticate user, create endpoint, create chat control, or connect chat control.
6. **Final descriptors only** - never expose the provisional immediate descriptor from `CreateNewNetwork(...)`; expose only the finalized base64 serialized `PartyNetworkDescriptor`.
7. **Separate Party from Multiplayer** - Party does not create lobbies, matchmaking tickets, or arranged-lobby joins.
8. **Chat is not packet transport** - text, transcription, mute, and permissions use Party chat-control APIs internally and are surfaced on the meshed `PlayFab.party.chat` ([PlayFabPartyChat]) surface keyed by PlayFab entity keys, not on the transport peer and not through Godot RPC packets.
9. **One chat control per local user** - the local Party chat control is created once for a local user (first network join), reused (reconnected) across networks, and destroyed only on user release or Party shutdown.

## Scope

| Domain | Included | Notes |
| --- | --- | --- |
| Party runtime | Yes | `PartyManager::Initialize`, state-change pumping, cleanup |
| Party host/join | Yes | convenience async operations return ready networks |
| Descriptor serialization | Yes | finalized descriptor only, base64 string |
| Godot high-level transport | Yes | `PlayFabPartyPeer` implements `MultiplayerPeerExtension` semantics |
| Peer-id mapping | Yes | host is peer `1`; clients receive positive ids through transport handshake |
| Data endpoints | Yes | internal only; gameplay uses Godot peer ids |
| Text chat / transcription | Yes | exposed on the meshed `PlayFab.party.chat` surface, keyed by PlayFab entity keys |
| Mute / chat permissions | Yes | exposed on the meshed `PlayFab.party.chat` surface, keyed by PlayFab entity keys |
| Per-chat-control access | Yes | `PlayFabPartyChatControl` objects are reachable via `PlayFab.party.chat` for advanced flows |
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
func initialize_async(config: PlayFabPartyConfig = null, local_udp_port: int = -1) -> Signal
func shutdown_async() -> Signal

func create_and_join_network_async(user: PlayFabUser, config: PlayFabPartyConfig = null) -> Signal
func join_network_async(user: PlayFabUser, descriptor: String, config: PlayFabPartyConfig = null) -> Signal
func leave_network_async(network: PlayFabPartyNetwork) -> Signal
func release_local_user_async(user: PlayFabUser) -> Signal

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
| `release_local_user_async()` | `null` |

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
7. Optionally create/connect the local chat control when chat is enabled. The local chat control's capture (microphone) and render (speaker) audio devices are always bound on creation; the `audio_input`/`audio_output` config fields choose specific devices and fall back to the system default communication device when empty. Audio binding is best-effort: device-init failures surface via `LocalChatAudioInputChanged`/`LocalChatAudioOutputChanged` warnings (degrading voice) without failing the join.
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
7. Optionally create/connect the local chat control when chat is enabled. The local chat control's capture (microphone) and render (speaker) audio devices are always bound on creation; the `audio_input`/`audio_output` config fields choose specific devices and fall back to the system default communication device when empty. Audio binding is best-effort: device-init failures surface via `LocalChatAudioInputChanged`/`LocalChatAudioOutputChanged` warnings (degrading voice) without failing the join.
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

`enable_voice_chat`, `enable_text_chat`, transcription, translation, and audio fields are addon policy flags. They decide whether and how the addon creates/connects Party chat controls; they are not native Party network settings. The local chat control's capture and render audio devices are always bound when the chat control is created (regardless of `enable_voice_chat`): `audio_input`/`audio_output` select a specific device id, and an empty string selects the platform's system default communication device. Whether voice actually flows still depends on chat permissions (`SendAudio`/`ReceiveAudio` granted via `set_chat_permissions_async`) and the audio subsystem successfully initializing the chosen devices; the addon logs warnings for non-`Initialized` audio device states to surface silent device-init failures.

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

func get_network() -> PlayFabPartyNetwork
func get_local_user() -> PlayFabUser
func get_descriptor() -> String
func get_peer_entity_key(peer_id: int) -> Dictionary
func get_peer_member(peer_id: int) -> PlayFabPartyMember
func get_peers() -> Array[int]
func close_with_reason(reason: String = "") -> void
```

Chat is not on the transport peer. Text, transcription, mute, and permissions
live on the single meshed `PlayFab.party.chat` ([PlayFabPartyChat]) surface,
keyed by PlayFab entity-key `Dictionary` (`{ "id", "type" }`), not by transport
peer id. Resolve a peer's entity key with `get_peer_entity_key(peer_id)` when you
need to correlate a transport peer with a chat control.

```gdscript
class_name PlayFabPartyChat
extends RefCounted

signal state_changed(change: PlayFabPartyChatStateChange)
signal chat_control_added(entity_key: Dictionary, chat_control: PlayFabPartyChatControl)
signal chat_control_removed(entity_key: Dictionary)
signal text_message_received(entity_key: Dictionary, message: PlayFabPartyChatMessage)
signal transcription_received(entity_key: Dictionary, message: PlayFabPartyChatMessage)
signal chat_permissions_changed(entity_key: Dictionary, permissions: int)
signal audio_muted_changed(entity_key: Dictionary, muted: bool)
signal text_muted_changed(entity_key: Dictionary, muted: bool)

func get_local_chat_control(user: PlayFabUser) -> PlayFabPartyChatControl
func get_chat_controls() -> Array
func get_remote_entity_keys() -> Array
func get_chat_control(entity_key: Dictionary) -> PlayFabPartyChatControl
func create_local_chat_control_async(user: PlayFabUser, config: PlayFabPartyConfig = null) -> Signal
func destroy_local_chat_control_async(user: PlayFabUser) -> Signal
func send_text_async(message: String, target_entity_keys: Array = [], config: PlayFabPartyTextMessageConfig = null) -> Signal
func set_chat_permissions_async(entity_key: Dictionary, permissions: int) -> Signal
func set_audio_muted_async(entity_key: Dictionary, muted: bool) -> Signal
func set_text_muted_async(entity_key: Dictionary, muted: bool) -> Signal
```

The chat mesh surfaces every connected chat control — no host relay. An empty
`target_entity_keys` array on `send_text_async()` broadcasts to the whole mesh.
There is exactly one local chat control per local user. Chat-control creation is
**decoupled from network join**: title code creates it explicitly via
`create_local_chat_control_async(user, config)` (which owns the audio in/out
devices and the voice/text/transcription flags from `config`), and then each
`create_and_join_network_async()` / `join_network_async()` for that user
connects the existing control to the network automatically. Network join never
creates a chat control — joining with chat enabled but no pre-created control
simply yields a network with no chat (a warning is logged). The control is
reused (via `ConnectChatControl`) across subsequent networks and destroyed only
via `destroy_local_chat_control_async(user)`, user release, or Party shutdown.
`create_local_chat_control_async()` is idempotent — a second call for an
existing control resolves with that control — and its completed signal carries
the `PlayFabPartyChatControl`. `get_local_chat_control(user)` and
`get_chat_control(entity_key)` are advanced escape hatches; normal title code
should prefer the entity-key helper methods so it does not maintain Party
chat-control identity itself.

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
2. The host marks its own endpoint at `CreateEndpoint` time with an immutable shared property (`pf.role` = `host`). Party endpoint shared properties cannot be changed after creation and are owned by the creating endpoint, so exactly one endpoint in the network carries the marker. Every device reads it off the remote endpoint (`PartyEndpoint::GetSharedProperty`) to identify the host deterministically.
3. After a client authenticates and its endpoint is ready, it sends a reserved transport-control packet with its entity-key `Dictionary` and a random join nonce to the host endpoint — the endpoint identified by the `pf.role=host` marker. A remote endpoint whose role cannot be read is contacted defensively (so a transient property-read failure cannot strand the join); client endpoints (marker absent) are skipped.
4. Host allocates the next positive peer id, records `{peer_id, entity-key Dictionary, Party endpoint}`, and replies with a reserved assignment packet.
5. Client stores the assigned peer id, transitions to connected, and includes the assigned source peer id in future gameplay packet envelopes.
6. If assignment does not complete before timeout, join fails with `party_peer_not_connected` and the network closes.

Reserved handshake/control packets are filtered out of `_get_packet()` so Godot RPC code only sees gameplay packets.

## Chat, mute, and permissions

Text chat, transcription, mute, and permissions use Party chat-control APIs internally. They are not sent through `_put_packet()`, `_get_packet()`, or Godot RPCs.

Peer-facing rules:

1. `send_text_async(message, target_entity_keys, config)` resolves target PlayFab entity keys to known Party chat controls.
2. An empty `target_entity_keys` array sends to all currently known remote chat controls.
3. Specific entity-key `Dictionary` values (`{ "id", "type" }`) target those peers.
4. Unknown or invalid entity keys return a failed `PlayFabResult`; messages must not be silently dropped.
5. `set_chat_permissions_async(entity_key, permissions)`, `set_audio_muted_async(entity_key, muted)`, and `set_text_muted_async(entity_key, muted)` wrap the local chat-control relationship APIs for the target peer. Voice and text mute are independent (Party's `SetIncomingAudioMuted` vs `SetIncomingTextMuted`): muting voice does not block text and vice versa.
6. Native relationship outcomes update the peer permission/mute cache and emit `chat_permissions_changed` / `audio_muted_changed` / `text_muted_changed` (all carry the target peer's entity-key `Dictionary`).
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
func set_audio_muted_async(target: PlayFabPartyChatControl, muted: bool) -> Signal
func set_text_muted_async(target: PlayFabPartyChatControl, muted: bool) -> Signal
func destroy_async() -> Signal
```

```gdscript
class_name PlayFabPartyChatMessage
extends RefCounted

var sender: PlayFabPartyChatControl
var sender_entity_key: Dictionary
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
var entity_key: Dictionary
var user: PlayFabUser
var is_local: bool

func get_peer_id() -> int
func get_entity_key() -> Dictionary
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

    # Phase C — create the local chat control explicitly before joining. This
    # owns the audio devices and voice/text/transcription flags; the subsequent
    # create_and_join / join connects it to the network automatically.
    var chat_result = await PlayFab.party.chat.create_local_chat_control_async(playfab_user, config)
    if not chat_result.ok:
        push_warning(chat_result.message)
        return null

    var result = await PlayFab.party.create_and_join_network_async(playfab_user, config)
    if not result.ok:
        push_warning(result.message)
        return null

    var network: PlayFabPartyNetwork = result.data
    var peer: PlayFabPartyPeer = network.get_local_peer()
    peer.connection_state_changed.connect(_on_party_peer_state_changed)
    # Chat is meshed onto the single persistent PlayFab.party.chat surface;
    # wire its signals once (e.g. at startup), not per network/peer.
    PlayFab.party.chat.text_message_received.connect(_on_party_text_message)
    PlayFab.party.chat.audio_muted_changed.connect(_on_party_audio_muted_changed)

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

    # Phase C — create the local chat control explicitly before joining.
    var chat_result = await PlayFab.party.chat.create_local_chat_control_async(playfab_user, config)
    if not chat_result.ok:
        push_warning(chat_result.message)
        return null

    var result = await PlayFab.party.join_network_async(playfab_user, descriptor, config)
    if not result.ok:
        push_warning(result.message)
        return null

    var network: PlayFabPartyNetwork = result.data
    var peer: PlayFabPartyPeer = network.get_local_peer()
    peer.connection_state_changed.connect(_on_party_peer_state_changed)
    # Chat lives on PlayFab.party.chat; connect its signals once, not per peer.
    PlayFab.party.chat.text_message_received.connect(_on_party_text_message)

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

### Send text and mute by entity key

```gdscript
func send_party_text(peer: PlayFabPartyPeer, target_peer_id: int, message: String) -> void:
    var entity_key := peer.get_peer_entity_key(target_peer_id)
    var result = await PlayFab.party.chat.send_text_async(message, [entity_key])
    if not result.ok:
        push_warning(result.message)

func mute_peer(peer: PlayFabPartyPeer, target_peer_id: int) -> void:
    var entity_key := peer.get_peer_entity_key(target_peer_id)
    # Voice and text mute are independent; mute voice here, set_text_muted_async for text.
    var result = await PlayFab.party.chat.set_audio_muted_async(entity_key, true)
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
    var entity_key := peer.get_peer_entity_key(target_peer_id)

    var result = await PlayFab.party.chat.set_chat_permissions_async(entity_key, permissions)
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

All validation failures must return an already-completed `Signal` with a failed `PlayFabResult`.

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

## Plan

Incremental hardening of the Party↔Godot integration. Each phase lands a
self-contained slice with its own validation.

- **Phase A — deterministic host identification (host endpoint marker).**
  The host marks its own endpoint at `CreateEndpoint` with an immutable
  `pf.role=host` shared property; clients read it to target the join
  handshake at the host deterministically instead of contacting every
  remote and relying on "only the host replies." No public API change;
  star topology unchanged. Tier: non-live GUT + single-client live Party.
- **Phase B — full mesh (deferred, may not be needed).** Optionally flip
  Godot peer registration to full uid-based mesh using
  `PartyEndpoint::GetUniqueIdentifier()` (network-consistent) so chat,
  voice, and Godot RPC become all-to-all with the host still id `1`.
  Deferred; revisit only if all-to-all RPC is required.
- **Phase C — decouple chat-control creation from the network.** Lift the
  local chat-control lifecycle (create / configure / destroy) out of the
  network-join path and expose it explicitly as
  `PlayFab.party.chat.create_local_chat_control_async(user, config)` /
  `destroy_local_chat_control_async(user)`. The control is owned by the local
  user (the SDK already scopes `PartyLocalChatControl` to the device), so its
  audio devices and voice/text/transcription flags are applied where the
  control lives; network join only *connects* an existing control and never
  creates one. **Breaking:** titles must create the chat control before joining
  with chat. Tier: non-live GUT (API surface) + live Party/MP orchestrator
  (runtime).
- **Phase D — independent voice/text mute.** Split the single audio-only
  `set_muted_async` into `set_audio_muted_async` (Party `SetIncomingAudioMuted`)
  and `set_text_muted_async` (Party `SetIncomingTextMuted`) on both
  `PlayFabPartyChat` (entity-key keyed) and `PlayFabPartyChatControl`, with
  matching `audio_muted_changed` / `text_muted_changed` signals. Voice and text
  mute are independent in the SDK; the prior API could only mute voice.
  **Breaking:** `set_muted_async` / `muted_changed` are renamed. Tier: non-live
  GUT (API surface) + live MP orchestrator (`party.chat.mute_peer` mutes text and
  asserts text delivery is blocked; `party.chat.text.three_clients` validates host
  fan-out to both guests).

## Progress

- **Phase A: ✅ implemented.** `addons/godot_playfab/src/playfab_party.cpp`:
  host sets `pf.role=host` on its endpoint; `endpoint_is_handshake_target()`
  drives client→host handshake targeting in both the join-enumeration loop
  and `EndpointCreated`. Validated: debug build + addon mirror, parse gate,
  full **non-live** orchestrator (`run_all_tests.ps1`) green, and all **live**
  GUT hosts green against sandbox title `10D176` (single-client live Party +
  lobby tests exercise the marker path). The multi-client MP orchestrator
  shows the **same** 34/27 pass/fail with and without Phase A (verified by an
  A/B run via the new `-SkipGut`), so its failures —
  `party_invalid_user: local user limit reached`, matchmaking
  `arranged_lobby_join_start_failed`, and live `timeout` — are pre-existing
  and unrelated to this change.
- **Phase B: ⬜ deferred** pending a need for all-to-all RPC.
- **Phase C: ✅ implemented.** Chat-control creation is decoupled
  from network join. `PlayFabPartyChat` gains
  `create_local_chat_control_async(user, config)` (idempotent; completed signal
  carries the `PlayFabPartyChatControl`) and `destroy_local_chat_control_async(user)`.
  `addons/godot_playfab/src/playfab_party.cpp`: `_create_local_chat_control` /
  `_destroy_local_chat_control` drive the explicit lifecycle;
  `_process_create_chat_control_completed` is now the terminal step of an
  explicit create (no `ConnectChatControl` tail); the post-authenticate join
  step only *connects* an existing per-user control and never creates one;
  audio-device backing strings moved from `PlayFabPartyNetwork` onto
  `PlayFabPartyChatControl` (the control now outlives any single network).
  Consumers migrated to create the control before join: tutorial autoloads
  (`sample/tutorial_playfab`, `sample/tutorial_integrated`), the MP harness
  (`tests/godot/mp_test_client/scripts/playfab_party_ops.gd`), and the
  `test_party.gd` API-surface list. Validated: debug build + addon mirror,
  parse gate, and the full non-live tier (`run_all_tests.ps1 -SkipOrchestrator`:
  368 GUT tests, 0 failed). **Live Party / MP orchestrator runtime validation
  attempted against title `10D176` but inconclusive** — that title is not
  provisioned for the orchestrator (host `create_lobby` calls time out at 60 s;
  `PlayFabCustomId`/`PlayFabMatchmakingQueue` unset). The Party failures were
  `party_invalid_user` ("local user limit reached"), which is a pre-existing
  accumulation unrelated to Phase C (see follow-up below), not a regression from
  this change. Re-run on a configured sandbox title before merge.
- **Phase D: ✅ implemented.** `set_muted_async` → `set_audio_muted_async`
  + `set_text_muted_async` on both `PlayFabPartyChat` (entity-key) and
  `PlayFabPartyChatControl` (target), backed by `PlayFabParty::_set_incoming_audio_muted`
  / new `_set_incoming_text_muted`; `muted_changed` → `audio_muted_changed` /
  `text_muted_changed`. doc_classes, spec, the PlayFab/integrated tutorial
  autoloads + integrated chat panel (mute toggle = voice), `test_party.gd`
  surface/detached-error assertions, and the MP harness (`party_set_peer_muted`
  gains a `channel` of `audio`/`text`/`both`) updated to match. The MP
  `party.chat.mute_peer` flow now mutes the **text** channel and asserts text is
  blocked (previously it muted audio and vacuously "passed" only because text
  never flowed).
  - **`_party_triplet`** (`mp_scenario_utils.gd`) was corrected to the
    host-centric star — the host converges to 2 transport peers while each guest
    converges to 1 (it previously expected an all-to-all `peer_count == 2`, which
    is Phase B and is deferred). It then waits on a new chat-mesh readiness gate
    (`remote_chat_control_count >= 2` on each role, surfaced from the client
    snapshot) so a chat send isn't raced against chat-control discovery.
  - **`party.chat.text.three_clients`** now validates host **fan-out** (host
    broadcasts; both guests receive). Direct guest→guest text is full-mesh
    territory (Phase B) and is intentionally not exercised. Root cause of the
    earlier 3-client failure was a **permission-readiness race**, not transport:
    the test client's fire-and-forget `_grant_default_chat_permissions` (issued on
    `chat_control_added`) can land before the peer's chat control is messaging-ready
    in the slower 3-client convergence, so the host's text was filtered out at the
    guests. The scenario now explicitly grants and **awaits** `RECEIVE_TEXT` for the
    host (peer id 1) on each guest via the retrying `party_set_peer_chat_permissions`
    gate — the same readiness probe the issue #73 rejoin scenario uses — before
    broadcasting, and re-sends until delivered. With this it passes reliably (~3–6s).
  - Validated: debug build + addon mirror, parse gate, non-live GUT tier, and
    **live** on sandbox title `10D176`: `party.chat.text.round_trip`,
    `party.chat.mute.peer` (text mute blocks delivery), and
    `party.chat.text.three_clients` all green.

### Pre-existing follow-up (not Phase A)

The MP orchestrator (`tests\godot\mp_orchestrator`) exhausts the Party SDK's
per-device local-user limit across its ~61 back-to-back scenarios
(`party_invalid_user: can't create local user; local user limit reached`), and
the leaked local chat controls land in a later same-device chat broadcast target
list, which the SDK rejects (`party_chat_permission_failed`: "tried to send to a
local target; loopback isn't yet supported"). Root cause: `_release_local_user`
(`addons/godot_playfab/src/playfab_party.cpp`) was dead code — it was never
called, so local users created by `_get_or_create_local_user` (and their reusable
local chat controls) were only freed at full Party shutdown
(`_release_all_local_users`), never between sessions.

**Resolved.** `PlayFab.party.release_local_user_async(user)` now exposes
`_release_local_user`: it tears down the user's local chat control and destroys
the `PartyLocalUser`, freeing the per-device slot. It is idempotent (releasing an
unknown/already-released user resolves with success), and a later create/join for
the same user re-creates the user and control. The MP harness `reset()`
(`tests/godot/mp_test_client/scripts/playfab_party_ops.gd`) calls it after leaving
networks, so each scenario starts with no accumulated local users or chat
controls. Pre-existing and unchanged by Phase C (Phase C's second caller of
`_get_or_create_local_user` reuses the per-handle cache and adds no
`CreateLocalUser` calls).
