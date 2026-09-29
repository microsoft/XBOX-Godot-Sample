extends "res://addons/godot_gdk_tests/playfab_test_base.gd"
## Offline fault injection into the production Party dispatch/completion paths.
## Missing native hooks are a failure, not a skipped regression suite.

const DETAIL := 0x1234
const E_FAIL := 0x80004005
const E_INVALIDARG := 0x80070057
const HRESULT_MASK := 0xFFFFFFFF
const NETWORK_CHANGE_PEER_JOINED := 2
const NETWORK_CHANGE_ERROR := 6
var _party: Object
var _results: Array = []


func before_each() -> void:
	get_playfab()
	assert_true(ClassDB.class_exists("PlayFabParty"), "Native Party is required")
	_party = ClassDB.instantiate("PlayFabParty")
	assert_not_null(_party)
	assert_true(_party.has_method("_test_begin_establishment"), "Build with GODOT_PLAYFAB_TEST_HOOKS_ENABLED=ON")
	_results.clear()


func after_each() -> void:
	if _party != null:
		await await_completion(_party.shutdown_async())
	_party = null


func _begin(host: bool = true, chat: bool = false, errors: Dictionary = {}) -> Signal:
	_results.clear()
	var completion: Signal = _party._test_begin_establishment(host, chat, errors)
	completion.connect(func(result): _results.append(result))
	return completion


func _batch(stages: Array) -> void:
	_party._test_party_batch(stages)


func _done(stage: String, failed: bool = false) -> Dictionary:
	return {"stage": stage, "result": 3 if failed else 0, "error": DETAIL if failed else 0}


func _assert_drained(code: String = "") -> void:
	var snapshot: Dictionary = _party._test_party_snapshot()
	assert_eq(_results.size(), 1, "Exactly one result")
	assert_eq(snapshot.pending, 0, "All callback contexts drained")
	assert_eq(snapshot.networks, 0, "No partial native networks")
	assert_eq(_party.get_networks().size(), 0, "No failed session exposed")
	if _results.size() == 1:
		assert_false(_results[0].ok)
		if not code.is_empty():
			assert_eq(_results[0].code, code)


func _entity(entity_id: String, entity_type: String = "title_player_account") -> Dictionary:
	return {"id": entity_id, "type": entity_type}


func _handshake_request(
		endpoint: int,
		payload: Dictionary,
		endpoint_entity: Variant) -> Dictionary:
	return {
		"stage": "request",
		"endpoint": endpoint,
		"payload": payload,
		"endpoint_entity": endpoint_entity,
	}


func _network_events_of_kind(events: Array, kind: int) -> Array:
	var matching: Array = []
	for change in events:
		if int(change.kind) == kind:
			matching.append(change)
	return matching


func _assert_hresult(result: Variant, expected: int, label: String) -> void:
	assert_eq(
		int(result.hresult) & HRESULT_MASK,
		expected & HRESULT_MASK,
		"%s preserves HRESULT low bits" % label)


func _host_handshake_fixture() -> Dictionary:
	_begin(true)
	_batch([_done("create"), _done("connect"), _done("authenticate"), _done("endpoint")])
	assert_eq(_results.size(), 1)
	assert_true(_results[0].ok)
	var network: Object = _results[0].data
	var events: Array = []
	network.state_changed.connect(func(change): events.append(change))
	return {
		"network": network,
		"peer": network.get_local_peer(),
		"events": events,
	}


func test_create_sync_failure_has_one_original_result_and_no_connect() -> void:
	_begin(true, false, {"create": DETAIL})
	await get_tree().process_frame
	_assert_drained("party_network_create_failed")
	assert_eq(_party._test_party_snapshot().dispatches, ["create"])
	assert_eq(_results[0].data.party_error, DETAIL)
	assert_eq(_results[0].data.stage, "PartyManager::CreateNewNetwork")


func test_create_async_failure_never_dispatches_connect_and_can_retry() -> void:
	_begin()
	assert_eq(_party._test_party_snapshot().dispatches, ["create"], "Create and connect never overlap")
	_batch([_done("create", true)])
	_assert_drained("party_network_create_failed")
	assert_eq(_results[0].data.state_change_result, 3)
	assert_eq(_party._test_party_snapshot().dispatches, ["create"])
	_begin()
	_batch([_done("create"), _done("connect"), _done("authenticate"), _done("endpoint")])
	assert_eq(_results.size(), 1)
	assert_true(_results[0].ok, "Manual retry succeeds")
	assert_eq(_party.get_networks().size(), 1)
	assert_eq(_party._test_party_snapshot().pending, 0)


func test_create_success_then_connect_sync_failure() -> void:
	_begin(true, false, {"connect": DETAIL})
	_batch([_done("create")])
	_assert_drained("party_network_connect_failed")
	assert_eq(_party._test_party_snapshot().dispatches, ["create", "connect"])
	assert_eq(_results[0].data.party_error, DETAIL)


func test_host_and_guest_async_failure_matrix_waits_for_rollback() -> void:
	for host in [true, false]:
		for stage in ["connect", "authenticate", "chat", "endpoint"]:
			_begin(host, true)
			var steps: Array = []
			if host:
				steps.append(_done("create"))
			for preceding in ["connect", "authenticate", "chat", "endpoint"]:
				steps.append(_done(preceding, preceding == stage))
				if preceding == stage:
					break
			_batch(steps)
			assert_eq(_results.size(), 0, "%s failure waits for leave" % stage)
			assert_eq(_party._test_party_snapshot().pending, 1)
			assert_eq(_party.get_networks().size(), 0, "Partial sessions stay private")
			_batch([_done("leave")])
			_assert_drained()
			assert_eq(_results[0].data.party_error, DETAIL, "Original failure survives rollback")
			assert_eq(_results[0].data.state_change_result, 3)


func test_host_and_guest_sync_failure_matrix_waits_for_rollback() -> void:
	for host in [true, false]:
		for stage in ["authenticate", "chat", "endpoint"]:
			_begin(host, true, {stage: DETAIL})
			var steps: Array = []
			if host:
				steps.append(_done("create"))
			steps.append(_done("connect"))
			if stage != "authenticate":
				steps.append(_done("authenticate"))
			if stage == "endpoint":
				steps.append(_done("chat"))
			_batch(steps)
			assert_eq(_results.size(), 0)
			_batch([_done("leave")])
			_assert_drained()
			assert_eq(_results[0].data.party_error, DETAIL)


func test_handshake_enumeration_and_send_failures_roll_back() -> void:
	for stage in ["endpoints", "handshake"]:
		_begin(false, false, {stage: DETAIL})
		_batch([_done("connect"), _done("authenticate"), _done("endpoint")])
		assert_eq(_results.size(), 0)
		_batch([_done("leave")])
		_assert_drained("party_peer_not_connected")
		assert_eq(_results[0].data.party_error, DETAIL)


func test_destruction_before_completion_same_and_separate_batches() -> void:
	for separate in [false, true]:
		for stage in ["connect", "authenticate", "chat", "endpoint"]:
			_begin(false, true)
			for preceding in ["connect", "authenticate", "chat", "endpoint"]:
				if preceding == stage:
					break
				_batch([_done(preceding)])
			var dispatches: Array = _party._test_party_snapshot().dispatches
			if separate:
				_batch([{"stage": "destroy"}])
				assert_eq(_results.size(), 0, "Destruction does not free SDK-owned context")
				assert_eq(_party._test_party_snapshot().pending, 1)
				_batch([_done(stage)])
			else:
				_batch([{"stage": "destroy"}, _done(stage)])
			_assert_drained("party_resource_not_ready")
			assert_eq(_party._test_party_snapshot().dispatches, dispatches, "No continuation after destruction")


func test_destruction_during_rollback_retains_leave_context_across_batches() -> void:
	_begin(false)
	_batch([_done("connect", true), {"stage": "destroy"}])
	assert_eq(_results.size(), 0)
	assert_eq(_party._test_party_snapshot().pending, 1, "Leave identifier still owned by SDK")
	_batch([])
	assert_eq(_party._test_party_snapshot().pending, 1, "One deferred batch is not an ownership fence")
	_batch([_done("leave")])
	_assert_drained("party_network_connect_failed")


func test_shutdown_from_network_destroyed_retains_context_until_cleanup() -> void:
	for during_handshake in [false, true]:
		_begin(false)
		if during_handshake:
			_batch([_done("connect"), _done("authenticate"), _done("endpoint")])
		var network: Object = _party._test_party_snapshot().network
		var shutdown_results: Array = []
		var observed: Array = []
		network.state_changed.connect(func(change):
			if change.kind == 5 and observed.is_empty():
				_party.shutdown_async().connect(func(result): shutdown_results.append(result))
				var snapshot: Dictionary = _party._test_party_snapshot()
				observed.append({"pending": snapshot.pending, "shutting_down": snapshot.shutting_down})
				assert_eq(_results.size(), 0, "Destroyed callback cannot settle before cleanup")
				assert_eq(shutdown_results.size(), 0, "Shutdown stays deferred inside the SDK batch")
		)
		_batch([{"stage": "destroy"}])
		await get_tree().process_frame
		assert_eq(observed.size(), 1)
		assert_eq(observed[0].pending, 1, "Pending context remains owned during the notification")
		assert_true(observed[0].shutting_down)
		assert_eq(shutdown_results.size(), 1)
		assert_true(shutdown_results[0].ok)
		_assert_drained("party_resource_not_ready")


func test_leave_completion_then_destruction_settles_once() -> void:
	_begin(false)
	_batch([_done("connect", true), _done("leave"), {"stage": "destroy"}])
	_assert_drained("party_network_connect_failed")


func test_failed_rollback_dispatch_or_completion_requires_scoped_cleanup() -> void:
	for synchronous in [true, false]:
		_begin(false, false, {"leave": DETAIL} if synchronous else {})
		_batch([_done("connect", true)])
		if not synchronous:
			_batch([_done("leave", true)])
		assert_eq(_results.size(), 0, "Failed leave cannot claim cleanup")
		var result = await await_completion(_party.shutdown_async())
		assert_true(result.ok)
		_assert_drained("party_network_connect_failed")
		assert_false(_party.is_initialized())
		_begin()
		_batch([_done("create", true)])
		_assert_drained("party_network_create_failed")


func test_shutdown_from_completion_and_from_intermediate_state() -> void:
	_begin(false)
	var shutdown_results: Array = []
	var network: Object = _party._test_party_snapshot().network
	network.state_changed.connect(func(_change):
		if shutdown_results.is_empty():
			shutdown_results.append(null)
			_party.shutdown_async().connect(func(result): shutdown_results[0] = result)
	)
	_batch([_done("connect")])
	await get_tree().process_frame
	_assert_drained("cancelled")
	assert_true(shutdown_results[0].ok)
	assert_eq(_party._test_party_snapshot().dispatches, ["connect", "cleanup"])
	_begin()
	var completion: Signal = _party._test_enqueue_shutdown_pending()
	completion.connect(func(_result): _party.shutdown_async())
	await await_completion(_party.shutdown_async())
	_assert_drained("cancelled")


func test_shutdown_inside_successful_completion_does_not_continue_old_callbacks() -> void:
	var completion: Signal = _begin(false)
	var network: Object = _party._test_party_snapshot().network
	var shutdown_results: Array = []
	var events: Array = []
	network.state_changed.connect(func(change): events.append(change.kind))
	completion.connect(func(_result):
		_party.shutdown_async().connect(func(result): shutdown_results.append(result))
	)
	_batch([_done("connect"), _done("authenticate"), _done("endpoint")])
	events.clear()
	_batch([{"stage": "reply"}])
	await get_tree().process_frame
	assert_eq(_results.size(), 1)
	assert_true(_results[0].ok)
	assert_eq(shutdown_results.size(), 1)
	assert_true(shutdown_results[0].ok)
	assert_eq(_party._test_party_snapshot().pending, 0)
	assert_eq(_party._test_party_snapshot().networks, 0)
	assert_eq(network.get_state(), 5)
	assert_eq(events, [5], "Shutdown from completion suppresses old joined/connected notifications")


func test_shutdown_inside_failed_completion_suppresses_late_error() -> void:
	var completion: Signal = _begin(true, false, {"connect": DETAIL})
	var network: Object = _party._test_party_snapshot().network
	var events: Array = []
	network.state_changed.connect(func(change): events.append(change.kind))
	completion.connect(func(_result): _party.shutdown_async())
	_batch([_done("create")])
	await get_tree().process_frame
	_assert_drained("party_network_connect_failed")
	assert_eq(events, [], "No late error follows shutdown when connect never created a native network")


func test_shutdown_inside_peer_joined_suppresses_connected_notification() -> void:
	_begin(false)
	_batch([_done("connect"), _done("authenticate"), _done("endpoint")])
	var network: Object = _party._test_party_snapshot().network
	var events: Array = []
	network.state_changed.connect(func(change):
		events.append(change.kind)
		if change.kind == 2:
			_party.shutdown_async()
	)
	_batch([{"stage": "reply"}])
	await get_tree().process_frame
	assert_eq(events, [2, 5], "Recheck shutdown after each synchronous notification")
	assert_eq(_results.size(), 1)
	assert_true(_results[0].ok)


func test_state_pump_start_and_finish_failures_reset_terminally() -> void:
	for stage in ["start", "finish"]:
		_begin(false, false, {stage: DETAIL})
		var terminal: Array = []
		var network: Object = _party._test_party_snapshot().network
		network.state_changed.connect(func(change): terminal.append(change.kind))
		_batch([])
		await get_tree().process_frame
		_assert_drained("party_state_%s_failed" % stage)
		assert_has(terminal, 6, "Explicit error")
		assert_has(terminal, 5, "Explicit terminal destruction")
		assert_false(_party.is_initialized())


func test_guest_happy_path_and_handshake_destruction() -> void:
	_begin(false)
	_batch([_done("connect"), _done("authenticate"), _done("endpoint")])
	assert_eq(_results.size(), 0)
	_batch([{"stage": "destroy"}])
	_assert_drained("party_resource_not_ready")
	_begin(false)
	_batch([_done("connect"), _done("authenticate"), _done("endpoint"), {"stage": "reply"}])
	assert_eq(_results.size(), 1)
	assert_true(_results[0].ok)
	assert_eq(_results[0].data.get_local_peer().get_unique_id(), 2)
	assert_eq(_party._test_party_snapshot().pending, 0)


func test_cleanup_failure_keeps_contexts_owned_until_successful_retry() -> void:
	_begin(true, false, {"cleanup": DETAIL})
	var result = await await_completion(_party.shutdown_async())
	assert_false(result.ok)
	assert_eq(result.code, "party_cleanup_failed")
	assert_eq(_results.size(), 0, "No cancellation result before native ownership fence")
	assert_eq(_party._test_party_snapshot().pending, 1)
	assert_true(_party._test_party_snapshot().shutting_down)
	result = await await_completion(_party.create_and_join_network_async(null))
	assert_eq(result.code, "party_shutting_down")
	_party._test_set_dispatch_errors({})
	assert_true((await await_completion(_party.shutdown_async())).ok)
	_assert_drained("cancelled")


func test_late_endpoint_handshake_send_failure_uses_same_rollback() -> void:
	_begin(false)
	_batch([_done("connect"), _done("authenticate"), _done("endpoint")])
	_party._test_set_dispatch_errors({"handshake": DETAIL})
	_batch([{"stage": "endpoint_created"}])
	assert_eq(_results.size(), 0)
	_batch([_done("leave")])
	_assert_drained("party_peer_not_connected")
	assert_eq(_results[0].data.party_error, DETAIL)


func test_host_reply_failure_does_not_publish_a_phantom_peer() -> void:
	_begin(true, false, {"handshake_reply": DETAIL})
	_batch([_done("create"), _done("connect"), _done("authenticate"), _done("endpoint")])
	assert_true(_results[0].ok)
	var network: Object = _results[0].data
	var events: Array = []
	network.state_changed.connect(func(change): events.append(change.kind))
	_batch([{"stage": "request"}])
	assert_eq(_party._test_party_snapshot().dispatches.count("handshake_reply"), 1)
	assert_has(events, 6, "Handshake send error is surfaced")
	assert_does_not_have(events, 2, "Failed reply never advertises peer joined")
	assert_eq(network.get_local_peer().get_peers().size(), 0)


func test_handshake_matching_identity() -> void:
	var fixture := _host_handshake_fixture()
	_batch([{"stage": "request"}])

	var expected := _entity("offline-fixture")
	assert_eq(fixture.peer.get_peers(), [2])
	assert_eq(fixture.peer.get_peer_entity_key(2), expected)
	var handles: Dictionary = _party._test_native_handles(fixture.network, fixture.peer)
	assert_eq(handles.endpoint_slots, {2: 0})
	var dispatches: Array = _party._test_party_snapshot().dispatches
	assert_eq(dispatches.count("endpoint_entity_id"), 1)
	assert_eq(dispatches.count("endpoint_entity_type"), 1)
	assert_eq(dispatches.count("handshake_reply"), 1)
	assert_eq(_network_events_of_kind(fixture.events, NETWORK_CHANGE_PEER_JOINED).size(), 1)


func test_handshake_mismatched_identity() -> void:
	var fixture := _host_handshake_fixture()
	var endpoint_entity := _entity("offline-fixture")
	_batch([_handshake_request(0, endpoint_entity, endpoint_entity)])

	var before_handles: Dictionary = _party._test_native_handles(fixture.network, fixture.peer)
	var before_slots: Dictionary = before_handles.endpoint_slots.duplicate()
	var before_replies: int = _party._test_party_snapshot().dispatches.count("handshake_reply")
	var before_joined: int = _network_events_of_kind(fixture.events, NETWORK_CHANGE_PEER_JOINED).size()
	for payload in [
		_entity("different-id"),
		_entity("offline-fixture", "different_type"),
	]:
		var errors_before: int = _network_events_of_kind(fixture.events, NETWORK_CHANGE_ERROR).size()
		_batch([_handshake_request(0, payload, endpoint_entity)])
		var errors := _network_events_of_kind(fixture.events, NETWORK_CHANGE_ERROR)
		assert_eq(errors.size(), errors_before + 1)
		var error_change = errors[-1]
		assert_eq(error_change.peer_id, 0)
		assert_eq(error_change.result.code, "party_handshake_entity_mismatch")
		_assert_hresult(error_change.result, E_INVALIDARG, "Mismatched handshake identity")
		assert_eq(
			_party._test_native_handles(fixture.network, fixture.peer).endpoint_slots,
			before_slots)
		assert_eq(fixture.peer.get_peer_entity_key(2), endpoint_entity)
		assert_eq(_party._test_party_snapshot().dispatches.count("handshake_reply"), before_replies)
		assert_eq(
			_network_events_of_kind(fixture.events, NETWORK_CHANGE_PEER_JOINED).size(),
			before_joined)

	var second_entity := _entity("second-endpoint")
	_batch([_handshake_request(1, second_entity, second_entity)])
	var final_slots: Dictionary = _party._test_native_handles(fixture.network, fixture.peer).endpoint_slots
	assert_eq(final_slots, {2: 0, 3: 1}, "Rejected payloads do not consume peer ids")
	assert_eq(fixture.peer.get_peer_entity_key(3), second_entity)


func test_handshake_unavailable_identity() -> void:
	var fixture := _host_handshake_fixture()
	for case in [
		{"errors": {"endpoint_entity_id": DETAIL}, "endpoint_entity": _entity("offline-fixture")},
		{"errors": {"endpoint_entity_type": DETAIL}, "endpoint_entity": _entity("offline-fixture")},
		{"errors": {}, "endpoint_entity": null},
		{"errors": {}, "endpoint_entity": _entity("")},
		{"errors": {}, "endpoint_entity": _entity("offline-fixture", "")},
	]:
		_party._test_set_dispatch_errors(case.errors)
		var errors_before: int = _network_events_of_kind(fixture.events, NETWORK_CHANGE_ERROR).size()
		var replies_before: int = _party._test_party_snapshot().dispatches.count("handshake_reply")
		_batch([_handshake_request(0, _entity("offline-fixture"), case.endpoint_entity)])
		var errors := _network_events_of_kind(fixture.events, NETWORK_CHANGE_ERROR)
		assert_eq(errors.size(), errors_before + 1)
		var error_change = errors[-1]
		assert_eq(error_change.peer_id, 0)
		assert_eq(error_change.result.code, "party_handshake_endpoint_entity_unavailable")
		_assert_hresult(error_change.result, E_FAIL, "Unavailable endpoint identity")
		assert_eq(_party._test_party_snapshot().dispatches.count("handshake_reply"), replies_before)
		assert_eq(fixture.peer.get_peers(), [])
		assert_eq(_party._test_native_handles(fixture.network, fixture.peer).endpoint_slots, {})

	_party._test_set_dispatch_errors({})
	var entity := _entity("offline-fixture")
	_batch([_handshake_request(0, entity, entity)])
	assert_eq(fixture.peer.get_peers(), [2], "Rejected identities do not consume peer ids")


func test_handshake_reconnect_same_entity() -> void:
	var fixture := _host_handshake_fixture()
	var entity := _entity("offline-fixture")
	_batch([_handshake_request(0, entity, entity)])
	assert_eq(_network_events_of_kind(fixture.events, NETWORK_CHANGE_PEER_JOINED).size(), 1)

	_batch([_handshake_request(1, entity, entity)])
	assert_eq(fixture.peer.get_peers(), [2])
	assert_eq(fixture.peer.get_peer_entity_key(2), entity)
	assert_eq(_party._test_native_handles(fixture.network, fixture.peer).endpoint_slots, {2: 1})
	assert_eq(_party._test_party_snapshot().dispatches.count("handshake_reply"), 2)
	assert_eq(
		_network_events_of_kind(fixture.events, NETWORK_CHANGE_PEER_JOINED).size(),
		1,
		"Reconnect does not announce a second peer")


func test_handshake_competing_payloads() -> void:
	var entity_a := _entity("entity-a")
	var entity_b := _entity("entity-b")
	for order_index in range(2):
		if order_index > 0:
			assert_true((await await_completion(_party.shutdown_async())).ok)
			_party = ClassDB.instantiate("PlayFabParty")
			assert_not_null(_party)
			assert_true(_party.has_method("_test_begin_establishment"))
			_results.clear()

		var fixture := _host_handshake_fixture()
		var endpoint_order: Array = [1, 0] if order_index == 0 else [0, 1]
		for endpoint in endpoint_order:
			_batch([_handshake_request(
				endpoint,
				entity_a,
				entity_a if endpoint == 0 else entity_b)])

		assert_eq(fixture.peer.get_peers(), [2])
		assert_eq(fixture.peer.get_peer_entity_key(2), entity_a)
		assert_eq(_party._test_native_handles(fixture.network, fixture.peer).endpoint_slots, {2: 0})
		assert_eq(_party._test_party_snapshot().dispatches.count("handshake_reply"), 1)
		assert_eq(_network_events_of_kind(fixture.events, NETWORK_CHANGE_PEER_JOINED).size(), 1)
		var errors := _network_events_of_kind(fixture.events, NETWORK_CHANGE_ERROR)
		assert_eq(errors.size(), 1)
		assert_eq(errors[0].peer_id, 0)
		assert_eq(errors[0].result.code, "party_handshake_entity_mismatch")
		_assert_hresult(errors[0].result, E_INVALIDARG, "Competing handshake identity")


func test_retained_chat_control_is_invalidated_before_reset_notifications() -> void:
	_begin(false, true)
	var control: Object = _party.get_chat().get_chat_controls()[0]
	var network: Object = _party._test_party_snapshot().network
	var terminal_indicators: Array = []
	network.state_changed.connect(func(change):
		if change.kind == 5:
			terminal_indicators.append(control.get_local_chat_indicator())
	)
	await await_completion(_party.shutdown_async())
	_assert_drained("cancelled")
	assert_eq(terminal_indicators, [3], "Terminal listeners cannot poll freed native chat handles")
	assert_eq(_party.get_chat().get_chat_controls().size(), 0)
	assert_eq(control.get_audio_input_state(), 0)
	assert_eq(control.get_audio_output_state(), 0)
	assert_false(control.is_voice_enabled())
	var result = await await_completion(control.send_text_async([], "offline"))
	assert_false(result.ok)
	assert_eq(result.code, "party_resource_not_ready")
	_begin(false, true)
	assert_ne(_party.get_chat().get_chat_controls()[0], control, "Retry creates a new control")


func _shutdown_fixture(pending_connect: bool) -> Dictionary:
	var networks: Array = []
	var peers: Array = []
	_begin(true, true)
	for index in range(2):
		if index != 0:
			_party._test_begin_establishment(true, true, {}, true).connect(
				func(result): _results.append(result))
		_batch([_done("create"), _done("connect"), _done("authenticate"), _done("chat"), _done("endpoint")])
		_batch([{"stage": "request"}])
		var network: Object = _party._test_party_snapshot().network
		var peer: Object = network.get_local_peer()
		networks.append(network)
		peers.append(peer)
		var handles: Dictionary = _party._test_native_handles(network, peer)
		assert_true(handles.network and handles.user and handles.endpoint and handles.chat)
		assert_eq(handles.remote_endpoints, 1, "Production handshake publishes an endpoint")
		assert_eq(peer.get_connection_status(), MultiplayerPeer.CONNECTION_CONNECTED)
	assert_eq(_results.size(), 2)
	assert_true(_results[0].ok and _results[1].ok)
	_party._test_begin_establishment(not pending_connect, true, {}, true).connect(
		func(result): _results.append(result))
	if pending_connect:
		_batch([_done("connect"), _done("authenticate"), _done("chat")])
	var partial: Object = _party._test_party_snapshot().network
	networks.append(partial)
	peers.append(partial.get_local_peer())
	assert_eq(_party.get_networks().size(), 2, "Partial network remains private")
	assert_eq(_party._test_party_snapshot().pending, 1)
	assert_eq(_party._test_party_snapshot().networks, 3 if pending_connect else 2)
	return {"networks": networks, "peers": peers, "control": _party.get_chat().get_chat_controls()[0]}


func _assert_all_shutdown_handles_invalid(fixture: Dictionary) -> void:
	var snapshot: Dictionary = _party._test_party_snapshot()
	assert_false(snapshot.initialized)
	assert_true(snapshot.shutting_down, "Reentrant entry stays fenced through notifications")
	assert_eq(snapshot.local_users, 0)
	assert_eq(snapshot.local_chat_controls, 0)
	assert_eq(snapshot.native_contexts, 0)
	assert_eq(snapshot.networks, 0)
	for index in range(fixture.networks.size()):
		var network: Object = fixture.networks[index]
		var peer: Object = fixture.peers[index]
		var handles: Dictionary = _party._test_native_handles(network, peer)
		for key in ["network", "user", "endpoint", "chat", "owner"]:
			assert_false(handles[key], "Retained network %d: %s cleared before first callback" % [index, key])
		assert_eq(handles.remote_endpoints, 0)
		assert_eq(network.get_state(), 5)
		assert_null(network.get_local_peer())
		assert_null(network.get_local_chat_control())
		# A red run checks presence only; never dereference sentinel SDK handles.
		if not handles.network:
			assert_eq(network.get_statistics(), {})
		if peer != null:
			assert_eq(peer.get_connection_status(), MultiplayerPeer.CONNECTION_DISCONNECTED)
			assert_eq(peer.get_unique_id(), 0)
			if not handles.endpoint:
				assert_eq(peer.put_packet(PackedByteArray([1])), ERR_UNCONFIGURED)
	assert_eq(fixture.control.get_local_chat_indicator(), 3)
	assert_eq(_party.get_chat().get_chat_controls().size(), 0)


func test_shutdown_invalidates_all_networks_before_peer_and_terminal_callbacks() -> void:
	for pending_connect in [false, true]:
		var fixture: Dictionary = _shutdown_fixture(pending_connect)
		var peer_events: Array = []
		var terminal_events: Array = []
		var release_results: Array = []
		var nested_results: Array = []
		fixture.peers[0].connection_state_changed.connect(func(_status):
			peer_events.append(true)
			_assert_all_shutdown_handles_invalid(fixture)
			_party.release_local_user_async(fixture.networks[0].get_local_user()).connect(
				func(result): release_results.append(result))
			_party.shutdown_async().connect(func(result): nested_results.append(result))
		)
		for network in fixture.networks:
			network.state_changed.connect(func(change):
				if change.kind == 5:
					terminal_events.append(change.network)
					_assert_all_shutdown_handles_invalid(fixture)
			)
		assert_true((await await_completion(_party.shutdown_async())).ok)
		assert_eq(peer_events.size(), 1)
		assert_eq(terminal_events.size(), 3, "Established and pending-only networks each terminate once")
		assert_eq(release_results.size(), 1)
		if release_results.size() == 1:
			assert_eq(release_results[0].code, "party_shutting_down")
		assert_eq(nested_results.size(), 1)
		if nested_results.size() == 1:
			assert_true(nested_results[0].ok)
		assert_eq(_results.size(), 3)
		assert_eq(_results[2].code, "cancelled")
		assert_eq(_party._test_party_snapshot().pending, 0)
		assert_eq(_party._test_party_snapshot().dispatches.count("cleanup"), 1)
		assert_true((await await_completion(_party.release_local_user_async(fixture.networks[0].get_local_user()))).ok)
		_begin()
		_batch([_done("create"), _done("connect"), _done("authenticate"), _done("endpoint")])
		assert_true(_results[0].ok, "Manual retry after notification reentrancy")
		assert_true((await await_completion(_party.shutdown_async())).ok)


func test_failed_cleanup_keeps_all_handles_until_successful_retry() -> void:
	var fixture: Dictionary = _shutdown_fixture(true)
	var events: Array = []
	for network in fixture.networks:
		network.state_changed.connect(func(change): events.append(change.kind))
	for peer in fixture.peers:
		if peer != null:
			peer.connection_state_changed.connect(func(status): events.append(status))
	var before: Array = []
	for index in range(fixture.networks.size()):
		before.append(_party._test_native_handles(fixture.networks[index], fixture.peers[index]))
	_party._test_set_dispatch_errors({"cleanup": DETAIL})
	var result = await await_completion(_party.shutdown_async())
	assert_false(result.ok)
	assert_eq(result.code, "party_cleanup_failed")
	assert_eq(events, [], "Failed Cleanup emits no false peer/terminal notifications")
	assert_eq(_results.size(), 2, "Unexposed pending operation is still SDK owned")
	var snapshot: Dictionary = _party._test_party_snapshot()
	assert_true(snapshot.initialized)
	assert_true(snapshot.shutting_down)
	assert_eq(snapshot.local_users, 1)
	assert_eq(snapshot.local_chat_controls, 1)
	assert_eq(snapshot.native_contexts, 1)
	assert_eq(snapshot.networks, 3)
	assert_eq(_party.get_chat().get_chat_controls(), [fixture.control])
	for index in range(fixture.networks.size()):
		assert_eq(_party._test_native_handles(fixture.networks[index], fixture.peers[index]), before[index])
	for peer in fixture.peers:
		if peer != null:
			assert_eq(peer.get_connection_status(), MultiplayerPeer.CONNECTION_CONNECTED)
	result = await await_completion(_party.release_local_user_async(fixture.networks[0].get_local_user()))
	assert_eq(result.code, "party_shutting_down", "Release must not mutate cleanup-owned registries")
	_party._test_set_dispatch_errors({})
	assert_true((await await_completion(_party.shutdown_async())).ok)
	assert_false(_party.is_initialized())
	assert_eq(events.count(5), 3)
	assert_eq(_results.size(), 3)
	assert_eq(_results[2].code, "cancelled")
	assert_eq(_party._test_party_snapshot().pending, 0)
