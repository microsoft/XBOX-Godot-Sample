extends Control

const AddonApi = preload("res://shared/addon_api.gd")

## GDK Tutorial 4 reference scene — Multiplayer Activity + presence.
##
## GDK-only, no PlayFab. This scene drives the Xbox Multiplayer Activity
## surface directly:
##   - Set the local user's activity with a title-defined connection
##     string (GDK.multiplayer_activity.set_activity_async). In a real
##     title this string encodes how a peer rejoins your session; the
##     integrated track derives it from a PlayFab lobby, but the GDK-only
##     track shows that MPA stands alone.
##   - List the local user's friends via the Xbox Social Manager
##     (GDK.social), filter them by the play_multiplayer permission
##     (GDK.privacy), and send targeted invites or open the system invite
##     picker (GDK.multiplayer_activity).
##   - Track friend activity + presence (GDK.multiplayer_activity +
##     GDK.presence).
##
## NOTE: scene scripts use `get_node("/root/GdkAuth")` instead of the bare
## `GdkAuth.` reference so the headless parse gate stays clean.
##
## Source: docs/tutorials/gdk/04-mpa.md

# Title-defined connection string. A shipping title encodes the data a
# peer needs to rejoin (e.g. a session id); here we use a static token so
# the MPA round-trip works without any networking backend.
const CONNECTION_STRING := "gdk-tutorial-session-1"
const MAX_PLAYERS := 4

@onready var _log: RichTextLabel = $Root/LogPanel/Log
@onready var _friends_list: ItemList = $Root/Friends
@onready var _activity_btn: Button = $Root/Buttons/SetActivityBtn
@onready var _refresh_btn: Button = $Root/Buttons/RefreshBtn
@onready var _invite_btn: Button = $Root/Buttons/InviteBtn
@onready var _picker_btn: Button = $Root/Buttons/PickerBtn
@onready var _track_btn: Button = $Root/Buttons/TrackBtn
@onready var _stop_btn: Button = $Root/Buttons/StopBtn
@onready var _back_btn: Button = $Root/Buttons/BackBtn

var _auth: Node = null
var _social_graph_started: bool = false
var _friends_group = null
var _activity_set: bool = false
var _watched_xuids: PackedStringArray = PackedStringArray()

func _ready() -> void:
	_back_btn.pressed.connect(_on_back_pressed)
	_activity_btn.pressed.connect(_on_set_activity_pressed)
	_refresh_btn.pressed.connect(_on_refresh_pressed)
	_invite_btn.pressed.connect(_on_invite_pressed)
	_picker_btn.pressed.connect(func(): await _open_invite_picker())
	_track_btn.pressed.connect(_on_track_pressed)
	_stop_btn.pressed.connect(_on_stop_pressed)

	_auth = get_node_or_null("/root/GdkAuth")
	if _auth == null:
		_append("[color=red]GdkAuth autoload missing.[/color]")
		_set_buttons_enabled(false)
		return
	if not Engine.has_singleton("GDK"):
		_append("[color=red]GDK extension is not loaded.[/color]")
		_set_buttons_enabled(false)
		return

	# Forward incoming MPA invites so the developer can see the payload.
	AddonApi.singleton("GDK").multiplayer_activity.pending_invite_received.connect(_on_pending_invite_received)
	AddonApi.singleton("GDK").multiplayer_activity.invite_accepted.connect(_on_invite_accepted)

	_set_buttons_enabled(false)
	_append("Waiting for sign-in…")
	if not await _auth.call("sign_in"):
		_append("[color=red]Sign-in failed at %s: %s[/color]" % [
				_auth.call("get_last_error_stage"),
				_auth.call("get_last_error_message")])
		return
	_append("Signed in. Set your activity first, then invite or track friends.")
	_set_buttons_enabled(true)
	await _on_refresh_pressed()

func _exit_tree() -> void:
	# Release the social-graph group so the Social Manager stops issuing
	# background work after the scene is gone.
	if not Engine.has_singleton("GDK"):
		return
	if _friends_group != null:
		AddonApi.singleton("GDK").social.destroy_social_group(_friends_group)
		_friends_group = null
	if _social_graph_started:
		var user = _auth.get("xbox_user") if _auth != null else null
		if user != null:
			AddonApi.singleton("GDK").social.stop_social_graph(user)
		_social_graph_started = false

# --- Step 1: set the local activity ---

func _on_set_activity_pressed() -> void:
	var user = _auth.get("xbox_user")
	if user == null:
		return
	var result = await AddonApi.singleton("GDK").multiplayer_activity.set_activity_async(
		user, CONNECTION_STRING, "followed", MAX_PLAYERS, 1)
	if result.ok:
		_activity_set = true
		_append("[color=green][MPA] Activity set (connection_string=%s).[/color]" % CONNECTION_STRING)
	else:
		_append("[color=orange][MPA] set_activity failed: %s (%s)[/color]" % [result.message, result.code])

# --- Friends list via the Xbox Social Manager ---

func _on_refresh_pressed() -> void:
	_friends_list.clear()
	_friends_list.add_item("(loading friends…)")
	_friends_list.set_item_disabled(0, true)
	var friends: Array = await _get_friends_async()
	_friends_list.clear()
	if friends.is_empty():
		_friends_list.add_item("(no friends found)")
		_friends_list.set_item_disabled(0, true)
		_append("[i]No friends returned by Social Manager. Confirm sign-in and that the title has the Social capability.[/i]")
		return
	for friend in friends:
		var label := "%s  —  %s" % [_format_gamertag(friend), friend.xuid]
		var idx := _friends_list.add_item(label)
		_friends_list.set_item_metadata(idx, friend.xuid)
	_append("Loaded %d friends." % friends.size())

func _get_friends_async() -> Array:
	var user = _auth.get("xbox_user")
	if user == null:
		return []
	if not _social_graph_started:
		var sg = AddonApi.singleton("GDK").social.start_social_graph(user)
		if not sg.ok:
			push_warning("[MPA] start_social_graph failed: %s" % sg.message)
			return []
		_social_graph_started = true
	if _friends_group == null:
		var f = await AddonApi.singleton("GDK").social.get_friends_async(user)
		if not f.ok:
			push_warning("[MPA] get_friends failed: %s" % f.message)
			return []
		_friends_group = f.data
	var users = AddonApi.singleton("GDK").social.get_group_users(_friends_group)
	if not users.ok:
		push_warning("[MPA] get_group_users failed: %s" % users.message)
		return []
	return users.data

# --- Step 5: invites ---

func _on_invite_pressed() -> void:
	if not _activity_set:
		_append("[color=orange]Set your activity first (Step 1) so invites carry a connection string.[/color]")
		return
	var xuids := _selected_xuids()
	if xuids.is_empty():
		_append("[color=orange]Select a friend in the list above first.[/color]")
		return
	var allowed := await _filter_invitable(xuids)
	if allowed.is_empty():
		_append("[color=orange]Invite blocked by the play_multiplayer permission.[/color]")
		return
	# Empty connection_string reuses the cached activity connection string
	# set by set_activity_async above.
	var result = await AddonApi.singleton("GDK").multiplayer_activity.send_invites_async(
		_auth.get("xbox_user"), allowed, false, "")
	if result.ok:
		_append("[MPA] Sent invite to %d friend(s)." % allowed.size())
	else:
		_append("[color=orange][MPA] send_invites failed: %s (%s)[/color]" % [result.message, result.code])

func _open_invite_picker() -> void:
	if not _activity_set:
		_append("[color=orange]Set your activity first (Step 1) before opening the picker.[/color]")
		return
	var result = await AddonApi.singleton("GDK").multiplayer_activity.show_invite_ui_async(_auth.get("xbox_user"))
	if not result.ok:
		_append("[color=orange][MPA] show_invite_ui failed: %s[/color]" % result.message)

func _filter_invitable(xuids: PackedStringArray) -> PackedStringArray:
	var user = _auth.get("xbox_user")
	if user == null or xuids.is_empty():
		return PackedStringArray()
	var pf = await AddonApi.singleton("GDK").privacy.batch_check_permission_async(
			user, "play_multiplayer", xuids)
	if not pf.ok:
		push_warning("[MPA] permission batch failed: %s" % pf.message)
		return PackedStringArray()
	var allowed := PackedStringArray()
	for entry: Dictionary in pf.data:
		if bool(entry.get("allowed", false)):
			allowed.append(String(entry.get("target_xuid", "")))
	return allowed

# --- Steps 6 + 7: friend activity + presence tracking ---

func _on_track_pressed() -> void:
	var xuids := _selected_xuids()
	if xuids.is_empty():
		_append("[color=orange]Select one or more friends in the list above first.[/color]")
		return
	_watched_xuids = xuids
	var user = _auth.get("xbox_user")

	var activities = await AddonApi.singleton("GDK").multiplayer_activity.get_activities_async(user, xuids)
	if not activities.ok:
		_append("[color=orange][MPA] get_activities failed: %s[/color]" % activities.message)

	AddonApi.singleton("GDK").presence.track_presence(user, xuids)
	var presence = await AddonApi.singleton("GDK").presence.get_presence_async(xuids)
	if not presence.ok:
		_append("[color=orange][Pres] get_presence failed: %s[/color]" % presence.message)

	for xuid in xuids:
		var info = AddonApi.singleton("GDK").multiplayer_activity.get_cached_activity(xuid)
		if info != null:
			_append("[MPA] %s activity: connection_string=%s" % [xuid, info.connection_string])
		else:
			_append("[MPA] %s has no cached activity." % xuid)

func _on_stop_pressed() -> void:
	if _watched_xuids.is_empty():
		return
	AddonApi.singleton("GDK").presence.stop_tracking_presence(_auth.get("xbox_user"), _watched_xuids)
	_append("Stopped tracking %d friend(s)." % _watched_xuids.size())
	_watched_xuids = PackedStringArray()

# --- Incoming invite signals ---

func _on_pending_invite_received(invite: Dictionary) -> void:
	_append("[MPA] Pending invite from %s." % invite.get("sender_xuid", "(unknown)"))

func _on_invite_accepted(invite: Dictionary) -> void:
	_append("[color=green][MPA] Invite accepted — join %s.[/color]" % invite.get("connection_string", invite.get("raw_uri", "")))

# --- Helpers ---

func _selected_xuids() -> PackedStringArray:
	var out := PackedStringArray()
	for idx in _friends_list.get_selected_items():
		var meta = _friends_list.get_item_metadata(idx)
		if typeof(meta) == TYPE_STRING and not (meta as String).is_empty():
			out.append(meta)
	return out

func _format_gamertag(friend) -> String:
	if not friend.gamertag.is_empty():
		return friend.gamertag
	if not friend.display_name.is_empty():
		return friend.display_name
	return "(unknown)"

func _set_buttons_enabled(enabled: bool) -> void:
	_activity_btn.disabled = not enabled
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
