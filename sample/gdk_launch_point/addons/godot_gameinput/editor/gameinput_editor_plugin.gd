@tool
extends EditorPlugin
## Godot GameInput — Editor Plugin
##
## Installs the bootstrap autoload (`GameInputBootstrap`) on enable, removes it
## on disable. The autoload is what consumes Project Settings and drives
## `GameInput.initialize()` / `GameInput.poll()` for game projects, so projects
## just need to enable the plugin and configure settings — no manual wiring.

const AUTOLOAD_NAME := "GameInputBootstrap"
const AUTOLOAD_PATH := "res://addons/godot_gameinput/runtime/gameinput_bootstrap.gd"


func _enable_plugin() -> void:
	add_autoload_singleton(AUTOLOAD_NAME, AUTOLOAD_PATH)
	print("[GameInput] Editor plugin enabled — '%s' autoload installed." % AUTOLOAD_NAME)


func _disable_plugin() -> void:
	remove_autoload_singleton(AUTOLOAD_NAME)
	print("[GameInput] Editor plugin disabled — '%s' autoload removed." % AUTOLOAD_NAME)
