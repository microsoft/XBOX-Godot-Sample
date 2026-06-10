extends Control

## Tutorial picker — default scene for the tutorial app.
##
## Plain Control with one button per tutorial. Pressing a button
## changes the current scene to that tutorial's reference end-state
## scene. Disables buttons while sign-in is in-flight so first-time
## opens don't race the Auth autoload's silent sign-in.

const TUTORIALS := [
	{ "label": "I1 — Sign in (Xbox → PlayFab)",   "scene": "res://i01_signin.tscn",                      "needs_auth": false },
	{ "label": "I2 — Integration tech demo",      "scene": "res://i02_integration/i02_integration.tscn", "needs_auth": true },
]

@onready var _status: Label = $Root/Status
@onready var _problems: RichTextLabel = $Root/Problems
@onready var _buttons: VBoxContainer = $Root/Buttons

# `Auth` is a user-defined GDScript autoload registered in project.godot.
# Use `/root/Auth` lookup so the parse gate (which doesn't resolve autoloads)
# stays clean.
var _auth: Node = null

func _ready() -> void:
	_auth = get_node_or_null("/root/Auth")
	_populate_buttons()
	_status.text = "Signing in…"
	_set_signin_gated(true)

	# Pre-flight the per-developer config so the picker shows actionable
	# errors instead of a vague "Sign-in failed (gdk.initialize)" line.
	# Sign-in is gated on whatever's broken until the user fixes it and
	# relaunches.
	var problems := _detect_config_problems()
	if not problems.is_empty():
		_show_config_problems(problems)
		if _has_problem_of_kind(problems, "game_config"):
			# GDK initialize will fail without a real MicrosoftGame.config —
			# even T1 can't run, so lock everything down.
			_set_all_gated(true)
		return

	if _auth == null:
		_status.text = "Auth autoload missing — register autoload/auth.gd in project.godot."
		_set_signin_gated(true)
		return

	# Drain any session a prior tutorial scene left active. The Lobby
	# and Party autoloads persist across scene changes; without this,
	# the user returns to the picker still IN_LOBBY / IN_NETWORK and
	# the next tutorial's host/join is silently rejected with "already
	# in a lobby — leave first". Gate every button during the drain so
	# clicks can't race the cleanup into a tutorial that would re-trip
	# the same orphan state.
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

	# 1. PlayFab title id — set in Project Settings (mirrored from
	#    sample_config.cfg by tools\setup_sample.ps1). The addon falls
	#    back to "" when unset, which causes PlayFab.initialize to fail
	#    with a noisy error during T1's playfab.sign_in_with_xuser step.
	var pf_title_raw: Variant = ProjectSettings.get_setting(
		"playfab/runtime/title_id", "")
	var pf_title := str(pf_title_raw).strip_edges()
	if pf_title.is_empty():
		problems.append({
			"kind": "pf_title",
			"title": "PlayFab title id is not set.",
			"fix": "Open [b]Project → Project Settings → PlayFab → Runtime → Title Id[/b], paste your PlayFab title id (3–5 hex chars from PlayFab Game Manager), then relaunch this project. The repo script [code]tools\\setup_sample.ps1[/code] can write this for you.",
		})

	# 2. MicrosoftGame.config — either missing entirely or still has
	#    template placeholder values. GDK initialize loads this file at
	#    startup; with FFFFFFFF / 9NXX / 00000000 placeholders, sign-in
	#    fails with an opaque GDK error.
	var config_path := "res://MicrosoftGame.config"
	if not FileAccess.file_exists(config_path):
		problems.append({
			"kind": "game_config",
			"title": "MicrosoftGame.config is missing.",
			"fix": "Copy [code]MicrosoftGame.config.template[/code] to [code]MicrosoftGame.config[/code] in this folder and fill in your Partner Center values, OR run [code]pwsh -File tools\\setup_sample.ps1[/code] from the repo root. See [code]docs/addon-getting-started.md[/code].",
		})
	else:
		var stale := _game_config_placeholder_summary(config_path)
		if not stale.is_empty():
			problems.append({
				"kind": "game_config",
				"title": "MicrosoftGame.config still has placeholder values (%s)." % stale,
				"fix": "Edit [code]MicrosoftGame.config[/code] and replace the placeholder TitleId / StoreId / MSAAppId / Publisher values with the ones from Partner Center, OR run [code]pwsh -File tools\\setup_sample.ps1[/code] from the repo root.",
			})

	return problems

func _game_config_placeholder_summary(config_path: String) -> String:
	var file := FileAccess.open(config_path, FileAccess.READ)
	if file == null:
		return ""
	var text := file.get_as_text()
	file.close()

	var stale_fields: Array = []
	if text.contains("FFFFFFFF"):
		stale_fields.append("TitleId")
	if text.contains("9NXXXXXXXXXX"):
		stale_fields.append("StoreId")
	if text.contains("00000000-0000-0000-0000-000000000000"):
		stale_fields.append("MSAAppId")
	if text.contains("XXXXXXXX-XXXX-XXXX-XXXX-XXXXXXXXXXXX"):
		stale_fields.append("Publisher")
	return ", ".join(stale_fields)

func _has_problem_of_kind(problems: Array, kind: String) -> bool:
	for problem in problems:
		if problem.get("kind", "") == kind:
			return true
	return false

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
		var xbox_user = _auth.get("xbox_user")
		var gamertag := str(xbox_user.gamertag) if xbox_user != null else "(unknown)"
		_status.text = "Signed in as %s" % gamertag
		_set_signin_gated(false)
	elif _auth.call("is_signing_in"):
		_status.text = "Signing in…"
		_set_signin_gated(true)
	elif _auth.call("is_failed"):
		var stage: String = _auth.call("get_last_error_stage")
		var message: String = _auth.call("get_last_error_message")
		_status.text = "Sign-in failed (%s): %s — T1 is still available." % [stage, message]
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
# Party sessions (T7 doesn't require an active Lobby).
func _release_orphaned_sessions() -> void:
	var lobby_node: Node = get_node_or_null("/root/Lobby")
	if lobby_node != null and lobby_node.has_method("is_in_lobby"):
		await _drain_session(lobby_node, &"is_in_lobby", &"leave_lobby", "lobby")

	var party_node: Node = get_node_or_null("/root/Party")
	if party_node != null and party_node.has_method("is_in_network"):
		await _drain_session(party_node, &"is_in_network", &"leave_party", "Party network")

# Waits for the autoload to settle out of HOSTING / JOINING / LEAVING
# (poll every 100ms, cap at 5s), then leaves if it's still holding a live
# session. The poll loop is the timeout guard — without it a hung SDK
# await would leave the picker disabled forever.
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
