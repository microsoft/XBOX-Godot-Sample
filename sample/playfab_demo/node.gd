extends Node

const PLAYFAB_TITLE_ID_SETTING := "playfab/titleid"

@onready var start_button: Button = $Button
@onready var status_label: Label = $StatusLabel

func _run_playfab() -> void:
	start_button.disabled = true

	var playfab = _get_playfab()
	if playfab == null:
		_set_status("PlayFab extension not loaded.", false)
		return

	var gdk = _get_gdk()
	if gdk == null:
		_set_status("GDK extension not loaded.", false)
		return

	if not gdk.is_initialized():
		_set_status("GDK is still initializing.", false)
		return

	var title_id := str(ProjectSettings.get_setting(PLAYFAB_TITLE_ID_SETTING, "")).strip_edges()
	if title_id == "":
		_set_status("Set Project Settings > playfab/titleid before running the PlayFab demo.", false)
		return

	var xbox_user = gdk.users.get_primary_user()
	if xbox_user == null or not xbox_user.signed_in:
		_set_status("Signing in to Xbox...", true)
		var xbox_result = await _await_async_result(gdk.users.add_user_with_ui_async())
		if xbox_result == null or not xbox_result.ok:
			_set_status(xbox_result.message if xbox_result != null else "Xbox sign-in failed.", false)
			return
		xbox_user = gdk.users.get_primary_user()

	if not playfab.is_initialized():
		var init_result = playfab.initialize()
		if init_result == null or not init_result.ok:
			_set_status(init_result.message if init_result != null else "PlayFab initialization failed.", false)
			return

	_set_status("Connecting PlayFab...", true)
	var sign_in_result = await _await_async_result(playfab.sign_in_with_xuser_async(xbox_user))
	if sign_in_result == null or not sign_in_result.ok:
		_set_status(sign_in_result.message if sign_in_result != null else "PlayFab sign-in failed.", false)
		return

	var playfab_user = sign_in_result.data
	var entity_key: Dictionary = playfab_user.entity_key if playfab_user != null else {}
	_set_status("PlayFab ready: %s:%s" % [str(entity_key.get("type", "")), str(entity_key.get("id", ""))], true)


func _await_async_result(op):
	return await op


func _set_status(text: String, is_ok: bool) -> void:
	status_label.text = text
	status_label.modulate = Color(0.3, 1.0, 0.3) if is_ok else Color(1.0, 0.5, 0.4)
	start_button.disabled = false


func _get_gdk():
	var bootstrap = get_node_or_null("/root/GDKBootstrap")
	if bootstrap != null and bootstrap.has_method("get_gdk"):
		return bootstrap.get_gdk()
	if Engine.has_singleton("GDK"):
		return Engine.get_singleton("GDK")
	return null


func _get_playfab():
	if Engine.has_singleton("PlayFab"):
		return Engine.get_singleton("PlayFab")
	return null
