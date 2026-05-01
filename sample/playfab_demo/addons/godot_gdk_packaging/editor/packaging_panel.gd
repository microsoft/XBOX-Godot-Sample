@tool
extends Control
## GDK dock panel — manages MicrosoftGame.config, store logos, PC packaging
## via makepkg, and achievements configuration.

const GDKToolchainScript = preload("res://addons/godot_gdk_packaging/editor/gdk_toolchain.gd")
const MakePkgExecutorScript = preload("res://addons/godot_gdk_packaging/editor/makepkg_executor.gd")
const GameConfigManagerScript = preload("res://addons/godot_gdk_packaging/editor/game_config_manager.gd")

const SAMPLE_CONFIG_PATH := "res://sample_config.cfg"
const PLAYFAB_CONFIG_PATH := "res://sample_pf_config.cfg"
const PACKAGING_SETTINGS_PATH := "res://.gdk_packaging.cfg"

# Encryption option indices (must match OptionButton order in _build_packaging_ui)
const ENCRYPT_NONE := 0
const ENCRYPT_LICENSE := 1
const ENCRYPT_CUSTOM_KEY := 2

var _toolchain: RefCounted
var _makepkg: RefCounted
var _config_mgr: RefCounted

# ── UI references ───────────────────────────────────────────────────────────

# Source configuration
var _source_dir_edit: LineEdit
var _map_file_edit: LineEdit
var _auto_genmap_check: CheckBox
var _output_dir_edit: LineEdit

# Packaging options
var _content_id_edit: LineEdit
var _product_id_edit: LineEdit
var _encrypt_option: OptionButton
var _encrypt_key_edit: LineEdit
var _updcompat_option: OptionButton

# Config status
var _config_status_label: Label
var _config_identity_label: Label
var _edit_config_btn: Button
var _create_config_btn: Button
var _config_preview_container: VBoxContainer

# Sandbox
var _sandbox_label: Label
var _sandbox_id_edit: LineEdit
var _sandbox_set_btn: Button
var _sandbox_retail_btn: Button
var _dev_account_label: Label
var _test_account_edit: LineEdit

# Actions
var _genmap_btn: Button
var _validate_btn: Button
var _pack_btn: Button

# Achievements
var _achievement_id_edit: LineEdit
var _achievement_save_btn: Button
var _achievement_status_label: Label

# PlayFab
var _playfab_title_id_edit: LineEdit
var _playfab_status_label: Label
var _playfab_version_label: Label

# Output
var _status_label: Label

# Logo watcher
var _watch_timer: float = 0.0
# How often (seconds) to check for logo files GameConfigEditor writes to project root
const WATCH_INTERVAL := 2.0
# Standard filenames GameConfigEditor writes when regenerating tile images
const ROOT_LOGO_FILES := [
	"StoreLogo.png",
	"Square44x44Logo.png",
	"Square150x150Logo.png",
	"Square480x480Logo.png",
	"SplashScreenImage.png",
]


## Initializes toolchain, builds UI, loads saved settings, and starts logo watcher.
func _ready() -> void:
	_toolchain = GDKToolchainScript.new()
	_makepkg = MakePkgExecutorScript.new(_toolchain)
	_config_mgr = GameConfigManagerScript.new(_toolchain)
	_build_ui()
	_load_packaging_settings()
	_refresh_sandbox_status()
	_refresh_config_status()
	set_process(true)


## Polls for GameConfigEditor logo output files at root and auto-relocates them.
func _process(delta: float) -> void:
	_watch_timer += delta
	if _watch_timer < WATCH_INTERVAL:
		return
	_watch_timer = 0.0
	_check_and_relocate_root_logos()


## Detects known logo PNGs that GameConfigEditor writes to project root,
## moves them to storelogos/, removes stale .import files, and triggers rescan.
func _check_and_relocate_root_logos() -> void:
	var project_dir = ProjectSettings.globalize_path("res://")
	var logos_dir = project_dir.path_join("storelogos")

	# Check if any known logo files exist at root
	var found_any := false
	for filename in ROOT_LOGO_FILES:
		if FileAccess.file_exists(project_dir.path_join(filename)):
			found_any = true
			break

	if not found_any:
		return

	# Ensure storelogos directory exists
	DirAccess.make_dir_recursive_absolute(logos_dir)

	var dir = DirAccess.open(project_dir)
	if dir == null:
		return

	var moved := 0
	for filename in ROOT_LOGO_FILES:
		var src = project_dir.path_join(filename)
		if not FileAccess.file_exists(src):
			continue
		var dest = logos_dir.path_join(filename)
		var err = dir.rename(src, dest)
		if err == OK:
			moved += 1
			print("[GDK Packaging] Auto-moved ", filename, " -> storelogos/")
			# Also move the .import file if it exists
			var import_src = src + ".import"
			if FileAccess.file_exists(import_src):
				dir.remove(import_src)
		else:
			push_warning("[GDK Packaging] Failed to move " + filename + ": " + error_string(err))

	if moved > 0:
		_log("Auto-relocated %d logo(s) to storelogos/" % moved)
		# Update config paths to point to storelogos/
		_config_mgr.relocate_logos_to_storelogos()
		# Remove stale .import files in storelogos/ so thumbnails regenerate
		for filename in ROOT_LOGO_FILES:
			var import_file = logos_dir.path_join(filename + ".import")
			if FileAccess.file_exists(import_file):
				dir.remove(import_file)
		# Trigger filesystem rescan to reimport and refresh thumbnails
		var fs = EditorInterface.get_resource_filesystem()
		if not fs.is_scanning():
			fs.scan()


# ── UI Construction ─────────────────────────────────────────────────────────

## Constructs the tabbed dock UI: header, tab bar, and content pages.
func _build_ui() -> void:
	var outer := VBoxContainer.new()
	outer.set_anchors_and_offsets_preset(PRESET_FULL_RECT)
	add_child(outer)

	# ── Header (above tabs) ──
	_status_label = Label.new()
	if _toolchain.is_gdk_available():
		var version_text = _toolchain.get_gdk_version()
		if version_text != "":
			_status_label.text = "✅ GDK %s" % version_text
		else:
			_status_label.text = "✅ GDK tools found"
	else:
		_status_label.text = "❌ GDK not found — install Microsoft GDK"
	outer.add_child(_status_label)

	# ── Tab Bar (visible, styled buttons) ──
	var tab_bar := TabBar.new()
	tab_bar.add_tab("⚙️ Config")
	tab_bar.add_tab("📦 Packaging")
	tab_bar.add_tab("🔒 Sandbox")
	tab_bar.add_tab("🏆 Achievements")
	tab_bar.add_tab("☁️ PlayFab")
	tab_bar.clip_tabs = false
	tab_bar.size_flags_horizontal = SIZE_EXPAND_FILL
	tab_bar.add_theme_font_size_override("font_size", 18)
	outer.add_child(tab_bar)

	# ── Content pages (one per tab) ──
	var _tab_pages: Array[ScrollContainer] = []

	var config_scroll := ScrollContainer.new()
	config_scroll.horizontal_scroll_mode = ScrollContainer.SCROLL_MODE_DISABLED
	config_scroll.size_flags_vertical = SIZE_EXPAND_FILL
	outer.add_child(config_scroll)
	var config_root := VBoxContainer.new()
	config_root.size_flags_horizontal = SIZE_EXPAND_FILL
	config_scroll.add_child(config_root)
	_build_config_ui(config_root)
	_tab_pages.append(config_scroll)

	var pkg_scroll := ScrollContainer.new()
	pkg_scroll.horizontal_scroll_mode = ScrollContainer.SCROLL_MODE_DISABLED
	pkg_scroll.size_flags_vertical = SIZE_EXPAND_FILL
	pkg_scroll.visible = false
	outer.add_child(pkg_scroll)
	var pkg := VBoxContainer.new()
	pkg.size_flags_horizontal = SIZE_EXPAND_FILL
	pkg_scroll.add_child(pkg)
	_build_packaging_ui(pkg)
	_tab_pages.append(pkg_scroll)

	var sandbox_scroll := ScrollContainer.new()
	sandbox_scroll.horizontal_scroll_mode = ScrollContainer.SCROLL_MODE_DISABLED
	sandbox_scroll.size_flags_vertical = SIZE_EXPAND_FILL
	sandbox_scroll.visible = false
	outer.add_child(sandbox_scroll)
	var sandbox := VBoxContainer.new()
	sandbox.size_flags_horizontal = SIZE_EXPAND_FILL
	sandbox_scroll.add_child(sandbox)
	_build_sandbox_ui(sandbox)
	_tab_pages.append(sandbox_scroll)

	var ach_scroll := ScrollContainer.new()
	ach_scroll.horizontal_scroll_mode = ScrollContainer.SCROLL_MODE_DISABLED
	ach_scroll.size_flags_vertical = SIZE_EXPAND_FILL
	ach_scroll.visible = false
	outer.add_child(ach_scroll)
	var ach := VBoxContainer.new()
	ach.size_flags_horizontal = SIZE_EXPAND_FILL
	ach_scroll.add_child(ach)
	_build_achievements_ui(ach)
	_tab_pages.append(ach_scroll)

	var playfab_scroll := ScrollContainer.new()
	playfab_scroll.horizontal_scroll_mode = ScrollContainer.SCROLL_MODE_DISABLED
	playfab_scroll.size_flags_vertical = SIZE_EXPAND_FILL
	playfab_scroll.visible = false
	outer.add_child(playfab_scroll)
	var playfab := VBoxContainer.new()
	playfab.size_flags_horizontal = SIZE_EXPAND_FILL
	playfab_scroll.add_child(playfab)
	_build_playfab_ui(playfab)
	_tab_pages.append(playfab_scroll)

	# Wire tab switching
	tab_bar.tab_changed.connect(func(idx: int):
		for i in _tab_pages.size():
			_tab_pages[i].visible = (i == idx)
	)

	_set_actions_enabled(_toolchain.is_gdk_available())
	_load_achievement_config()
	_load_playfab_config()
	_connect_autosave()


## Builds the Sandbox tab: sandbox switcher, Partner Center account, test account.
func _build_sandbox_ui(root: VBoxContainer) -> void:
	_sandbox_label = Label.new()
	_sandbox_label.text = "Current: checking..."
	root.add_child(_sandbox_label)

	var sandbox_row := HBoxContainer.new()
	root.add_child(sandbox_row)
	var sandbox_id_label := Label.new()
	sandbox_id_label.text = "Sandbox ID"
	sandbox_id_label.custom_minimum_size.x = 130
	sandbox_row.add_child(sandbox_id_label)
	_sandbox_id_edit = LineEdit.new()
	_sandbox_id_edit.placeholder_text = "e.g. XDKS.1"
	_sandbox_id_edit.size_flags_horizontal = SIZE_EXPAND_FILL
	sandbox_row.add_child(_sandbox_id_edit)

	var sandbox_btn_row := HBoxContainer.new()
	root.add_child(sandbox_btn_row)
	_sandbox_set_btn = Button.new()
	_sandbox_set_btn.text = "Set Sandbox"
	_sandbox_set_btn.pressed.connect(_on_sandbox_set)
	sandbox_btn_row.add_child(_sandbox_set_btn)

	_sandbox_retail_btn = Button.new()
	_sandbox_retail_btn.text = "Switch to RETAIL"
	_sandbox_retail_btn.pressed.connect(_on_sandbox_retail)
	sandbox_btn_row.add_child(_sandbox_retail_btn)

	var sandbox_refresh_btn := Button.new()
	sandbox_refresh_btn.text = "Refresh"
	sandbox_refresh_btn.pressed.connect(_refresh_sandbox_status)
	sandbox_btn_row.add_child(sandbox_refresh_btn)

	root.add_child(HSeparator.new())

	# ── Dev Account ──
	_add_section_header(root, "Partner Center Account")
	_dev_account_label = Label.new()
	_dev_account_label.text = "Checking..."
	_dev_account_label.autowrap_mode = TextServer.AUTOWRAP_WORD_SMART
	root.add_child(_dev_account_label)

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

	# ── Test Account ──
	_add_section_header(root, "Active Test Account")

	var test_row := HBoxContainer.new()
	root.add_child(test_row)
	var test_label := Label.new()
	test_label.text = "Gamertag / Email"
	test_label.custom_minimum_size.x = 130
	test_row.add_child(test_label)
	_test_account_edit = LineEdit.new()
	_test_account_edit.placeholder_text = "e.g. TestAccount1 or test@xboxtest.com"
	_test_account_edit.size_flags_horizontal = SIZE_EXPAND_FILL
	test_row.add_child(_test_account_edit)

	var test_hint := Label.new()
	test_hint.text = "Sign into this account via the Xbox App before running your game."
	test_hint.add_theme_font_size_override("font_size", 11)
	test_hint.add_theme_color_override("font_color", Color(0.6, 0.6, 0.6))
	test_hint.autowrap_mode = TextServer.AUTOWRAP_WORD_SMART
	root.add_child(test_hint)


## Builds the Config tab: MicrosoftGame.config status, buttons, and preview.
func _build_config_ui(root: VBoxContainer) -> void:
	_add_section_header(root, "MicrosoftGame.config")

	_config_status_label = Label.new()
	root.add_child(_config_status_label)

	_config_identity_label = Label.new()
	_config_identity_label.autowrap_mode = TextServer.AUTOWRAP_WORD_SMART
	_config_identity_label.add_theme_font_size_override("font_size", 12)
	root.add_child(_config_identity_label)

	var config_btns := HBoxContainer.new()
	root.add_child(config_btns)

	_create_config_btn = Button.new()
	_create_config_btn.text = "Create MicrosoftGame.config"
	_create_config_btn.pressed.connect(_on_create_config)
	config_btns.add_child(_create_config_btn)

	_edit_config_btn = Button.new()
	_edit_config_btn.text = "Edit with GameConfigEditor"
	_edit_config_btn.pressed.connect(_on_edit_config)
	config_btns.add_child(_edit_config_btn)

	var refresh_btn := Button.new()
	refresh_btn.text = "Refresh"
	refresh_btn.pressed.connect(_refresh_config_status)
	config_btns.add_child(refresh_btn)

	var open_folder_btn := Button.new()
	open_folder_btn.text = "Open Folder"
	open_folder_btn.pressed.connect(_on_open_config_folder)
	config_btns.add_child(open_folder_btn)

	root.add_child(HSeparator.new())

	# ── Config Preview ──
	_add_section_header(root, "MicrosoftGame.config Preview")
	_config_preview_container = VBoxContainer.new()
	_config_preview_container.size_flags_horizontal = SIZE_EXPAND_FILL
	root.add_child(_config_preview_container)


## Builds the Achievements tab: demo achievement ID field.
func _build_achievements_ui(root: VBoxContainer) -> void:
	var ach_row := HBoxContainer.new()
	root.add_child(ach_row)
	var ach_label := Label.new()
	ach_label.text = "Demo Achievement ID"
	ach_label.custom_minimum_size.x = 130
	ach_row.add_child(ach_label)
	_achievement_id_edit = LineEdit.new()
	_achievement_id_edit.placeholder_text = "Achievement ID to test (e.g. 1)"
	_achievement_id_edit.size_flags_horizontal = SIZE_EXPAND_FILL
	ach_row.add_child(_achievement_id_edit)

	var ach_btn_row := HBoxContainer.new()
	root.add_child(ach_btn_row)
	_achievement_save_btn = Button.new()
	_achievement_save_btn.text = "Save"
	_achievement_save_btn.pressed.connect(_on_achievement_save)
	ach_btn_row.add_child(_achievement_save_btn)

	_achievement_status_label = Label.new()
	_achievement_status_label.text = ""
	root.add_child(_achievement_status_label)


## Builds the PlayFab tab: Title ID field, Game Manager link, SDK version.
func _build_playfab_ui(root: VBoxContainer) -> void:
	var desc := Label.new()
	desc.text = "Configure your PlayFab Title ID for Xbox Live integration.\nThe Title ID is used at runtime to connect to PlayFab services."
	desc.autowrap_mode = TextServer.AUTOWRAP_WORD_SMART
	desc.add_theme_font_size_override("font_size", 12)
	desc.add_theme_color_override("font_color", Color(0.7, 0.7, 0.7))
	root.add_child(desc)

	root.add_child(HSeparator.new())

	# PlayFab Title ID
	var title_row := HBoxContainer.new()
	root.add_child(title_row)
	var title_label := Label.new()
	title_label.text = "PlayFab Title ID"
	title_label.custom_minimum_size.x = 130
	title_label.tooltip_text = "Your PlayFab Title ID from Game Manager → Settings → API Keys. Used at runtime to initialize the PlayFab SDK."
	title_row.add_child(title_label)
	_playfab_title_id_edit = LineEdit.new()
	_playfab_title_id_edit.placeholder_text = "e.g. A1B2C"
	_playfab_title_id_edit.size_flags_horizontal = SIZE_EXPAND_FILL
	title_row.add_child(_playfab_title_id_edit)

	var btn_row := HBoxContainer.new()
	root.add_child(btn_row)

	var save_btn := Button.new()
	save_btn.text = "Save"
	save_btn.pressed.connect(_on_playfab_save)
	btn_row.add_child(save_btn)

	var manager_btn := Button.new()
	manager_btn.text = "Open Game Manager"
	manager_btn.tooltip_text = "Open the PlayFab Game Manager in your browser"
	manager_btn.pressed.connect(func(): OS.shell_open("https://developer.playfab.com/en-us/r/sign-in"))
	btn_row.add_child(manager_btn)

	_playfab_status_label = Label.new()
	_playfab_status_label.text = ""
	root.add_child(_playfab_status_label)

	root.add_child(HSeparator.new())

	# PlayFab SDK version
	_playfab_version_label = Label.new()
	_playfab_version_label.text = "PlayFab SDK: detecting..."
	_playfab_version_label.add_theme_font_size_override("font_size", 12)
	_playfab_version_label.add_theme_color_override("font_color", Color(0.7, 0.7, 0.7))
	root.add_child(_playfab_version_label)
	_detect_playfab_version()


## Builds the Packaging tab: source config, options, and action buttons.
func _build_packaging_ui(root: VBoxContainer) -> void:
	# ── Source Configuration ──
	_add_section_header(root, "Package Source Configuration")

	_source_dir_edit = _add_path_field(root, "Content Directory",
		"Directory with exported game files", true)
	_map_file_edit = _add_path_field(root, "Mapping File",
		"XML mapping file (or auto-generate)", false)

	_auto_genmap_check = CheckBox.new()
	_auto_genmap_check.text = "Auto-generate mapping file before packaging"
	_auto_genmap_check.button_pressed = true
	_auto_genmap_check.toggled.connect(_on_auto_genmap_toggled)
	root.add_child(_auto_genmap_check)

	_output_dir_edit = _add_path_field(root, "Output Directory",
		"Where .msixvc package is placed", true)

	root.add_child(HSeparator.new())

	# ── Packaging Options ──
	_add_section_header(root, "Packaging Options")

	var cid_row := HBoxContainer.new()
	root.add_child(cid_row)
	var cid_label := Label.new()
	cid_label.text = "Content ID"
	cid_label.custom_minimum_size.x = 130
	cid_row.add_child(cid_label)
	_content_id_edit = LineEdit.new()
	_content_id_edit.placeholder_text = "Optional — from MicrosoftGame.config if blank"
	_content_id_edit.size_flags_horizontal = SIZE_EXPAND_FILL
	cid_row.add_child(_content_id_edit)

	var pid_row := HBoxContainer.new()
	root.add_child(pid_row)
	var pid_label := Label.new()
	pid_label.text = "Product ID"
	pid_label.custom_minimum_size.x = 130
	pid_row.add_child(pid_label)
	_product_id_edit = LineEdit.new()
	_product_id_edit.placeholder_text = "Optional — from MicrosoftGame.config if blank"
	_product_id_edit.size_flags_horizontal = SIZE_EXPAND_FILL
	pid_row.add_child(_product_id_edit)

	var enc_row := HBoxContainer.new()
	root.add_child(enc_row)
	var enc_label := Label.new()
	enc_label.text = "Encryption"
	enc_label.custom_minimum_size.x = 130
	enc_row.add_child(enc_label)
	_encrypt_option = OptionButton.new()
	_encrypt_option.add_item("None (dev default)")
	_encrypt_option.add_item("License encrypt (/l)")
	_encrypt_option.add_item("Custom key (/lk)")
	_encrypt_option.item_selected.connect(_on_encrypt_changed)
	enc_row.add_child(_encrypt_option)

	_encrypt_key_edit = _add_path_field(root, "EKB Key File",
		"Path to encryption key bundle file", false)
	_encrypt_key_edit.get_parent().visible = false

	var compat_row := HBoxContainer.new()
	root.add_child(compat_row)
	var compat_label := Label.new()
	compat_label.text = "Update Compat"
	compat_label.custom_minimum_size.x = 130
	compat_row.add_child(compat_label)
	_updcompat_option = OptionButton.new()
	_updcompat_option.add_item("3 — Sub-file granularity (default)")
	_updcompat_option.add_item("2 — File-level granularity")
	_updcompat_option.add_item("1 — Legacy")
	compat_row.add_child(_updcompat_option)

	root.add_child(HSeparator.new())

	# ── Action Buttons ──
	_add_section_header(root, "Packaging Actions")

	var action_row := HBoxContainer.new()
	root.add_child(action_row)

	_genmap_btn = Button.new()
	_genmap_btn.text = "Generate Map"
	_genmap_btn.pressed.connect(_on_genmap)
	action_row.add_child(_genmap_btn)

	_validate_btn = Button.new()
	_validate_btn.text = "Validate"
	_validate_btn.pressed.connect(_on_validate)
	action_row.add_child(_validate_btn)

	_pack_btn = Button.new()
	_pack_btn.text = "Create Package"
	_pack_btn.pressed.connect(_on_pack)
	action_row.add_child(_pack_btn)


# ── UI Helpers ──────────────────────────────────────────────────────────────

## Adds a bold 14pt section label to the given parent container.
func _add_section_header(parent: Control, text: String) -> void:
	var label := Label.new()
	label.text = text
	label.add_theme_font_size_override("font_size", 14)
	parent.add_child(label)

## Creates a labeled path field with a browse button. Returns the LineEdit.
func _add_path_field(parent: VBoxContainer, label_text: String,
		placeholder: String, is_dir: bool) -> LineEdit:
	var row := HBoxContainer.new()
	parent.add_child(row)
	var label := Label.new()
	label.text = label_text
	label.custom_minimum_size.x = 130
	row.add_child(label)
	var edit := LineEdit.new()
	edit.placeholder_text = placeholder
	edit.size_flags_horizontal = SIZE_EXPAND_FILL
	row.add_child(edit)
	var browse := Button.new()
	browse.text = "..."
	browse.pressed.connect(_make_browse_callback(edit, is_dir))
	row.add_child(browse)
	return edit

## Returns a Callable that opens a file/directory dialog and sets the edit text.
func _make_browse_callback(edit: LineEdit, is_dir: bool) -> Callable:
	return func():
		var dialog := FileDialog.new()
		if is_dir:
			dialog.file_mode = FileDialog.FILE_MODE_OPEN_DIR
		else:
			dialog.file_mode = FileDialog.FILE_MODE_OPEN_FILE
		dialog.access = FileDialog.ACCESS_FILESYSTEM
		add_child(dialog)
		dialog.popup_centered(Vector2i(700, 500))

		if is_dir:
			dialog.dir_selected.connect(func(dir: String):
				edit.text = dir
				_save_packaging_settings()
				dialog.queue_free())
		else:
			dialog.file_selected.connect(func(path: String):
				edit.text = path
				_save_packaging_settings()
				dialog.queue_free())

		dialog.canceled.connect(func(): dialog.queue_free())

## Enables or disables the packaging action buttons based on GDK availability.
func _set_actions_enabled(enabled: bool) -> void:
	_genmap_btn.disabled = not enabled
	_validate_btn.disabled = not enabled
	_pack_btn.disabled = not enabled
	_edit_config_btn.disabled = not enabled

## Prints a message to Godot's Output panel with the [GDK Packaging] prefix.
func _log(text: String) -> void:
	print("[GDK Packaging] ", text)

## Logs the stdout/stderr/exit_code from a tool execution result dict.
func _log_result(result: Dictionary) -> void:
	if result["stdout"] != "":
		_log(result["stdout"])
	if result["stderr"] != "":
		_log("[stderr] " + result["stderr"])
		push_warning("[GDK Packaging] " + result["stderr"])
	if result["exit_code"] == 0:
		_log("Completed successfully (exit code 0)")
	else:
		_log("Failed with exit code " + str(result["exit_code"]))
		push_error("[GDK Packaging] Command failed with exit code " + str(result["exit_code"]))


# ── Sandbox ─────────────────────────────────────────────────────────────────

## Queries XblPCSandbox /get and updates the sandbox status label.
func _refresh_sandbox_status() -> void:
	var sandbox_exe = _toolchain.get_sandbox_path()
	if sandbox_exe == "":
		_sandbox_label.text = "Current: XblPCSandbox.exe not found"
		_sandbox_set_btn.disabled = true
		_sandbox_retail_btn.disabled = true
		return

	var result = _toolchain.execute_tool(sandbox_exe, PackedStringArray(["/get"]))
	if result["exit_code"] == 0:
		var output: String = result["stdout"].strip_edges()
		# XblPCSandbox /get outputs: "Current Xbox Live sandbox: <name>"
		var idx = output.find(":")
		if idx >= 0:
			var sandbox_name = output.substr(idx + 1).strip_edges()
			_sandbox_label.text = "Current: %s" % sandbox_name
			if sandbox_name != "" and sandbox_name != "RETAIL":
				_sandbox_id_edit.text = sandbox_name
		else:
			_sandbox_label.text = "Current: %s" % output
	else:
		_sandbox_label.text = "Current: could not determine"
	_sandbox_set_btn.disabled = false
	_sandbox_retail_btn.disabled = false
	_refresh_dev_account()

## Sets the PC sandbox via XblPCSandbox /set with /noApps flag.
func _on_sandbox_set() -> void:
	var sandbox_id = _sandbox_id_edit.text.strip_edges()
	if sandbox_id == "":
		_sandbox_label.text = "Enter a sandbox ID first"
		return

	_sandbox_label.text = "Switching to %s..." % sandbox_id
	_sandbox_set_btn.disabled = true
	_sandbox_retail_btn.disabled = true
	_log("Setting sandbox to: %s" % sandbox_id)

	var sandbox_exe = _toolchain.get_sandbox_path()
	var result = _toolchain.execute_tool(sandbox_exe, PackedStringArray(["/set", sandbox_id, "/noApps"]))
	if result["exit_code"] == 0:
		_log("Sandbox set to %s" % sandbox_id)
	else:
		_log("Sandbox switch failed: %s" % result["stdout"])
		push_warning("[GDK] Sandbox switch failed — may need admin privileges")
	_refresh_sandbox_status()

## Switches back to RETAIL sandbox via XblPCSandbox /retail.
func _on_sandbox_retail() -> void:
	_sandbox_label.text = "Switching to RETAIL..."
	_sandbox_set_btn.disabled = true
	_sandbox_retail_btn.disabled = true
	_log("Switching sandbox to RETAIL")

	var sandbox_exe = _toolchain.get_sandbox_path()
	var result = _toolchain.execute_tool(sandbox_exe, PackedStringArray(["/retail", "/noApps"]))
	if result["exit_code"] == 0:
		_log("Sandbox set to RETAIL")
	else:
		_log("Sandbox switch failed: %s" % result["stdout"])
		push_warning("[GDK] Sandbox switch failed — may need admin privileges")
	_refresh_sandbox_status()


## Queries XblDevAccount show and displays the signed-in Partner Center email.
func _refresh_dev_account() -> void:
	var dev_exe = _toolchain.get_dev_account_path()
	if dev_exe == "":
		_dev_account_label.text = "XblDevAccount.exe not found"
		return

	var result = _toolchain.execute_tool(dev_exe, PackedStringArray(["show"]))
	if result["exit_code"] == 0:
		var output: String = result["stdout"].strip_edges()
		if output.contains("is currently signed in"):
			# XblDevAccount show outputs: "Microsoft Partner Center account <email> from <source> is currently signed in."
			var email_start= output.find("account ") + 8
			var email_end = output.find(" from")
			if email_start > 8 and email_end > email_start:
				var email = output.substr(email_start, email_end - email_start)
				_dev_account_label.text = "✅ Signed in: %s" % email
			else:
				_dev_account_label.text = "✅ Signed in"
		elif output.contains("No account"):
			_dev_account_label.text = "⚠️ Not signed in"
		else:
			_dev_account_label.text = output.substr(0, 80)
	else:
		_dev_account_label.text = "⚠️ Not signed in"

## Launches XblDevAccount signin (opens browser auth flow).
func _on_dev_account_signin() -> void:
	var dev_exe = _toolchain.get_dev_account_path()
	if dev_exe == "":
		return
	_dev_account_label.text = "Signing in..."
	_log("Launching Partner Center sign-in...")
	_toolchain.launch_detached(dev_exe, PackedStringArray(["signin"]))
	# Refresh after a delay to pick up the new state
	get_tree().create_timer(5.0).timeout.connect(_refresh_dev_account)

## Signs out the Partner Center dev account via XblDevAccount signout.
func _on_dev_account_signout() -> void:
	var dev_exe = _toolchain.get_dev_account_path()
	if dev_exe == "":
		return
	_dev_account_label.text = "Signing out..."
	var result = _toolchain.execute_tool(dev_exe, PackedStringArray(["signout"]))
	if result["exit_code"] == 0:
		_log("Dev account signed out")
	else:
		_log("Sign out failed: %s" % result["stdout"])
	_refresh_dev_account()

## Launches the XblTestAccountGui.exe tool.
func _on_open_test_accounts() -> void:
	var test_gui = _toolchain.get_bin_dir().path_join("XblTestAccountGui.exe")
	if FileAccess.file_exists(test_gui):
		_toolchain.launch_detached(test_gui, PackedStringArray([]))
		_log("Launched Xbox Live Test Account GUI")
	else:
		_log("XblTestAccountGui.exe not found")
		push_warning("[GDK] XblTestAccountGui.exe not found")


# ── Config Status ───────────────────────────────────────────────────────────

## Parses MicrosoftGame.config, updates status labels, relocates logos, syncs sizes.
func _refresh_config_status() -> void:
	if _config_mgr.config_exists():
		_config_status_label.text = "✅ MicrosoftGame.config found"
		_create_config_btn.visible = false
		var info = _config_mgr.parse_config()
		if info.size() > 0 and info["name"] != "":
			_config_identity_label.text = "%s | %s | v%s" % [
				info.get("display_name", info["name"]),
				info.get("publisher", ""),
				info.get("version", "?"),
			]
			if _product_id_edit.text == "" and info.get("product_id", "") != "":
				_product_id_edit.text = info["product_id"]
		else:
			_config_identity_label.text = "(could not parse identity)"

		_refresh_config_preview(info)

		var relocated = _config_mgr.relocate_logos_to_storelogos()
		if relocated > 0:
			_log("Relocated %d logo(s) to storelogos/" % relocated)

		var synced = _config_mgr.sync_store_logos()
		if synced > 0:
			_log("Synced %d store logo(s) from 480x480 source" % synced)
	else:
		_config_status_label.text = "⚠️ MicrosoftGame.config not found"
		_config_identity_label.text = "Create a template or use GameConfigEditor to get started."
		_create_config_btn.visible = true
		_refresh_config_preview({})


## Populates the config preview grid with parsed values and schema tooltips.
func _refresh_config_preview(info: Dictionary) -> void:
	# Clear existing preview rows
	for child in _config_preview_container.get_children():
		child.queue_free()

	if info.is_empty():
		var empty_label := Label.new()
		empty_label.text = "No config loaded."
		empty_label.add_theme_color_override("font_color", Color(0.6, 0.6, 0.6))
		_config_preview_container.add_child(empty_label)
		return

	# Preview fields with tooltips from the MicrosoftGame.config schema
	var fields := [
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

	for field in fields:
		var label_text: String = field[0]
		var value: String = field[1]
		var tooltip: String = field[2]

		if value == "":
			continue

		var row := HBoxContainer.new()
		_config_preview_container.add_child(row)

		var key_label := Label.new()
		key_label.text = label_text
		key_label.custom_minimum_size.x = 120
		key_label.add_theme_color_override("font_color", Color(0.7, 0.7, 0.7))
		key_label.add_theme_font_size_override("font_size", 14)
		key_label.tooltip_text = tooltip
		key_label.mouse_filter = Control.MOUSE_FILTER_PASS
		row.add_child(key_label)

		var val_label := Label.new()
		val_label.text = value
		val_label.add_theme_font_size_override("font_size", 14)
		val_label.size_flags_horizontal = SIZE_EXPAND_FILL
		val_label.text_overrun_behavior = TextServer.OVERRUN_TRIM_ELLIPSIS
		val_label.tooltip_text = tooltip
		val_label.mouse_filter = Control.MOUSE_FILTER_PASS
		row.add_child(val_label)


# ── Settings Persistence ────────────────────────────────────────────────────

## Restores all dock field values from .gdk_packaging.cfg.
func _load_packaging_settings() -> void:
	var cfg := ConfigFile.new()
	if cfg.load(PACKAGING_SETTINGS_PATH) != OK:
		return
	_source_dir_edit.text = cfg.get_value("packaging", "source_dir", "")
	_map_file_edit.text = cfg.get_value("packaging", "map_file", "")
	_auto_genmap_check.button_pressed = cfg.get_value("packaging", "auto_genmap", true)
	_output_dir_edit.text = cfg.get_value("packaging", "output_dir", "")
	_content_id_edit.text = cfg.get_value("packaging", "content_id", "")
	_product_id_edit.text = cfg.get_value("packaging", "product_id", "")
	_encrypt_option.selected = cfg.get_value("packaging", "encrypt_option", 0)
	_encrypt_key_edit.text = cfg.get_value("packaging", "encrypt_key", "")
	_updcompat_option.selected = cfg.get_value("packaging", "updcompat_option", 0)
	_sandbox_id_edit.text = cfg.get_value("sandbox", "sandbox_id", "")
	_test_account_edit.text = cfg.get_value("sandbox", "test_account", "")
	# Trigger visibility update for encrypt key field
	_on_encrypt_changed(_encrypt_option.selected)
	_on_auto_genmap_toggled(_auto_genmap_check.button_pressed)

## Persists all dock field values to .gdk_packaging.cfg.
func _save_packaging_settings() -> void:
	var cfg := ConfigFile.new()
	cfg.set_value("packaging", "source_dir", _source_dir_edit.text)
	cfg.set_value("packaging", "map_file", _map_file_edit.text)
	cfg.set_value("packaging", "auto_genmap", _auto_genmap_check.button_pressed)
	cfg.set_value("packaging", "output_dir", _output_dir_edit.text)
	cfg.set_value("packaging", "content_id", _content_id_edit.text)
	cfg.set_value("packaging", "product_id", _product_id_edit.text)
	cfg.set_value("packaging", "encrypt_option", _encrypt_option.selected)
	cfg.set_value("packaging", "encrypt_key", _encrypt_key_edit.text)
	cfg.set_value("packaging", "updcompat_option", _updcompat_option.selected)
	cfg.set_value("sandbox", "sandbox_id", _sandbox_id_edit.text)
	cfg.set_value("sandbox", "test_account", _test_account_edit.text)
	cfg.save(PACKAGING_SETTINGS_PATH)

## Connects text_changed/focus_exited/toggled signals to auto-save settings.
func _connect_autosave() -> void:
	var save_fn = func(_arg = null): _save_packaging_settings()
	for edit in [_source_dir_edit, _map_file_edit, _output_dir_edit,
			_content_id_edit, _product_id_edit, _encrypt_key_edit, _sandbox_id_edit,
			_test_account_edit]:
		edit.text_changed.connect(save_fn)
		edit.focus_exited.connect(_save_packaging_settings)
	_auto_genmap_check.toggled.connect(save_fn)
	_encrypt_option.item_selected.connect(save_fn)
	_updcompat_option.item_selected.connect(save_fn)


# ── Packaging Helpers ───────────────────────────────────────────────────────

## Ensures MicrosoftGame.config, logos, and VC14 dependency exist in the content directory.
## makepkg requires these alongside the game files.
func _ensure_config_in_content_dir(content_dir: String) -> bool:
	var project_dir = ProjectSettings.globalize_path("res://")
	var config_src = _config_mgr.get_config_path()
	var config_dest = content_dir.path_join("MicrosoftGame.config")

	if not FileAccess.file_exists(config_src):
		_log("❌ MicrosoftGame.config not found — create one first.")
		return false

	# Read the config, patch it, and write to content dir
	var file = FileAccess.open(config_src, FileAccess.READ)
	if file == null:
		_log("❌ Cannot read MicrosoftGame.config")
		return false
	var content = file.get_as_text()
	file.close()

	# GDK DLLs link against VC++ runtime; makepkg requires this declared as a framework dependency
	if not content.contains('<KnownDependency Name="VC14"/>') and content.contains("</Game>"):
		var dep_xml = '  <DesktopRegistration>\n    <DependencyList>\n      <KnownDependency Name="VC14"/>\n    </DependencyList>\n  </DesktopRegistration>\n'
		content = content.replace("</Game>", dep_xml + "</Game>")
		_log("Added VC14 KnownDependency to config")

	# Patch executable name to match actual exe in content dir
	var dir = DirAccess.open(content_dir)
	if dir:
		dir.list_dir_begin()
		var fname = dir.get_next()
		# Find the main game exe — skip .console.exe (Godot's headless variant, not the game entry point)
		while fname != "":
			if fname.ends_with(".exe") and not fname.ends_with(".console.exe"):
				var regex = RegEx.new()
				regex.compile('Executable Name="[^"]*"')
				if regex.search(content):
					content = regex.sub(content, 'Executable Name="%s"' % fname)
					_log("Patched executable name to: %s" % fname)
				break
			fname = dir.get_next()
		dir.list_dir_end()

	file = FileAccess.open(config_dest, FileAccess.WRITE)
	if file == null:
		_log("❌ Cannot write to content directory")
		return false
	file.store_string(content)
	file.close()
	_log("Copied MicrosoftGame.config to content directory")

	# Parse the config to find where logos are referenced
	var info = _config_mgr.parse_config()
	var logo_keys := {
		"store_logo": "StoreLogo",
		"logo_150": "Square150x150Logo",
		"logo_44": "Square44x44Logo",
		"logo_480": "Square480x480Logo",
		"splash_screen": "SplashScreenImage",
	}

	# Copy each logo to the path the config expects (relative to content dir)
	for key in logo_keys:
		var rel_path: String = info.get(key, "")
		if rel_path == "":
			# Default name at root if not in config
			rel_path = logo_keys[key] + ".png"
		var normalized = rel_path.replace("\\", "/")
		var dest_path = content_dir.path_join(normalized)

		# Find the source — check storelogos/ first, then project root
		var src_path = ""
		var filename = normalized.get_file()
		var storelogos_src = project_dir.path_join("storelogos").path_join(filename)
		var root_src = project_dir.path_join(filename)
		if FileAccess.file_exists(storelogos_src):
			src_path = storelogos_src
		elif FileAccess.file_exists(root_src):
			src_path = root_src

		if src_path != "":
			var dest_dir = dest_path.get_base_dir()
			DirAccess.make_dir_recursive_absolute(dest_dir)
			DirAccess.open(project_dir).copy(src_path, dest_path)

	return true


# ── Signal Handlers ─────────────────────────────────────────────────────────

## Shows/hides the EKB key file field based on encryption option selection.
func _on_encrypt_changed(index: int) -> void:
	# Show EKB key file field only when "Custom key" is selected
	_encrypt_key_edit.get_parent().visible = (index == ENCRYPT_CUSTOM_KEY)

## Toggles map file field editability when auto-generate checkbox changes.
func _on_auto_genmap_toggled(pressed: bool) -> void:
	_map_file_edit.editable = not pressed
	if pressed:
		_map_file_edit.placeholder_text = "Will be auto-generated in output directory"
	else:
		_map_file_edit.placeholder_text = "XML mapping file path"

## Launches GameConfigEditor with the project's MicrosoftGame.config.
func _on_edit_config() -> void:
	if not _config_mgr.config_exists():
		_log("MicrosoftGame.config not found — create one first.")
		push_warning("[GDK Packaging] MicrosoftGame.config not found — create one first.")
		return
	var pid = _config_mgr.launch_editor()
	if pid >= 0:
		_log("Launched GameConfigEditor (PID: %d)" % pid)
	else:
		_log("Failed to launch GameConfigEditor")
		push_error("[GDK Packaging] Failed to launch GameConfigEditor")

## Creates a template MicrosoftGame.config and triggers filesystem rescan.
func _on_create_config() -> void:
	var err = _config_mgr.create_template()
	if err == OK:
		_log("Created template MicrosoftGame.config in project root")
		_refresh_config_status()
		# Request a filesystem rescan so the file appears in the dock.
		# The scan is async — the file will appear after a short delay.
		var fs = EditorInterface.get_resource_filesystem()
		if not fs.is_scanning():
			fs.scan()
	elif err == ERR_ALREADY_EXISTS:
		_log("MicrosoftGame.config already exists")
		push_warning("[GDK Packaging] MicrosoftGame.config already exists")
	else:
		_log("Failed to create MicrosoftGame.config: " + error_string(err))
		push_error("[GDK Packaging] Failed to create MicrosoftGame.config: " + error_string(err))

## Opens the project root folder in the OS file manager.
func _on_open_config_folder() -> void:
	var folder_path = ProjectSettings.globalize_path("res://")
	OS.shell_open(folder_path)

## Generates a layout.xml mapping file, with overwrite confirmation if it exists.
func _on_genmap() -> void:
	var source := _source_dir_edit.text.strip_edges()
	if source == "":
		_log("❌ Content directory is required for genmap.")
		return
	var output := _output_dir_edit.text.strip_edges()
	if output == "":
		output = source
	var map_path := output.path_join("layout.xml")

	# Confirm overwrite if layout.xml already exists
	if FileAccess.file_exists(map_path):
		var confirm := ConfirmationDialog.new()
		confirm.dialog_text = "layout.xml already exists at:\n%s\n\nOverwrite it?" % map_path
		confirm.title = "Overwrite Mapping File?"
		confirm.confirmed.connect(func():
			_do_genmap(source, map_path)
			confirm.queue_free())
		confirm.canceled.connect(func(): confirm.queue_free())
		add_child(confirm)
		confirm.popup_centered()
		return

	_do_genmap(source, map_path)

## Runs makepkg genmap and updates the map file field on success.
func _do_genmap(source: String, map_path: String) -> void:
	_log("Generating mapping file...")
	var result = _makepkg.genmap(source, map_path)
	_log_result(result)
	if result["exit_code"] == 0:
		_map_file_edit.text = map_path

## Validates the package layout with a progress dialog.
func _on_validate() -> void:
	var source := _source_dir_edit.text.strip_edges()
	var map_file := _map_file_edit.text.strip_edges()
	var output := _output_dir_edit.text.strip_edges()
	if source == "" or map_file == "":
		_log("❌ Content directory and mapping file are required for validation.")
		return
	if output == "":
		output = source
	if not _ensure_config_in_content_dir(source):
		return

	var progress := AcceptDialog.new()
	progress.title = "Validating Package"
	progress.dialog_text = "Validating package, this may take a minute..."
	progress.get_ok_button().visible = false
	add_child(progress)
	progress.popup_centered(Vector2i(450, 150))

	# Wait two frames so the dialog fully renders before the blocking call
	await get_tree().process_frame
	await get_tree().process_frame

	_log("Validating package layout...")
	var result = _makepkg.validate(map_file, source, output)
	_log_result(result)

	progress.get_ok_button().visible = true
	if result["exit_code"] == 0:
		progress.dialog_text = "✅ Package validation passed!"
	else:
		progress.dialog_text = "❌ Package validation failed.\nCheck the Output panel for details."
	progress.confirmed.connect(func(): progress.queue_free())

## Creates an MSIXVC package with progress dialog, auto-genmap, and options.
func _on_pack() -> void:
	var source := _source_dir_edit.text.strip_edges()
	var output := _output_dir_edit.text.strip_edges()
	if source == "":
		_log("❌ Content directory is required.")
		return
	if output == "":
		_log("❌ Output directory is required.")
		return
	if not _ensure_config_in_content_dir(source):
		return

	var progress := AcceptDialog.new()
	progress.title = "Creating Package"
	progress.dialog_text = "Creating MSIXVC package...\nThis may take a minute."
	progress.get_ok_button().visible = false
	add_child(progress)
	progress.popup_centered(Vector2i(400, 120))

	# Defer the blocking call so the dialog renders
	await get_tree().process_frame

	# Auto-generate mapping file if checkbox is on
	var map_file := _map_file_edit.text.strip_edges()
	if _auto_genmap_check.button_pressed or map_file == "":
		var map_path := output.path_join("layout.xml")

		# Confirm overwrite of layout.xml during pack flow
		if FileAccess.file_exists(map_path):
			_log("Overwriting existing layout.xml for packaging...")

		progress.dialog_text = "Generating mapping file..."
		await get_tree().process_frame

		var genmap_result = _makepkg.genmap(source, map_path)
		_log_result(genmap_result)
		if genmap_result["exit_code"] != 0:
			_log("❌ Mapping file generation failed — aborting package.")
			progress.dialog_text = "❌ Mapping file generation failed."
			progress.get_ok_button().visible = true
			progress.confirmed.connect(func(): progress.queue_free())
			return
		map_file = map_path
		_map_file_edit.text = map_file

	progress.dialog_text = "Creating MSIXVC package...\nThis may take a minute."
	await get_tree().process_frame

	# Build options
	var options := {}
	if _content_id_edit.text.strip_edges() != "":
		options["content_id"] = _content_id_edit.text.strip_edges()
	if _product_id_edit.text.strip_edges() != "":
		options["product_id"] = _product_id_edit.text.strip_edges()

	# Map encryption OptionButton selection to makepkg flags
	match _encrypt_option.selected:
		ENCRYPT_LICENSE:
			options["encrypt"] = true
		ENCRYPT_CUSTOM_KEY:
			options["encrypt_key"] = _encrypt_key_edit.text.strip_edges()

	var updcompat_map := [3, 2, 1]  # Maps OptionButton index → /updcompat value (reversed: index 0=level 3, 1=level 2, 2=level 1)
	options["updcompat"] = updcompat_map[_updcompat_option.selected]

	_log("Creating MSIXVC package...")
	var result = _makepkg.pack(source, map_file, output, options)
	_log_result(result)

	progress.get_ok_button().visible = true
	if result["exit_code"] == 0:
		progress.dialog_text = "✅ Package created successfully!"
	else:
		progress.dialog_text = "❌ Package creation failed.\nCheck the Output panel for details."
	progress.confirmed.connect(func(): progress.queue_free())


# ── Achievements ────────────────────────────────────────────────────────────

## Loads the demo achievement ID from sample_config.cfg.
func _load_achievement_config() -> void:
	var cfg := ConfigFile.new()
	if cfg.load(SAMPLE_CONFIG_PATH) == OK:
		var val = cfg.get_value("achievements", "demo_achievement_id", "")
		_achievement_id_edit.text = str(val)
		_achievement_status_label.text = "Loaded from sample_config.cfg"
	else:
		_achievement_status_label.text = "No sample_config.cfg — enter a value and save."

## Saves the demo achievement ID to sample_config.cfg.
func _on_achievement_save() -> void:
	var cfg := ConfigFile.new()
	cfg.load(SAMPLE_CONFIG_PATH)
	cfg.set_value("achievements", "demo_achievement_id", _achievement_id_edit.text.strip_edges())
	var err = cfg.save(SAMPLE_CONFIG_PATH)
	if err == OK:
		_achievement_status_label.text = "✅ Saved to sample_config.cfg"
		_log("Achievement config saved")
		var fs = EditorInterface.get_resource_filesystem()
		if not fs.is_scanning():
			fs.scan()
	else:
		_achievement_status_label.text = "Failed to save: " + error_string(err)
		push_error("[GDK] Failed to save achievement config: " + error_string(err))


# ── PlayFab ─────────────────────────────────────────────────────────────────

## Loads the PlayFab Title ID from sample_pf_config.cfg.
func _load_playfab_config() -> void:
	var cfg := ConfigFile.new()
	if cfg.load(PLAYFAB_CONFIG_PATH) == OK:
		var val = cfg.get_value("playfab", "title_id", "")
		_playfab_title_id_edit.text = str(val)
		if val != "":
			_playfab_status_label.text = "Loaded from sample_pf_config.cfg"
		else:
			_playfab_status_label.text = "No PlayFab Title ID set — enter one and save."
	else:
		_playfab_status_label.text = "No sample_pf_config.cfg — enter a value and save."

## Saves the PlayFab Title ID to sample_pf_config.cfg.
func _on_playfab_save() -> void:
	var cfg := ConfigFile.new()
	cfg.load(PLAYFAB_CONFIG_PATH)
	cfg.set_value("playfab", "title_id", _playfab_title_id_edit.text.strip_edges())
	var err = cfg.save(PLAYFAB_CONFIG_PATH)
	if err == OK:
		_playfab_status_label.text = "✅ Saved to sample_pf_config.cfg"
		_log("PlayFab Title ID saved")
		var fs = EditorInterface.get_resource_filesystem()
		if not fs.is_scanning():
			fs.scan()
	else:
		_playfab_status_label.text = "Failed to save: " + error_string(err)
		push_error("[GDK] Failed to save PlayFab config: " + error_string(err))

## Reads PlayFabCore.dll product version via PowerShell.
func _detect_playfab_version() -> void:
	# Look for PlayFabCore.dll in the project's addon bin directory
	var search_paths := [
		"res://addons/godot_playfab/bin/PlayFabCore.dll",
	]
	var dll_path := ""
	for p in search_paths:
		var global_p = ProjectSettings.globalize_path(p)
		if FileAccess.file_exists(global_p):
			dll_path = global_p
			break

	if dll_path == "":
		_playfab_version_label.text = "PlayFab SDK: not found"
		return

	# GDScript can't read DLL metadata directly; use PowerShell to extract ProductVersion
	var output: Array = []
	var ps_cmd= "(Get-Item '%s').VersionInfo.ProductVersion" % dll_path.replace("'", "''")
	var exit_code = OS.execute("powershell", PackedStringArray(["-NoProfile", "-Command", ps_cmd]), output, true, false)
	if exit_code == 0 and output.size() > 0:
		var version = str(output[0]).strip_edges()
		if version != "":
			_playfab_version_label.text = "PlayFab SDK: %s" % version
			return

	_playfab_version_label.text = "PlayFab SDK: installed (version unknown)"
