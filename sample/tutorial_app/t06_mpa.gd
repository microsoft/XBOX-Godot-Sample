extends Control

## Tutorial 6 reference scene — Multiplayer Activity + presence + invites.
##
## Layered on top of the T5 Lobby autoload: pull the live friends list
## from Xbox Social Manager via Lobby.get_friends_async, send a targeted
## invite to the selected friend(s), open the system invite picker, and
## track friend activity + presence for the selected friend(s).
##
## Source: docs/tutorials/06-multiplayer-activity.md

@onready var _log: RichTextLabel = $Root/LogPanel/Log
@onready var _friends_list: ItemList = $Root/Friends
@onready var _refresh_btn: Button = $Root/Buttons/RefreshBtn
@onready var _invite_btn: Button = $Root/Buttons/InviteBtn
@onready var _picker_btn: Button = $Root/Buttons/PickerBtn
@onready var _track_btn: Button = $Root/Buttons/TrackBtn
@onready var _stop_btn: Button = $Root/Buttons/StopBtn
@onready var _back_btn: Button = $Root/Buttons/BackBtn
@onready var _invite_dialog: ConfirmationDialog = $InviteDialog

var _lobby_node: Node = null
# Item 3 / B3 — the invite_id this scene's dialog is currently bound
# to. confirm/reject calls pass it back so a stale dialog (overwritten
# by a newer invite) can't fire the wrong leave+join.
var _dialog_invite_id: int = 0

func _ready() -> void:
	_back_btn.pressed.connect(_on_back_pressed)
	_refresh_btn.pressed.connect(_on_refresh_pressed)
	_invite_btn.pressed.connect(_on_invite_pressed)
	_picker_btn.pressed.connect(func(): await _lobby_node.open_invite_picker())
	_track_btn.pressed.connect(_on_track_pressed)
	_stop_btn.pressed.connect(func(): _lobby_node.stop_tracking_friends())
	_invite_dialog.confirmed.connect(_on_invite_dialog_confirmed)
	_invite_dialog.canceled.connect(_on_invite_dialog_canceled)

	_lobby_node = get_node_or_null("/root/Lobby")
	if _lobby_node == null:
		_append("[color=red]Lobby autoload missing.[/color]")
		_set_buttons_enabled(false)
		return

	# Item 3 / B3 — prompt the user before tearing down their current
	# lobby to honor an MPA invite. The Lobby autoload only fires the
	# pending signal when accepting would destroy a live session;
	# invites accepted while not in a lobby join directly.
	_lobby_node.invite_pending_confirmation.connect(_on_invite_pending_confirmation)
	_lobby_node.invite_pending_cleared.connect(_on_invite_pending_cleared)

	# Wait for sign-in, then enable buttons + populate the friends list.
	# PlayFab Multiplayer init happens lazily inside Lobby.host_lobby() /
	# invite_friend() — this scene doesn't need it brought up at _ready.
	var auth: Node = get_node_or_null("/root/Auth")
	if auth == null or not await auth.call("sign_in"):
		_append("[color=red]Sign-in failed.[/color]")
		_set_buttons_enabled(false)
		return
	_append("Signed in. Host a lobby in T5 first, then invite or track friends here.")
	_set_buttons_enabled(true)
	await _on_refresh_pressed()

func _on_refresh_pressed() -> void:
	_friends_list.clear()
	_friends_list.add_item("(loading friends…)")
	_friends_list.set_item_disabled(0, true)
	var friends: Array = await _lobby_node.get_friends_async()
	_friends_list.clear()
	if friends.is_empty():
		_friends_list.add_item("(no friends found)")
		_friends_list.set_item_disabled(0, true)
		_append("[i]No friends returned by Social Manager. Confirm sign-in and that the title has the Social capability.[/i]")
		return
	for friend: GDKSocialUser in friends:
		var label := "%s  —  %s" % [_format_gamertag(friend), friend.xuid]
		var idx := _friends_list.add_item(label)
		_friends_list.set_item_metadata(idx, friend.xuid)
	_append("Loaded %d friends." % friends.size())

func _on_invite_pressed() -> void:
	var xuids := _selected_xuids()
	if xuids.is_empty():
		_append("[color=orange]Select a friend in the list above first.[/color]")
		return
	for xuid in xuids:
		await _lobby_node.invite_friend(xuid)

func _on_track_pressed() -> void:
	var xuids := _selected_xuids()
	if xuids.is_empty():
		_append("[color=orange]Select one or more friends in the list above first.[/color]")
		return
	await _lobby_node.track_friend_activities(xuids)

func _selected_xuids() -> PackedStringArray:
	var out := PackedStringArray()
	for idx in _friends_list.get_selected_items():
		var meta = _friends_list.get_item_metadata(idx)
		if typeof(meta) == TYPE_STRING and not (meta as String).is_empty():
			out.append(meta)
	return out

func _format_gamertag(friend: GDKSocialUser) -> String:
	if not friend.gamertag.is_empty():
		return friend.gamertag
	if not friend.display_name.is_empty():
		return friend.display_name
	return "(unknown)"

func _set_buttons_enabled(enabled: bool) -> void:
	_refresh_btn.disabled = not enabled
	_invite_btn.disabled = not enabled
	_picker_btn.disabled = not enabled
	_track_btn.disabled = not enabled
	_stop_btn.disabled = not enabled

func _append(line: String) -> void:
	_log.append_text(line + "\n")
	print(line)

func _on_back_pressed() -> void:
	get_tree().change_scene_to_file("res://shared/tutorial_picker.tscn")

func _on_invite_pending_confirmation(invite_id: int, _connection_string: String) -> void:
	# Bind this dialog instance to the latest invite_id. If a second
	# invite arrives while the dialog is still up, the signal fires
	# again with a new id — we just update the bound id (the dialog
	# text is generic enough that the same dialog works for both).
	_dialog_invite_id = invite_id
	_append("[i]Invite received while in a lobby — see prompt.[/i]")
	_invite_dialog.popup_centered()

func _on_invite_pending_cleared(invite_id: int) -> void:
	# Autoload signaled that the slot is now empty (rejected, accepted,
	# disconnected, etc.). Hide the dialog if it's still bound to that
	# same invite, otherwise we'd be closing the prompt for a different
	# invite the user hasn't responded to yet.
	if invite_id == _dialog_invite_id and _invite_dialog.visible:
		_invite_dialog.hide()

func _on_invite_dialog_confirmed() -> void:
	# Surface progress: confirm_pending_invite internally leaves the
	# current lobby and joins the invited one, which can take a couple
	# of seconds. Without this line the user sees a closed dialog and
	# no feedback until lobby_joined fires from the new session.
	_append("[i]Accepting invite — leaving current lobby and joining the invited one…[/i]")
	var ok: bool = await _lobby_node.confirm_pending_invite(_dialog_invite_id)
	if not ok:
		_append("[i]Invite accept dropped — pending invite was stale or leave/join failed.[/i]")

func _on_invite_dialog_canceled() -> void:
	_lobby_node.reject_pending_invite(_dialog_invite_id)
