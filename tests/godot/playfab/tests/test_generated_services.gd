extends "res://addons/godot_gdk_tests/playfab_test_base.gd"
## Covers generated client-safe PlayFab service surfaces that hang off the
## root `PlayFab` singleton.


const GENERATED_MANIFEST_PATH := "res://addons/godot_playfab/playfab_api_manifest.json"
const EXPECTED_GENERATED_METHOD_COUNT := 139

const GENERATED_SERVICE_SPECS := [
	{
		"property": "accounts",
		"getter": "get_accounts",
		"class": "PlayFabAccounts",
		"method": "get_account_info_async",
	},
	{
		"property": "catalog",
		"getter": "get_catalog",
		"class": "PlayFabCatalog",
		"method": "get_catalog_config_async",
	},
	{
		"property": "cloud_script",
		"getter": "get_cloud_script",
		"class": "PlayFabCloudScript",
		"method": "execute_cloud_script_async",
	},
	{
		"property": "entity_data",
		"getter": "get_entity_data",
		"class": "PlayFabEntityData",
		"method": "get_objects_async",
	},
	{
		"property": "events",
		"getter": "get_events",
		"class": "PlayFabEvents",
		"method": "",
	},
	{
		"property": "experimentation",
		"getter": "get_experimentation",
		"class": "PlayFabExperimentation",
		"method": "get_treatment_assignment_async",
	},
	{
		"property": "friends",
		"getter": "get_friends",
		"class": "PlayFabFriends",
		"method": "add_friend_async",
	},
	{
		"property": "groups",
		"getter": "get_groups",
		"class": "PlayFabGroups",
		"method": "get_group_async",
	},
	{
		"property": "inventory",
		"getter": "get_inventory",
		"class": "PlayFabInventory",
		"method": "get_inventory_items_async",
	},
	{
		"property": "localization",
		"getter": "get_localization",
		"class": "PlayFabLocalization",
		"method": "get_language_list_async",
	},
	{
		"property": "player_data",
		"getter": "get_player_data",
		"class": "PlayFabPlayerData",
		"method": "get_user_data_async",
	},
	{
		"property": "statistics",
		"getter": "get_statistics",
		"class": "PlayFabStatistics",
		"method": "get_statistics_async",
	},
	{
		"property": "title_data",
		"getter": "get_title_data",
		"class": "PlayFabTitleData",
		"method": "get_title_data_async",
	},
]


func test_generated_service_accessors_and_manifest_method_contracts() -> void:
	if pending_unless_playfab_available():
		return
	var playfab = get_playfab()

	reset_playfab_runtime()
	var blank_user = instantiate_class("PlayFabUser")

	for spec in GENERATED_SERVICE_SPECS:
		var service = playfab.call(spec["getter"])
		assert_object_is(service, spec["class"], "PlayFab.%s returns %s" % [spec["property"], spec["class"]])
		assert_object_is(playfab.get(spec["property"]), spec["class"], "PlayFab.%s property returns %s" % [spec["property"], spec["class"]])

	var manifest := _load_generated_api_manifest()
	assert_eq(manifest.size(), EXPECTED_GENERATED_METHOD_COUNT, "generated PlayFab API manifest method count")
	if manifest.is_empty():
		return

	for entry in manifest:
		var property_name := str(entry.get("prop", ""))
		var generated_class_name := str(entry.get("class", ""))
		var method_name := str(entry.get("method", ""))
		assert_false(property_name.is_empty(), "generated manifest entry has service property")
		assert_false(generated_class_name.is_empty(), "generated manifest entry has class")
		assert_false(method_name.is_empty(), "generated manifest entry has method")
		if property_name.is_empty() or method_name.is_empty():
			continue

		var service = playfab.get(property_name)
		assert_object_is(service, generated_class_name, "PlayFab.%s property returns %s" % [property_name, generated_class_name])
		if service == null:
			continue

		assert_has_method_named(service, method_name)
		var completion_signal = _call_generated_method(service, method_name, blank_user, entry)
		await _assert_playfab_signal_result_error(
			completion_signal,
			"not_initialized",
			"PlayFab.%s.%s() before initialize()" % [property_name, method_name])


func _load_generated_api_manifest() -> Array:
	assert_true(FileAccess.file_exists(GENERATED_MANIFEST_PATH), "generated PlayFab API manifest is synced into the test host")
	if not FileAccess.file_exists(GENERATED_MANIFEST_PATH):
		return []

	var manifest_text := FileAccess.get_file_as_string(GENERATED_MANIFEST_PATH)
	var parsed = JSON.parse_string(manifest_text)
	assert_true(parsed is Array, "generated PlayFab API manifest parses as Array")
	if not (parsed is Array):
		return []
	return parsed


func _call_generated_method(service: Object, method_name: String, user, manifest_entry: Dictionary):
	if manifest_entry.has("request") and manifest_entry["request"] == null:
		return service.call(method_name, user)
	return service.call(method_name, user, {})


func _assert_playfab_signal_result_error(async_signal, expected_code: String, name: String) -> void:
	assert_eq(typeof(async_signal), TYPE_SIGNAL, "%s returns completion Signal" % name)
	if typeof(async_signal) != TYPE_SIGNAL:
		return
	assert_playfab_result_error(await await_completion(async_signal), expected_code, name)
