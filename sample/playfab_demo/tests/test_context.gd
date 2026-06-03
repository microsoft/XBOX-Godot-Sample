extends RefCounted
## Shared helpers for the PlayFab contract suite.

const DEFAULT_ASYNC_TIMEOUT_MSEC := 5000
const ASYNC_POLL_INTERVAL_MSEC := 10

const PLAYFAB_EXTENSION_PATH := "res://addons/godot_playfab/godot_playfab.gdextension"
const GDK_EXTENSION_PATH := "res://addons/godot_gdk/godot_gdk.gdextension"

const PLAYFAB_TITLE_ID_SETTING := "playfab/titleid"
const PLAYFAB_ENDPOINT_SETTING := "playfab/endpoint"
const PLAYFAB_EMBED_DISPATCH_SETTING := "playfab/runtime/embed_dispatch"

var pass_count := 0
var fail_count := 0
var skip_count := 0

var _playfab_extension = null
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


func assert_result_error(result, expected_code: String, name: String) -> void:
	assert_not_null(result, "%s returns PlayFabResult" % name)
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


func get_playfab():
	if Engine.has_singleton("PlayFab"):
		return Engine.get_singleton("PlayFab")

	if _playfab_extension == null and FileAccess.file_exists(PLAYFAB_EXTENSION_PATH):
		_playfab_extension = load(PLAYFAB_EXTENSION_PATH)

	if Engine.has_singleton("PlayFab"):
		return Engine.get_singleton("PlayFab")

	return null


func get_gdk():
	if Engine.has_singleton("GDK"):
		return Engine.get_singleton("GDK")

	if _gdk_extension == null and FileAccess.file_exists(GDK_EXTENSION_PATH):
		_gdk_extension = load(GDK_EXTENSION_PATH)

	if Engine.has_singleton("GDK"):
		return Engine.get_singleton("GDK")

	return null


func reset_playfab_runtime() -> void:
	var playfab = get_playfab()
	if playfab != null:
		playfab.shutdown()


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
	var playfab = get_playfab()
	var gdk = get_gdk()
	var main_loop = Engine.get_main_loop()
	var started_msec := Time.get_ticks_msec()
	while not bool(state.get("completed", false)):
		if playfab != null:
			playfab.dispatch()
		if gdk != null:
			gdk.dispatch()
		if Time.get_ticks_msec() - started_msec >= timeout_msec:
			return null
		if main_loop != null and main_loop.has_signal("process_frame"):
			await main_loop.process_frame
		else:
			OS.delay_msec(ASYNC_POLL_INTERVAL_MSEC)

	if playfab != null:
		playfab.dispatch()
	if gdk != null:
		gdk.dispatch()

	return state.get("result")


func wait_for_signal(async_signal, timeout_msec: int = DEFAULT_ASYNC_TIMEOUT_MSEC):
	if typeof(async_signal) != TYPE_SIGNAL:
		return null
	return await wait_for_tracked_signal(track_signal(async_signal), timeout_msec)


func ensure_gdk_primary_user(timeout_msec: int = DEFAULT_ASYNC_TIMEOUT_MSEC) -> Dictionary:
	var outcome := {
		"user": null,
		"result": null,
		"signal": null,
		"op": null,
		"skip_reason": "",
	}

	var gdk = get_gdk()
	if gdk == null:
		outcome["skip_reason"] = "GDK singleton is not available."
		return outcome

	if not gdk.is_initialized():
		var init_result = gdk.initialize()
		outcome["result"] = init_result
		if init_result == null or not init_result.ok:
			outcome["skip_reason"] = init_result.message if init_result != null else "GDK.initialize() failed."
			return outcome

	var user = gdk.users.get_primary_user()
	if user != null and user.signed_in:
		outcome["user"] = user
		return outcome

	var completion_signal = gdk.users.add_default_user_async()
	outcome["signal"] = completion_signal
	outcome["op"] = completion_signal
	if typeof(completion_signal) != TYPE_SIGNAL:
		outcome["skip_reason"] = "GDK.users.add_default_user_async() did not start."
		return outcome

	var result = await wait_for_signal(completion_signal, timeout_msec)
	outcome["result"] = result
	if result == null:
		outcome["skip_reason"] = "Timed out waiting for the GDK default-user flow."
		return outcome
	if not result.ok:
		outcome["skip_reason"] = result.message
		return outcome

	user = result.data if result.data != null else gdk.users.get_primary_user()
	if user == null or not user.signed_in:
		outcome["skip_reason"] = "GDK default-user flow did not return a signed-in user."
		return outcome

	outcome["user"] = user
	return outcome
