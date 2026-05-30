extends "res://addons/godot_gdk_tests/gdk_test_base.gd"


func before_each() -> void:
	reset_runtime()


func after_each() -> void:
	reset_runtime()


func test_initialize_shutdown_reinitialize_rearms_runtime_and_signals() -> void:
	if pending_unless_runtime_available():
		return

	var gdk = get_gdk()
	var initialized_events: Array = []
	var shutdown_events: Array = []
	var init_handler = func(): initialized_events.append(Time.get_ticks_msec())
	var shutdown_handler = func(): shutdown_events.append(Time.get_ticks_msec())
	gdk.initialized.connect(init_handler)
	gdk.shutdown_completed.connect(shutdown_handler)

	var users = gdk.get_users()
	assert_not_null(users, "GDK.users remains available during lifecycle cycling")
	var user_changed_events: Array = []
	var user_changed_handler = func(_user, change_kind): user_changed_events.append(change_kind)
	if users != null:
		users.user_changed.connect(user_changed_handler)

	var first_init = gdk.initialize()
	assert_not_null(first_init, "first initialize() returns GDKResult")
	if first_init == null:
		_cleanup_handlers(gdk, init_handler, shutdown_handler, users, user_changed_handler)
		return
	if not first_init.ok:
		pending("First GDK.initialize() failed: %s" % first_init.message)
		_cleanup_handlers(gdk, init_handler, shutdown_handler, users, user_changed_handler)
		return

	assert_eq(initialized_events.size(), 1, "initialized emitted for first initialize()")
	assert_eq(gdk.is_initialized(), true, "runtime reports initialized after first initialize()")
	if users != null:
		assert_true(users.user_changed.is_connected(user_changed_handler), "user_changed handler remains connected after first initialize()")

	gdk.shutdown()
	assert_eq(shutdown_events.size(), 1, "shutdown_completed emitted for first shutdown()")
	assert_eq(gdk.is_initialized(), false, "runtime reports uninitialized after first shutdown()")

	var second_init = gdk.initialize()
	assert_result_ok(second_init, "second initialize() after shutdown")
	if second_init == null or not second_init.ok:
		_cleanup_handlers(gdk, init_handler, shutdown_handler, users, user_changed_handler)
		return

	assert_eq(initialized_events.size(), 2, "initialized emitted again after reinitialize()")
	assert_eq(gdk.is_initialized(), true, "runtime reports initialized after reinitialize()")
	if users != null:
		assert_true(users.user_changed.is_connected(user_changed_handler), "user_changed handler remains connected after reinitialize()")
	assert_true(gdk.dispatch() is int, "dispatch() remains callable after reinitialize()")

	gdk.shutdown()
	assert_eq(shutdown_events.size(), 2, "shutdown_completed emitted again after final shutdown()")
	assert_eq(gdk.is_initialized(), false, "runtime reports uninitialized after final shutdown()")
	_cleanup_handlers(gdk, init_handler, shutdown_handler, users, user_changed_handler)


func _cleanup_handlers(gdk: Object, init_handler: Callable, shutdown_handler: Callable, users: Variant, user_changed_handler: Callable) -> void:
	if gdk != null:
		if gdk.initialized.is_connected(init_handler):
			gdk.initialized.disconnect(init_handler)
		if gdk.shutdown_completed.is_connected(shutdown_handler):
			gdk.shutdown_completed.disconnect(shutdown_handler)
	if users != null and users.user_changed.is_connected(user_changed_handler):
		users.user_changed.disconnect(user_changed_handler)
