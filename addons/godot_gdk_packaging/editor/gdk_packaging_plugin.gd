@tool
extends EditorPlugin
## GDK Packaging Editor Plugin — adds a "GDK" top-level menu in the editor
## menu bar and a dock panel for Microsoft GDK PC packaging tools.

const PackagingPanel = preload("res://addons/godot_gdk_packaging/editor/packaging_panel.gd")
const GDKToolchainScript = preload("res://addons/godot_gdk_packaging/editor/gdk_toolchain.gd")
const GameConfigManagerScript = preload("res://addons/godot_gdk_packaging/editor/game_config_manager.gd")
const ConfigImportPlugin = preload("res://addons/godot_gdk_packaging/editor/config_import_plugin.gd")

# Documentation URLs
const DOC_PC_PACKAGING := "https://learn.microsoft.com/en-us/gaming/gdk/_content/gc/packaging/pc/pc-packaging-getting-started"
const DOC_MAKEPKG := "https://learn.microsoft.com/en-us/gaming/gdk/_content/gc/packaging/deployment/makepkg-package-creation"
const DOC_GAME_CONFIG_EDITOR := "https://learn.microsoft.com/en-us/gaming/gdk/_content/gc/system/overviews/game-config-editor"

var _menu_bar: MenuBar
var _gdk_popup: PopupMenu
var _gdk_menu_index: int = -1
var _packaging_panel: Control
var _toolchain: RefCounted
var _config_mgr: RefCounted
var _config_import_plugin: EditorImportPlugin

# Menu item IDs
enum MenuID {
	CREATE_PACKAGE,
	GENERATE_MAP,
	VALIDATE,
	SEP_1,
	EDIT_CONFIG,
	CREATE_CONFIG,
	SEP_2,
	DOC_PACKAGING,
	DOC_MAKEPKG,
	DOC_CONFIG_EDITOR,
}


func _enter_tree() -> void:
	_toolchain = GDKToolchainScript.new()
	_config_mgr = GameConfigManagerScript.new(_toolchain)

	# Register .config file import so they appear in the FileSystem dock
	_config_import_plugin = ConfigImportPlugin.new()
	add_import_plugin(_config_import_plugin)

	# ── Find the editor MenuBar and add a "GDK" top-level menu ──
	_menu_bar = _find_menu_bar(EditorInterface.get_base_control())
	if _menu_bar:
		_gdk_popup = PopupMenu.new()
		_gdk_popup.name = "GDKMenu"
		_gdk_popup.add_item("Create MSIXVC Package...", MenuID.CREATE_PACKAGE)
		_gdk_popup.add_item("Generate Mapping File...", MenuID.GENERATE_MAP)
		_gdk_popup.add_item("Validate Package...", MenuID.VALIDATE)
		_gdk_popup.add_separator("", MenuID.SEP_1)
		_gdk_popup.add_item("Edit MicrosoftGame.config", MenuID.EDIT_CONFIG)
		_gdk_popup.add_item("Create MicrosoftGame.config", MenuID.CREATE_CONFIG)
		_gdk_popup.add_separator("", MenuID.SEP_2)
		_gdk_popup.add_item("📖 PC Packaging Overview", MenuID.DOC_PACKAGING)
		_gdk_popup.add_item("📖 makepkg Reference", MenuID.DOC_MAKEPKG)
		_gdk_popup.add_item("📖 GameConfigEditor Reference", MenuID.DOC_CONFIG_EDITOR)
		_gdk_popup.id_pressed.connect(_on_menu_item_pressed)

		_menu_bar.add_child(_gdk_popup)
		_gdk_menu_index = _menu_bar.get_menu_count() - 1
		_menu_bar.set_menu_title(_gdk_menu_index, "GDK")
		_menu_bar.set_menu_tooltip(_gdk_menu_index, "Microsoft GDK PC Packaging Tools")
	else:
		push_warning("[GDK Packaging] Could not find editor MenuBar — falling back to toolbar button.")

	# ── Packaging dock panel ──
	_packaging_panel = PackagingPanel.new()
	_packaging_panel.name = "GDK Packaging"
	add_control_to_dock(DOCK_SLOT_RIGHT_BL, _packaging_panel)

	if not _toolchain.is_gdk_available():
		push_warning("[GDK Packaging] Microsoft GDK not detected — packaging tools unavailable.")

	print("[GDK Packaging] Editor plugin loaded")


func _exit_tree() -> void:
	if _packaging_panel:
		remove_control_from_docks(_packaging_panel)
		_packaging_panel.queue_free()
		_packaging_panel = null

	if _gdk_popup and is_instance_valid(_gdk_popup):
		_gdk_popup.queue_free()
		_gdk_popup = null
	_menu_bar = null
	_gdk_menu_index = -1

	_toolchain = null
	_config_mgr = null

	if _config_import_plugin:
		remove_import_plugin(_config_import_plugin)
		_config_import_plugin = null

	print("[GDK Packaging] Editor plugin unloaded")


func _find_menu_bar(node: Node) -> MenuBar:
	if node is MenuBar:
		return node
	for child in node.get_children():
		var result := _find_menu_bar(child)
		if result:
			return result
	return null


func _on_menu_item_pressed(id: int) -> void:
	match id:
		MenuID.CREATE_PACKAGE:
			# Focus the packaging panel — user clicks "Create Package" there
			_focus_packaging_panel()

		MenuID.GENERATE_MAP:
			_focus_packaging_panel()

		MenuID.VALIDATE:
			_focus_packaging_panel()

		MenuID.EDIT_CONFIG:
			if _config_mgr.config_exists():
				_config_mgr.launch_editor()
			else:
				push_warning("[GDK Packaging] MicrosoftGame.config not found — create one first.")

		MenuID.CREATE_CONFIG:
			var err = _config_mgr.create_template()
			if err == OK:
				print("[GDK Packaging] Created template MicrosoftGame.config")
			elif err == ERR_ALREADY_EXISTS:
				push_warning("[GDK Packaging] MicrosoftGame.config already exists.")

		MenuID.DOC_PACKAGING:
			OS.shell_open(DOC_PC_PACKAGING)

		MenuID.DOC_MAKEPKG:
			OS.shell_open(DOC_MAKEPKG)

		MenuID.DOC_CONFIG_EDITOR:
			OS.shell_open(DOC_GAME_CONFIG_EDITOR)


func _focus_packaging_panel() -> void:
	if _packaging_panel:
		# Make the dock visible and bring it to focus
		_packaging_panel.visible = true
		_packaging_panel.grab_focus()
