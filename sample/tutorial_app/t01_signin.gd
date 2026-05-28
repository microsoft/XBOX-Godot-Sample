extends Control

## Tutorial 1 reference scene — sign-in status panel.
##
## Reads the `Auth` autoload (introduced in T1) and renders the
## current sign-in state via Auth.state_changed. Pressing **Sign in**
## re-runs the check → silent → UI fallback; **Back** returns to the
## picker.
##
## NOTE: scene scripts use `get_node("/root/Auth")` instead of the
## bare `Auth.` reference shown in the tutorial markdown so that the
## headless parse gate (`tools\check_gd_scripts_headless.ps1`) — which
## does not resolve GDScript autoloads — stays clean.

@onready var _identity: Label = $Root/Identity
@onready var _status: Label = $Root/Status
@onready var _sign_in_button: Button = $Root/Buttons/SignIn
@onready var _back_button: Button = $Root/Buttons/Back

var _auth: Node = null

func _ready() -> void:
	_back_button.pressed.connect(_on_back_pressed)
	_sign_in_button.pressed.connect(_on_sign_in_pressed)

	_auth = get_node_or_null("/root/Auth")
	if _auth == null:
		_status.text = "Auth autoload missing — register autoload/auth.gd in project.godot."
		_sign_in_button.disabled = true
		return

	_auth.state_changed.connect(_on_auth_state_changed)
	_refresh()

func _refresh() -> void:
	if _auth == null:
		return
	if _auth.call("is_signed_in"):
		_refresh_identity(_auth.get("xbox_user"), _auth.get("playfab_user"))
		_status.text = "Signed in."
	elif _auth.call("is_signing_in"):
		_refresh_identity(null, null)
		_status.text = "Signing in…"
	elif _auth.call("is_failed"):
		_refresh_identity(null, null)
		_status.text = "Sign-in failed at %s: %s" % [
				_auth.call("get_last_error_stage"),
				_auth.call("get_last_error_message")]
	else:
		_refresh_identity(null, null)
		_status.text = "Not signed in."

func _refresh_identity(xbox_user, playfab_user) -> void:
	if xbox_user == null:
		_identity.text = "Xbox: (not signed in)"
		return
	var gamertag := str(xbox_user.gamertag)
	var xuid := str(xbox_user.xuid)
	var pf_text := ""
	if playfab_user != null:
		var key: Dictionary = playfab_user.entity_key
		pf_text = "\nPlayFab: %s:%s" % [key.get("type", ""), key.get("id", "")]
	_identity.text = "Xbox: %s (%s)%s" % [gamertag, xuid, pf_text]

func _on_auth_state_changed(_state) -> void:
	_refresh()

func _on_sign_in_pressed() -> void:
	if _auth == null:
		return
	# sign_in() is idempotent — if already signed in returns immediately;
	# if signing in joins the in-flight attempt; otherwise starts fresh.
	await _auth.call("sign_in")

func _on_back_pressed() -> void:
	get_tree().change_scene_to_file("res://shared/tutorial_picker.tscn")
