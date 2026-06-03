@tool
extends ScrollContainer

var _coordinator

var sandbox_label: Label
var sandbox_id_edit: LineEdit
var sandbox_set_btn: Button
var sandbox_retail_btn: Button
var dev_account_label: Label
var test_account_edit: LineEdit


func setup(coordinator) -> void:
	_coordinator = coordinator
	horizontal_scroll_mode = ScrollContainer.SCROLL_MODE_DISABLED
	size_flags_vertical = SIZE_EXPAND_FILL

	var root := VBoxContainer.new()
	root.size_flags_horizontal = SIZE_EXPAND_FILL
	add_child(root)

	sandbox_label = Label.new()
	sandbox_label.text = "Current: checking..."
	root.add_child(sandbox_label)

	var sandbox_row := HBoxContainer.new()
	root.add_child(sandbox_row)

	var sandbox_id_label := Label.new()
	sandbox_id_label.text = "Sandbox ID"
	sandbox_id_label.custom_minimum_size.x = 130
	sandbox_row.add_child(sandbox_id_label)

	sandbox_id_edit = LineEdit.new()
	sandbox_id_edit.placeholder_text = "e.g. XDKS.1"
	sandbox_id_edit.size_flags_horizontal = SIZE_EXPAND_FILL
	sandbox_row.add_child(sandbox_id_edit)

	var sandbox_btn_row := HBoxContainer.new()
	root.add_child(sandbox_btn_row)

	sandbox_set_btn = Button.new()
	sandbox_set_btn.text = "Set Sandbox"
	sandbox_set_btn.pressed.connect(_on_sandbox_set)
	sandbox_btn_row.add_child(sandbox_set_btn)

	sandbox_retail_btn = Button.new()
	sandbox_retail_btn.text = "Switch to RETAIL"
	sandbox_retail_btn.pressed.connect(_on_sandbox_retail)
	sandbox_btn_row.add_child(sandbox_retail_btn)

	var sandbox_refresh_btn := Button.new()
	sandbox_refresh_btn.text = "Refresh"
	sandbox_refresh_btn.pressed.connect(refresh_status)
	sandbox_btn_row.add_child(sandbox_refresh_btn)

	root.add_child(HSeparator.new())

	_coordinator._add_section_header(root, "Partner Center Account")
	dev_account_label = Label.new()
	dev_account_label.text = "Checking..."
	dev_account_label.autowrap_mode = TextServer.AUTOWRAP_WORD_SMART
	root.add_child(dev_account_label)

	var dev_btn_row := HBoxContainer.new()
	root.add_child(dev_btn_row)

	var signin_btn := Button.new()
	signin_btn.text = "Sign In"
	signin_btn.pressed.connect(_on_dev_account_signin)
	dev_btn_row.add_child(signin_btn)

	var signout_btn := Button.new()
	signout_btn.text = "Sign Out"
	signout_btn.pressed.connect(_on_dev_account_signout)
	dev_btn_row.add_child(signout_btn)

	var test_accounts_btn := Button.new()
	test_accounts_btn.text = "Test Accounts"
	test_accounts_btn.tooltip_text = "Open the Xbox Live Test Account GUI"
	test_accounts_btn.pressed.connect(_on_open_test_accounts)
	dev_btn_row.add_child(test_accounts_btn)

	root.add_child(HSeparator.new())

	_coordinator._add_section_header(root, "Active Test Account")

	var test_row := HBoxContainer.new()
	root.add_child(test_row)

	var test_label := Label.new()
	test_label.text = "Gamertag / Email"
	test_label.custom_minimum_size.x = 130
	test_row.add_child(test_label)

	test_account_edit = LineEdit.new()
	test_account_edit.placeholder_text = "e.g. TestAccount1 or test@xboxtest.com"
	test_account_edit.size_flags_horizontal = SIZE_EXPAND_FILL
	test_row.add_child(test_account_edit)

	var test_hint := Label.new()
	test_hint.text = "Sign into this account via the Xbox App before running your game."
	test_hint.add_theme_font_size_override("font_size", 11)
	test_hint.add_theme_color_override("font_color", Color(0.6, 0.6, 0.6))
	test_hint.autowrap_mode = TextServer.AUTOWRAP_WORD_SMART
	root.add_child(test_hint)


func apply_state(state: Dictionary) -> void:
	var sandbox_state: Dictionary = state.get("sandbox", {})
	sandbox_id_edit.text = str(sandbox_state.get("sandbox_id", ""))
	test_account_edit.text = str(sandbox_state.get("test_account", ""))


func collect_state() -> Dictionary:
	return {
		"sandbox": {
			"sandbox_id": sandbox_id_edit.text,
			"test_account": test_account_edit.text,
		}
	}


func connect_autosave(save_callback: Callable) -> void:
	for edit in [sandbox_id_edit, test_account_edit]:
		edit.text_changed.connect(func(_value): save_callback.call())
		edit.focus_exited.connect(save_callback)


func refresh_status() -> void:
	var sandbox_exe = _coordinator.get_toolchain().get_sandbox_path()
	if sandbox_exe == "":
		sandbox_label.text = "Current: XblPCSandbox.exe not found"
		sandbox_set_btn.disabled = true
		sandbox_retail_btn.disabled = true
		return

	var result = _coordinator.get_toolchain().execute_tool(sandbox_exe, PackedStringArray(["/get"]))
	if result["exit_code"] == 0:
		var output: String = result["stdout"].strip_edges()
		var idx = output.find(":")
		if idx >= 0:
			var sandbox_name = output.substr(idx + 1).strip_edges()
			sandbox_label.text = "Current: %s" % sandbox_name
			if sandbox_name != "" and sandbox_name != "RETAIL":
				sandbox_id_edit.text = sandbox_name
		else:
			sandbox_label.text = "Current: %s" % output
	else:
		sandbox_label.text = "Current: could not determine"

	sandbox_set_btn.disabled = false
	sandbox_retail_btn.disabled = false
	_refresh_dev_account()


func _refresh_dev_account() -> void:
	var dev_exe = _coordinator.get_toolchain().get_dev_account_path()
	if dev_exe == "":
		dev_account_label.text = "XblDevAccount.exe not found"
		return

	var result = _coordinator.get_toolchain().execute_tool(dev_exe, PackedStringArray(["show"]))
	if result["exit_code"] == 0:
		var output: String = result["stdout"].strip_edges()
		if output.contains("is currently signed in"):
			var email_start = output.find("account ") + 8
			var email_end = output.find(" from")
			if email_start > 8 and email_end > email_start:
				var email = output.substr(email_start, email_end - email_start)
				dev_account_label.text = "✅ Signed in: %s" % email
			else:
				dev_account_label.text = "✅ Signed in"
		elif output.contains("No account"):
			dev_account_label.text = "⚠️ Not signed in"
		else:
			dev_account_label.text = output.substr(0, 80)
	else:
		dev_account_label.text = "⚠️ Not signed in"


func _on_sandbox_set() -> void:
	var sandbox_id = sandbox_id_edit.text.strip_edges()
	if sandbox_id == "":
		sandbox_label.text = "Enter a sandbox ID first"
		return

	sandbox_label.text = "Switching to %s..." % sandbox_id
	sandbox_set_btn.disabled = true
	sandbox_retail_btn.disabled = true
	_coordinator._log("Setting sandbox to: %s" % sandbox_id)

	var sandbox_exe = _coordinator.get_toolchain().get_sandbox_path()
	var result = _coordinator.get_toolchain().execute_tool(
		sandbox_exe,
		PackedStringArray(["/set", sandbox_id, "/noApps"])
	)
	if result["exit_code"] == 0:
		_coordinator._log("Sandbox set to %s" % sandbox_id)
	else:
		_coordinator._log("Sandbox switch failed: %s" % result["stdout"])
		push_warning("[GDK] Sandbox switch failed — may need admin privileges")
	refresh_status()


func _on_sandbox_retail() -> void:
	sandbox_label.text = "Switching to RETAIL..."
	sandbox_set_btn.disabled = true
	sandbox_retail_btn.disabled = true
	_coordinator._log("Switching sandbox to RETAIL")

	var sandbox_exe = _coordinator.get_toolchain().get_sandbox_path()
	var result = _coordinator.get_toolchain().execute_tool(
		sandbox_exe,
		PackedStringArray(["/retail", "/noApps"])
	)
	if result["exit_code"] == 0:
		_coordinator._log("Sandbox set to RETAIL")
	else:
		_coordinator._log("Sandbox switch failed: %s" % result["stdout"])
		push_warning("[GDK] Sandbox switch failed — may need admin privileges")
	refresh_status()


func _on_dev_account_signin() -> void:
	var dev_exe = _coordinator.get_toolchain().get_dev_account_path()
	if dev_exe == "":
		return

	dev_account_label.text = "Signing in..."
	_coordinator._log("Launching Partner Center sign-in...")
	_coordinator.get_toolchain().launch_detached(dev_exe, PackedStringArray(["signin"]))
	get_tree().create_timer(5.0).timeout.connect(_refresh_dev_account)


func _on_dev_account_signout() -> void:
	var dev_exe = _coordinator.get_toolchain().get_dev_account_path()
	if dev_exe == "":
		return

	dev_account_label.text = "Signing out..."
	var result = _coordinator.get_toolchain().execute_tool(dev_exe, PackedStringArray(["signout"]))
	if result["exit_code"] == 0:
		_coordinator._log("Dev account signed out")
	else:
		_coordinator._log("Sign out failed: %s" % result["stdout"])
	_refresh_dev_account()


func _on_open_test_accounts() -> void:
	var test_gui = _coordinator.get_toolchain().get_bin_dir().path_join("XblTestAccountGui.exe")
	if FileAccess.file_exists(test_gui):
		_coordinator.get_toolchain().launch_detached(test_gui, PackedStringArray([]))
		_coordinator._log("Launched Xbox Live Test Account GUI")
	else:
		_coordinator._log("XblTestAccountGui.exe not found")
		push_warning("[GDK] XblTestAccountGui.exe not found")
