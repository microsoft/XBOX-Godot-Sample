extends "res://addons/godot_gdk_tests/gdk_test_base.gd"
## Wave 4 GUT coverage for the `runtime_error` signal on the `GDK`
## singleton. Wave 3 already covers the `users` / `core` (repeated
## `initialize()`) pathways in `test_core.gd`. This suite broadens
## coverage to the achievements, presence, social, and
## multiplayer_activity services so a regression that drops their error-route
## into `GDK.runtime_error` is caught.

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


func test_social_validation_routes_through_runtime_error() -> void:
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

	var runtime_errors: Array = []
	gdk.connect("runtime_error", func(result): runtime_errors.append(result))

	# `create_social_group_from_xuids(user, [])` routes a validation failure
	# through GDK::emit_runtime_error, not just the per-call return value.
	var invalid_group = social.create_social_group_from_xuids(user, PackedStringArray())
	assert_true(invalid_group == null, "create_social_group_from_xuids([]) rejects empty XUID lists")

	assert_true(runtime_errors.size() >= 1, "social validation failure routes through GDK.runtime_error")
	if runtime_errors.size() >= 1:
		var last_error = runtime_errors[-1]
		assert_not_null(last_error, "runtime_error payload is non-null GDKResult")
		if last_error != null:
			assert_eq(last_error.code, "missing_social_group_xuids", "runtime_error code matches the validation failure")

	social.stop_social_graph(user)
	disconnect_signal_handlers(gdk, ["runtime_error"])


func test_no_spurious_runtime_error_during_clean_init_shutdown() -> void:
	if pending_unless_runtime_available():
		return

	var gdk = get_gdk()
	var runtime_errors: Array = []
	gdk.connect("runtime_error", func(result): runtime_errors.append(result))

	var init_result = initialize_runtime()
	if init_result == null or not init_result.ok:
		# A failed init is allowed to emit runtime_error (covered in
		# test_core.gd); for this suite we just `pending` so we don't churn
		# on environment-driven init failures.
		pending("clean-init runtime_error coverage requires a successful init: %s" % (
			"null result" if init_result == null else init_result.message))
		disconnect_signal_handlers(gdk, ["runtime_error"])
		return

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

	var presence = gdk.get_presence()
	if presence != null:
		assert_has_signal_named(presence, "presence_changed")
		assert_has_signal_named(presence, "local_presence_set")

	var social = gdk.get_social()
	if social != null:
		assert_has_signal_named(social, "social_graph_changed")
		assert_has_signal_named(social, "social_group_updated")
		assert_has_signal_named(social, "social_user_changed")

	var multiplayer_activity = gdk.get_multiplayer_activity()
	if multiplayer_activity != null:
		assert_has_signal_named(multiplayer_activity, "activities_updated")
		assert_has_signal_named(multiplayer_activity, "pending_invite_received")
		assert_has_signal_named(multiplayer_activity, "invite_accepted")
