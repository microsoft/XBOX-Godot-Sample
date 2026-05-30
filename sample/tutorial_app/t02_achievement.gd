extends Control

const AddonApi = preload("res://shared/addon_api.gd")

## Tutorial 2 reference scene — unlock an Xbox achievement.
##
## Buttons drive each tutorial step in turn:
##   - List declared achievements (Step 1)
##   - Push 50% then 100% progress (Step 2)
##   - Run the gameplay-side helper (Step 3, simulated)
##
## Wires the `achievement_unlocked` and `runtime_error` signals
## (Steps 2 + 4). Output is appended to the log RichTextLabel.
##
## Source: docs/tutorials/02-unlock-achievement.md

const FIRST_SCORE_ID := "1"

@onready var _log: RichTextLabel = $Root/LogPanel/Log
@onready var _list_btn: Button = $Root/Buttons/ListBtn
@onready var _push_btn: Button = $Root/Buttons/PushBtn
@onready var _back_btn: Button = $Root/Buttons/BackBtn

var _auth: Node = null
var _unlocked: Dictionary = {}

func _ready() -> void:
	_back_btn.pressed.connect(_on_back_pressed)
	_list_btn.pressed.connect(_on_list_pressed)
	_push_btn.pressed.connect(_on_push_pressed)

	_auth = get_node_or_null("/root/Auth")
	if _auth == null:
		_append("[color=red]Auth autoload missing.[/color]")
		_set_buttons_enabled(false)
		return

	if not Engine.has_singleton("GDK"):
		_append("[color=red]GDK extension is not loaded.[/color]")
		_set_buttons_enabled(false)
		return

	AddonApi.singleton("GDK").achievements.achievement_unlocked.connect(_on_achievement_unlocked)
	AddonApi.singleton("GDK").achievements.runtime_error.connect(_on_achievements_runtime_error)

	_set_buttons_enabled(false)
	_append("Waiting for sign-in…")
	if await _auth.call("sign_in"):
		_append("Signed in.")
		_set_buttons_enabled(true)
	else:
		_append("[color=red]Sign-in failed at %s: %s[/color]" % [
				_auth.call("get_last_error_stage"),
				_auth.call("get_last_error_message")])

func _on_list_pressed() -> void:
	await _print_cached_achievements()

func _on_push_pressed() -> void:
	await _push_progress(50)
	await _push_progress(100)

func _print_cached_achievements() -> void:
	var user = _auth.get("xbox_user")
	if user == null:
		return

	var result = await AddonApi.singleton("GDK").achievements.query_player_achievements_async(user)
	if not result.ok:
		_append("[color=orange][Ach] query failed: %s[/color]" % result.message)
		return

	var cache: Array = AddonApi.singleton("GDK").achievements.get_cached_achievements(user)
	_append("[Ach] %d achievement(s) declared for this title" % cache.size())
	for entry in cache:
		var ach = entry
		_append("[Ach]   %s (%s) — %d%%" % [ach.id, ach.name, ach.progress_percent])

func _push_progress(percent: int) -> void:
	var user = _auth.get("xbox_user")
	if user == null:
		return
	var result = await AddonApi.singleton("GDK").achievements.update_achievement_async(
		user, FIRST_SCORE_ID, percent)
	if result.ok:
		_append("[Ach] Updated to %d%% — result ok" % percent)
	else:
		_append("[color=orange][Ach] Update to %d%% failed: %s (%s)[/color]" % [percent, result.message, result.code])

func _on_achievement_unlocked(user, achievement_id: String) -> void:
	_unlocked[achievement_id] = true
	var cache: Array = AddonApi.singleton("GDK").achievements.get_cached_achievements(user)
	for entry in cache:
		var ach = entry
		if ach.id == achievement_id:
			_append("[color=green][Ach] Unlocked: %s[/color]" % ach.name)
			return
	_append("[color=green][Ach] Unlocked id=%s (not in cache yet)[/color]" % achievement_id)

func _on_achievements_runtime_error(result) -> void:
	_append("[color=orange][Ach] Achievements subsystem error: %s (0x%08X)[/color]" % [result.message, result.hresult])

func _set_buttons_enabled(enabled: bool) -> void:
	_list_btn.disabled = not enabled
	_push_btn.disabled = not enabled

func _append(line: String) -> void:
	_log.append_text(line + "\n")
	print(line)

func _on_back_pressed() -> void:
	get_tree().change_scene_to_file("res://shared/tutorial_picker.tscn")
