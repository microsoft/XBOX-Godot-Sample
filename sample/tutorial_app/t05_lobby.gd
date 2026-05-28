extends Control

## Tutorial 5 reference scene — host / join / leave a PlayFab lobby.
##
## Demonstrates the Lobby autoload's host/join/leave flow with manual
## connection-string handoff. Members are shown live as MEMBER_ADDED /
## MEMBER_REMOVED events fire through autoload/lobby.gd.
##
## Source: docs/tutorials/05-multiplayer-lobby.md

@onready var _log: RichTextLabel = $Root/LogPanel/Log
@onready var _conn_edit: LineEdit = $Root/JoinRow/ConnEdit
@onready var _host_btn: Button = $Root/Buttons/HostBtn
@onready var _join_btn: Button = $Root/Buttons/JoinBtn
@onready var _leave_btn: Button = $Root/Buttons/LeaveBtn
@onready var _loadout_btn: Button = $Root/Buttons/LoadoutBtn
@onready var _map_btn: Button = $Root/Buttons/MapBtn
@onready var _back_btn: Button = $Root/Buttons/BackBtn
@onready var _members: ItemList = $Root/MembersPanel/Members

var _lobby_node: Node = null

func _ready() -> void:
	_back_btn.pressed.connect(_on_back_pressed)
	_host_btn.pressed.connect(func(): await _lobby_node.host_lobby())
	_join_btn.pressed.connect(_on_join_pressed)
	_leave_btn.pressed.connect(func(): await _lobby_node.leave_lobby())
	_loadout_btn.pressed.connect(func(): await _lobby_node.push_loadout_change("smg"))
	_map_btn.pressed.connect(func(): await _lobby_node.change_map("docks"))

	_lobby_node = get_node_or_null("/root/Lobby")
	if _lobby_node == null:
		_append("[color=red]Lobby autoload missing.[/color]")
		_set_buttons_enabled(false)
		return

	_lobby_node.lobby_joined.connect(_on_lobby_joined)
	_lobby_node.lobby_left.connect(_on_lobby_left)
	_lobby_node.lobby_disconnected.connect(_on_lobby_disconnected)
	# Drive in-progress feedback off the autoload's state machine so the
	# log shows "Hosting…" / "Joining…" / "Leaving…" the moment the user
	# clicks (or an invite-accept cascades through). Without this the UI
	# looks frozen between the click and the lobby_joined fire.
	_lobby_node.state_changed.connect(_on_lobby_state_changed)

	# Wait for sign-in, then enable buttons. PlayFab Multiplayer init
	# happens lazily inside Lobby.host_lobby() / join_lobby() so we
	# don't pay the cost until the user actually clicks Host or Join.
	var auth: Node = get_node_or_null("/root/Auth")
	if auth == null or not await auth.call("sign_in"):
		_append("[color=red]Sign-in failed.[/color]")
		_set_buttons_enabled(false)
		return
	_append("Signed in. Click Host or Join to bring up multiplayer.")
	_set_buttons_enabled(true)
	_leave_btn.disabled = true

func _on_lobby_joined(lobby: PlayFabLobby) -> void:
	_append("[color=green]Joined lobby %s[/color]" % lobby.lobby_id)
	# Surface the connection string in the same LineEdit the client
	# pastes into, so the host can select-and-copy it (Ctrl+C) and hand
	# it to a second device. select_all() puts the field in a state
	# where Ctrl+C copies the full string without an extra click.
	_conn_edit.text = lobby.connection_string
	_conn_edit.caret_column = lobby.connection_string.length()
	_conn_edit.select_all()
	_conn_edit.grab_focus()
	_refresh_members(lobby)
	_leave_btn.disabled = false
	lobby.state_changed.connect(func(_change): _refresh_members(lobby))

func _on_lobby_left() -> void:
	_append("Left lobby.")
	_members.clear()
	_leave_btn.disabled = true

func _on_lobby_disconnected() -> void:
	_append("[color=orange]Disconnected from lobby (kicked or network error).[/color]")
	_members.clear()
	_leave_btn.disabled = true

func _on_lobby_state_changed(state) -> void:
	# State enum lives on the Lobby autoload; resolve at runtime since the
	# parse gate doesn't see autoload types. Only the busy transitions
	# need feedback — IN_LOBBY / READY are already announced by
	# lobby_joined / lobby_left below.
	var s = _lobby_node.State
	match state:
		s.HOSTING:
			_append("[color=yellow]Hosting lobby…[/color]")
		s.JOINING:
			_append("[color=yellow]Joining lobby…[/color]")
		s.LEAVING:
			_append("[color=yellow]Leaving lobby…[/color]")
	# Gate buttons against the in-flight op so a second click can't race
	# the cascade. host_lobby / join_lobby already refuse non-READY
	# entries but the user shouldn't see a clickable Host button while
	# we're already hosting.
	var busy: bool = state == s.HOSTING or state == s.JOINING or state == s.LEAVING
	var in_lobby: bool = state == s.IN_LOBBY
	_host_btn.disabled = busy or in_lobby
	_join_btn.disabled = busy or in_lobby
	_leave_btn.disabled = not in_lobby
	_loadout_btn.disabled = not in_lobby
	_map_btn.disabled = not in_lobby

func _refresh_members(lobby: PlayFabLobby) -> void:
	_members.clear()
	for member: PlayFabLobbyMember in lobby.members:
		var marker := " (local)" if member.is_local else ""
		_members.add_item("%s%s" % [member.user_id, marker])

func _on_join_pressed() -> void:
	var conn := _conn_edit.text.strip_edges()
	if conn.is_empty():
		_append("[color=orange]Paste a connection string into the field first.[/color]")
		return
	await _lobby_node.join_lobby(conn)

func _set_buttons_enabled(enabled: bool) -> void:
	_host_btn.disabled = not enabled
	_join_btn.disabled = not enabled
	_loadout_btn.disabled = not enabled
	_map_btn.disabled = not enabled

func _append(line: String) -> void:
	_log.append_text(line + "\n")
	print(line)

func _on_back_pressed() -> void:
	get_tree().change_scene_to_file("res://shared/tutorial_picker.tscn")
