extends Node
## Autoload script that initializes the shared GDK runtime and pumps async dispatch.

var _startup_user_op = null
var _bootstrap_active := false

func _ready() -> void:
	var args := OS.get_cmdline_args()
	if args.has("--script") and args.has("res://tests/run_tests.gd"):
		print("[GDK] Bootstrap skipped for headless tests")
		return

	_bootstrap_active = true
	print("=== GodotGDK Bootstrap ===")

	GDK.initialized.connect(_on_gdk_initialized)
	GDK.runtime_error.connect(_on_gdk_runtime_error)
	GDK.users.user_added.connect(_on_user_added)
	GDK.users.user_removed.connect(_on_user_removed)
	GDK.users.primary_user_changed.connect(_on_primary_user_changed)

	var init_result = GDK.initialize()
	if not init_result.ok:
		push_warning("[GDK] %s" % init_result.message)

func _on_gdk_initialized() -> void:
	print("[GDK] Runtime initialized")

	_startup_user_op = GDK.users.add_default_user_async()
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
	if _bootstrap_active and GDK.is_initialized():
		GDK.dispatch()

func _exit_tree() -> void:
	if _bootstrap_active:
		GDK.shutdown()
