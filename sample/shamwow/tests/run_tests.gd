extends SceneTree
## godot_gameinput headless test runner.
## Run from sample/shamwow:
##   godot --headless --script res://tests/run_tests.gd

const TestContext = preload("res://tests/test_context.gd")
const GameInputCoreSuite = preload("res://tests/suites/gameinput_core_suite.gd")
const GameInputResourceSuite = preload("res://tests/suites/gameinput_resource_suite.gd")
const GameInputMapperSuite = preload("res://tests/suites/gameinput_mapper_suite.gd")


func _initialize() -> void:
	print("╔══════════════════════════════════════╗")
	print("║   GodotGameInput Headless Tests      ║")
	print("╚══════════════════════════════════════╝")

	var context := TestContext.new()
	var suites := [
		GameInputCoreSuite.new(),
		GameInputResourceSuite.new(),
		GameInputMapperSuite.new(),
	]

	for suite in suites:
		suite.run(context)

	var total := context.pass_count + context.fail_count + context.skip_count
	print("\n══════════════════════════════════════")
	print("Results: %d passed, %d failed, %d skipped (of %d)" % [
		context.pass_count, context.fail_count, context.skip_count, total])
	print("══════════════════════════════════════")

	if context.fail_count > 0:
		printerr("SUITE FAILED")
		quit(1)
	else:
		print("SUITE PASSED")
		quit(0)
