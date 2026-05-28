extends VBoxContainer

## Tutorial 8 Step 2 — Achievements panel.
##
## Mirrors the T2 happy path: buttons for the canonical progress curve
## (25 / 50 / 75 / Unlock), live status from the cached achievement.
##
## Item 5 / B5 (signal hygiene): re-driven by Auth.state_changed so a
## sign-in retry after a transient failure still wires the panel up,
## and external connections are explicitly torn down in _exit_tree.
##
## Source: docs/tutorials/08-integration-tech-demo.md Step 2

const ACHIEVEMENT_ID := "1"

@onready var _status: Label = $Status
@onready var _progress_25: Button = $Progress25
@onready var _progress_50: Button = $Progress50
@onready var _progress_75: Button = $Progress75
@onready var _unlock: Button = $Unlock

var _auth: Node = null
var _initialized: bool = false

func _ready() -> void:
	_auth = get_node_or_null("/root/Auth")
	if _auth == null:
		_status.text = "[ERR] Auth autoload missing"
		return
	_auth.state_changed.connect(_on_auth_state_changed)
	if _auth.is_signed_in():
		_initialize_after_sign_in()
		return
	# Cold entry — kick off sign-in. The state_changed listener above
	# is our safety net if sign_in fails and the user later hits the
	# HUD retry.
	await _auth.sign_in()
	if is_inside_tree() and _auth.is_signed_in():
		_initialize_after_sign_in()

func _exit_tree() -> void:
	if _auth != null and _auth.state_changed.is_connected(_on_auth_state_changed):
		_auth.state_changed.disconnect(_on_auth_state_changed)
	if _initialized and Engine.has_singleton("GDK"):
		if GDK.achievements.achievement_unlocked.is_connected(_on_achievement_unlocked):
			GDK.achievements.achievement_unlocked.disconnect(_on_achievement_unlocked)

func _on_auth_state_changed(_state) -> void:
	if _initialized or _auth == null:
		return
	if _auth.is_signed_in():
		_initialize_after_sign_in()

func _initialize_after_sign_in() -> void:
	if _initialized:
		return
	_initialized = true

	GDK.achievements.achievement_unlocked.connect(_on_achievement_unlocked)
	_progress_25.pressed.connect(_push_progress.bind(25))
	_progress_50.pressed.connect(_push_progress.bind(50))
	_progress_75.pressed.connect(_push_progress.bind(75))
	_unlock.pressed.connect(_push_progress.bind(100))

	var user: GDKUser = _auth.get("xbox_user")
	var result: GDKResult = await GDK.achievements.query_player_achievements_async(user)
	if not is_inside_tree():
		return
	if result.ok:
		print("[Ach] cached %d achievement(s) for the local user" % result.data.size())
		_refresh_status()
	else:
		push_warning("[Ach] query failed: %s" % result.message)
		_status.text = "Query failed: %s" % result.message

func _push_progress(percent: int) -> void:
	var user: GDKUser = _auth.get("xbox_user")
	var result: GDKResult = await GDK.achievements.update_achievement_async(
			user, ACHIEVEMENT_ID, percent)
	if not is_inside_tree():
		return
	if result.ok:
		_status.text = "Pushed %d%%" % percent
		print("[Ach] Updated to %d%%" % percent)
	else:
		_status.text = "Update failed: %s" % result.message
		push_warning("[Ach] %s" % _status.text)

func _on_achievement_unlocked(user: GDKUser, achievement_id: String) -> void:
	if achievement_id != ACHIEVEMENT_ID:
		return
	_status.text = "Unlocked %s for %s" % [achievement_id, user.gamertag]
	print("[Ach] Unlocked id=%s" % achievement_id)
	_refresh_status()

func _refresh_status() -> void:
	var user: GDKUser = _auth.get("xbox_user")
	var cached: Array = GDK.achievements.get_cached_achievements(user)
	for ach in cached:
		if ach.id == ACHIEVEMENT_ID:
			var verb: String = "Unlocked" if ach.progress_percent >= 100 else "In progress"
			_status.text = "%s: %d%% — %s" % [ach.id, ach.progress_percent, verb]
			return
	_status.text = "Achievement %s not yet in cache" % ACHIEVEMENT_ID
