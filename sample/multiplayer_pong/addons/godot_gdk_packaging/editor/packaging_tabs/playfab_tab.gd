@tool
extends ScrollContainer

const PLAYFAB_TITLE_ID_SETTING := "playfab/runtime/title_id"
const PLAYFAB_ENDPOINT_SETTING := "playfab/runtime/endpoint"

var _coordinator: Variant

var title_id_edit: LineEdit
var endpoint_edit: LineEdit
var status_label: Label
var version_label: Label


func setup(coordinator: Variant) -> void:
	_coordinator = coordinator
	horizontal_scroll_mode = ScrollContainer.SCROLL_MODE_DISABLED
	size_flags_vertical = SIZE_EXPAND_FILL

	var root: VBoxContainer = VBoxContainer.new()
	root.size_flags_horizontal = SIZE_EXPAND_FILL
	add_child(root)

	var desc: Label = Label.new()
	desc.text = "Configure PlayFab project settings for runtime sign-in and leaderboard requests.\nLeave the endpoint blank to use the default endpoint derived from the Title ID."
	desc.autowrap_mode = TextServer.AUTOWRAP_WORD_SMART
	desc.add_theme_font_size_override("font_size", 12)
	desc.add_theme_color_override("font_color", Color(0.7, 0.7, 0.7))
	root.add_child(desc)

	root.add_child(HSeparator.new())

	var title_row: HBoxContainer = HBoxContainer.new()
	root.add_child(title_row)

	var title_label: Label = Label.new()
	title_label.text = "PlayFab Title ID"
	title_label.custom_minimum_size.x = 130
	title_label.tooltip_text = "Your PlayFab Title ID from Game Manager → Settings → API Keys. Used at runtime to initialize the PlayFab SDK."
	title_row.add_child(title_label)

	title_id_edit = LineEdit.new()
	title_id_edit.placeholder_text = "e.g. A1B2C"
	title_id_edit.size_flags_horizontal = SIZE_EXPAND_FILL
	title_row.add_child(title_id_edit)

	var endpoint_row: HBoxContainer = HBoxContainer.new()
	root.add_child(endpoint_row)

	var endpoint_label: Label = Label.new()
	endpoint_label.text = "PlayFab Endpoint"
	endpoint_label.custom_minimum_size.x = 130
	endpoint_label.tooltip_text = "Optional endpoint override. Leave blank to use https://<titleid>.playfabapi.com."
	endpoint_row.add_child(endpoint_label)

	endpoint_edit = LineEdit.new()
	endpoint_edit.placeholder_text = "Optional override"
	endpoint_edit.size_flags_horizontal = SIZE_EXPAND_FILL
	endpoint_row.add_child(endpoint_edit)

	var btn_row: HBoxContainer = HBoxContainer.new()
	root.add_child(btn_row)

	var save_btn: Button = Button.new()
	save_btn.text = "Save"
	save_btn.pressed.connect(_on_save)
	btn_row.add_child(save_btn)

	var manager_btn: Button = Button.new()
	manager_btn.text = "Open Game Manager"
	manager_btn.tooltip_text = "Open the PlayFab Game Manager in your browser"
	manager_btn.pressed.connect(func() -> void: OS.shell_open("https://developer.playfab.com/en-us/r/sign-in"))
	btn_row.add_child(manager_btn)

	status_label = Label.new()
	status_label.text = ""
	root.add_child(status_label)

	root.add_child(HSeparator.new())

	version_label = Label.new()
	version_label.text = "PlayFab SDK: detecting..."
	version_label.add_theme_font_size_override("font_size", 12)
	version_label.add_theme_color_override("font_color", Color(0.7, 0.7, 0.7))
	root.add_child(version_label)

	_detect_version()


func load_config() -> void:
	title_id_edit.text = str(ProjectSettings.get_setting(PLAYFAB_TITLE_ID_SETTING, ""))
	endpoint_edit.text = str(ProjectSettings.get_setting(PLAYFAB_ENDPOINT_SETTING, ""))
	if title_id_edit.text.strip_edges() != "" or endpoint_edit.text.strip_edges() != "":
		status_label.text = "Loaded from project.godot"
	else:
		status_label.text = "No PlayFab settings saved yet — enter values and save."


func _on_save() -> void:
	ProjectSettings.set_setting(PLAYFAB_TITLE_ID_SETTING, title_id_edit.text.strip_edges())
	ProjectSettings.set_setting(PLAYFAB_ENDPOINT_SETTING, endpoint_edit.text.strip_edges())
	var err: Error = ProjectSettings.save()
	if err == OK:
		status_label.text = "✅ Saved to project.godot"
		_coordinator._log("PlayFab settings saved")
		var fs: EditorFileSystem = EditorInterface.get_resource_filesystem()
		if not fs.is_scanning():
			fs.scan()
	else:
		status_label.text = "Failed to save: " + error_string(err)
		push_error("[GDK] Failed to save PlayFab config: " + error_string(err))


func _detect_version() -> void:
	var search_paths: Array = [
		"res://addons/godot_playfab/bin/PlayFabCore.dll",
	]
	var dll_path: String = ""
	for path: String in search_paths:
		var global_path: String = ProjectSettings.globalize_path(path)
		if FileAccess.file_exists(global_path):
			dll_path = global_path
			break

	if dll_path == "":
		version_label.text = "PlayFab SDK: not found"
		return

	var output: Array = []
	var ps_cmd: String = "(Get-Item '%s').VersionInfo.ProductVersion" % dll_path.replace("'", "''")
	var exit_code: int = OS.execute(
		"powershell",
		PackedStringArray(["-NoProfile", "-Command", ps_cmd]),
		output,
		true,
		false
	)
	if exit_code == 0 and output.size() > 0:
		var version: String = str(output[0]).strip_edges()
		if version != "":
			version_label.text = "PlayFab SDK: %s" % version
			return

	version_label.text = "PlayFab SDK: installed (version unknown)"
