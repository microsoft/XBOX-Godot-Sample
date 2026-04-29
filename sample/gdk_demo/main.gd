extends Control
## Minimal demo scene for the runtime/users/achievements/presence/social/multiplayer-activity baseline.

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
@onready var social_label: RichTextLabel = $VBoxContainer/SocialLabel

var _silent_sign_in_op = null
var _gamer_picture_op = null
var _achievement_query_op = null
var _achievement_update_op = null
var _presence_query_op = null
var _friends_op = null
var _friends_group = null
var _mpa_set_op = null
var _mpa_delete_op = null
var _mpa_invite_ui_op = null
var _loaded_gamer_picture_xuid := ""
var _pending_gamer_picture_xuid := ""
var _last_mpa_event_text := "No invite events yet."

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

var gdk = GDKBootstrap.get_gdk()
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
gdk.presence.presence_changed.connect(_on_presence_changed)
gdk.presence.local_presence_set.connect(_on_local_presence_set)
gdk.social.social_graph_changed.connect(_on_social_graph_changed)
gdk.social.social_group_updated.connect(_on_social_group_updated)
gdk.social.social_user_changed.connect(_on_social_user_changed)
gdk.multiplayer_activity.activities_updated.connect(_on_mpa_activities_updated)
gdk.multiplayer_activity.pending_invite_received.connect(_on_pending_invite_received)
gdk.multiplayer_activity.invite_accepted.connect(_on_invite_accepted)

_refresh_state()

func _refresh_state() -> void:
var gdk = GDKBootstrap.get_gdk()
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
_refresh_social_ui()
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
_start_presence_query(user)
_start_social_flow(user)
_refresh_achievement_ui()
_refresh_social_ui()
_refresh_mpa_ui()

func _clear_user() -> void:
gamertag_label.text = "No active user"
xuid_label.text = ""
user_label.text = "User: Not signed in"
_friends_group = null
_clear_avatar()
var gdk = GDKBootstrap.get_gdk()
if gdk != null and gdk.is_initialized():
sign_in_button.text = "Retry Silent Sign-In"
sign_in_button.disabled = false
achievement_button.text = "Update Achievement %s" % DEMO_ACHIEVEMENT_ID
achievement_button.disabled = true
achievement_label.text = "Achievement %s: sign in to load progress" % DEMO_ACHIEVEMENT_ID
_refresh_social_ui()
_refresh_mpa_ui()

func _clear_avatar() -> void:
avatar_rect.texture = null
avatar_rect.visible = false
_loaded_gamer_picture_xuid = ""
_pending_gamer_picture_xuid = ""

func _load_gamer_picture(user) -> void:
if user == null:
_clear_avatar()
return

if _loaded_gamer_picture_xuid == user.xuid and avatar_rect.texture != null:
avatar_rect.visible = true
return

if _gamer_picture_op != null and not _gamer_picture_op.is_done():
if _pending_gamer_picture_xuid == user.xuid:
return
_gamer_picture_op.cancel()

_pending_gamer_picture_xuid = user.xuid
avatar_rect.texture = null
avatar_rect.visible = false

var requested_xuid: String = user.xuid
var gdk = GDKBootstrap.get_gdk()
if gdk == null:
_clear_avatar()
return

var gamer_picture_op = gdk.users.get_gamer_picture_async(user)
_gamer_picture_op = gamer_picture_op
if gamer_picture_op.is_done():
_on_gamer_picture_completed(gamer_picture_op.get_result(), gamer_picture_op, requested_xuid)
else:
gamer_picture_op.completed.connect(_on_gamer_picture_completed.bind(gamer_picture_op, requested_xuid))

func _on_gamer_picture_completed(result, gamer_picture_op, requested_xuid: String) -> void:
if gamer_picture_op != _gamer_picture_op:
return

_pending_gamer_picture_xuid = ""

var gdk = GDKBootstrap.get_gdk()
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
if _silent_sign_in_op and not _silent_sign_in_op.is_done():
return

var gdk = GDKBootstrap.get_gdk()
if gdk == null:
status_label.text = "GDK: Extension not loaded"
return

status_label.text = "GDK: Attempting silent sign-in..."
sign_in_button.disabled = true
sign_in_button.text = "Silent sign-in..."

_silent_sign_in_op = gdk.users.add_default_user_async()
if _silent_sign_in_op == null:
status_label.text = "GDK: Silent sign-in could not start"
sign_in_button.text = "Retry Silent Sign-In"
sign_in_button.disabled = false
_refresh_achievement_ui()
return

if _silent_sign_in_op.is_done():
_on_sign_in_completed(_silent_sign_in_op.get_result())
else:
_silent_sign_in_op.completed.connect(_on_sign_in_completed)

func _on_sign_in_completed(result) -> void:
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
var gdk = GDKBootstrap.get_gdk()
status_label.text = "GDK: Initialized ✓"
if gdk != null and not gdk.users.get_primary_user():
sign_in_button.text = "Retry Silent Sign-In"
sign_in_button.disabled = false
_refresh_achievement_ui()
_refresh_social_ui()
_refresh_mpa_ui()

func _on_runtime_shutdown() -> void:
status_label.text = "GDK: Shut down"
_clear_user()
sign_in_button.disabled = true

func _on_runtime_error(result) -> void:
var gdk = GDKBootstrap.get_gdk()
status_label.text = "GDK Error: %s" % result.message
if gdk != null and not gdk.users.get_primary_user():
sign_in_button.text = "Retry Silent Sign-In"
sign_in_button.disabled = false
_refresh_achievement_ui()
_refresh_social_ui()
_refresh_mpa_ui()

func _on_user_added(user) -> void:
_show_user(user)

func _on_user_changed(user) -> void:
_show_user(user)

func _on_user_removed(local_id: int) -> void:
status_label.text = "GDK: User %d removed" % local_id
_clear_user()

func _on_primary_user_changed(user) -> void:
if user:
_show_user(user)
else:
_clear_user()

func _is_primary_user(user) -> bool:
var gdk = GDKBootstrap.get_gdk()
var primary_user = gdk.users.get_primary_user() if gdk != null else null
return user != null and primary_user != null and user.local_id == primary_user.local_id

func _find_cached_achievement(user, achievement_id: String):
var gdk = GDKBootstrap.get_gdk()
if gdk == null:
return null

for achievement in gdk.achievements.get_cached_achievements(user):
if achievement.id == achievement_id:
return achievement
return null

func _find_cached_presence(xuid: String):
var gdk = GDKBootstrap.get_gdk()
if gdk == null:
return null
return gdk.presence.get_cached_presence(xuid)

func _presence_summary_text(presence) -> String:
if presence == null:
return "no cached record"

var summary := presence.get_user_state_name()
var title_records = presence.get_title_records()
if title_records.size() > 0:
var title_record: Dictionary = title_records[0]
var rich_presence := String(title_record.get("rich_presence_string", ""))
if rich_presence != "":
summary += " — %s" % rich_presence
return summary

func _refresh_social_ui() -> void:
var social_lines := PackedStringArray()
var gdk = GDKBootstrap.get_gdk()

if gdk == null or not gdk.is_initialized():
social_lines.append("Presence: GDK not initialized")
social_lines.append("Friends tracked: unavailable")
social_label.text = "\n".join(social_lines)
return

var user = gdk.users.get_primary_user()
if user == null:
social_lines.append("Presence: sign in to load")
social_lines.append("Friends tracked: sign in to load")
social_label.text = "\n".join(social_lines)
return

var presence = _find_cached_presence(user.xuid)
if presence != null:
social_lines.append("Presence: %s" % _presence_summary_text(presence))
elif _presence_query_op != null and not _presence_query_op.is_done():
social_lines.append("Presence: loading...")
else:
social_lines.append("Presence: no cached record")

if _friends_group != null and _friends_group.is_loaded():
var friends = gdk.social.get_group_users(_friends_group)
social_lines.append("Friends tracked: %d" % friends.size())
elif _friends_op != null and not _friends_op.is_done():
social_lines.append("Friends tracked: loading...")
else:
social_lines.append("Friends tracked: social graph not ready")

social_label.text = "\n".join(social_lines)

func _refresh_achievement_ui() -> void:
var gdk = GDKBootstrap.get_gdk()
var query_in_progress: bool = _achievement_query_op != null and not _achievement_query_op.is_done()
var update_in_progress: bool = _achievement_update_op != null and not _achievement_update_op.is_done()

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
var gdk = GDKBootstrap.get_gdk()
var set_in_progress = _mpa_set_op != null and not _mpa_set_op.is_done()
var delete_in_progress = _mpa_delete_op != null and not _mpa_delete_op.is_done()
var invite_ui_in_progress = _mpa_invite_ui_op != null and not _mpa_invite_ui_op.is_done()

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
if _mpa_set_op != null and not _mpa_set_op.is_done():
return

var gdk = GDKBootstrap.get_gdk()
if gdk == null:
return

var user = gdk.users.get_primary_user()
if user == null:
return

_mpa_set_op = gdk.multiplayer_activity.set_activity_async(
user,
DEMO_MPA_CONNECTION_STRING,
"followed",
DEMO_MPA_MAX_PLAYERS,
DEMO_MPA_CURRENT_PLAYERS,
DEMO_MPA_GROUP_ID,
false
)
if _mpa_set_op.is_done():
_on_mpa_set_completed(_mpa_set_op.get_result())
else:
_mpa_set_op.completed.connect(_on_mpa_set_completed)
_refresh_mpa_ui()

func _on_mpa_set_completed(result) -> void:
if result != null and result.ok:
status_label.text = "GDK: Multiplayer activity ready ✓"
else:
status_label.text = "GDK: Multiplayer activity failed: %s" % result.message
_refresh_mpa_ui()

func _on_mpa_clear_pressed() -> void:
if _mpa_delete_op != null and not _mpa_delete_op.is_done():
return

var gdk = GDKBootstrap.get_gdk()
if gdk == null:
return

var user = gdk.users.get_primary_user()
if user == null:
return

_mpa_delete_op = gdk.multiplayer_activity.delete_activity_async(user)
if _mpa_delete_op.is_done():
_on_mpa_delete_completed(_mpa_delete_op.get_result())
else:
_mpa_delete_op.completed.connect(_on_mpa_delete_completed)
_refresh_mpa_ui()

func _on_mpa_delete_completed(result) -> void:
if result != null and result.ok:
status_label.text = "GDK: Multiplayer activity cleared"
else:
status_label.text = "GDK: Clear activity failed: %s" % result.message
_refresh_mpa_ui()

func _on_mpa_invite_ui_pressed() -> void:
if _mpa_invite_ui_op != null and not _mpa_invite_ui_op.is_done():
return

var gdk = GDKBootstrap.get_gdk()
if gdk == null:
return

var user = gdk.users.get_primary_user()
if user == null:
return

_mpa_invite_ui_op = gdk.multiplayer_activity.show_invite_ui_async(user)
if _mpa_invite_ui_op.is_done():
_on_mpa_invite_ui_completed(_mpa_invite_ui_op.get_result())
else:
_mpa_invite_ui_op.completed.connect(_on_mpa_invite_ui_completed)
_refresh_mpa_ui()

func _on_mpa_invite_ui_completed(result) -> void:
if result != null and result.ok:
status_label.text = "GDK: Invite UI completed"
else:
status_label.text = "GDK: Invite UI failed: %s" % result.message
_refresh_mpa_ui()

func _on_mpa_activities_updated(xuids: PackedStringArray) -> void:
var gdk = GDKBootstrap.get_gdk()
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
if _achievement_query_op != null and not _achievement_query_op.is_done():
return

var gdk = GDKBootstrap.get_gdk()
if gdk == null:
return

achievement_button.text = "Loading Achievement %s..." % DEMO_ACHIEVEMENT_ID
achievement_button.disabled = true
achievement_label.text = "Achievement %s: syncing current progress..." % DEMO_ACHIEVEMENT_ID

_achievement_query_op = gdk.achievements.query_player_achievements_async(user)
if _achievement_query_op.is_done():
_on_achievement_query_completed(_achievement_query_op.get_result())
else:
_achievement_query_op.completed.connect(_on_achievement_query_completed)

func _on_achievement_query_completed(result) -> void:
if not result.ok:
achievement_button.text = "Update Achievement %s" % DEMO_ACHIEVEMENT_ID
achievement_button.disabled = true
achievement_label.text = "Achievement %s: %s" % [DEMO_ACHIEVEMENT_ID, result.message]
return

_refresh_achievement_ui()

func _on_achievement_pressed() -> void:
if _achievement_update_op != null and not _achievement_update_op.is_done():
return

var gdk = GDKBootstrap.get_gdk()
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

_achievement_update_op = gdk.achievements.update_achievement_async(user, DEMO_ACHIEVEMENT_ID, next_progress)
if _achievement_update_op.is_done():
_on_achievement_update_completed(_achievement_update_op.get_result())
else:
_achievement_update_op.completed.connect(_on_achievement_update_completed)

func _on_achievement_update_completed(result) -> void:
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

func _start_presence_query(user) -> void:
if user == null:
return
if _presence_query_op != null and not _presence_query_op.is_done():
return

var gdk = GDKBootstrap.get_gdk()
if gdk == null:
return

var xuids := PackedStringArray([user.xuid])
_presence_query_op = gdk.presence.get_presence_async(xuids)
if _presence_query_op.is_done():
_on_presence_query_completed(_presence_query_op.get_result())
else:
_presence_query_op.completed.connect(_on_presence_query_completed)

func _on_presence_query_completed(_result) -> void:
_refresh_social_ui()

func _start_social_flow(user) -> void:
if user == null:
return
if _friends_group != null and _friends_group.is_loaded():
_refresh_social_ui()
return
if _friends_op != null and not _friends_op.is_done():
return

var gdk = GDKBootstrap.get_gdk()
if gdk == null:
return

var start_result = gdk.social.start_social_graph(user)
if not start_result.ok:
social_label.text = "Presence: unavailable\nFriends tracked: %s" % start_result.message
return

_friends_op = gdk.social.get_friends_async(user)
if _friends_op.is_done():
_on_friends_completed(_friends_op.get_result())
else:
_friends_op.completed.connect(_on_friends_completed)

func _on_friends_completed(result) -> void:
if result != null and result.ok and result.data != null:
_friends_group = result.data
_refresh_social_ui()

func _on_presence_changed(xuid: String, _presence) -> void:
var gdk = GDKBootstrap.get_gdk()
var primary_user = gdk.users.get_primary_user() if gdk != null else null
if primary_user != null and primary_user.xuid == xuid:
_refresh_social_ui()

func _on_local_presence_set(user) -> void:
if _is_primary_user(user):
_refresh_social_ui()

func _on_social_graph_changed(user) -> void:
if _is_primary_user(user):
_refresh_social_ui()

func _on_social_group_updated(group) -> void:
var gdk = GDKBootstrap.get_gdk()
var primary_user = gdk.users.get_primary_user() if gdk != null else null
if primary_user == null or group == null:
return

var local_user = group.get_local_user()
if local_user == null or local_user.local_id != primary_user.local_id:
return

if group.get_group_type() == GDKSocialGroup.GROUP_TYPE_FILTER \
and group.get_relationship_filter() == GDKSocialFilter.RELATIONSHIP_FILTER_FRIENDS:
_friends_group = group

_refresh_social_ui()

func _on_social_user_changed(_xuid: String, _social_user) -> void:
if _friends_group != null and _friends_group.is_loaded():
_refresh_social_ui()
