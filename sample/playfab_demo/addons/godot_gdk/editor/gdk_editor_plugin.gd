@tool
extends EditorPlugin
## GodotGDK Editor Plugin — registers the Xbox/GDK export platform.

const GDKExportPlatform = preload("res://addons/godot_gdk/editor/gdk_export_platform.gd")

var _export_platform: EditorExportPlatformExtension = null

func _enter_tree() -> void:
	_export_platform = GDKExportPlatform.new()
	add_export_platform(_export_platform)

	print("[GodotGDK] Editor plugin loaded — Xbox GDK (PC) export platform registered")

func _exit_tree() -> void:
	if _export_platform:
		remove_export_platform(_export_platform)
		_export_platform = null
	print("[GodotGDK] Editor plugin unloaded")
