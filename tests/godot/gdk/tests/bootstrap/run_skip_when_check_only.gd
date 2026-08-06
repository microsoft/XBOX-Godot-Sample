extends SceneTree
## Wave 4 bootstrap mini-runner.
##
## Asserts: the `GDKBootstrap` autoload's `_should_skip_bootstrap()` skips
## the runtime init when the `--gd-script-check` user-arg is passed (the
## "check-only" code path used by the headless validator and by tests that
## want to load the project without booting the GDK runtime).
##
## We pin the contract by loading the bootstrap source script and asserting:
##   * GD_SCRIPT_CHECK_FLAG, TEST_SCRIPT_PATH, and GUT_COMMAND_SCRIPT_PATH
##     constants are present and match the documented values;
##   * `_should_skip_bootstrap()` exists, returns `bool`, and returns
##     `false` when the cmdline does NOT contain the check-only flag (the
##     negative case — pinning that the skip path is fenced behind the user
##     arg sentinel).

const BOOTSTRAP_SCRIPT := "res://addons/godot_gdk/runtime/gdk_bootstrap.gd"
const SCENARIO := "skip_when_check_only"

# This runner deliberately launches *without* `--gd-script-check` -- that is
# the whole point of the negative assertion below -- so the GDKBootstrap
# autoload boots the runtime and kicks off a silent sign-in alongside us. The
# assertions here are synchronous constant/reflection checks that finish in
# microseconds, so quitting the moment they pass tears the SceneTree down while
# that sign-in is still in flight. `_exit_tree()` then calls gdk.shutdown(),
# which cancels the in-flight request, and the process aborts during unload
# (exit 0x80004004) even though every assertion passed.
#
# The sibling runners in this directory all let startup settle before quitting.
# Do the same: record the outcome, then give the autoload's startup work a
# bounded window to finish before we quit.
const STARTUP_SETTLE_MSEC := 6000

var _exit_code := 0


func _initialize() -> void:
	_exit_code = _check_contract()
	# Defer the quit so the autoload's in-flight silent sign-in can settle.
	create_timer(float(STARTUP_SETTLE_MSEC) / 1000.0).timeout.connect(Callable(self, "_finish"))


func _finish() -> void:
	quit(_exit_code)


# Returns the process exit code: 0 on success, non-zero scenario code on
# failure. Emits the BOOTSTRAP_OK / BOOTSTRAP_FAIL line as a side effect.
func _check_contract() -> int:
	if not FileAccess.file_exists(BOOTSTRAP_SCRIPT):
		printerr("BOOTSTRAP_FAIL: %s -- bootstrap script not found at %s" % [SCENARIO, BOOTSTRAP_SCRIPT])
		return 2

	var bootstrap_script: GDScript = load(BOOTSTRAP_SCRIPT)
	if bootstrap_script == null:
		printerr("BOOTSTRAP_FAIL: %s -- could not load bootstrap script" % SCENARIO)
		return 3

	var constants: Dictionary = bootstrap_script.get_script_constant_map()
	if not constants.has("GD_SCRIPT_CHECK_FLAG"):
		printerr("BOOTSTRAP_FAIL: %s -- bootstrap script missing GD_SCRIPT_CHECK_FLAG constant" % SCENARIO)
		return 4
	var check_flag = constants.get("GD_SCRIPT_CHECK_FLAG", "")
	if str(check_flag) != "--gd-script-check":
		printerr("BOOTSTRAP_FAIL: %s -- GD_SCRIPT_CHECK_FLAG is %s; expected '--gd-script-check'" % [SCENARIO, str(check_flag)])
		return 5

	if not constants.has("TEST_SCRIPT_PATH"):
		printerr("BOOTSTRAP_FAIL: %s -- bootstrap script missing TEST_SCRIPT_PATH constant" % SCENARIO)
		return 6
	var test_path = constants.get("TEST_SCRIPT_PATH", "")
	if str(test_path) != "res://tests/run_tests.gd":
		printerr("BOOTSTRAP_FAIL: %s -- TEST_SCRIPT_PATH is %s; expected 'res://tests/run_tests.gd'" % [SCENARIO, str(test_path)])
		return 7

	if not constants.has("GUT_COMMAND_SCRIPT_PATH"):
		printerr("BOOTSTRAP_FAIL: %s -- bootstrap script missing GUT_COMMAND_SCRIPT_PATH constant" % SCENARIO)
		return 8
	var gut_path = constants.get("GUT_COMMAND_SCRIPT_PATH", "")
	if str(gut_path) != "res://addons/gut/gut_cmdln.gd":
		printerr("BOOTSTRAP_FAIL: %s -- GUT_COMMAND_SCRIPT_PATH is %s; expected 'res://addons/gut/gut_cmdln.gd'" % [SCENARIO, str(gut_path)])
		return 9

	var bootstrap_instance: Node = bootstrap_script.new()
	if bootstrap_instance == null:
		printerr("BOOTSTRAP_FAIL: %s -- bootstrap_script.new() returned null" % SCENARIO)
		return 10
	if not bootstrap_instance.has_method("_should_skip_bootstrap"):
		printerr("BOOTSTRAP_FAIL: %s -- bootstrap autoload missing _should_skip_bootstrap()" % SCENARIO)
		bootstrap_instance.queue_free()
		return 11

	var skip_value = bootstrap_instance._should_skip_bootstrap()
	if typeof(skip_value) != TYPE_BOOL:
		printerr("BOOTSTRAP_FAIL: %s -- _should_skip_bootstrap() returned non-bool" % SCENARIO)
		bootstrap_instance.queue_free()
		return 12
	if skip_value:
		printerr("BOOTSTRAP_FAIL: %s -- _should_skip_bootstrap() returned true without check-only flag in cmdline" % SCENARIO)
		bootstrap_instance.queue_free()
		return 13

	bootstrap_instance.queue_free()
	print("BOOTSTRAP_OK: %s (GD_SCRIPT_CHECK_FLAG, TEST_SCRIPT_PATH, GUT_COMMAND_SCRIPT_PATH, _should_skip_bootstrap() pinned)" % SCENARIO)
	return 0
