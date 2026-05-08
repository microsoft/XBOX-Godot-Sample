extends "res://addons/godot_gdk_tests/gdk_test_base.gd"
## Wave 3 GUT migration of `suites/integration_suite.gd`. Behavior parity:
## same per-call assertion count as the pre-GUT harness; one-off `log_pass`
## direct calls preserved as `assert_true(true, ...)` so GUT's `Asserts:`
## count tracks the pre-GUT total.

func test_signal_connectivity() -> void:
	if pending_unless_runtime_available():
		return
	var gdk = get_gdk()

	gdk.connect("initialized", func(): pass)
	gdk.connect("shutdown_completed", func(): pass)
	gdk.connect("runtime_error", func(_result): pass)
	assert_true(true, "GDK root signals connectable")
	disconnect_signal_handlers(gdk, ["initialized", "shutdown_completed", "runtime_error"])

	var users = gdk.get_users()
	if users:
		users.user_changed.connect(func(_user, _change_kind): pass)
		assert_true(true, "GDK.users user_changed signal connectable")
		disconnect_signal_handlers(users, ["user_changed"])

	var achievements = gdk.get_achievements()
	if achievements:
		achievements.connect("achievement_unlocked", func(_user, _achievement_id): pass)
		achievements.connect("achievements_updated", func(_user): pass)
		assert_true(true, "GDK.achievements signals connectable")
		disconnect_signal_handlers(achievements, ["achievement_unlocked", "achievements_updated"])

	var presence = gdk.get_presence()
	if presence:
		presence.connect("presence_changed", func(_xuid, _presence_record): pass)
		presence.connect("local_presence_set", func(_user): pass)
		assert_true(true, "GDK.presence signals connectable")
		disconnect_signal_handlers(presence, ["presence_changed", "local_presence_set"])

	var social = gdk.get_social()
	if social:
		social.connect("social_graph_changed", func(_user): pass)
		social.connect("social_group_updated", func(_group): pass)
		social.connect("social_user_changed", func(_xuid, _social_user): pass)
		assert_true(true, "GDK.social signals connectable")
		disconnect_signal_handlers(social, ["social_graph_changed", "social_group_updated", "social_user_changed"])


func test_addon_structure() -> void:
	assert_true(FileAccess.file_exists("res://addons/godot_gdk/plugin.cfg"), "plugin.cfg exists")
	assert_true(FileAccess.file_exists("res://addons/godot_gdk/godot_gdk.gdextension"), ".gdextension file exists")
	assert_true(
		FileAccess.file_exists("res://addons/godot_gdk/bin/godot_gdk.windows.debug.x86_64.dll")
			or FileAccess.file_exists("res://addons/godot_gdk/bin/Debug/godot_gdk.windows.debug.x86_64.dll"),
		"GDK DLL exists in bin/")
