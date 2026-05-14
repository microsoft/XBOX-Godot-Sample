extends "res://addons/godot_gdk_tests/gdk_test_base.gd"
## Wave 4 GUT coverage for the `runtime_error` signals across the GDK addon.
##
## After the result-only error-handling refactor:
##   * Root `GDK.runtime_error(result)` is reserved for `XError` callback
##     events sourced from `GDKErrorReporting`.
##   * Per-service `runtime_error(result)` signals on `GDKSocial` and
##     `GDKAchievements` carry unsolicited subsystem failures (e.g. failures
##     inside the Social Manager or Achievements Manager work pumps, and
##     async social-group factory failures).
##   * Caller-driven failures are returned directly via the per-call
##     `GDKResult` or async completion `Signal`, not via `runtime_error`.

func before_each() -> void:
	reset_runtime()


func after_each() -> void:
	reset_runtime()


func _signal_arg_count(obj: Object, signal_name: String) -> int:
	for s in obj.get_signal_list():
		if s.get("name", "") == signal_name:
			return s.get("args", []).size()
	return -1


func test_root_runtime_error_signal_registered() -> void:
	if pending_unless_runtime_available():
		return

	var gdk = get_gdk()
	assert_has_signal_named(gdk, "runtime_error")
	assert_eq(_signal_arg_count(gdk, "runtime_error"), 1, "runtime_error has 1 arg (result)")


func test_per_service_runtime_error_signals_registered() -> void:
	if pending_unless_runtime_available():
		return

	var gdk = get_gdk()

	var social = gdk.get_social()
	assert_not_null(social, "GDK.social returns service object")
	if social != null:
		assert_has_signal_named(social, "runtime_error")
		assert_eq(_signal_arg_count(social, "runtime_error"), 1, "GDK.social.runtime_error has 1 arg (result)")

	var achievements = gdk.get_achievements()
	assert_not_null(achievements, "GDK.achievements returns service object")
	if achievements != null:
		assert_has_signal_named(achievements, "runtime_error")
		assert_eq(_signal_arg_count(achievements, "runtime_error"), 1, "GDK.achievements.runtime_error has 1 arg (result)")


func test_social_validation_routes_through_per_service_runtime_error() -> void:
	if pending_unless_runtime_available():
		return

	var gdk = get_gdk()
	var social = gdk.get_social()
	assert_not_null(social, "GDK.social returns service object")
	if social == null:
		return

	var init_result = initialize_runtime()
	if init_result == null or not init_result.ok:
		pending("social runtime_error coverage requires a working runtime: %s" % (
			"init returned null" if init_result == null else init_result.message))
		return

	var sign_in = await ensure_primary_user()
	var user = sign_in["user"]
	if user == null:
		pending("social runtime_error coverage requires a signed-in user to start the social graph.")
		return

	var graph_started = social.start_social_graph(user)
	if graph_started == null or not graph_started.ok:
		pending("Social graph could not start for runtime_error coverage: %s" % (
			"null result" if graph_started == null else graph_started.message))
		return

	var root_runtime_errors: Array = []
	var social_runtime_errors: Array = []
	gdk.connect("runtime_error", func(result): root_runtime_errors.append(result))
	social.connect("runtime_error", func(result): social_runtime_errors.append(result))

	# `create_social_group_from_xuids(user, [])` is the canonical caller-driven
	# validation failure surface. After the result-only refactor it returns a
	# GDKResult whose `.ok` is false AND emits the per-service runtime_error.
	# It must NOT emit the root XError-only `GDK.runtime_error`.
	var invalid_result = social.create_social_group_from_xuids(user, PackedStringArray())
	assert_not_null(invalid_result, "create_social_group_from_xuids() returns GDKResult")
	if invalid_result != null:
		assert_eq(invalid_result.ok, false, "empty XUID list returns failed GDKResult")
		assert_eq(invalid_result.code, "missing_social_group_xuids", "validation result code matches")

	assert_true(social_runtime_errors.size() >= 1, "social validation failure routes through GDK.social.runtime_error")
	if social_runtime_errors.size() >= 1:
		var social_err = social_runtime_errors[-1]
		assert_not_null(social_err, "GDK.social.runtime_error payload is non-null GDKResult")
		if social_err != null:
			assert_eq(social_err.code, "missing_social_group_xuids", "GDK.social.runtime_error code matches the validation failure")

	assert_eq(root_runtime_errors.size(), 0, "social validation failure does NOT emit root GDK.runtime_error (XError-only)")

	social.stop_social_graph(user)
	disconnect_signal_handlers(gdk, ["runtime_error"])
	disconnect_signal_handlers(social, ["runtime_error"])


func test_no_spurious_runtime_error_during_clean_init_shutdown() -> void:
	if pending_unless_runtime_available():
		return

	var gdk = get_gdk()
	var runtime_errors: Array = []
	gdk.connect("runtime_error", func(result): runtime_errors.append(result))

	var init_result = initialize_runtime()
	if init_result == null or not init_result.ok:
		pending("clean-init runtime_error coverage requires a successful init: %s" % (
			"null result" if init_result == null else init_result.message))
		disconnect_signal_handlers(gdk, ["runtime_error"])
		return

	# After the result-only refactor, init failures are not surfaced via
	# runtime_error (the result is returned to the caller). Clean init+shutdown
	# must still leave the signal silent.
	var pre_shutdown_errors = runtime_errors.size()
	gdk.shutdown()
	assert_eq(runtime_errors.size(), pre_shutdown_errors, "shutdown does not emit spurious runtime_error")

	disconnect_signal_handlers(gdk, ["runtime_error"])


func test_per_service_signal_surfaces_present() -> void:
	if pending_unless_runtime_available():
		return

	var gdk = get_gdk()

	var achievements = gdk.get_achievements()
	if achievements != null:
		assert_has_signal_named(achievements, "achievement_unlocked")
		assert_has_signal_named(achievements, "achievements_updated")
		assert_has_signal_named(achievements, "runtime_error")

	var presence = gdk.get_presence()
	if presence != null:
		assert_has_signal_named(presence, "presence_changed")
		assert_has_signal_named(presence, "local_presence_set")

	var social = gdk.get_social()
	if social != null:
		assert_has_signal_named(social, "social_graph_changed")
		assert_has_signal_named(social, "social_group_updated")
		assert_has_signal_named(social, "social_user_changed")
		assert_has_signal_named(social, "runtime_error")

	var multiplayer_activity = gdk.get_multiplayer_activity()
	if multiplayer_activity != null:
		assert_has_signal_named(multiplayer_activity, "activities_updated")
		assert_has_signal_named(multiplayer_activity, "pending_invite_received")
		assert_has_signal_named(multiplayer_activity, "invite_accepted")
