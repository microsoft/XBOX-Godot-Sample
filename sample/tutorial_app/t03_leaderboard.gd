extends Control

## Tutorial 3 reference scene — record a statistic + read its PlayFab
## leaderboard.
##
## Buttons drive each tutorial step:
##   - Record a fixed demo score to the statistic (Step 1)
##   - Print the global top 10 of the linked leaderboard (Step 2)
##   - Walk every page (Step 3)
##   - Print the around-user window (Step 4)
##   - Print the Xbox-friend leaderboard (Step 5)
##
## The leaderboard ranks values written to the statistic — direct
## leaderboard writes are server-only on PlayFab, so the client uses
## PlayFab.statistics.update_statistics_async to feed the leaderboard.
##
## Source: docs/tutorials/03-playfab-leaderboard.md

const STATISTIC_NAME := "high_score"
const LEADERBOARD_NAME := "high_score"
const DEMO_SCORE := 1234

@onready var _log: RichTextLabel = $Root/LogPanel/Log
@onready var _submit_btn: Button = $Root/Buttons/SubmitBtn
@onready var _top_btn: Button = $Root/Buttons/TopBtn
@onready var _pages_btn: Button = $Root/Buttons/PagesBtn
@onready var _around_btn: Button = $Root/Buttons/AroundBtn
@onready var _friend_btn: Button = $Root/Buttons/FriendBtn
@onready var _back_btn: Button = $Root/Buttons/BackBtn

var _auth: Node = null

func _ready() -> void:
	_back_btn.pressed.connect(_on_back_pressed)
	_submit_btn.pressed.connect(func(): await _record_score(DEMO_SCORE))
	_top_btn.pressed.connect(func(): await _print_global_top())
	_pages_btn.pressed.connect(func(): await _print_all_pages())
	_around_btn.pressed.connect(func(): await _print_around_user())
	_friend_btn.pressed.connect(func(): await _print_xbox_friend_leaderboard())

	_auth = get_node_or_null("/root/Auth")
	if _auth == null:
		_append("[color=red]Auth autoload missing.[/color]")
		_set_buttons_enabled(false)
		return

	if not Engine.has_singleton("PlayFab"):
		_append("[color=red]PlayFab extension is not loaded.[/color]")
		_set_buttons_enabled(false)
		return

	_set_buttons_enabled(false)
	_append("Waiting for sign-in…")
	if await _auth.call("sign_in"):
		_append("Signed in.")
		_set_buttons_enabled(true)
	else:
		_append("[color=red]Sign-in failed at %s: %s[/color]" % [
				_auth.call("get_last_error_stage"),
				_auth.call("get_last_error_message")])

func _record_score(score: int) -> void:
	var user: PlayFabUser = _auth.get("playfab_user")
	if user == null:
		return
	var result: PlayFabResult = await PlayFab.statistics.update_statistics_async(user, {
		"statistics": [
			{"name": STATISTIC_NAME, "scores": [str(score)]},
		],
	})
	if not result.ok:
		_append("[color=orange][Lead] Record failed: %s[/color]" % result.message)
		return
	_append("[Lead] Recorded score %d to statistic \"%s\"" % [score, STATISTIC_NAME])

func _print_global_top() -> void:
	var user: PlayFabUser = _auth.get("playfab_user")
	if user == null:
		return
	var result: PlayFabResult = await PlayFab.leaderboards.get_leaderboard_async(
			user, LEADERBOARD_NAME, 1, 10)
	if not result.ok:
		_append("[color=orange][Lead] get_leaderboard failed: %s[/color]" % result.message)
		return
	var page: Dictionary = result.data
	var rankings: Array = page.get("rankings", [])
	_append("[Lead] Global page 1: rank 1..%d of ~%d entries (version %d)" % [
			rankings.size(),
			page.get("entry_count", 0),
			page.get("version", -1)])
	for entry in rankings:
		var row: Dictionary = entry
		_append("[Lead]   #%d  %s — %d" % [
				row.get("rank", 0),
				_display_name(row),
				_primary_score(row)])

func _print_all_pages() -> void:
	var user: PlayFabUser = _auth.get("playfab_user")
	if user == null:
		return
	const PAGE_SIZE := 10
	var first: PlayFabResult = await PlayFab.leaderboards.get_leaderboard_async(
			user, LEADERBOARD_NAME, 1, PAGE_SIZE)
	if not first.ok:
		_append("[color=orange][Lead] first page failed: %s[/color]" % first.message)
		return
	var page: Dictionary = first.data
	var total: int = page.get("entry_count", 0)
	var version: int = page.get("version", -1)
	var next_position := 1
	var page_index := 1
	while page != null:
		var rankings: Array = page.get("rankings", [])
		_append("[Lead] Page %d: %d row(s)" % [page_index, rankings.size()])
		for entry in rankings:
			var row: Dictionary = entry
			_append("[Lead]   #%d  %s — %d" % [row.get("rank", 0), _display_name(row), _primary_score(row)])
		next_position += rankings.size()
		if rankings.is_empty() or next_position > total:
			break
		var next_page: PlayFabResult = await PlayFab.leaderboards.get_leaderboard_async(
				user, LEADERBOARD_NAME, next_position, PAGE_SIZE, version)
		if not next_page.ok:
			_append("[color=orange][Lead] page %d failed: %s[/color]" % [page_index + 1, next_page.message])
			return
		page = next_page.data
		page_index += 1

func _print_around_user() -> void:
	var user: PlayFabUser = _auth.get("playfab_user")
	if user == null:
		return
	var result: PlayFabResult = await PlayFab.leaderboards.get_leaderboard_around_user_async(
			user, LEADERBOARD_NAME, 3)
	if not result.ok:
		_append("[color=orange][Lead] around_user failed: %s[/color]" % result.message)
		return
	var page: Dictionary = result.data
	var rankings: Array = page.get("rankings", [])
	_append("[Lead] Around-user: %d row(s) centered on you" % rankings.size())
	var my_id: String = user.entity_key.get("id", "")
	for entry in rankings:
		var row: Dictionary = entry
		var entity: Dictionary = row.get("entity", {})
		var marker := " (you)" if entity.get("id", "") == my_id else ""
		_append("[Lead]   #%d  %s — %d%s" % [
				row.get("rank", 0),
				_display_name(row),
				_primary_score(row),
				marker])

func _print_xbox_friend_leaderboard() -> void:
	var user: PlayFabUser = _auth.get("playfab_user")
	if user == null:
		return
	var result: PlayFabResult = await PlayFab.leaderboards.get_friend_leaderboard_async(
			user, LEADERBOARD_NAME, true)
	if not result.ok:
		_append("[color=orange][Lead] friend leaderboard failed: %s[/color]" % result.message)
		return
	var page: Dictionary = result.data
	var rankings: Array = page.get("rankings", [])
	_append("[Lead] Xbox-friend leaderboard: %d row(s)" % rankings.size())
	for entry in rankings:
		var row: Dictionary = entry
		_append("[Lead]   #%d  %s — %d" % [row.get("rank", 0), _display_name(row), _primary_score(row)])

func _display_name(row: Dictionary) -> String:
	var entry_name: String = row.get("display_name", "")
	if not entry_name.is_empty():
		return entry_name
	var entity: Dictionary = row.get("entity", {})
	return entity.get("id", "?")

func _primary_score(row: Dictionary) -> int:
	var scores: PackedStringArray = row.get("scores", PackedStringArray())
	return scores[0].to_int() if not scores.is_empty() else 0

func _set_buttons_enabled(enabled: bool) -> void:
	_submit_btn.disabled = not enabled
	_top_btn.disabled = not enabled
	_pages_btn.disabled = not enabled
	_around_btn.disabled = not enabled
	_friend_btn.disabled = not enabled

func _append(line: String) -> void:
	_log.append_text(line + "\n")
	print(line)

func _on_back_pressed() -> void:
	get_tree().change_scene_to_file("res://shared/tutorial_picker.tscn")
