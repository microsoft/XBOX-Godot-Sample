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
var _gdk_extension: Variant = null
var _gdk_load_attempted := false
var _last_initialize_result: Variant = null
var _last_default_user_result: Variant = null
var _default_user_attempted := false


func get_gdk() -> Object:
	if Engine.has_singleton("GDK"):
		return Engine.get_singleton("GDK")

	if not _gdk_load_attempted and _gdk_extension == null and FileAccess.file_exists(GDK_EXTENSION_PATH):
		_gdk_load_attempted = true
		_gdk_extension = load(GDK_EXTENSION_PATH)

	if Engine.has_singleton("GDK"):
		return Engine.get_singleton("GDK")

	return null


## Returns the GDKResult from the most recent bootstrap-driven `GDK.initialize()`
## call, or `null` if the autoload has not attempted initialization. Tests use
## this to distinguish "init never attempted" (returns `null`) from "init
## attempted and failed" (returns a result whose `.ok` is `false`).
func get_last_initialize_result() -> Variant:
	return _last_initialize_result


## Returns the GDKResult from the most recent bootstrap-driven
## `GDK.users.add_default_user_async()` call, or `null` if the autoload has
## not attempted to add a default user yet (or the request never completed).
## Tests use this to observe sign-in outcomes without polling a global
## `get_last_error()` accessor.
func get_last_default_user_result() -> Variant:
	return _last_default_user_result


## Returns `true` once the bootstrap autoload has begun the
## `add_default_user_async()` call (regardless of completion).
func has_attempted_default_user() -> bool:
	return _default_user_attempted


func _ready() -> void:
	if _should_skip_bootstrap():
		return

	var gdk: Object = get_gdk()
	if gdk == null:
		push_warning("[GDK] Bootstrap: 'GDK' singleton not registered. Is the godot_gdk GDExtension built and loaded?")
		return

	_bind_gdk_signals(gdk)

	var initialize_on_startup: bool = bool(
			ProjectSettings.get_setting(SETTING_INITIALIZE_ON_STARTUP, false))
	if initialize_on_startup and not gdk.is_initialized():
		var init_result: Variant = gdk.initialize()
		_last_initialize_result = init_result
		if init_result == null:
			push_warning("[GDK] Bootstrap: GDK.initialize() did not return a GDKResult.")
		elif init_result.ok:
			print("[GDK] Bootstrap: GDK.initialize() succeeded.")
			_maybe_start_default_user(gdk)
		else:
			push_warning("[GDK] Bootstrap: %s" % init_result.message)
	elif gdk.is_initialized():
		_maybe_start_default_user(gdk)


func _bind_gdk_signals(gdk: Object) -> void:
	var initialized_handler := Callable(self, "_on_gdk_initialized")
	if not gdk.initialized.is_connected(initialized_handler):
		gdk.initialized.connect(initialized_handler)

	var shutdown_handler := Callable(self, "_on_gdk_shutdown_completed")
	if not gdk.shutdown_completed.is_connected(shutdown_handler):
		gdk.shutdown_completed.connect(shutdown_handler)

	var runtime_error_handler := Callable(self, "_on_gdk_runtime_error")
	if not gdk.runtime_error.is_connected(runtime_error_handler):
		gdk.runtime_error.connect(runtime_error_handler)

	var user_changed_handler := Callable(self, "_on_gdk_user_changed")
	if not gdk.users.user_changed.is_connected(user_changed_handler):
		gdk.users.user_changed.connect(user_changed_handler)


func _maybe_start_default_user(gdk: Object) -> void:
	var auto_add_primary_user: bool = bool(
			ProjectSettings.get_setting(SETTING_AUTO_ADD_PRIMARY_USER, false))
	if not auto_add_primary_user or not gdk.is_initialized():
		return

	if gdk.users.get_primary_user() != null:
		return

	if _startup_user_in_progress:
		return

	_startup_user_in_progress = true
	_default_user_attempted = true
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

	var gdk: Object = get_gdk()
	if gdk == null:
		push_warning("[GDK] Bootstrap: extension not loaded")
		return

	_maybe_start_default_user(gdk)


func _on_gdk_shutdown_completed() -> void:
	_startup_user_in_progress = false


## Listener for the root `GDK.runtime_error` signal. After the result-only
## refactor that signal is reserved for `XError` callback events (the global
## X-error bridge); per-service errors are surfaced on
## `GDK.<service>.runtime_error` instead.
func _on_gdk_runtime_error(result: Variant) -> void:
	push_warning("[GDK] %s" % result.message)


func _on_startup_user_completed(result: Variant) -> void:
	_startup_user_in_progress = false
	_last_default_user_result = result

	if result == null:
		push_warning("[GDK] Bootstrap: silent sign-in could not start.")
		return

	if not result.ok:
		push_warning("[GDK] Bootstrap: silent sign-in did not complete successfully: %s" % result.message)


func _on_gdk_user_changed(user: Variant, change_kind: String) -> void:
	if user == null:
		return

	if change_kind == "added":
		print("[GDK] User added: %s" % user.gamertag)
	elif change_kind == "removed":
		print("[GDK] User removed: %d" % int(user.local_id))
	else:
		print("[GDK] User changed (%s): %s" % [change_kind, user.gamertag])


func _exit_tree() -> void:
	var gdk: Object = get_gdk()
	if gdk != null and gdk.is_initialized():
		gdk.shutdown()
