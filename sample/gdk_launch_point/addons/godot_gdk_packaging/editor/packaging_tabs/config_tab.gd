@tool
extends ScrollContainer

var _coordinator: Variant

var status_label: Label
var identity_label: Label
var game_config_btn: Button
var preview_container: VBoxContainer


func setup(coordinator: Variant) -> void:
	_coordinator = coordinator
	horizontal_scroll_mode = ScrollContainer.SCROLL_MODE_DISABLED
	size_flags_vertical = SIZE_EXPAND_FILL

	var root: VBoxContainer = VBoxContainer.new()
	root.size_flags_horizontal = SIZE_EXPAND_FILL
	add_child(root)

	_coordinator._add_section_header(root, "MicrosoftGame.config")

	status_label = Label.new()
	root.add_child(status_label)

	identity_label = Label.new()
	identity_label.autowrap_mode = TextServer.AUTOWRAP_WORD_SMART
	identity_label.add_theme_font_size_override("font_size", 12)
	root.add_child(identity_label)

	var config_btns: HBoxContainer = HBoxContainer.new()
	root.add_child(config_btns)

	game_config_btn = Button.new()
	game_config_btn.text = "Create MicrosoftGame.config"
	game_config_btn.pressed.connect(_on_game_config_action)
	config_btns.add_child(game_config_btn)

	var refresh_btn: Button = Button.new()
	refresh_btn.text = "Refresh"
	refresh_btn.pressed.connect(refresh_status)
	config_btns.add_child(refresh_btn)

	var open_folder_btn: Button = Button.new()
	open_folder_btn.text = "Open Folder"
	open_folder_btn.pressed.connect(_on_open_config_folder)
	config_btns.add_child(open_folder_btn)

	root.add_child(HSeparator.new())

	_coordinator._add_section_header(root, "MicrosoftGame.config Preview")
	preview_container = VBoxContainer.new()
	preview_container.size_flags_horizontal = SIZE_EXPAND_FILL
	root.add_child(preview_container)


func refresh_status() -> void:
	var config_mgr: Variant = _coordinator.get_config_manager()
	if config_mgr.config_exists():
		status_label.text = "✅ MicrosoftGame.config found"
		game_config_btn.text = "Edit MicrosoftGame.config"

		var info: Dictionary = config_mgr.parse_config()
		if info.size() > 0 and info["name"] != "":
			identity_label.text = "%s | %s | v%s" % [
				info.get("display_name", info["name"]),
				info.get("publisher", ""),
				info.get("version", "?"),
			]

			var packaging_tab: Variant = _coordinator.get_packaging_tab()
			if packaging_tab != null and packaging_tab.product_id_edit != null:
				if packaging_tab.product_id_edit.text == "" and info.get("product_id", "") != "":
					packaging_tab.product_id_edit.text = info["product_id"]
		else:
			identity_label.text = "(could not parse identity)"

		_refresh_preview(info)

		var relocated: int = config_mgr.relocate_logos_to_storelogos()
		if relocated > 0:
			_coordinator._log("Relocated %d logo(s) to storelogos/" % relocated)

		var synced: int = config_mgr.sync_store_logos()
		if synced > 0:
			_coordinator._log("Synced %d store logo(s) from 480x480 source" % synced)
	else:
		status_label.text = "⚠️ MicrosoftGame.config not found"
		identity_label.text = "Create a template or use GameConfigEditor to get started."
		game_config_btn.text = "Create MicrosoftGame.config"
		_refresh_preview({})


func _refresh_preview(info: Dictionary) -> void:
	for child: Node in preview_container.get_children():
		child.queue_free()

	if info.is_empty():
		var empty_label: Label = Label.new()
		empty_label.text = "No config loaded."
		empty_label.add_theme_color_override("font_color", Color(0.6, 0.6, 0.6))
		preview_container.add_child(empty_label)
		return

	var fields: Array = [
		["Config Version", info.get("config_version", ""), "Game configVersion — must be '1' for GDK Oct 2023+. Controls schema validation rules."],
		["Identity Name", info.get("name", ""), "Identity.Name — unique package name from Partner Center → Product Identity. Used during registration."],
		["Publisher", info.get("publisher", ""), "Identity.Publisher — publisher CN string from Partner Center → Product Identity."],
		["Version", info.get("version", ""), "Identity.Version — package version (Major.Minor.Build.Revision). Defaults to 1.0.0.0."],
		["Display Name", info.get("display_name", ""), "ShellVisuals.DefaultDisplayName — title name shown in the Xbox shell and Store."],
		["Description", info.get("description", ""), "ShellVisuals.Description — short description shown in the shell."],
		["Title ID", info.get("title_id", ""), "TitleId — hex title ID from Partner Center → Xbox Live Setup. Required with configVersion 1."],
		["MSA App ID", info.get("msa_app_id", ""), "MSAAppId — Microsoft Account app ID from Partner Center. Required when TitleId is set."],
		["Store ID", info.get("store_id", ""), "StoreId — Microsoft Store product ID (e.g. 9XXXXXXXXX)."],
		["Product ID", info.get("product_id", ""), "MSStore.ProductId — used for licensing and Store association."],
		["Executable", info.get("executable", ""), "Executable.Name — the game executable filename (e.g. MyGame.exe)."],
		["Background Color", info.get("background_color", ""), "ShellVisuals.BackgroundColor — hex color for the tile background (e.g. #000000)."],
		["Foreground Text", info.get("foreground_text", ""), "ShellVisuals.ForegroundText — 'light' or 'dark' text on the tile background."],
		["480x480 Logo", info.get("logo_480", ""), "ShellVisuals.Square480x480Logo — primary tile image path. Used to generate other sizes."],
		["Store Logo", info.get("store_logo", ""), "ShellVisuals.StoreLogo — small logo used in the Store listing."],
		["Splash Screen", info.get("splash_screen", ""), "ShellVisuals.SplashScreenImage — image shown during game launch."],
	]

	for field: Array in fields:
		var label_text: String = field[0]
		var value: String = field[1]
		var tooltip: String = field[2]

		if value == "":
			continue

		var row: HBoxContainer = HBoxContainer.new()
		preview_container.add_child(row)

		var key_label: Label = Label.new()
		key_label.text = label_text
		key_label.custom_minimum_size.x = 120
		key_label.add_theme_color_override("font_color", Color(0.7, 0.7, 0.7))
		key_label.add_theme_font_size_override("font_size", 14)
		key_label.tooltip_text = tooltip
		key_label.mouse_filter = Control.MOUSE_FILTER_PASS
		row.add_child(key_label)

		var val_label: Label = Label.new()
		val_label.text = value
		val_label.add_theme_font_size_override("font_size", 14)
		val_label.size_flags_horizontal = SIZE_EXPAND_FILL
		val_label.text_overrun_behavior = TextServer.OVERRUN_TRIM_ELLIPSIS
		val_label.tooltip_text = tooltip
		val_label.mouse_filter = Control.MOUSE_FILTER_PASS
		row.add_child(val_label)


func _on_game_config_action() -> void:
	var config_mgr: Variant = _coordinator.get_config_manager()
	if not config_mgr.config_exists():
		_create_config(config_mgr)
		return

	var pid: int = config_mgr.launch_editor()
	if pid >= 0:
		_coordinator._log("Launched GameConfigEditor (PID: %d)" % pid)
	else:
		_coordinator._log("Failed to launch GameConfigEditor")
		push_error("[GDK Packaging] Failed to launch GameConfigEditor")


func _create_config(config_mgr: Variant) -> void:
	var err: Error = config_mgr.create_template()
	if err == OK:
		_coordinator._log("Created template MicrosoftGame.config in project root")
		refresh_status()
		var fs: EditorFileSystem = EditorInterface.get_resource_filesystem()
		if not fs.is_scanning():
			fs.scan()
	elif err == ERR_ALREADY_EXISTS:
		_coordinator._log("MicrosoftGame.config already exists")
		push_warning("[GDK Packaging] MicrosoftGame.config already exists")
	else:
		_coordinator._log("Failed to create MicrosoftGame.config: " + error_string(err))
		push_error("[GDK Packaging] Failed to create MicrosoftGame.config: " + error_string(err))


func _on_open_config_folder() -> void:
	OS.shell_open(ProjectSettings.globalize_path("res://"))
