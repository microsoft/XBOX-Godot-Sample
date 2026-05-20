@tool
extends EditorPlugin
## GodotGDK Editor Plugin — keeps the runtime bootstrap wired into projects
## and registers the [code]Xbox GDK (PC)[/code] export platform.

const AUTOLOAD_NAME := "GDKBootstrap"
const AUTOLOAD_PATH := "res://addons/godot_gdk/runtime/gdk_bootstrap.gd"
const GDKExportPlatform = preload("res://addons/godot_gdk/editor/gdk_export_platform.gd")

var _export_platform: EditorExportPlatformExtension


func _enter_tree() -> void:
	_export_platform = GDKExportPlatform.new()
	add_export_platform(_export_platform)


func _exit_tree() -> void:
	if _export_platform != null:
		remove_export_platform(_export_platform)
		_export_platform = null


func _enable_plugin() -> void:
	var autoload_setting: String = "autoload/%s" % AUTOLOAD_NAME
	var current_path: String = ""
	if ProjectSettings.has_setting(autoload_setting):
		current_path = str(ProjectSettings.get_setting(autoload_setting, ""))
		if current_path.begins_with("*"):
			current_path = current_path.substr(1)

	if current_path == AUTOLOAD_PATH:
		print("[GodotGDK] Editor plugin enabled — '%s' autoload already installed." % AUTOLOAD_NAME)
		return

	if ProjectSettings.has_setting(autoload_setting):
		remove_autoload_singleton(AUTOLOAD_NAME)

	add_autoload_singleton(AUTOLOAD_NAME, AUTOLOAD_PATH)
	print("[GodotGDK] Editor plugin enabled — '%s' autoload installed." % AUTOLOAD_NAME)


func _disable_plugin() -> void:
	var autoload_setting: String = "autoload/%s" % AUTOLOAD_NAME
	if ProjectSettings.has_setting(autoload_setting):
		remove_autoload_singleton(AUTOLOAD_NAME)
	print("[GodotGDK] Editor plugin disabled — '%s' autoload removed." % AUTOLOAD_NAME)
