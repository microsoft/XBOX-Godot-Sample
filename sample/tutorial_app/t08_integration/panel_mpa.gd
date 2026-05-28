extends VBoxContainer

## Tutorial 8 Step 6 — Multiplayer Activity panel.
##
## Reactive view of the activity advertised by the Lobby autoload.
## Refresh pulls the friends list from Xbox Social Manager via
## Lobby.get_friends_async; Send button calls Lobby.invite_friend on
## the selected friend; picker button calls Lobby.open_invite_picker.
## Per-target permission filtering is inherited from T6 Step 5 — no
## panel code needed.
##
## Item 5 / B5 (signal hygiene): re-driven by Auth.state_changed so a
## sign-in retry after a transient failure still wires the panel up,
## and the GDK + PlayFab + Lobby external connections are explicitly
## torn down in _exit_tree.
##
## Source: docs/tutorials/08-integration-tech-demo.md Step 6

@onready var _state: Label = $State
@onready var _friends: OptionButton = $Friends
@onready var _refresh: Button = $ButtonRow/Refresh
@onready var _send: Button = $ButtonRow/Send
@onready var _picker: Button = $ButtonRow/Picker
@onready var _log: RichTextLabel = $Log
@onready var _invite_dialog: ConfirmationDialog = $InviteDialog

var _auth: Node = null
var _lobby_node: Node = null
# Item 3 / B3 — see t06_mpa.gd for the full rationale.
var _dialog_invite_id: int = 0
var _initialized: bool = false

func _ready() -> void:
	_auth = get_node_or_null("/root/Auth")
	_lobby_node = get_node_or_null("/root/Lobby")
	if _auth == null or _lobby_node == null:
		_state.text = "[ERR] Auth/Lobby autoload missing"
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
	if Engine.has_singleton("GDK"):
		if GDK.multiplayer_activity.invite_accepted.is_connected(_on_invite_accepted):
			GDK.multiplayer_activity.invite_accepted.disconnect(_on_invite_accepted)
		if GDK.multiplayer_activity.pending_invite_received.is_connected(_on_pending_invite):
			GDK.multiplayer_activity.pending_invite_received.disconnect(_on_pending_invite)
	if Engine.has_singleton("PlayFab"):
		if PlayFab.multiplayer.state_changed.is_connected(_refresh_state):
			PlayFab.multiplayer.state_changed.disconnect(_refresh_state)
	if _lobby_node != null:
		if _lobby_node.invite_pending_confirmation.is_connected(_on_invite_pending_confirmation):
			_lobby_node.invite_pending_confirmation.disconnect(_on_invite_pending_confirmation)
		if _lobby_node.invite_pending_cleared.is_connected(_on_invite_pending_cleared):
			_lobby_node.invite_pending_cleared.disconnect(_on_invite_pending_cleared)

func _on_auth_state_changed(_state) -> void:
	if _initialized or _auth == null:
		return
	if _auth.is_signed_in():
		_initialize_after_sign_in()

func _initialize_after_sign_in() -> void:
	if _initialized:
		return
	_initialized = true
	_refresh.pressed.connect(_on_refresh_pressed)
	_send.pressed.connect(_on_send_pressed)
	_picker.pressed.connect(_on_picker_pressed)
	_invite_dialog.confirmed.connect(_on_invite_dialog_confirmed)
	_invite_dialog.canceled.connect(_on_invite_dialog_canceled)
	GDK.multiplayer_activity.invite_accepted.connect(_on_invite_accepted)
	GDK.multiplayer_activity.pending_invite_received.connect(_on_pending_invite)
	PlayFab.multiplayer.state_changed.connect(_refresh_state)
	# Item 3 / B3 — prompt before MPA invite tears down current session.
	_lobby_node.invite_pending_confirmation.connect(_on_invite_pending_confirmation)
	_lobby_node.invite_pending_cleared.connect(_on_invite_pending_cleared)
	print("[Mpa] activity panel ready (idle — no lobby)")
	_refresh_state()
	await _on_refresh_pressed()

func _refresh_state(_change = null) -> void:
	var current: PlayFabLobby = _lobby_node.call("get_current_lobby")
	if current == null:
		_state.text = "No lobby — activity not advertised"
		return
	_state.text = "Advertising %s (%d / %d, cross=%s)" % [
			current.lobby_id.left(8),
			current.member_count,
			current.max_member_count,
			str(false)]

func _on_refresh_pressed() -> void:
	_friends.clear()
	_friends.add_item("(loading…)")
	_friends.set_item_disabled(0, true)
	var list: Array = await _lobby_node.get_friends_async()
	if not is_inside_tree():
		return
	_friends.clear()
	if list.is_empty():
		_friends.add_item("(no friends found)")
		_friends.set_item_disabled(0, true)
		return
	for friend: GDKSocialUser in list:
		var label := friend.gamertag if not friend.gamertag.is_empty() else friend.display_name
		if label.is_empty():
			label = friend.xuid
		var idx := _friends.item_count
		_friends.add_item(label)
		_friends.set_item_metadata(idx, friend.xuid)

func _on_send_pressed() -> void:
	if _friends.item_count == 0 or _friends.is_item_disabled(_friends.selected):
		_log.append_text("[i]Refresh first and pick a friend[/i]\n")
		return
	var xuid = _friends.get_item_metadata(_friends.selected)
	if typeof(xuid) != TYPE_STRING or (xuid as String).is_empty():
		_log.append_text("[i]No XUID on selected entry[/i]\n")
		return
	var sent: bool = await _lobby_node.invite_friend(xuid)
	if not is_inside_tree():
		return
	if sent:
		_log.append_text("Sent invite to %s\n" % xuid)
	else:
		_log.append_text("[i]Invite to %s suppressed (no lobby, permission-denied, or send failure)[/i]\n" % xuid)

func _on_picker_pressed() -> void:
	await _lobby_node.open_invite_picker()
	if not is_inside_tree():
		return
	_log.append_text("Closed system invite picker\n")

func _on_invite_accepted(invite: Dictionary) -> void:
	_log.append_text("Accepted: %s\n" % invite.get("raw_uri", ""))

func _on_pending_invite(invite: Dictionary) -> void:
	_log.append_text("Pending: %s\n" % invite.get("raw_uri", ""))

func _on_invite_pending_confirmation(invite_id: int, _connection_string: String) -> void:
	_dialog_invite_id = invite_id
	_log.append_text("[i]Invite received while in a lobby — see prompt[/i]\n")
	_invite_dialog.popup_centered()

func _on_invite_pending_cleared(invite_id: int) -> void:
	if invite_id == _dialog_invite_id and _invite_dialog.visible:
		_invite_dialog.hide()

func _on_invite_dialog_confirmed() -> void:
	var ok: bool = await _lobby_node.confirm_pending_invite(_dialog_invite_id)
	if not is_inside_tree():
		return
	if not ok:
		_log.append_text("[i]Invite accept dropped — pending invite was stale or leave/join failed[/i]\n")

func _on_invite_dialog_canceled() -> void:
	_lobby_node.reject_pending_invite(_dialog_invite_id)
