extends "res://addons/godot_gdk_tests/playfab_test_base.gd"
## Live regression coverage for PlayFab Party network leave completion.
##
## Gated by `requires_live_write()` because it signs in to the configured
## PlayFab sandbox and creates a transient Party network before leaving it.

const _PARTY_INIT_TIMEOUT_MSEC := 60_000
const _PARTY_CREATE_TIMEOUT_MSEC := 90_000
const _PARTY_LEAVE_TIMEOUT_MSEC := 30_000
const _STATE_PUMP_FRAMES := 30


func test_party_create_and_leave_single_host_reports_success_without_resource_not_ready() -> void:
	if not requires_live_write():
		return
	if pending_unless_playfab_available():
		return

	var playfab = get_playfab()
	reset_playfab_runtime()
	var runtime_init = playfab.initialize()
	if runtime_init == null or not runtime_init.ok:
		pending("PlayFab.initialize() live setup skipped: %s" % (runtime_init.message if runtime_init != null else "null result"))
		return

	var sign_in = await sign_in_with_configured_custom_id(playfab, "PlayFab Party create/leave regression", _PARTY_INIT_TIMEOUT_MSEC)
	var playfab_user = sign_in.get("playfab_user")
	if playfab_user == null:
		playfab.shutdown()
		return

	var party = playfab.get_party()
	assert_object_is(party, "PlayFabParty", "PlayFab.get_party() returns PlayFabParty for live Party regression")
	if party == null:
		playfab.shutdown()
		return

	var config = instantiate_class("PlayFabPartyConfig")
	assert_object_is(config, "PlayFabPartyConfig", "PlayFabPartyConfig instantiable for live Party regression")
	if config == null:
		playfab.shutdown()
		return
	config.max_players = 4
	config.direct_peer_connectivity = get_class_constant("PlayFabParty", "DIRECT_PEER_CONNECTIVITY_ANY")
	config.enable_voice_chat = false
	config.enable_text_chat = false

	var init_result = await await_completion(party.initialize_async(config), _PARTY_INIT_TIMEOUT_MSEC)
	if init_result == null:
		pending("PlayFab.party.initialize_async timed out before Party leave regression check.")
		playfab.shutdown()
		return
	if not init_result.ok:
		pending("PlayFab.party.initialize_async failed before Party leave regression check: %s" % init_result.message)
		playfab.shutdown()
		return

	var create_result = await await_completion(party.create_and_join_network_async(playfab_user, config), _PARTY_CREATE_TIMEOUT_MSEC)
	if create_result == null:
		pending("PlayFab.party.create_and_join_network_async timed out before Party leave regression check.")
		await await_completion(party.shutdown_async(), _PARTY_INIT_TIMEOUT_MSEC)
		playfab.shutdown()
		return
	if not create_result.ok:
		pending("PlayFab.party.create_and_join_network_async failed before Party leave regression check: %s" % create_result.message)
		await await_completion(party.shutdown_async(), _PARTY_INIT_TIMEOUT_MSEC)
		playfab.shutdown()
		return

	var network = create_result.data
	assert_object_is(network, "PlayFabPartyNetwork", "create_and_join_network_async returns PlayFabPartyNetwork")
	if network == null:
		await await_completion(party.shutdown_async(), _PARTY_INIT_TIMEOUT_MSEC)
		playfab.shutdown()
		return

	var network_changes: Array = []
	var on_network_change = func(change): network_changes.append(change)
	network.state_changed.connect(on_network_change)

	var leave_result = await await_completion(network.leave_async(), _PARTY_LEAVE_TIMEOUT_MSEC)
	assert_true(leave_result != null and leave_result.ok,
			"network.leave_async succeeds (%s)" % (leave_result.message if leave_result != null else "null"))
	await advance_process_frames(_STATE_PUMP_FRAMES)

	for change in network_changes:
		var result = change.result
		if result == null:
			continue
		assert_ne(String(result.code), "party_resource_not_ready", "leave_async state changes do not emit party_resource_not_ready")
		assert_false(String(result.message).contains("PartyLeaveNetwork: operation succeeded"), "leave_async state changes do not report operation-succeeded as an error")

	if network.is_connected("state_changed", on_network_change):
		network.state_changed.disconnect(on_network_change)
	await await_completion(party.shutdown_async(), _PARTY_INIT_TIMEOUT_MSEC)
	playfab.shutdown()
