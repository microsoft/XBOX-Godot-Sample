extends Node
## Autoload that keeps the GDK extension loaded and pumps dispatch when initialized.

const GDK_EXTENSION_PATH = "res://addons/godot_gdk/godot_gdk.gdextension"

var _gdk_extension = null
var _gdk_load_attempted = false

func _gdk():
	if Engine.has_singleton("GDK"):
		return Engine.get_singleton("GDK")

	if not _gdk_load_attempted and _gdk_extension == null and FileAccess.file_exists(GDK_EXTENSION_PATH):
		_gdk_load_attempted = true
		_gdk_extension = load(GDK_EXTENSION_PATH)

	if Engine.has_singleton("GDK"):
		return Engine.get_singleton("GDK")

	return null

func _ready() -> void:
	if _gdk() == null:
		push_warning("[GDK] Extension singleton is not available yet. Build the addon before opening this sample.")

func _process(_delta: float) -> void:
	var gdk = _gdk()
	if gdk != null and gdk.is_initialized():
		gdk.dispatch()

func _exit_tree() -> void:
	var gdk = _gdk()
	if gdk != null and gdk.is_initialized():
		gdk.shutdown()
