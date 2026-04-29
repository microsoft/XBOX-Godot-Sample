extends RefCounted

func run(context) -> void:
	_test_signal_connectivity(context)
	_test_addon_structure(context)

func _test_signal_connectivity(context) -> void:
	context.log_section("Signal Connectivity")

	var gdk = context.get_gdk()
	if gdk == null:
		return

	gdk.connect("initialized", func(): pass)
	gdk.connect("shutdown_completed", func(): pass)
	gdk.connect("runtime_error", func(_result): pass)
	context.log_pass("GDK root signals connectable")
	context.disconnect_signal_handlers(gdk, ["initialized", "shutdown_completed", "runtime_error"])

	var users = gdk.get_users()
	if users:
		users.connect("user_added", func(_user): pass)
		users.connect("user_removed", func(_local_id): pass)
		users.connect("user_changed", func(_user): pass)
		users.connect("primary_user_changed", func(_user): pass)
		context.log_pass("GDK.users signals connectable")
		context.disconnect_signal_handlers(users, ["user_added", "user_removed", "user_changed", "primary_user_changed"])

	var achievements = gdk.get_achievements()
	if achievements:
		achievements.connect("achievement_unlocked", func(_user, _achievement_id): pass)
		achievements.connect("achievements_updated", func(_user): pass)
		context.log_pass("GDK.achievements signals connectable")
		context.disconnect_signal_handlers(achievements, ["achievement_unlocked", "achievements_updated"])

	var presence = gdk.get_presence()
	if presence:
		presence.connect("presence_changed", func(_xuid, _presence_record): pass)
		presence.connect("local_presence_set", func(_user): pass)
		context.log_pass("GDK.presence signals connectable")
		context.disconnect_signal_handlers(presence, ["presence_changed", "local_presence_set"])

	var social = gdk.get_social()
	if social:
		social.connect("social_graph_changed", func(_user): pass)
		social.connect("social_group_updated", func(_group): pass)
		social.connect("social_user_changed", func(_xuid, _social_user): pass)
		context.log_pass("GDK.social signals connectable")
		context.disconnect_signal_handlers(social, ["social_graph_changed", "social_group_updated", "social_user_changed"])

func _test_addon_structure(context) -> void:
	context.log_section("Addon Structure")

	context.assert_true(FileAccess.file_exists("res://addons/godot_gdk/plugin.cfg"), "plugin.cfg exists")
	context.assert_true(FileAccess.file_exists("res://addons/godot_gdk/godot_gdk.gdextension"), ".gdextension file exists")
	context.assert_true(
		FileAccess.file_exists("res://addons/godot_gdk/bin/godot_gdk.windows.debug.x86_64.dll")
			or FileAccess.file_exists("res://addons/godot_gdk/bin/Debug/godot_gdk.windows.debug.x86_64.dll"),
		"GDK DLL exists in bin/")
