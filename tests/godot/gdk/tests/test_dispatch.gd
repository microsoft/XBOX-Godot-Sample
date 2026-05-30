extends "res://addons/godot_gdk_tests/gdk_test_base.gd"

const DISPATCH_COMPLETION_COUNT := 2

var _original_embed_dispatch := true


func before_each() -> void:
	_original_embed_dispatch = get_embed_dispatch_enabled()
	set_embed_dispatch_enabled(false)
	reset_runtime()


func after_each() -> void:
	reset_runtime()
	set_embed_dispatch_enabled(_original_embed_dispatch)


func test_dispatch_returns_number_of_drained_completion_events() -> void:
	if pending_unless_runtime_available():
		return

	var gdk = get_gdk()
	var init_result = initialize_runtime()
	assert_not_null(init_result, "GDK.initialize() returns GDKResult for dispatch count coverage")
	if init_result == null:
		return
	if not init_result.ok:
		pending("Dispatch count coverage: %s" % init_result.message)
		return

	var users = gdk.get_users()
	assert_not_null(users, "GDK.users is available for queueing completion events")
	if users == null:
		return

	var sign_in = await ensure_primary_user()
	var sign_in_signal = sign_in["signal"]
	var sign_in_result = sign_in["result"]
	var user = sign_in["user"]
	if typeof(sign_in_signal) == TYPE_SIGNAL and sign_in_result == null:
		assert_true(false, "Default-user flow for dispatch count coverage completes")
		return
	if user == null:
		if sign_in_result != null and not sign_in_result.ok:
			assert_true(sign_in_result.code.length() > 0, "failed dispatch sign-in exposes an error code")
			assert_true(sign_in_result.message.length() > 0, "failed dispatch sign-in exposes an error message")
			pending("Dispatch count coverage: %s" % sign_in_result.message)
		else:
			pending("Dispatch count coverage: No signed-in user is available on this machine.")
		return

	var states: Array = []
	for _index in range(DISPATCH_COMPLETION_COUNT):
		var picture_signal = users.get_gamer_picture_async(user, "small")
		assert_eq(typeof(picture_signal), TYPE_SIGNAL, "get_gamer_picture_async() queues a completion Signal")
		if typeof(picture_signal) == TYPE_SIGNAL:
			states.append(track_signal(picture_signal))

	if states.size() != DISPATCH_COMPLETION_COUNT:
		return

	var drained := 0
	var max_wait_ms := 5000
	var poll_interval_ms := 100
	var elapsed := 0
	while drained < DISPATCH_COMPLETION_COUNT and elapsed < max_wait_ms:
		OS.delay_msec(poll_interval_ms)
		drained += gdk.dispatch()
		elapsed += poll_interval_ms

	assert_eq(drained, DISPATCH_COMPLETION_COUNT, "dispatch() returns the exact number of queued completion events it drained")
	for state in states:
		assert_eq(state["completed"], true, "drained completion event resolves its tracked Signal")
