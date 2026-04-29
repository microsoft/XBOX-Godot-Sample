extends RefCounted

const DEFAULT_ASYNC_TIMEOUT_MSEC = 5000
const ASYNC_POLL_INTERVAL_MSEC = 10

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

func assert_not_null(value, name: String) -> void:
	if value != null:
		log_pass(name, str(typeof(value)))
	else:
		log_fail(name, "got null")

func assert_has_method(obj: Object, method_name: String, test_name: String = "") -> void:
	var label = test_name if test_name else "%s.%s() exists" % [obj.get_class(), method_name]
	assert_true(obj.has_method(method_name), label)

func assert_has_signal(obj: Object, signal_name: String, test_name: String = "") -> void:
	var label = test_name if test_name else "%s.%s signal exists" % [obj.get_class(), signal_name]
	assert_true(obj.has_signal(signal_name), label)

func disconnect_signal_handlers(obj: Object, signal_names: Array) -> void:
	for signal_name in signal_names:
		for conn in obj.get_signal_connection_list(signal_name):
			obj.disconnect(signal_name, conn["callable"])

func get_gdk():
	return Engine.get_singleton("GDK")

func reset_runtime() -> void:
	var gdk = get_gdk()
	if gdk != null:
		gdk.shutdown()

func initialize_runtime():
	var gdk = get_gdk()
	if gdk == null:
		return null

	reset_runtime()
	return gdk.initialize()

func wait_for_op(op, timeout_msec: int = DEFAULT_ASYNC_TIMEOUT_MSEC):
	if op == null:
		return null

	var gdk = get_gdk()
	var started_msec = Time.get_ticks_msec()
	while not op.is_done():
		if gdk != null:
			gdk.dispatch()
		if Time.get_ticks_msec() - started_msec >= timeout_msec:
			return null
		OS.delay_msec(ASYNC_POLL_INTERVAL_MSEC)

	if gdk != null:
		gdk.dispatch()

	return op.get_result()

func ensure_primary_user(timeout_msec: int = DEFAULT_ASYNC_TIMEOUT_MSEC) -> Dictionary:
	var outcome = {
		"had_existing_user": false,
		"op": null,
		"result": null,
		"user": null
	}

	var gdk = get_gdk()
	if gdk == null:
		return outcome

	var users = gdk.get_users()
	if users == null:
		return outcome

	var primary_user = users.get_primary_user()
	if primary_user != null:
		outcome["had_existing_user"] = true
		outcome["user"] = primary_user
		return outcome

	var op = users.add_default_user_async()
	outcome["op"] = op
	if op == null:
		return outcome

	var result = wait_for_op(op, timeout_msec)
	outcome["result"] = result
	if result != null and result.ok and result.data != null:
		outcome["user"] = result.data
	else:
		outcome["user"] = users.get_primary_user()

	return outcome

func assert_result_error(result, expected_code: String, name: String) -> void:
	assert_not_null(result, "%s returns GDKResult" % name)
	if result == null:
		return

	assert_eq(result.ok, false, "%s result.ok == false" % name)
	assert_eq(result.code, expected_code, "%s error code" % name)
	assert_true(result.message.length() > 0, "%s error message present" % name)

func assert_dict_has_key(dictionary: Dictionary, key: String, name: String) -> void:
	assert_true(dictionary.has(key), name)
