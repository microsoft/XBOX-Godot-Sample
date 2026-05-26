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
- Your PlayFab title has **Party** enabled. In the PlayFab Game
  Manager open **Multiplayer → Party** and confirm the title shows
  "Party enabled". Most titles created in the last few years have
  it on by default.
- Two Godot processes (host + client), each signed into a different
  Xbox test account in the same sandbox. As with the lobby tutorial,
  the easiest setup is one editor scene as host and an exported
  build as client; two editors with different PlayFab sessions also
  work.
- A working microphone on both sides if you want to test the voice
  path. Text and RPC traffic work without a mic.
- One-page primer on the addons' async model: [Async patterns](../async-patterns.md).

## Relevant addon surfaces

- [`PlayFab.party`](../playfab/plugin.md) —
  `initialize_async`, `create_and_join_network_async`,
  `join_network_async`, signal `party_error`.
- [`PlayFabPartyConfig`](../playfab/plugin.md) — Party network
  configuration (voice, text, max players, direct peer
  connectivity). `set_voice_chat_enabled` and
  `set_text_chat_enabled` decide which chat surfaces are wired
  for the network — Step 6 reads the local user's privileges
  before flipping them on.
- [`PlayFabPartyNetwork`](../playfab/plugin.md) — the network
  handle returned by create / join; carries the peer list,
  descriptor, `local_peer`, and the `state_changed(change)`
  signal that carries network lifecycle changes.
- [`PlayFabPartyPeer`](../playfab/plugin.md) — the per-peer chat
  surface: `send_text_async`, `set_peer_muted_async`,
  `set_peer_chat_permissions_async`,
  `text_message_received(peer_id, message)`,
  `chat_control_added`, `chat_permissions_changed`.
- [`PlayFab.multiplayer`](../playfab/plugin.md) — used here only to
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

## Step 1 — Initialize PlayFab Party

PlayFab Party sits on top of `PartyManager` and must be initialized
once before any network method works. Do it after PlayFab itself is
ready (so it shares the runtime queue with the rest of the addon)
but it does **not** need to wait for `PlayFab.multiplayer` — Party
and Multiplayer are independent services:

```gdscript
extends Node

const PARTY_DESCRIPTOR_KEY := "party_descriptor"

var _network: PlayFabPartyNetwork = null
var _is_host := false


func _ready() -> void:
    if Auth.playfab_user == null:
        await Auth.sign_in_completed

    if not PlayFab.party.is_initialized():
        var cfg := PlayFabPartyConfig.new()
        cfg.max_players = 8
        cfg.direct_peer_connectivity = PlayFabParty.DIRECT_PEER_CONNECTIVITY_ANY_PLATFORM_TYPE
        cfg.enable_voice_chat = true
        cfg.enable_text_chat = true
        cfg.enable_transcription = false  # Flip to true to receive
                                          # speech-to-text on transcription_received.

        var init: PlayFabResult = await PlayFab.party.initialize_async(cfg)
        if not init.ok:
            push_error("[Party] init failed: %s (%s)" % [init.message, init.code])
            return

    print("[Party] Party initialized (voice=true text=true transcription=false)")

    PlayFab.party.party_error.connect(_on_party_error)


func _on_party_error(result: PlayFabResult) -> void:
    push_warning("[Party] party error: %s (%s)" % [result.message, result.code])
```

`PlayFabPartyConfig` is reused for both initialization and the per-
network create / join calls. The `enable_voice_chat`,
`enable_text_chat`, and `enable_transcription` flags are **addon
policy** — they decide whether the addon creates and connects a
chat control alongside the network endpoint. You can flip them per
network, not just at init time.

## Step 2 — Host: create the Party network

The lobby owner creates the Party network. The host's
`PlayFabPartyPeer` is **always** Godot peer id `1`, so the existing
`multiplayer.is_server()` and `multiplayer.get_unique_id() == 1`
checks in your gameplay code still mean what you expect:

```gdscript
func host_party() -> void:
    _is_host = true

    var user: PlayFabUser = Auth.playfab_user
    var cfg := PlayFabPartyConfig.new()
    cfg.max_players = 4
    cfg.direct_peer_connectivity = PlayFabParty.DIRECT_PEER_CONNECTIVITY_ANY_PLATFORM_TYPE

    var result: PlayFabResult = await PlayFab.party.create_and_join_network_async(user, cfg)
    if not result.ok:
        push_warning("[Party] create_and_join failed: %s (%s)" % [result.message, result.code])
        return

    _network = result.data
    _network.state_changed.connect(_on_network_state_changed)
    print("[Party] Network created — waiting for descriptor…")

    # The provisional descriptor is never exposed. The finalized base64
    # descriptor is published through state_changed with
    # NETWORK_CHANGE_DESCRIPTOR_UPDATED; we publish onto the lobby from
    # _on_network_state_changed once that fires (Step 3).
    if not _network.descriptor.is_empty():
        _publish_descriptor_on_lobby(_network.descriptor)


func _on_network_state_changed(change: PlayFabPartyNetworkStateChange) -> void:
    match change.kind:
        PlayFabParty.NETWORK_CHANGE_DESCRIPTOR_UPDATED:
            if _is_host and not _network.descriptor.is_empty():
                _publish_descriptor_on_lobby(_network.descriptor)
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
            print("[Party] Network destroyed (%s)" % change.reason)
            _network = null
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

## Step 3 — Host: publish the descriptor on the lobby

The simplest discovery flow is: write the descriptor into the
lobby's lobby-properties dictionary. Every lobby member already gets
a `PROPERTIES_UPDATED` event when properties change, so each client
sees the descriptor as soon as it's available:

```gdscript
var _lobby: PlayFabLobby = null  # Set this from Tutorial 5's create_lobby_async.


func _publish_descriptor_on_lobby(descriptor: String) -> void:
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
    if not pf.ok:
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
    if _network != null or _is_host:
        return  # We're either already joined or we're the host.

    var descriptor: String = String(change.lobby.properties.get(PARTY_DESCRIPTOR_KEY, ""))
    if descriptor.is_empty():
        return  # Owner is still creating the network.

    await _join_party_network(descriptor)


func _join_party_network(descriptor: String) -> void:
    var user: PlayFabUser = Auth.playfab_user
    var cfg := PlayFabPartyConfig.new()
    cfg.enable_voice_chat = true
    cfg.enable_text_chat = true

    var result: PlayFabResult = await PlayFab.party.join_network_async(user, descriptor, cfg)
    if not result.ok:
        push_warning("[Party] join_network failed: %s (%s)" % [result.message, result.code])
        return

    _network = result.data
    _network.state_changed.connect(_on_network_state_changed)
    print("[Party] Joined Party network: %s" % _network.network_id)
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
    var peer_xuid: String = _xuid_for_peer(peer_id) # from your lobby roster
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
func toggle_mute(peer_id: int, muted: bool) -> void:
    var peer: PlayFabPartyPeer = _network.local_peer
    var pf: PlayFabResult = await peer.set_peer_muted_async(peer_id, muted)
    if not pf.ok:
        push_warning("[Party] mute toggle failed: %s" % pf.message)
```

`set_peer_muted_async` updates the **local** chat control's view of
the remote peer — that is, "I will not hear peer X" rather than
"peer X cannot speak". Use it to power per-peer mute UI on the
local client.

Text chat is sent through the same peer object:

```gdscript
func send_chat(text: String) -> void:
    var peer: PlayFabPartyPeer = _network.local_peer
    var pf: PlayFabResult = await peer.send_text_async(text)
    if not pf.ok:
        push_warning("[Party] send_text failed: %s" % pf.message)


func _on_party_text_received(peer_id: int, message: PlayFabPartyChatMessage) -> void:
    print("[Party] Text from peer %d: \"%s\"" % [peer_id, message.text])
```

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
func leave_party() -> void:
    if _network == null:
        return
    var pf: PlayFabResult = await _network.leave_async()
    if not pf.ok:
        push_warning("[Party] leave failed: %s" % pf.message)
    _network = null
    multiplayer.multiplayer_peer = null
```

When the **host** leaves, the network is destroyed (every client
receives `NETWORK_CHANGE_DESTROYED`). When a client leaves, only
that peer's `NETWORK_CHANGE_PEER_LEFT` reaches the host. Call
`PlayFab.party.shutdown_async()` from a top-level autoload's
`_exit_tree` if you want a guaranteed clean teardown on app exit;
the addon also tears Party down automatically when `PlayFab.shutdown()`
runs.

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

## What's next

You now have an end-to-end multiplayer stack: Xbox + PlayFab
identity (Tutorial 1), a lobby for roster + invites (Tutorial 5),
MPA for shell-level discovery (Tutorial 6), and PlayFab Party for
the real transport (this tutorial). The [capstone](08-integration-tech-demo.md)
is next — a single Control scene that wires every surface from
T1–T7 into one panel-per-surface dashboard so you can see them
all running against one signed-in identity:

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
  `direct_peer_connectivity = DIRECT_PEER_CONNECTIVITY_ANY_PLATFORM_TYPE`
  on `PlayFabPartyConfig` (as in the snippets above) when you
  expect PC ↔ Xbox sessions. Same-platform-only is a small latency
  win when you ship single-platform builds.
- Reference: [`PlayFabParty`](../playfab/plugin.md),
  [`PlayFabPartyNetwork`](../playfab/plugin.md),
  [`PlayFabPartyPeer`](../playfab/plugin.md),
  [`PlayFabPartyConfig`](../playfab/plugin.md),
  [`PlayFabPartyNetworkStateChange`](../playfab/plugin.md),
  [`PlayFabPartyChatMessage`](../playfab/plugin.md)
