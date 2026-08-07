extends Node
## Godot PlayFab - Bootstrap autoload.
##
## Installed by `GodotPlayFab` when the editor plugin is enabled. Reads
## Project Settings and:
##  * Resolves the Engine singleton name from `playfab/runtime/singleton_name`
##    (default `"PlayFab"`).
##  * Calls `PlayFab.initialize()` when `playfab/runtime/initialize_on_startup`
##    is true.
##  * Skips headless validation and sample test runs.
##  * Shuts the runtime down when the SceneTree is torn down.
##
## PlayFab sign-in (`PlayFab.users.sign_in_with_xuser_async` /
## `sign_in_with_custom_id_async`) is intentionally NOT auto-driven from
## here: PlayFab sessions need a per-player key (a `GDKUser` or a custom
## id) and that decision belongs to title code, not a project-wide flag.

const PLAYFAB_EXTENSION_PATH := "res://addons/godot_playfab/godot_playfab.gdextension"
const GD_SCRIPT_CHECK_FLAG := "--gd-script-check"
const TEST_SCRIPT_PATH := "res://tests/run_tests.gd"
const SETTING_INITIALIZE_ON_STARTUP := "playfab/runtime/initialize_on_startup"
const SETTING_SINGLETON_NAME := "playfab/runtime/singleton_name"
const DEFAULT_SINGLETON_NAME := "PlayFab"

var _playfab_extension: Variant = null
var _playfab_load_attempted := false
var _last_initialize_result: Variant = null


## Returns the Engine singleton name configured by
## `playfab/runtime/singleton_name`, falling back to `"PlayFab"` when the
## setting is unset or not a valid identifier. Mirrors the resolution in the
## addon's `register_types.cpp`.
static func get_singleton_name() -> String:
	var configured := str(
			ProjectSettings.get_setting(SETTING_SINGLETON_NAME, DEFAULT_SINGLETON_NAME)).strip_edges()
	if configured.is_empty() or not configured.is_valid_ascii_identifier():
		return DEFAULT_SINGLETON_NAME
	return configured


## Resolves the registered singleton by configured name, retrying under the
## default name because the C++ side falls back to `"PlayFab"` when the
## configured name is unusable (for example when it collides with an existing
## singleton).
static func find_singleton() -> Object:
	var configured := get_singleton_name()
	if Engine.has_singleton(configured):
		return Engine.get_singleton(configured)
	if configured != DEFAULT_SINGLETON_NAME and Engine.has_singleton(DEFAULT_SINGLETON_NAME):
		return Engine.get_singleton(DEFAULT_SINGLETON_NAME)
	return null


func get_playfab() -> Object:
	var playfab: Object = find_singleton()
	if playfab != null:
		return playfab

	if not _playfab_load_attempted and _playfab_extension == null and FileAccess.file_exists(PLAYFAB_EXTENSION_PATH):
		_playfab_load_attempted = true
		_playfab_extension = load(PLAYFAB_EXTENSION_PATH)

	return find_singleton()


## Returns the PlayFabResult from the most recent bootstrap-driven
## `PlayFab.initialize()` call, or `null` if the autoload has not attempted
## initialization. Tests use this to distinguish "init never attempted"
## (returns `null`) from "init attempted and failed" (returns a result whose
## `.ok` is `false`).
func get_last_initialize_result() -> Variant:
	return _last_initialize_result


func _ready() -> void:
	if _should_skip_bootstrap():
		return

	var playfab: Object = get_playfab()
	if playfab == null:
		push_warning("[PlayFab] Bootstrap: '%s' singleton not registered. Is the godot_playfab GDExtension built and loaded?"
				% get_singleton_name())
		return

	_bind_playfab_signals(playfab)

	var initialize_on_startup: bool = bool(
			ProjectSettings.get_setting(SETTING_INITIALIZE_ON_STARTUP, false))
	if initialize_on_startup and not playfab.is_initialized():
		var init_result: Variant = playfab.initialize()
		_last_initialize_result = init_result
		if init_result == null:
			push_warning("[PlayFab] Bootstrap: PlayFab.initialize() did not return a PlayFabResult.")
		elif init_result.ok:
			print("[PlayFab] Bootstrap: PlayFab.initialize() succeeded.")
		else:
			push_warning("[PlayFab] Bootstrap: %s" % init_result.message)


func _bind_playfab_signals(playfab: Object) -> void:
	var initialized_handler := Callable(self, "_on_playfab_initialized")
	if not playfab.initialized.is_connected(initialized_handler):
		playfab.initialized.connect(initialized_handler)

	var shutdown_handler := Callable(self, "_on_playfab_shutdown_completed")
	if not playfab.shutdown_completed.is_connected(shutdown_handler):
		playfab.shutdown_completed.connect(shutdown_handler)


func _should_skip_bootstrap() -> bool:
	return _should_skip_bootstrap_for_args(
			OS.get_cmdline_user_args(),
			OS.get_cmdline_args())


## Pure decision function used by `_should_skip_bootstrap()` and exposed for
## tests so the skip contract can be exercised positively (with the
## check-only flag present) without re-invoking Godot.
func _should_skip_bootstrap_for_args(user_args: PackedStringArray, args: PackedStringArray) -> bool:
	if user_args.has(GD_SCRIPT_CHECK_FLAG):
		return true

	if args.has("--script") and args.has(TEST_SCRIPT_PATH):
		print("[PlayFab] Bootstrap skipped for headless tests")
		return true

	return false


func _on_playfab_initialized() -> void:
	print("[PlayFab] Runtime initialized")


func _on_playfab_shutdown_completed() -> void:
	print("[PlayFab] Runtime shutdown")


func _exit_tree() -> void:
	var playfab: Object = get_playfab()
	if playfab != null and playfab.is_initialized():
		playfab.shutdown()
