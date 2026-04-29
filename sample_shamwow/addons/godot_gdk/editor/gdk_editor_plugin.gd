@tool
extends EditorPlugin
## GodotGDK Editor Plugin — registers the Xbox/GDK export platform and setup panel.

const GDKExportPlatform = preload("res://addons/godot_gdk/editor/gdk_export_platform.gd")
const GDKSetupPanel = preload("res://addons/godot_gdk/editor/gdk_setup_panel.gd")

var _export_platform: EditorExportPlatformExtension = null
var _setup_panel: Control = null

func _enter_tree() -> void:
	_export_platform = GDKExportPlatform.new()
	add_export_platform(_export_platform)

	_setup_panel = GDKSetupPanel.new()
	_setup_panel.name = "GDK Setup"
	add_control_to_dock(DOCK_SLOT_RIGHT_BL, _setup_panel)

	print("[GodotGDK] Editor plugin loaded — Xbox GDK (PC) export platform registered")

func _exit_tree() -> void:
	if _setup_panel:
		remove_control_from_docks(_setup_panel)
		_setup_panel.queue_free()
		_setup_panel = null
	if _export_platform:
		remove_export_platform(_export_platform)
		_export_platform = null
	print("[GodotGDK] Editor plugin unloaded")
