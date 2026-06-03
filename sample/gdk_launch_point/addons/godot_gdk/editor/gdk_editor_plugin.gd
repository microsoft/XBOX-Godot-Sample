@tool
extends EditorPlugin
## GodotGDK Editor Plugin — keeps the runtime bootstrap wired into projects.

const AUTOLOAD_NAME := "GDKBootstrap"
const AUTOLOAD_PATH := "res://addons/godot_gdk/runtime/gdk_bootstrap.gd"


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
