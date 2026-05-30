extends Control

const AddonApi = preload("res://shared/addon_api.gd")

## Tutorial 7 — PlayFab Party demo scene.
##
## Demonstrates the Party autoload's host/join/leave flow on top of the
## T5 Lobby autoload. The user hosts (or joins) a lobby first; once the
## Party autoload observes the lobby it creates/joins the network and
## exposes voice + text chat + RPC.
##
## Uses get_node_or_null indirection for the GDScript autoloads (Auth,
## Lobby, Party) so the parse gate (which does not load autoloads)
## still resolves cleanly.
##
## Source: docs/tutorials/07-playfab-party.md

@onready var _status_label: Label = $Root/Status
@onready var _network_label: Label = $Root/NetworkLabel
@onready var _host_button: Button = $Root/Buttons/Host
@onready var _join_lobby_id: LineEdit = $Root/JoinRow/LobbyId
@onready var _join_button: Button = $Root/JoinRow/Join
@onready var _leave_button: Button = $Root/Buttons/Leave
@onready var _chat_input: LineEdit = $Root/ChatRow/Message
@onready var _send_button: Button = $Root/ChatRow/Send
@onready var _ping_button: Button = $Root/ChatRow/Ping
@onready var _chat_log: TextEdit = $Root/ChatLog
@onready var _back_button: Button = $Root/Back

var _auth: Node = null
var _lobby_node: Node = null
var _party_node: Node = null

func _ready() -> void:
	_auth = get_node_or_null("/root/Auth")
	_lobby_node = get_node_or_null("/root/Lobby")
	_party_node = get_node_or_null("/root/Party")

	_host_button.pressed.connect(_on_host_pressed)
	_join_button.pressed.connect(_on_join_pressed)
	_leave_button.pressed.connect(_on_leave_pressed)
	_send_button.pressed.connect(_on_send_pressed)
	_ping_button.pressed.connect(_on_ping_pressed)
	_back_button.pressed.connect(_on_back_pressed)

	_set_buttons_for_state(false)
	_status_label.text = "Sign-in pending."
	_network_label.text = "Network: (none)"
	_send_button.disabled = true
	_ping_button.disabled = true

	if _auth == null or _lobby_node == null or _party_node == null:
		_status_label.text = "[ERR] Auth/Lobby/Party autoload missing"
		return

	if not await _auth.call("sign_in"):
		_status_label.text = "Sign-in failed (%s): %s" % [
				_auth.call("get_last_error_stage"),
				_auth.call("get_last_error_message")]
		return

	_status_label.text = "Signed in. Host or join a lobby to bring up the Party network."

	_lobby_node.lobby_joined.connect(_on_lobby_joined)
	_lobby_node.lobby_left.connect(_on_lobby_left)
	_lobby_node.lobby_disconnected.connect(_on_lobby_disconnected)
	# Drive in-progress feedback off both autoloads' state machines so the
	# status line stays accurate during the lobby host/join → Party
	# network bring-up cascade. Without this the static "Hosting…" string
	# set on press would never update if the autoload bailed silently.
	_lobby_node.state_changed.connect(_on_lobby_state_changed)
	_party_node.network_joined.connect(_on_network_joined)
	_party_node.network_left.connect(_on_network_left)
	_party_node.network_destroyed.connect(_on_network_destroyed)
	_party_node.peer_connected.connect(_on_peer_connected)
	_party_node.peer_disconnected.connect(_on_peer_disconnected)
	_party_node.chat_received.connect(_on_chat_received)
	_party_node.rpc_received.connect(_on_rpc_received)
	_party_node.state_changed.connect(_on_party_state_changed)

	_set_buttons_for_state(true)

func _set_buttons_for_state(signed_in: bool) -> void:
	_host_button.disabled = not signed_in
	_join_button.disabled = not signed_in
	_leave_button.disabled = true

func _append_log(line: String) -> void:
	_chat_log.text += line + "\n"

func _on_host_pressed() -> void:
	_status_label.text = "Hosting lobby + Party network..."
	_host_button.disabled = true
	_join_button.disabled = true
	await _lobby_node.host_lobby()
	# Party.host_party fires from _on_lobby_joined once we are owner.

func _on_join_pressed() -> void:
	var connection: String = _join_lobby_id.text.strip_edges()
	if connection.is_empty():
		_status_label.text = "Paste a lobby connection string before Join."
		return
	_status_label.text = "Joining lobby + Party network..."
	_host_button.disabled = true
	_join_button.disabled = true
	await _lobby_node.join_lobby(connection)
	# Party autoload sees lobby_joined and joins the network from the
	# descriptor already published on the lobby.

func _on_leave_pressed() -> void:
	_status_label.text = "Leaving Party and lobby..."
	if _party_node.call("get_current_network") != null:
		await _party_node.leave_party()
	await _lobby_node.leave_lobby()
	_status_label.text = "Left. Ready to host or join again."
	_host_button.disabled = false
	_join_button.disabled = false
	_leave_button.disabled = true
	_send_button.disabled = true
	_ping_button.disabled = true
	_network_label.text = "Network: (none)"

func _on_send_pressed() -> void:
	var text: String = _chat_input.text
	if text.is_empty():
		return
	_chat_input.clear()
	if await _party_node.send_chat(text):
		_append_log("you> " + text)
	else:
		_append_log("[send failed]")

func _on_ping_pressed() -> void:
	# Broadcast an RPC to every connected peer to exercise the Godot
	# MultiplayerAPI path (peer.send_text_async goes through
	# PartyLocalChatControl, not through the multiplayer peer, so chat
	# alone doesn't prove RPC delivery).
	var text: String = "ping @%s" % str(Time.get_ticks_msec())
	if _party_node.send_rpc_ping(text):
		_append_log("you (rpc)> " + text)
	else:
		_append_log("[ping failed — not in a network]")

func _on_lobby_joined(lobby) -> void:
	_status_label.text = "Lobby ready: %s" % lobby.lobby_id
	# Surface the connection string in the same LineEdit the client
	# pastes into so the host can select-and-copy it (Ctrl+C) and hand
	# it to a second device. select_all() puts the field in a state
	# where the first Ctrl+C copies the full string without an extra
	# click — same pattern as T5 / T8 panel_lobby.
	_join_lobby_id.text = lobby.connection_string
	_join_lobby_id.caret_column = lobby.connection_string.length()
	_join_lobby_id.select_all()
	_join_lobby_id.grab_focus()
	_leave_button.disabled = false
	# Host: trigger Party network create now that the lobby owns us.
	var user = _auth.get("playfab_user")
	if user != null and lobby.is_owner(user):
		await _party_node.host_party()

func _on_lobby_left() -> void:
	_status_label.text = "Lobby ended."
	_host_button.disabled = false
	_join_button.disabled = false
	_leave_button.disabled = true
	_send_button.disabled = true
	_ping_button.disabled = true
	_network_label.text = "Network: (none)"

func _on_lobby_disconnected() -> void:
	_status_label.text = "Disconnected from lobby (kicked or network error)."
	_host_button.disabled = false
	_join_button.disabled = false
	_leave_button.disabled = true
	_send_button.disabled = true
	_ping_button.disabled = true
	_network_label.text = "Network: (none)"

func _on_network_joined(network) -> void:
	_network_label.text = "Network: %s" % network.network_id
	_send_button.disabled = false
	_ping_button.disabled = false
	_status_label.text = "Party network up. Voice/text chat active."

func _on_network_left() -> void:
	_network_label.text = "Network: (none)"
	_send_button.disabled = true
	_ping_button.disabled = true

func _on_network_destroyed() -> void:
	_network_label.text = "Network: (lost)"
	_send_button.disabled = true
	_ping_button.disabled = true
	_status_label.text = "Party network destroyed (lobby host left, network error, or shutdown)."

func _on_lobby_state_changed(state) -> void:
	# State enum lives on the Lobby autoload; resolve at runtime since the
	# parse gate doesn't see autoload types. Only the busy transitions
	# drive a message — IN_LOBBY / READY are owned by lobby_joined /
	# lobby_left handlers above.
	var s = _lobby_node.State
	match state:
		s.HOSTING:
			_status_label.text = "Hosting lobby…"
		s.JOINING:
			_status_label.text = "Joining lobby…"
		s.LEAVING:
			_status_label.text = "Leaving lobby…"

func _on_party_state_changed(state) -> void:
	# Party's network bring-up runs immediately after the lobby joins,
	# so surface its progress separately. network_joined /
	# network_destroyed handlers above own the terminal states.
	var s = _party_node.State
	match state:
		s.HOSTING:
			_status_label.text = "Bringing up Party network…"
		s.JOINING:
			_status_label.text = "Joining Party network…"
		s.LEAVING:
			_status_label.text = "Tearing down Party network…"

func _on_peer_connected(peer_id: int) -> void:
	_append_log("[peer connected] id=%d" % peer_id)

func _on_peer_disconnected(peer_id: int) -> void:
	_append_log("[peer left] id=%d" % peer_id)

func _on_chat_received(peer_id: int, text: String) -> void:
	_append_log("peer %d> %s" % [peer_id, text])

func _on_rpc_received(peer_id: int, text: String) -> void:
	_append_log("peer %d (rpc)> %s" % [peer_id, text])

func _on_back_pressed() -> void:
	get_tree().change_scene_to_file("res://shared/tutorial_picker.tscn")
