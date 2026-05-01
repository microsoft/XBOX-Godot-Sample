extends Node
## Autoload that keeps the GDK extension loaded.
## Samples require native auto-dispatch and do not provide a manual pump path.

const GDK_EXTENSION_PATH = "res://addons/godot_gdk/godot_gdk.gdextension"
const GD_SCRIPT_CHECK_FLAG = "--gd-script-check"

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
	var user_args := OS.get_cmdline_user_args()
	if user_args.has(GD_SCRIPT_CHECK_FLAG):
		return
	if _gdk() == null:
		push_warning("[GDK] Extension singleton is not available yet. Build the addon before opening this sample.")

func _exit_tree() -> void:
	var gdk = _gdk()
	if gdk != null and gdk.is_initialized():
		gdk.shutdown()
