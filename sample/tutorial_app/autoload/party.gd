extends Node

## Tutorial 7 — PlayFab Party network for voice + text chat + RPC.
##
## Stands up a peer-to-peer transport layered on top of the T5/T6 lobby.
## Subscribes to Lobby.lobby_joined to capture the live PlayFabLobby,
## creates/joins a Party network whose descriptor is published through
## lobby properties, and exposes voice + text chat + Godot RPC over the
## resulting PlayFabPartyPeer.
##
## Chat surfaces are gated on the local user's privileges (Communications
## + CommunicationVoiceIngame) and on per-peer permissions
## (communicate_using_voice / communicate_using_text). See T7 Step 6.
##
## Item 15 — state machine. The autoload exposes a tracked `State` plus a
## `state_changed` firehose; panels drive button enable/disable from the
## state instead of holding their own bookkeeping. host_party,
## _join_party_network, leave_party, send_chat, and toggle_mute return
## bool and reject re-entrant or out-of-state calls.
##
## Item 9 — single-slot design (intentional for the sample). This
## autoload owns exactly one PlayFabPartyNetwork at a time
## (`_network`) layered on top of the one PlayFabLobby owned by the
## Lobby autoload. host_party / _join_party_network reject
## re-entrant calls. A shipping game that needs concurrent Party
## networks (e.g. a persistent clan voice channel plus a per-match
## network) should refactor host_party / _join_party_network to
## return the new PlayFabPartyNetwork and have the caller hold the
## reference + connect directly. The addon supports multiple live
## Party networks; the single-slot choice lives in this sample
## autoload.
##
## Source: docs/tutorials/07-playfab-party.md

const PARTY_DESCRIPTOR_KEY := "party_descriptor"

# XGameRuntime XUserPrivilege values (from <XUser.h>).
const XUSER_PRIVILEGE_COMMUNICATIONS := 252
const XUSER_PRIVILEGE_COMMUNICATION_VOICE_INGAME := 205

# Party chat permission bitmask -> Party::PartyChatPermissionOptions.
const PARTY_CHAT_NONE := 0
const PARTY_CHAT_SEND_AUDIO := 1
const PARTY_CHAT_RECEIVE_AUDIO := 2
const PARTY_CHAT_SEND_TEXT := 4
const PARTY_CHAT_RECEIVE_TEXT := 8

enum State {
	UNINITIALIZED,    ## Autoload _ready has not finished sign-in + lobby wiring.
	READY,            ## Signed in, lobby wiring up, no Party network; host/join allowed.
	HOSTING,          ## host_party() in flight (create_and_join_network_async).
	JOINING,          ## _join_party_network() in flight (join_network_async).
	IN_NETWORK,       ## Active PlayFabPartyNetwork; chat/leave allowed.
	LEAVING,          ## leave_party() in flight.
}

signal state_changed(state: State)
signal network_joined(network: PlayFabPartyNetwork)
signal network_left           ## Voluntary teardown (leave_party).
signal network_destroyed      ## Involuntary teardown (NETWORK_CHANGE_DESTROYED / lobby drop / shutdown).
signal peer_connected(peer_id: int)
signal peer_disconnected(peer_id: int)
signal chat_received(peer_id: int, text: String)

var _state: State = State.UNINITIALIZED
var _auth: Node = null
var _lobby_node: Node = null
var _lobby: PlayFabLobby = null
var _network: PlayFabPartyNetwork = null
var _is_host: bool = false
var _gdk_lobby_signals_connected: bool = false
var _pf_party_signals_connected: bool = false

# Rubber-duck issue #1 + #2 — guard concurrent teardown so an in-flight
# host/join op that completes AFTER the lobby disappeared (or after the
# user voluntarily left) does not leave an orphan network bound to
# Godot's MultiplayerAPI.
var _abort_party_op: bool = false       ## Set true when state/ownership flipped mid-await.
var _teardown_in_progress: bool = false ## True while leave_party is unwinding voluntary teardown.

## Guarded accessor — returns null unless we are actually in a network.
## Callers that need the network across teardown should listen for
## network_left / network_destroyed and capture the payload there.
var network: PlayFabPartyNetwork:
	get:
		return _network if _state == State.IN_NETWORK else null

func _ready() -> void:
	_auth = get_node_or_null("/root/Auth")
	if _auth == null:
		push_error("[Party] Auth autoload missing")
		return

	_lobby_node = get_node_or_null("/root/Lobby")
	if _lobby_node == null:
		push_error("[Party] Lobby autoload missing")
		return

	# _ensure_ready awaits sign-in, wires the Lobby autoload signals once,
	# and transitions UNINITIALIZED -> READY. PlayFab Party SDK init stays
	# lazy and happens on first host_party / _join_party_network.
	await _ensure_ready()

func get_state() -> State:
	return _state

func is_ready() -> bool:
	return _state == State.READY

func is_in_network() -> bool:
	return _state == State.IN_NETWORK

func is_busy() -> bool:
	return _state == State.HOSTING or _state == State.JOINING or _state == State.LEAVING

# Kept for back-compat with sample / test code that called the getter
# directly. New consumers should use the `network` property.
func get_current_network() -> PlayFabPartyNetwork:
	return network

func _set_state(next: State) -> void:
	if _state == next:
		return
	_state = next
	state_changed.emit(_state)

# Idempotent — safe to call from _ready and lazily from host_party /
# _join_party_network. Auth.sign_in coalesces concurrent callers; lobby
# wiring is guarded against duplicate connections.
func _ensure_ready() -> bool:
	if _state == State.READY or _state == State.IN_NETWORK:
		return true
	if is_busy() or _state != State.UNINITIALIZED:
		return false

	if not await _auth.call("sign_in"):
		push_warning("[Party] sign-in failed (%s) — autoload will not initialize" %
				_auth.call("get_last_error_stage"))
		return false

	# Re-check after the await: a concurrent _ensure_ready caller may
	# have already advanced the state.
	if _state == State.READY or _state == State.IN_NETWORK:
		return true
	if _state != State.UNINITIALIZED:
		return false

	if not _gdk_lobby_signals_connected:
		_lobby_node.lobby_joined.connect(_on_lobby_joined_from_lobby_autoload)
		_lobby_node.lobby_left.connect(_on_lobby_left_from_lobby_autoload)
		# Item 15 — voluntary vs involuntary lobby departure tear down the
		# Party network the same way, so route both to the same handler.
		_lobby_node.lobby_disconnected.connect(_on_lobby_left_from_lobby_autoload)
		_gdk_lobby_signals_connected = true

	_set_state(State.READY)
	print("[Party] Lobby wiring connected. PlayFab Party init is lazy.")
	return true

# Item 10 — lazy initialization. PlayFab.party owns voice / text /
# transport state and is expensive (audio engine, network stack). Bring
# it up on first host/join instead of in _ready, and connect the
# party_error firehose exactly once on success.
func _ensure_pf_party_initialized() -> bool:
	if not Engine.has_singleton("PlayFab"):
		push_error("[Party] PlayFab extension not loaded")
		return false

	if not PlayFab.party.is_initialized():
		var cfg := PlayFabPartyConfig.new()
		cfg.max_players = 8
		cfg.direct_peer_connectivity = PlayFabParty.DIRECT_PEER_CONNECTIVITY_ANY
		cfg.enable_voice_chat = true
		cfg.enable_text_chat = true
		cfg.enable_transcription = false

		var init: PlayFabResult = await PlayFab.party.initialize_async(cfg)
		if not init.ok:
			push_warning("[Party] PlayFab.party init failed: %s (%s)" % [init.message, init.code])
			return false
		print("[Party] PlayFab.party initialized lazily (voice=true text=true transcription=false)")

	if not _pf_party_signals_connected:
		PlayFab.party.party_error.connect(_on_party_error)
		_pf_party_signals_connected = true

	return true

# Tutorial 7 Step 2 — host creates the Party network.
func host_party() -> bool:
	if not await _ensure_ready():
		return false
	if _state != State.READY:
		push_warning("[Party] host_party rejected — busy or already in network (state=%d)" % _state)
		return false

	_set_state(State.HOSTING)
	_abort_party_op = false
	_is_host = true

	if not await _ensure_pf_party_initialized():
		_is_host = false
		_set_state(State.READY)
		return false

	var caps: Dictionary = await resolve_chat_capabilities()

	var user: PlayFabUser = _auth.get("playfab_user")
	var cfg := PlayFabPartyConfig.new()
	cfg.max_players = 4
	cfg.direct_peer_connectivity = PlayFabParty.DIRECT_PEER_CONNECTIVITY_ANY
	cfg.set_voice_chat_enabled(caps.voice)
	cfg.set_text_chat_enabled(caps.text)

	var result: PlayFabResult = await PlayFab.party.create_and_join_network_async(user, cfg)

	# Rubber-duck issue #1 — the lobby may have disappeared while we were
	# awaiting create_and_join. Bail out without binding the multiplayer
	# peer or emitting network_joined; teardown the orphan network.
	if _abort_party_op or _state != State.HOSTING:
		if result.ok:
			print("[Party] Aborting orphaned host network (lobby left mid-await)")
			var orphan: PlayFabPartyNetwork = result.data
			orphan.leave_async()
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

	var net: PlayFabPartyNetwork = result.data
	_attach_network(net)
	_set_state(State.IN_NETWORK)
	print("[Party] Network created — waiting for descriptor…")
	network_joined.emit(_network)

	# If the finalized descriptor was populated synchronously, publish now.
	# Otherwise NETWORK_CHANGE_DESCRIPTOR_UPDATED publishes when it arrives.
	if not _network.descriptor.is_empty():
		await _publish_descriptor_on_lobby(_network.descriptor, net)
	return true

# Tutorial 7 Step 4 — client joins via descriptor pulled from lobby.
func _join_party_network(descriptor: String) -> bool:
	if not await _ensure_ready():
		return false
	if _state != State.READY:
		push_warning("[Party] join rejected — busy or already in network (state=%d)" % _state)
		return false

	_set_state(State.JOINING)
	_abort_party_op = false
	_is_host = false

	if not await _ensure_pf_party_initialized():
		_set_state(State.READY)
		return false

	var caps: Dictionary = await resolve_chat_capabilities()

	var user: PlayFabUser = _auth.get("playfab_user")
	var cfg := PlayFabPartyConfig.new()
	cfg.set_voice_chat_enabled(caps.voice)
	cfg.set_text_chat_enabled(caps.text)

	var result: PlayFabResult = await PlayFab.party.join_network_async(user, descriptor, cfg)

	# Same abort-after-await guard as host_party.
	if _abort_party_op or _state != State.JOINING:
		if result.ok:
			print("[Party] Aborting orphaned join network (lobby left mid-await)")
			var orphan: PlayFabPartyNetwork = result.data
			orphan.leave_async()
		_abort_party_op = false
		if _state == State.JOINING:
			_set_state(State.READY)
		return false

	if not result.ok:
		push_warning("[Party] join_network failed: %s (%s)" % [result.message, result.code])
		_set_state(State.READY)
		return false

	_attach_network(result.data)
	_set_state(State.IN_NETWORK)
	print("[Party] Joined Party network: %s" % _network.network_id)
	network_joined.emit(_network)
	return true

# Tutorial 7 Step 8 — leave the network voluntarily.
func leave_party() -> bool:
	if _state != State.IN_NETWORK:
		push_warning("[Party] leave_party rejected — not in a network (state=%d)" % _state)
		return false

	_set_state(State.LEAVING)
	_teardown_in_progress = true

	# Clear the descriptor we published if we're the host leaving the
	# network. Best-effort: a failure here is logged and ignored.
	if _is_host and _lobby != null:
		var pf_user: PlayFabUser = _auth.get("playfab_user")
		if pf_user != null and _lobby.is_owner(pf_user):
			var clear: PlayFabResult = await _lobby.set_properties_async({PARTY_DESCRIPTOR_KEY: ""})
			if not clear.ok:
				push_warning("[Party] descriptor clear failed: %s" % clear.message)

	var pf: PlayFabResult = await _network.leave_async()
	if not pf.ok:
		push_warning("[Party] leave failed: %s" % pf.message)

	_detach_network()
	_is_host = false
	_set_state(State.READY)
	network_left.emit()
	_teardown_in_progress = false
	return pf.ok

# Tutorial 7 Step 6 — local user's chat privileges.
func resolve_chat_capabilities() -> Dictionary:
	var text_allowed: bool = await _has_privilege(XUSER_PRIVILEGE_COMMUNICATIONS)
	var voice_allowed: bool = text_allowed and await _has_privilege(
			XUSER_PRIVILEGE_COMMUNICATION_VOICE_INGAME)
	return { "text": text_allowed, "voice": voice_allowed }

# Tutorial 7 Step 7 — per-peer mute. Returns false if not in a network
# or the underlying SDK call fails.
func toggle_mute(peer_id: int, muted: bool) -> bool:
	if _state != State.IN_NETWORK:
		push_warning("[Party] toggle_mute rejected — not in a network (state=%d)" % _state)
		return false
	var peer: PlayFabPartyPeer = _network.local_peer
	var pf: PlayFabResult = await peer.set_peer_muted_async(peer_id, muted)
	if not pf.ok:
		push_warning("[Party] mute toggle failed: %s" % pf.message)
	return pf.ok

# Tutorial 7 Step 7 — broadcast text chat. Returns false if not in a
# network or the underlying SDK call fails (callers should skip the
# local "you> ..." echo on false to avoid showing un-sent text).
func send_chat(text: String) -> bool:
	if _state != State.IN_NETWORK:
		push_warning("[Party] send_chat rejected — not in a network (state=%d)" % _state)
		return false
	var peer: PlayFabPartyPeer = _network.local_peer
	var pf: PlayFabResult = await peer.send_text_async(text)
	if not pf.ok:
		push_warning("[Party] send_text failed: %s" % pf.message)
	return pf.ok

# Tutorial 7 Step 5 — example RPC.
@rpc("any_peer", "reliable")
func handshake_message(text: String) -> void:
	var sender: int = multiplayer.get_remote_sender_id()
	print("[Party] RPC from peer %d: \"%s\"" % [sender, text])

# --- Internal helpers ---

func _has_privilege(privilege: int) -> bool:
	var user: GDKUser = _auth.get("xbox_user")
	if user == null:
		return false
	var pf: GDKResult = await GDK.users.check_privilege_async(user, privilege)
	return pf.ok and bool(pf.data.get("has_privilege", false))

func _check_permission(permission: String, peer_xuid: String) -> bool:
	var user: GDKUser = _auth.get("xbox_user")
	if user == null:
		return false
	var pf: GDKResult = await GDK.privacy.check_permission_async(
			user, permission, peer_xuid)
	return pf.ok and bool(pf.data.get("allowed", false))

# Publishes the network descriptor on the lobby. Rubber-duck issue #3 —
# re-check before writing so we don't publish a stale descriptor after
# the host already tore the network down (or the lobby flipped owners).
func _publish_descriptor_on_lobby(descriptor: String, expected_network: PlayFabPartyNetwork) -> void:
	if _state != State.IN_NETWORK:
		return
	if not _is_host:
		return
	if _network != expected_network:
		return
	if _lobby == null:
		push_warning("[Party] No lobby to publish descriptor on")
		return
	var pf_user: PlayFabUser = _auth.get("playfab_user")
	if pf_user == null or not _lobby.is_owner(pf_user):
		return
	print("[Party] Descriptor ready, publishing on the lobby")
	var pf: PlayFabResult = await _lobby.set_properties_async({
		PARTY_DESCRIPTOR_KEY: descriptor,
	})
	# Re-check after the await so we don't warn about a failure that
	# happened because we already tore down.
	if not pf.ok and _state == State.IN_NETWORK and _network == expected_network:
		push_warning("[Party] descriptor publish failed: %s" % pf.message)

# Rubber-duck issue #4 — centralize lobby signal lifetime so a stale
# PlayFabLobby ref can't keep delivering PROPERTIES_UPDATED events that
# trigger a join on the wrong lobby.
func _attach_lobby(lobby: PlayFabLobby) -> void:
	if _lobby == lobby:
		return
	_detach_lobby()
	_lobby = lobby
	if _lobby == null:
		return
	if not _lobby.state_changed.is_connected(_on_lobby_state_changed):
		_lobby.state_changed.connect(_on_lobby_state_changed)

func _detach_lobby() -> void:
	if _lobby != null and _lobby.state_changed.is_connected(_on_lobby_state_changed):
		_lobby.state_changed.disconnect(_on_lobby_state_changed)
	_lobby = null

# Rubber-duck issue #5 — centralize network signal lifetime + the
# multiplayer-peer binding so leave/destroy paths share one teardown.
func _attach_network(net: PlayFabPartyNetwork) -> void:
	_detach_network()
	_network = net
	if _network == null:
		return
	_network.state_changed.connect(_on_network_state_changed)
	var peer: PlayFabPartyPeer = _network.local_peer
	if peer == null:
		return
	multiplayer.multiplayer_peer = peer
	peer.text_message_received.connect(_on_party_text_received)
	peer.connection_state_changed.connect(_on_party_connection_state_changed)
	peer.chat_control_added.connect(_on_chat_control_added)

	if peer.get_connection_status() == MultiplayerPeer.CONNECTION_CONNECTED:
		rpc("handshake_message", "ready")

func _detach_network() -> void:
	if _network != null:
		if _network.state_changed.is_connected(_on_network_state_changed):
			_network.state_changed.disconnect(_on_network_state_changed)
		var peer: PlayFabPartyPeer = _network.local_peer
		if peer != null:
			if peer.text_message_received.is_connected(_on_party_text_received):
				peer.text_message_received.disconnect(_on_party_text_received)
			if peer.connection_state_changed.is_connected(_on_party_connection_state_changed):
				peer.connection_state_changed.disconnect(_on_party_connection_state_changed)
			if peer.chat_control_added.is_connected(_on_chat_control_added):
				peer.chat_control_added.disconnect(_on_chat_control_added)
	_clear_multiplayer_peer()
	_network = null

# --- Signal handlers ---

func _on_lobby_joined_from_lobby_autoload(lobby: PlayFabLobby) -> void:
	_attach_lobby(lobby)

	# Client side: descriptor may already be on the lobby snapshot (host
	# created the network before we joined). Honour it without waiting
	# for a properties-updated event. Guard on state so we don't
	# accidentally try to join while still HOSTING / JOINING / IN_NETWORK.
	if _is_host or _state != State.READY:
		return
	var descriptor: String = String(lobby.properties.get(PARTY_DESCRIPTOR_KEY, ""))
	if descriptor.is_empty():
		return
	await _join_party_network(descriptor)

func _on_lobby_left_from_lobby_autoload() -> void:
	_detach_lobby()
	# Rubber-duck issue #1 — flag in-flight host/join so they unwind on
	# completion instead of leaving an orphan network bound to the
	# multiplayer peer.
	if is_busy() and _state != State.LEAVING:
		_abort_party_op = true
		push_warning("[Party] Lobby left while busy (state=%d); in-flight op will abort on completion" % _state)
		return
	if _state == State.IN_NETWORK:
		await leave_party()

func _on_lobby_state_changed(change: PlayFabLobbyStateChange) -> void:
	if change.kind != PlayFabLobby.PROPERTIES_UPDATED:
		return
	# Only the not-yet-joined client side cares about descriptor updates.
	if _is_host or _state != State.READY:
		return

	var descriptor: String = String(change.lobby.properties.get(PARTY_DESCRIPTOR_KEY, ""))
	if descriptor.is_empty():
		return

	await _join_party_network(descriptor)

func _on_network_state_changed(change: PlayFabPartyNetworkStateChange) -> void:
	match change.kind:
		PlayFabParty.NETWORK_CHANGE_DESCRIPTOR_UPDATED:
			if _is_host and _state == State.IN_NETWORK and _network != null and not _network.descriptor.is_empty():
				await _publish_descriptor_on_lobby(_network.descriptor, _network)
		PlayFabParty.NETWORK_CHANGE_PEER_JOINED:
			var entity := ""
			if _network != null and _network.local_peer != null:
				entity = str(_network.local_peer.get_peer_entity_key(change.peer_id))
			print("[Party] Peer connected: id=%d entity=%s" % [change.peer_id, entity])
			peer_connected.emit(change.peer_id)
		PlayFabParty.NETWORK_CHANGE_PEER_LEFT:
			print("[Party] Peer %d left" % change.peer_id)
			peer_disconnected.emit(change.peer_id)
		PlayFabParty.NETWORK_CHANGE_STATE:
			print("[Party] State → %d (%s)" % [change.state, change.reason])
		PlayFabParty.NETWORK_CHANGE_ERROR:
			push_warning("[Party] network error: %s" % change.reason)
		PlayFabParty.NETWORK_CHANGE_DESTROYED:
			_handle_network_destroyed(change.reason)

# Rubber-duck issue #2 — centralized DESTROYED dispatch. The destroyed
# event may arrive:
#   - while leave_party is mid-flight (voluntary; leave_party will emit
#     network_left + transition to READY itself)
#   - while leave_party has already completed and we're in READY
#     (voluntary residue; ignore)
#   - while still IN_NETWORK (involuntary; emit network_destroyed)
#   - during engine shutdown after the autoload is out of the tree
#     (suppress; nothing to clean up safely)
func _handle_network_destroyed(reason: String) -> void:
	if _teardown_in_progress or _state == State.LEAVING:
		# leave_party owns the teardown + signal emission.
		return
	if _state != State.IN_NETWORK:
		# READY (or UNINITIALIZED during teardown) — nothing to detach.
		return
	if not is_inside_tree():
		# Engine shutdown beat us. PlayFab.shutdown clears the SDK; the
		# autoload removal will drop our refs. Don't poke MultiplayerAPI.
		return
	print("[Party] Network destroyed (%s)" % reason)
	_detach_network()
	_is_host = false
	_set_state(State.READY)
	network_destroyed.emit()

# PlayFab.shutdown() during engine teardown emits NETWORK_CHANGE_DESTROYED
# from playfab_bootstrap.gd::_exit_tree. By that point this autoload may
# already have been removed from the SceneTree (autoload teardown order is
# not guaranteed), so `multiplayer` (Node.multiplayer) returns a null
# instance and the assignment would crash. Guard both the SceneTree
# membership and the multiplayer reference.
func _clear_multiplayer_peer() -> void:
	if not is_inside_tree():
		return
	var api: MultiplayerAPI = multiplayer
	if api == null:
		return
	api.multiplayer_peer = null

func _on_party_text_received(peer_id: int, message: PlayFabPartyChatMessage) -> void:
	print("[Party] Text from peer %d: \"%s\"" % [peer_id, message.text])
	chat_received.emit(peer_id, message.text)

func _on_party_connection_state_changed(status: int) -> void:
	if status == MultiplayerPeer.CONNECTION_DISCONNECTED:
		print("[Party] Multiplayer peer disconnected")

func _on_chat_control_added(peer_id: int, _control) -> void:
	# Per-peer permission gate (Step 6). XUID resolution depends on the
	# lobby roster — fall back to skipping the per-peer check when we
	# cannot resolve the XUID (e.g. local peer).
	var peer_xuid: String = _xuid_for_peer(peer_id)
	if peer_xuid.is_empty():
		return
	var allow_voice: bool = await _check_permission("communicate_using_voice", peer_xuid)
	var allow_text: bool = await _check_permission("communicate_using_text", peer_xuid)

	var permissions := PARTY_CHAT_NONE
	if allow_voice:
		permissions |= PARTY_CHAT_SEND_AUDIO | PARTY_CHAT_RECEIVE_AUDIO
	if allow_text:
		permissions |= PARTY_CHAT_SEND_TEXT | PARTY_CHAT_RECEIVE_TEXT

	# Re-check after the await — the network may have torn down.
	if _state != State.IN_NETWORK or _network == null:
		return
	var pf: PlayFabResult = await _network.local_peer.set_peer_chat_permissions_async(
			peer_id, permissions)
	if not pf.ok:
		push_warning("[Party] chat permissions for peer %d failed: %s" % [peer_id, pf.message])

func _xuid_for_peer(peer_id: int) -> String:
	# Item 5 / B1 — map Party peer_id -> XUID via the lobby roster.
	#
	# Party gives us a PlayFab entity key (id + type) for the peer via
	# PlayFabPartyPeer.get_peer_entity_key. The lobby autoload writes the
	# local user's XUID into member_properties["xuid"] on host/join, so
	# we can match the entity key against the lobby roster and read the
	# XUID off the matching member.
	#
	# Returns "" in three cases the chat-control-added handler treats
	# the same way (skip the per-peer permission check):
	#   - We are not in a lobby right now (e.g. running Party without
	#     T5's lobby host_lobby/join_lobby path).
	#   - The peer hasn't propagated yet into the lobby snapshot
	#     (chat_control_added fires before the lobby's MEMBER_UPDATED
	#     for the same join).
	#   - The peer is on a non-Xbox sign-in path (custom-id sessions)
	#     so no XUID was written. The privilege gate (T5 Step 2 / T7
	#     Step 6) already filtered out non-comms users.
	if _network == null or _network.local_peer == null or _lobby == null:
		return ""
	var key: Dictionary = _network.local_peer.get_peer_entity_key(peer_id)
	if key.is_empty():
		return ""
	var entity_id: String = String(key.get("id", ""))
	if entity_id.is_empty():
		return ""
	for member in _lobby.members:
		var member_entity: Dictionary = member.entity_key
		if String(member_entity.get("id", "")) == entity_id:
			return String(member.properties.get("xuid", ""))
	return ""

func _on_party_error(result: PlayFabResult) -> void:
	push_warning("[Party] party error: %s (%s)" % [result.message, result.code])
