extends "res://addons/godot_gdk_tests/gdk_test_base.gd"
## Wave 4 GUT coverage for `GDKMultiplayerActivity`.
##
## Mirrors the per-service style of `test_achievements.gd` / `test_presence.gd`:
## * Surface checks (methods + signals) that always run.
## * Validation-error checks for synchronous rejection paths.
## * Live calls for `set_activity_async` / `delete_activity_async` /
##   `send_invites_async` gated by `pending_unless_live()` so default CI / dev
##   runs stay green when no signed-in user is available.

func before_each() -> void:
	reset_runtime()


func after_each() -> void:
	reset_runtime()


func test_multiplayer_activity_full_flow() -> void:
	if pending_unless_runtime_available():
		return

	var gdk = get_gdk()
	var multiplayer_activity = gdk.get_multiplayer_activity()
	assert_not_null(multiplayer_activity, "GDK.multiplayer_activity returns service object")
	if multiplayer_activity == null:
		return

	for method_name in [
		"set_activity_async",
		"get_activities_async",
		"get_cached_activity",
		"delete_activity_async",
		"send_invites_async",
		"show_invite_ui_async",
		"update_recent_players",
		"flush_recent_players_async",
		"accept_pending_invite",
	]:
		assert_has_method_named(multiplayer_activity, method_name)

	for signal_name in ["activities_updated", "pending_invite_received", "invite_accepted"]:
		assert_has_signal_named(multiplayer_activity, signal_name)

	var blank_info = instantiate_class("GDKMultiplayerActivityInfo")
	assert_not_null(blank_info, "GDKMultiplayerActivityInfo.new() returns wrapper")
	if blank_info != null:
		for method_name in [
			"get_xuid",
			"get_connection_string",
			"get_join_restriction",
			"get_max_players",
			"get_current_players",
			"get_group_id",
			"get_platform",
		]:
			assert_has_method_named(blank_info, method_name)
		assert_eq(blank_info.get_xuid(), "", "blank GDKMultiplayerActivityInfo xuid defaults empty")
		assert_eq(blank_info.get_connection_string(), "", "blank GDKMultiplayerActivityInfo connection_string defaults empty")
		assert_eq(blank_info.get_join_restriction(), "", "blank GDKMultiplayerActivityInfo join_restriction defaults empty")
		assert_eq(blank_info.get_max_players(), 0, "blank GDKMultiplayerActivityInfo max_players defaults 0")
		assert_eq(blank_info.get_current_players(), 0, "blank GDKMultiplayerActivityInfo current_players defaults 0")
		assert_eq(blank_info.get_group_id(), "", "blank GDKMultiplayerActivityInfo group_id defaults empty")
		assert_eq(blank_info.get_platform(), "", "blank GDKMultiplayerActivityInfo platform defaults empty")

	assert_true(multiplayer_activity.get_cached_activity("0") == null, "get_cached_activity() returns null before any query")

	var blank_user = instantiate_class("GDKUser")

	var pre_init_set_signal = multiplayer_activity.set_activity_async(blank_user, "")
	await assert_signal_result_error(pre_init_set_signal, "invalid_connection_string", "set_activity_async() rejects empty connection_string before init")

	var pre_init_send_signal = multiplayer_activity.send_invites_async(blank_user, PackedStringArray())
	await assert_signal_result_error(pre_init_send_signal, "missing_xuids", "send_invites_async() rejects empty XUID list before init")

	var init_result = initialize_runtime()
	assert_not_null(init_result, "GDK.initialize() for multiplayer_activity behavior returns GDKResult")
	if init_result == null:
		return
	if not init_result.ok:
		pending("Multiplayer activity runtime behavior: %s" % init_result.message)
		return

	var activities_null_user_signal = multiplayer_activity.get_activities_async(null, PackedStringArray(["1"]))
	await assert_signal_result_error(activities_null_user_signal, "invalid_user", "get_activities_async() rejects null users after initialize")

	var invite_ui_null_user_signal = multiplayer_activity.show_invite_ui_async(null)
	await assert_signal_result_error(invite_ui_null_user_signal, "invalid_user", "show_invite_ui_async() rejects null users after initialize")

	var recent_players_null_user_result = multiplayer_activity.update_recent_players(null, PackedStringArray(["1"]))
	assert_result_error(recent_players_null_user_result, "invalid_user", "update_recent_players() rejects null users after initialize")

	var flush_recent_null_user_signal = multiplayer_activity.flush_recent_players_async(null)
	await assert_signal_result_error(flush_recent_null_user_signal, "invalid_user", "flush_recent_players_async() rejects null users after initialize")

	var invalid_restriction_signal = multiplayer_activity.set_activity_async(blank_user, "join-token", "not-a-restriction")
	await assert_signal_result_error(invalid_restriction_signal, "invalid_join_restriction", "set_activity_async() rejects unknown join_restriction values")

	var invalid_player_counts_signal = multiplayer_activity.set_activity_async(blank_user, "join-token", "followed", -1, 0)
	await assert_signal_result_error(invalid_player_counts_signal, "invalid_player_counts", "set_activity_async() rejects negative max_players")

	var swap_player_counts_signal = multiplayer_activity.set_activity_async(blank_user, "join-token", "followed", 4, 5)
	await assert_signal_result_error(swap_player_counts_signal, "invalid_player_counts", "set_activity_async() rejects current_players > max_players")

	var empty_xuids_invite_signal = multiplayer_activity.send_invites_async(blank_user, PackedStringArray())
	await assert_signal_result_error(empty_xuids_invite_signal, "missing_xuids", "send_invites_async() rejects empty XUID list after init")

	var invalid_xuid_invite_signal = multiplayer_activity.send_invites_async(blank_user, PackedStringArray(["not-a-number"]))
	await assert_signal_result_error(invalid_xuid_invite_signal, "invalid_xuids", "send_invites_async() rejects non-numeric XUID strings")

	var invalid_invite_uri = multiplayer_activity.accept_pending_invite("")
	assert_not_null(invalid_invite_uri, "accept_pending_invite('') returns GDKResult")
	if invalid_invite_uri != null:
		assert_false(invalid_invite_uri.ok, "accept_pending_invite('') rejects empty URIs")
		assert_eq(invalid_invite_uri.code, "invalid_invite_uri", "accept_pending_invite('') reports invalid_invite_uri")

	var runtime_errors: Array = []
	gdk.connect("runtime_error", func(result): runtime_errors.append(result))

	var sign_in = await ensure_primary_user()
	var sign_in_signal = sign_in["signal"]
	var sign_in_result = sign_in["result"]
	var user = sign_in["user"]
	if typeof(sign_in_signal) == TYPE_SIGNAL and sign_in_result == null:
		assert_true(false, "Default-user flow for multiplayer_activity completes — timed out waiting for a signed-in user")
		disconnect_signal_handlers(gdk, ["runtime_error"])
		return
	if user == null:
		if sign_in_result != null and not sign_in_result.ok:
			assert_true(sign_in_result.code.length() > 0, "failed multiplayer_activity sign-in exposes an error code")
			assert_true(sign_in_result.message.length() > 0, "failed multiplayer_activity sign-in exposes an error message")
			pending("Multiplayer activity sign-in: %s" % sign_in_result.message)
		else:
			pending("Multiplayer activity behavior: No signed-in user is available on this machine.")
		disconnect_signal_handlers(gdk, ["runtime_error"])
		return

	if pending_unless_live():
		disconnect_signal_handlers(gdk, ["runtime_error"])
		return

	var connection_string = with_unique_id("gdkfleet-mpa-conn")
	var set_signal = multiplayer_activity.set_activity_async(user, connection_string, "followed", 4, 1, "", false)
	assert_true(typeof(set_signal) == TYPE_SIGNAL, "set_activity_async() returns completion Signal for a signed-in user")
	if typeof(set_signal) != TYPE_SIGNAL:
		disconnect_signal_handlers(gdk, ["runtime_error"])
		return

	var set_result = await await_completion(set_signal, 8000)
	if set_result == null:
		pending("set_activity_async(): Timed out waiting for the activity service to finish.")
		disconnect_signal_handlers(gdk, ["runtime_error"])
		return

	if not set_result.ok:
		assert_true(set_result.code.length() > 0, "failed set_activity_async exposes an error code")
		assert_true(set_result.message.length() > 0, "failed set_activity_async exposes an error message")
		pending("set_activity_async(): %s" % set_result.message)
		disconnect_signal_handlers(gdk, ["runtime_error"])
		return

	assert_true(set_result.ok, "set_activity_async() reports ok for a signed-in user")

	var delete_signal = multiplayer_activity.delete_activity_async(user)
	assert_true(typeof(delete_signal) == TYPE_SIGNAL, "delete_activity_async() returns completion Signal")
	if typeof(delete_signal) == TYPE_SIGNAL:
		var delete_result = await await_completion(delete_signal, 8000)
		if delete_result == null:
			pending("delete_activity_async(): Timed out waiting for the activity service to finish.")
		elif not delete_result.ok:
			pending("delete_activity_async(): %s" % delete_result.message)
		else:
			assert_true(delete_result.ok, "delete_activity_async() reports ok after a successful set")

	disconnect_signal_handlers(gdk, ["runtime_error"])
