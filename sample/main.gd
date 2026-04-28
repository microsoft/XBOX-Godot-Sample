extends Control
## Minimal demo scene for the runtime/users/achievements baseline.

const DEMO_ACHIEVEMENT_ID := "1"
const DEMO_ACHIEVEMENT_STEP := 25

@onready var status_label: Label = $VBoxContainer/StatusLabel
@onready var user_label: Label = $VBoxContainer/UserLabel
@onready var gamertag_label: Label = $VBoxContainer/UserPanel/UserHBox/GamertagLabel
@onready var xuid_label: Label = $VBoxContainer/UserPanel/UserHBox/XuidLabel
@onready var avatar_rect: TextureRect = $VBoxContainer/UserPanel/UserHBox/AvatarRect
@onready var input_label: Label = $VBoxContainer/InputLabel
@onready var sign_in_button: Button = $VBoxContainer/SignInButton
@onready var haptics_toggle: CheckButton = $VBoxContainer/HapticsToggle
@onready var achievement_button: Button = $VBoxContainer/AchievementButton
@onready var achievement_label: Label = $VBoxContainer/AchievementLabel
@onready var gamepad_display: RichTextLabel = $VBoxContainer/GamepadDisplay

var _silent_sign_in_op = null
var _achievement_query_op = null
var _achievement_update_op = null

func _ready() -> void:
	input_label.visible = false
	haptics_toggle.visible = false
	achievement_button.visible = true
	achievement_label.visible = true
	gamepad_display.visible = false
	avatar_rect.visible = false
	user_label.visible = true

	sign_in_button.pressed.connect(_on_sign_in_pressed)
	achievement_button.pressed.connect(_on_achievement_pressed)

	GDK.initialized.connect(_on_runtime_initialized)
	GDK.shutdown_completed.connect(_on_runtime_shutdown)
	GDK.runtime_error.connect(_on_runtime_error)
	GDK.users.user_added.connect(_on_user_added)
	GDK.users.user_changed.connect(_on_user_changed)
	GDK.users.user_removed.connect(_on_user_removed)
	GDK.users.primary_user_changed.connect(_on_primary_user_changed)
	GDK.achievements.achievements_updated.connect(_on_achievements_updated)
	GDK.achievements.achievement_unlocked.connect(_on_achievement_unlocked)

	_refresh_state()

func _refresh_state() -> void:
	if GDK.is_initialized():
		status_label.text = "GDK: Initialized ✓"
		sign_in_button.disabled = false
	else:
		status_label.text = "GDK: Not initialized"
		sign_in_button.disabled = true

	var user = GDK.users.get_primary_user()
	if user:
		_show_user(user)
		sign_in_button.text = "User Ready"
		sign_in_button.disabled = true
	else:
		_clear_user()
		sign_in_button.text = "Retry Silent Sign-In"

	_refresh_achievement_ui()

func _show_user(user) -> void:
	gamertag_label.text = user.gamertag
	xuid_label.text = "XUID: %s" % user.xuid
	user_label.text = "User: %s" % user.gamertag
	sign_in_button.text = "User Ready"
	sign_in_button.disabled = true
	_start_achievement_query(user)
	_refresh_achievement_ui()

func _clear_user() -> void:
	gamertag_label.text = "No active user"
	xuid_label.text = ""
	user_label.text = "User: Not signed in"
	if GDK.is_initialized():
		sign_in_button.text = "Retry Silent Sign-In"
		sign_in_button.disabled = false
	achievement_button.text = "Update Achievement %s" % DEMO_ACHIEVEMENT_ID
	achievement_button.disabled = true
	achievement_label.text = "Achievement %s: sign in to load progress" % DEMO_ACHIEVEMENT_ID

func _on_sign_in_pressed() -> void:
	if _silent_sign_in_op and not _silent_sign_in_op.is_done():
		return

	status_label.text = "GDK: Attempting silent sign-in..."
	sign_in_button.disabled = true
	sign_in_button.text = "Silent sign-in..."

	_silent_sign_in_op = GDK.users.add_default_user_async()
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
	status_label.text = "GDK: Initialized ✓"
	if not GDK.users.get_primary_user():
		sign_in_button.text = "Retry Silent Sign-In"
		sign_in_button.disabled = false
	_refresh_achievement_ui()

func _on_runtime_shutdown() -> void:
	status_label.text = "GDK: Shut down"
	_clear_user()
	sign_in_button.disabled = true

func _on_runtime_error(result) -> void:
	status_label.text = "GDK Error: %s" % result.message
	if not GDK.users.get_primary_user():
		sign_in_button.text = "Retry Silent Sign-In"
		sign_in_button.disabled = false
	_refresh_achievement_ui()

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
	var primary_user = GDK.users.get_primary_user()
	return user != null and primary_user != null and user.local_id == primary_user.local_id

func _find_cached_achievement(user, achievement_id: String):
	for achievement in GDK.achievements.get_cached_achievements(user):
		if achievement.id == achievement_id:
			return achievement
	return null

func _refresh_achievement_ui() -> void:
	var query_in_progress: bool = _achievement_query_op != null and not _achievement_query_op.is_done()
	var update_in_progress: bool = _achievement_update_op != null and not _achievement_update_op.is_done()

	if not GDK.is_initialized():
		achievement_button.text = "Update Achievement %s" % DEMO_ACHIEVEMENT_ID
		achievement_button.disabled = true
		achievement_label.text = "Achievement %s: GDK not initialized" % DEMO_ACHIEVEMENT_ID
		return

	var user = GDK.users.get_primary_user()
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

func _start_achievement_query(user) -> void:
	if user == null:
		return
	if _achievement_query_op != null and not _achievement_query_op.is_done():
		return

	achievement_button.text = "Loading Achievement %s..." % DEMO_ACHIEVEMENT_ID
	achievement_button.disabled = true
	achievement_label.text = "Achievement %s: syncing current progress..." % DEMO_ACHIEVEMENT_ID

	_achievement_query_op = GDK.achievements.query_player_achievements_async(user)
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

	var user = GDK.users.get_primary_user()
	if user == null:
		return

	var achievement = _find_cached_achievement(user, DEMO_ACHIEVEMENT_ID)
	var next_progress: int = DEMO_ACHIEVEMENT_STEP
	if achievement != null:
		next_progress = mini(100, int(achievement.progress_percent) + DEMO_ACHIEVEMENT_STEP)

	achievement_button.text = "Updating Achievement %s..." % DEMO_ACHIEVEMENT_ID
	achievement_button.disabled = true
	achievement_label.text = "Achievement %s: requesting %d%%..." % [DEMO_ACHIEVEMENT_ID, next_progress]

	_achievement_update_op = GDK.achievements.update_achievement_async(user, DEMO_ACHIEVEMENT_ID, next_progress)
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
