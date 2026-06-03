extends SceneTree
## Godot PlayFab contract test runner.
## Run from sample\playfab_demo:
##   godot --headless --script res://tests/run_tests.gd

const TestContext = preload("res://tests/test_context.gd")
const CoreSuite = preload("res://tests/suites/core_suite.gd")
const UsersSuite = preload("res://tests/suites/users_suite.gd")
const ServicesSuite = preload("res://tests/suites/services_suite.gd")
const IntegrationSuite = preload("res://tests/suites/integration_suite.gd")


func _initialize() -> void:
	print("╔══════════════════════════════════════╗")
	print("║   Godot PlayFab Contract Tests      ║")
	print("╚══════════════════════════════════════╝")

	call_deferred("_run_suites")


func _run_suites() -> void:
	var context = TestContext.new()
	var suites = [
		CoreSuite.new(),
		UsersSuite.new(),
		ServicesSuite.new(),
		IntegrationSuite.new(),
	]

	for suite in suites:
		await suite.run(context)

	var total = context.pass_count + context.fail_count + context.skip_count
	print("\n══════════════════════════════════════")
	print("Results: %d passed, %d failed, %d skipped (of %d)" % [
		context.pass_count, context.fail_count, context.skip_count, total])
	print("══════════════════════════════════════")

	if context.fail_count > 0:
		printerr("SUITE FAILED")
		call_deferred("_finish", 1)
	else:
		print("SUITE PASSED")
		call_deferred("_finish", 0)


func _finish(exit_code: int) -> void:
	quit(exit_code)
