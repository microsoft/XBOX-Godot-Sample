extends "res://addons/godot_gdk_tests/gdk_test_base.gd"


func before_each() -> void:
	reset_runtime()


func after_each() -> void:
	reset_runtime()


func test_game_ui_surface_and_validation() -> void:
	if pending_unless_runtime_available():
		return

	var gdk = get_gdk()
	var game_ui = gdk.get_game_ui()
	assert_not_null(game_ui, "GDK.game_ui returns service object")
	if game_ui == null:
		return

	for method_name in [
		"show_message_dialog_async",
		"set_notification_position_hint",
		"show_player_profile_card_async",
		"show_player_picker_async",
		"resolve_privilege_with_ui_async",
	]:
		assert_has_method_named(game_ui, method_name)

	var blank_user = instantiate_class("GDKUser")

	var empty_title_signal = game_ui.show_message_dialog_async("", "Body")
	await assert_signal_result_error(empty_title_signal, "invalid_title", "show_message_dialog_async() rejects empty title")

	var empty_message_signal = game_ui.show_message_dialog_async("Title", "")
	await assert_signal_result_error(empty_message_signal, "invalid_message", "show_message_dialog_async() rejects empty message")

	var invalid_notification_result = game_ui.set_notification_position_hint("middle")
	assert_result_error(invalid_notification_result, "invalid_notification_position", "set_notification_position_hint() rejects unsupported positions")

	var not_initialized_profile_signal = game_ui.show_player_profile_card_async(blank_user, "123")
	await assert_signal_result_error(not_initialized_profile_signal, "not_initialized", "show_player_profile_card_async() requires initialized runtime")

	var not_initialized_privilege_signal = game_ui.resolve_privilege_with_ui_async(blank_user, 254)
	await assert_signal_result_error(not_initialized_privilege_signal, "not_initialized", "resolve_privilege_with_ui_async() requires initialized runtime")

	var init_result = initialize_runtime()
	assert_not_null(init_result, "GDK.initialize() for game_ui behavior returns GDKResult")
	if init_result == null:
		return
	if not init_result.ok:
		pending("Game UI runtime behavior: %s" % init_result.message)
		return

	var invalid_user_profile_signal = game_ui.show_player_profile_card_async(blank_user, "123")
	await assert_signal_result_error(invalid_user_profile_signal, "invalid_user", "show_player_profile_card_async() rejects invalid users")

	var invalid_target_profile_signal = game_ui.show_player_profile_card_async(blank_user, "not-a-number")
	await assert_signal_result_error(invalid_target_profile_signal, "invalid_target_xuid", "show_player_profile_card_async() rejects invalid target_xuid")

	var zero_target_profile_signal = game_ui.show_player_profile_card_async(blank_user, "0")
	await assert_signal_result_error(zero_target_profile_signal, "invalid_target_xuid", "show_player_profile_card_async() rejects zero target_xuid")

	var invalid_picker_xuid_signal = game_ui.show_player_picker_async(
		blank_user,
		"Pick players",
		PackedStringArray(["not-a-number"]))
	await assert_signal_result_error(invalid_picker_xuid_signal, "invalid_xuids", "show_player_picker_async() rejects invalid selectable XUIDs")

	var zero_picker_xuid_signal = game_ui.show_player_picker_async(
		blank_user,
		"Pick players",
		PackedStringArray(["0"]))
	await assert_signal_result_error(zero_picker_xuid_signal, "invalid_xuids", "show_player_picker_async() rejects zero selectable XUIDs")

	var invalid_preselected_signal = game_ui.show_player_picker_async(
		blank_user,
		"Pick players",
		PackedStringArray(["12345"]),
		PackedStringArray(["99999"]))
	await assert_signal_result_error(invalid_preselected_signal, "invalid_preselected_xuids", "show_player_picker_async() rejects preselected XUIDs not present in selectable list")

	var invalid_user_privilege_signal = game_ui.resolve_privilege_with_ui_async(blank_user, 254)
	await assert_signal_result_error(invalid_user_privilege_signal, "invalid_user", "resolve_privilege_with_ui_async() rejects invalid users")
