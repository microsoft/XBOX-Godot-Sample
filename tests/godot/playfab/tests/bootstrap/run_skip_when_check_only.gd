extends SceneTree
## PlayFab bootstrap mini-runner.
##
## Asserts: the `PlayFabBootstrap` autoload's check-only / headless-test
## skip path is fenced behind the documented sentinels.
##
## We pin the contract by loading the bootstrap source script and asserting:
##   * GD_SCRIPT_CHECK_FLAG and TEST_SCRIPT_PATH constants are present and
##     match the documented values;
##   * `_should_skip_bootstrap()` exists, returns `bool`, and returns
##     `false` against the test host's real cmdline (the negative case --
##     no check-only flag is forwarded to this runner);
##   * `_should_skip_bootstrap_for_args()` returns `true` when the
##     `--gd-script-check` user-arg is supplied, and `true` when the
##     headless-test script path appears on the main cmdline (the
##     positive cases that protect parse-time validators and the in-repo
##     test runner from booting the PlayFab runtime).

const BOOTSTRAP_SCRIPT := "res://addons/godot_playfab/runtime/playfab_bootstrap.gd"
const SCENARIO := "skip_when_check_only"


func _initialize() -> void:
	if not FileAccess.file_exists(BOOTSTRAP_SCRIPT):
		printerr("BOOTSTRAP_FAIL: %s -- bootstrap script not found at %s" % [SCENARIO, BOOTSTRAP_SCRIPT])
		quit(2)
		return

	var bootstrap_script: GDScript = load(BOOTSTRAP_SCRIPT)
	if bootstrap_script == null:
		printerr("BOOTSTRAP_FAIL: %s -- could not load bootstrap script" % SCENARIO)
		quit(3)
		return

	var constants: Dictionary = bootstrap_script.get_script_constant_map()
	if not constants.has("GD_SCRIPT_CHECK_FLAG"):
		printerr("BOOTSTRAP_FAIL: %s -- bootstrap script missing GD_SCRIPT_CHECK_FLAG constant" % SCENARIO)
		quit(4)
		return
	var check_flag = constants.get("GD_SCRIPT_CHECK_FLAG", "")
	if str(check_flag) != "--gd-script-check":
		printerr("BOOTSTRAP_FAIL: %s -- GD_SCRIPT_CHECK_FLAG is %s; expected '--gd-script-check'" % [SCENARIO, str(check_flag)])
		quit(5)
		return

	if not constants.has("TEST_SCRIPT_PATH"):
		printerr("BOOTSTRAP_FAIL: %s -- bootstrap script missing TEST_SCRIPT_PATH constant" % SCENARIO)
		quit(6)
		return
	var test_path = constants.get("TEST_SCRIPT_PATH", "")
	if str(test_path) != "res://tests/run_tests.gd":
		printerr("BOOTSTRAP_FAIL: %s -- TEST_SCRIPT_PATH is %s; expected 'res://tests/run_tests.gd'" % [SCENARIO, str(test_path)])
		quit(7)
		return

	var bootstrap_instance: Node = bootstrap_script.new()
	if bootstrap_instance == null:
		printerr("BOOTSTRAP_FAIL: %s -- bootstrap_script.new() returned null" % SCENARIO)
		quit(8)
		return
	if not bootstrap_instance.has_method("_should_skip_bootstrap"):
		printerr("BOOTSTRAP_FAIL: %s -- bootstrap autoload missing _should_skip_bootstrap()" % SCENARIO)
		bootstrap_instance.queue_free()
		quit(9)
		return
	if not bootstrap_instance.has_method("_should_skip_bootstrap_for_args"):
		printerr("BOOTSTRAP_FAIL: %s -- bootstrap autoload missing _should_skip_bootstrap_for_args()" % SCENARIO)
		bootstrap_instance.queue_free()
		quit(10)
		return

	var skip_value = bootstrap_instance._should_skip_bootstrap()
	if typeof(skip_value) != TYPE_BOOL:
		printerr("BOOTSTRAP_FAIL: %s -- _should_skip_bootstrap() returned non-bool" % SCENARIO)
		bootstrap_instance.queue_free()
		quit(11)
		return
	if skip_value:
		printerr("BOOTSTRAP_FAIL: %s -- _should_skip_bootstrap() returned true without check-only flag in cmdline" % SCENARIO)
		bootstrap_instance.queue_free()
		quit(12)
		return

	var positive_user_arg: bool = bootstrap_instance._should_skip_bootstrap_for_args(
			PackedStringArray([str(check_flag)]),
			PackedStringArray())
	if not positive_user_arg:
		printerr("BOOTSTRAP_FAIL: %s -- _should_skip_bootstrap_for_args(['%s'], []) returned false; expected true" % [SCENARIO, str(check_flag)])
		bootstrap_instance.queue_free()
		quit(13)
		return

	var positive_test_script: bool = bootstrap_instance._should_skip_bootstrap_for_args(
			PackedStringArray(),
			PackedStringArray(["--script", str(test_path)]))
	if not positive_test_script:
		printerr("BOOTSTRAP_FAIL: %s -- _should_skip_bootstrap_for_args([], ['--script', '%s']) returned false; expected true" % [SCENARIO, str(test_path)])
		bootstrap_instance.queue_free()
		quit(14)
		return

	var negative_empty: bool = bootstrap_instance._should_skip_bootstrap_for_args(
			PackedStringArray(),
			PackedStringArray())
	if negative_empty:
		printerr("BOOTSTRAP_FAIL: %s -- _should_skip_bootstrap_for_args([], []) returned true; expected false" % SCENARIO)
		bootstrap_instance.queue_free()
		quit(15)
		return

	bootstrap_instance.queue_free()
	print("BOOTSTRAP_OK: %s (constants pinned; negative + both positive skip paths verified)" % SCENARIO)
	quit(0)
