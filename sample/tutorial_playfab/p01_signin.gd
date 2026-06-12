extends Control

## PlayFab Tutorial 1 reference scene — PlayFab sign-in status panel.
##
## Reads the `PlayFabAuth` autoload and renders the current sign-in state
## via PlayFabAuth.state_changed. Pressing **Sign in** re-runs the custom-id
## sign-in; **Back** returns to the picker. PlayFab-only: no Xbox step.
##
## NOTE: scene scripts use `get_node("/root/PlayFabAuth")` instead of the
## bare `PlayFabAuth.` reference so the headless parse gate stays clean.
##
## Source: docs/tutorials/playfab/01-signin.md

@onready var _identity: Label = $Root/Identity
@onready var _subtitle: Label = $Root/Subtitle
@onready var _status: Label = $Root/Status
@onready var _sign_in_button: Button = $Root/Buttons/SignIn
@onready var _back_button: Button = $Root/Buttons/Back

var _auth: Node = null

func _ready() -> void:
	_back_button.pressed.connect(_on_back_pressed)
	_sign_in_button.pressed.connect(_on_sign_in_pressed)

	_auth = get_node_or_null("/root/PlayFabAuth")
	if _auth == null:
		_status.text = "PlayFabAuth autoload missing — register autoload/playfab_auth.gd in project.godot."
		_sign_in_button.disabled = true
		return

	# Show which per-instance user this copy signs in as. Run several
	# instances with distinct `--pf-user=<name>` arguments (Debug ->
	# Customize Run Instances) to put different users in a lobby/Party.
	var custom_id: String = _auth.call("get_custom_id")
	_subtitle.text = "Signs in as custom id '%s'. Override per instance with --pf-user=<name>." % custom_id

	_auth.state_changed.connect(_on_auth_state_changed)
	_refresh()

func _refresh() -> void:
	if _auth == null:
		return
	if _auth.call("is_signed_in"):
		_refresh_identity(_auth.get("playfab_user"))
		_status.text = "Signed in."
	elif _auth.call("is_signing_in"):
		_refresh_identity(null)
		_status.text = "Signing in…"
	elif _auth.call("is_failed"):
		_refresh_identity(null)
		_status.text = "Sign-in failed at %s: %s" % [
				_auth.call("get_last_error_stage"),
				_auth.call("get_last_error_message")]
	else:
		_refresh_identity(null)
		_status.text = "Not signed in."

func _refresh_identity(playfab_user) -> void:
	if playfab_user == null:
		_identity.text = "PlayFab: (not signed in)"
		return
	var key: Dictionary = playfab_user.entity_key
	_identity.text = "PlayFab: %s:%s" % [key.get("type", ""), key.get("id", "")]

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
