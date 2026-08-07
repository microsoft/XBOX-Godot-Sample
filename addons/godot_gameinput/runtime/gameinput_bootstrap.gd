extends Node
## Godot GameInput — Bootstrap autoload
##
## Installed by `GameInputEditorPlugin`. Reads Project Settings and:
##  * Resolves the Engine singleton name from `game_input/runtime/singleton_name`
##    (default `"GameInput"`).
##  * Calls `GameInput.initialize()` if `game_input/runtime/initialize_on_startup` is true.
##  * Calls `GameInput.poll()` every `_process` if `game_input/runtime/auto_poll` is true.
##  * Spawns a `GameInputMapper` child if `game_input/mapper/default_action_map`
##    points at a `GameInputActionMap` resource — turning the action-bridge into a
##    zero-code, project-settings-only integration for projects that just want
##    "controller goes through GameInput".
##  * Skips headless validation and sample test runs.
##  * Calls `GameInput.shutdown()` when the tree is being torn down.
##
## Apps that need finer control can leave the settings disabled and call the API
## directly. `GameInputMapper` nodes also call `poll()` defensively (it's
## per-frame idempotent), so adding a Mapper to a scene is enough even when the
## bootstrap's `auto_poll` is off.

const SETTING_INITIALIZE_ON_STARTUP := "game_input/runtime/initialize_on_startup"
const SETTING_AUTO_POLL := "game_input/runtime/auto_poll"
const SETTING_DEFAULT_ACTION_MAP := "game_input/mapper/default_action_map"
const SETTING_SINGLETON_NAME := "game_input/runtime/singleton_name"
const DEFAULT_SINGLETON_NAME := "GameInput"
## Native class the singleton must be an instance of. The singleton *name* is
## configurable; the class it resolves to is not, so this is what proves a
## looked-up singleton is actually the GameInput runtime.
const SINGLETON_CLASS_NAME := "GameInput"
const GD_SCRIPT_CHECK_FLAG := "--gd-script-check"
const TEST_SCRIPT_PATH := "res://tests/run_tests.gd"

var _auto_poll: bool = false
var _initialized_here: bool = false
var _default_mapper: Node = null


## Returns the Engine singleton name configured by
## `game_input/runtime/singleton_name`, falling back to `"GameInput"` when the
## setting is unset or not a valid identifier. Mirrors the resolution in the
## addon's `register_types.cpp`.
static func get_singleton_name() -> String:
	var configured := str(
			ProjectSettings.get_setting(SETTING_SINGLETON_NAME, DEFAULT_SINGLETON_NAME)).strip_edges()
	if configured.is_empty() or not configured.is_valid_ascii_identifier():
		return DEFAULT_SINGLETON_NAME
	return configured


## Resolves the registered singleton by configured name, retrying under the
## default name because the C++ side falls back to `"GameInput"` when the
## configured name is unusable (for example when it collides with an existing
## singleton).
##
## Each candidate is class-checked: a configured name that collides with an
## unrelated engine singleton (say `Input`) still resolves through
## `Engine.has_singleton()`, and returning that object would silently hand
## callers the wrong runtime.
static func find_singleton() -> Object:
	var configured := get_singleton_name()
	if Engine.has_singleton(configured):
		var candidate: Object = Engine.get_singleton(configured)
		if candidate != null and candidate.is_class(SINGLETON_CLASS_NAME):
			return candidate
	if configured != DEFAULT_SINGLETON_NAME and Engine.has_singleton(DEFAULT_SINGLETON_NAME):
		var fallback: Object = Engine.get_singleton(DEFAULT_SINGLETON_NAME)
		if fallback != null and fallback.is_class(SINGLETON_CLASS_NAME):
			return fallback
	return null


func _ready() -> void:
	if _should_skip_bootstrap():
		return

	var gi: Object = find_singleton()
	if gi == null:
		push_warning("[GameInput] Bootstrap: '%s' singleton not registered. " % get_singleton_name() +
				"Is the godot_gameinput GDExtension built and loaded?")
		set_process(false)
		return

	_auto_poll = bool(ProjectSettings.get_setting(SETTING_AUTO_POLL, true))

	var initialize_on_startup := bool(
			ProjectSettings.get_setting(SETTING_INITIALIZE_ON_STARTUP, false))
	if initialize_on_startup:
		if gi.is_initialized():
			# Editor reload or another caller already initialised it — skip.
			pass
		elif gi.initialize():
			_initialized_here = true
			print("[GameInput] Bootstrap: GameInput.initialize() succeeded.")
		else:
			push_warning("[GameInput] Bootstrap: GameInput.initialize() failed. " +
					"Subsequent calls will return safe defaults.")

	_maybe_spawn_default_mapper()

	set_process(_auto_poll)


func _process(_delta: float) -> void:
	if not _auto_poll:
		return
	var gi: Object = find_singleton()
	if gi != null:
		gi.poll()


func _notification(what: int) -> void:
	if what == NOTIFICATION_PREDELETE or what == NOTIFICATION_WM_CLOSE_REQUEST:
		# Only call shutdown() if WE initialized it. Otherwise leave it to the caller
		# that brought it up (e.g. tests, editor tooling).
		if _initialized_here:
			var gi: Object = find_singleton()
			if gi != null and gi.is_initialized():
				gi.shutdown()
				_initialized_here = false


func _should_skip_bootstrap() -> bool:
	var user_args: PackedStringArray = OS.get_cmdline_user_args()
	if user_args.has(GD_SCRIPT_CHECK_FLAG):
		return true

	var args: PackedStringArray = OS.get_cmdline_args()
	if args.has("--script") and args.has(TEST_SCRIPT_PATH):
		print("[GameInput] Bootstrap skipped for headless tests")
		return true

	return false


func _maybe_spawn_default_mapper() -> void:
	# Honour `game_input/mapper/default_action_map`: if the setting points at a
	# loadable GameInputActionMap, spawn a GameInputMapper child wired to it.
	# Soft-fails on every error path so a broken setting never breaks startup.
	var raw_path: Variant = ProjectSettings.get_setting(SETTING_DEFAULT_ACTION_MAP, "")
	var path := str(raw_path).strip_edges()
	if path.is_empty():
		return

	if not ResourceLoader.exists(path):
		push_warning("[GameInput] Bootstrap: default_action_map '%s' does not exist." % path)
		return

	var resource: Resource = ResourceLoader.load(path)
	if resource == null:
		push_warning("[GameInput] Bootstrap: default_action_map '%s' failed to load." % path)
		return

	# Use `is`-via-class-name through ClassDB since GameInputActionMap is a
	# native class registered by this addon and not a script class.
	if not ClassDB.is_parent_class(resource.get_class(), "GameInputActionMap"):
		push_warning("[GameInput] Bootstrap: default_action_map '%s' is a %s, not a GameInputActionMap." %
				[path, resource.get_class()])
		return

	if not ClassDB.class_exists("GameInputMapper"):
		push_warning("[GameInput] Bootstrap: GameInputMapper class is not registered.")
		return

	var mapper: Node = ClassDB.instantiate("GameInputMapper")
	if mapper == null:
		push_warning("[GameInput] Bootstrap: failed to instantiate GameInputMapper.")
		return

	mapper.name = "DefaultMapper"
	mapper.set("action_map", resource)
	add_child(mapper)
	_default_mapper = mapper
	print("[GameInput] Bootstrap: default GameInputMapper spawned for '%s'." % path)
