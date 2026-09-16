extends "res://addons/godot_gdk_tests/playfab_test_base.gd"
## Wave 3 migration of the previous `tests/suites/services_suite.gd`.
##
## Covers the `PlayFab.game_saves` and `PlayFab.leaderboards` service
## contracts: method exposure, exported constants, and the
## not-initialized failure path on async + sync entry points.


func test_game_saves_contract() -> void:
	if pending_unless_playfab_available():
		return
	var playfab = get_playfab()

	var game_saves = playfab.get_game_saves()
	assert_object_is(game_saves, "PlayFabGameSaves", "PlayFab.get_game_saves() returns PlayFabGameSaves")
	if game_saves == null:
		return

	for method_name in [
		"add_user_with_ui_async",
		"upload_with_ui_async",
		"set_save_description_async",
		"reset_cloud_async",
		"get_folder",
		"get_folder_size",
		"get_remaining_quota",
		"is_connected_to_cloud",
	]:
		assert_has_method_named(game_saves, method_name)

	assert_eq(get_class_constant("PlayFabGameSaves", "ADD_USER_OPTION_NONE"), 0, "PlayFabGameSaves.ADD_USER_OPTION_NONE == 0")

	reset_playfab_runtime()
	var blank_user = instantiate_class("PlayFabUser")

	var add_user_signal = game_saves.add_user_with_ui_async(blank_user)
	await _assert_playfab_signal_result_error(
		add_user_signal, "not_initialized", "PlayFab.game_saves.add_user_with_ui_async() before initialize()")

	var folder_result = game_saves.get_folder(blank_user)
	assert_playfab_result_error(folder_result, "not_initialized", "PlayFab.game_saves.get_folder() before initialize()")


func test_leaderboards_contract() -> void:
	if pending_unless_playfab_available():
		return
	var playfab = get_playfab()

	var leaderboards = playfab.get_leaderboards()
	assert_object_is(leaderboards, "PlayFabLeaderboards", "PlayFab.get_leaderboards() returns PlayFabLeaderboards")
	if leaderboards == null:
		return

	for method_name in [
		"submit_score_async",
		"get_leaderboard_async",
		"get_leaderboard_around_user_async",
		"get_friend_leaderboard_async",
		"get_friend_leaderboard_with_sources_async",
	]:
		assert_has_method_named(leaderboards, method_name)

	var expected_constants := {
		"FRIEND_SOURCE_NONE": 0,
		"FRIEND_SOURCE_STEAM": 1,
		"FRIEND_SOURCE_FACEBOOK": 2,
		"FRIEND_SOURCE_XBOX": 4,
		"FRIEND_SOURCE_PSN": 8,
		"FRIEND_SOURCE_ALL": 16,
	}
	var constant_names := ClassDB.class_get_integer_constant_list("PlayFabLeaderboards", true)
	for constant_name in expected_constants:
		var exists := constant_names.has(constant_name)
		assert_true(exists, "PlayFabLeaderboards.%s exists" % constant_name)
		if exists:
			assert_eq(
				get_class_constant("PlayFabLeaderboards", constant_name),
				expected_constants[constant_name],
				"PlayFabLeaderboards.%s has the expected value" % constant_name)

	assert_true(
		ClassDB.is_class_enum_bitfield("PlayFabLeaderboards", "FriendSources", true),
		"PlayFabLeaderboards.FriendSources is registered as a bitfield")
	var enum_constants := ClassDB.class_get_enum_constants(
		"PlayFabLeaderboards", "FriendSources", true)
	for constant_name in expected_constants:
		assert_true(
			enum_constants.has(constant_name),
			"PlayFabLeaderboards.%s belongs to FriendSources" % constant_name)

	var legacy_method := _class_method_info(
		"PlayFabLeaderboards", "get_friend_leaderboard_async")
	assert_false(legacy_method.is_empty(), "legacy friend leaderboard method metadata exists")
	if not legacy_method.is_empty():
		var legacy_args: Array = legacy_method.get("args", [])
		var legacy_defaults: Array = legacy_method.get("default_args", [])
		assert_eq(legacy_args.size(), 4, "legacy friend method has four arguments")
		if legacy_args.size() == 4:
			assert_eq(
				str(legacy_args[2].get("name", "")),
				"include_xbox_friends",
				"legacy friend bool argument name remains compatible")
			assert_eq(
				int(legacy_args[2].get("type", TYPE_NIL)),
				TYPE_BOOL,
				"legacy friend source argument remains bool")
		assert_eq(legacy_defaults, [true, -1], "legacy friend method defaults remain [true, -1]")

	var sources_method := _class_method_info(
		"PlayFabLeaderboards", "get_friend_leaderboard_with_sources_async")
	assert_false(sources_method.is_empty(), "source-selecting friend method metadata exists")
	if not sources_method.is_empty():
		var sources_args: Array = sources_method.get("args", [])
		var sources_defaults: Array = sources_method.get("default_args", [])
		assert_eq(sources_args.size(), 4, "source-selecting friend method has four arguments")
		if sources_args.size() == 4:
			var source_arg: Dictionary = sources_args[2]
			assert_eq(
				str(source_arg.get("name", "")),
				"friend_sources",
				"source-selecting method exposes friend_sources")
			assert_eq(
				int(source_arg.get("type", TYPE_NIL)),
				TYPE_INT,
				"friend_sources is represented as an integer bitfield")
			assert_eq(
				str(source_arg.get("class_name", "")),
				"PlayFabLeaderboards.FriendSources",
				"friend_sources is associated with the FriendSources enum")
			assert_true(
				(int(source_arg.get("usage", 0)) & PROPERTY_USAGE_CLASS_IS_BITFIELD) != 0,
				"friend_sources metadata marks the argument as a bitfield")
		assert_eq(sources_defaults, [-1], "source-selecting friend method defaults only version")

	reset_playfab_runtime()
	var blank_user = instantiate_class("PlayFabUser")

	var submit_signal = leaderboards.submit_score_async(blank_user, "contract_suite", 42)
	await _assert_playfab_signal_result_error(
		submit_signal, "not_initialized", "PlayFab.leaderboards.submit_score_async() before initialize()")

	var query_signal = leaderboards.get_leaderboard_async(blank_user, "contract_suite")
	await _assert_playfab_signal_result_error(
		query_signal, "not_initialized", "PlayFab.leaderboards.get_leaderboard_async() before initialize()")


func test_friend_leaderboard_request_shape() -> void:
	if pending_unless_playfab_available():
		return
	var leaderboards = get_playfab().get_leaderboards()
	if not leaderboards.has_method("_test_friend_leaderboard_request"):
		pending("PlayFab request-shape hooks are compiled out in this host.")
		return

	const SYNTHETIC_TOKEN := "synthetic-xbox-token"
	for friend_sources in range(17):
		var result = leaderboards._test_friend_leaderboard_request(
			friend_sources, SYNTHETIC_TOKEN)
		assert_playfab_result_ok(
			result, "friend request snapshot accepts source mask %d" % friend_sources)
		if result != null and result.ok:
			assert_true(
				result.data is Dictionary,
				"friend request snapshot returns Dictionary data for source mask %d" % friend_sources)
		if result == null or not result.ok or not (result.data is Dictionary):
			continue

		var snapshot: Dictionary = result.data
		var requires_xbox := friend_sources == 16 or (friend_sources & 4) != 0
		if friend_sources == 0:
			assert_null(
				snapshot.get("external_friend_sources"),
				"FRIEND_SOURCE_NONE omits externalFriendSources")
		else:
			assert_eq(
				int(snapshot.get("external_friend_sources", -1)),
				friend_sources,
				"friend request preserves source mask %d" % friend_sources)
		assert_eq(
			bool(snapshot.get("requires_xbox_token", false)),
			requires_xbox,
			"source mask %d has expected Xbox routing" % friend_sources)
		if requires_xbox:
			assert_eq(
				str(snapshot.get("xbox_token", "")),
				SYNTHETIC_TOKEN,
				"Xbox-containing source mask %d preserves the token" % friend_sources)
		else:
			assert_null(
				snapshot.get("xbox_token"),
				"non-Xbox source mask %d omits xboxToken" % friend_sources)
		assert_null(snapshot.get("version"), "default version is omitted for mask %d" % friend_sources)

	for version in [0, 7]:
		var version_result = leaderboards._test_friend_leaderboard_request(0, "", version)
		assert_playfab_result_ok(
			version_result, "friend request snapshot accepts version %d" % version)
		if version_result != null and version_result.ok:
			assert_true(
				version_result.data is Dictionary,
				"friend request snapshot returns Dictionary data for version %d" % version)
		if version_result != null and version_result.ok and version_result.data is Dictionary:
			assert_eq(
				int(version_result.data.get("version", -1)),
				version,
				"friend request preserves version %d" % version)

	for friend_sources in [4, 16]:
		var empty_token_result = leaderboards._test_friend_leaderboard_request(
			friend_sources, "")
		assert_playfab_result_ok(
			empty_token_result,
			"friend request snapshot accepts empty synthetic token for mask %d" % friend_sources)
		if empty_token_result != null and empty_token_result.ok:
			assert_true(
				empty_token_result.data is Dictionary,
				"empty-token friend request snapshot returns Dictionary data for mask %d" % friend_sources)
		if empty_token_result != null and empty_token_result.ok and empty_token_result.data is Dictionary:
			assert_null(
				empty_token_result.data.get("xbox_token"),
				"empty token is omitted for Xbox-containing mask %d" % friend_sources)

	assert_playfab_result_error(
		leaderboards._test_friend_leaderboard_request(17),
		"invalid_friend_sources",
		"request snapshot rejects FRIEND_SOURCE_ALL combined with ordinary flags")


func test_friend_token_context_preserves_source_masks() -> void:
	if pending_unless_playfab_available():
		return
	var leaderboards = get_playfab().get_leaderboards()
	if not leaderboards.has_method("_test_friend_token_context_sources"):
		pending("PlayFab token-context hooks are compiled out in this host.")
		return

	for friend_sources in range(17):
		assert_eq(
			int(leaderboards._test_friend_token_context_sources(friend_sources)),
			friend_sources,
			"friend token context preserves source mask %d" % friend_sources)


func _class_method_info(target_class: String, method_name: String) -> Dictionary:
	for method_info in ClassDB.class_get_method_list(target_class, true):
		if str(method_info.get("name", "")) == method_name:
			return method_info
	return {}


# Mirror of the previous `assert_signal_result_error`; routes through the
# PlayFab dual-pump `await_completion` and the PlayFabResult-labeled error
# assertion so failure messages point at the right type.
func _assert_playfab_signal_result_error(async_signal, expected_code: String, name: String) -> void:
	assert_eq(typeof(async_signal), TYPE_SIGNAL, "%s returns completion Signal" % name)
	if typeof(async_signal) != TYPE_SIGNAL:
		return
	assert_playfab_result_error(await await_completion(async_signal), expected_code, name)
