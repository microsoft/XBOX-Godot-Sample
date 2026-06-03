extends Node
## Godot GDK - Bootstrap autoload.
##
## Installed by `GodotGDK` when the editor plugin is enabled. Reads Project
## Settings and:
##  * Calls `GDK.initialize()` when `gdk/runtime/initialize_on_startup` is true.
##  * Starts `GDK.users.add_default_user_async()` when
##    `gdk/runtime/auto_add_primary_user` is true.
##  * Skips headless validation and sample test runs.
##  * Shuts the runtime down when the SceneTree is torn down.

const GDK_EXTENSION_PATH := "res://addons/godot_gdk/godot_gdk.gdextension"
const GD_SCRIPT_CHECK_FLAG := "--gd-script-check"
const TEST_SCRIPT_PATH := "res://tests/run_tests.gd"
const SETTING_INITIALIZE_ON_STARTUP := "gdk/runtime/initialize_on_startup"
const SETTING_AUTO_ADD_PRIMARY_USER := "gdk/runtime/auto_add_primary_user"

var _startup_user_in_progress := false
var _gdk_extension = null
var _gdk_load_attempted := false


func get_gdk():
	if Engine.has_singleton("GDK"):
		return Engine.get_singleton("GDK")

	if not _gdk_load_attempted and _gdk_extension == null and FileAccess.file_exists(GDK_EXTENSION_PATH):
		_gdk_load_attempted = true
		_gdk_extension = load(GDK_EXTENSION_PATH)

	if Engine.has_singleton("GDK"):
		return Engine.get_singleton("GDK")

	return null


func _ready() -> void:
	if _should_skip_bootstrap():
		return

	var gdk = get_gdk()
	if gdk == null:
		push_warning("[GDK] Bootstrap: 'GDK' singleton not registered. Is the godot_gdk GDExtension built and loaded?")
		return

	_bind_gdk_signals(gdk)

	var initialize_on_startup: bool = bool(
			ProjectSettings.get_setting(SETTING_INITIALIZE_ON_STARTUP, false))
	if initialize_on_startup and not gdk.is_initialized():
		var init_result = gdk.initialize()
		if init_result == null:
			push_warning("[GDK] Bootstrap: GDK.initialize() did not return a GDKResult.")
		elif init_result.ok:
			print("[GDK] Bootstrap: GDK.initialize() succeeded.")
			_maybe_start_default_user(gdk)
		else:
			push_warning("[GDK] Bootstrap: %s" % init_result.message)
	elif gdk.is_initialized():
		_maybe_start_default_user(gdk)


func _bind_gdk_signals(gdk) -> void:
	var initialized_handler = Callable(self, "_on_gdk_initialized")
	if not gdk.initialized.is_connected(initialized_handler):
		gdk.initialized.connect(initialized_handler)

	var shutdown_handler = Callable(self, "_on_gdk_shutdown_completed")
	if not gdk.shutdown_completed.is_connected(shutdown_handler):
		gdk.shutdown_completed.connect(shutdown_handler)

	var runtime_error_handler = Callable(self, "_on_gdk_runtime_error")
	if not gdk.runtime_error.is_connected(runtime_error_handler):
		gdk.runtime_error.connect(runtime_error_handler)

	var user_added_handler = Callable(self, "_on_user_added")
	if not gdk.users.user_added.is_connected(user_added_handler):
		gdk.users.user_added.connect(user_added_handler)

	var user_removed_handler = Callable(self, "_on_user_removed")
	if not gdk.users.user_removed.is_connected(user_removed_handler):
		gdk.users.user_removed.connect(user_removed_handler)

	var user_changed_handler = Callable(self, "_on_user_changed")
	if not gdk.users.user_changed.is_connected(user_changed_handler):
		gdk.users.user_changed.connect(user_changed_handler)

	var primary_user_changed_handler = Callable(self, "_on_primary_user_changed")
	if not gdk.users.primary_user_changed.is_connected(primary_user_changed_handler):
		gdk.users.primary_user_changed.connect(primary_user_changed_handler)


func _maybe_start_default_user(gdk) -> void:
	var auto_add_primary_user: bool = bool(
			ProjectSettings.get_setting(SETTING_AUTO_ADD_PRIMARY_USER, false))
	if not auto_add_primary_user or not gdk.is_initialized():
		return

	if gdk.users.get_primary_user() != null:
		return

	if _startup_user_in_progress:
		return

	_startup_user_in_progress = true
	var startup_user_signal: Signal = gdk.users.add_default_user_async()
	startup_user_signal.connect(Callable(self, "_on_startup_user_completed"), CONNECT_ONE_SHOT)


func _should_skip_bootstrap() -> bool:
	var user_args: PackedStringArray = OS.get_cmdline_user_args()
	if user_args.has(GD_SCRIPT_CHECK_FLAG):
		return true

	var args: PackedStringArray = OS.get_cmdline_args()
	if args.has("--script") and args.has(TEST_SCRIPT_PATH):
		print("[GDK] Bootstrap skipped for headless tests")
		return true

	return false


func _on_gdk_initialized() -> void:
	print("[GDK] Runtime initialized")

	var gdk = get_gdk()
	if gdk == null:
		push_warning("[GDK] Bootstrap: extension not loaded")
		return

	_maybe_start_default_user(gdk)


func _on_gdk_shutdown_completed() -> void:
	_startup_user_in_progress = false


func _on_gdk_runtime_error(result) -> void:
	push_warning("[GDK] %s" % result.message)


func _on_startup_user_completed(result) -> void:
	_startup_user_in_progress = false

	if result == null:
		push_warning("[GDK] Bootstrap: silent sign-in could not start.")
		return

	if not result.ok:
		push_warning("[GDK] Bootstrap: silent sign-in did not complete successfully: %s" % result.message)


func _on_user_added(user) -> void:
	print("[GDK] User added: %s" % user.gamertag)


func _on_user_removed(local_id: int) -> void:
	print("[GDK] User removed: %d" % local_id)


func _on_user_changed(user, change_kind: String) -> void:
	print("[GDK] User changed (%s): %s" % [change_kind, user.gamertag])


func _on_primary_user_changed(user) -> void:
	if user:
		print("[GDK] Primary user: %s" % user.gamertag)
	else:
		print("[GDK] No primary user")


func _exit_tree() -> void:
	var gdk = get_gdk()
	if gdk != null and gdk.is_initialized():
		gdk.shutdown()
