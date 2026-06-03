@tool
extends Control
## GDK dock panel — coordinator for the tab-local packaging surfaces.

const GDKToolchainScript = preload("res://addons/godot_gdk_packaging/editor/gdk_toolchain.gd")
const MakePkgExecutorScript = preload("res://addons/godot_gdk_packaging/editor/makepkg_executor.gd")
const GameConfigManagerScript = preload("res://addons/godot_gdk_packaging/editor/game_config_manager.gd")
const PackagingSettingsStoreScript = preload("res://addons/godot_gdk_packaging/editor/packaging_settings_store.gd")
const ExportPresetCatalogScript = preload("res://addons/godot_gdk_packaging/editor/export_preset_catalog.gd")
const PackagingContentPreparerScript = preload("res://addons/godot_gdk_packaging/editor/packaging_content_preparer.gd")
const WdappManagerScript = preload("res://addons/godot_gdk_packaging/editor/wdapp_manager.gd")

const ConfigTabScript = preload("res://addons/godot_gdk_packaging/editor/packaging_tabs/config_tab.gd")
const SandboxTabScript = preload("res://addons/godot_gdk_packaging/editor/packaging_tabs/sandbox_tab.gd")
const ExportPackageTabScript = preload("res://addons/godot_gdk_packaging/editor/packaging_tabs/export_package_tab.gd")
const InstallLaunchTabScript = preload("res://addons/godot_gdk_packaging/editor/packaging_tabs/install_launch_tab.gd")
const AchievementsTabScript = preload("res://addons/godot_gdk_packaging/editor/packaging_tabs/achievements_tab.gd")
const PlayFabTabScript = preload("res://addons/godot_gdk_packaging/editor/packaging_tabs/playfab_tab.gd")

const PACKAGING_SETTINGS_PATH := "res://.gdk_packaging.cfg"

var _toolchain: RefCounted
var _makepkg: RefCounted
var _config_mgr: RefCounted
var _settings_store: RefCounted
var _preset_catalog: RefCounted
var _content_preparer: RefCounted
var _wdapp_manager: RefCounted

var _status_label: Label
var _tab_pages: Array[Control] = []

var _config_tab
var _sandbox_tab
var _packaging_tab
var _install_launch_tab
var _achievements_tab
var _playfab_tab

var _watch_timer := 0.0
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
	_settings_store = PackagingSettingsStoreScript.new()
	_preset_catalog = ExportPresetCatalogScript.new()
	_content_preparer = PackagingContentPreparerScript.new(_config_mgr)
	_wdapp_manager = WdappManagerScript.new(_toolchain)

	_build_ui()
	_load_packaging_settings()
	_refresh_sandbox_status()
	_refresh_config_status()
	_load_achievement_config()
	_load_playfab_config()
	_connect_autosave()
	set_process(true)


func _process(delta: float) -> void:
	_watch_timer += delta
	if _watch_timer < WATCH_INTERVAL:
		return
	_watch_timer = 0.0
	_check_and_relocate_root_logos()


func get_toolchain() -> RefCounted:
	return _toolchain


func get_makepkg() -> RefCounted:
	return _makepkg


func get_config_manager() -> RefCounted:
	return _config_mgr


func get_settings_store() -> RefCounted:
	return _settings_store


func get_export_preset_catalog() -> RefCounted:
	return _preset_catalog


func get_content_preparer() -> RefCounted:
	return _content_preparer


func get_wdapp_manager() -> RefCounted:
	return _wdapp_manager


func get_packaging_tab():
	return _packaging_tab


func get_build_dir() -> String:
	return ProjectSettings.globalize_path("res://Build")


func get_package_dir() -> String:
	return ProjectSettings.globalize_path("res://Package")


func _build_ui() -> void:
	var outer := VBoxContainer.new()
	outer.set_anchors_and_offsets_preset(PRESET_FULL_RECT)
	add_child(outer)

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

	_config_tab = ConfigTabScript.new()
	_add_tab_page(outer, _config_tab, true)

	_sandbox_tab = SandboxTabScript.new()
	_add_tab_page(outer, _sandbox_tab, false)

	_packaging_tab = ExportPackageTabScript.new()
	_add_tab_page(outer, _packaging_tab, false)

	_install_launch_tab = InstallLaunchTabScript.new()
	_add_tab_page(outer, _install_launch_tab, false)

	_achievements_tab = AchievementsTabScript.new()
	_add_tab_page(outer, _achievements_tab, false)

	_playfab_tab = PlayFabTabScript.new()
	_add_tab_page(outer, _playfab_tab, false)

	tab_bar.tab_changed.connect(func(idx: int):
		for i in _tab_pages.size():
			_tab_pages[i].visible = (i == idx)
	)

	_set_actions_enabled(_toolchain.is_gdk_available())


func _add_tab_page(parent: Control, page: Control, visible: bool) -> void:
	if page.has_method("setup"):
		page.setup(self)
	page.visible = visible
	parent.add_child(page)
	_tab_pages.append(page)


func _set_actions_enabled(enabled: bool) -> void:
	if _packaging_tab != null and _packaging_tab.has_method("set_actions_enabled"):
		_packaging_tab.set_actions_enabled(enabled)
	if _config_tab != null and _config_tab.edit_config_btn != null:
		_config_tab.edit_config_btn.disabled = not enabled


func _check_and_relocate_root_logos() -> void:
	var project_dir = ProjectSettings.globalize_path("res://")
	var logos_dir = project_dir.path_join("storelogos")

	var found_any := false
	for filename in ROOT_LOGO_FILES:
		if FileAccess.file_exists(project_dir.path_join(filename)):
			found_any = true
			break

	if not found_any:
		return

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
			var import_src = src + ".import"
			if FileAccess.file_exists(import_src):
				dir.remove(import_src)
		else:
			push_warning("[GDK Packaging] Failed to move " + filename + ": " + error_string(err))

	if moved > 0:
		_log("Auto-relocated %d logo(s) to storelogos/" % moved)
		_config_mgr.relocate_logos_to_storelogos()
		for filename in ROOT_LOGO_FILES:
			var import_file = logos_dir.path_join(filename + ".import")
			if FileAccess.file_exists(import_file):
				dir.remove(import_file)
		var fs = EditorInterface.get_resource_filesystem()
		if not fs.is_scanning():
			fs.scan()
		_refresh_config_status()


func _load_packaging_settings() -> void:
	var state: Dictionary = _settings_store.load_state(PACKAGING_SETTINGS_PATH)

	if _packaging_tab != null and _packaging_tab.has_method("apply_state"):
		_packaging_tab.apply_state(state)
	if _sandbox_tab != null and _sandbox_tab.has_method("apply_state"):
		_sandbox_tab.apply_state(state)


func _save_packaging_settings() -> void:
	var state: Dictionary = _settings_store.get_default_state()
	if _packaging_tab != null and _packaging_tab.has_method("collect_state"):
		_merge_settings_state(state, _packaging_tab.collect_state())
	if _sandbox_tab != null and _sandbox_tab.has_method("collect_state"):
		_merge_settings_state(state, _sandbox_tab.collect_state())
	_settings_store.save_state(PACKAGING_SETTINGS_PATH, state)


func _connect_autosave() -> void:
	var save_callback := Callable(self, "_save_packaging_settings")
	if _packaging_tab != null and _packaging_tab.has_method("connect_autosave"):
		_packaging_tab.connect_autosave(save_callback)
	if _sandbox_tab != null and _sandbox_tab.has_method("connect_autosave"):
		_sandbox_tab.connect_autosave(save_callback)


func _merge_settings_state(target: Dictionary, source: Dictionary) -> void:
	for section_name in source:
		var target_section: Dictionary = target.get(section_name, {})
		var source_section: Dictionary = source[section_name]
		for key in source_section:
			target_section[key] = source_section[key]
		target[section_name] = target_section


func _refresh_sandbox_status() -> void:
	if _sandbox_tab != null and _sandbox_tab.has_method("refresh_status"):
		_sandbox_tab.refresh_status()


func _refresh_config_status() -> void:
	if _config_tab != null and _config_tab.has_method("refresh_status"):
		_config_tab.refresh_status()


func _load_achievement_config() -> void:
	if _achievements_tab != null and _achievements_tab.has_method("load_config"):
		_achievements_tab.load_config()


func _load_playfab_config() -> void:
	if _playfab_tab != null and _playfab_tab.has_method("load_config"):
		_playfab_tab.load_config()


func _on_export_and_package() -> void:
	if _packaging_tab != null and _packaging_tab.has_method("on_export_and_package"):
		await _packaging_tab.on_export_and_package()


func _on_genmap() -> void:
	if _packaging_tab != null and _packaging_tab.has_method("on_genmap"):
		_packaging_tab.on_genmap()


func _on_validate() -> void:
	if _packaging_tab != null and _packaging_tab.has_method("on_validate"):
		await _packaging_tab.on_validate()


func _add_section_header(parent: Control, text: String) -> void:
	var label := Label.new()
	label.text = text
	label.add_theme_font_size_override("font_size", 14)
	parent.add_child(label)


func _add_path_field(parent: VBoxContainer, label_text: String, placeholder: String, is_dir: bool) -> LineEdit:
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
