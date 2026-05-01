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

# Export
var _preset_selector: OptionButton
var _clean_build_check: CheckBox
var _export_btn: Button
var _register_btn: Button
var _export_register_btn: Button
var _export_package_btn: Button
var _export_status_label: Label

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

# Install & Launch
var _install_btn: Button
var _uninstall_btn: Button
var _install_status_label: Label
var _app_selector: OptionButton
var _launch_btn: Button
var _terminate_btn: Button
var _launch_status_label: Label

# Output
var _status_label: Label

# Logo watcher
var _watch_timer: float = 0.0
const WATCH_INTERVAL := 2.0
const ROOT_LOGO_FILES := [
	"StoreLogo.png",
	"Square44x44Logo.png",
	"Square150x150Logo.png",
	"Square480x480Logo.png",
	"SplashScreenImage.png",
]


func _ready() -> void:
	_toolchain = GDKToolchainScript.new()
	_makepkg = MakePkgExecutorScript.new(_toolchain)
	_config_mgr = GameConfigManagerScript.new(_toolchain)
	_build_ui()
	_load_packaging_settings()
	_refresh_sandbox_status()
	_refresh_config_status()
	set_process(true)


func _process(delta: float) -> void:
	_watch_timer += delta
	if _watch_timer < WATCH_INTERVAL:
		return
	_watch_timer = 0.0
	_check_and_relocate_root_logos()


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
	tab_bar.add_tab("🔒 Sandbox")
	tab_bar.add_tab("📦 Export & Package")
	tab_bar.add_tab("🚀 Install & Launch")
	tab_bar.add_tab("🏆 Achievements")
	tab_bar.add_tab("☁️ PlayFab")
	tab_bar.clip_tabs = false
	tab_bar.size_flags_horizontal = SIZE_EXPAND_FILL
	tab_bar.add_theme_font_size_override("font_size", 18)
	outer.add_child(tab_bar)

	# ── Content pages (one per tab, order must match tab_bar) ──
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

	var install_launch_scroll := ScrollContainer.new()
	install_launch_scroll.horizontal_scroll_mode = ScrollContainer.SCROLL_MODE_DISABLED
	install_launch_scroll.size_flags_vertical = SIZE_EXPAND_FILL
	install_launch_scroll.visible = false
	outer.add_child(install_launch_scroll)
	var install_launch := VBoxContainer.new()
	install_launch.size_flags_horizontal = SIZE_EXPAND_FILL
	install_launch_scroll.add_child(install_launch)
	_build_install_launch_ui(install_launch)
	_tab_pages.append(install_launch_scroll)

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

## Builds the PlayFab tab: Title ID field, Game Manager link, SDK version.
	# PlayFab SDK version
	_playfab_version_label = Label.new()
	_playfab_version_label.text = "PlayFab SDK: detecting..."
	_playfab_version_label.add_theme_font_size_override("font_size", 12)
	_playfab_version_label.add_theme_color_override("font_color", Color(0.7, 0.7, 0.7))
	root.add_child(_playfab_version_label)
	_detect_playfab_version()


## Builds the Export & Package tab: export section at top, packaging below.
func _build_packaging_ui(root: VBoxContainer) -> void:
	# ── Export ──
	_add_section_header(root, "Export Presets & Actions")

	# Preset selector row
	var preset_row := HBoxContainer.new()
	root.add_child(preset_row)
	var preset_label := Label.new()
	preset_label.text = "Export Preset"
	preset_label.custom_minimum_size.x = 130
	preset_row.add_child(preset_label)
	_preset_selector = OptionButton.new()
	_preset_selector.size_flags_horizontal = SIZE_EXPAND_FILL
	_preset_selector.tooltip_text = "Select a Windows Desktop export preset"
	preset_row.add_child(_preset_selector)

	# Clean build checkbox
	_clean_build_check = CheckBox.new()
	_clean_build_check.text = "Clean Build/ folder before export"
	_clean_build_check.button_pressed = false
	root.add_child(_clean_build_check)

	# Export buttons row
	var export_btn_row := HBoxContainer.new()
	root.add_child(export_btn_row)

	_export_btn = Button.new()
	_export_btn.text = "Export Build"
	_export_btn.tooltip_text = "Export to Build/ folder using the selected preset"
	_export_btn.pressed.connect(_on_export)
	export_btn_row.add_child(_export_btn)

	_export_register_btn = Button.new()
	_export_register_btn.text = "Export + Register"
	_export_register_btn.tooltip_text = "Export then register for immediate testing"
	_export_register_btn.pressed.connect(_on_export_and_register)
	export_btn_row.add_child(_export_register_btn)

	_register_btn = Button.new()
	_register_btn.text = "Register Build"
	_register_btn.tooltip_text = "Register the Build/ folder with wdapp for fast dev iteration"
	_register_btn.pressed.connect(_on_register_loose)
	export_btn_row.add_child(_register_btn)

	_export_status_label = Label.new()
	_export_status_label.text = ""
	root.add_child(_export_status_label)

	root.add_child(HSeparator.new())

	# ── Package Source Configuration ──
	_add_section_header(root, "Package Source Configuration")

	_source_dir_edit = _add_path_field(root, "Content Directory",
		"Directory with exported game files", true)
	_add_open_folder_btn(_source_dir_edit)
	_map_file_edit = _add_path_field(root, "Mapping File",
		"XML mapping file (or auto-generate)", false)

	_auto_genmap_check = CheckBox.new()
	_auto_genmap_check.text = "Auto-generate mapping file before packaging"
	_auto_genmap_check.button_pressed = true
	_auto_genmap_check.toggled.connect(_on_auto_genmap_toggled)
	root.add_child(_auto_genmap_check)

	_output_dir_edit = _add_path_field(root, "Output Directory",
		"Package/ (default)", true)
	_add_open_folder_btn(_output_dir_edit)

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

	_export_package_btn = Button.new()
	_export_package_btn.text = "Export & Package"
	_export_package_btn.tooltip_text = "Export project then create MSIXVC package in one step"
	_export_package_btn.pressed.connect(_on_export_and_package)
	action_row.add_child(_export_package_btn)

	_pack_btn = Button.new()
	_pack_btn.text = "Create Package Only"
	_pack_btn.pressed.connect(_on_pack)
	action_row.add_child(_pack_btn)

	_validate_btn = Button.new()
	_validate_btn.text = "Validate Package"
	_validate_btn.pressed.connect(_on_validate)
	action_row.add_child(_validate_btn)

	_genmap_btn = Button.new()
	_genmap_btn.text = "Generate Map"
	_genmap_btn.pressed.connect(_on_genmap)
	action_row.add_child(_genmap_btn)

	_populate_preset_selector()


## Populates the export preset dropdown with available Windows Desktop presets.
## Populates the export preset dropdown by parsing export_presets.cfg directly.
func _populate_preset_selector() -> void:
	_preset_selector.clear()
	var cfg_path = ProjectSettings.globalize_path("res://export_presets.cfg")
	if not FileAccess.file_exists(cfg_path):
		_preset_selector.add_item("No export presets — add one in Project → Export")
		_export_btn.disabled = true
		_export_register_btn.disabled = true
		_export_package_btn.disabled = true
		return

	# Parse preset names from export_presets.cfg
	var file = FileAccess.open(cfg_path, FileAccess.READ)
	var content = file.get_as_text()
	file.close()

	var found_any := false
	var regex = RegEx.new()
	regex.compile('\\[preset\\.(\\d+)\\][\\s\\S]*?name="([^"]*)"[\\s\\S]*?platform="([^"]*)"')
	for result in regex.search_all(content):
		var idx = int(result.get_string(1))
		var name = result.get_string(2)
		var platform = result.get_string(3)
		if platform == "Windows Desktop":
			_preset_selector.add_item(name, idx)
			found_any = true

	if not found_any:
		_preset_selector.add_item("No Windows preset — add one in Project → Export")
		_export_btn.disabled = true
		_export_register_btn.disabled = true
		_export_package_btn.disabled = true


# ── UI Helpers ──────────────────────────────────────────────────────────────

func _add_section_header(parent: Control, text: String) -> void:
	var label := Label.new()
	label.text = text
	label.add_theme_font_size_override("font_size", 14)
	parent.add_child(label)

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

## Adds a folder-open button to the row of the given LineEdit field.
func _add_open_folder_btn(edit: LineEdit) -> void:
	var row = edit.get_parent()
	var open_btn := Button.new()
	open_btn.text = "📂"
	open_btn.tooltip_text = "Open folder in file manager"
	open_btn.pressed.connect(func():
		var path = edit.text.strip_edges()
		if path != "" and DirAccess.dir_exists_absolute(path):
			OS.shell_open(path)
		elif path != "":
			push_warning("[GDK] Directory not found: " + path)
	)
	row.add_child(open_btn)

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

func _set_actions_enabled(enabled: bool) -> void:
	_genmap_btn.disabled = not enabled
	_validate_btn.disabled = not enabled
	_pack_btn.disabled = not enabled
	_edit_config_btn.disabled = not enabled

func _log(text: String) -> void:
	print("[GDK Packaging] ", text)

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
		# Parse the sandbox name from output like "Current Xbox Live sandbox: XDKS.1"
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


func _refresh_dev_account() -> void:
	var dev_exe = _toolchain.get_dev_account_path()
	if dev_exe == "":
		_dev_account_label.text = "XblDevAccount.exe not found"
		return

	var result = _toolchain.execute_tool(dev_exe, PackedStringArray(["show"]))
	if result["exit_code"] == 0:
		var output: String = result["stdout"].strip_edges()
		if output.contains("is currently signed in"):
			# Parse email from "Microsoft Partner Center account <email> from <source> is currently signed in."
			var email_start = output.find("account ") + 8
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

func _on_dev_account_signin() -> void:
	var dev_exe = _toolchain.get_dev_account_path()
	if dev_exe == "":
		return
	_dev_account_label.text = "Signing in..."
	_log("Launching Partner Center sign-in...")
	_toolchain.launch_detached(dev_exe, PackedStringArray(["signin"]))
	# Refresh after a delay to pick up the new state
	get_tree().create_timer(5.0).timeout.connect(_refresh_dev_account)

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

func _on_open_test_accounts() -> void:
	var test_gui = _toolchain.get_bin_dir().path_join("XblTestAccountGui.exe")
	if FileAccess.file_exists(test_gui):
		_toolchain.launch_detached(test_gui, PackedStringArray([]))
		_log("Launched Xbox Live Test Account GUI")
	else:
		_log("XblTestAccountGui.exe not found")
		push_warning("[GDK] XblTestAccountGui.exe not found")


# ── Config Status ───────────────────────────────────────────────────────────

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
	_encrypt_option.selected = cfg.get_value("packaging", "encrypt_option", ENCRYPT_NONE)
	_encrypt_key_edit.text = cfg.get_value("packaging", "encrypt_key", "")
	_updcompat_option.selected = cfg.get_value("packaging", "updcompat_option", 0)
	_sandbox_id_edit.text = cfg.get_value("sandbox", "sandbox_id", "")
	_test_account_edit.text = cfg.get_value("sandbox", "test_account", "")
	# Export settings
	_clean_build_check.button_pressed = cfg.get_value("export", "clean_build", false)
	var saved_preset = cfg.get_value("export", "preset_name", "")
	# Select the saved preset after populating
	for i in _preset_selector.get_item_count():
		if _preset_selector.get_item_text(i) == saved_preset:
			_preset_selector.select(i)
			break
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
	# Export settings
	if _preset_selector.selected >= 0:
		cfg.set_value("export", "preset_name", _preset_selector.get_item_text(_preset_selector.selected))
	cfg.set_value("export", "clean_build", _clean_build_check.button_pressed)
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
	_preset_selector.item_selected.connect(save_fn)
	_clean_build_check.toggled.connect(save_fn)


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

	# GDK DLLs link against VC++ runtime; makepkg requires this declared as a framework dependency.
	if not content.contains('<KnownDependency Name="VC14"/>') and content.contains("</Game>"):
		var dep_xml = '  <DesktopRegistration>\n    <DependencyList>\n      <KnownDependency Name="VC14"/>\n    </DependencyList>\n  </DesktopRegistration>\n'
		content = content.replace("</Game>", dep_xml + "</Game>")
		_log("Added VC14 KnownDependency to config")

	# Patch executable name to match actual exe in content dir
	var dir = DirAccess.open(content_dir)
	if dir:
		dir.list_dir_begin()
		var fname = dir.get_next()
		# Find the main game exe - skip .console.exe (Godot's headless variant, not the game entry point).
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

func _on_encrypt_changed(index: int) -> void:
	# Show EKB key file field only when "Custom key" is selected
	_encrypt_key_edit.get_parent().visible = (index == ENCRYPT_CUSTOM_KEY)

func _on_auto_genmap_toggled(pressed: bool) -> void:
	_map_file_edit.editable = not pressed
	if pressed:
		_map_file_edit.placeholder_text = "Will be auto-generated in output directory"
	else:
		_map_file_edit.placeholder_text = "XML mapping file path"

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

func _on_open_config_folder() -> void:
	var folder_path = ProjectSettings.globalize_path("res://")
	OS.shell_open(folder_path)

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
	# Ensure the output directory for the layout file exists
	DirAccess.make_dir_recursive_absolute(map_path.get_base_dir())
	_log("Generating mapping file...")
	var result = _makepkg.genmap(source, map_path)
	_log_result(result)
	if result["exit_code"] == 0:
		_map_file_edit.text = map_path

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
	progress.exclusive = false
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
	progress.exclusive = false
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
		DirAccess.make_dir_recursive_absolute(output)

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

	match _encrypt_option.selected:
		ENCRYPT_LICENSE:
			options["encrypt"] = true
		ENCRYPT_CUSTOM_KEY:
			options["encrypt_key"] = _encrypt_key_edit.text.strip_edges()

	var updcompat_map := [3, 2, 1]
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


# ── Export ───────────────────────────────────────────────────────────────────

## Returns the absolute path to the Build/ folder in the project root.
func _get_build_dir() -> String:
	return ProjectSettings.globalize_path("res://Build")

## Returns the absolute path to the Package/ folder in the project root.
func _get_package_dir() -> String:
	return ProjectSettings.globalize_path("res://Package")

## Exports the project using the selected Windows preset to Build/.
func _on_export() -> void:
	var build_dir = _get_build_dir()
	DirAccess.make_dir_recursive_absolute(build_dir)

	if _clean_build_check.button_pressed:
		_clean_directory(build_dir)
		_log("Cleaned Build/ folder")

	var preset_name = _preset_selector.get_item_text(_preset_selector.selected)
	if preset_name == "" or preset_name.begins_with("No "):
		_export_status_label.text = "❌ No valid export preset selected"
		return

	var game_name = ProjectSettings.get_setting("application/config/name", "Game")
	var exe_path = build_dir.path_join(game_name + ".exe")

	var progress := AcceptDialog.new()
	progress.exclusive = false
	progress.title = "Exporting"
	progress.dialog_text = "Exporting project to Build/...\nThis may take a minute."
	progress.get_ok_button().visible = false
	add_child(progress)
	progress.popup_centered(Vector2i(450, 150))

	await get_tree().process_frame
	await get_tree().process_frame

	# Use Godot CLI to export (--export-debug for debug build)
	var godot_path = OS.get_executable_path()
	var project_path = ProjectSettings.globalize_path("res://")
	_log("Exporting '%s' preset to: %s" % [preset_name, exe_path])

	var output: Array = []
	var exit_code = OS.execute(godot_path, PackedStringArray([
		"--headless",
		"--path", project_path,
		"--export-debug", preset_name, exe_path
	]), output, true, false)

	var stdout_text = str(output[0]) if output.size() > 0 else ""

	if exit_code == OK:
		_export_status_label.text = "✅ Exported to Build/"
		_log("Export completed successfully")
		_post_export_prepare(build_dir)
		if _source_dir_edit.text.strip_edges() == "":
			_source_dir_edit.text = build_dir
			_save_packaging_settings()
	else:
		_export_status_label.text = "❌ Export failed (exit code %d)" % exit_code
		_log("Export failed (exit code %d): %s" % [exit_code, stdout_text])
		push_error("[GDK] Export failed (exit code %d)" % exit_code)

	progress.get_ok_button().visible = true
	if exit_code == OK:
		progress.dialog_text = "✅ Export completed!\nBuild files are in the Build/ folder."
	else:
		progress.dialog_text = "❌ Export failed.\nCheck the Output panel for details."
	progress.confirmed.connect(func(): progress.queue_free())

## Runs post-export preparation: copies config, logos, and verifies DLLs.
func _post_export_prepare(build_dir: String) -> void:
	if not _ensure_config_in_content_dir(build_dir):
		_log("⚠️ Post-export config setup had issues")

	if _output_dir_edit.text.strip_edges() == "":
		_output_dir_edit.text = _get_package_dir()
		_save_packaging_settings()

## Registers a loose build with wdapp for fast dev iteration.
func _on_register_loose() -> void:
	var build_dir = _get_build_dir()
	if not DirAccess.dir_exists_absolute(build_dir):
		_export_status_label.text = "❌ Build/ folder not found — export first"
		return

	var wdapp_path = _toolchain.get_bin_dir().path_join("wdapp.exe")
	if not FileAccess.file_exists(wdapp_path):
		_export_status_label.text = "❌ wdapp.exe not found"
		return

	_log("Registering loose build: %s" % build_dir)
	var result = _toolchain.execute_tool(wdapp_path, PackedStringArray(["register", build_dir]))
	if result["exit_code"] == 0:
		_export_status_label.text = "✅ Registered loose build"
		_log("wdapp register succeeded")
	else:
		_export_status_label.text = "❌ Registration failed"
		_log("wdapp register failed: %s" % result["stdout"])
		push_warning("[GDK] wdapp register failed — may need admin privileges")

## Exports then registers in one step.
func _on_export_and_register() -> void:
	await _on_export()
	if _export_status_label.text.begins_with("✅"):
		_on_register_loose()

## Exports then packages in one step.
func _on_export_and_package() -> void:
	await _on_export()
	if _export_status_label.text.begins_with("✅"):
		_source_dir_edit.text = _get_build_dir()
		_output_dir_edit.text = _get_package_dir()
		_save_packaging_settings()
		await _on_pack()

# ── Install & Launch ────────────────────────────────────────────────────────

func _build_install_launch_ui(root: VBoxContainer) -> void:
	# ── Install ──
	_add_section_header(root, "Install")

	var install_desc := Label.new()
	install_desc.text = "Install or uninstall the MSIXVC package from the Package/ folder."
	install_desc.autowrap_mode = TextServer.AUTOWRAP_WORD_SMART
	install_desc.add_theme_font_size_override("font_size", 12)
	install_desc.add_theme_color_override("font_color", Color(0.7, 0.7, 0.7))
	root.add_child(install_desc)

	var install_btn_row := HBoxContainer.new()
	root.add_child(install_btn_row)

	_install_btn = Button.new()
	_install_btn.text = "Install"
	_install_btn.tooltip_text = "Install the MSIXVC package from the Package/ folder"
	_install_btn.pressed.connect(_on_pkg_install)
	install_btn_row.add_child(_install_btn)

	_uninstall_btn = Button.new()
	_uninstall_btn.text = "Uninstall"
	_uninstall_btn.tooltip_text = "Uninstall the selected app"
	_uninstall_btn.pressed.connect(_on_pkg_uninstall)
	install_btn_row.add_child(_uninstall_btn)

	_install_status_label = Label.new()
	_install_status_label.text = ""
	root.add_child(_install_status_label)

	root.add_child(HSeparator.new())

	# ── Launch ──
	_add_section_header(root, "Launch")

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
	_app_selector = OptionButton.new()
	_app_selector.size_flags_horizontal = SIZE_EXPAND_FILL
	_app_selector.tooltip_text = "Select a registered app to launch or terminate"
	app_row.add_child(_app_selector)

	var refresh_btn := Button.new()
	refresh_btn.text = "Refresh"
	refresh_btn.tooltip_text = "Refresh the list of registered apps"
	refresh_btn.pressed.connect(_refresh_registered_apps)
	app_row.add_child(refresh_btn)

	var launch_btn_row := HBoxContainer.new()
	root.add_child(launch_btn_row)

	_launch_btn = Button.new()
	_launch_btn.text = "Launch"
	_launch_btn.tooltip_text = "Launch the selected app"
	_launch_btn.pressed.connect(_on_app_launch)
	launch_btn_row.add_child(_launch_btn)

	_terminate_btn = Button.new()
	_terminate_btn.text = "Terminate"
	_terminate_btn.tooltip_text = "Terminate the selected app"
	_terminate_btn.pressed.connect(_on_app_terminate)
	launch_btn_row.add_child(_terminate_btn)

	_launch_status_label = Label.new()
	_launch_status_label.text = ""
	root.add_child(_launch_status_label)

	# Auto-populate on load
	_refresh_registered_apps()


## Stores AUMID and PackageFullName for each registered app entry.
var _registered_apps: Array[Dictionary] = []

## Populates the app selector dropdown with registered apps from wdapp list.
func _refresh_registered_apps() -> void:
	_app_selector.clear()
	_registered_apps.clear()

	var wdapp_path = _toolchain.get_bin_dir().path_join("wdapp.exe")
	if not FileAccess.file_exists(wdapp_path):
		_app_selector.add_item("wdapp.exe not found")
		_launch_btn.disabled = true
		_terminate_btn.disabled = true
		return

	var result = _toolchain.execute_tool(wdapp_path, PackedStringArray(["list"]))
	if result["exit_code"] != 0:
		_app_selector.add_item("Failed to list apps")
		return

	# Parse wdapp list output:
	# PackageFullName lines have no "!" and contain "_"
	# AUMID lines have "!" and are indented under the PackageFullName
	var current_pfn := ""
	for line in result["stdout"].split("\n"):
		var trimmed = line.strip_edges()
		if trimmed == "" or trimmed.begins_with("Run this") or trimmed.begins_with("The operation") or trimmed.begins_with("Registered"):
			continue
		if trimmed.contains("!"):
			# This is an AUMID line
			if current_pfn != "":
				_registered_apps.append({"pfn": current_pfn, "aumid": trimmed})
				_app_selector.add_item(trimmed)
		elif trimmed.contains("_"):
			# This is a PackageFullName line
			current_pfn = trimmed

	if _registered_apps.is_empty():
		_app_selector.add_item("No registered apps")
		_launch_btn.disabled = true
		_terminate_btn.disabled = true
	else:
		_launch_btn.disabled = false
		_terminate_btn.disabled = false


## Gets the selected app's AUMID from the dropdown.
func _get_selected_aumid() -> String:
	var idx = _app_selector.selected
	if idx < 0 or idx >= _registered_apps.size():
		return ""
	return _registered_apps[idx]["aumid"]

## Gets the selected app's PackageFullName from the dropdown.
func _get_selected_pfn() -> String:
	var idx = _app_selector.selected
	if idx < 0 or idx >= _registered_apps.size():
		return ""
	return _registered_apps[idx]["pfn"]

func _on_app_launch() -> void:
	var aumid = _get_selected_aumid()
	if aumid == "":
		_launch_status_label.text = "❌ No app selected"
		return
	var wdapp_path = _toolchain.get_bin_dir().path_join("wdapp.exe")
	_log("Launching: %s" % aumid)
	var result = _toolchain.execute_tool(wdapp_path, PackedStringArray(["launch", aumid]))
	if result["exit_code"] == 0:
		_launch_status_label.text = "✅ Launched"
		_log("App launched successfully")
	else:
		_launch_status_label.text = "❌ Launch failed"
		_log("Launch failed: %s" % result["stdout"])

func _on_app_terminate() -> void:
	var aumid = _get_selected_aumid()
	var pfn = _get_selected_pfn()
	if aumid == "":
		_launch_status_label.text = "❌ No app selected"
		return
	var wdapp_path = _toolchain.get_bin_dir().path_join("wdapp.exe")
	# Try wdapp terminate first (works for MSIXVC packages)
	_log("Terminating: %s" % pfn)
	var result = _toolchain.execute_tool(wdapp_path, PackedStringArray(["terminate", pfn]))
	if result["exit_code"] == 0:
		_launch_status_label.text = "✅ Terminated"
		_log("App terminated")
		return
	# Fallback: find exe in Build/ and use taskkill
	var build_dir = _get_build_dir()
	if DirAccess.dir_exists_absolute(build_dir):
		var dir = DirAccess.open(build_dir)
		if dir:
			dir.list_dir_begin()
			var fname = dir.get_next()
			while fname != "":
				if fname.ends_with(".exe") and not fname.ends_with(".console.exe"):
					_log("Falling back to taskkill: %s" % fname)
					var output: Array = []
					OS.execute("taskkill", PackedStringArray(["/IM", fname, "/F"]), output, true, false)
					_launch_status_label.text = "✅ Terminated (taskkill)"
					_log("Process terminated via taskkill")
					dir.list_dir_end()
					return
				fname = dir.get_next()
			dir.list_dir_end()
	_launch_status_label.text = "❌ Terminate failed"
	_log("Terminate failed: %s" % result["stdout"])

## Finds the .msixvc file in the Package/ folder.
func _find_msixvc_package() -> String:
	var pkg_dir = _get_package_dir()
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
func _get_app_aumid() -> String:
	var info = _config_mgr.parse_config()
	var name = info.get("name", "")
	if name == "":
		return ""
	# AUMID format: <Identity.Name>_<publisher_hash>!Game
	# We can get it from wdapp list output instead
	var wdapp_path = _toolchain.get_bin_dir().path_join("wdapp.exe")
	var result = _toolchain.execute_tool(wdapp_path, PackedStringArray(["list"]))
	if result["exit_code"] != 0:
		return ""
	# Find an AUMID line containing our app name
	for line in result["stdout"].split("\n"):
		var trimmed = line.strip_edges()
		if trimmed.contains("!") and trimmed.contains(name):
			return trimmed
	return ""


## Gets the PackageFullName for the registered app.
func _get_package_full_name() -> String:
	var info = _config_mgr.parse_config()
	var name = info.get("name", "")
	if name == "":
		return ""
	var wdapp_path = _toolchain.get_bin_dir().path_join("wdapp.exe")
	var result = _toolchain.execute_tool(wdapp_path, PackedStringArray(["list"]))
	if result["exit_code"] != 0:
		return ""
	for line in result["stdout"].split("\n"):
		var trimmed = line.strip_edges()
		if not trimmed.contains("!") and trimmed.contains(name) and trimmed.contains("_"):
			return trimmed
	return ""


func _on_pkg_install() -> void:
	var msixvc = _find_msixvc_package()
	if msixvc == "":
		_install_status_label.text = "❌ No .msixvc package found in Package/"
		return
	var wdapp_path = _toolchain.get_bin_dir().path_join("wdapp.exe")
	_log("Installing: %s" % msixvc)
	_install_status_label.text = "Installing package..."
	var result = _toolchain.execute_tool(wdapp_path, PackedStringArray(["install", msixvc]))
	if result["exit_code"] == 0:
		_install_status_label.text = "✅ Package installed"
		_log("Package installed successfully")
		_refresh_registered_apps()
	else:
		_install_status_label.text = "❌ Install failed"
		_log("Install failed: %s" % result["stdout"])


func _on_pkg_uninstall() -> void:
	var pfn = _get_selected_pfn()
	if pfn == "":
		_install_status_label.text = "❌ No app selected — select one from the Launch section"
		return
	var wdapp_path = _toolchain.get_bin_dir().path_join("wdapp.exe")
	_log("Uninstalling: %s" % pfn)
	var result = _toolchain.execute_tool(wdapp_path, PackedStringArray(["uninstall", pfn]))
	if result["exit_code"] == 0:
		_install_status_label.text = "✅ Package uninstalled"
		_log("Package uninstalled")
		_refresh_registered_apps()
	else:
		_install_status_label.text = "❌ Uninstall failed"
		_log("Uninstall failed: %s" % result["stdout"])


## Removes all files and subdirectories from a directory.
func _clean_directory(dir_path: String) -> void:
	var dir = DirAccess.open(dir_path)
	if dir == null:
		return
	dir.list_dir_begin()
	var fname = dir.get_next()
	while fname != "":
		if dir.current_is_dir():
			_clean_directory(dir_path.path_join(fname))
			dir.remove(fname)
		else:
			dir.remove(fname)
		fname = dir.get_next()
	dir.list_dir_end()


# ── Achievements ────────────────────────────────────────────────────────────

func _load_achievement_config() -> void:
	var cfg := ConfigFile.new()
	if cfg.load(SAMPLE_CONFIG_PATH) == OK:
		var val = cfg.get_value("achievements", "demo_achievement_id", "")
		_achievement_id_edit.text = str(val)
		_achievement_status_label.text = "Loaded from sample_config.cfg"
	else:
		_achievement_status_label.text = "No sample_config.cfg — enter a value and save."

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

	# GDScript can't read DLL metadata directly; use PowerShell to extract ProductVersion.
	var output: Array = []
	var ps_cmd = "(Get-Item '%s').VersionInfo.ProductVersion" % dll_path.replace("'", "''")
	var exit_code = OS.execute("powershell", PackedStringArray(["-NoProfile", "-Command", ps_cmd]), output, true, false)
	if exit_code == 0 and output.size() > 0:
		var version = str(output[0]).strip_edges()
		if version != "":
			_playfab_version_label.text = "PlayFab SDK: %s" % version
			return

	_playfab_version_label.text = "PlayFab SDK: installed (version unknown)"
