@tool
extends EditorPlugin
## GodotPlayFab Editor Plugin — placeholder stub.
##
## PlayFab is a runtime-only addon today: there is no auto-installable
## bootstrap autoload, and the editor plugin currently does no work.
## This file exists so `plugin.cfg` is a valid Godot 4 EditorPlugin
## manifest. When PlayFab gains editor tooling (e.g., a title-config
## helper), wire it up here.


func _enable_plugin() -> void:
	print("[GodotPlayFab] Editor plugin enabled — no editor tooling registered yet.")


func _disable_plugin() -> void:
	print("[GodotPlayFab] Editor plugin disabled.")
