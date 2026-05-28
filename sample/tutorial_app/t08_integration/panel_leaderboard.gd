extends VBoxContainer

## Tutorial 8 Step 3 — Leaderboards panel.
##
## Top-10 + around-user views side by side; submit button drives the
## next score into the statistic via
## PlayFab.statistics.update_statistics_async. The leaderboard then
## ranks the statistic, so the read paths stay unchanged.
##
## Item 5 / B5 (signal hygiene): re-driven by Auth.state_changed so a
## sign-in retry after a transient failure still wires the panel up.
## No external long-lived signals are kept open between awaits, so
## _exit_tree only needs to drop the Auth listener.
##
## Source: docs/tutorials/08-integration-tech-demo.md Step 3

const STATISTIC_NAME := "high_score"
const LEADERBOARD_NAME := "high_score"

@onready var _top10: Label = $Top10
@onready var _around: Label = $AroundUser
@onready var _submit: Button = $SubmitScore
@onready var _refresh: Button = $Refresh
@onready var _status: Label = $Status

var _auth: Node = null
var _scratch_score: int = 100
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
	await _auth.sign_in()
	if is_inside_tree() and _auth.is_signed_in():
		_initialize_after_sign_in()

func _exit_tree() -> void:
	if _auth != null and _auth.state_changed.is_connected(_on_auth_state_changed):
		_auth.state_changed.disconnect(_on_auth_state_changed)

func _on_auth_state_changed(_state) -> void:
	if _initialized or _auth == null:
		return
	if _auth.is_signed_in():
		_initialize_after_sign_in()

func _initialize_after_sign_in() -> void:
	if _initialized:
		return
	_initialized = true
	_submit.pressed.connect(_on_submit_pressed)
	_refresh.pressed.connect(_refresh_views)
	await _refresh_views()

func _on_submit_pressed() -> void:
	_scratch_score += 10
	var user: PlayFabUser = _auth.get("playfab_user")
	var result: PlayFabResult = await PlayFab.statistics.update_statistics_async(user, {
		"statistics": [
			{"name": STATISTIC_NAME, "scores": [str(_scratch_score)]},
		],
	})
	if not is_inside_tree():
		return
	if result.ok:
		_status.text = "Recorded %d to %s" % [_scratch_score, STATISTIC_NAME]
		print("[Lb] Recorded %d to %s" % [_scratch_score, STATISTIC_NAME])
	else:
		_status.text = "Record failed: %s" % result.message
		return
	await _refresh_views()

func _refresh_views() -> void:
	var user: PlayFabUser = _auth.get("playfab_user")
	var top: PlayFabResult = await PlayFab.leaderboards.get_leaderboard_async(
			user, LEADERBOARD_NAME, 1, 10)
	if not is_inside_tree():
		return
	if top.ok:
		var rankings: Array = top.data.get("rankings", [])
		_top10.text = _render(rankings)
		if not rankings.is_empty():
			var first: Dictionary = rankings[0]
			print("[Lb] Top-10 refresh: 1. %s %d" % [
					_display_name(first), _primary_score(first)])
		else:
			print("[Lb] Top-10 refresh: (empty)")
	else:
		_top10.text = "Top-10 failed: %s" % top.message

	var around: PlayFabResult = await PlayFab.leaderboards.get_leaderboard_around_user_async(
			user, LEADERBOARD_NAME, 3)
	if not is_inside_tree():
		return
	if around.ok:
		_around.text = _render(around.data.get("rankings", []))
	else:
		_around.text = "Around-user failed: %s" % around.message

func _render(rankings: Array) -> String:
	var lines := PackedStringArray()
	for entry in rankings:
		var row: Dictionary = entry
		lines.append("%d. %s — %d" % [row.get("rank", 0), _display_name(row), _primary_score(row)])
	if lines.is_empty():
		return "(no entries)"
	return "\n".join(lines)

func _display_name(row: Dictionary) -> String:
	var disp: String = row.get("display_name", "")
	if not disp.is_empty():
		return disp
	var entity: Dictionary = row.get("entity", {})
	return entity.get("id", "?")

func _primary_score(row: Dictionary) -> int:
	var scores: PackedStringArray = row.get("scores", PackedStringArray())
	return scores[0].to_int() if not scores.is_empty() else 0
