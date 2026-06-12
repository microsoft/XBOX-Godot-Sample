extends Control

## Tutorial picker — default scene for the PlayFab-only tutorial track.
##
## Plain Control with one button per tutorial. Pressing a button changes
## the current scene to that tutorial's reference end-state scene. Disables
## auth-gated buttons while sign-in is in-flight so first-time opens don't
## race the PlayFabAuth autoload's sign-in.
##
## This is the PlayFab-only track: it signs into PlayFab via PlayFabAuth
## (a custom-id session) and never touches Xbox/GDK, so the config
## pre-flight only checks the PlayFab title id.

const TUTORIALS := [
	{ "label": "P1 — Sign in (PlayFab custom id)", "scene": "res://p01_signin.tscn",     "needs_auth": false },
	{ "label": "P2 — PlayFab leaderboard",         "scene": "res://p02_leaderboard.tscn", "needs_auth": true },
	{ "label": "P3 — Multiplayer lobby",           "scene": "res://p03_lobby.tscn",       "needs_auth": true },
	{ "label": "P4 — PlayFab Party",               "scene": "res://p04_party.tscn",       "needs_auth": true },
]

@onready var _status: Label = $Root/Status
@onready var _problems: RichTextLabel = $Root/Problems
@onready var _buttons: VBoxContainer = $Root/Buttons

# `PlayFabAuth` is a user-defined GDScript autoload registered in
# project.godot. Use `/root/PlayFabAuth` lookup so the parse gate (which
# doesn't resolve autoloads) stays clean.
var _auth: Node = null

func _ready() -> void:
	_auth = get_node_or_null("/root/PlayFabAuth")
	_populate_buttons()
	_status.text = "Signing in…"
	_set_signin_gated(true)

	# Pre-flight the per-developer config so the picker shows actionable
	# errors instead of a vague "Sign-in failed (playfab.initialize)" line.
	var problems := _detect_config_problems()
	if not problems.is_empty():
		_show_config_problems(problems)
		# PlayFab initialize will fail without a title id — even P1 can't
		# run, so lock everything down.
		_set_all_gated(true)
		return

	if _auth == null:
		_status.text = "PlayFabAuth autoload missing — register autoload/playfab_auth.gd in project.godot."
		_set_signin_gated(true)
		return

	# Drain any session a prior tutorial scene left active. The Lobby and
	# Party autoloads persist across scene changes; without this, the user
	# returns to the picker still IN_LOBBY / IN_NETWORK and the next
	# tutorial's host/join is silently rejected with "already in a lobby".
	_set_all_gated(true)
	await _release_orphaned_sessions()
	_set_all_gated(false)
	_set_signin_gated(true)

	_auth.connect("state_changed", _on_auth_state_changed)

	# If sign-in already finished before the picker loaded, light up immediately.
	_on_auth_state_changed(_auth.call("get_state"))

func _populate_buttons() -> void:
	for entry in TUTORIALS:
		var button := Button.new()
		button.text = entry.label
		button.pressed.connect(_on_button_pressed.bind(entry.scene))
		_buttons.add_child(button)

func _set_signin_gated(gated: bool) -> void:
	for i in range(TUTORIALS.size()):
		var entry: Dictionary = TUTORIALS[i]
		if not entry.needs_auth:
			continue
		var button: Button = _buttons.get_child(i)
		button.disabled = gated

func _set_all_gated(gated: bool) -> void:
	for i in range(TUTORIALS.size()):
		var button: Button = _buttons.get_child(i)
		button.disabled = gated

func _detect_config_problems() -> Array:
	var problems: Array = []

	# PlayFab title id — set in Project Settings (mirrored from
	# sample_config.cfg by tools\setup_sample.ps1). The addon falls back to
	# "" when unset, which causes PlayFab.initialize to fail during P1.
	var pf_title_raw: Variant = ProjectSettings.get_setting(
		"playfab/runtime/title_id", "")
	var pf_title := str(pf_title_raw).strip_edges()
	if pf_title.is_empty():
		problems.append({
			"kind": "pf_title",
			"title": "PlayFab title id is not set.",
			"fix": "Open [b]Project → Project Settings → PlayFab → Runtime → Title Id[/b], paste your PlayFab title id (3–5 hex chars from PlayFab Game Manager), then relaunch this project. The repo script [code]tools\\setup_sample.ps1[/code] can write this for you.",
		})

	return problems

func _show_config_problems(problems: Array) -> void:
	_status.text = "Configuration problems detected — fix the items below and relaunch."

	var lines: Array = []
	for problem in problems:
		lines.append("[color=#ff8080][b]• %s[/b][/color]" % problem.title)
		lines.append("    %s" % problem.fix)
		lines.append("")
	_problems.text = "\n".join(lines)
	_problems.visible = true

func _on_auth_state_changed(_state) -> void:
	if _auth.call("is_signed_in"):
		var pf_user = _auth.get("playfab_user")
		var id := "(unknown)"
		if pf_user != null:
			var key: Dictionary = pf_user.entity_key
			id = "%s:%s" % [key.get("type", ""), key.get("id", "")]
		_status.text = "Signed in as '%s' (%s)" % [_auth.call("get_custom_id"), id]
		_set_signin_gated(false)
	elif _auth.call("is_signing_in"):
		_status.text = "Signing in…"
		_set_signin_gated(true)
	elif _auth.call("is_failed"):
		var stage: String = _auth.call("get_last_error_stage")
		var message: String = _auth.call("get_last_error_message")
		_status.text = "Sign-in failed (%s): %s — P1 is still available." % [stage, message]
		_set_signin_gated(true)
	else:
		_status.text = "Not signed in."
		_set_signin_gated(true)

func _on_button_pressed(scene_path: String) -> void:
	get_tree().change_scene_to_file(scene_path)

# Drains any persistent multiplayer session left by a prior tutorial scene
# that backed out without calling Leave. Lobby is drained first so its
# `lobby_left` signal cascades into Party's auto-leave handler; Party is
# then drained explicitly to wait out that cascade and to catch standalone
# Party sessions.
func _release_orphaned_sessions() -> void:
	var lobby_node: Node = get_node_or_null("/root/Lobby")
	if lobby_node != null and lobby_node.has_method("is_in_lobby"):
		await _drain_session(lobby_node, &"is_in_lobby", &"leave_lobby", "lobby")

	var party_node: Node = get_node_or_null("/root/Party")
	if party_node != null and party_node.has_method("is_in_network"):
		await _drain_session(party_node, &"is_in_network", &"leave_party", "Party network")

# Waits for the autoload to settle out of HOSTING / JOINING / LEAVING
# (poll every 100ms, cap at 5s), then leaves if it's still holding a live
# session.
func _drain_session(autoload: Node, in_session_check: StringName, leave_func: StringName, label: String) -> void:
	if autoload.has_method("is_busy") and autoload.call("is_busy"):
		_status.text = "Waiting for previous %s op to finish…" % label
		var deadline := Time.get_ticks_msec() + 5000
		while autoload.call("is_busy") and Time.get_ticks_msec() < deadline:
			await get_tree().create_timer(0.1).timeout
		if autoload.call("is_busy"):
			push_warning("[Picker] Previous %s op did not settle within 5s; skipping cleanup" % label)
			return
	if autoload.call(in_session_check):
		_status.text = "Cleaning up previous %s…" % label
		await autoload.call(leave_func)
