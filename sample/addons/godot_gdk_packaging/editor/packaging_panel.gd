@tool
extends Control
## Dock panel UI for GDK PC packaging — configures makepkg flags, runs
## genmap / validate / pack, and manages MicrosoftGame.config.

const GDKToolchainScript = preload("res://addons/godot_gdk_packaging/editor/gdk_toolchain.gd")
const MakePkgExecutorScript = preload("res://addons/godot_gdk_packaging/editor/makepkg_executor.gd")
const GameConfigManagerScript = preload("res://addons/godot_gdk_packaging/editor/game_config_manager.gd")

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

# Actions
var _genmap_btn: Button
var _validate_btn: Button
var _pack_btn: Button

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
	var scroll := ScrollContainer.new()
	scroll.set_anchors_and_offsets_preset(PRESET_FULL_RECT)
	scroll.horizontal_scroll_mode = ScrollContainer.SCROLL_MODE_DISABLED
	add_child(scroll)

	var root := VBoxContainer.new()
	root.size_flags_horizontal = SIZE_EXPAND_FILL
	scroll.add_child(root)

	# ── Header ──
	var title := Label.new()
	title.text = "GDK PC Packaging"
	title.add_theme_font_size_override("font_size", 18)
	root.add_child(title)

	_status_label = Label.new()
	if _toolchain.is_gdk_available():
		_status_label.text = "✅ GDK tools found"
	else:
		_status_label.text = "❌ GDK not found — install Microsoft GDK"
	root.add_child(_status_label)

	root.add_child(HSeparator.new())

	# ── MicrosoftGame.config Status ──
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

	# ── Source Configuration ──
	_add_section_header(root, "Source Configuration")

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

	# Content ID
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

	# Product ID
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

	# Encryption
	var enc_row := HBoxContainer.new()
	root.add_child(enc_row)
	var enc_label := Label.new()
	enc_label.text = "Encryption"
	enc_label.custom_minimum_size.x = 130
	enc_row.add_child(enc_label)
	_encrypt_option = OptionButton.new()
	_encrypt_option.add_item("None (dev default)")       # 0
	_encrypt_option.add_item("License encrypt (/l)")     # 1
	_encrypt_option.add_item("Custom key (/lk)")         # 2
	_encrypt_option.item_selected.connect(_on_encrypt_changed)
	enc_row.add_child(_encrypt_option)

	_encrypt_key_edit = _add_path_field(root, "EKB Key File",
		"Path to encryption key bundle file", false)
	_encrypt_key_edit.get_parent().visible = false  # hidden until /lk selected

	# Update compatibility
	var compat_row := HBoxContainer.new()
	root.add_child(compat_row)
	var compat_label := Label.new()
	compat_label.text = "Update Compat"
	compat_label.custom_minimum_size.x = 130
	compat_row.add_child(compat_label)
	_updcompat_option = OptionButton.new()
	_updcompat_option.add_item("3 — Sub-file granularity (default)")  # 0 → value 3
	_updcompat_option.add_item("2 — File-level granularity")           # 1 → value 2
	_updcompat_option.add_item("1 — Legacy")                           # 2 → value 1
	compat_row.add_child(_updcompat_option)

	root.add_child(HSeparator.new())

	# ── Action Buttons ──
	_add_section_header(root, "Actions")

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

	root.add_child(HSeparator.new())

	_set_actions_enabled(_toolchain.is_gdk_available())


# ── UI Helpers ──────────────────────────────────────────────────────────────

func _add_section_header(parent: VBoxContainer, text: String) -> void:
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
				dialog.queue_free())
		else:
			dialog.file_selected.connect(func(path: String):
				edit.text = path
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
			# Auto-populate IDs if panel fields are empty
			if _product_id_edit.text == "" and info.get("product_id", "") != "":
				_product_id_edit.text = info["product_id"]
		else:
			_config_identity_label.text = "(could not parse identity)"

		# Relocate any logos GameConfigEditor wrote to project root into storelogos/
		var relocated = _config_mgr.relocate_logos_to_storelogos()
		if relocated > 0:
			_log("Relocated %d logo(s) to storelogos/" % relocated)

		# Sync remaining logos — regenerate other sizes from the 480x480 source
		var synced = _config_mgr.sync_store_logos()
		if synced > 0:
			_log("Synced %d store logo(s) from 480x480 source" % synced)
	else:
		_config_status_label.text = "⚠️ MicrosoftGame.config not found"
		_config_identity_label.text = "Create a template or use GameConfigEditor to get started."
		_create_config_btn.visible = true


# ── Signal Handlers ─────────────────────────────────────────────────────────

func _on_encrypt_changed(index: int) -> void:
	# Show EKB key file field only when "Custom key" is selected
	_encrypt_key_edit.get_parent().visible = (index == 2)

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
	_log("Generating mapping file...")
	var result = _makepkg.genmap(source, map_path)
	_log_result(result)
	if result["exit_code"] == 0:
		_map_file_edit.text = map_path

func _on_validate() -> void:
	var source := _source_dir_edit.text.strip_edges()
	var map_file := _map_file_edit.text.strip_edges()
	if source == "" or map_file == "":
		_log("❌ Content directory and mapping file are required for validation.")
		return
	_log("Validating package layout...")
	var result = _makepkg.validate(map_file, source)
	_log_result(result)

func _on_pack() -> void:
	var source := _source_dir_edit.text.strip_edges()
	var output := _output_dir_edit.text.strip_edges()
	if source == "":
		_log("❌ Content directory is required.")
		return
	if output == "":
		_log("❌ Output directory is required.")
		return

	# Auto-generate mapping file if checkbox is on
	var map_file := _map_file_edit.text.strip_edges()
	if _auto_genmap_check.button_pressed or map_file == "":
		_log("Auto-generating mapping file...")
		var map_path := output.path_join("layout.xml")
		var genmap_result = _makepkg.genmap(source, map_path)
		_log_result(genmap_result)
		if genmap_result["exit_code"] != 0:
			_log("❌ Mapping file generation failed — aborting package.")
			return
		map_file = map_path
		_map_file_edit.text = map_file

	# Build options
	var options := {}
	if _content_id_edit.text.strip_edges() != "":
		options["content_id"] = _content_id_edit.text.strip_edges()
	if _product_id_edit.text.strip_edges() != "":
		options["product_id"] = _product_id_edit.text.strip_edges()

	match _encrypt_option.selected:
		1:
			options["encrypt"] = true
		2:
			options["encrypt_key"] = _encrypt_key_edit.text.strip_edges()

	var updcompat_map := [3, 2, 1]
	options["updcompat"] = updcompat_map[_updcompat_option.selected]

	_log("Creating MSIXVC package...")
	var result = _makepkg.pack(source, map_file, output, options)
	_log_result(result)
