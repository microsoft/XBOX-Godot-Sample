extends Control

## Tutorial picker — default scene for the GDK-only tutorial track.
##
## Plain Control with one button per tutorial. Pressing a button changes
## the current scene to that tutorial's reference end-state scene. Disables
## auth-gated buttons while sign-in is in-flight so first-time opens don't
## race the GdkAuth autoload's silent sign-in.
##
## This is the GDK-only track: it signs into Xbox via GdkAuth and never
## touches PlayFab, so the config pre-flight only checks MicrosoftGame.config.

const TUTORIALS := [
	{ "label": "G1 — Sign in (Xbox)",            "scene": "res://g01_signin.tscn",        "needs_auth": false },
	{ "label": "G2 — Unlock an achievement",     "scene": "res://g02_achievement.tscn",   "needs_auth": true },
	{ "label": "G3 — Title storage & stats",     "scene": "res://g03_storage_stats.tscn", "needs_auth": true },
	{ "label": "G4 — Multiplayer Activity",      "scene": "res://g04_mpa.tscn",           "needs_auth": true },
]

@onready var _status: Label = $Root/Status
@onready var _problems: RichTextLabel = $Root/Problems
@onready var _buttons: VBoxContainer = $Root/Buttons

# `GdkAuth` is a user-defined GDScript autoload registered in project.godot.
# Use `/root/GdkAuth` lookup so the parse gate (which doesn't resolve
# autoloads) stays clean.
var _auth: Node = null

func _ready() -> void:
	_auth = get_node_or_null("/root/GdkAuth")
	_populate_buttons()
	_status.text = "Signing in…"
	_set_signin_gated(true)

	# Pre-flight the per-developer config so the picker shows actionable
	# errors instead of a vague "Sign-in failed (gdk.initialize)" line.
	var problems := _detect_config_problems()
	if not problems.is_empty():
		_show_config_problems(problems)
		# GDK initialize will fail without a real MicrosoftGame.config —
		# even G1 can't run, so lock everything down.
		_set_all_gated(true)
		return

	if _auth == null:
		_status.text = "GdkAuth autoload missing — register autoload/gdk_auth.gd in project.godot."
		_set_signin_gated(true)
		return

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

	# MicrosoftGame.config — either missing entirely or still has template
	# placeholder values. GDK initialize loads this file at startup; with
	# FFFFFFFF / 9NXX / 00000000 placeholders, sign-in fails with an opaque
	# GDK error.
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
		_status.text = "Sign-in failed (%s): %s — G1 is still available." % [stage, message]
		_set_signin_gated(true)
	else:
		_status.text = "Not signed in."
		_set_signin_gated(true)

func _on_button_pressed(scene_path: String) -> void:
	get_tree().change_scene_to_file(scene_path)
