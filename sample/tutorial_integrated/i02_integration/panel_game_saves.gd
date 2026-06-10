extends VBoxContainer

const AddonApi = preload("res://shared/addon_api.gd")

## Tutorial 8 Step 4 — Game Saves panel.
##
## Adds the local PlayFab user to Game Saves, writes a timestamped blob
## via FileAccess into the resolved folder, uploads with UI sync, and
## reads back on demand. Item 4 / B4 — the "Resolve conflict…" button
## surfaces the three ADD_USER_OPTION_* recovery strategies as an
## explicit chooser instead of a hard-wired single rollback.
##
## Item 5 / B5 (signal hygiene): re-driven by Auth.state_changed so a
## sign-in retry after a transient failure still wires the panel up.
##
## Source: docs/tutorials/08-integration-tech-demo.md Step 4

const SAVE_FILE := "progress.dat"
const ACTION_KEEP_CLOUD := "keep_cloud"
const ACTION_LAST_KNOWN_GOOD := "last_known_good"
const ACTION_LAST_CONFLICT := "last_conflict"

@onready var _status: Label = $Status
@onready var _last_read: Label = $LastRead
@onready var _write: Button = $Write
@onready var _read: Button = $Read
@onready var _resolve: Button = $Resolve
@onready var _resolve_dialog: AcceptDialog = $ResolveDialog

var _auth: Node = null
var _save_folder: String = ""
var _initialized: bool = false

func _ready() -> void:
	# Static dialog configuration — no sign-in required for the local
	# AcceptDialog to declare its custom buttons.
	_resolve_dialog.add_button("Keep cloud version", true, ACTION_KEEP_CLOUD)
	_resolve_dialog.add_button("Roll back to last known good", true, ACTION_LAST_KNOWN_GOOD)
	_resolve_dialog.add_button("Roll back to last conflict", true, ACTION_LAST_CONFLICT)

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

	var user = _auth.get("playfab_user")
	var result = await AddonApi.singleton("PlayFab").game_saves.add_user_with_ui_async(user)
	if not is_inside_tree():
		return
	if not result.ok:
		_status.text = "Add user failed: %s" % result.message
		push_warning("[Gs] add_user failed: %s" % result.message)
		return
	_save_folder = String(result.data.get("folder", ""))
	print("[Gs] user folder resolved: %s" % _save_folder)
	_status.text = "Folder: %s" % _save_folder

	_write.pressed.connect(_on_write_pressed)
	_read.pressed.connect(_on_read_pressed)
	_resolve.pressed.connect(_on_resolve_pressed)
	_resolve_dialog.custom_action.connect(_on_resolve_action)

func _on_write_pressed() -> void:
	if _save_folder.is_empty():
		return
	var path: String = "%s/%s" % [_save_folder, SAVE_FILE]
	var xbox = _auth.get("xbox_user")
	var gamertag: String = xbox.gamertag if xbox != null else "(unknown)"
	var payload: String = "saved=%s timestamp=%s" % [
			gamertag, Time.get_datetime_string_from_system()]
	var f := FileAccess.open(path, FileAccess.WRITE)
	if f == null:
		_status.text = "Write open failed: %s" % str(FileAccess.get_open_error())
		return
	f.store_string(payload)
	f.close()
	var bytes: int = payload.length()
	var user = _auth.get("playfab_user")
	var upload = await AddonApi.singleton("PlayFab").game_saves.upload_with_ui_async(user, false)
	if not is_inside_tree():
		return
	if upload.ok:
		_status.text = "Wrote %s (%d bytes), upload synced" % [SAVE_FILE, bytes]
		print("[Gs] Wrote %s (%d bytes), upload synced" % [SAVE_FILE, bytes])
	else:
		_status.text = "Wrote locally, upload failed: %s" % upload.message
		push_warning("[Gs] upload failed: %s" % upload.message)

func _on_read_pressed() -> void:
	if _save_folder.is_empty():
		return
	var path: String = "%s/%s" % [_save_folder, SAVE_FILE]
	var f := FileAccess.open(path, FileAccess.READ)
	if f == null:
		_last_read.text = "Open failed: %s" % str(FileAccess.get_open_error())
		return
	_last_read.text = f.get_as_text()
	f.close()

func _on_resolve_pressed() -> void:
	if _save_folder.is_empty():
		_status.text = "Resolve unavailable — folder not yet resolved"
		return
	_resolve_dialog.popup_centered()

func _on_resolve_action(action: StringName) -> void:
	_resolve_dialog.hide()
	var option: int = AddonApi.constant("PlayFabGameSaves", "ADD_USER_OPTION_NONE")
	var label := ""
	match str(action):
		ACTION_KEEP_CLOUD:
			option = AddonApi.constant("PlayFabGameSaves", "ADD_USER_OPTION_NONE")
			label = "keep cloud version"
		ACTION_LAST_KNOWN_GOOD:
			option = AddonApi.constant("PlayFabGameSaves", "ADD_USER_OPTION_ROLLBACK_TO_LAST_KNOWN_GOOD")
			label = "rolled back to last known good"
		ACTION_LAST_CONFLICT:
			option = AddonApi.constant("PlayFabGameSaves", "ADD_USER_OPTION_ROLLBACK_TO_LAST_CONFLICT")
			label = "rolled back to last conflict"
		_:
			return
	var user = _auth.get("playfab_user")
	if user == null:
		return
	var result = await AddonApi.singleton("PlayFab").game_saves.add_user_with_ui_async(user, option)
	if not is_inside_tree():
		return
	if not result.ok:
		_status.text = "Resolution failed (%s): %s" % [label, result.message]
		push_warning("[Gs] resolution failed: %s" % result.message)
		return
	# Folder may change across rollback strategies; refresh the cache.
	_save_folder = String(result.data.get("folder", _save_folder))
	_status.text = "Resolved — %s. Folder: %s" % [label, _save_folder]
	print("[Gs] resolution complete — %s" % label)
