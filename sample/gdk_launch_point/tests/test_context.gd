extends RefCounted
## Lightweight test context for the godot_gameinput suite.

var pass_count := 0
var fail_count := 0
var skip_count := 0


func log_section(name: String) -> void:
	print("\n── %s ──" % name)


func log_pass(name: String, detail: String = "") -> void:
	pass_count += 1
	if detail:
		print("  PASS: %s — %s" % [name, detail])
	else:
		print("  PASS: %s" % name)


func log_fail(name: String, detail: String = "") -> void:
	fail_count += 1
	if detail:
		printerr("  FAIL: %s — %s" % [name, detail])
	else:
		printerr("  FAIL: %s" % name)


func log_skip(name: String, reason: String = "") -> void:
	skip_count += 1
	if reason:
		print("  SKIP: %s — %s" % [name, reason])
	else:
		print("  SKIP: %s" % name)


func assert_true(condition: bool, name: String, detail: String = "") -> void:
	if condition:
		log_pass(name, detail)
	else:
		log_fail(name, detail)


func assert_eq(actual, expected, name: String) -> void:
	if actual == expected:
		log_pass(name, str(actual))
	else:
		log_fail(name, "expected %s, got %s" % [str(expected), str(actual)])


func assert_eq_approx(actual: float, expected: float, name: String) -> void:
	if is_equal_approx(actual, expected):
		log_pass(name, str(actual))
	else:
		log_fail(name, "expected %s, got %s" % [str(expected), str(actual)])


func assert_not_null(value, name: String) -> void:
	if value != null:
		log_pass(name, str(typeof(value)))
	else:
		log_fail(name, "got null")


func assert_has_method(obj: Object, method_name: String, test_name: String = "") -> void:
	var label := test_name if test_name else "%s.%s() exists" % [obj.get_class(), method_name]
	assert_true(obj.has_method(method_name), label)


func assert_has_signal(obj: Object, signal_name: String, test_name: String = "") -> void:
	var label := test_name if test_name else "%s.%s signal exists" % [obj.get_class(), signal_name]
	assert_true(obj.has_signal(signal_name), label)


func get_gameinput():
	return Engine.get_singleton("GameInput") if Engine.has_singleton("GameInput") else null
