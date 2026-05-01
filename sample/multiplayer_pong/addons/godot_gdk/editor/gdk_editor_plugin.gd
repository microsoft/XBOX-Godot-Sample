@tool
extends EditorPlugin
## GodotGDK Editor Plugin — registers the Xbox/GDK export platform.

func _enter_tree() -> void:
	# Export platform deprecated — use the GDK dock Export & Package tab instead
	print("[GodotGDK] Editor plugin loaded")

func _exit_tree() -> void:
	print("[GodotGDK] Editor plugin unloaded")
