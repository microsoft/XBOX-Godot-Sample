extends "res://addons/godot_gdk_tests/playfab_test_base.gd"
## Covers client-safe PlayFab service surfaces that hang off the
## root `PlayFab` singleton.


const EXPECTED_API_METHOD_COUNT := 139

const API_SERVICE_SPECS := [
	{
		"property": "accounts",
		"getter": "get_accounts",
		"class": "PlayFabAccounts",
	},
	{
		"property": "catalog",
		"getter": "get_catalog",
		"class": "PlayFabCatalog",
	},
	{
		"property": "cloud_script",
		"getter": "get_cloud_script",
		"class": "PlayFabCloudScript",
	},
	{
		"property": "entity_data",
		"getter": "get_entity_data",
		"class": "PlayFabEntityData",
	},
	{
		"property": "events",
		"getter": "get_events",
		"class": "PlayFabEvents",
	},
	{
		"property": "experimentation",
		"getter": "get_experimentation",
		"class": "PlayFabExperimentation",
	},
	{
		"property": "friends",
		"getter": "get_friends",
		"class": "PlayFabFriends",
	},
	{
		"property": "groups",
		"getter": "get_groups",
		"class": "PlayFabGroups",
	},
	{
		"property": "inventory",
		"getter": "get_inventory",
		"class": "PlayFabInventory",
	},
	{
		"property": "localization",
		"getter": "get_localization",
		"class": "PlayFabLocalization",
	},
	{
		"property": "player_data",
		"getter": "get_player_data",
		"class": "PlayFabPlayerData",
	},
	{
		"property": "statistics",
		"getter": "get_statistics",
		"class": "PlayFabStatistics",
	},
	{
		"property": "title_data",
		"getter": "get_title_data",
		"class": "PlayFabTitleData",
	},
]

const API_METHOD_SPECS := [
	['accounts', 'PlayFabAccounts', 'add_or_update_contact_email_async', true],
	['accounts', 'PlayFabAccounts', 'get_account_info_async', true],
	['accounts', 'PlayFabAccounts', 'get_player_combined_info_async', true],
	['accounts', 'PlayFabAccounts', 'get_player_profile_async', true],
	['accounts', 'PlayFabAccounts', 'get_play_fab_ids_from_battle_net_account_ids_async', true],
	['accounts', 'PlayFabAccounts', 'get_play_fab_ids_from_google_ids_async', true],
	['accounts', 'PlayFabAccounts', 'get_play_fab_ids_from_kongregate_ids_async', true],
	['accounts', 'PlayFabAccounts', 'get_play_fab_ids_from_steam_ids_async', true],
	['accounts', 'PlayFabAccounts', 'get_play_fab_ids_from_steam_names_async', true],
	['accounts', 'PlayFabAccounts', 'get_play_fab_ids_from_xbox_live_ids_async', true],
	['accounts', 'PlayFabAccounts', 'link_battle_net_account_async', true],
	['accounts', 'PlayFabAccounts', 'link_custom_id_async', true],
	['accounts', 'PlayFabAccounts', 'link_open_id_connect_async', true],
	['accounts', 'PlayFabAccounts', 'link_steam_account_async', true],
	['accounts', 'PlayFabAccounts', 'link_xbox_account_async', true],
	['accounts', 'PlayFabAccounts', 'remove_contact_email_async', true],
	['accounts', 'PlayFabAccounts', 'report_player_async', true],
	['accounts', 'PlayFabAccounts', 'unlink_battle_net_account_async', true],
	['accounts', 'PlayFabAccounts', 'unlink_custom_id_async', true],
	['accounts', 'PlayFabAccounts', 'unlink_open_id_connect_async', true],
	['accounts', 'PlayFabAccounts', 'unlink_steam_account_async', true],
	['accounts', 'PlayFabAccounts', 'unlink_xbox_account_async', true],
	['accounts', 'PlayFabAccounts', 'update_avatar_url_async', true],
	['accounts', 'PlayFabAccounts', 'update_user_title_display_name_async', true],
	['accounts', 'PlayFabAccounts', 'get_title_players_from_xbox_live_ids_async', true],
	['accounts', 'PlayFabAccounts', 'set_display_name_async', true],
	['accounts', 'PlayFabAccounts', 'get_profile_async', true],
	['accounts', 'PlayFabAccounts', 'get_profiles_async', true],
	['accounts', 'PlayFabAccounts', 'get_title_players_from_master_player_account_ids_async', true],
	['accounts', 'PlayFabAccounts', 'set_profile_language_async', true],
	['accounts', 'PlayFabAccounts', 'set_profile_policy_async', true],
	['catalog', 'PlayFabCatalog', 'create_draft_item_async', true],
	['catalog', 'PlayFabCatalog', 'create_upload_urls_async', true],
	['catalog', 'PlayFabCatalog', 'delete_entity_item_reviews_async', true],
	['catalog', 'PlayFabCatalog', 'delete_item_async', true],
	['catalog', 'PlayFabCatalog', 'get_catalog_config_async', true],
	['catalog', 'PlayFabCatalog', 'get_draft_item_async', true],
	['catalog', 'PlayFabCatalog', 'get_draft_items_async', true],
	['catalog', 'PlayFabCatalog', 'get_entity_draft_items_async', true],
	['catalog', 'PlayFabCatalog', 'get_entity_item_review_async', true],
	['catalog', 'PlayFabCatalog', 'get_item_async', true],
	['catalog', 'PlayFabCatalog', 'get_item_containers_async', true],
	['catalog', 'PlayFabCatalog', 'get_item_moderation_state_async', true],
	['catalog', 'PlayFabCatalog', 'get_item_publish_status_async', true],
	['catalog', 'PlayFabCatalog', 'get_item_reviews_async', true],
	['catalog', 'PlayFabCatalog', 'get_item_review_summary_async', true],
	['catalog', 'PlayFabCatalog', 'get_items_async', true],
	['catalog', 'PlayFabCatalog', 'publish_draft_item_async', true],
	['catalog', 'PlayFabCatalog', 'report_item_async', true],
	['catalog', 'PlayFabCatalog', 'report_item_review_async', true],
	['catalog', 'PlayFabCatalog', 'review_item_async', true],
	['catalog', 'PlayFabCatalog', 'search_items_async', true],
	['catalog', 'PlayFabCatalog', 'set_item_moderation_state_async', true],
	['catalog', 'PlayFabCatalog', 'submit_item_review_vote_async', true],
	['catalog', 'PlayFabCatalog', 'takedown_item_reviews_async', true],
	['catalog', 'PlayFabCatalog', 'update_catalog_config_async', true],
	['catalog', 'PlayFabCatalog', 'update_draft_item_async', true],
	['cloud_script', 'PlayFabCloudScript', 'execute_cloud_script_async', true],
	['cloud_script', 'PlayFabCloudScript', 'execute_entity_cloud_script_async', true],
	['cloud_script', 'PlayFabCloudScript', 'execute_function_async', true],
	['entity_data', 'PlayFabEntityData', 'abort_file_uploads_async', true],
	['entity_data', 'PlayFabEntityData', 'delete_files_async', true],
	['entity_data', 'PlayFabEntityData', 'finalize_file_uploads_async', true],
	['entity_data', 'PlayFabEntityData', 'get_files_async', true],
	['entity_data', 'PlayFabEntityData', 'get_objects_async', true],
	['entity_data', 'PlayFabEntityData', 'initiate_file_uploads_async', true],
	['entity_data', 'PlayFabEntityData', 'set_objects_async', true],
	['experimentation', 'PlayFabExperimentation', 'get_treatment_assignment_async', true],
	['friends', 'PlayFabFriends', 'add_friend_async', true],
	['friends', 'PlayFabFriends', 'get_friends_list_async', true],
	['friends', 'PlayFabFriends', 'remove_friend_async', true],
	['friends', 'PlayFabFriends', 'set_friend_tags_async', true],
	['groups', 'PlayFabGroups', 'accept_group_application_async', true],
	['groups', 'PlayFabGroups', 'accept_group_invitation_async', true],
	['groups', 'PlayFabGroups', 'add_members_async', true],
	['groups', 'PlayFabGroups', 'apply_to_group_async', true],
	['groups', 'PlayFabGroups', 'block_entity_async', true],
	['groups', 'PlayFabGroups', 'change_member_role_async', true],
	['groups', 'PlayFabGroups', 'create_group_async', true],
	['groups', 'PlayFabGroups', 'create_role_async', true],
	['groups', 'PlayFabGroups', 'delete_group_async', true],
	['groups', 'PlayFabGroups', 'delete_role_async', true],
	['groups', 'PlayFabGroups', 'get_group_async', true],
	['groups', 'PlayFabGroups', 'invite_to_group_async', true],
	['groups', 'PlayFabGroups', 'is_member_async', true],
	['groups', 'PlayFabGroups', 'list_group_applications_async', true],
	['groups', 'PlayFabGroups', 'list_group_blocks_async', true],
	['groups', 'PlayFabGroups', 'list_group_invitations_async', true],
	['groups', 'PlayFabGroups', 'list_group_members_async', true],
	['groups', 'PlayFabGroups', 'list_membership_async', true],
	['groups', 'PlayFabGroups', 'list_membership_opportunities_async', true],
	['groups', 'PlayFabGroups', 'remove_group_application_async', true],
	['groups', 'PlayFabGroups', 'remove_group_invitation_async', true],
	['groups', 'PlayFabGroups', 'remove_members_async', true],
	['groups', 'PlayFabGroups', 'unblock_entity_async', true],
	['groups', 'PlayFabGroups', 'update_group_async', true],
	['groups', 'PlayFabGroups', 'update_role_async', true],
	['inventory', 'PlayFabInventory', 'add_inventory_items_async', true],
	['inventory', 'PlayFabInventory', 'delete_inventory_collection_async', true],
	['inventory', 'PlayFabInventory', 'delete_inventory_items_async', true],
	['inventory', 'PlayFabInventory', 'execute_inventory_operations_async', true],
	['inventory', 'PlayFabInventory', 'execute_transfer_operations_async', true],
	['inventory', 'PlayFabInventory', 'get_inventory_collection_ids_async', true],
	['inventory', 'PlayFabInventory', 'get_inventory_items_async', true],
	['inventory', 'PlayFabInventory', 'get_inventory_operation_status_async', true],
	['inventory', 'PlayFabInventory', 'get_transaction_history_async', true],
	['inventory', 'PlayFabInventory', 'purchase_inventory_items_async', true],
	['inventory', 'PlayFabInventory', 'redeem_google_play_inventory_items_async', true],
	['inventory', 'PlayFabInventory', 'redeem_microsoft_store_inventory_items_async', true],
	['inventory', 'PlayFabInventory', 'redeem_play_station_store_inventory_items_async', true],
	['inventory', 'PlayFabInventory', 'redeem_steam_inventory_items_async', true],
	['inventory', 'PlayFabInventory', 'subtract_inventory_items_async', true],
	['inventory', 'PlayFabInventory', 'transfer_inventory_items_async', true],
	['inventory', 'PlayFabInventory', 'update_inventory_items_async', true],
	['localization', 'PlayFabLocalization', 'get_language_list_async', true],
	['player_data', 'PlayFabPlayerData', 'delete_player_custom_properties_async', true],
	['player_data', 'PlayFabPlayerData', 'get_player_custom_property_async', true],
	['player_data', 'PlayFabPlayerData', 'get_user_data_async', true],
	['player_data', 'PlayFabPlayerData', 'get_user_publisher_data_async', true],
	['player_data', 'PlayFabPlayerData', 'get_user_publisher_read_only_data_async', true],
	['player_data', 'PlayFabPlayerData', 'get_user_read_only_data_async', true],
	['player_data', 'PlayFabPlayerData', 'list_player_custom_properties_async', false],
	['player_data', 'PlayFabPlayerData', 'update_player_custom_properties_async', true],
	['player_data', 'PlayFabPlayerData', 'update_user_data_async', true],
	['player_data', 'PlayFabPlayerData', 'update_user_publisher_data_async', true],
	['statistics', 'PlayFabStatistics', 'create_statistic_definition_async', true],
	['statistics', 'PlayFabStatistics', 'delete_statistic_definition_async', true],
	['statistics', 'PlayFabStatistics', 'delete_statistics_async', true],
	['statistics', 'PlayFabStatistics', 'get_statistic_definition_async', true],
	['statistics', 'PlayFabStatistics', 'get_statistics_async', true],
	['statistics', 'PlayFabStatistics', 'get_statistics_for_entities_async', true],
	['statistics', 'PlayFabStatistics', 'increment_statistic_version_async', true],
	['statistics', 'PlayFabStatistics', 'list_statistic_definitions_async', true],
	['statistics', 'PlayFabStatistics', 'update_statistic_definition_async', true],
	['statistics', 'PlayFabStatistics', 'update_statistics_async', true],
	['title_data', 'PlayFabTitleData', 'get_publisher_data_async', true],
	['title_data', 'PlayFabTitleData', 'get_time_async', false],
	['title_data', 'PlayFabTitleData', 'get_title_data_async', true],
	['title_data', 'PlayFabTitleData', 'get_title_news_async', true],
]


func test_api_service_accessors_and_method_contracts() -> void:
	if pending_unless_playfab_available():
		return
	var playfab = get_playfab()

	reset_playfab_runtime()
	var blank_user = instantiate_class("PlayFabUser")

	for spec in API_SERVICE_SPECS:
		var service = playfab.call(spec["getter"])
		assert_object_is(service, spec["class"], "PlayFab.%s returns %s" % [spec["property"], spec["class"]])
		assert_object_is(playfab.get(spec["property"]), spec["class"], "PlayFab.%s property returns %s" % [spec["property"], spec["class"]])

	assert_eq(API_METHOD_SPECS.size(), EXPECTED_API_METHOD_COUNT, "PlayFab API method count")
	for spec in API_METHOD_SPECS:
		var property_name := str(spec[0])
		var api_class_name := str(spec[1])
		var method_name := str(spec[2])
		var has_request := bool(spec[3])
		var service = playfab.get(property_name)
		assert_object_is(service, api_class_name, "PlayFab.%s property returns %s" % [property_name, api_class_name])
		if service == null:
			continue

		assert_has_method_named(service, method_name)
		var completion_signal = _call_api_method(service, method_name, blank_user, has_request)
		await _assert_playfab_signal_result_error(
			completion_signal,
			"not_initialized",
			"PlayFab.%s.%s() before initialize()" % [property_name, method_name])


func _call_api_method(service: Object, method_name: String, user, has_request: bool):
	if has_request:
		return service.call(method_name, user, {})
	return service.call(method_name, user)


func _assert_playfab_signal_result_error(async_signal, expected_code: String, name: String) -> void:
	assert_eq(typeof(async_signal), TYPE_SIGNAL, "%s returns completion Signal" % name)
	if typeof(async_signal) != TYPE_SIGNAL:
		return
	assert_playfab_result_error(await await_completion(async_signal), expected_code, name)
