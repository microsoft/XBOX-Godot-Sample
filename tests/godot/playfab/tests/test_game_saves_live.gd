extends "res://addons/godot_gdk_tests/playfab_test_base.gd"
## Wave 4 — Live PlayFab Game Saves contract.
##
## Custom-ID users are valid PlayFab sessions, but Game Saves requires an
## Xbox-backed local-user handle. These live checks verify the high-level
## `xbox_user_required` rejection without mutating Game Saves state.

const _DESCRIPTION_PREFIX := "save_settings"


# ── Live setup ────────────────────────────────────────────────────────────

func _begin_live_session() -> Dictionary:
	var outcome := {
		"playfab_user": null,
		"playfab": null,
		"skip_reason": "",
	}

	if pending_unless_live():
		outcome["skip_reason"] = "live"
		return outcome
	if pending_unless_playfab_available():
		outcome["skip_reason"] = "playfab_unavailable"
		return outcome

	var playfab = get_playfab()
	outcome["playfab"] = playfab

	var configured_title_id := str(ProjectSettings.get_setting(PLAYFAB_TITLE_ID_SETTING, "")).strip_edges()
	if configured_title_id.is_empty():
		pending("Set ProjectSettings['playfab/runtime/title_id'] to exercise live PlayFab Game Saves.")
		outcome["skip_reason"] = "no_title_id"
		return outcome

	reset_playfab_runtime()
	var init_result = playfab.initialize()
	if init_result == null or not init_result.ok:
		pending("PlayFab.initialize() live setup skipped: %s" % (init_result.message if init_result != null else "null result"))
		outcome["skip_reason"] = "init_failed"
		return outcome

	var custom_id_session = await sign_in_with_configured_custom_id(playfab, "Game Saves live test")
	if custom_id_session.get("playfab_user") == null:
		outcome["skip_reason"] = custom_id_session.get("skip_reason", "sign_in_failed")
		return outcome

	outcome["playfab_user"] = custom_id_session["playfab_user"]
	return outcome


# ── Custom-ID Game Saves rejection coverage (live, no Game Saves writes) ──

func test_game_saves_read_apis_reject_custom_id_user() -> void:
	var session = await _begin_live_session()
	var playfab_user = session.get("playfab_user")
	if playfab_user == null:
		return

	var playfab = session["playfab"]
	var game_saves = playfab.get_game_saves()
	assert_object_is(game_saves, "PlayFabGameSaves", "PlayFab.get_game_saves() returns PlayFabGameSaves after init")

	var folder_result = game_saves.get_folder(playfab_user)
	assert_playfab_result_error(folder_result, "xbox_user_required", "game_saves.get_folder() rejects custom-ID user")

	var folder_size_result = game_saves.get_folder_size(playfab_user)
	assert_playfab_result_error(folder_size_result, "xbox_user_required", "game_saves.get_folder_size() rejects custom-ID user")

	var quota_result = game_saves.get_remaining_quota(playfab_user)
	assert_playfab_result_error(quota_result, "xbox_user_required", "game_saves.get_remaining_quota() rejects custom-ID user")

	var connected_result = game_saves.is_connected_to_cloud(playfab_user)
	assert_playfab_result_error(connected_result, "xbox_user_required", "game_saves.is_connected_to_cloud() rejects custom-ID user")

	playfab.shutdown()


func test_game_saves_add_user_with_ui_async_rejects_custom_id_user() -> void:
	var session = await _begin_live_session()
	var playfab_user = session.get("playfab_user")
	if playfab_user == null:
		return

	var playfab = session["playfab"]
	var game_saves = playfab.get_game_saves()

	var add_user_signal = game_saves.add_user_with_ui_async(
		playfab_user, PlayFabGameSaves.ADD_USER_OPTION_NONE)
	assert_eq(typeof(add_user_signal), TYPE_SIGNAL,
		"PlayFab.game_saves.add_user_with_ui_async() returns a Signal")
	if typeof(add_user_signal) != TYPE_SIGNAL:
		playfab.shutdown()
		return

	var add_user_result = await await_completion(add_user_signal, 60000)
	assert_playfab_result_error(add_user_result, "xbox_user_required", "game_saves.add_user_with_ui_async() rejects custom-ID user")
	playfab.shutdown()


func test_game_saves_set_save_description_async_rejects_custom_id_user() -> void:
	var session = await _begin_live_session()
	var playfab_user = session.get("playfab_user")
	if playfab_user == null:
		return

	var playfab = session["playfab"]
	var game_saves = playfab.get_game_saves()
	var description := with_unique_id(_DESCRIPTION_PREFIX)

	var set_signal = game_saves.set_save_description_async(playfab_user, description)
	assert_eq(typeof(set_signal), TYPE_SIGNAL,
		"PlayFab.game_saves.set_save_description_async() returns a Signal")
	if typeof(set_signal) != TYPE_SIGNAL:
		playfab.shutdown()
		return

	var set_result = await await_completion(set_signal, 60000)
	assert_playfab_result_error(set_result, "xbox_user_required", "game_saves.set_save_description_async() rejects custom-ID user")
	playfab.shutdown()


func test_game_saves_reset_cloud_async_rejects_custom_id_user() -> void:
	var session = await _begin_live_session()
	var playfab_user = session.get("playfab_user")
	if playfab_user == null:
		return

	var playfab = session["playfab"]
	var game_saves = playfab.get_game_saves()

	var reset_signal = game_saves.reset_cloud_async(playfab_user)
	assert_eq(typeof(reset_signal), TYPE_SIGNAL,
		"PlayFab.game_saves.reset_cloud_async() returns a Signal")
	if typeof(reset_signal) != TYPE_SIGNAL:
		playfab.shutdown()
		return

	var reset_result = await await_completion(reset_signal, 60000)
	assert_playfab_result_error(reset_result, "xbox_user_required", "game_saves.reset_cloud_async() rejects custom-ID user")
	playfab.shutdown()
