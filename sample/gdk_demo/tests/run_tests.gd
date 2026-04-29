extends SceneTree
## GodotGDK runtime/services baseline test suite
## Run: godot --headless --script res://tests/run_tests.gd

var _pass_count := 0
var _fail_count := 0
var _skip_count := 0

func _log_pass(name: String, detail: String = "") -> void:
	_pass_count += 1
	if detail:
		print("  PASS: %s — %s" % [name, detail])
	else:
		print("  PASS: %s" % name)

func _log_fail(name: String, detail: String = "") -> void:
	_fail_count += 1
	if detail:
		printerr("  FAIL: %s — %s" % [name, detail])
	else:
		printerr("  FAIL: %s" % name)

func _log_skip(name: String, reason: String = "") -> void:
	_skip_count += 1
	if reason:
		print("  SKIP: %s — %s" % [name, reason])
	else:
		print("  SKIP: %s" % name)

func _assert_true(condition: bool, name: String, detail: String = "") -> void:
	if condition:
		_log_pass(name, detail)
	else:
		_log_fail(name, detail)

func _assert_eq(actual, expected, name: String) -> void:
	if actual == expected:
		_log_pass(name, str(actual))
	else:
		_log_fail(name, "expected %s, got %s" % [str(expected), str(actual)])

func _assert_not_null(value, name: String) -> void:
	if value != null:
		_log_pass(name, str(typeof(value)))
	else:
		_log_fail(name, "got null")

func _assert_has_method(obj: Object, method_name: String, test_name: String = "") -> void:
	var label = test_name if test_name else "%s.%s() exists" % [obj.get_class(), method_name]
	_assert_true(obj.has_method(method_name), label)

func _assert_has_signal(obj: Object, signal_name: String, test_name: String = "") -> void:
	var label = test_name if test_name else "%s.%s signal exists" % [obj.get_class(), signal_name]
	_assert_true(obj.has_signal(signal_name), label)

func _test_singleton_availability() -> void:
	print("\n── Singleton Availability ──")

	var gdk = Engine.get_singleton("GDK")
	_assert_not_null(gdk, "Engine.get_singleton('GDK')")
	_assert_true(Engine.get_singleton("GDKUser") == null, "legacy GDKUser singleton removed")

func _test_class_registration() -> void:
	print("\n── Class Registration ──")

	for registered_class in ["GDK", "GDKUsers", "GDKUser", "GDKAchievements", "GDKAchievement", "GDKAsyncOp", "GDKDispatchOp", "GDKResult"]:
		_assert_true(ClassDB.class_exists(registered_class), "%s registered in ClassDB" % registered_class)

	_assert_true(ClassDB.is_parent_class("GDK", "Object"), "GDK extends Object")
	_assert_true(ClassDB.is_parent_class("GDKUsers", "RefCounted"), "GDKUsers extends RefCounted")
	_assert_true(ClassDB.is_parent_class("GDKUser", "RefCounted"), "GDKUser extends RefCounted")
	_assert_true(ClassDB.is_parent_class("GDKAchievements", "RefCounted"), "GDKAchievements extends RefCounted")
	_assert_true(ClassDB.is_parent_class("GDKAchievement", "RefCounted"), "GDKAchievement extends RefCounted")
	_assert_true(ClassDB.is_parent_class("GDKAsyncOp", "RefCounted"), "GDKAsyncOp extends RefCounted")
	_assert_true(ClassDB.is_parent_class("GDKDispatchOp", "GDKAsyncOp"), "GDKDispatchOp extends GDKAsyncOp")
	_assert_true(ClassDB.is_parent_class("GDKResult", "RefCounted"), "GDKResult extends RefCounted")

func _test_gdk_root_api() -> void:
	print("\n── GDK Root API ──")

	var gdk = Engine.get_singleton("GDK")
	if gdk == null:
		_log_fail("GDK root singleton missing, skipping root API group")
		return

	gdk.shutdown()

	for method_name in ["initialize", "shutdown", "is_available", "is_initialized", "dispatch", "get_last_error", "get_users", "get_achievements"]:
		_assert_has_method(gdk, method_name)

	for signal_name in ["initialized", "shutdown_completed", "runtime_error"]:
		_assert_has_signal(gdk, signal_name)

	_assert_true(gdk.get_users() != null, "GDK.users service available")
	_assert_true(gdk.get_achievements() != null, "GDK.achievements service available")
	_assert_true(gdk.is_available() is bool, "is_available() returns bool")
	_assert_eq(gdk.is_initialized(), false, "is_initialized() starts false")
	_assert_eq(gdk.dispatch(), 0, "dispatch() safe before init")

	var last_error = gdk.get_last_error()
	_assert_not_null(last_error, "get_last_error() returns GDKResult")
	if last_error != null:
		_assert_true(last_error.has_method("is_ok"), "last error exposes result API")

	var init_result = gdk.initialize()
	_assert_not_null(init_result, "initialize() returns GDKResult")
	if init_result == null:
		return

	if init_result.ok:
		_assert_eq(gdk.is_initialized(), true, "is_initialized() true after init")
		_assert_true(gdk.dispatch() is int, "dispatch() returns int after init")
		gdk.shutdown()
		_assert_eq(gdk.is_initialized(), false, "is_initialized() false after shutdown")
	else:
		_log_skip("GDK.initialize()", init_result.message)
		_assert_eq(gdk.is_initialized(), false, "is_initialized() remains false after failed init")

func _test_gdk_users_api() -> void:
	print("\n── GDK Users API ──")

	var gdk = Engine.get_singleton("GDK")
	if gdk == null:
		_log_fail("GDK root singleton missing, skipping users API group")
		return

	gdk.shutdown()

	var users = gdk.get_users()
	_assert_not_null(users, "GDK.users returns service object")
	if users == null:
		return

	for method_name in [
		"add_default_user_async",
		"add_user_with_ui_async",
		"get_primary_user",
		"get_users",
		"check_privilege_async",
		"resolve_privilege_with_ui_async",
		"resolve_issue_with_ui_async",
		"get_gamer_picture_async",
		"get_token_and_signature_async"
	]:
		_assert_has_method(users, method_name)

	for signal_name in ["user_added", "user_removed", "user_changed", "primary_user_changed"]:
		_assert_has_signal(users, signal_name)

	_assert_true(users.get_users() is Array, "get_users() returns Array")
	_assert_true(users.get_primary_user() == null, "get_primary_user() starts null")

	var blank_user = GDKUser.new()
	_assert_not_null(blank_user, "GDKUser.new() returns wrapper")
	if blank_user != null:
		for method_name in [
			"get_local_id",
			"get_xuid",
			"get_gamertag",
			"get_age_group",
			"get_age_group_name",
			"get_sign_in_state",
			"get_sign_in_state_name",
			"is_guest",
			"is_signed_in",
			"is_store_user"
		]:
			_assert_has_method(blank_user, method_name)

		_assert_eq(blank_user.get_local_id(), 0, "blank GDKUser local_id defaults to 0")
		_assert_eq(blank_user.get_xuid(), "", "blank GDKUser xuid defaults empty")
		_assert_eq(blank_user.get_gamertag(), "", "blank GDKUser gamertag defaults empty")
		_assert_eq(blank_user.get_age_group(), GDKUser.AGE_GROUP_UNKNOWN, "blank GDKUser age_group defaults to AGE_GROUP_UNKNOWN")
		_assert_eq(blank_user.get_age_group_name(), "unknown", "blank GDKUser age_group_name defaults to unknown")
		_assert_eq(blank_user.get_sign_in_state(), GDKUser.SIGN_IN_STATE_SIGNED_OUT, "blank GDKUser sign_in_state defaults to SIGN_IN_STATE_SIGNED_OUT")
		_assert_eq(blank_user.get_sign_in_state_name(), "signed_out", "blank GDKUser sign_in_state_name defaults to signed_out")
		_assert_eq(blank_user.is_guest(), false, "blank GDKUser guest defaults false")
		_assert_eq(blank_user.is_signed_in(), false, "blank GDKUser signed_in defaults false")
		_assert_eq(blank_user.is_store_user(), false, "blank GDKUser store_user defaults false")

	var op = users.add_default_user_async()
	_assert_not_null(op, "add_default_user_async() returns GDKAsyncOp")
	if op != null:
		_assert_true(op is GDKAsyncOp, "add_default_user_async() uses XAsync-backed op type")
		_assert_true(op.is_done(), "uninitialized add_default_user_async() completes immediately")
		var result = op.get_result()
		_assert_not_null(result, "immediate async error exposes result")
		if result != null:
			_assert_eq(result.ok, false, "immediate async error result.ok == false")
			_assert_true(result.message.length() > 0, "immediate async error includes message")

	var privilege_op = users.check_privilege_async(blank_user, 254)
	_assert_not_null(privilege_op, "check_privilege_async() returns GDKAsyncOp")
	if privilege_op != null:
		_assert_true(privilege_op is GDKAsyncOp, "check_privilege_async() uses GDKAsyncOp")
		_assert_true(privilege_op.is_done(), "uninitialized check_privilege_async() completes immediately")

	var resolve_privilege_op = users.resolve_privilege_with_ui_async(blank_user, 254)
	_assert_not_null(resolve_privilege_op, "resolve_privilege_with_ui_async() returns GDKAsyncOp")
	if resolve_privilege_op != null:
		_assert_true(resolve_privilege_op is GDKAsyncOp, "resolve_privilege_with_ui_async() uses GDKAsyncOp")
		_assert_true(resolve_privilege_op.is_done(), "uninitialized resolve_privilege_with_ui_async() completes immediately")

	var resolve_issue_op = users.resolve_issue_with_ui_async(blank_user)
	_assert_not_null(resolve_issue_op, "resolve_issue_with_ui_async() returns GDKAsyncOp")
	if resolve_issue_op != null:
		_assert_true(resolve_issue_op is GDKAsyncOp, "resolve_issue_with_ui_async() uses GDKAsyncOp")
		_assert_true(resolve_issue_op.is_done(), "uninitialized resolve_issue_with_ui_async() completes immediately")

	var gamer_picture_op = users.get_gamer_picture_async(blank_user)
	_assert_not_null(gamer_picture_op, "get_gamer_picture_async() returns GDKAsyncOp")
	if gamer_picture_op != null:
		_assert_true(gamer_picture_op is GDKAsyncOp, "get_gamer_picture_async() uses GDKAsyncOp")
		_assert_true(gamer_picture_op.is_done(), "uninitialized get_gamer_picture_async() completes immediately")

	var token_op = users.get_token_and_signature_async(blank_user, "GET", "https://example.com")
	_assert_not_null(token_op, "get_token_and_signature_async() returns GDKAsyncOp")
	if token_op != null:
		_assert_true(token_op is GDKAsyncOp, "get_token_and_signature_async() uses GDKAsyncOp")
		_assert_true(token_op.is_done(), "uninitialized get_token_and_signature_async() completes immediately")

func _test_gdk_achievements_api() -> void:
	print("\n── GDK Achievements API ──")

	var gdk = Engine.get_singleton("GDK")
	if gdk == null:
		_log_fail("GDK root singleton missing, skipping achievements API group")
		return

	gdk.shutdown()

	var achievements = gdk.get_achievements()
	_assert_not_null(achievements, "GDK.achievements returns service object")
	if achievements == null:
		return

	for method_name in ["query_player_achievements_async", "update_achievement_async", "get_cached_achievements"]:
		_assert_has_method(achievements, method_name)

	for signal_name in ["achievement_unlocked", "achievements_updated"]:
		_assert_has_signal(achievements, signal_name)

	_assert_true(achievements.get_cached_achievements(null) is Array, "get_cached_achievements() returns Array")

	var query_op = achievements.query_player_achievements_async(null)
	_assert_not_null(query_op, "query_player_achievements_async() returns GDKDispatchOp")
	if query_op != null:
		_assert_true(query_op is GDKDispatchOp, "query_player_achievements_async() uses dispatch-backed op type")
		_assert_true(query_op.is_done(), "uninitialized query_player_achievements_async() completes immediately")
		var query_result = query_op.get_result()
		_assert_not_null(query_result, "achievement query error exposes result")
		if query_result != null:
			_assert_eq(query_result.ok, false, "achievement query error result.ok == false")
			_assert_true(query_result.message.length() > 0, "achievement query error includes message")

	var update_op = achievements.update_achievement_async(null, "1", 25)
	_assert_not_null(update_op, "update_achievement_async() returns GDKDispatchOp")
	if update_op != null:
		_assert_true(update_op is GDKDispatchOp, "update_achievement_async() uses dispatch-backed op type")
		_assert_true(update_op.is_done(), "uninitialized update_achievement_async() completes immediately")
		var update_result = update_op.get_result()
		_assert_not_null(update_result, "achievement update error exposes result")
		if update_result != null:
			_assert_eq(update_result.ok, false, "achievement update error result.ok == false")
			_assert_true(update_result.message.length() > 0, "achievement update error includes message")

func _test_signal_connectivity() -> void:
	print("\n── Signal Connectivity ──")

	var gdk = Engine.get_singleton("GDK")
	if gdk:
		gdk.connect("initialized", func(): pass)
		gdk.connect("shutdown_completed", func(): pass)
		gdk.connect("runtime_error", func(_result): pass)
		_log_pass("GDK root signals connectable")
		for signal_name in ["initialized", "shutdown_completed", "runtime_error"]:
			for conn in gdk.get_signal_connection_list(signal_name):
				gdk.disconnect(signal_name, conn["callable"])

		var users = gdk.get_users()
		if users:
			users.connect("user_added", func(_user): pass)
			users.connect("user_removed", func(_local_id): pass)
			users.connect("user_changed", func(_user): pass)
			users.connect("primary_user_changed", func(_user): pass)
			_log_pass("GDK.users signals connectable")
			for signal_name in ["user_added", "user_removed", "user_changed", "primary_user_changed"]:
				for conn in users.get_signal_connection_list(signal_name):
					users.disconnect(signal_name, conn["callable"])

		var achievements = gdk.get_achievements()
		if achievements:
			achievements.connect("achievement_unlocked", func(_user, _achievement_id): pass)
			achievements.connect("achievements_updated", func(_user): pass)
			_log_pass("GDK.achievements signals connectable")
			for signal_name in ["achievement_unlocked", "achievements_updated"]:
				for conn in achievements.get_signal_connection_list(signal_name):
					achievements.disconnect(signal_name, conn["callable"])

func _test_addon_structure() -> void:
	print("\n── Addon Structure ──")

	_assert_true(FileAccess.file_exists("res://addons/godot_gdk/plugin.cfg"), "plugin.cfg exists")
	_assert_true(FileAccess.file_exists("res://addons/godot_gdk/godot_gdk.gdextension"), ".gdextension file exists")
	_assert_true(FileAccess.file_exists("res://addons/godot_gdk/bin/godot_gdk.windows.debug.x86_64.dll") or \
		FileAccess.file_exists("res://addons/godot_gdk/bin/Debug/godot_gdk.windows.debug.x86_64.dll"), "GDK DLL exists in bin/")

func _initialize() -> void:
	print("╔══════════════════════════════════════╗")
	print("║   GodotGDK Runtime/Services Tests    ║")
	print("╚══════════════════════════════════════╝")

	_test_singleton_availability()
	_test_class_registration()
	_test_gdk_root_api()
	_test_gdk_users_api()
	_test_gdk_achievements_api()
	_test_signal_connectivity()
	_test_addon_structure()

	var total = _pass_count + _fail_count + _skip_count
	print("\n══════════════════════════════════════")
	print("Results: %d passed, %d failed, %d skipped (of %d)" % [
		_pass_count, _fail_count, _skip_count, total])
	print("══════════════════════════════════════")

	if _fail_count > 0:
		printerr("SUITE FAILED")
		quit(1)
	else:
		print("SUITE PASSED")
		quit(0)
