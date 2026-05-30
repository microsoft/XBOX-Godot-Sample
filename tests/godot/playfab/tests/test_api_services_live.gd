extends "res://addons/godot_gdk_tests/playfab_test_base.gd"
## Live smoke coverage for fixture-backed PlayFab service calls.

const LIVE_MARKER_KEY := "godot_public_gdk_ext_live_tests"
const _DEFAULT_OP_TIMEOUT_MSEC := 60000


func test_api_services_live_fixture_smoke() -> void:
	var session = await _begin_live_session()
	var playfab_user = session.get("playfab_user")
	if playfab_user == null:
		return

	var playfab = session["playfab"]
	var entity_key: Dictionary = playfab_user.entity_key
	assert_true(not str(entity_key.get("id", "")).is_empty(), "live PlayFabUser.entity_key.id is populated")
	assert_true(not str(entity_key.get("type", "")).is_empty(), "live PlayFabUser.entity_key.type is populated")

	var marker := await _load_live_marker(playfab, playfab_user)
	var api_fixtures: Dictionary = marker.get("api_services", {})
	if api_fixtures.is_empty():
		pending("Run tools\\configure_playfab_test_title.ps1 so the live title marker includes api_services fixtures.")
		playfab.shutdown()
		return

	var accounts_data = await _assert_api_ok(
		playfab.get_accounts(),
		"get_account_info_async",
		playfab_user,
		{},
		"PlayFab.accounts.get_account_info_async")
	if accounts_data is Dictionary:
		assert_true(accounts_data.has("account_info"), "account info response includes account_info")

	var title_data: Dictionary = api_fixtures.get("title_data", {})
	var title_key := str(title_data.get("key", ""))
	var title_value := str(title_data.get("value", ""))
	var title_data_result = await _assert_api_ok(
		playfab.get_title_data(),
		"get_title_data_async",
		playfab_user,
		{"keys": [LIVE_MARKER_KEY, title_key]},
		"PlayFab.title_data.get_title_data_async")
	_assert_data_value(title_data_result, LIVE_MARKER_KEY, "PlayFab title-data marker")
	_assert_data_value(title_data_result, title_key, "PlayFab API title-data fixture", title_value)

	var publisher_data: Dictionary = api_fixtures.get("publisher_data", {})
	var publisher_key := str(publisher_data.get("key", ""))
	var publisher_value := str(publisher_data.get("value", ""))
	var publisher_data_result = await _assert_api_ok(
		playfab.get_title_data(),
		"get_publisher_data_async",
		playfab_user,
		{"keys": [publisher_key]},
		"PlayFab.title_data.get_publisher_data_async")
	_assert_data_value(publisher_data_result, publisher_key, "PlayFab API publisher-data fixture", publisher_value)

	var time_result = await _assert_api_ok(
		playfab.get_title_data(),
		"get_time_async",
		playfab_user,
		null,
		"PlayFab.title_data.get_time_async")
	if time_result is Dictionary:
		assert_true(int(time_result.get("time", 0)) > 0, "server time response includes a positive time")

	await _assert_player_data_fixtures(playfab, playfab_user, api_fixtures)
	await _assert_api_read_services(playfab, playfab_user, api_fixtures, entity_key)
	await _assert_friend_fixture(playfab, playfab_user, api_fixtures)
	await _assert_cloud_script_reaches_service(playfab, playfab_user, api_fixtures)

	playfab.shutdown()


func _begin_live_session() -> Dictionary:
	var outcome := {
		"playfab_user": null,
		"playfab": null,
	}

	if pending_unless_live():
		return outcome
	if pending_unless_playfab_available():
		return outcome

	var playfab = get_playfab()
	outcome["playfab"] = playfab

	var configured_title_id := str(ProjectSettings.get_setting(PLAYFAB_TITLE_ID_SETTING, "")).strip_edges()
	if configured_title_id.is_empty():
		pending("Set ProjectSettings['playfab/runtime/title_id'] to exercise PlayFab service live coverage.")
		return outcome

	reset_playfab_runtime()
	var init_result = playfab.initialize()
	if init_result == null or not init_result.ok:
		pending("PlayFab.initialize() live setup skipped: %s" % (init_result.message if init_result != null else "null result"))
		return outcome

	var custom_id_session = await sign_in_with_configured_custom_id(playfab, "PlayFab services live test")
	if custom_id_session.get("playfab_user") == null:
		return outcome

	outcome["playfab_user"] = custom_id_session["playfab_user"]
	return outcome


func _load_live_marker(playfab: Object, playfab_user) -> Dictionary:
	var marker_result = await _assert_api_ok(
		playfab.get_title_data(),
		"get_title_data_async",
		playfab_user,
		{"keys": [LIVE_MARKER_KEY]},
		"PlayFab.title_data.get_title_data_async(marker)")
	if not (marker_result is Dictionary):
		return {}

	var data: Dictionary = marker_result.get("data", {})
	var marker_text := str(data.get(LIVE_MARKER_KEY, ""))
	assert_false(marker_text.is_empty(), "live title marker is present")
	if marker_text.is_empty():
		return {}

	var parsed = JSON.parse_string(marker_text)
	assert_true(parsed is Dictionary, "live title marker parses as Dictionary")
	if not (parsed is Dictionary):
		return {}
	return parsed


func _assert_player_data_fixtures(playfab: Object, playfab_user, api_fixtures: Dictionary) -> void:
	var player_data: Dictionary = api_fixtures.get("player_data", {})
	var data_key := str(player_data.get("key", ""))
	var read_only_key := str(player_data.get("read_only_key", ""))
	var publisher_key := str(player_data.get("publisher_data_key", ""))
	var expected_value := "configured:%s:%s" % [
		str(playfab_user.custom_id),
		str(ProjectSettings.get_setting(PLAYFAB_TITLE_ID_SETTING, "")).strip_edges(),
	]

	var user_data = await _assert_api_ok(
		playfab.get_player_data(),
		"get_user_data_async",
		playfab_user,
		{"keys": [data_key]},
		"PlayFab.player_data.get_user_data_async")
	_assert_data_record_value(user_data, data_key, "PlayFab API player-data fixture", expected_value)

	var read_only_data = await _assert_api_ok(
		playfab.get_player_data(),
		"get_user_read_only_data_async",
		playfab_user,
		{"keys": [read_only_key]},
		"PlayFab.player_data.get_user_read_only_data_async")
	_assert_data_record_value(read_only_data, read_only_key, "PlayFab API read-only player-data fixture", expected_value)

	var publisher_data = await _assert_api_ok(
		playfab.get_player_data(),
		"get_user_publisher_data_async",
		playfab_user,
		{"keys": [publisher_key]},
		"PlayFab.player_data.get_user_publisher_data_async")
	_assert_data_record_value(publisher_data, publisher_key, "PlayFab API publisher player-data fixture", expected_value)

	var properties_data = await _assert_api_ok(
		playfab.get_player_data(),
		"list_player_custom_properties_async",
		playfab_user,
		null,
		"PlayFab.player_data.list_player_custom_properties_async")
	if properties_data is Dictionary:
		assert_true(properties_data.has("properties"), "player custom properties response includes properties")


func _assert_api_read_services(playfab: Object, playfab_user, api_fixtures: Dictionary, entity_key: Dictionary) -> void:
	var catalog_data = await _assert_api_ok(
		playfab.get_catalog(),
		"search_items_async",
		playfab_user,
		{"count": 1},
		"PlayFab.catalog.search_items_async")
	if catalog_data is Dictionary:
		assert_true(catalog_data.has("items"), "catalog search response includes items")

	var entity_data = await _assert_api_ok(
		playfab.get_entity_data(),
		"get_objects_async",
		playfab_user,
		{"entity": entity_key},
		"PlayFab.entity_data.get_objects_async")
	if entity_data is Dictionary:
		assert_true(entity_data.has("objects"), "entity data response includes objects")

	var groups_data = await _assert_api_ok(
		playfab.get_groups(),
		"list_membership_async",
		playfab_user,
		{"entity": entity_key},
		"PlayFab.groups.list_membership_async")
	if groups_data is Dictionary:
		assert_true(groups_data.has("groups"), "group membership response includes groups")

	var inventory: Dictionary = api_fixtures.get("inventory", {})
	var inventory_data = await _assert_api_ok(
		playfab.get_inventory(),
		"get_inventory_items_async",
		playfab_user,
		{
			"entity": entity_key,
			"collection_id": str(inventory.get("collection_id", "default")),
			"count": 1,
		},
		"PlayFab.inventory.get_inventory_items_async")
	if inventory_data is Dictionary:
		assert_true(inventory_data.has("items"), "inventory response includes items")

	var localization_result = await _assert_service_response(
		playfab.get_localization(),
		"get_language_list_async",
		playfab_user,
		{},
		"PlayFab.localization.get_language_list_async")
	if localization_result != null and localization_result.ok and localization_result.data is Dictionary:
		assert_true(localization_result.data.has("language_list"), "localization response includes language_list")

	var statistic: Dictionary = api_fixtures.get("statistic", {})
	var statistics_data = await _assert_api_ok(
		playfab.get_statistics(),
		"get_statistics_async",
		playfab_user,
		{
			"entity": entity_key,
			"statistic_names": [str(statistic.get("name", ""))],
		},
		"PlayFab.statistics.get_statistics_async")
	if statistics_data is Dictionary:
		assert_true(statistics_data.has("statistics"), "statistics response includes statistics")


func _assert_friend_fixture(playfab: Object, playfab_user, api_fixtures: Dictionary) -> void:
	var accounts: Dictionary = api_fixtures.get("accounts", {})
	var friend_custom_id := str(accounts.get("friend_custom_id", ""))
	assert_false(friend_custom_id.is_empty(), "API friend custom ID is configured")
	if friend_custom_id.is_empty():
		return

	var friend_sign_in_signal = playfab.get_users().sign_in_with_custom_id_async(friend_custom_id, false)
	assert_eq(typeof(friend_sign_in_signal), TYPE_SIGNAL, "friend fixture custom-ID sign-in returns Signal")
	if typeof(friend_sign_in_signal) != TYPE_SIGNAL:
		return

	var friend_sign_in_result = await await_completion(friend_sign_in_signal, _DEFAULT_OP_TIMEOUT_MSEC)
	assert_playfab_result_ok(friend_sign_in_result, "friend fixture custom-ID sign-in")
	if friend_sign_in_result == null or not friend_sign_in_result.ok:
		return

	var friend_user = friend_sign_in_result.data
	var friend_account = await _assert_api_ok(
		playfab.get_accounts(),
		"get_account_info_async",
		friend_user,
		{},
		"PlayFab.accounts.get_account_info_async(friend)")
	if not (friend_account is Dictionary):
		return

	var account_info: Dictionary = friend_account.get("account_info", {})
	var friend_playfab_id := str(account_info.get("play_fab_id", ""))
	assert_false(friend_playfab_id.is_empty(), "friend fixture PlayFabId is discoverable")
	if friend_playfab_id.is_empty():
		return

	var add_friend_result = await _assert_service_response(
		playfab.get_friends(),
		"add_friend_async",
		playfab_user,
		{"friend_play_fab_id": friend_playfab_id},
		"PlayFab.friends.add_friend_async")
	if add_friend_result != null and add_friend_result.ok and add_friend_result.data is Dictionary:
		assert_true(add_friend_result.data.has("created"), "add friend response includes created")


func _assert_cloud_script_reaches_service(playfab: Object, playfab_user, api_fixtures: Dictionary) -> void:
	var cloud_script: Dictionary = api_fixtures.get("cloud_script", {})
	var function_name := str(cloud_script.get("function_name", "godot_services_smoke"))
	var result = await _assert_service_response(
		playfab.get_cloud_script(),
		"execute_cloud_script_async",
		playfab_user,
		{
			"function_name": function_name,
			"generate_play_stream_event": false,
		},
		"PlayFab.cloud_script.execute_cloud_script_async")
	assert_not_null(result, "PlayFab.cloud_script.execute_cloud_script_async returns PlayFabResult")
	if result == null:
		return
	assert_ne(result.code, "playfab_api_start_failed", "CloudScript request reached PlayFab service instead of failing at API start")
	assert_ne(result.code, "invalid_request", "CloudScript request marshalled before reaching PlayFab service")
	if result.ok:
		assert_true(result.data is Dictionary, "CloudScript service response includes data Dictionary")
		if result.data is Dictionary:
			var response: Dictionary = result.data
			assert_eq(str(response.get("function_name", "")), function_name, "CloudScript response echoes requested function_name")
			assert_true(response.has("error") or response.has("function_result") or response.has("logs"), "CloudScript response includes service payload fields")
	else:
		assert_true(str(result.message).length() > 0, "CloudScript service failure includes message")

func _assert_api_ok(service: Object, method_name: String, playfab_user, request, label: String):
	var result = await _call_api(service, method_name, playfab_user, request, label)
	if result == null:
		return null
	if not result.ok:
		assert_true(false, "%s live result failed: %s (%s)" % [label, result.message, result.code])
		return null
	assert_true(result.ok, "%s result.ok == true" % label)
	return result.data


func _assert_service_response(service: Object, method_name: String, playfab_user, request, label: String):
	var result = await _call_api(service, method_name, playfab_user, request, label)
	if result == null:
		return null
	if not result.ok:
		assert_ne(result.code, "playfab_api_start_failed", "%s started successfully" % label)
		assert_ne(result.code, "invalid_request", "%s request marshalled successfully" % label)
	return result


func _call_api(service: Object, method_name: String, playfab_user, request, label: String):
	assert_not_null(service, "%s service is available" % label)
	if service == null:
		return null
	assert_has_method_named(service, method_name)

	var completion_signal = service.call(method_name, playfab_user) if request == null else service.call(method_name, playfab_user, request)
	assert_eq(typeof(completion_signal), TYPE_SIGNAL, "%s returns Signal" % label)
	if typeof(completion_signal) != TYPE_SIGNAL:
		return null
	return await await_completion(completion_signal, _DEFAULT_OP_TIMEOUT_MSEC)


func _assert_data_value(result_data, key: String, label: String, expected_value: String = "") -> void:
	assert_false(key.is_empty(), "%s key is configured" % label)
	if key.is_empty() or not (result_data is Dictionary):
		return
	var data: Dictionary = result_data.get("data", {})
	assert_true(data.has(key), "%s is present" % label)
	if not expected_value.is_empty() and data.has(key):
		assert_eq(str(data[key]), expected_value, "%s value matches configured fixture" % label)


func _assert_data_record_value(result_data, key: String, label: String, expected_value: String) -> void:
	assert_false(key.is_empty(), "%s key is configured" % label)
	if key.is_empty() or not (result_data is Dictionary):
		return
	var data: Dictionary = result_data.get("data", {})
	assert_true(data.has(key), "%s is present" % label)
	if not data.has(key):
		return
	var record = data[key]
	if record is Dictionary and record.has("value"):
		assert_eq(str(record["value"]), expected_value, "%s value matches configured fixture" % label)
	else:
		assert_eq(str(record), expected_value, "%s value matches configured fixture" % label)
