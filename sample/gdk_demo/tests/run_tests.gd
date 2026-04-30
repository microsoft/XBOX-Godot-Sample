extends SceneTree
## GodotGDK runtime/services API suite runner
## Run: godot --headless --script res://tests/run_tests.gd

const TestContext = preload("res://tests/test_context.gd")
const CoreSuite = preload("res://tests/suites/core_suite.gd")
const UsersSuite = preload("res://tests/suites/users_suite.gd")
const AchievementsSuite = preload("res://tests/suites/achievements_suite.gd")
const PresenceSuite = preload("res://tests/suites/presence_suite.gd")
const SocialSuite = preload("res://tests/suites/social_suite.gd")
const IntegrationSuite = preload("res://tests/suites/integration_suite.gd")
const PackagingSuite = preload("res://tests/suites/packaging_suite.gd")

func _initialize() -> void:
	print("╔══════════════════════════════════════╗")
	print("║   GodotGDK Runtime/Services Tests    ║")
	print("╚══════════════════════════════════════╝")

	call_deferred("_run_suites")

func _run_suites() -> void:
	var context = TestContext.new()
	var suites = [
		CoreSuite.new(),
		UsersSuite.new(),
		AchievementsSuite.new(),
		PresenceSuite.new(),
		SocialSuite.new(),
		IntegrationSuite.new(),
		PackagingSuite.new()
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
		quit(1)
	else:
		print("SUITE PASSED")
		quit(0)
