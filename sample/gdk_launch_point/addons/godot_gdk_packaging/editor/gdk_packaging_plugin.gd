@tool
extends EditorPlugin
## GDK Packaging Editor Plugin — adds a "GDK" top-level menu in the editor
## menu bar for Microsoft GDK PC packaging tools.

const GDKToolchainScript = preload("res://addons/godot_gdk_packaging/core/gdk_toolchain.gd")
const GameConfigManagerScript = preload("res://addons/godot_gdk_packaging/core/game_config_manager.gd")
const ConfigImportPlugin = preload("res://addons/godot_gdk_packaging/editor/config_import_plugin.gd")
const GDKTutorialWizard = preload("res://addons/godot_gdk_packaging/editor/gdk_tutorial_wizard.gd")
const GDKSandboxDialog = preload("res://addons/godot_gdk_packaging/editor/gdk_sandbox_dialog.gd")

# Documentation URLs
const DOC_PC_PACKAGING := "https://learn.microsoft.com/en-us/gaming/gdk/_content/gc/packaging/pc/pc-packaging-getting-started"
const DOC_MAKEPKG := "https://learn.microsoft.com/en-us/gaming/gdk/_content/gc/packaging/deployment/makepkg-package-creation"
const DOC_GAME_CONFIG_EDITOR := "https://learn.microsoft.com/en-us/gaming/gdk/_content/gc/system/overviews/game-config-editor"
const DOC_ACHIEVEMENTS := "https://learn.microsoft.com/en-us/gaming/gdk/docs/gdk-dev/pc-dev/tutorials/pc-e2e-guide/e2e-services/e2e-achievements"
const DOC_PLAYFAB_GAME_MANAGER := "https://developer.playfab.com/en-us/r/sign-in"
const DOC_PLAYFAB_IDS := "https://learn.microsoft.com/en-us/rest/api/playfab/client/account-management/get-playfab-ids-from-xbox-live-ids"
const DOC_PLAYFAB_GDK := "https://learn.microsoft.com/en-us/gaming/playfab/sdks/playfab-sdk-for-gdk/quickstart-gdk"

var _menu_bar: MenuBar
var _gdk_popup: PopupMenu
var _gdk_menu_index: int = -1
var _toolchain: RefCounted
var _config_mgr: RefCounted
var _config_import_plugin: EditorImportPlugin
var _sandbox_dialog: AcceptDialog

# Menu item IDs
enum MenuID {
	GETTING_STARTED,
	SEP_0,
	GAME_CONFIG,
	OPEN_SANDBOX,
	SEP_1,
	DOC_PACKAGING,
	DOC_MAKEPKG,
	DOC_CONFIG_EDITOR,
	DOC_ACHIEVEMENTS,
	DOC_PLAYFAB,
	DOC_PLAYFAB_IDS_LINK,
	DOC_PLAYFAB_GDK_LINK,
}


func _enter_tree() -> void:
	_toolchain = GDKToolchainScript.new()
	_config_mgr = GameConfigManagerScript.new(_toolchain)

	# Register .config file import so they appear in the FileSystem dock
	_config_import_plugin = ConfigImportPlugin.new()
	add_import_plugin(_config_import_plugin)

	# ── GDK Sandbox Switcher dialog (persistent, owned by the plugin) ──
	# Parented under the editor base control so it inherits theme and input
	# gating like any other editor dialog.
	_sandbox_dialog = GDKSandboxDialog.new()
	var base: Control = EditorInterface.get_base_control()
	if base != null:
		base.add_child(_sandbox_dialog)
		_sandbox_dialog.setup(_toolchain)
	else:
		push_warning("[GDK Packaging] No editor base control — Sandbox dialog will be unparented.")

	# ── Find the editor MenuBar and add a "GDK" top-level menu ──
	_menu_bar = _find_menu_bar(EditorInterface.get_base_control())
	if _menu_bar:
		_gdk_popup = PopupMenu.new()
		_gdk_popup.name = "GDKMenu"
		_gdk_popup.add_item("Getting Started", MenuID.GETTING_STARTED)
		_gdk_popup.add_separator("", MenuID.SEP_0)
		_gdk_popup.add_item(_get_game_config_label(), MenuID.GAME_CONFIG)
		_gdk_popup.add_item("Change Sandbox…", MenuID.OPEN_SANDBOX)
		_gdk_popup.add_separator("", MenuID.SEP_1)
		_gdk_popup.add_item("PC Packaging Overview", MenuID.DOC_PACKAGING)
		_gdk_popup.add_item("makepkg Reference", MenuID.DOC_MAKEPKG)
		_gdk_popup.add_item("GameConfigEditor Reference", MenuID.DOC_CONFIG_EDITOR)
		_gdk_popup.add_item("Achievements Guide", MenuID.DOC_ACHIEVEMENTS)
		_gdk_popup.add_item("PlayFab Game Manager", MenuID.DOC_PLAYFAB)
		_gdk_popup.add_item("PlayFab IDs from Xbox Live", MenuID.DOC_PLAYFAB_IDS_LINK)
		_gdk_popup.add_item("PlayFab + GDK Quickstart", MenuID.DOC_PLAYFAB_GDK_LINK)
		_gdk_popup.id_pressed.connect(_on_menu_item_pressed)
		_gdk_popup.about_to_popup.connect(_update_game_config_label)

		_menu_bar.add_child(_gdk_popup)
		_gdk_menu_index = _menu_bar.get_menu_count() - 1
		_menu_bar.set_menu_title(_gdk_menu_index, "GDK")
		_menu_bar.set_menu_tooltip(_gdk_menu_index, "Microsoft GDK PC Packaging Tools")
	else:
		push_warning("[GDK Packaging] Could not find editor MenuBar — falling back to toolbar button.")

	if not _toolchain.is_gdk_available():
		push_warning("[GDK Packaging] Microsoft GDK not detected — packaging tools unavailable.")

	print("[GDK Packaging] Editor plugin loaded")


func _exit_tree() -> void:
	if _sandbox_dialog and is_instance_valid(_sandbox_dialog):
		if _sandbox_dialog.get_parent() != null:
			_sandbox_dialog.get_parent().remove_child(_sandbox_dialog)
		_sandbox_dialog.queue_free()
		_sandbox_dialog = null

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
	for child: Node in node.get_children():
		var result: MenuBar = _find_menu_bar(child)
		if result:
			return result
	return null


func _on_menu_item_pressed(id: int) -> void:
	match id:
		MenuID.GETTING_STARTED:
			var wizard: Window = GDKTutorialWizard.new()
			EditorInterface.get_base_control().add_child(wizard)
			wizard.popup_centered()

		MenuID.GAME_CONFIG:
			_on_game_config_action()

		MenuID.OPEN_SANDBOX:
			_open_sandbox_dialog()

		MenuID.DOC_PACKAGING:
			OS.shell_open(DOC_PC_PACKAGING)

		MenuID.DOC_MAKEPKG:
			OS.shell_open(DOC_MAKEPKG)

		MenuID.DOC_CONFIG_EDITOR:
			OS.shell_open(DOC_GAME_CONFIG_EDITOR)

		MenuID.DOC_ACHIEVEMENTS:
			OS.shell_open(DOC_ACHIEVEMENTS)

		MenuID.DOC_PLAYFAB:
			OS.shell_open(DOC_PLAYFAB_GAME_MANAGER)

		MenuID.DOC_PLAYFAB_IDS_LINK:
			OS.shell_open(DOC_PLAYFAB_IDS)

		MenuID.DOC_PLAYFAB_GDK_LINK:
			OS.shell_open(DOC_PLAYFAB_GDK)


func _get_game_config_label() -> String:
	if _config_mgr != null and _config_mgr.config_exists():
		return "Edit MicrosoftGame.config"
	return "Create MicrosoftGame.config"


func _update_game_config_label() -> void:
	if _gdk_popup == null:
		return
	var index: int = _gdk_popup.get_item_index(MenuID.GAME_CONFIG)
	if index >= 0:
		_gdk_popup.set_item_text(index, _get_game_config_label())


func _open_sandbox_dialog() -> void:
	if _sandbox_dialog == null or not is_instance_valid(_sandbox_dialog):
		push_error("[GDK Packaging] Sandbox dialog not initialized")
		return
	_sandbox_dialog.show_centered_clamped()


func _on_game_config_action() -> void:
	if _config_mgr.config_exists():
		var pid: int = _config_mgr.launch_editor()
		if pid < 0:
			push_error("[GDK Packaging] Failed to launch GameConfigEditor")
		return

	var err: Error = _config_mgr.create_template()
	if err != OK:
		push_error("[GDK Packaging] Failed to create MicrosoftGame.config: " + error_string(err))
		return

	print("[GDK Packaging] Created template MicrosoftGame.config")
	_update_game_config_label()
	var fs: EditorFileSystem = EditorInterface.get_resource_filesystem()
	if not fs.is_scanning():
		fs.scan()

	var pid: int = _config_mgr.launch_editor()
	if pid < 0:
		push_error("[GDK Packaging] Failed to launch GameConfigEditor")
