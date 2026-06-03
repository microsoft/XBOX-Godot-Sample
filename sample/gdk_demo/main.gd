extends Control
## Minimal demo scene for the runtime/users/achievements baseline.

const DEMO_ACHIEVEMENT_ID := "1"
const DEMO_ACHIEVEMENT_STEP := 25
const DEMO_MPA_CONNECTION_STRING := "godot-gdk-demo://sample-session"
const DEMO_MPA_GROUP_ID := "sample-mpa-group"
const DEMO_MPA_MAX_PLAYERS := 4
const DEMO_MPA_CURRENT_PLAYERS := 1

@onready var status_label: Label = $VBoxContainer/StatusLabel
@onready var user_label: Label = $VBoxContainer/UserLabel
@onready var gamertag_label: Label = $VBoxContainer/UserPanel/UserHBox/GamertagLabel
@onready var xuid_label: Label = $VBoxContainer/UserPanel/UserHBox/XuidLabel
@onready var avatar_rect: TextureRect = $VBoxContainer/UserPanel/UserHBox/AvatarRect
@onready var sign_in_button: Button = $VBoxContainer/SignInButton
@onready var mpa_set_button: Button = $VBoxContainer/MpaSetActivityButton
@onready var mpa_clear_button: Button = $VBoxContainer/MpaClearActivityButton
@onready var mpa_invite_ui_button: Button = $VBoxContainer/MpaInviteUiButton
@onready var achievement_button: Button = $VBoxContainer/AchievementButton
@onready var achievement_label: Label = $VBoxContainer/AchievementLabel
@onready var mpa_label: RichTextLabel = $VBoxContainer/MpaLabel

var _silent_sign_in_in_progress := false
var _gamer_picture_request_id := 0
var _achievement_query_in_progress := false
var _achievement_update_in_progress := false
var _mpa_set_in_progress := false
var _mpa_delete_in_progress := false
var _mpa_invite_ui_in_progress := false
var _loaded_gamer_picture_xuid := ""
var _pending_gamer_picture_xuid := ""
var _last_mpa_event_text := "No invite events yet."

func _get_gdk():
	var bootstrap = get_node_or_null("/root/GDKBootstrap")
	if bootstrap != null and bootstrap.has_method("get_gdk"):
		return bootstrap.get_gdk()
	return null

func _ready() -> void:
	achievement_button.visible = true
	achievement_label.visible = true
	avatar_rect.visible = false
	user_label.visible = true

	sign_in_button.pressed.connect(_on_sign_in_pressed)
	mpa_set_button.pressed.connect(_on_mpa_set_pressed)
	mpa_clear_button.pressed.connect(_on_mpa_clear_pressed)
	mpa_invite_ui_button.pressed.connect(_on_mpa_invite_ui_pressed)
	achievement_button.pressed.connect(_on_achievement_pressed)

	var gdk = _get_gdk()
	if gdk != null:
		gdk.initialized.connect(_on_runtime_initialized)
		gdk.shutdown_completed.connect(_on_runtime_shutdown)
		gdk.runtime_error.connect(_on_runtime_error)
		gdk.users.user_added.connect(_on_user_added)
		gdk.users.user_changed.connect(_on_user_changed)
		gdk.users.user_removed.connect(_on_user_removed)
		gdk.users.primary_user_changed.connect(_on_primary_user_changed)
		gdk.achievements.achievements_updated.connect(_on_achievements_updated)
		gdk.achievements.achievement_unlocked.connect(_on_achievement_unlocked)
		gdk.multiplayer_activity.activities_updated.connect(_on_mpa_activities_updated)
		gdk.multiplayer_activity.pending_invite_received.connect(_on_pending_invite_received)
		gdk.multiplayer_activity.invite_accepted.connect(_on_invite_accepted)

	_refresh_state()

func _refresh_state() -> void:
	var gdk = _get_gdk()
	if gdk != null and gdk.is_initialized():
		status_label.text = "GDK: Initialized ✓"
		sign_in_button.disabled = false
	elif gdk != null:
		status_label.text = "GDK: Not initialized"
		sign_in_button.disabled = true
	else:
		status_label.text = "GDK: Extension not loaded"
		sign_in_button.disabled = true

	var user = gdk.users.get_primary_user() if gdk != null else null
	if user:
		_show_user(user)
		sign_in_button.text = "User Ready"
		sign_in_button.disabled = true
	else:
		_clear_user()
		sign_in_button.text = "Retry Silent Sign-In"

	_refresh_achievement_ui()
	_refresh_mpa_ui()

func _show_user(user) -> void:
	var store_user_text: String = "yes" if user.store_user else "no"
	var sign_in_state_text: String = user.get_sign_in_state_name()
	var age_group_text: String = user.get_age_group_name()
	gamertag_label.text = user.gamertag
	xuid_label.text = "XUID: %s" % user.xuid
	user_label.text = "State: %s • Age: %s • Store user: %s" % [sign_in_state_text, age_group_text, store_user_text]
	sign_in_button.text = "User Ready"
	sign_in_button.disabled = true
	_load_gamer_picture(user)
	_start_achievement_query(user)
	_refresh_achievement_ui()
	_refresh_mpa_ui()

func _clear_user() -> void:
	gamertag_label.text = "No active user"
	xuid_label.text = ""
	user_label.text = "User: Not signed in"
	_clear_avatar()
	var gdk = _get_gdk()
	if gdk != null and gdk.is_initialized():
		sign_in_button.text = "Retry Silent Sign-In"
		sign_in_button.disabled = false
	achievement_button.text = "Update Achievement %s" % DEMO_ACHIEVEMENT_ID
	achievement_button.disabled = true
	achievement_label.text = "Achievement %s: sign in to load progress" % DEMO_ACHIEVEMENT_ID
	_refresh_mpa_ui()

func _clear_avatar() -> void:
	avatar_rect.texture = null
	avatar_rect.visible = false
	_gamer_picture_request_id += 1
	_loaded_gamer_picture_xuid = ""
	_pending_gamer_picture_xuid = ""

func _load_gamer_picture(user) -> void:
	if user == null:
		_clear_avatar()
		return

	if _loaded_gamer_picture_xuid == user.xuid and avatar_rect.texture != null:
		avatar_rect.visible = true
		return

	if _pending_gamer_picture_xuid == user.xuid:
		return

	_gamer_picture_request_id += 1
	var request_id: int = _gamer_picture_request_id
	_pending_gamer_picture_xuid = user.xuid
	_loaded_gamer_picture_xuid = ""
	avatar_rect.texture = null
	avatar_rect.visible = false

	var requested_xuid: String = user.xuid
	var gdk = _get_gdk()
	if gdk == null:
		_clear_avatar()
		return

	var gamer_picture_signal: Signal = gdk.users.get_gamer_picture_async(user)
	gamer_picture_signal.connect(_on_gamer_picture_completed.bind(request_id, requested_xuid), CONNECT_ONE_SHOT)

func _on_gamer_picture_completed(result, request_id: int, requested_xuid: String) -> void:
	if request_id != _gamer_picture_request_id:
		return

	_pending_gamer_picture_xuid = ""

	var gdk = _get_gdk()
	var primary_user = gdk.users.get_primary_user() if gdk != null else null
	if primary_user == null or primary_user.xuid != requested_xuid:
		return

	if result == null or not result.ok or result.data == null:
		_clear_avatar()
		return

	var image: Image = result.data
	var texture: ImageTexture = ImageTexture.create_from_image(image)
	avatar_rect.texture = texture
	avatar_rect.visible = texture != null
	_loaded_gamer_picture_xuid = primary_user.xuid

func _on_sign_in_pressed() -> void:
	if _silent_sign_in_in_progress:
		return

	var gdk = _get_gdk()
	if gdk == null:
		status_label.text = "GDK: Extension not loaded"
		return

	status_label.text = "GDK: Attempting silent sign-in..."
	sign_in_button.disabled = true
	sign_in_button.text = "Silent sign-in..."

	_silent_sign_in_in_progress = true
	var sign_in_signal: Signal = gdk.users.add_default_user_async()
	sign_in_signal.connect(_on_sign_in_completed, CONNECT_ONE_SHOT)

func _on_sign_in_completed(result) -> void:
	_silent_sign_in_in_progress = false

	if result == null:
		status_label.text = "GDK: Silent sign-in could not start"
		sign_in_button.text = "Retry Silent Sign-In"
		sign_in_button.disabled = false
		_refresh_achievement_ui()
		return

	if result.ok and result.data:
		status_label.text = "GDK: User ready ✓"
		_show_user(result.data)
	else:
		status_label.text = "GDK: Silent sign-in unavailable: %s" % result.message
		sign_in_button.text = "Retry Silent Sign-In"
		sign_in_button.disabled = false
		_refresh_achievement_ui()

func _on_runtime_initialized() -> void:
	var gdk = _get_gdk()
	status_label.text = "GDK: Initialized ✓"
	if gdk != null and not gdk.users.get_primary_user():
		sign_in_button.text = "Retry Silent Sign-In"
		sign_in_button.disabled = false
	_refresh_achievement_ui()
	_refresh_mpa_ui()

func _on_runtime_shutdown() -> void:
	_silent_sign_in_in_progress = false
	status_label.text = "GDK: Shut down"
	_clear_user()
	sign_in_button.disabled = true

func _on_runtime_error(result) -> void:
	var gdk = _get_gdk()
	status_label.text = "GDK Error: %s" % result.message
	if gdk != null and not gdk.users.get_primary_user():
		sign_in_button.text = "Retry Silent Sign-In"
		sign_in_button.disabled = false
	_refresh_achievement_ui()
	_refresh_mpa_ui()

func _on_user_added(user) -> void:
	if _is_primary_user(user):
		_show_user(user)

func _on_user_changed(user, change_kind: String) -> void:
	if _is_primary_user(user):
		status_label.text = "GDK: Primary user updated (%s)" % change_kind
		_show_user(user)

func _on_user_removed(local_id: int) -> void:
	status_label.text = "GDK: User %d removed" % local_id
	var gdk = _get_gdk()
	var primary_user = gdk.users.get_primary_user() if gdk != null else null
	if primary_user == null:
		_clear_user()

func _on_primary_user_changed(user) -> void:
	if user:
		_show_user(user)
	else:
		_clear_user()

func _is_primary_user(user) -> bool:
	var gdk = _get_gdk()
	var primary_user = gdk.users.get_primary_user() if gdk != null else null
	return user != null and primary_user != null and user.local_id == primary_user.local_id

func _find_cached_achievement(user, achievement_id: String):
	var gdk = _get_gdk()
	if gdk == null:
		return null

	for achievement in gdk.achievements.get_cached_achievements(user):
		if achievement.id == achievement_id:
			return achievement
	return null

func _refresh_achievement_ui() -> void:
	var gdk = _get_gdk()
	var query_in_progress: bool = _achievement_query_in_progress
	var update_in_progress: bool = _achievement_update_in_progress

	if gdk == null:
		achievement_button.text = "Update Achievement %s" % DEMO_ACHIEVEMENT_ID
		achievement_button.disabled = true
		achievement_label.text = "Achievement %s: extension not loaded" % DEMO_ACHIEVEMENT_ID
		return

	if not gdk.is_initialized():
		achievement_button.text = "Update Achievement %s" % DEMO_ACHIEVEMENT_ID
		achievement_button.disabled = true
		achievement_label.text = "Achievement %s: GDK not initialized" % DEMO_ACHIEVEMENT_ID
		return

	var user = gdk.users.get_primary_user()
	if user == null:
		achievement_button.text = "Update Achievement %s" % DEMO_ACHIEVEMENT_ID
		achievement_button.disabled = true
		achievement_label.text = "Achievement %s: sign in to load progress" % DEMO_ACHIEVEMENT_ID
		return

	var achievement = _find_cached_achievement(user, DEMO_ACHIEVEMENT_ID)
	if achievement != null:
		achievement_label.text = "Achievement %s: %s (%d%%)" % [
			DEMO_ACHIEVEMENT_ID,
			achievement.progress_state,
			achievement.progress_percent
		]
		if achievement.unlocked:
			achievement_button.text = "Achievement %s Unlocked" % DEMO_ACHIEVEMENT_ID
			achievement_button.disabled = true
		else:
			var next_progress: int = mini(100, int(achievement.progress_percent) + DEMO_ACHIEVEMENT_STEP)
			achievement_button.text = "Update Achievement %s to %d%%" % [DEMO_ACHIEVEMENT_ID, next_progress]
			achievement_button.disabled = query_in_progress or update_in_progress
		return

	if query_in_progress:
		achievement_button.text = "Loading Achievement %s..." % DEMO_ACHIEVEMENT_ID
		achievement_button.disabled = true
		achievement_label.text = "Achievement %s: syncing current progress..." % DEMO_ACHIEVEMENT_ID
		return

	achievement_button.text = "Update Achievement %s to %d%%" % [DEMO_ACHIEVEMENT_ID, DEMO_ACHIEVEMENT_STEP]
	achievement_button.disabled = update_in_progress
	achievement_label.text = "Achievement %s: no cached progress yet" % DEMO_ACHIEVEMENT_ID

func _format_mpa_activity(activity) -> String:
	if activity == null:
		return "No cached activity."

	var max_players = int(activity.max_players)
	var current_players = int(activity.current_players)
	var player_text = "%d/%d players" % [current_players, max_players] if max_players > 0 else "%d players" % current_players
	var restriction_text = activity.join_restriction if activity.join_restriction != "" else "unknown"
	var group_text = activity.group_id if activity.group_id != "" else "no-group"
	return "Activity ready • %s • %s • %s" % [player_text, restriction_text, group_text]

func _format_invite_event(invite) -> String:
	if typeof(invite) != TYPE_DICTIONARY:
		return "Unknown invite event"

	var action = String(invite.get("action", "unknown"))
	if action == "invite_handle_accept":
		return "invite from %s to %s" % [
			String(invite.get("sender_xuid", "?")),
			String(invite.get("invited_xuid", "?"))
		]
	if action == "activity_handle_join":
		return "join from %s to %s" % [
			String(invite.get("joiner_xuid", "?")),
			String(invite.get("joinee_xuid", "?"))
		]

	return String(invite.get("raw_uri", "Unknown invite event"))

func _refresh_mpa_ui() -> void:
	var gdk = _get_gdk()
	var set_in_progress = _mpa_set_in_progress
	var delete_in_progress = _mpa_delete_in_progress
	var invite_ui_in_progress = _mpa_invite_ui_in_progress

	if gdk == null:
		mpa_set_button.disabled = true
		mpa_clear_button.disabled = true
		mpa_invite_ui_button.disabled = true
		mpa_label.text = "MPA: extension not loaded\nInvite events: %s" % _last_mpa_event_text
		return

	if not gdk.is_initialized():
		mpa_set_button.disabled = true
		mpa_clear_button.disabled = true
		mpa_invite_ui_button.disabled = true
		mpa_label.text = "MPA: GDK not initialized\nInvite events: %s" % _last_mpa_event_text
		return

	var user = gdk.users.get_primary_user()
	if user == null:
		mpa_set_button.disabled = true
		mpa_clear_button.disabled = true
		mpa_invite_ui_button.disabled = true
		mpa_label.text = "MPA: sign in to create an activity\nInvite events: %s" % _last_mpa_event_text
		return

	var activity = gdk.multiplayer_activity.get_cached_activity(user.xuid)
	mpa_set_button.disabled = set_in_progress or delete_in_progress
	mpa_clear_button.disabled = activity == null or set_in_progress or delete_in_progress
	mpa_invite_ui_button.disabled = activity == null or set_in_progress or delete_in_progress or invite_ui_in_progress

	if set_in_progress:
		mpa_label.text = "MPA: setting local activity...\nInvite events: %s" % _last_mpa_event_text
		return
	if delete_in_progress:
		mpa_label.text = "MPA: clearing local activity...\nInvite events: %s" % _last_mpa_event_text
		return
	if invite_ui_in_progress:
		mpa_label.text = "MPA: waiting for invite UI...\nInvite events: %s" % _last_mpa_event_text
		return

	mpa_label.text = "MPA: %s\nInvite events: %s" % [_format_mpa_activity(activity), _last_mpa_event_text]

func _on_mpa_set_pressed() -> void:
	if _mpa_set_in_progress:
		return

	var gdk = _get_gdk()
	if gdk == null:
		return

	var user = gdk.users.get_primary_user()
	if user == null:
		return

	var completion_signal = gdk.multiplayer_activity.set_activity_async(
		user,
		DEMO_MPA_CONNECTION_STRING,
		"followed",
		DEMO_MPA_MAX_PLAYERS,
		DEMO_MPA_CURRENT_PLAYERS,
		DEMO_MPA_GROUP_ID,
		false
	)
	_mpa_set_in_progress = true
	completion_signal.connect(_on_mpa_set_completed, CONNECT_ONE_SHOT)
	_refresh_mpa_ui()

func _on_mpa_set_completed(result) -> void:
	_mpa_set_in_progress = false
	if result != null and result.ok:
		status_label.text = "GDK: Multiplayer activity ready ✓"
	else:
		status_label.text = "GDK: Multiplayer activity failed: %s" % result.message
	_refresh_mpa_ui()

func _on_mpa_clear_pressed() -> void:
	if _mpa_delete_in_progress:
		return

	var gdk = _get_gdk()
	if gdk == null:
		return

	var user = gdk.users.get_primary_user()
	if user == null:
		return

	var completion_signal = gdk.multiplayer_activity.delete_activity_async(user)
	_mpa_delete_in_progress = true
	completion_signal.connect(_on_mpa_delete_completed, CONNECT_ONE_SHOT)
	_refresh_mpa_ui()

func _on_mpa_delete_completed(result) -> void:
	_mpa_delete_in_progress = false
	if result != null and result.ok:
		status_label.text = "GDK: Multiplayer activity cleared"
	else:
		status_label.text = "GDK: Clear activity failed: %s" % result.message
	_refresh_mpa_ui()

func _on_mpa_invite_ui_pressed() -> void:
	if _mpa_invite_ui_in_progress:
		return

	var gdk = _get_gdk()
	if gdk == null:
		return

	var user = gdk.users.get_primary_user()
	if user == null:
		return

	var completion_signal = gdk.multiplayer_activity.show_invite_ui_async(user)
	_mpa_invite_ui_in_progress = true
	completion_signal.connect(_on_mpa_invite_ui_completed, CONNECT_ONE_SHOT)
	_refresh_mpa_ui()

func _on_mpa_invite_ui_completed(result) -> void:
	_mpa_invite_ui_in_progress = false
	if result != null and result.ok:
		status_label.text = "GDK: Invite UI completed"
	else:
		status_label.text = "GDK: Invite UI failed: %s" % result.message
	_refresh_mpa_ui()

func _on_mpa_activities_updated(xuids: PackedStringArray) -> void:
	var gdk = _get_gdk()
	var user = gdk.users.get_primary_user() if gdk != null else null
	if user != null and xuids.has(user.xuid):
		_refresh_mpa_ui()

func _on_pending_invite_received(invite: Dictionary) -> void:
	_last_mpa_event_text = "Pending invite — %s" % _format_invite_event(invite)
	status_label.text = "GDK: Pending invite received"
	_refresh_mpa_ui()

func _on_invite_accepted(invite: Dictionary) -> void:
	_last_mpa_event_text = "Accepted invite — %s" % _format_invite_event(invite)
	status_label.text = "GDK: Invite accepted"
	_refresh_mpa_ui()

func _start_achievement_query(user) -> void:
	if user == null:
		return
	if _achievement_query_in_progress:
		return

	var gdk = _get_gdk()
	if gdk == null:
		return

	achievement_button.text = "Loading Achievement %s..." % DEMO_ACHIEVEMENT_ID
	achievement_button.disabled = true
	achievement_label.text = "Achievement %s: syncing current progress..." % DEMO_ACHIEVEMENT_ID

	var completion_signal = gdk.achievements.query_player_achievements_async(user)
	_achievement_query_in_progress = true
	completion_signal.connect(_on_achievement_query_completed, CONNECT_ONE_SHOT)

func _on_achievement_query_completed(result) -> void:
	_achievement_query_in_progress = false
	if not result.ok:
		achievement_button.text = "Update Achievement %s" % DEMO_ACHIEVEMENT_ID
		achievement_button.disabled = true
		achievement_label.text = "Achievement %s: %s" % [DEMO_ACHIEVEMENT_ID, result.message]
		return

	_refresh_achievement_ui()

func _on_achievement_pressed() -> void:
	if _achievement_update_in_progress:
		return

	var gdk = _get_gdk()
	if gdk == null:
		return

	var user = gdk.users.get_primary_user()
	if user == null:
		return

	var achievement = _find_cached_achievement(user, DEMO_ACHIEVEMENT_ID)
	var next_progress: int = DEMO_ACHIEVEMENT_STEP
	if achievement != null:
		next_progress = mini(100, int(achievement.progress_percent) + DEMO_ACHIEVEMENT_STEP)

	achievement_button.text = "Updating Achievement %s..." % DEMO_ACHIEVEMENT_ID
	achievement_button.disabled = true
	achievement_label.text = "Achievement %s: requesting %d%%..." % [DEMO_ACHIEVEMENT_ID, next_progress]

	var completion_signal = gdk.achievements.update_achievement_async(user, DEMO_ACHIEVEMENT_ID, next_progress)
	_achievement_update_in_progress = true
	completion_signal.connect(_on_achievement_update_completed, CONNECT_ONE_SHOT)

func _on_achievement_update_completed(result) -> void:
	_achievement_update_in_progress = false
	if result.ok and result.data:
		var achievement = result.data
		achievement_label.text = "Achievement %s: %s (%d%%)" % [
			DEMO_ACHIEVEMENT_ID,
			achievement.progress_state,
			achievement.progress_percent
		]
	else:
		achievement_label.text = "Achievement %s: %s" % [DEMO_ACHIEVEMENT_ID, result.message]

	_refresh_achievement_ui()

func _on_achievements_updated(user) -> void:
	if _is_primary_user(user):
		_refresh_achievement_ui()

func _on_achievement_unlocked(user, achievement_id: String) -> void:
	if _is_primary_user(user) and achievement_id == DEMO_ACHIEVEMENT_ID:
		status_label.text = "GDK: Achievement %s unlocked ✓" % DEMO_ACHIEVEMENT_ID
		_refresh_achievement_ui()
		_refresh_mpa_ui()
