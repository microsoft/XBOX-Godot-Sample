extends VBoxContainer

## Tutorial 8 Step 5 — Lobby panel.
##
## Host / join (from a pasted connection string) / leave with member
## roster. Inherits privilege gating + presence writes from the Lobby
## autoload (T5 Step 2 + Step 8).
##
## Item 5 / B5 (signal hygiene): re-driven by Auth.state_changed so a
## sign-in retry after a transient failure still wires the panel up,
## and external connections (PlayFab.multiplayer.state_changed +
## Lobby.lobby_disconnected) are explicitly torn down in _exit_tree.
##
## Source: docs/tutorials/08-integration-tech-demo.md Step 5

@onready var _host: Button = $Host
@onready var _join: Button = $Join
@onready var _leave: Button = $Leave
@onready var _status: Label = $Status
@onready var _members: Label = $Members
@onready var _connection_string: LineEdit = $ConnectionString

var _auth: Node = null
var _lobby_node: Node = null
var _initialized: bool = false

func _ready() -> void:
	_auth = get_node_or_null("/root/Auth")
	_lobby_node = get_node_or_null("/root/Lobby")
	if _auth == null or _lobby_node == null:
		_status.text = "[ERR] Auth/Lobby autoload missing"
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
	if Engine.has_singleton("PlayFab"):
		if PlayFab.multiplayer.state_changed.is_connected(_refresh):
			PlayFab.multiplayer.state_changed.disconnect(_refresh)
	if _lobby_node != null and _lobby_node.lobby_disconnected.is_connected(_on_lobby_disconnected):
		_lobby_node.lobby_disconnected.disconnect(_on_lobby_disconnected)
	if _lobby_node != null and _lobby_node.state_changed.is_connected(_on_lobby_state_changed):
		_lobby_node.state_changed.disconnect(_on_lobby_state_changed)

func _on_auth_state_changed(_state) -> void:
	if _initialized or _auth == null:
		return
	if _auth.is_signed_in():
		_initialize_after_sign_in()

func _initialize_after_sign_in() -> void:
	if _initialized:
		return
	_initialized = true
	_host.pressed.connect(_on_host_pressed)
	_join.pressed.connect(_on_join_pressed)
	_leave.pressed.connect(_on_leave_pressed)
	PlayFab.multiplayer.state_changed.connect(_refresh)
	_lobby_node.lobby_disconnected.connect(_on_lobby_disconnected)
	# Drive in-progress feedback off the Lobby autoload so the status
	# label flips to "Hosting…" / "Joining…" / "Leaving…" the moment the
	# user clicks. Without this the panel looks frozen between the click
	# and the PlayFab.multiplayer.state_changed fire.
	_lobby_node.state_changed.connect(_on_lobby_state_changed)
	print("[Lobby] panel ready")
	_refresh()

func _on_host_pressed() -> void:
	await _lobby_node.host_lobby()
	if not is_inside_tree():
		return
	var current: PlayFabLobby = _lobby_node.call("get_current_lobby")
	if current != null:
		_connection_string.text = current.connection_string
	_refresh()

func _on_join_pressed() -> void:
	var text: String = _connection_string.text.strip_edges()
	if text.is_empty():
		_status.text = "Paste a connection string into the field first"
		return
	await _lobby_node.join_lobby_with_string(text)
	if not is_inside_tree():
		return
	_refresh()

func _on_leave_pressed() -> void:
	await _lobby_node.leave_lobby()
	if not is_inside_tree():
		return
	_refresh()

func _refresh(_change = null) -> void:
	var current: PlayFabLobby = _lobby_node.call("get_current_lobby")
	if current == null:
		_status.text = "Not in a lobby"
		_members.text = ""
		return
	_status.text = "Lobby %s (%d / %d)" % [
			current.lobby_id.left(8),
			current.member_count,
			current.max_member_count]
	var lines := PackedStringArray()
	for member: PlayFabLobbyMember in current.members:
		lines.append("- %s%s" % [member.user_id, " (you)" if member.is_local else ""])
	_members.text = "\n".join(lines)

func _on_lobby_disconnected() -> void:
	_status.text = "Disconnected from lobby (kicked or network error)"
	_members.text = ""
	_connection_string.text = ""

func _on_lobby_state_changed(state) -> void:
	# State enum lives on the Lobby autoload; resolve at runtime since the
	# parse gate doesn't see autoload types. Terminal states (IN_LOBBY /
	# READY) fall through to _refresh so the existing roster + "Not in a
	# lobby" path drives them.
	var s = _lobby_node.State
	match state:
		s.HOSTING:
			_status.text = "Hosting lobby…"
		s.JOINING:
			_status.text = "Joining lobby…"
		s.LEAVING:
			_status.text = "Leaving lobby…"
		_:
			_refresh()
