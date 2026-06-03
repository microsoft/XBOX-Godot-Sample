@tool
extends ScrollContainer

var _coordinator
var _registered_apps: Array[Dictionary] = []

var install_status_label: Label
var app_selector: OptionButton
var launch_btn: Button
var terminate_btn: Button
var launch_status_label: Label


func setup(coordinator) -> void:
	_coordinator = coordinator
	horizontal_scroll_mode = ScrollContainer.SCROLL_MODE_DISABLED
	size_flags_vertical = SIZE_EXPAND_FILL

	var root := VBoxContainer.new()
	root.size_flags_horizontal = SIZE_EXPAND_FILL
	add_child(root)

	_coordinator._add_section_header(root, "Install")

	var install_desc := Label.new()
	install_desc.text = "Install or uninstall the MSIXVC package from the Package/ folder."
	install_desc.autowrap_mode = TextServer.AUTOWRAP_WORD_SMART
	install_desc.add_theme_font_size_override("font_size", 12)
	install_desc.add_theme_color_override("font_color", Color(0.7, 0.7, 0.7))
	root.add_child(install_desc)

	var install_btn_row := HBoxContainer.new()
	root.add_child(install_btn_row)

	var install_btn := Button.new()
	install_btn.text = "Install"
	install_btn.tooltip_text = "Install the MSIXVC package from the Package/ folder"
	install_btn.pressed.connect(_on_pkg_install)
	install_btn_row.add_child(install_btn)

	var uninstall_btn := Button.new()
	uninstall_btn.text = "Uninstall"
	uninstall_btn.tooltip_text = "Uninstall the selected app"
	uninstall_btn.pressed.connect(_on_pkg_uninstall)
	install_btn_row.add_child(uninstall_btn)

	install_status_label = Label.new()
	install_status_label.text = ""
	root.add_child(install_status_label)

	root.add_child(HSeparator.new())

	_coordinator._add_section_header(root, "Launch")

	var launch_desc := Label.new()
	launch_desc.text = "Select a registered app to launch or terminate."
	launch_desc.add_theme_font_size_override("font_size", 12)
	launch_desc.add_theme_color_override("font_color", Color(0.7, 0.7, 0.7))
	root.add_child(launch_desc)

	var app_row := HBoxContainer.new()
	root.add_child(app_row)

	var app_label := Label.new()
	app_label.text = "Registered App"
	app_label.custom_minimum_size.x = 130
	app_row.add_child(app_label)

	app_selector = OptionButton.new()
	app_selector.size_flags_horizontal = SIZE_EXPAND_FILL
	app_selector.tooltip_text = "Select a registered app to launch or terminate"
	app_row.add_child(app_selector)

	var refresh_btn := Button.new()
	refresh_btn.text = "Refresh"
	refresh_btn.tooltip_text = "Refresh the list of registered apps"
	refresh_btn.pressed.connect(refresh_registered_apps)
	app_row.add_child(refresh_btn)

	var launch_btn_row := HBoxContainer.new()
	root.add_child(launch_btn_row)

	launch_btn = Button.new()
	launch_btn.text = "Launch"
	launch_btn.tooltip_text = "Launch the selected app"
	launch_btn.pressed.connect(_on_app_launch)
	launch_btn_row.add_child(launch_btn)

	terminate_btn = Button.new()
	terminate_btn.text = "Terminate"
	terminate_btn.tooltip_text = "Terminate the selected app"
	terminate_btn.pressed.connect(_on_app_terminate)
	launch_btn_row.add_child(terminate_btn)

	launch_status_label = Label.new()
	launch_status_label.text = ""
	root.add_child(launch_status_label)

	refresh_registered_apps()


func refresh_registered_apps() -> void:
	app_selector.clear()
	_registered_apps.clear()

	if not _coordinator.get_wdapp_manager().is_available():
		app_selector.add_item("wdapp.exe not found")
		launch_btn.disabled = true
		terminate_btn.disabled = true
		return

	var result = _coordinator.get_wdapp_manager().list_registered_apps()
	if result["exit_code"] != 0:
		app_selector.add_item("Failed to list apps")
		launch_btn.disabled = true
		terminate_btn.disabled = true
		return

	_registered_apps = result.get("apps", [])
	for app in _registered_apps:
		app_selector.add_item(str(app["aumid"]))

	if _registered_apps.is_empty():
		app_selector.add_item("No registered apps")
		launch_btn.disabled = true
		terminate_btn.disabled = true
	else:
		launch_btn.disabled = false
		terminate_btn.disabled = false


func _get_selected_aumid() -> String:
	var idx = app_selector.selected
	if idx < 0 or idx >= _registered_apps.size():
		return ""
	return _registered_apps[idx]["aumid"]


func _get_selected_pfn() -> String:
	var idx = app_selector.selected
	if idx < 0 or idx >= _registered_apps.size():
		return ""
	return _registered_apps[idx]["pfn"]


func _on_app_launch() -> void:
	var aumid = _get_selected_aumid()
	if aumid == "":
		launch_status_label.text = "❌ No app selected"
		return

	_coordinator._log("Launching: %s" % aumid)
	var result = _coordinator.get_wdapp_manager().launch_app(aumid)
	if result["exit_code"] == 0:
		launch_status_label.text = "✅ Launched"
		_coordinator._log("App launched successfully")
	else:
		launch_status_label.text = "❌ Launch failed"
		_coordinator._log("Launch failed: %s" % result["stdout"])


func _on_app_terminate() -> void:
	var aumid = _get_selected_aumid()
	var pfn = _get_selected_pfn()
	if aumid == "":
		launch_status_label.text = "❌ No app selected"
		return

	_coordinator._log("Terminating: %s" % pfn)
	var result = _coordinator.get_wdapp_manager().terminate_app(pfn, _coordinator.get_build_dir())
	if result["exit_code"] == 0:
		var terminated_with = str(result.get("terminated_with", "wdapp"))
		launch_status_label.text = "✅ Terminated"
		if terminated_with == "taskkill":
			launch_status_label.text = "✅ Terminated (taskkill)"
		_coordinator._log("App terminated via %s" % terminated_with)
		return

	launch_status_label.text = "❌ Terminate failed"
	_coordinator._log("Terminate failed: %s" % result["stdout"])


func _find_msixvc_package() -> String:
	var pkg_dir = _coordinator.get_package_dir()
	if not DirAccess.dir_exists_absolute(pkg_dir):
		return ""

	var dir = DirAccess.open(pkg_dir)
	if dir == null:
		return ""

	dir.list_dir_begin()
	var fname = dir.get_next()
	while fname != "":
		if fname.ends_with(".msixvc") and not fname.begins_with("clear."):
			dir.list_dir_end()
			return pkg_dir.path_join(fname)
		fname = dir.get_next()
	dir.list_dir_end()
	return ""


func _on_pkg_install() -> void:
	var msixvc = _find_msixvc_package()
	if msixvc == "":
		install_status_label.text = "❌ No .msixvc package found in Package/"
		return

	_coordinator._log("Installing: %s" % msixvc)
	install_status_label.text = "Installing package..."
	var result = _coordinator.get_wdapp_manager().install_package(msixvc)
	if result["exit_code"] == 0:
		install_status_label.text = "✅ Package installed"
		_coordinator._log("Package installed successfully")
		refresh_registered_apps()
	else:
		install_status_label.text = "❌ Install failed"
		_coordinator._log("Install failed: %s" % result["stdout"])


func _on_pkg_uninstall() -> void:
	var pfn = _get_selected_pfn()
	if pfn == "":
		install_status_label.text = "❌ No app selected — select one from the Launch section"
		return

	_coordinator._log("Uninstalling: %s" % pfn)
	var result = _coordinator.get_wdapp_manager().uninstall_package(pfn)
	if result["exit_code"] == 0:
		install_status_label.text = "✅ Package uninstalled"
		_coordinator._log("Package uninstalled")
		refresh_registered_apps()
	else:
		install_status_label.text = "❌ Uninstall failed"
		_coordinator._log("Uninstall failed: %s" % result["stdout"])
