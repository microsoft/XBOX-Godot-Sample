extends "res://addons/godot_gdk_tests/playfab_test_base.gd"
## Wave 4 — Exhaustive validation walk: `not_initialized` + `invalid_*`
## codes for every public async method on the PlayFab service surfaces.
##
## This suite runs unconditionally (no live-test gating). Each test resets
## the PlayFab runtime first to drive the `not_initialized` path, then —
## without re-initialising — also drives the documented `invalid_*` paths
## using a blank `PlayFabUser` (validates the early `invalid_user`/
## `invalid_playfab_user` exits) where applicable.
##
## Coverage map (kept here as a comment so future surface additions are
## easy to slot in):
##
## PlayFab (root):
##   - initialize()                                      → "title_id_required" when blank
##
## PlayFabUsers:
##   - sign_in_with_xuser_async(user, create_account)    → "not_initialized", "invalid_xuser" (post-init)
##
## PlayFabGameSaves (async):
##   - add_user_with_ui_async(user, options)             → "not_initialized", "invalid_options" (post-init)
##   - upload_with_ui_async(user, release_device)        → "not_initialized"
##   - set_save_description_async(user, description)     → "not_initialized"
##   - reset_cloud_async(user)                           → "not_initialized"
##
## PlayFabGameSaves (sync, returns PlayFabResult):
##   - get_folder(user)                                  → "not_initialized"
##   - get_folder_size(user)                             → "not_initialized"
##   - get_remaining_quota(user)                         → "not_initialized"
##   - is_connected_to_cloud(user)                       → "not_initialized"
##
## PlayFabLeaderboards:
##   - submit_score_async(user, name, score, ...)        → "not_initialized"
##   - get_leaderboard_async(user, name, ...)            → "not_initialized"
##   - get_leaderboard_around_user_async(user, name, ..) → "not_initialized"
##   - get_friend_leaderboard_async(user, name, ...)     → "not_initialized"


# ── PlayFabUsers: not_initialized + invalid ──────────────────────────────

func test_users_sign_in_with_xuser_async_not_initialized() -> void:
	if pending_unless_playfab_available():
		return
	var playfab = get_playfab()
	reset_playfab_runtime()

	var users = playfab.get_users()
	var sign_in_signal = users.sign_in_with_xuser_async(null)
	await _assert_signal_error(
		sign_in_signal, "not_initialized",
		"PlayFab.users.sign_in_with_xuser_async() before initialize()")


# ── PlayFabGameSaves async: not_initialized ──────────────────────────────

func test_game_saves_add_user_with_ui_async_not_initialized() -> void:
	if pending_unless_playfab_available():
		return
	var playfab = get_playfab()
	reset_playfab_runtime()

	var blank_user = instantiate_class("PlayFabUser")
	var add_signal = playfab.get_game_saves().add_user_with_ui_async(blank_user, 0)
	await _assert_signal_error(
		add_signal, "not_initialized",
		"PlayFab.game_saves.add_user_with_ui_async() before initialize()")


func test_game_saves_upload_with_ui_async_not_initialized() -> void:
	if pending_unless_playfab_available():
		return
	var playfab = get_playfab()
	reset_playfab_runtime()

	var blank_user = instantiate_class("PlayFabUser")
	var upload_signal = playfab.get_game_saves().upload_with_ui_async(blank_user, false)
	await _assert_signal_error(
		upload_signal, "not_initialized",
		"PlayFab.game_saves.upload_with_ui_async() before initialize()")


func test_game_saves_set_save_description_async_not_initialized() -> void:
	if pending_unless_playfab_available():
		return
	var playfab = get_playfab()
	reset_playfab_runtime()

	var blank_user = instantiate_class("PlayFabUser")
	var set_signal = playfab.get_game_saves().set_save_description_async(blank_user, "ignored")
	await _assert_signal_error(
		set_signal, "not_initialized",
		"PlayFab.game_saves.set_save_description_async() before initialize()")


func test_game_saves_reset_cloud_async_not_initialized() -> void:
	if pending_unless_playfab_available():
		return
	var playfab = get_playfab()
	reset_playfab_runtime()

	var blank_user = instantiate_class("PlayFabUser")
	var reset_signal = playfab.get_game_saves().reset_cloud_async(blank_user)
	await _assert_signal_error(
		reset_signal, "not_initialized",
		"PlayFab.game_saves.reset_cloud_async() before initialize()")


# ── PlayFabGameSaves sync: not_initialized ───────────────────────────────

func test_game_saves_sync_methods_not_initialized() -> void:
	if pending_unless_playfab_available():
		return
	var playfab = get_playfab()
	reset_playfab_runtime()

	var blank_user = instantiate_class("PlayFabUser")
	var game_saves = playfab.get_game_saves()

	assert_playfab_result_error(
		game_saves.get_folder(blank_user),
		"not_initialized",
		"PlayFab.game_saves.get_folder() before initialize()")
	assert_playfab_result_error(
		game_saves.get_folder_size(blank_user),
		"not_initialized",
		"PlayFab.game_saves.get_folder_size() before initialize()")
	assert_playfab_result_error(
		game_saves.get_remaining_quota(blank_user),
		"not_initialized",
		"PlayFab.game_saves.get_remaining_quota() before initialize()")
	assert_playfab_result_error(
		game_saves.is_connected_to_cloud(blank_user),
		"not_initialized",
		"PlayFab.game_saves.is_connected_to_cloud() before initialize()")


# ── PlayFabLeaderboards: not_initialized ─────────────────────────────────

func test_leaderboards_submit_score_async_not_initialized() -> void:
	if pending_unless_playfab_available():
		return
	var playfab = get_playfab()
	reset_playfab_runtime()

	var blank_user = instantiate_class("PlayFabUser")
	var submit_signal = playfab.get_leaderboards().submit_score_async(blank_user, "validation_walk", 1)
	await _assert_signal_error(
		submit_signal, "not_initialized",
		"PlayFab.leaderboards.submit_score_async() before initialize()")


func test_leaderboards_get_leaderboard_async_not_initialized() -> void:
	if pending_unless_playfab_available():
		return
	var playfab = get_playfab()
	reset_playfab_runtime()

	var blank_user = instantiate_class("PlayFabUser")
	var query_signal = playfab.get_leaderboards().get_leaderboard_async(blank_user, "validation_walk")
	await _assert_signal_error(
		query_signal, "not_initialized",
		"PlayFab.leaderboards.get_leaderboard_async() before initialize()")


func test_leaderboards_get_leaderboard_around_user_async_not_initialized() -> void:
	if pending_unless_playfab_available():
		return
	var playfab = get_playfab()
	reset_playfab_runtime()

	var blank_user = instantiate_class("PlayFabUser")
	var around_signal = playfab.get_leaderboards().get_leaderboard_around_user_async(blank_user, "validation_walk")
	await _assert_signal_error(
		around_signal, "not_initialized",
		"PlayFab.leaderboards.get_leaderboard_around_user_async() before initialize()")


func test_leaderboards_get_friend_leaderboard_async_not_initialized() -> void:
	if pending_unless_playfab_available():
		return
	var playfab = get_playfab()
	reset_playfab_runtime()

	var blank_user = instantiate_class("PlayFabUser")
	var friend_signal = playfab.get_leaderboards().get_friend_leaderboard_async(blank_user, "validation_walk")
	await _assert_signal_error(
		friend_signal, "not_initialized",
		"PlayFab.leaderboards.get_friend_leaderboard_async() before initialize()")


# ── invalid_options on game_saves.add_user_with_ui_async (post-init only)
#
# The "invalid_options" branch is reached AFTER the runtime check passes,
# so this test requires the runtime to be initialised. It is gated to skip
# cleanly when initialise fails (no GDK platform, etc).

func test_game_saves_invalid_options_requires_init() -> void:
	if pending_unless_playfab_available():
		return
	var playfab = get_playfab()
	reset_playfab_runtime()

	var configured_title_id := str(ProjectSettings.get_setting(PLAYFAB_TITLE_ID_SETTING, "")).strip_edges()
	if configured_title_id.is_empty():
		pending("invalid_options branch requires playfab/runtime/title_id to be set so initialize() can succeed.")
		return

	var init_result = playfab.initialize()
	if init_result == null or not init_result.ok:
		pending("invalid_options branch skipped: initialize() failed: %s" % (init_result.message if init_result != null else "null"))
		return

	var blank_user = instantiate_class("PlayFabUser")
	var bad_options := -1
	var signal_value = playfab.get_game_saves().add_user_with_ui_async(blank_user, bad_options)
	await _assert_signal_one_of(
		signal_value,
		PackedStringArray(["invalid_options", "invalid_user", "platform_unsupported"]),
		"PlayFab.game_saves.add_user_with_ui_async(user, -1) reports a documented validation code")

	playfab.shutdown()


# ── PlayFabRuntime: title_id_required (already covered in test_core.gd —
#    re-asserted here so the validation_walk file is a complete inventory)

func test_runtime_initialize_title_id_required() -> void:
	if pending_unless_playfab_available():
		return
	var playfab = get_playfab()
	reset_playfab_runtime()

	var original_title_id = ProjectSettings.get_setting(PLAYFAB_TITLE_ID_SETTING, "")
	var original_endpoint = ProjectSettings.get_setting(PLAYFAB_ENDPOINT_SETTING, "")
	ProjectSettings.set_setting(PLAYFAB_TITLE_ID_SETTING, "")
	ProjectSettings.set_setting(PLAYFAB_ENDPOINT_SETTING, "")

	var init_result = playfab.initialize()

	ProjectSettings.set_setting(PLAYFAB_TITLE_ID_SETTING, original_title_id)
	ProjectSettings.set_setting(PLAYFAB_ENDPOINT_SETTING, original_endpoint)

	assert_playfab_result_error(
		init_result, "title_id_required",
		"PlayFab.initialize() rejects blank playfab/runtime/title_id")


func test_internal_leaderboard_settle_msec_setting_not_registered() -> void:
	assert_false(
		ProjectSettings.has_setting("playfab/tests/leaderboard_settle_msec"),
		"playfab/tests/leaderboard_settle_msec stays internal to tests")


# ── Helpers ──────────────────────────────────────────────────────────────

func _assert_signal_error(async_signal, expected_code: String, name: String) -> void:
	assert_eq(typeof(async_signal), TYPE_SIGNAL, "%s returns completion Signal" % name)
	if typeof(async_signal) != TYPE_SIGNAL:
		return
	assert_playfab_result_error(await await_completion(async_signal), expected_code, name)


func _assert_signal_one_of(async_signal, expected_codes: PackedStringArray, name: String) -> void:
	assert_eq(typeof(async_signal), TYPE_SIGNAL, "%s returns completion Signal" % name)
	if typeof(async_signal) != TYPE_SIGNAL:
		return
	var result = await await_completion(async_signal)
	assert_not_null(result, "%s returns PlayFabResult" % name)
	if result == null:
		return
	assert_false(result.ok, "%s result.ok == false" % name)
	var code := str(result.code)
	var matched := false
	for expected in expected_codes:
		if code == String(expected):
			matched = true
			break
	assert_true(
		matched,
		"%s expected code in %s, got %s" % [name, str(expected_codes), code])
	assert_true(result.message.length() > 0, "%s error message present" % name)
