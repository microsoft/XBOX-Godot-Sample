# Tutorial 7 — Stand up a PlayFab Party network for voice and RPC

## What you'll build

A peer-to-peer game session that uses your [Tutorial 5
lobby](05-multiplayer-lobby.md) for discovery and a **PlayFab Party
network** for the actual transport — Godot RPC traffic plus voice
chat ride on the same Party peer. By the end you will:

- Initialize `PlayFab.party` on top of the existing PlayFab runtime.
- **Host:** create a Party network with
  `create_and_join_network_async`, wait for the finalized network
  descriptor, and publish it through the lobby's lobby-properties so
  every other lobby member can find it.
- **Client:** read the descriptor from the lobby, call
  `join_network_async`, and join the same Party network.
- Assign the `PlayFabPartyPeer` returned by the network to
  `multiplayer.multiplayer_peer` and call a one-line RPC.
- Send a text chat message and mute / unmute a remote peer using the
  per-peer chat helpers on `PlayFabPartyPeer`.
- Tear the network down on the way out so the next test run starts
  clean.

Sample output (host side):

```
[Party] Party initialized (voice=true text=true transcription=false)
[Party] Network created — waiting for descriptor…
[Party] Descriptor ready, publishing on the lobby
[Party] Peer connected: id=2 entity=title_player_account:6F4B…
[Party] RPC from peer 2: "ready"
[Party] Text from peer 2: "gg"
[Party] Peer 2 left, leaving network
```

## Prerequisites

- [Tutorial 1 — Sign in a user](01-sign-in-user.md) is complete and
  `Auth.playfab_user` resolves to an Xbox-backed PlayFab session on
  both sides. Custom-ID sessions also work for Party but Xbox-shell
  invites will not.
- [Tutorial 5 — Create and join a lobby](05-multiplayer-lobby.md) is
  complete. This tutorial assumes the host has already called
  `PlayFab.multiplayer.create_lobby_async(...)` and the client has
  already called `PlayFab.multiplayer.join_lobby_async(...)`, so both
  sides have a live `PlayFabLobby` and the lobby's `state_changed`
  signal is connected.
- [Tutorial 6 — Multiplayer Activity](06-multiplayer-activity.md)
  is recommended. The lobby integration in T6 is not a hard
  prerequisite for Party — Party only needs a live `PlayFabLobby`
  to publish its descriptor — but most titles ship Party + MPA
  together so friends can both **find** and **talk to** each
  other in the same session.
- The title-side Party configuration is in place: PlayFab
  Multiplayer → Party is enabled in Game Manager. Recently created
  titles enable this by default; older titles require the feature to
  be enabled manually. See
  [PlayFab title prerequisites — §2 Party](../playfab/prerequisites.md#party-t7-t8).
- Two Godot processes (host + client), each signed into a different
  Xbox test account in the same sandbox. As with the lobby tutorial,
  the easiest setup is one editor scene as host and an exported
  build as client; two editors with different PlayFab sessions also
  work.
- A working microphone on both sides if you want to test the voice
  path. Text and RPC traffic work without a mic.
- One-page primer on the addons' async model: [Async patterns](../async-patterns.md).

## Relevant addon surfaces

- [`PlayFab.party`](../../addons/godot_playfab/doc_classes/PlayFabParty.xml) —
  `initialize_async`, `create_and_join_network_async`,
  `join_network_async`, signal `party_error`.
- [`PlayFabPartyConfig`](../../addons/godot_playfab/doc_classes/PlayFabPartyConfig.xml) — Party network
  configuration (voice, text, max players, direct peer
  connectivity). `set_voice_chat_enabled` and
  `set_text_chat_enabled` decide which chat surfaces are wired
  for the network — Step 6 reads the local user's privileges
  before flipping them on.
- [`PlayFabPartyNetwork`](../../addons/godot_playfab/doc_classes/PlayFabPartyNetwork.xml) — the network
  handle returned by create / join; carries the peer list,
  descriptor, `local_peer`, and the `state_changed(change)`
  signal that carries network lifecycle changes.
- [`PlayFabPartyPeer`](../../addons/godot_playfab/doc_classes/PlayFabPartyPeer.xml) — the per-peer chat
  surface: `send_text_async`, `set_peer_muted_async`,
  `set_peer_chat_permissions_async`,
  `text_message_received(peer_id, message)`,
  `chat_control_added`, `chat_permissions_changed`.
- [`PlayFab.multiplayer`](../../addons/godot_playfab/doc_classes/PlayFabMultiplayer.xml) — used here only to
  publish the network descriptor through lobby properties.
- [`GDK.users`](../../addons/godot_gdk/doc_classes/GDKUsers.xml)
  — `check_privilege_async`. Step 6 checks the local user's
  **Communications** privilege (covers text + voice chat) and
  the **CommunicationVoiceIngame** privilege (specifically in-game
  voice) before the chat UI is exposed.
- [`GDK.privacy`](../../addons/godot_gdk/doc_classes/GDKPrivacy.xml)
  — `check_permission_async`. Step 6 uses it with the
  `communicate_using_voice` / `communicate_using_text` tokens to
  filter **per-peer** chat permissions so a blocked or mute-list
  user does not get a live mic.

> **Party vs. Lobby.** A `PlayFabLobby` is a *roster* — it tracks
> who's in the session, holds owner state, and surfaces invites.
> A `PlayFabPartyNetwork` is the actual *transport* — RPC, voice
> chat, and text chat all flow through the network's
> `PlayFabPartyPeer`. The lobby is the discovery mechanism; the
> Party network is what your gameplay code talks over. Most titles
> create exactly one Party network per lobby and hand the network
> descriptor around through lobby properties, which is what this
> tutorial does.

## Step 1 — Bring up the Party autoload

PlayFab Party sits on top of `PartyManager` and must be initialized
once before any network method works. Do it after PlayFab itself is
ready (so it shares the runtime queue with the rest of the addon).
Party and Multiplayer are independent services so you don't have to
order them against each other.

Like Lobby in [Tutorial 5](05-multiplayer-lobby.md#step-1--bring-up-the-lobby-autoload),
initialize Party **lazily** on first host / join rather than eagerly
in `_ready`. Party allocates an audio engine and a network stack at
init time — paying that cost on app boot bloats every scene that
doesn't use voice. The pattern is the same `_ensure_initialized()`
helper guarded by a "signals connected" flag.

Pair the lazy init with the same `State` enum the Auth (T1) and
Lobby (T5) autoloads use. The state machine pays for itself on
Party because the autoload has more interlocked entry points than
either of its peers: `host_party` / `_join_party_network` /
`leave_party` plus three indirect triggers from the Lobby autoload
(`lobby_joined`, `lobby_left`, `lobby_disconnected`) and the
`NETWORK_CHANGE_DESTROYED` firehose from PlayFab itself. Without a
state machine these paths race; with one they reject or coalesce
deterministically.

> **Note — single-slot design.** Like the Lobby autoload, this one
> owns exactly one `PlayFabPartyNetwork` at a time and the
> host/join methods reject re-entrant calls. The PlayFab addon
> supports multiple live Party networks per process, so if your
> game needs concurrent networks (e.g. a persistent clan voice
> channel plus a per-match network), refactor `host_party` /
> `_join_party_network` to return the new `PlayFabPartyNetwork`
> and have the caller hold the reference. The single-slot choice
> lives entirely in this sample autoload, not in the addon.

```gdscript
extends Node

const PARTY_DESCRIPTOR_KEY := "party_descriptor"

enum State {
    UNINITIALIZED,    # autoload _ready has not finished sign-in / lobby wiring
    READY,            # signed in, lobby wiring up, no network; host/join allowed
    HOSTING,          # host_party() in flight (create_and_join_network_async)
    JOINING,          # _join_party_network() in flight (join_network_async)
    IN_NETWORK,       # active PlayFabPartyNetwork; chat/leave allowed
    LEAVING,          # leave_party() in flight
}

signal state_changed(state: State)
signal network_joined(network: PlayFabPartyNetwork)
signal network_left          # voluntary teardown (leave_party)
signal network_destroyed     # involuntary teardown (lobby host left, network error, shutdown)

var _state: State = State.UNINITIALIZED
var _network: PlayFabPartyNetwork = null
var _is_host: bool = false
var _pf_party_signals_connected: bool = false

# Abort flag for in-flight host/join when the lobby disappears mid-await.
var _abort_party_op: bool = false
# Set true while leave_party is unwinding; NETWORK_CHANGE_DESTROYED
# uses this to know the teardown is voluntary and skip double-emit.
var _teardown_in_progress: bool = false

# Guarded accessor — returns null unless we're actually in a network.
var network: PlayFabPartyNetwork:
    get:
        return _network if _state == State.IN_NETWORK else null

func is_in_network() -> bool: return _state == State.IN_NETWORK
func is_busy() -> bool:
    return _state == State.HOSTING or _state == State.JOINING or _state == State.LEAVING

func _set_state(next: State) -> void:
    if _state == next:
        return
    _state = next
    state_changed.emit(_state)

func _ready() -> void:
    if not await Auth.sign_in():
        return
    # Lobby wiring is cheap and does not require the PlayFab Party SDK
    # to be initialized; lazy-init happens on first host/join.
    Lobby.lobby_joined.connect(_on_lobby_joined_from_lobby_autoload)
    Lobby.lobby_left.connect(_on_lobby_left_from_lobby_autoload)
    Lobby.lobby_disconnected.connect(_on_lobby_left_from_lobby_autoload)
    _set_state(State.READY)
    print("[Party] autoload ready (PlayFab Party init is lazy)")

func _ensure_initialized() -> bool:
    if not PlayFab.party.is_initialized():
        var cfg := PlayFabPartyConfig.new()
        cfg.max_players = 8
        cfg.direct_peer_connectivity = PlayFabParty.DIRECT_PEER_CONNECTIVITY_ANY
        cfg.enable_voice_chat = true
        cfg.enable_text_chat = true
        cfg.enable_transcription = false  # Flip to true to receive
                                          # speech-to-text on transcription_received.

        var init: PlayFabResult = await PlayFab.party.initialize_async(cfg)
        if not init.ok:
            push_error("[Party] init failed: %s (%s)" % [init.message, init.code])
            return false
        print("[Party] initialized lazily (voice=true text=true transcription=false)")

    if not _pf_party_signals_connected:
        PlayFab.party.party_error.connect(_on_party_error)
        _pf_party_signals_connected = true
    return true


func _on_party_error(result: PlayFabResult) -> void:
    push_warning("[Party] party error: %s (%s)" % [result.message, result.code])
```

`PlayFabPartyConfig` is reused for both initialization and the per-
network create / join calls. The `enable_voice_chat`,
`enable_text_chat`, and `enable_transcription` flags are **addon
policy** — they decide whether the addon creates and connects a
chat control alongside the network endpoint. You can flip them per
network, not just at init time.

> **Voluntary vs involuntary teardown.** `network_left` fires when
> *we* called `leave_party`. `network_destroyed` fires when PlayFab
> tore the network down on us — the lobby host left, a network
> error killed the peer, or the engine is shutting down. UI usually
> handles them differently: `network_left` is "clean, ready to host
> again," while `network_destroyed` is "show a banner explaining
> the disconnection." Both reset the autoload to `READY`.

## Step 2 — Host: create the Party network

The lobby owner creates the Party network. The host's
`PlayFabPartyPeer` is **always** Godot peer id `1`, so the existing
`multiplayer.is_server()` and `multiplayer.get_unique_id() == 1`
checks in your gameplay code still mean what you expect:

```gdscript
func host_party() -> bool:
    if _state != State.READY:
        push_warning("[Party] host_party rejected — busy or already in network (state=%d)" % _state)
        return false

    _set_state(State.HOSTING)
    _abort_party_op = false
    _is_host = true

    if not await _ensure_initialized():
        _is_host = false
        _set_state(State.READY)
        return false

    var user: PlayFabUser = Auth.playfab_user
    var cfg := PlayFabPartyConfig.new()
    cfg.max_players = 4
    cfg.direct_peer_connectivity = PlayFabParty.DIRECT_PEER_CONNECTIVITY_ANY

    var result: PlayFabResult = await PlayFab.party.create_and_join_network_async(user, cfg)

    # The lobby may have disappeared while we were awaiting create_and_join.
    # Tear down the orphan network without binding it or emitting joined.
    if _abort_party_op or _state != State.HOSTING:
        if result.ok:
            result.data.leave_async()
        _abort_party_op = false
        _is_host = false
        if _state == State.HOSTING:
            _set_state(State.READY)
        return false

    if not result.ok:
        push_warning("[Party] create_and_join failed: %s (%s)" % [result.message, result.code])
        _is_host = false
        _set_state(State.READY)
        return false

    _network = result.data
    _network.state_changed.connect(_on_network_state_changed)
    _set_state(State.IN_NETWORK)
    network_joined.emit(_network)
    print("[Party] Network created — waiting for descriptor…")

    # The provisional descriptor is never exposed. The finalized base64
    # descriptor is published through state_changed with
    # NETWORK_CHANGE_DESCRIPTOR_UPDATED; we publish onto the lobby from
    # _on_network_state_changed once that fires (Step 3).
    if not _network.descriptor.is_empty():
        await _publish_descriptor_on_lobby(_network.descriptor, _network)
    return true


func _on_network_state_changed(change: PlayFabPartyNetworkStateChange) -> void:
    match change.kind:
        PlayFabParty.NETWORK_CHANGE_DESCRIPTOR_UPDATED:
            if _is_host and _state == State.IN_NETWORK and not _network.descriptor.is_empty():
                await _publish_descriptor_on_lobby(_network.descriptor, _network)
        PlayFabParty.NETWORK_CHANGE_PEER_JOINED:
            print("[Party] Peer connected: id=%d entity=%s" % [
                change.peer_id,
                str(_network.local_peer.get_peer_entity_key(change.peer_id)),
            ])
        PlayFabParty.NETWORK_CHANGE_PEER_LEFT:
            print("[Party] Peer %d left" % change.peer_id)
        PlayFabParty.NETWORK_CHANGE_STATE:
            print("[Party] State → %d (%s)" % [change.state, change.reason])
        PlayFabParty.NETWORK_CHANGE_ERROR:
            push_warning("[Party] network error: %s" % change.reason)
        PlayFabParty.NETWORK_CHANGE_DESTROYED:
            _handle_network_destroyed(change.reason)
```

`create_and_join_network_async` resolves as soon as the local user is
joined and connected. The **finalized** base64 descriptor may already
be populated when the await returns (the snippet above publishes it
immediately if so); if the descriptor is still empty at that point —
which can happen on slower-arriving change batches — the
`NETWORK_CHANGE_DESCRIPTOR_UPDATED` branch above publishes it as soon
as it arrives. Either way, the host publishes exactly once. The
provisional descriptor from the immediate Party API is intentionally
never exposed — only the finalized one is publishable.

> **Why the abort-after-await check?** `host_party` and
> `_join_party_network` are async — between the `await` going out
> and the await coming back, the user could leave the lobby, the
> lobby host could kick everyone, or the network connection could
> die. Without the `_abort_party_op` check the autoload would
> happily bind Godot's `multiplayer.multiplayer_peer` to a network
> whose backing lobby is already gone and emit `network_joined` to
> consumers who are about to receive `network_destroyed` anyway.
> The flag is set by `_on_lobby_left_from_lobby_autoload` when it
> sees `is_busy()` (Step 8).

## Step 3 — Host: publish the descriptor on the lobby

The simplest discovery flow is: write the descriptor into the
lobby's lobby-properties dictionary. Every lobby member already gets
a `PROPERTIES_UPDATED` event when properties change, so each client
sees the descriptor as soon as it's available:

```gdscript
var _lobby: PlayFabLobby = null  # Set this from Tutorial 5's create_lobby_async.


func _publish_descriptor_on_lobby(descriptor: String, expected_network: PlayFabPartyNetwork) -> void:
    # Re-check before writing — by the time the await on set_properties_async
    # returns, the host could have torn the network down (LEAVING) or a
    # newer network could be in flight. Publishing a stale descriptor would
    # send clients chasing a dead network.
    if _state != State.IN_NETWORK or _network != expected_network:
        return
    if _lobby == null:
        push_warning("[Party] No lobby to publish descriptor on")
        return
    if not _lobby.is_owner(Auth.playfab_user):
        # Lobby properties are owner-only; non-owners would fail asynchronously.
        return
    print("[Party] Descriptor ready, publishing on the lobby")
    var pf: PlayFabResult = await _lobby.set_properties_async({
        PARTY_DESCRIPTOR_KEY: descriptor,
    })
    # Re-check once more so we don't warn about a failure caused by our
    # own teardown.
    if not pf.ok and _state == State.IN_NETWORK and _network == expected_network:
        push_warning("[Party] descriptor publish failed: %s" % pf.message)
```

`set_properties_async` is lobby-wide and **only the current owner**
can push it, which is exactly the host in our case. Clients see the
update through `PlayFabLobby.PROPERTIES_UPDATED` on the
`state_changed` signal you wired in Tutorial 5.

> **Why not use `search_properties` instead?** `search_properties`
> uses PlayFab's reserved key namespace (`string_key1`, `string_key2`,
> …) and is meant for `find_lobbies_async` filtering, not for
> arbitrary blobs. Lobby-properties — plain `string -> string`
> entries — are the right place for the descriptor.

## Step 4 — Client: join the Party network

On the client side, watch the lobby's `state_changed` for a
`PROPERTIES_UPDATED` event and read the refreshed snapshot from
`change.lobby.properties` (the lobby snapshot is always refreshed
before `state_changed` fires, so `change.lobby.properties` carries
the new value; the separate `change.properties` field is not the
updated payload here). Then call `join_network_async`:

```gdscript
func _on_lobby_state_changed(change: PlayFabLobbyStateChange) -> void:
    # Tutorial 5 handled MEMBER_ADDED / MEMBER_REMOVED here; this branch
    # is the new piece for the Party tutorial.
    if change.kind != PlayFabLobby.PROPERTIES_UPDATED:
        return
    # Only the not-yet-joined client side cares about descriptor updates.
    # State.READY also rules out the host (HOSTING / IN_NETWORK) and any
    # in-flight client join (JOINING).
    if _is_host or _state != State.READY:
        return

    var descriptor: String = String(change.lobby.properties.get(PARTY_DESCRIPTOR_KEY, ""))
    if descriptor.is_empty():
        return  # Owner is still creating the network.

    await _join_party_network(descriptor)


func _join_party_network(descriptor: String) -> bool:
    if _state != State.READY:
        push_warning("[Party] join rejected — busy or already in network (state=%d)" % _state)
        return false

    _set_state(State.JOINING)
    _abort_party_op = false
    _is_host = false

    if not await _ensure_initialized():
        _set_state(State.READY)
        return false

    var user: PlayFabUser = Auth.playfab_user
    var cfg := PlayFabPartyConfig.new()
    cfg.enable_voice_chat = true
    cfg.enable_text_chat = true

    var result: PlayFabResult = await PlayFab.party.join_network_async(user, descriptor, cfg)

    # Same abort-after-await guard as host_party (Step 2).
    if _abort_party_op or _state != State.JOINING:
        if result.ok:
            result.data.leave_async()
        _abort_party_op = false
        if _state == State.JOINING:
            _set_state(State.READY)
        return false

    if not result.ok:
        push_warning("[Party] join_network failed: %s (%s)" % [result.message, result.code])
        _set_state(State.READY)
        return false

    _network = result.data
    _network.state_changed.connect(_on_network_state_changed)
    _set_state(State.IN_NETWORK)
    network_joined.emit(_network)
    print("[Party] Joined Party network: %s" % _network.network_id)
    return true
```

`PlayFabResult.data` for both create and join is the same
`PlayFabPartyNetwork` type, so the rest of the code path is
symmetric between host and client.

## Step 5 — Wire the peer into Godot multiplayer

`PlayFabPartyNetwork.get_local_peer()` returns a
`PlayFabPartyPeer` that **is** a `MultiplayerPeerExtension`. Assign
it to `multiplayer.multiplayer_peer` and Godot's `rpc` / `rpc_id`
infrastructure starts working immediately:

```gdscript
@rpc("any_peer", "reliable")
func handshake_message(text: String) -> void:
    var sender: int = multiplayer.get_remote_sender_id()
    print("[Party] RPC from peer %d: \"%s\"" % [sender, text])


func _bind_party_peer_to_multiplayer() -> void:
    if _network == null:
        return
    var peer: PlayFabPartyPeer = _network.local_peer
    multiplayer.multiplayer_peer = peer
    peer.text_message_received.connect(_on_party_text_received)
    peer.connection_state_changed.connect(_on_party_connection_state_changed)

    # The await in host_party()/_join_party_network() does not return until
    # the peer's status is already CONNECTION_CONNECTED, so the
    # connection_state_changed signal already fired before we connected
    # the handler. Send the handshake RPC synchronously here; keep the
    # signal handler for later state transitions (disconnects, errors).
    if peer.get_connection_status() == MultiplayerPeer.CONNECTION_CONNECTED:
        rpc("handshake_message", "ready")


func _on_party_connection_state_changed(status: int) -> void:
    # MultiplayerPeer.ConnectionStatus enum: 0 disconnected, 1 connecting, 2 connected.
    if status == MultiplayerPeer.CONNECTION_DISCONNECTED:
        print("[Party] Multiplayer peer disconnected")
```

Call `_bind_party_peer_to_multiplayer()` once on each side right
after `_network` is assigned — both the host's
`host_party()` branch and the client's `_join_party_network()`
branch. The host shows up as peer id `1`; clients receive positive
ids through the Party transport's handshake.

Reserved handshake and control packets are filtered out of the
public packet queue by the addon, so they never surface as Godot
`packet_received` traffic and do not collide with RPC channels.

## Step 6 — Gate chat on the user's privileges and permissions

Voice and text chat have **two** gating layers, both of which the
cert pass will hit:

1. The **local user's privileges** — `Communications` covers text
   and voice as a whole, `CommunicationVoiceIngame` specifically
   covers in-game voice. Parental controls and account
   restrictions live here. If the user lacks
   `CommunicationVoiceIngame`, you must not bring up a mic; if
   the user lacks `Communications`, you must not bring up the
   chat surface at all.
2. The **per-peer permission** — for each remote peer the local
   user is allowed to talk to, you separately check
   `communicate_using_voice` and `communicate_using_text` against
   that peer's XUID. Block lists, mute lists, privacy settings,
   and cross-network friend rules layer on top of the privilege.

Add this helper to your Party autoload. It is the same shape as
[T5 Step 2](05-multiplayer-lobby.md#step-2--gate-hostjoin-on-the-multiplayer-privilege)'s
`can_use_multiplayer`, but for the two chat privileges:

```gdscript
# XGameRuntime XUserPrivilege values from <XUser.h>.
const XUSER_PRIVILEGE_COMMUNICATIONS := 252
const XUSER_PRIVILEGE_COMMUNICATION_VOICE_INGAME := 205

func _has_privilege(privilege: int) -> bool:
    var user: GDKUser = Auth.xbox_user
    if user == null:
        return false
    var pf: GDKResult = await GDK.users.check_privilege_async(user, privilege)
    return pf.ok and bool(pf.data.get("has_privilege", false))

func resolve_chat_capabilities() -> Dictionary:
    var text_allowed: bool = await _has_privilege(XUSER_PRIVILEGE_COMMUNICATIONS)
    var voice_allowed: bool = text_allowed and await _has_privilege(
            XUSER_PRIVILEGE_COMMUNICATION_VOICE_INGAME)
    return { "text": text_allowed, "voice": voice_allowed }
```

Then wire the result into your `PlayFabPartyConfig` **before**
the create / join call from Step 2 / Step 4:

```gdscript
var caps: Dictionary = await resolve_chat_capabilities()

var config := PlayFabPartyConfig.new()
config.set_voice_chat_enabled(caps.voice)
config.set_text_chat_enabled(caps.text)
# … rest of the config (max players, transcription, etc.)
```

For the **per-peer** layer, call `check_permission_async` against
the peer's XUID before flipping the chat-control send / receive
flags. The addon's `set_peer_chat_permissions_async` takes a
bitmask the runtime translates into `Party::PartyChatPermissionOptions`:

```gdscript
const PARTY_CHAT_NONE := 0
const PARTY_CHAT_SEND_AUDIO := 1
const PARTY_CHAT_RECEIVE_AUDIO := 2
const PARTY_CHAT_SEND_TEXT := 4
const PARTY_CHAT_RECEIVE_TEXT := 8

func _on_chat_control_added(peer_id: int, _control) -> void:
    var peer_xuid: String = _xuid_for_peer(peer_id)
    if peer_xuid.is_empty():
        # Fall through: no XUID for this peer (non-Xbox sign-in, or the
        # lobby roster hasn't propagated yet). The privilege gate (Step 6)
        # already filtered out non-comms users on the local side, so
        # leaving the default permissions in place is safe for the demo.
        # A shipping title would defer the permission set and retry on
        # the lobby's MEMBER_UPDATED event for this entity.
        return
    var allow_voice: bool = await _check_permission("communicate_using_voice", peer_xuid)
    var allow_text: bool = await _check_permission("communicate_using_text", peer_xuid)

    var permissions := PARTY_CHAT_NONE
    if allow_voice:
        permissions |= PARTY_CHAT_SEND_AUDIO | PARTY_CHAT_RECEIVE_AUDIO
    if allow_text:
        permissions |= PARTY_CHAT_SEND_TEXT | PARTY_CHAT_RECEIVE_TEXT

    var pf: PlayFabResult = await _network.local_peer.set_peer_chat_permissions_async(
            peer_id, permissions)
    if not pf.ok:
        push_warning("[Party] chat permissions for peer %d failed: %s" % [peer_id, pf.message])

func _check_permission(permission: String, peer_xuid: String) -> bool:
    var pf: GDKResult = await GDK.privacy.check_permission_async(
            Auth.xbox_user, permission, peer_xuid)
    return pf.ok and bool(pf.data.get("allowed", false))

func _xuid_for_peer(peer_id: int) -> String:
    # Walk the lobby roster to map Party peer_id -> XUID. The Lobby
    # autoload writes the local user's XUID into member_properties["xuid"]
    # on host / join (T5 Step 3 / Step 4), so we match the peer's
    # PlayFab entity key against the roster and read the XUID off the
    # matching member. Returns "" when:
    #   - We're not currently in a lobby (no roster to walk).
    #   - The peer hasn't propagated into the lobby snapshot yet
    #     (chat_control_added can fire before the lobby's MEMBER_UPDATED
    #     for the same join — fail open and rely on the privilege gate
    #     for now; a shipping title would defer and retry on the
    #     MEMBER_UPDATED state-change).
    #   - The peer signed in via a non-Xbox path (no XUID was written).
    if _network == null or _network.local_peer == null or Lobby.current_lobby == null:
        return ""
    var key: Dictionary = _network.local_peer.get_peer_entity_key(peer_id)
    if key.is_empty():
        return ""
    var entity_id: String = String(key.get("id", ""))
    if entity_id.is_empty():
        return ""
    for member in Lobby.current_lobby.members:
        if String(member.entity_key.get("id", "")) == entity_id:
            return String(member.properties.get("xuid", ""))
    return ""
```

Notes:

- The per-peer check runs **on top of** the per-user privilege
  check. The local user can have `Communications` allowed (so
  the chat surface is up) but `communicate_using_voice` against
  a specific peer denied (so that peer's mic is off). Both gates
  must pass to enable that channel.
- Permission tokens are **snake_case**:
  `communicate_using_voice`, `communicate_using_text`,
  `communicate_using_video`. Same set as T5 / T6.
- `chat_control_added(peer_id, control)` is the right hook
  because remote chat controls land after the peer joins
  (covered in Step 7); checking permissions at network-create
  time would be too early — the peer isn't there yet.
- For chat permission **changes mid-session** (the local user
  blocks a remote peer through the Xbox shell), the addon fires
  `chat_permissions_changed(peer_id, permissions)`. Re-run the
  same `check_permission_async` flow against the affected peer
  in response and call `set_peer_chat_permissions_async` again.

## Step 7 — Voice and text chat

Voice is automatic once `enable_voice_chat` is on: every peer
on each side has a chat control attached when they join, microphones
default to "on", and the addon mixes incoming audio through the
platform default output. Per-peer mute is one call away:

```gdscript
func toggle_mute(peer_id: int, muted: bool) -> bool:
    if _state != State.IN_NETWORK:
        push_warning("[Party] toggle_mute rejected — not in a network (state=%d)" % _state)
        return false
    var peer: PlayFabPartyPeer = _network.local_peer
    var pf: PlayFabResult = await peer.set_peer_muted_async(peer_id, muted)
    if not pf.ok:
        push_warning("[Party] mute toggle failed: %s" % pf.message)
    return pf.ok
```

`set_peer_muted_async` updates the **local** chat control's view of
the remote peer — that is, "I will not hear peer X" rather than
"peer X cannot speak". Use it to power per-peer mute UI on the
local client.

Text chat is sent through the same peer object:

```gdscript
func send_chat(text: String) -> bool:
    if _state != State.IN_NETWORK:
        push_warning("[Party] send_chat rejected — not in a network (state=%d)" % _state)
        return false
    var peer: PlayFabPartyPeer = _network.local_peer
    var pf: PlayFabResult = await peer.send_text_async(text)
    if not pf.ok:
        push_warning("[Party] send_text failed: %s" % pf.message)
    return pf.ok


func _on_party_text_received(peer_id: int, message: PlayFabPartyChatMessage) -> void:
    print("[Party] Text from peer %d: \"%s\"" % [peer_id, message.text])
```

`send_chat` returning `bool` lets the UI suppress the local
"you> ..." echo on failure so the user doesn't see text that
was never broadcast.

`send_text_async` with no `target_peer_ids` broadcasts to every
remote chat control the addon has mapped. Pass a `PackedInt32Array`
to send to a subset; unknown ids cause the awaited `PlayFabResult`
to come back failed.

> **Chat controls arrive after the peer.** Remote chat controls are
> reported through `PlayFabPartyPeer.chat_control_added(peer_id,
> chat_control)`, which fires **after** the peer has joined the
> network. Until that signal fires for a given peer, broadcasting
> text via `send_text_async("…")` silently delivers to zero
> remote controls. If you have a "send" button in your UI, gate it
> on `chat_control_added` for at least one remote peer, or wait one
> dispatch tick after `NETWORK_CHANGE_PEER_JOINED` before exposing
> chat to the user.

If you also turn on `enable_transcription` in
`PlayFabPartyConfig`, the same chat path delivers
speech-to-text on `PlayFabPartyPeer.transcription_received` — same
`PlayFabPartyChatMessage` shape, with
`PlayFabPartyChatMessage.is_transcription` set to `true`.

## Step 8 — Tear down cleanly

When the local player wants to leave (back-button in the lobby UI,
scene exit, etc.):

```gdscript
func leave_party() -> bool:
    if _state != State.IN_NETWORK:
        push_warning("[Party] leave_party rejected — not in a network (state=%d)" % _state)
        return false

    _set_state(State.LEAVING)
    _teardown_in_progress = true

    # Clear the descriptor we published if we're the host leaving. Best
    # effort: clients who already joined ignore the empty value; clients
    # that hadn't joined yet won't chase a stale descriptor.
    if _is_host and _lobby != null and _lobby.is_owner(Auth.playfab_user):
        var clear: PlayFabResult = await _lobby.set_properties_async({PARTY_DESCRIPTOR_KEY: ""})
        if not clear.ok:
            push_warning("[Party] descriptor clear failed: %s" % clear.message)

    var pf: PlayFabResult = await _network.leave_async()
    if not pf.ok:
        push_warning("[Party] leave failed: %s" % pf.message)

    _network = null
    _is_host = false
    _clear_multiplayer_peer()
    _set_state(State.READY)
    network_left.emit()
    _teardown_in_progress = false
    return pf.ok


# NETWORK_CHANGE_DESTROYED arrives in several scenarios:
#   - mid-leave_party (voluntary; leave_party owns the emit, skip here)
#   - after leave_party already returned us to READY (voluntary residue, ignore)
#   - while still IN_NETWORK (involuntary; emit network_destroyed)
#   - during engine shutdown (autoload may be out of the tree; suppress)
func _handle_network_destroyed(reason: String) -> void:
    if _teardown_in_progress or _state == State.LEAVING or _state != State.IN_NETWORK:
        return
    if not is_inside_tree():
        return
    print("[Party] Network destroyed (%s)" % reason)
    _network = null
    _is_host = false
    _clear_multiplayer_peer()
    _set_state(State.READY)
    network_destroyed.emit()


# PlayFab.shutdown() during engine teardown emits
# NETWORK_CHANGE_DESTROYED from the bootstrap autoload's _exit_tree.
# At that point this autoload may already be detached from the
# SceneTree, so Node.multiplayer is null and a direct assignment
# would crash. Guard both the tree membership and the multiplayer
# reference.
func _clear_multiplayer_peer() -> void:
    if not is_inside_tree():
        return
    var api: MultiplayerAPI = multiplayer
    if api == null:
        return
    api.multiplayer_peer = null


# Triggered by the Lobby autoload when the local user leaves or is
# kicked. If we're mid-host/join, flag the in-flight op to abort on
# completion (Step 2 / Step 4). If we're already in the network,
# unwind cleanly.
func _on_lobby_left_from_lobby_autoload() -> void:
    _lobby = null
    if is_busy() and _state != State.LEAVING:
        _abort_party_op = true
        push_warning("[Party] Lobby left while busy (state=%d); in-flight op will abort" % _state)
        return
    if _state == State.IN_NETWORK:
        await leave_party()
```

When the **host** leaves, the network is destroyed (every client
receives `NETWORK_CHANGE_DESTROYED`). When a client leaves, only
that peer's `NETWORK_CHANGE_PEER_LEFT` reaches the host. Call
`PlayFab.party.shutdown_async()` from a top-level autoload's
`_exit_tree` if you want a guaranteed clean teardown on app exit;
the addon also tears Party down automatically when `PlayFab.shutdown()`
runs.

> **Why the `_teardown_in_progress` flag matters.**
> `NETWORK_CHANGE_DESTROYED` fires after `leave_async` completes
> too. Without the flag, `_handle_network_destroyed` would emit
> `network_destroyed` for what is actually a voluntary leave —
> a real double-emit bug consumers had to defend against.
> The flag plus the `_state != State.IN_NETWORK` check guarantee
> exactly one teardown signal per network lifecycle, correctly
> classified.

## Verify

Host run, client joins, host stops:

Host Output:

```
[Party] Party initialized (voice=true text=true transcription=false)
[Party] Network created — waiting for descriptor…
[Party] Descriptor ready, publishing on the lobby
[Party] State → 3 (connected)
[Party] Peer connected: id=2 entity=title_player_account:6F4B…
[Party] RPC from peer 2: "ready"
[Party] Text from peer 2: "gg"
[Party] Peer 2 left
[Party] Network destroyed (host leaving)
```

Client Output:

```
[Party] Party initialized (voice=true text=true transcription=false)
[Party] Joined Party network: 6acf3…
[Party] State → 3 (connected)
[Party] RPC from peer 1: "ready"
```

Common failures:

| Output | Diagnosis | Fix |
|---|---|---|
| `init failed: party_not_initialized` | `PartyManager` startup failed. | Check that the title has **Multiplayer → Party** enabled in PlayFab Game Manager and that `PlayFab.initialize()` resolved cleanly. |
| `create_and_join failed: party_invalid_user` | The signed-in `PlayFabUser` doesn't have an entity token, or the title id doesn't match. | Run Tutorial 1's sign-in to completion; `PlayFab.users.sign_in_*` must resolve to an Xbox-backed or Custom-ID `PlayFabUser` first. |
| `create_and_join failed: party_network_create_failed` or `party_network_connect_failed` | The Party service rejected the create or the local user failed to authenticate into the new network. | Re-check the title's Party configuration; verify the network and the local user are not already part of another network (`party_network_already_active`). |
| `join_network failed: party_descriptor_invalid` | The descriptor string was empty or malformed when the client called `join_network_async`. | Confirm the host's descriptor arrived through `change.lobby.properties` and is non-empty before joining. |
| Host prints `Network created` but never `Descriptor ready` | The descriptor finalization didn't reach `NETWORK_CHANGE_DESCRIPTOR_UPDATED` and was not populated synchronously after the await either. | Confirm `_network.state_changed` is connected in `host_party()` before yielding control; do not reassign `_network` between the await and the signal. |
| Client never sees `Joined Party network` | The client's `_on_lobby_state_changed` is filtering out `PROPERTIES_UPDATED`, or the host never published the key. | Print `change.lobby.properties` on every `PROPERTIES_UPDATED` to confirm the key arrived; verify the host got `Descriptor ready`. |
| RPC fires but `multiplayer.get_remote_sender_id() == 0` | `multiplayer.multiplayer_peer` was overwritten by a different transport (e.g. an autoload assigning `ENetMultiplayerPeer`). | Assign the Party peer **after** any other autoload has run, or remove the competing assignment. |
| `send_text_async` succeeds but no `text_message_received` ever fires | The remote peer's chat control had not been mapped yet at send time (zero broadcast targets). | Wait for `PlayFabPartyPeer.chat_control_added(peer_id, control)` for at least one remote peer before exposing the chat send UI. |
| Voice works one direction only | The peer was muted locally (Step 7) or `enable_voice_chat` was set differently on the two sides. | Both sides must initialize Party with `enable_voice_chat = true`; mute is per-direction. |

## Reference implementation

The cumulative end-state lives in
[`sample/tutorial_app/`](../../sample/tutorial_app/README.md):

- Scene: [`sample/tutorial_app/t07_party.tscn`](../../sample/tutorial_app/t07_party.tscn)
- Script: [`sample/tutorial_app/t07_party.gd`](../../sample/tutorial_app/t07_party.gd)
- Autoload introduced here: [`sample/tutorial_app/autoload/party.gd`](../../sample/tutorial_app/autoload/party.gd)
  (consumed by T8).

> **Path note.** The tutorial places `party.gd` at
> `res://party/party.gd` (one folder per topic). The sample
> collapses every autoload under `res://autoload/` because all
> three (`Auth`, `Lobby`, `Party`) are registered from the first
> tutorial. Same code, different folder.

## What's next

- [**Tutorial 8 — Integration tech demo**](08-integration-tech-demo.md)

From here you might also want to:

- **Map your game traffic onto RPC channels.** Godot RPC over the
  Party peer is unreliable-by-default unless you annotate
  `@rpc("any_peer", "reliable", "call_remote")`. Pick channels per
  message class (e.g. unreliable position updates + reliable
  spawn/despawn) so voice and gameplay don't share bandwidth.
- **Add matchmaking on top.** `PlayFab.multiplayer.create_match_ticket_async`
  returns a ticket whose `arranged_lobby_connection_string` plugs
  straight into `PlayFab.multiplayer.join_arranged_lobby_async`,
  which is exactly the Tutorial 5 lobby flow — your Party setup
  above keeps working unchanged.
- **Cross-platform voice.** Set
  `direct_peer_connectivity = DIRECT_PEER_CONNECTIVITY_ANY`
  on `PlayFabPartyConfig` (as in the snippets above) when you
  expect PC ↔ Xbox sessions. `DIRECT_PEER_CONNECTIVITY_ANY` is
  shorthand for "any platform type + any login provider", which is
  the SDK's required pairing. Same-platform-only is a small latency
  win when you ship single-platform builds — e.g.
  `DIRECT_PEER_CONNECTIVITY_SAME_PLATFORM_TYPE | DIRECT_PEER_CONNECTIVITY_ANY_ENTITY_LOGIN_PROVIDER`.
- Reference: [`PlayFabParty`](../../addons/godot_playfab/doc_classes/PlayFabParty.xml),
  [`PlayFabPartyNetwork`](../../addons/godot_playfab/doc_classes/PlayFabPartyNetwork.xml),
  [`PlayFabPartyPeer`](../../addons/godot_playfab/doc_classes/PlayFabPartyPeer.xml),
  [`PlayFabPartyConfig`](../../addons/godot_playfab/doc_classes/PlayFabPartyConfig.xml),
  [`PlayFabPartyNetworkStateChange`](../../addons/godot_playfab/doc_classes/PlayFabPartyNetworkStateChange.xml),
  [`PlayFabPartyChatMessage`](../../addons/godot_playfab/doc_classes/PlayFabPartyChatMessage.xml)
