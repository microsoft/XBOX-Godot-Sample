@tool
extends Control
## GDK Setup panel — editor UI for configuring Partner Center values.
## Generates sample_config.cfg and populates export preset fields.

const CONFIG_PATH := "res://sample_config.cfg"
const MICROSOFT_GAME_CONFIG_PATH := "res://MicrosoftGame.config"

var _fields := {}
var _status_label: Label
var _save_button: Button

# Field definitions: [section, key, label, placeholder]
const FIELD_DEFS := [
	["xbox_live", "title_id", "Title ID", "Hex ID from Partner Center (e.g. 6718942c)"],
	["xbox_live", "msa_app_id", "MSA App ID", "GUID from Partner Center"],
	["xbox_live", "store_id", "Store ID", "e.g. 9XXXXXXXXX"],
	["xbox_live", "scid", "SCID Override", "Optional override; runtime derives SCID from Title ID when blank"],
	["xbox_live", "sandbox_id", "Sandbox ID", "e.g. XDKS.1"],
	["identity", "game_name", "Game Name", "Display name for your title"],
	["identity", "publisher", "Publisher CN", "e.g. CN=XXXXXXXX-XXXX-..."],
	["identity", "publisher_display_name", "Publisher Name", "Your studio name"],
	["identity", "version", "Version", "1.0.0.0"],
	["achievements", "demo_achievement_id", "Demo Achievement ID", "Achievement ID to test"],
]

func _ready() -> void:
	_build_ui()
	# Defer config load to ensure the control is fully in the scene tree
	call_deferred("_load_config")

func _build_ui() -> void:
	var scroll := ScrollContainer.new()
	scroll.set_anchors_and_offsets_preset(PRESET_FULL_RECT)
	scroll.horizontal_scroll_mode = ScrollContainer.SCROLL_MODE_DISABLED
	add_child(scroll)

	var vbox := VBoxContainer.new()
	vbox.size_flags_horizontal = SIZE_EXPAND_FILL
	scroll.add_child(vbox)

	# Title
	var title := Label.new()
	title.text = "GDK Configuration"
	title.add_theme_font_size_override("font_size", 18)
	vbox.add_child(title)

	var desc := Label.new()
	desc.text = "Enter your Partner Center values. These are saved to sample_config.cfg\nand used by the sample at runtime and the exporter at build time."
	desc.autowrap_mode = TextServer.AUTOWRAP_WORD_SMART
	vbox.add_child(desc)

	vbox.add_child(HSeparator.new())

	# Build fields grouped by section
	var current_section := ""
	for def in FIELD_DEFS:
		var section: String = def[0]
		var key: String = def[1]
		var label_text: String = def[2]
		var placeholder: String = def[3]

		# Section header
		if section != current_section:
			current_section = section
			var header := Label.new()
			header.text = section.replace("_", " ").capitalize()
			header.add_theme_font_size_override("font_size", 14)
			vbox.add_child(header)

		var hbox := HBoxContainer.new()
		vbox.add_child(hbox)

		var label := Label.new()
		label.text = label_text
		label.custom_minimum_size.x = 160
		hbox.add_child(label)

		var edit := LineEdit.new()
		edit.placeholder_text = placeholder
		edit.size_flags_horizontal = SIZE_EXPAND_FILL
		hbox.add_child(edit)

		_fields[section + "/" + key] = edit

	vbox.add_child(HSeparator.new())

	# Buttons
	var btn_row := HBoxContainer.new()
	vbox.add_child(btn_row)

	_save_button = Button.new()
	_save_button.text = "Save Configuration"
	_save_button.pressed.connect(_on_save_pressed)
	btn_row.add_child(_save_button)

	var apply_button := Button.new()
	apply_button.text = "Apply to Export Preset"
	apply_button.pressed.connect(_on_apply_to_export_pressed)
	btn_row.add_child(apply_button)

	# Status
	_status_label = Label.new()
	_status_label.text = ""
	vbox.add_child(_status_label)

func _normalize_title_id(p_title_id: String) -> String:
	var normalized: String = p_title_id.strip_edges().to_lower()
	if normalized.begins_with("0x"):
		normalized = normalized.substr(2)
	if normalized == "" or normalized.length() > 8:
		return ""

	for i in range(normalized.length()):
		var digit: String = normalized.substr(i, 1)
		if not "0123456789abcdef".contains(digit):
			return ""

	return normalized.lpad(8, "0")

func _validate_title_id_field() -> bool:
	var field_key := "xbox_live/title_id"
	if not _fields.has(field_key):
		return true

	var raw_title_id: String = _fields[field_key].text
	if raw_title_id.strip_edges() == "":
		return true

	var normalized_title_id: String = _normalize_title_id(raw_title_id)
	if normalized_title_id == "":
		_status_label.text = "Title ID must be 1-8 hex characters (optionally prefixed with 0x)."
		return false

	_fields[field_key].text = normalized_title_id
	return true

func _load_config() -> void:
	var cfg := ConfigFile.new()
	var err := cfg.load(CONFIG_PATH)
	if err != OK:
		# Try globalized path as fallback
		var global_path := ProjectSettings.globalize_path(CONFIG_PATH)
		err = cfg.load(global_path)

	if err == OK:
		for def in FIELD_DEFS:
			var section: String = def[0]
			var key: String = def[1]
			var field_key := section + "/" + key
			if field_key in _fields:
				var value = cfg.get_value(section, key, "")
				_fields[field_key].text = str(value)
		_status_label.text = "Loaded from " + CONFIG_PATH
		return

	# Fallback: prepopulate from MicrosoftGame.config when present (standard GDK
	# packaging artifact). Devs bringing an existing title into Godot or copying
	# a config from Partner Center shouldn't have to retype identity/title fields.
	var ms_values := _load_from_microsoft_game_config()
	if ms_values.is_empty():
		_status_label.text = "No config found — fill in values and save."
		return

	for field_key in ms_values:
		if field_key in _fields:
			_fields[field_key].text = str(ms_values[field_key])
	_status_label.text = "Prepopulated from %s — Save to mirror into %s." % [MICROSOFT_GAME_CONFIG_PATH, CONFIG_PATH]

func _load_from_microsoft_game_config() -> Dictionary:
	var values := {}
	if not FileAccess.file_exists(MICROSOFT_GAME_CONFIG_PATH):
		return values

	var parser := XMLParser.new()
	var err := parser.open(MICROSOFT_GAME_CONFIG_PATH)
	if err != OK:
		return values

	while parser.read() == OK:
		if parser.get_node_type() != XMLParser.NODE_ELEMENT:
			continue
		var name := parser.get_node_name()
		match name:
			"Identity":
				var publisher := _xml_attr(parser, "Publisher")
				if publisher != "":
					values["identity/publisher"] = publisher
				var version := _xml_attr(parser, "Version")
				if version != "":
					values["identity/version"] = version
			"TitleId":
				if not parser.is_empty():
					if parser.read() == OK and parser.get_node_type() == XMLParser.NODE_TEXT:
						var text := parser.get_node_data().strip_edges()
						if text != "":
							values["xbox_live/title_id"] = text
			"MSAAppId":
				if not parser.is_empty():
					if parser.read() == OK and parser.get_node_type() == XMLParser.NODE_TEXT:
						var text := parser.get_node_data().strip_edges()
						if text != "":
							values["xbox_live/msa_app_id"] = text
			"ShellVisuals":
				var display_name := _xml_attr(parser, "DefaultDisplayName")
				if display_name != "":
					values["identity/game_name"] = display_name
				var publisher_name := _xml_attr(parser, "PublisherDisplayName")
				if publisher_name != "":
					values["identity/publisher_display_name"] = publisher_name

	return values

func _xml_attr(p_parser: XMLParser, p_name: String) -> String:
	for i in range(p_parser.get_attribute_count()):
		if p_parser.get_attribute_name(i) == p_name:
			return p_parser.get_attribute_value(i)
	return ""

func _on_save_pressed() -> void:
	if not _validate_title_id_field():
		return

	var cfg := ConfigFile.new()

	for def in FIELD_DEFS:
		var section: String = def[0]
		var key: String = def[1]
		var field_key := section + "/" + key
		if field_key in _fields:
			cfg.set_value(section, key, _fields[field_key].text)

	var err := cfg.save(CONFIG_PATH)
	if err == OK:
		_status_label.text = "✅ Saved to " + CONFIG_PATH
	else:
		_status_label.text = "Failed to save: " + error_string(err)

func _on_apply_to_export_pressed() -> void:
	# Map config fields to export preset option names
	var mapping := {
		"xbox_live/title_id": "xbox_live/title_id",
		"xbox_live/msa_app_id": "xbox_live/msa_app_id",
		"xbox_live/store_id": "xbox_live/store_id",
		"xbox_live/scid": "xbox_live/scid",
		"xbox_live/sandbox_id": "dev/sandbox_id",
		"identity/game_name": "identity/game_name",
		"identity/publisher": "identity/publisher_name",
		"identity/publisher_display_name": "identity/publisher_display_name",
		"identity/version": "identity/version",
	}

	# Update export_presets.cfg directly
	var preset_path := "res://export_presets.cfg"
	if not FileAccess.file_exists(preset_path):
		_status_label.text = "No export_presets.cfg found. Add an export preset first."
		return

	if not _validate_title_id_field():
		return

	var content := FileAccess.get_file_as_string(preset_path)

	for config_key in mapping:
		var preset_key: String = mapping[config_key]
		if config_key in _fields:
			var value: String = _fields[config_key].text
			var pattern := preset_key + '="'
			var idx := content.find(pattern)
			if idx >= 0:
				var end_quote := content.find('"', idx + pattern.length())
				if end_quote >= 0:
					content = content.substr(0, idx) + preset_key + '="' + value + '"' + content.substr(end_quote + 1)

	var f := FileAccess.open(preset_path, FileAccess.WRITE)
	if f:
		f.store_string(content)
		f.close()
		_status_label.text = "✅ Export preset updated"
	else:
		_status_label.text = "Failed to write export_presets.cfg"
