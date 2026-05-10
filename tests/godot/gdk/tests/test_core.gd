extends "res://addons/godot_gdk_tests/gdk_test_base.gd"
## Wave 3 GUT migration of `suites/core_suite.gd`. Behavior parity:
## same per-call assertion count as the pre-GUT harness; `log_skip` mapped to
## `pending(...)`; one-off `log_pass` direct calls preserved as
## `assert_true(true, ...)` so GUT's `Asserts:` count tracks the pre-GUT total.

const INITIALIZE_ON_STARTUP_SETTING := "gdk/runtime/initialize_on_startup"
const AUTO_ADD_PRIMARY_USER_SETTING := "gdk/runtime/auto_add_primary_user"
const GDK_BOOTSTRAP_SCRIPT_PATH := "res://addons/godot_gdk/runtime/gdk_bootstrap.gd"


func before_each() -> void:
	reset_runtime()


func after_each() -> void:
	reset_runtime()


func test_singleton_availability() -> void:
	var gdk = get_gdk()
	assert_not_null(gdk, "Engine.get_singleton('GDK')")
	assert_true(not Engine.has_singleton("GDKUser"), "deprecated GDKUser singleton removed")


func test_class_registration() -> void:
	for registered_class in [
		"GDK",
		"GDKUsers",
		"GDKUser",
		"GDKAccessibility",
		"GDKClosedCaptionProperties",
		"GDKAchievements",
		"GDKAchievement",
		"GDKPackage",
		"GDKPackageMount",
		"GDKPackageResourcePack",
		"GDKStats",
		"GDKLeaderboards",
		"GDKLeaderboard",
		"GDKLeaderboardColumn",
		"GDKLeaderboardRow",
		"GDKPrivacy",
		"GDKPresence",
		"GDKPresenceRecord",
		"GDKSocial",
		"GDKSocialFilter",
		"GDKSocialGroup",
		"GDKSocialUser",
		"GDKStore",
		"GDKStoreLicenseStatus",
		"GDKProfile",
		"GDKUserProfile",
		"GDKStringVerify",
		"GDKTitleStorage",
		"GDKTitleStorageBlobMetadata",
		"GDKTitleStorageBlobMetadataResult",
		"GDKErrorReporting",
		"GDKSystem",
		"GDKLauncher",
		"GDKCapture",
		"GDKCaptureMetaData",
		"GDKResult",
	]:
		assert_true(ClassDB.class_exists(registered_class), "%s registered in ClassDB" % registered_class)

	assert_true(ClassDB.is_parent_class("GDK", "Object"), "GDK extends Object")
	assert_true(ClassDB.is_parent_class("GDKUsers", "RefCounted"), "GDKUsers extends RefCounted")
	assert_true(ClassDB.is_parent_class("GDKUser", "RefCounted"), "GDKUser extends RefCounted")
	assert_true(ClassDB.is_parent_class("GDKAccessibility", "RefCounted"), "GDKAccessibility extends RefCounted")
	assert_true(ClassDB.is_parent_class("GDKClosedCaptionProperties", "RefCounted"), "GDKClosedCaptionProperties extends RefCounted")
	assert_true(ClassDB.is_parent_class("GDKAchievements", "RefCounted"), "GDKAchievements extends RefCounted")
	assert_true(ClassDB.is_parent_class("GDKAchievement", "RefCounted"), "GDKAchievement extends RefCounted")
	assert_true(ClassDB.is_parent_class("GDKPackage", "RefCounted"), "GDKPackage extends RefCounted")
	assert_true(ClassDB.is_parent_class("GDKPackageMount", "RefCounted"), "GDKPackageMount extends RefCounted")
	assert_true(ClassDB.is_parent_class("GDKPackageResourcePack", "RefCounted"), "GDKPackageResourcePack extends RefCounted")
	assert_true(ClassDB.is_parent_class("GDKStats", "RefCounted"), "GDKStats extends RefCounted")
	assert_true(ClassDB.is_parent_class("GDKLeaderboards", "RefCounted"), "GDKLeaderboards extends RefCounted")
	assert_true(ClassDB.is_parent_class("GDKLeaderboard", "RefCounted"), "GDKLeaderboard extends RefCounted")
	assert_true(ClassDB.is_parent_class("GDKLeaderboardColumn", "RefCounted"), "GDKLeaderboardColumn extends RefCounted")
	assert_true(ClassDB.is_parent_class("GDKLeaderboardRow", "RefCounted"), "GDKLeaderboardRow extends RefCounted")
	assert_true(ClassDB.is_parent_class("GDKPrivacy", "RefCounted"), "GDKPrivacy extends RefCounted")
	assert_true(ClassDB.is_parent_class("GDKPresence", "RefCounted"), "GDKPresence extends RefCounted")
	assert_true(ClassDB.is_parent_class("GDKPresenceRecord", "RefCounted"), "GDKPresenceRecord extends RefCounted")
	assert_true(ClassDB.is_parent_class("GDKSocial", "RefCounted"), "GDKSocial extends RefCounted")
	assert_true(ClassDB.is_parent_class("GDKSocialFilter", "RefCounted"), "GDKSocialFilter extends RefCounted")
	assert_true(ClassDB.is_parent_class("GDKSocialGroup", "RefCounted"), "GDKSocialGroup extends RefCounted")
	assert_true(ClassDB.is_parent_class("GDKSocialUser", "RefCounted"), "GDKSocialUser extends RefCounted")
	assert_true(ClassDB.is_parent_class("GDKStore", "RefCounted"), "GDKStore extends RefCounted")
	assert_true(ClassDB.is_parent_class("GDKStoreLicenseStatus", "RefCounted"), "GDKStoreLicenseStatus extends RefCounted")
	assert_true(ClassDB.is_parent_class("GDKProfile", "RefCounted"), "GDKProfile extends RefCounted")
	assert_true(ClassDB.is_parent_class("GDKUserProfile", "RefCounted"), "GDKUserProfile extends RefCounted")
	assert_true(ClassDB.is_parent_class("GDKStringVerify", "RefCounted"), "GDKStringVerify extends RefCounted")
	assert_true(ClassDB.is_parent_class("GDKTitleStorage", "RefCounted"), "GDKTitleStorage extends RefCounted")
	assert_true(ClassDB.is_parent_class("GDKTitleStorageBlobMetadata", "RefCounted"), "GDKTitleStorageBlobMetadata extends RefCounted")
	assert_true(ClassDB.is_parent_class("GDKTitleStorageBlobMetadataResult", "RefCounted"), "GDKTitleStorageBlobMetadataResult extends RefCounted")
	assert_true(ClassDB.is_parent_class("GDKErrorReporting", "RefCounted"), "GDKErrorReporting extends RefCounted")
	assert_true(ClassDB.is_parent_class("GDKSystem", "RefCounted"), "GDKSystem extends RefCounted")
	assert_true(ClassDB.is_parent_class("GDKLauncher", "RefCounted"), "GDKLauncher extends RefCounted")
	assert_true(ClassDB.is_parent_class("GDKCapture", "RefCounted"), "GDKCapture extends RefCounted")
	assert_true(ClassDB.is_parent_class("GDKCaptureMetaData", "RefCounted"), "GDKCaptureMetaData extends RefCounted")
	assert_true(ClassDB.is_parent_class("GDKResult", "RefCounted"), "GDKResult extends RefCounted")


func test_gdk_root_api() -> void:
	if pending_unless_runtime_available():
		return

	var gdk = get_gdk()

	for method_name in ["initialize", "shutdown", "is_available", "is_initialized", "dispatch", "get_last_error", "get_users", "get_game_ui", "get_accessibility", "get_achievements", "get_package", "get_stats", "get_leaderboards", "get_privacy", "get_presence", "get_social", "get_store", "get_profile", "get_string_verify", "get_title_storage", "get_error_reporting", "get_launcher", "get_multiplayer_activity", "get_capture", "get_system"]:
		assert_has_method_named(gdk, method_name)

	for signal_name in ["initialized", "shutdown_completed", "runtime_error"]:
		assert_has_signal_named(gdk, signal_name)

	assert_true(gdk.get_users() != null, "GDK.users service available")
	assert_true(gdk.get_accessibility() != null, "GDK.accessibility service available")
	assert_true(gdk.get_achievements() != null, "GDK.achievements service available")
	assert_true(gdk.get_package() != null, "GDK.package service available")
	assert_true(gdk.get_stats() != null, "GDK.stats service available")
	assert_true(gdk.get_leaderboards() != null, "GDK.leaderboards service available")
	assert_true(gdk.get_privacy() != null, "GDK.privacy service available")
	assert_true(gdk.get_presence() != null, "GDK.presence service available")
	assert_true(gdk.get_social() != null, "GDK.social service available")
	assert_true(gdk.get_store() != null, "GDK.store service available")
	assert_true(gdk.get_profile() != null, "GDK.profile service available")
	assert_true(gdk.get_string_verify() != null, "GDK.string_verify service available")
	assert_true(gdk.get_title_storage() != null, "GDK.title_storage service available")
	assert_true(gdk.get_error_reporting() != null, "GDK.error_reporting service available")
	assert_true(gdk.get_launcher() != null, "GDK.launcher service available")
	assert_true(gdk.get_multiplayer_activity() != null, "GDK.multiplayer_activity service available")
	assert_true(gdk.get_capture() != null, "GDK.capture service available")
	assert_true(gdk.get_system() != null, "GDK.system service available")
	assert_true(gdk.is_available() is bool, "is_available() returns bool")
	assert_eq(gdk.is_initialized(), false, "is_initialized() starts false")
	assert_eq(gdk.dispatch(), 0, "dispatch() safe before init")
	assert_true(ProjectSettings.has_setting(INITIALIZE_ON_STARTUP_SETTING), "gdk/runtime/initialize_on_startup project setting registered")
	assert_eq(bool(get_setting_default(INITIALIZE_ON_STARTUP_SETTING)), false, "gdk/runtime/initialize_on_startup default remains false")
	assert_eq(bool(ProjectSettings.get_setting(INITIALIZE_ON_STARTUP_SETTING, false)), true, "GDK test host sets gdk/runtime/initialize_on_startup true")
	assert_true(ProjectSettings.has_setting(EMBED_DISPATCH_SETTING), "gdk/runtime/embed_dispatch project setting registered")
	assert_eq(bool(ProjectSettings.get_setting(EMBED_DISPATCH_SETTING, false)), true, "gdk/runtime/embed_dispatch defaults to true")
	assert_true(ProjectSettings.has_setting(AUTO_ADD_PRIMARY_USER_SETTING), "gdk/runtime/auto_add_primary_user project setting registered")
	assert_eq(bool(get_setting_default(AUTO_ADD_PRIMARY_USER_SETTING)), false, "gdk/runtime/auto_add_primary_user default remains false")
	assert_eq(bool(ProjectSettings.get_setting(AUTO_ADD_PRIMARY_USER_SETTING, false)), true, "GDK test host sets gdk/runtime/auto_add_primary_user true")

	var initialized_events: Array = []
	var shutdown_events: Array = []
	var runtime_errors: Array = []
	gdk.connect("initialized", func(): initialized_events.append(true))
	gdk.connect("shutdown_completed", func(): shutdown_events.append(true))
	gdk.connect("runtime_error", func(result): runtime_errors.append(result))

	var last_error = gdk.get_last_error()
	assert_not_null(last_error, "get_last_error() returns GDKResult")
	if last_error != null:
		assert_true(last_error.has_method("is_ok"), "last error exposes result API")

	var init_result = initialize_runtime()
	assert_not_null(init_result, "initialize() returns GDKResult")
	if init_result == null:
		disconnect_signal_handlers(gdk, ["initialized", "shutdown_completed", "runtime_error"])
		return

	if init_result.ok:
		assert_eq(initialized_events.size(), 1, "initialized signal emitted once")
		assert_eq(gdk.is_initialized(), true, "is_initialized() true after init")
		assert_true(gdk.dispatch() is int, "dispatch() returns int after init")

		var runtime_error_count_before_repeat = runtime_errors.size()
		var repeat_init_result = gdk.initialize()
		assert_not_null(repeat_init_result, "second initialize() returns GDKResult")
		if repeat_init_result != null:
			assert_eq(repeat_init_result.ok, false, "second initialize() fails while already initialized")
			assert_eq(repeat_init_result.code, "already_initialized", "second initialize() reports already_initialized")

			var repeat_last_error = gdk.get_last_error()
			assert_not_null(repeat_last_error, "get_last_error() tracks repeated initialize failure")
			if repeat_last_error != null:
				assert_eq(repeat_last_error.code, repeat_init_result.code, "last error matches repeated initialize() failure")

			assert_eq(runtime_errors.size(), runtime_error_count_before_repeat + 1, "runtime_error emitted for repeated initialize()")

		gdk.shutdown()
		assert_eq(shutdown_events.size(), 1, "shutdown_completed signal emitted once")
		assert_eq(gdk.is_initialized(), false, "is_initialized() false after shutdown")
	else:
		assert_true(runtime_errors.size() >= 1, "runtime_error emitted for initialize() failure")
		pending("GDK.initialize(): %s" % init_result.message)
		assert_eq(gdk.is_initialized(), false, "is_initialized() remains false after failed init")

	disconnect_signal_handlers(gdk, ["initialized", "shutdown_completed", "runtime_error"])


func test_bootstrap_routes_user_signals_through_combined_handler() -> void:
	var src := FileAccess.get_file_as_string(GDK_BOOTSTRAP_SCRIPT_PATH)
	assert_true(src.length() > 0, "GDKBootstrap source is mirrored into the GDK test host")
	if src.is_empty():
		return

	assert_string_contains(src, "func _on_gdk_user_changed", "GDKBootstrap defines one GDK user-event handler")
	assert_string_contains(src, 'Callable(self, "_on_gdk_user_changed")', "GDKBootstrap binds the user_changed handler")
	assert_string_contains(src, "gdk.users.user_changed.connect", "GDKBootstrap connects only the public GDKUsers user event")
	for signal_name in ["user_added", "user_removed", "primary_user_changed"]:
		assert_false(
				src.contains("gdk.users.%s" % signal_name),
				"GDKBootstrap does not reference removed GDKUsers.%s signal" % signal_name)

	for legacy_handler_name in ["_on_user_added", "_on_user_removed", "_on_user_changed", "_on_primary_user_changed"]:
		assert_false(
				src.contains("func %s" % legacy_handler_name),
				"GDKBootstrap no longer keeps separate %s handlers" % legacy_handler_name)


func test_embed_dispatch_behavior() -> void:
	if pending_unless_runtime_available():
		return

	var gdk = get_gdk()
	var original_embed_dispatch: bool = get_embed_dispatch_enabled()

	set_embed_dispatch_enabled(true)
	var auto_init_result = initialize_runtime()
	assert_not_null(auto_init_result, "initialize() returns GDKResult for auto-dispatch coverage")
	if auto_init_result == null:
		set_embed_dispatch_enabled(original_embed_dispatch)
		return
	if not auto_init_result.ok:
		pending("Auto-dispatch behavior: %s" % auto_init_result.message)
		set_embed_dispatch_enabled(original_embed_dispatch)
		return

	var auto_signal = gdk.users.add_default_user_async()
	assert_true(typeof(auto_signal) == TYPE_SIGNAL, "add_default_user_async() returns Signal for auto-dispatch coverage")
	if typeof(auto_signal) != TYPE_SIGNAL:
		reset_runtime()
		set_embed_dispatch_enabled(original_embed_dispatch)
		return

	var auto_state = track_signal(auto_signal)
	if auto_state["completed"]:
		pending("Auto-dispatch behavior: The default-user op completed synchronously before frame-based coverage could run.")
	else:
		var auto_result = await await_completion_state_no_dispatch(auto_state, 8000)
		if auto_result == null:
			pending("Auto-dispatch behavior: Timed out waiting for add_default_user_async() without manual GDK.dispatch().")
		else:
			assert_true(true, "Auto-dispatch behavior — completed without manual GDK.dispatch()")

	reset_runtime()

	set_embed_dispatch_enabled(false)
	var manual_init_result = initialize_runtime()
	assert_not_null(manual_init_result, "initialize() returns GDKResult for manual-dispatch coverage")
	if manual_init_result == null:
		set_embed_dispatch_enabled(original_embed_dispatch)
		return
	if not manual_init_result.ok:
		pending("Manual-dispatch fallback: %s" % manual_init_result.message)
		set_embed_dispatch_enabled(original_embed_dispatch)
		return

	var manual_signal = gdk.users.add_default_user_async()
	assert_true(typeof(manual_signal) == TYPE_SIGNAL, "add_default_user_async() returns Signal when embed_dispatch is disabled")
	if typeof(manual_signal) != TYPE_SIGNAL:
		reset_runtime()
		set_embed_dispatch_enabled(original_embed_dispatch)
		return

	var manual_state = track_signal(manual_signal)
	if manual_state["completed"]:
		pending("Manual-dispatch fallback: The default-user op completed synchronously before disabled-mode coverage could run.")
	else:
		var advanced_frames = await advance_process_frames(5)
		if not advanced_frames:
			pending("Manual-dispatch fallback: The headless runner could not access process_frame for disabled-mode coverage.")
		else:
			assert_eq(manual_state["completed"], false, "embed_dispatch disabled keeps async completion pending")

			var manual_result = await await_completion_state(manual_state, 8000)
			if manual_result == null:
				pending("Manual-dispatch fallback completion: Timed out waiting for completion after manual GDK.dispatch().")
			else:
				assert_true(true, "Manual-dispatch fallback completion — completed after manual GDK.dispatch()")

	set_embed_dispatch_enabled(original_embed_dispatch)
