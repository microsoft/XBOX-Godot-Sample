extends RefCounted

const DEFAULT_ASYNC_TIMEOUT_MSEC = 5000
const ASYNC_POLL_INTERVAL_MSEC = 10
const EMBED_DISPATCH_SETTING = "gdk/runtime/embed_dispatch"
const GDK_EXTENSION_PATH = "res://addons/godot_gdk/godot_gdk.gdextension"

var pass_count := 0
var fail_count := 0
var skip_count := 0
var _gdk_extension = null

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

func instantiate_class(target_class: String):
	if not ClassDB.class_exists(target_class) or not ClassDB.can_instantiate(target_class):
		return null
	return ClassDB.instantiate(target_class)

func get_class_constant(target_class: String, constant_name: String) -> int:
	return ClassDB.class_get_integer_constant(target_class, constant_name)

func is_class_instance(value, target_class: String) -> bool:
	if value == null or typeof(value) != TYPE_OBJECT:
		return false
	var object_value: Object = value
	return object_value.is_class(target_class)

func assert_object_is(value, target_class: String, name: String) -> void:
	assert_true(is_class_instance(value, target_class), name)

func disconnect_signal_handlers(obj: Object, signal_names: Array) -> void:
	for signal_name in signal_names:
		for conn in obj.get_signal_connection_list(signal_name):
			obj.disconnect(signal_name, conn["callable"])

func get_gdk():
	if Engine.has_singleton("GDK"):
		return Engine.get_singleton("GDK")

	if _gdk_extension == null and FileAccess.file_exists(GDK_EXTENSION_PATH):
		_gdk_extension = load(GDK_EXTENSION_PATH)

	if Engine.has_singleton("GDK"):
		return Engine.get_singleton("GDK")

	return null

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

func get_embed_dispatch_enabled() -> bool:
	return bool(ProjectSettings.get_setting(EMBED_DISPATCH_SETTING, true))

func set_embed_dispatch_enabled(enabled: bool) -> void:
	ProjectSettings.set_setting(EMBED_DISPATCH_SETTING, enabled)

func get_setting_default(setting_name: String):
	if ProjectSettings.property_can_revert(setting_name):
		return ProjectSettings.property_get_revert(setting_name)
	return null

func track_signal(async_signal) -> Dictionary:
	var state := {
		"completed": false,
		"result": null
	}
	if typeof(async_signal) != TYPE_SIGNAL:
		return state
	async_signal.connect(
		func(result):
			state["completed"] = true
			state["result"] = result,
		CONNECT_ONE_SHOT)
	return state

func wait_for_tracked_signal(state: Dictionary, timeout_msec: int = DEFAULT_ASYNC_TIMEOUT_MSEC):
	var gdk = get_gdk()
	var main_loop = Engine.get_main_loop()
	var started_msec = Time.get_ticks_msec()
	while not bool(state.get("completed", false)):
		if gdk != null:
			gdk.dispatch()
		if Time.get_ticks_msec() - started_msec >= timeout_msec:
			return null
		if main_loop != null and main_loop.has_signal("process_frame"):
			await main_loop.process_frame
		else:
			OS.delay_msec(ASYNC_POLL_INTERVAL_MSEC)

	if gdk != null:
		gdk.dispatch()

	return state.get("result")

func wait_for_signal(async_signal, timeout_msec: int = DEFAULT_ASYNC_TIMEOUT_MSEC):
	if typeof(async_signal) != TYPE_SIGNAL:
		return null
	return await wait_for_tracked_signal(track_signal(async_signal), timeout_msec)

func advance_process_frames(frame_count: int) -> bool:
	var main_loop = Engine.get_main_loop()
	if main_loop == null or not main_loop.has_signal("process_frame"):
		return false

	for _frame_index in range(frame_count):
		await main_loop.process_frame

	return true

func wait_for_tracked_signal_without_manual_dispatch(state: Dictionary, timeout_msec: int = DEFAULT_ASYNC_TIMEOUT_MSEC):
	var main_loop = Engine.get_main_loop()
	if main_loop == null or not main_loop.has_signal("process_frame"):
		return null

	var started_msec = Time.get_ticks_msec()
	while not bool(state.get("completed", false)):
		if Time.get_ticks_msec() - started_msec >= timeout_msec:
			return null
		await main_loop.process_frame

	return state.get("result")

func wait_for_signal_without_manual_dispatch(async_signal, timeout_msec: int = DEFAULT_ASYNC_TIMEOUT_MSEC):
	if typeof(async_signal) != TYPE_SIGNAL:
		return null
	return await wait_for_tracked_signal_without_manual_dispatch(track_signal(async_signal), timeout_msec)

func ensure_primary_user(timeout_msec: int = DEFAULT_ASYNC_TIMEOUT_MSEC) -> Dictionary:
	var outcome = {
		"had_existing_user": false,
		"signal": null,
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

	var completion_signal = users.add_default_user_async()
	outcome["signal"] = completion_signal
	outcome["op"] = completion_signal
	if typeof(completion_signal) != TYPE_SIGNAL:
		return outcome

	var result = await wait_for_signal(completion_signal, timeout_msec)
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

func assert_signal_result_error(async_signal, expected_code: String, name: String, timeout_msec: int = DEFAULT_ASYNC_TIMEOUT_MSEC) -> void:
	assert_true(typeof(async_signal) == TYPE_SIGNAL, "%s returns completion Signal" % name)
	if typeof(async_signal) != TYPE_SIGNAL:
		return
	assert_result_error(await wait_for_signal(async_signal, timeout_msec), expected_code, name)

func assert_dict_has_key(dictionary: Dictionary, key: String, name: String) -> void:
	assert_true(dictionary.has(key), name)
