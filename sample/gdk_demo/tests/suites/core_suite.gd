extends RefCounted

func run(context) -> void:
	_test_singleton_availability(context)
	_test_class_registration(context)
	_test_gdk_root_api(context)

func _test_singleton_availability(context) -> void:
	context.log_section("Singleton Availability")

	var gdk = context.get_gdk()
	context.assert_not_null(gdk, "Engine.get_singleton('GDK')")
	context.assert_true(Engine.get_singleton("GDKUser") == null, "legacy GDKUser singleton removed")

func _test_class_registration(context) -> void:
	context.log_section("Class Registration")

	for registered_class in [
		"GDK",
		"GDKUsers",
		"GDKUser",
		"GDKAchievements",
		"GDKAchievement",
		"GDKPresence",
		"GDKPresenceRecord",
		"GDKSocial",
		"GDKSocialFilter",
		"GDKSocialGroup",
		"GDKSocialUser",
		"GDKAsyncOp",
		"GDKDispatchOp",
		"GDKResult"
	]:
		context.assert_true(ClassDB.class_exists(registered_class), "%s registered in ClassDB" % registered_class)

	context.assert_true(ClassDB.is_parent_class("GDK", "Object"), "GDK extends Object")
	context.assert_true(ClassDB.is_parent_class("GDKUsers", "RefCounted"), "GDKUsers extends RefCounted")
	context.assert_true(ClassDB.is_parent_class("GDKUser", "RefCounted"), "GDKUser extends RefCounted")
	context.assert_true(ClassDB.is_parent_class("GDKAchievements", "RefCounted"), "GDKAchievements extends RefCounted")
	context.assert_true(ClassDB.is_parent_class("GDKAchievement", "RefCounted"), "GDKAchievement extends RefCounted")
	context.assert_true(ClassDB.is_parent_class("GDKPresence", "RefCounted"), "GDKPresence extends RefCounted")
	context.assert_true(ClassDB.is_parent_class("GDKPresenceRecord", "RefCounted"), "GDKPresenceRecord extends RefCounted")
	context.assert_true(ClassDB.is_parent_class("GDKSocial", "RefCounted"), "GDKSocial extends RefCounted")
	context.assert_true(ClassDB.is_parent_class("GDKSocialFilter", "RefCounted"), "GDKSocialFilter extends RefCounted")
	context.assert_true(ClassDB.is_parent_class("GDKSocialGroup", "RefCounted"), "GDKSocialGroup extends RefCounted")
	context.assert_true(ClassDB.is_parent_class("GDKSocialUser", "RefCounted"), "GDKSocialUser extends RefCounted")
	context.assert_true(ClassDB.is_parent_class("GDKAsyncOp", "RefCounted"), "GDKAsyncOp extends RefCounted")
	context.assert_true(ClassDB.is_parent_class("GDKDispatchOp", "GDKAsyncOp"), "GDKDispatchOp extends GDKAsyncOp")
	context.assert_true(ClassDB.is_parent_class("GDKResult", "RefCounted"), "GDKResult extends RefCounted")

func _test_gdk_root_api(context) -> void:
	context.log_section("GDK Root API")

	var gdk = context.get_gdk()
	if gdk == null:
		context.log_fail("GDK root singleton missing, skipping root API group")
		return

	context.reset_runtime()

	for method_name in ["initialize", "shutdown", "is_available", "is_initialized", "dispatch", "get_last_error", "get_users", "get_achievements", "get_presence", "get_social"]:
		context.assert_has_method(gdk, method_name)

	for signal_name in ["initialized", "shutdown_completed", "runtime_error"]:
		context.assert_has_signal(gdk, signal_name)

	context.assert_true(gdk.get_users() != null, "GDK.users service available")
	context.assert_true(gdk.get_achievements() != null, "GDK.achievements service available")
	context.assert_true(gdk.get_presence() != null, "GDK.presence service available")
	context.assert_true(gdk.get_social() != null, "GDK.social service available")
	context.assert_true(gdk.is_available() is bool, "is_available() returns bool")
	context.assert_eq(gdk.is_initialized(), false, "is_initialized() starts false")
	context.assert_eq(gdk.dispatch(), 0, "dispatch() safe before init")

	var initialized_events: Array = []
	var shutdown_events: Array = []
	var runtime_errors: Array = []
	gdk.connect("initialized", func(): initialized_events.append(true))
	gdk.connect("shutdown_completed", func(): shutdown_events.append(true))
	gdk.connect("runtime_error", func(result): runtime_errors.append(result))

	var last_error = gdk.get_last_error()
	context.assert_not_null(last_error, "get_last_error() returns GDKResult")
	if last_error != null:
		context.assert_true(last_error.has_method("is_ok"), "last error exposes result API")

	var init_result = context.initialize_runtime()
	context.assert_not_null(init_result, "initialize() returns GDKResult")
	if init_result == null:
		context.disconnect_signal_handlers(gdk, ["initialized", "shutdown_completed", "runtime_error"])
		return

	if init_result.ok:
		context.assert_eq(initialized_events.size(), 1, "initialized signal emitted once")
		context.assert_eq(gdk.is_initialized(), true, "is_initialized() true after init")
		context.assert_true(gdk.dispatch() is int, "dispatch() returns int after init")

		var runtime_error_count_before_repeat = runtime_errors.size()
		var repeat_init_result = gdk.initialize()
		context.assert_not_null(repeat_init_result, "second initialize() returns GDKResult")
		if repeat_init_result != null:
			context.assert_eq(repeat_init_result.ok, false, "second initialize() fails while already initialized")
			context.assert_eq(repeat_init_result.code, "already_initialized", "second initialize() reports already_initialized")

			var repeat_last_error = gdk.get_last_error()
			context.assert_not_null(repeat_last_error, "get_last_error() tracks repeated initialize failure")
			if repeat_last_error != null:
				context.assert_eq(repeat_last_error.code, repeat_init_result.code, "last error matches repeated initialize() failure")

			context.assert_eq(runtime_errors.size(), runtime_error_count_before_repeat + 1, "runtime_error emitted for repeated initialize()")

		gdk.shutdown()
		context.assert_eq(shutdown_events.size(), 1, "shutdown_completed signal emitted once")
		context.assert_eq(gdk.is_initialized(), false, "is_initialized() false after shutdown")
	else:
		context.assert_true(runtime_errors.size() >= 1, "runtime_error emitted for initialize() failure")
		context.log_skip("GDK.initialize()", init_result.message)
		context.assert_eq(gdk.is_initialized(), false, "is_initialized() remains false after failed init")

	context.disconnect_signal_handlers(gdk, ["initialized", "shutdown_completed", "runtime_error"])
