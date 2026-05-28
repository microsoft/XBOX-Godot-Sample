extends Control

## Tutorial 4 reference scene — PlayFab Game Saves.
##
## Buttons drive each tutorial step:
##   - Add user to Game Saves (Step 1)
##   - Write a sample save file (Step 2)
##   - Upload to the cloud (Step 3)
##   - Print folder/quota/connectivity state (Step 4)
##   - Resolve a conflict via the rollback chooser (Step 5)
##
## Source: docs/tutorials/04-game-saves.md

const ACTION_KEEP_CLOUD := "keep_cloud"
const ACTION_LAST_KNOWN_GOOD := "last_known_good"
const ACTION_LAST_CONFLICT := "last_conflict"

@onready var _log: RichTextLabel = $Root/LogPanel/Log
@onready var _add_btn: Button = $Root/Buttons/AddBtn
@onready var _write_btn: Button = $Root/Buttons/WriteBtn
@onready var _upload_btn: Button = $Root/Buttons/UploadBtn
@onready var _state_btn: Button = $Root/Buttons/StateBtn
@onready var _resolve_btn: Button = $Root/Buttons/ResolveBtn
@onready var _back_btn: Button = $Root/Buttons/BackBtn
@onready var _resolve_dialog: AcceptDialog = $ResolveDialog

var _auth: Node = null
var _save_folder: String = ""

func _ready() -> void:
	_back_btn.pressed.connect(_on_back_pressed)
	_add_btn.pressed.connect(func(): await _add_to_game_saves())
	_write_btn.pressed.connect(func(): _write_save(1234))
	_upload_btn.pressed.connect(func(): await _upload("Tutorial 4 — demo save"))
	_state_btn.pressed.connect(_print_cloud_state)
	_resolve_btn.pressed.connect(_on_resolve_pressed)

	# Item 4 / B4 — three explicit recovery strategies. We append them
	# as custom buttons on the AcceptDialog so the user picks one
	# explicitly. The default OK button is repurposed as Cancel.
	_resolve_dialog.add_button("Keep cloud version", true, ACTION_KEEP_CLOUD)
	_resolve_dialog.add_button("Roll back to last known good", true, ACTION_LAST_KNOWN_GOOD)
	_resolve_dialog.add_button("Roll back to last conflict", true, ACTION_LAST_CONFLICT)
	_resolve_dialog.custom_action.connect(_on_resolve_action)

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

func _add_to_game_saves() -> void:
	var user: PlayFabUser = _auth.get("playfab_user")
	if user == null:
		return
	if not user.has_local_user_handle:
		_append("[color=red][Save] PlayFab session is custom-id; Game Saves needs Xbox.[/color]")
		return

	var result: PlayFabResult = await PlayFab.game_saves.add_user_with_ui_async(user)
	if not result.ok:
		_append("[color=orange][Save] Add user failed: %s (%s)[/color]" % [result.message, result.code])
		return

	var data: Dictionary = result.data
	_save_folder = data.get("folder", "")
	var connected: bool = data.get("connected_to_cloud", false)
	var quota: int = data.get("remaining_quota", -1)

	_append("[Save] Game Saves folder: %s" % _save_folder)
	_append("[Save] Cloud connected: %s, quota left: %d bytes" % [str(connected), quota])

func _write_save(highscore: int) -> void:
	if _save_folder.is_empty():
		_append("[color=red][Save] _save_folder not resolved yet — press 'Add user' first.[/color]")
		return

	var path := _save_folder.path_join("progress.json")
	var file := FileAccess.open(path, FileAccess.WRITE)
	if file == null:
		_append("[color=red][Save] Open failed: %s[/color]" % path)
		return

	var payload := {
		"highscore": highscore,
		"saved_at": Time.get_datetime_string_from_system(true),
	}
	file.store_string(JSON.stringify(payload))
	file.close()

	_append("[Save] Wrote save: highscore=%d" % highscore)

func _upload(description: String) -> void:
	var user: PlayFabUser = _auth.get("playfab_user")
	if user == null:
		return

	if not description.is_empty():
		var desc_result: PlayFabResult = await PlayFab.game_saves.set_save_description_async(user, description)
		if not desc_result.ok:
			_append("[color=orange][Save] Description set failed: %s[/color]" % desc_result.message)

	var result: PlayFabResult = await PlayFab.game_saves.upload_with_ui_async(user, false)
	if result.ok:
		_append("[Save] Upload complete")
	else:
		_append("[color=orange][Save] Upload failed: %s (%s)[/color]" % [result.message, result.code])

func _print_cloud_state() -> void:
	var user: PlayFabUser = _auth.get("playfab_user")
	if user == null:
		return

	var connected: PlayFabResult = PlayFab.game_saves.is_connected_to_cloud(user)
	if connected.ok:
		_append("[Save] Cloud connected: %s" % str(connected.data))

	var folder_size: PlayFabResult = PlayFab.game_saves.get_folder_size(user)
	if folder_size.ok:
		_append("[Save] Folder size on disk: %d bytes" % int(folder_size.data))

	var quota: PlayFabResult = PlayFab.game_saves.get_remaining_quota(user)
	if quota.ok:
		_append("[Save] Remaining quota: %d bytes" % int(quota.data))

func _on_resolve_pressed() -> void:
	# Item 4 / B4 — surface the three add-user-with-options strategies
	# as a single chooser instead of a hard-wired "rollback" button.
	# Pre-flight: refuse to open if PlayFab isn't ready or the session
	# isn't Xbox-backed (Game Saves only supports Xbox sign-ins).
	var user: PlayFabUser = _auth.get("playfab_user")
	if user == null or not user.has_local_user_handle:
		_append("[color=red][Save] Resolve unavailable — sign in with an Xbox account first.[/color]")
		return
	_resolve_dialog.popup_centered()

func _on_resolve_action(action: StringName) -> void:
	_resolve_dialog.hide()
	var option: int = PlayFabGameSaves.ADD_USER_OPTION_NONE
	var label := ""
	match str(action):
		ACTION_KEEP_CLOUD:
			option = PlayFabGameSaves.ADD_USER_OPTION_NONE
			label = "keep cloud version"
		ACTION_LAST_KNOWN_GOOD:
			option = PlayFabGameSaves.ADD_USER_OPTION_ROLLBACK_TO_LAST_KNOWN_GOOD
			label = "rolled back to last known good"
		ACTION_LAST_CONFLICT:
			option = PlayFabGameSaves.ADD_USER_OPTION_ROLLBACK_TO_LAST_CONFLICT
			label = "rolled back to last conflict"
		_:
			return
	await _apply_resolution(option, label)

func _apply_resolution(option: int, label: String) -> void:
	var user: PlayFabUser = _auth.get("playfab_user")
	if user == null:
		return
	var result: PlayFabResult = await PlayFab.game_saves.add_user_with_ui_async(user, option)
	if not result.ok:
		_append("[color=orange][Save] Resolution failed (%s): %s[/color]" % [label, result.message])
		return
	# Re-resolve the folder — rollback strategies can change which
	# version of the save tree is on disk, so the local cached path
	# could be stale.
	_save_folder = String(result.data.get("folder", _save_folder))
	_append("[Save] Resolution complete — %s. Folder: %s" % [label, _save_folder])

func _set_buttons_enabled(enabled: bool) -> void:
	_add_btn.disabled = not enabled
	_write_btn.disabled = not enabled
	_upload_btn.disabled = not enabled
	_state_btn.disabled = not enabled
	_resolve_btn.disabled = not enabled

func _append(line: String) -> void:
	_log.append_text(line + "\n")
	print(line)

func _on_back_pressed() -> void:
	get_tree().change_scene_to_file("res://shared/tutorial_picker.tscn")
