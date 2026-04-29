extends Node
## Autoload script that initializes the shared GDK runtime and pumps async dispatch.

const GDK_EXTENSION_PATH := "res://addons/godot_gdk/godot_gdk.gdextension"

var _startup_user_op = null
var _bootstrap_active := false
var _gdk_extension = null

func get_gdk():
	if Engine.has_singleton("GDK"):
		return Engine.get_singleton("GDK")

	if _gdk_extension == null and FileAccess.file_exists(GDK_EXTENSION_PATH):
		_gdk_extension = load(GDK_EXTENSION_PATH)

	if Engine.has_singleton("GDK"):
		return Engine.get_singleton("GDK")

	return null

func _ready() -> void:
	var args := OS.get_cmdline_args()
	if args.has("--script") and args.has("res://tests/run_tests.gd"):
		print("[GDK] Bootstrap skipped for headless tests")
		return

	_bootstrap_active = true
	print("=== GodotGDK Bootstrap ===")

	var gdk = get_gdk()
	if gdk == null:
		push_warning("[GDK] Extension not loaded")
		return

	gdk.initialized.connect(_on_gdk_initialized)
	gdk.runtime_error.connect(_on_gdk_runtime_error)
	gdk.users.user_added.connect(_on_user_added)
	gdk.users.user_removed.connect(_on_user_removed)
	gdk.users.primary_user_changed.connect(_on_primary_user_changed)

	var init_result = gdk.initialize()
	if not init_result.ok:
		push_warning("[GDK] %s" % init_result.message)

func _on_gdk_initialized() -> void:
	print("[GDK] Runtime initialized")

	var gdk = get_gdk()
	if gdk == null:
		push_warning("[GDK] Extension not loaded")
		return

	_startup_user_op = gdk.users.add_default_user_async()
	if _startup_user_op == null:
		push_warning("[GDK] Silent sign-in could not start")
		return

	if _startup_user_op.is_done():
		_on_startup_user_completed(_startup_user_op.get_result())
	else:
		_startup_user_op.completed.connect(_on_startup_user_completed)

func _on_gdk_runtime_error(result) -> void:
	push_warning("[GDK] %s" % result.message)

func _on_startup_user_completed(result) -> void:
	if result == null:
		push_warning("[GDK] Silent sign-in could not start")
		return

	if not result.ok:
		push_warning("[GDK] Silent sign-in did not complete successfully: %s" % result.message)

func _on_user_added(user) -> void:
	print("[GDK] User added: %s" % user.gamertag)

func _on_user_removed(local_id: int) -> void:
	print("[GDK] User removed: %d" % local_id)

func _on_primary_user_changed(user) -> void:
	if user:
		print("[GDK] Primary user: %s" % user.gamertag)
	else:
		print("[GDK] No primary user")

func _process(_delta: float) -> void:
	var gdk = get_gdk()
	if _bootstrap_active and gdk != null and gdk.is_initialized():
		gdk.dispatch()

func _exit_tree() -> void:
	var gdk = get_gdk()
	if _bootstrap_active and gdk != null:
		gdk.shutdown()
