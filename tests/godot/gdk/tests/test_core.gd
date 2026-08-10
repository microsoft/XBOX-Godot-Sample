extends "res://addons/godot_gdk_tests/gdk_test_base.gd"
## Wave 3 GUT migration of `suites/core_suite.gd`. Behavior parity:
## same per-call assertion count as the pre-GUT harness; `log_skip` mapped to
## `pending(...)`; one-off `log_pass` direct calls preserved as
## `assert_true(true, ...)` so GUT's `Asserts:` count tracks the pre-GUT total.

const INITIALIZE_ON_STARTUP_SETTING := "gdk/runtime/initialize_on_startup"
const AUTO_ADD_PRIMARY_USER_SETTING := "gdk/runtime/auto_add_primary_user"
const TESTS_LIVE_REQUIRED_SETTING := "gdk/tests/live_required"
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
		"Xbox",
		"XboxUsers",
		"XboxUser",
		"XboxAccessibility",
		"XboxClosedCaptionProperties",
		"XboxAchievements",
		"XboxAchievement",
		"XboxPackage",
		"XboxPackageMount",
		"XboxPackageResourcePack",
		"XboxStats",
		"XboxLeaderboards",
		"XboxLeaderboard",
		"XboxLeaderboardColumn",
		"XboxLeaderboardRow",
		"XboxPrivacy",
		"XboxPresence",
		"XboxPresenceRecord",
		"XboxSocial",
		"XboxSocialFilter",
		"XboxSocialGroup",
		"XboxSocialUser",
		"XboxStore",
		"XboxStoreLicenseStatus",
		"XboxProfile",
		"XboxUserProfile",
		"XboxStringVerify",
		"XboxTitleStorage",
		"XboxTitleStorageBlobMetadata",
		"XboxTitleStorageBlobMetadataResult",
		"XboxErrorReporting",
		"XboxSystem",
		"XboxLauncher",
		"XboxCapture",
		"XboxCaptureMetaData",
		"XboxDisplay",
		"XboxDisplayTimeoutDeferral",
		"XboxActivation",
		"XboxResult",
	]:
		assert_true(ClassDB.class_exists(registered_class), "%s registered in ClassDB" % registered_class)

	assert_true(ClassDB.is_parent_class("Xbox", "Object"), "Xbox extends Object")
	assert_true(ClassDB.is_parent_class("XboxUsers", "RefCounted"), "XboxUsers extends RefCounted")
	assert_true(ClassDB.is_parent_class("XboxUser", "RefCounted"), "XboxUser extends RefCounted")
	assert_true(ClassDB.is_parent_class("XboxAccessibility", "RefCounted"), "XboxAccessibility extends RefCounted")
	assert_true(ClassDB.is_parent_class("XboxClosedCaptionProperties", "RefCounted"), "XboxClosedCaptionProperties extends RefCounted")
	assert_true(ClassDB.is_parent_class("XboxAchievements", "RefCounted"), "XboxAchievements extends RefCounted")
	assert_true(ClassDB.is_parent_class("XboxAchievement", "RefCounted"), "XboxAchievement extends RefCounted")
	assert_true(ClassDB.is_parent_class("XboxPackage", "RefCounted"), "XboxPackage extends RefCounted")
	assert_true(ClassDB.is_parent_class("XboxPackageMount", "RefCounted"), "XboxPackageMount extends RefCounted")
	assert_true(ClassDB.is_parent_class("XboxPackageResourcePack", "RefCounted"), "XboxPackageResourcePack extends RefCounted")
	assert_true(ClassDB.is_parent_class("XboxStats", "RefCounted"), "XboxStats extends RefCounted")
	assert_true(ClassDB.is_parent_class("XboxLeaderboards", "RefCounted"), "XboxLeaderboards extends RefCounted")
	assert_true(ClassDB.is_parent_class("XboxLeaderboard", "RefCounted"), "XboxLeaderboard extends RefCounted")
	assert_true(ClassDB.is_parent_class("XboxLeaderboardColumn", "RefCounted"), "XboxLeaderboardColumn extends RefCounted")
	assert_true(ClassDB.is_parent_class("XboxLeaderboardRow", "RefCounted"), "XboxLeaderboardRow extends RefCounted")
	assert_true(ClassDB.is_parent_class("XboxPrivacy", "RefCounted"), "XboxPrivacy extends RefCounted")
	assert_true(ClassDB.is_parent_class("XboxPresence", "RefCounted"), "XboxPresence extends RefCounted")
	assert_true(ClassDB.is_parent_class("XboxPresenceRecord", "RefCounted"), "XboxPresenceRecord extends RefCounted")
	assert_true(ClassDB.is_parent_class("XboxSocial", "RefCounted"), "XboxSocial extends RefCounted")
	assert_true(ClassDB.is_parent_class("XboxSocialFilter", "RefCounted"), "XboxSocialFilter extends RefCounted")
	assert_true(ClassDB.is_parent_class("XboxSocialGroup", "RefCounted"), "XboxSocialGroup extends RefCounted")
	assert_true(ClassDB.is_parent_class("XboxSocialUser", "RefCounted"), "XboxSocialUser extends RefCounted")
	assert_true(ClassDB.is_parent_class("XboxStore", "RefCounted"), "XboxStore extends RefCounted")
	assert_true(ClassDB.is_parent_class("XboxStoreLicenseStatus", "RefCounted"), "XboxStoreLicenseStatus extends RefCounted")
	assert_true(ClassDB.is_parent_class("XboxProfile", "RefCounted"), "XboxProfile extends RefCounted")
	assert_true(ClassDB.is_parent_class("XboxUserProfile", "RefCounted"), "XboxUserProfile extends RefCounted")
	assert_true(ClassDB.is_parent_class("XboxStringVerify", "RefCounted"), "XboxStringVerify extends RefCounted")
	assert_true(ClassDB.is_parent_class("XboxTitleStorage", "RefCounted"), "XboxTitleStorage extends RefCounted")
	assert_true(ClassDB.is_parent_class("XboxTitleStorageBlobMetadata", "RefCounted"), "XboxTitleStorageBlobMetadata extends RefCounted")
	assert_true(ClassDB.is_parent_class("XboxTitleStorageBlobMetadataResult", "RefCounted"), "XboxTitleStorageBlobMetadataResult extends RefCounted")
	assert_true(ClassDB.is_parent_class("XboxErrorReporting", "RefCounted"), "XboxErrorReporting extends RefCounted")
	assert_true(ClassDB.is_parent_class("XboxSystem", "RefCounted"), "XboxSystem extends RefCounted")
	assert_true(ClassDB.is_parent_class("XboxLauncher", "RefCounted"), "XboxLauncher extends RefCounted")
	assert_true(ClassDB.is_parent_class("XboxCapture", "RefCounted"), "XboxCapture extends RefCounted")
	assert_true(ClassDB.is_parent_class("XboxCaptureMetaData", "RefCounted"), "XboxCaptureMetaData extends RefCounted")
	assert_true(ClassDB.is_parent_class("XboxDisplay", "RefCounted"), "XboxDisplay extends RefCounted")
	assert_true(ClassDB.is_parent_class("XboxDisplayTimeoutDeferral", "RefCounted"), "XboxDisplayTimeoutDeferral extends RefCounted")
	assert_true(ClassDB.is_parent_class("XboxActivation", "RefCounted"), "XboxActivation extends RefCounted")
	assert_true(ClassDB.is_parent_class("XboxResult", "RefCounted"), "XboxResult extends RefCounted")


func test_gdk_root_api() -> void:
	if pending_unless_runtime_available():
		return

	var gdk = get_gdk()

	for method_name in ["initialize", "shutdown", "is_available", "is_initialized", "dispatch", "get_users", "get_game_ui", "get_accessibility", "get_achievements", "get_package", "get_stats", "get_leaderboards", "get_privacy", "get_presence", "get_social", "get_store", "get_profile", "get_string_verify", "get_title_storage", "get_error_reporting", "get_launcher", "get_multiplayer_activity", "get_capture", "get_system", "get_display", "get_activation"]:
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
	assert_true(gdk.get_display() != null, "GDK.display service available")
	assert_true(gdk.get_activation() != null, "GDK.activation service available")
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
	assert_false(ProjectSettings.has_setting(TESTS_LIVE_REQUIRED_SETTING), "gdk/tests/live_required stays internal to tests")

	var initialized_events: Array = []
	var shutdown_events: Array = []
	var runtime_errors: Array = []
	gdk.connect("initialized", func(): initialized_events.append(true))
	gdk.connect("shutdown_completed", func(): shutdown_events.append(true))
	gdk.connect("runtime_error", func(result): runtime_errors.append(result))

	var init_result = initialize_runtime()
	assert_not_null(init_result, "initialize() returns XboxResult")
	if init_result == null:
		disconnect_signal_handlers(gdk, ["initialized", "shutdown_completed", "runtime_error"])
		return

	if init_result.ok:
		assert_eq(initialized_events.size(), 1, "initialized signal emitted once")
		assert_eq(gdk.is_initialized(), true, "is_initialized() true after init")
		assert_true(gdk.dispatch() is int, "dispatch() returns int after init")

		var runtime_error_count_before_repeat = runtime_errors.size()
		var repeat_init_result = gdk.initialize()
		assert_not_null(repeat_init_result, "second initialize() returns XboxResult")
		if repeat_init_result != null:
			assert_eq(repeat_init_result.ok, false, "second initialize() fails while already initialized")
			assert_eq(repeat_init_result.code, "already_initialized", "second initialize() reports already_initialized")

			# After the result-only refactor the root GDK.runtime_error is reserved
			# for XError callback events, so repeated initialize() failures only
			# surface via the returned XboxResult and must NOT emit runtime_error.
			assert_eq(runtime_errors.size(), runtime_error_count_before_repeat, "repeated initialize() does not emit root runtime_error")

		gdk.shutdown()
		assert_eq(shutdown_events.size(), 1, "shutdown_completed signal emitted once")
		assert_eq(gdk.is_initialized(), false, "is_initialized() false after shutdown")
	else:
		assert_eq(runtime_errors.size(), 0, "initialize() failure does not emit root runtime_error (result returned instead)")
		pending("GDK.initialize(): %s" % init_result.message)
		assert_eq(gdk.is_initialized(), false, "is_initialized() remains false after failed init")

	disconnect_signal_handlers(gdk, ["initialized", "shutdown_completed", "runtime_error"])


func test_bootstrap_routes_user_signals_through_combined_handler() -> void:
	var src := FileAccess.get_file_as_string(GDK_BOOTSTRAP_SCRIPT_PATH)
	assert_true(src.length() > 0, "XboxBootstrap source is mirrored into the GDK test host")
	if src.is_empty():
		return

	assert_string_contains(src, "func _on_gdk_user_changed", "XboxBootstrap defines one GDK user-event handler")
	assert_string_contains(src, 'Callable(self, "_on_gdk_user_changed")', "XboxBootstrap binds the user_changed handler")
	assert_string_contains(src, "gdk.users.user_changed.connect", "XboxBootstrap connects only the public XboxUsers user event")
	for signal_name in ["user_added", "user_removed", "primary_user_changed"]:
		assert_false(
				src.contains("gdk.users.%s" % signal_name),
				"XboxBootstrap does not reference removed XboxUsers.%s signal" % signal_name)

	for legacy_handler_name in ["_on_user_added", "_on_user_removed", "_on_user_changed", "_on_primary_user_changed"]:
		assert_false(
				src.contains("func %s" % legacy_handler_name),
				"XboxBootstrap no longer keeps separate %s handlers" % legacy_handler_name)


func test_embed_dispatch_behavior() -> void:
	if pending_unless_runtime_available():
		return

	var gdk = get_gdk()
	var original_embed_dispatch: bool = get_embed_dispatch_enabled()

	# ── Manual-dispatch control (embed_dispatch=false) ───────────────────────
	# First establish whether add_default_user_async() resolves at all on this
	# host when GDK.dispatch() is pumped by hand. A signed-in Xbox identity is
	# NOT required — the op completes with a failure result on identity-less
	# machines — but if it does not resolve even under a manual pump we cannot
	# make any claim about the frame-callback auto-pump and must report pending
	# rather than fail (e.g. a hosted CI runner where the silent add never
	# posts a completion).
	set_embed_dispatch_enabled(false)
	var manual_init_result = initialize_runtime()
	assert_not_null(manual_init_result, "initialize() returns XboxResult for manual-dispatch coverage")
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
	var manual_resolved: Variant = null
	if manual_state["completed"]:
		manual_resolved = manual_state["result"]
	else:
		var advanced_frames = await advance_process_frames(5)
		if not advanced_frames:
			pending("Manual-dispatch fallback: The headless runner could not access process_frame for disabled-mode coverage.")
			reset_runtime()
			set_embed_dispatch_enabled(original_embed_dispatch)
			return
		# With embed_dispatch=false and no manual pump yet, the completion must
		# still be pending — proving the disabled path does not auto-pump.
		assert_eq(manual_state["completed"], false, "embed_dispatch=false keeps async completion pending until GDK.dispatch() is pumped")
		manual_resolved = await await_completion_state(manual_state, 8000)

	reset_runtime()

	if manual_resolved == null:
		# The op never resolved even under a manual pump (e.g. no Xbox identity
		# and the silent add stays pending on this host). We cannot assess the
		# auto-pump, so do not fail CI on an environment limitation.
		pending("Auto-pump regression guard: add_default_user_async() did not resolve under manual GDK.dispatch() on this host; cannot assess the frame-callback pump.")
		set_embed_dispatch_enabled(original_embed_dispatch)
		return

	# ── Auto-dispatch subject (embed_dispatch=true) ──────────────────────────
	# The control above proved the op resolves on this host. Now verify the
	# frame-callback auto-pump drains the SAME completion with NO manual
	# GDK.dispatch(). A timeout here is a real regression (issue #126: the
	# per-frame pump was silently compiled out via a missing GODOT_VERSION_MINOR
	# include), so it is a hard failure — not a pending — because we have
	# positive evidence the op resolves.
	set_embed_dispatch_enabled(true)
	var auto_init_result = initialize_runtime()
	assert_not_null(auto_init_result, "initialize() returns XboxResult for auto-dispatch coverage")
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
	var auto_resolved: Variant = null
	if auto_state["completed"]:
		auto_resolved = auto_state["result"]
	else:
		auto_resolved = await await_completion_state_no_dispatch(auto_state, 8000)
	assert_true(
		auto_resolved != null,
		"Auto-pump regression (issue #126): add_default_user_async() resolves under manual GDK.dispatch() but the frame-callback auto-pump did not drain it with embed_dispatch=true — the per-frame GDK.dispatch() pump is not running.")

	reset_runtime()
	set_embed_dispatch_enabled(original_embed_dispatch)

