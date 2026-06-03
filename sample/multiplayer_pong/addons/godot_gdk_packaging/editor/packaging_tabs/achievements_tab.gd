@tool
extends ScrollContainer

const SAMPLE_CONFIG_PATH := "res://sample_config.cfg"

var _coordinator

var achievement_id_edit: LineEdit
var achievement_status_label: Label


func setup(coordinator) -> void:
	_coordinator = coordinator
	horizontal_scroll_mode = ScrollContainer.SCROLL_MODE_DISABLED
	size_flags_vertical = SIZE_EXPAND_FILL

	var root := VBoxContainer.new()
	root.size_flags_horizontal = SIZE_EXPAND_FILL
	add_child(root)

	var ach_row := HBoxContainer.new()
	root.add_child(ach_row)

	var ach_label := Label.new()
	ach_label.text = "Demo Achievement ID"
	ach_label.custom_minimum_size.x = 130
	ach_row.add_child(ach_label)

	achievement_id_edit = LineEdit.new()
	achievement_id_edit.placeholder_text = "Achievement ID to test (e.g. 1)"
	achievement_id_edit.size_flags_horizontal = SIZE_EXPAND_FILL
	ach_row.add_child(achievement_id_edit)

	var ach_btn_row := HBoxContainer.new()
	root.add_child(ach_btn_row)

	var achievement_save_btn := Button.new()
	achievement_save_btn.text = "Save"
	achievement_save_btn.pressed.connect(_on_save)
	ach_btn_row.add_child(achievement_save_btn)

	achievement_status_label = Label.new()
	achievement_status_label.text = ""
	root.add_child(achievement_status_label)


func load_config() -> void:
	var cfg := ConfigFile.new()
	if cfg.load(SAMPLE_CONFIG_PATH) == OK:
		var val = cfg.get_value("achievements", "demo_achievement_id", "")
		achievement_id_edit.text = str(val)
		achievement_status_label.text = "Loaded from sample_config.cfg"
	else:
		achievement_status_label.text = "No sample_config.cfg — enter a value and save."


func _on_save() -> void:
	var cfg := ConfigFile.new()
	cfg.load(SAMPLE_CONFIG_PATH)
	cfg.set_value("achievements", "demo_achievement_id", achievement_id_edit.text.strip_edges())
	var err = cfg.save(SAMPLE_CONFIG_PATH)
	if err == OK:
		achievement_status_label.text = "✅ Saved to sample_config.cfg"
		_coordinator._log("Achievement config saved")
		var fs = EditorInterface.get_resource_filesystem()
		if not fs.is_scanning():
			fs.scan()
	else:
		achievement_status_label.text = "Failed to save: " + error_string(err)
		push_error("[GDK] Failed to save achievement config: " + error_string(err))
