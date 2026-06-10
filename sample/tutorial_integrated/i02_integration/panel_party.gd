extends VBoxContainer

const AddonApi = preload("res://shared/addon_api.gd")

## Tutorial 8 Step 7 — PlayFab Party panel.
##
## Peer roster + broadcast text chat + per-peer mute toggle on top of
## the Party autoload's active network. Voice / text gating is inherited
## from the autoload (T7 Step 6) — the panel surfaces the post-gate UI.
##
## Item 5 / B5 (signal hygiene): re-driven by Auth.state_changed so a
## sign-in retry after a transient failure still wires the panel up.
## _exit_tree explicitly disconnects the Auth listener + the Party
## autoload network-lifecycle signals; per-network signals are already
## owned by _attach_network and detach when the network changes.
##
## Source: docs/tutorials/08-integration-tech-demo.md Step 7

@onready var _peer_list: Label = $PeerList
@onready var _chat_log: RichTextLabel = $ChatLog
@onready var _chat_input: LineEdit = $ChatInput
@onready var _send: Button = $Send
@onready var _mute_remotes: CheckButton = $MuteRemotes

var _auth: Node = null
var _party_node: Node = null
var _network = null
var _peer = null
var _initialized: bool = false

func _ready() -> void:
	_auth = get_node_or_null("/root/Auth")
	_party_node = get_node_or_null("/root/Party")
	if _auth == null or _party_node == null:
		_peer_list.text = "[ERR] Auth/Party autoload missing"
		return
	_auth.state_changed.connect(_on_auth_state_changed)
	if _auth.is_signed_in():
		_initialize_after_sign_in()
		return
	await _auth.sign_in()
	if is_inside_tree() and _auth.is_signed_in():
		_initialize_after_sign_in()

func _exit_tree() -> void:
	if _auth != null and _auth.state_changed.is_connected(_on_auth_state_changed):
		_auth.state_changed.disconnect(_on_auth_state_changed)
	if not _initialized:
		return
	if _party_node != null:
		if _party_node.network_joined.is_connected(_attach_network):
			_party_node.network_joined.disconnect(_attach_network)
		if _party_node.network_left.is_connected(_on_network_left):
			_party_node.network_left.disconnect(_on_network_left)
		if _party_node.network_destroyed.is_connected(_on_network_destroyed):
			_party_node.network_destroyed.disconnect(_on_network_destroyed)
		if _party_node.state_changed.is_connected(_on_party_state_changed):
			_party_node.state_changed.disconnect(_on_party_state_changed)
	# Detach the active network so its state_changed / peer signals
	# are dropped explicitly rather than left to instance-free cleanup.
	_attach_network(null)

func _on_auth_state_changed(_state) -> void:
	if _initialized or _auth == null:
		return
	if _auth.is_signed_in():
		_initialize_after_sign_in()

func _initialize_after_sign_in() -> void:
	if _initialized:
		return
	_initialized = true
	# PlayFab.party init happens lazily inside the Party autoload when
	# host_party() / _join_party_network() is invoked. The panel
	# observes via Party.network_joined / network_left and does not
	# need to bring the SDK up itself.
	_send.pressed.connect(_on_send_pressed)
	_mute_remotes.toggled.connect(_on_mute_remotes_toggled)
	_party_node.network_joined.connect(_attach_network)
	_party_node.network_left.connect(_on_network_left)
	_party_node.network_destroyed.connect(_on_network_destroyed)
	# Drive in-progress feedback off the Party autoload so the peer list
	# flips to "Bringing up…" / "Joining…" the moment a host/join starts
	# (typically cascaded from the Lobby panel). _attach_network owns the
	# terminal IN_NETWORK / READY states.
	_party_node.state_changed.connect(_on_party_state_changed)
	_attach_network(_party_node.get("network"))
	var label: String = "connected" if _network != null else "idle — no network"
	print("[Pty] party panel ready (%s)" % label)

func _attach_network(network) -> void:
	if network == _network:
		return
	if _network != null and _network.state_changed.is_connected(_on_network_state_changed):
		_network.state_changed.disconnect(_on_network_state_changed)
	if _peer != null:
		if _peer.text_message_received.is_connected(_on_text_received):
			_peer.text_message_received.disconnect(_on_text_received)
		if _peer.chat_control_added.is_connected(_on_chat_control_added):
			_peer.chat_control_added.disconnect(_on_chat_control_added)
		if _peer.chat_control_removed.is_connected(_on_chat_control_removed):
			_peer.chat_control_removed.disconnect(_on_chat_control_removed)
	_network = network
	if _network == null:
		_peer = null
		_refresh_peers()
		return
	_peer = _network.local_peer
	_network.state_changed.connect(_on_network_state_changed)
	if _peer != null:
		_peer.text_message_received.connect(_on_text_received)
		_peer.chat_control_added.connect(_on_chat_control_added)
		_peer.chat_control_removed.connect(_on_chat_control_removed)
	_refresh_peers()

func _on_network_left() -> void:
	_attach_network(null)

func _on_network_destroyed() -> void:
	_attach_network(null)
	_chat_log.append_text("[i]Party network destroyed (lobby host left, network error, or shutdown)[/i]\n")

func _on_party_state_changed(state) -> void:
	# State enum lives on the Party autoload; resolve at runtime since the
	# parse gate doesn't see autoload types. IN_NETWORK / READY transitions
	# are owned by _attach_network (via network_joined / network_left) so
	# we only need to surface the busy phases here.
	var s = _party_node.State
	match state:
		s.HOSTING:
			_peer_list.text = "Bringing up Party network…"
		s.JOINING:
			_peer_list.text = "Joining Party network…"
		s.LEAVING:
			_peer_list.text = "Leaving Party network…"

func _on_send_pressed() -> void:
	var text: String = _chat_input.text.strip_edges()
	if text.is_empty() or _peer == null:
		return
	var result = await _peer.send_text_async(text)
	if not is_inside_tree():
		return
	if result.ok:
		_chat_log.append_text("[me] %s\n" % text)
		_chat_input.text = ""
	else:
		_chat_log.append_text("[i]send_text_async failed: %s[/i]\n" % result.message)

func _on_mute_remotes_toggled(button_pressed: bool) -> void:
	if _peer == null:
		return
	for peer_id in _peer.get_peers():
		_peer.set_peer_muted_async(peer_id, button_pressed)

func _on_network_state_changed(_change) -> void:
	_refresh_peers()

func _on_chat_control_added(_peer_id: int, _control) -> void:
	_refresh_peers()

func _on_chat_control_removed(_peer_id: int) -> void:
	_refresh_peers()

func _on_text_received(peer_id: int, message) -> void:
	var label: String = "?"
	if _peer != null:
		var entity: Dictionary = _peer.get_peer_entity_key(peer_id)
		label = String(entity.get("id", "?")).left(8)
	_chat_log.append_text("[%s] %s\n" % [label, message.text])

func _refresh_peers() -> void:
	if _peer == null:
		_peer_list.text = "Not connected"
		return
	var lines := PackedStringArray()
	for peer_id in _peer.get_peers():
		var entity: Dictionary = _peer.get_peer_entity_key(peer_id)
		var id_label: String = String(entity.get("id", "?")).left(8)
		lines.append("- %s (peer %d)" % [id_label, peer_id])
	if lines.is_empty():
		lines.append("- (waiting for remote peers)")
	_peer_list.text = "\n".join(lines)
