extends Control

const AddonApi = preload("res://shared/addon_api.gd")

## Tutorial 8 — Integration tech demo (capstone).
##
## Root wiring layer: HUD strip (identity + runtime-error indicator +
## sign-in retry) and a TabContainer holding one panel per surface
## (Achievements, Leaderboard, Game Saves, Lobby, MPA, Party). Each
## panel has its own script attached to its VBoxContainer.
##
## Service-level error signals from GDK + PlayFab are funneled into
## ErrorLabel so the panels can stay focused on the happy path.
##
## Source: docs/tutorials/08-integration-tech-demo.md

@onready var _identity: Label = $Root/Hud/IdentityLabel
@onready var _error: Label = $Root/Hud/ErrorLabel
@onready var _retry: Button = $Root/Hud/SignInRetry
@onready var _back: Button = $Root/Hud/Back

var _auth: Node = null

func _ready() -> void:
	_auth = get_node_or_null("/root/Auth")

	_identity.text = "Signing in…"
	_error.text = ""
	_retry.pressed.connect(_on_retry_pressed)
	_back.pressed.connect(_on_back_pressed)

	if _auth == null:
		_identity.text = "[ERR] Auth autoload missing"
		return

	_auth.connect("state_changed", _on_auth_state_changed)
	_on_auth_state_changed(_auth.call("get_state"))

	if Engine.has_singleton("GDK"):
		AddonApi.singleton("GDK").runtime_error.connect(_on_runtime_error.bind("gdk"))
		AddonApi.singleton("GDK").achievements.runtime_error.connect(_on_runtime_error.bind("achievements"))
	if Engine.has_singleton("PlayFab"):
		AddonApi.singleton("PlayFab").multiplayer.multiplayer_error.connect(_on_pf_runtime_error.bind("multiplayer"))
		AddonApi.singleton("PlayFab").party.party_error.connect(_on_pf_runtime_error.bind("party"))

	# Make sure sign_in fires for cold T8 entry even without other listeners.
	await _auth.call("sign_in")

func _on_auth_state_changed(_state) -> void:
	if _auth == null:
		return
	if _auth.call("is_signed_in"):
		var xbox = _auth.get("xbox_user")
		var pf = _auth.get("playfab_user")
		if xbox != null and pf != null:
			var entity_id: String = str(pf.entity_key.get("id", ""))
			_identity.text = "%s ↔ PlayFab:%s" % [xbox.gamertag, entity_id.left(8)]
			print("[Hud] identity badge live for %s" % xbox.gamertag)
		_error.text = ""
	elif _auth.call("is_signing_in"):
		_identity.text = "Signing in…"
	elif _auth.call("is_failed"):
		var stage: String = _auth.call("get_last_error_stage")
		var message: String = _auth.call("get_last_error_message")
		_identity.text = "(not signed in)"
		_error.text = "Sign-in failed (%s): %s" % [stage, message]
		push_warning("[Hud] %s" % _error.text)
	else:
		_identity.text = "(not signed in)"

func _on_retry_pressed() -> void:
	_error.text = ""
	if _auth != null:
		await _auth.call("sign_in")

func _on_back_pressed() -> void:
	get_tree().change_scene_to_file("res://shared/tutorial_picker.tscn")

func _on_runtime_error(result, source: String) -> void:
	_error.text = "[%s] %s" % [source, result.message]
	push_warning("[Hud] runtime error from %s: %s" % [source, result.message])

func _on_pf_runtime_error(result, source: String) -> void:
	_error.text = "[%s] %s" % [source, result.message]
	push_warning("[Hud] PlayFab runtime error from %s: %s" % [source, result.message])
