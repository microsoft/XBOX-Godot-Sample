## PlayFabPartyOps — handle-based Party command implementations.
##
## Mirrors the patterns from sample/tutorial_app/autoload/party.gd
## (initialize → create_and_join_network_async → leave_async → send_text_async)
## but tracks networks by orchestrator-supplied handle instead of a singleton.
## Event frames expose Party network, peer, RPC-packet, and chat-control changes
## so mp_orchestrator scenarios can validate the C1 P0/P1 matrix without the
## retired legacy worker harness.
extends RefCounted

const PlayFabRuntime := preload("res://scripts/playfab_runtime.gd")

const DEFAULT_HANDLE := "main"
const DEFAULT_AWAIT_TIMEOUT_MS := 60_000
const LEAVE_TIMEOUT_MS := 30_000
const SEND_TEXT_TIMEOUT_MS := 10_000
const JOIN_RETRY_ATTEMPTS := 3
const JOIN_RETRY_BACKOFF_MS := 1_500
const CONNECTED_STATE := 3
const DISCONNECTED_STATE := 5
const FAILED_STATE := 6

var _runtime: PlayFabRuntime = null
var _party_initialized: bool = false
var _networks: Dictionary = {}  # handle (String) -> PlayFabPartyNetwork (Object)
var _peer_sets: Dictionary = {}  # handle (String) -> Array[int]
var _pending_events: Array[Dictionary] = []


func bind(runtime: PlayFabRuntime) -> void:
	_runtime = runtime


func handles() -> Array:
	return _networks.keys()


func has_network(handle: String) -> bool:
	return _networks.has(handle) and _networks[handle] != null


# ---------------------------------------------------------------------------
# Commands
# ---------------------------------------------------------------------------

func initialize_party(params: Dictionary) -> Dictionary:
	var pf_err: Dictionary = _require_playfab("party_initialize")
	if not pf_err.is_empty():
		return pf_err
	var party: Object = _runtime.get_playfab().get_party()
	if party == null:
		return _err("party_unavailable", "PlayFab.get_party() returned null")
	if party.is_initialized():
		_party_initialized = true
		return _ok({ "already_initialized": true })

	var config: Object = _instantiate("PlayFabPartyConfig")
	if config == null:
		return _err("class_unavailable", "PlayFabPartyConfig not registered in ClassDB")
	config.max_players = int(params.get("max_players", 8))
	var connectivity: int = int(params.get("direct_peer_connectivity",
		ClassDB.class_get_integer_constant("PlayFabParty", "DIRECT_PEER_CONNECTIVITY_ANY")))
	config.direct_peer_connectivity = connectivity
	config.enable_voice_chat = bool(params.get("enable_voice_chat", false))
	config.enable_text_chat = bool(params.get("enable_text_chat", true))
	config.enable_transcription = bool(params.get("enable_transcription", false))

	var result: Variant = await _runtime.await_completion(party.initialize_async(config), DEFAULT_AWAIT_TIMEOUT_MS)
	if result == null or not bool(result.ok):
		return _err_from_result(result, "PlayFabParty.initialize_async")
	_party_initialized = true
	return _ok({ "max_players": int(config.max_players), "direct_peer_connectivity": connectivity })


func create_network(params: Dictionary) -> Dictionary:
	var session_err: Dictionary = _require_session("party_create_network")
	if not session_err.is_empty():
		return session_err
	var handle: String = String(params.get("as", DEFAULT_HANDLE))
	if has_network(handle):
		return _err("handle_in_use", "party network handle '%s' is already tracked" % handle)
	if not _party_initialized:
		var init_resp: Dictionary = await initialize_party(params)
		if not bool(init_resp.get("ok", false)):
			return init_resp

	var party: Object = _runtime.get_playfab().get_party()
	var user: Object = _runtime.get_user()

	var config: Object = _instantiate("PlayFabPartyConfig")
	if config == null:
		return _err("class_unavailable", "PlayFabPartyConfig not registered in ClassDB")
	config.max_players = int(params.get("max_players", 4))
	config.direct_peer_connectivity = int(params.get("direct_peer_connectivity",
		ClassDB.class_get_integer_constant("PlayFabParty", "DIRECT_PEER_CONNECTIVITY_ANY")))
	config.enable_voice_chat = bool(params.get("enable_voice_chat", false))
	config.enable_text_chat = bool(params.get("enable_text_chat", true))
	config.enable_transcription = bool(params.get("enable_transcription", false))
	config.invitation_id = String(params.get("invitation_id", "mp-test-invite-%s" % handle))

	var result: Variant = await _runtime.await_completion(party.create_and_join_network_async(user, config), DEFAULT_AWAIT_TIMEOUT_MS)
	if result == null or not bool(result.ok):
		return _err_from_result(result, "PlayFabParty.create_and_join_network_async")

	var network: Object = result.data
	if network == null:
		return _err("invalid_response", "create_and_join_network_async returned ok with null network")
	_attach_network(handle, network)
	_queue_event("party.network_ready", { "handle": handle, "network": _network_snapshot(network) })
	return _ok({
		"handle": handle,
		"invitation_id": String(config.invitation_id),
		"network": _network_snapshot(network),
	})


func join_network(params: Dictionary) -> Dictionary:
	var session_err: Dictionary = _require_session("party_join_network")
	if not session_err.is_empty():
		return session_err
	var handle: String = String(params.get("as", DEFAULT_HANDLE))
	if has_network(handle):
		return _err("handle_in_use", "party network handle '%s' is already tracked" % handle)
	var descriptor: String = String(params.get("descriptor", ""))
	if descriptor.is_empty():
		return _err("missing_descriptor", "party_join_network requires non-empty descriptor")
	var invitation_id: String = String(params.get("invitation_id", ""))
	if invitation_id.is_empty():
		return _err("missing_invitation_id", "party_join_network requires non-empty invitation_id (the host's create-time value)")

	if not _party_initialized:
		var init_resp: Dictionary = await initialize_party(params)
		if not bool(init_resp.get("ok", false)):
			return init_resp

	var party: Object = _runtime.get_playfab().get_party()
	var user: Object = _runtime.get_user()

	var config: Object = _instantiate("PlayFabPartyConfig")
	if config == null:
		return _err("class_unavailable", "PlayFabPartyConfig not registered in ClassDB")
	config.enable_voice_chat = bool(params.get("enable_voice_chat", false))
	config.enable_text_chat = bool(params.get("enable_text_chat", true))
	config.enable_transcription = bool(params.get("enable_transcription", false))
	config.invitation_id = invitation_id

	var attempts: int = max(1, int(params.get("retry_attempts", JOIN_RETRY_ATTEMPTS)))
	var backoff_ms: int = max(0, int(params.get("retry_backoff_ms", JOIN_RETRY_BACKOFF_MS)))
	var result: Variant = null
	var last_err: Dictionary = {}
	for attempt in range(attempts):
		result = await _runtime.await_completion(party.join_network_async(user, descriptor, config), DEFAULT_AWAIT_TIMEOUT_MS)
		if result != null and bool(result.ok):
			break
		last_err = _err_from_result(result, "PlayFabParty.join_network_async")
		var code: String = String(last_err.get("error", {}).get("code", ""))
		if code != "party_resource_not_ready":
			return last_err
		if attempt + 1 >= attempts:
			return last_err
		await _runtime.wait_until(func(): return false, backoff_ms)
	if result == null or not bool(result.ok):
		return last_err if not last_err.is_empty() else _err_from_result(result, "PlayFabParty.join_network_async")

	var network: Object = result.data
	if network == null:
		return _err("invalid_response", "join_network_async returned ok with null network")
	_attach_network(handle, network)
	_queue_event("party.network_ready", { "handle": handle, "network": _network_snapshot(network) })
	return _ok({
		"handle": handle,
		"invitation_id": invitation_id,
		"network": _network_snapshot(network),
	})


func get_snapshot(params: Dictionary) -> Dictionary:
	var lookup: Dictionary = _lookup_network(params, "party_snapshot")
	if not lookup.has("network"):
		return lookup
	var handle: String = lookup.get("handle", DEFAULT_HANDLE)
	var network: Object = lookup.get("network")
	return _ok({ "handle": handle, "network": _network_snapshot(network) })


func leave_network(params: Dictionary) -> Dictionary:
	var lookup: Dictionary = _lookup_network(params, "party_leave_network")
	if not lookup.has("network"):
		return lookup
	var handle: String = lookup.get("handle", DEFAULT_HANDLE)
	var network: Object = lookup.get("network")
	var network_id: String = String(network.network_id) if "network_id" in network else ""
	var result: Variant = await _runtime.await_completion(network.leave_async(), LEAVE_TIMEOUT_MS)
	if result == null or not bool(result.ok):
		return _err_from_result(result, "PlayFabPartyNetwork.leave_async")
	_detach_network(handle, network)
	_queue_event("party.network_destroyed", { "handle": handle, "left_network_id": network_id })
	return _ok({ "handle": handle, "left_network_id": network_id })


func send_rpc_ping(params: Dictionary) -> Dictionary:
	var lookup: Dictionary = _lookup_network(params, "party_send_rpc_ping")
	if not lookup.has("network"):
		return lookup
	var network: Object = lookup.get("network")
	var peer: Object = network.local_peer if "local_peer" in network else null
	if peer == null:
		return _err("peer_not_ready", "PartyNetwork.local_peer is null (still connecting?)")
	var correlation_id: String = String(params.get("correlation_id", "rpc-%d" % Time.get_ticks_msec()))
	var payload: Dictionary = params.get("payload", {})
	var err: int = _send_rpc_frame(peer, {
		"kind": "mp_test_rpc",
		"type": "ping",
		"correlation_id": correlation_id,
		"payload": payload,
	})
	if err != OK:
		return _err("party_rpc_send_failed", "put_packet failed: %s" % error_string(err), { "error_code": err })
	return _ok({ "handle": lookup.get("handle", DEFAULT_HANDLE), "correlation_id": correlation_id })


func send_chat_text(params: Dictionary) -> Dictionary:
	var lookup: Dictionary = _lookup_network(params, "party_send_chat_text")
	if not lookup.has("network"):
		return lookup
	var network: Object = lookup.get("network")
	var text: String = String(params.get("text", ""))
	if text.is_empty():
		return _err("missing_text", "party_send_chat_text requires non-empty text")
	var peer: Object = network.local_peer if "local_peer" in network else null
	if peer == null:
		return _err("peer_not_ready", "PartyNetwork.local_peer is null (still connecting?)")
	var target_peer_ids := PackedInt32Array()
	for raw_id in params.get("target_peer_ids", []):
		target_peer_ids.append(int(raw_id))
	var result: Variant = await _runtime.await_completion(peer.send_text_async(text, target_peer_ids), SEND_TEXT_TIMEOUT_MS)
	if result == null or not bool(result.ok):
		return _err_from_result(result, "PlayFabPartyPeer.send_text_async")
	return _ok({ "handle": lookup.get("handle", DEFAULT_HANDLE), "text": text, "target_peer_ids": Array(target_peer_ids) })


func set_peer_muted(params: Dictionary) -> Dictionary:
	var lookup: Dictionary = _lookup_network(params, "party_set_peer_muted")
	if not lookup.has("network"):
		return lookup
	var peer: Object = _network_peer(lookup.get("network"))
	if peer == null:
		return _err("peer_not_ready", "PartyNetwork.local_peer is null")
	var peer_id: int = int(params.get("peer_id", 0))
	if peer_id <= 0:
		return _err("invalid_peer_id", "party_set_peer_muted requires peer_id > 0")
	var muted: bool = bool(params.get("muted", true))
	var result: Variant = await _runtime.await_completion(peer.set_peer_muted_async(peer_id, muted), DEFAULT_AWAIT_TIMEOUT_MS)
	if result == null or not bool(result.ok):
		return _err_from_result(result, "PlayFabPartyPeer.set_peer_muted_async")
	return _ok({ "handle": lookup.get("handle", DEFAULT_HANDLE), "peer_id": peer_id, "muted": muted })


func set_peer_chat_permissions(params: Dictionary) -> Dictionary:
	var lookup: Dictionary = _lookup_network(params, "party_set_peer_chat_permissions")
	if not lookup.has("network"):
		return lookup
	var peer: Object = _network_peer(lookup.get("network"))
	if peer == null:
		return _err("peer_not_ready", "PartyNetwork.local_peer is null")
	var peer_id: int = int(params.get("peer_id", 0))
	if peer_id <= 0:
		return _err("invalid_peer_id", "party_set_peer_chat_permissions requires peer_id > 0")
	var permissions: int = int(params.get("permissions", ClassDB.class_get_integer_constant("PlayFabParty", "CHAT_PERMISSION_RECEIVE_TEXT")))
	var result: Variant = await _runtime.await_completion(peer.set_peer_chat_permissions_async(peer_id, permissions), DEFAULT_AWAIT_TIMEOUT_MS)
	if result == null or not bool(result.ok):
		return _err_from_result(result, "PlayFabPartyPeer.set_peer_chat_permissions_async")
	return _ok({ "handle": lookup.get("handle", DEFAULT_HANDLE), "peer_id": peer_id, "permissions": permissions })


func reset(_params: Dictionary) -> Dictionary:
	var left: int = 0
	var failures: Array = []
	for handle in _networks.keys():
		var network: Object = _networks[handle]
		if network == null:
			continue
		var result: Variant = await _runtime.await_completion(network.leave_async(), LEAVE_TIMEOUT_MS)
		if result != null and bool(result.ok):
			left += 1
		else:
			var err_payload: Dictionary = _err_from_result(result, "PlayFabPartyNetwork.leave_async")
			var err_details: Dictionary = err_payload.get("error", {})
			err_details["handle"] = handle
			failures.append(err_details)
	for detach_handle in _networks.keys():
		var detach_network: Object = _networks[detach_handle]
		if detach_network != null:
			_detach_network(String(detach_handle), detach_network)
	_networks.clear()
	_peer_sets.clear()
	if not failures.is_empty():
		return _err(
			"reset_failed",
			"failed to leave %d Party network(s) during reset; respawn required" % failures.size(),
			{ "left": left, "failed": failures },
		)
	return _ok({ "left": left })


func poll_events(emit: Callable) -> void:
	_update_peer_events()
	_drain_packets()
	while not _pending_events.is_empty():
		var event: Dictionary = _pending_events.pop_front()
		emit.call(String(event.get("event_type", "")), event.get("payload", {}))


# ---------------------------------------------------------------------------
# Helpers
# ---------------------------------------------------------------------------

func _require_playfab(op: String) -> Dictionary:
	if _runtime == null or _runtime.get_playfab() == null:
		return _err("playfab_unavailable", "%s requires PlayFab extension" % op)
	return {}


func _require_session(op: String) -> Dictionary:
	var pf_err: Dictionary = _require_playfab(op)
	if not pf_err.is_empty():
		return pf_err
	if not _runtime.has_user():
		return _err("not_signed_in", "%s requires sign_in first" % op)
	return {}


func _lookup_network(params: Dictionary, op: String) -> Dictionary:
	var handle: String = String(params.get("handle", DEFAULT_HANDLE))
	if not has_network(handle):
		return _err("unknown_handle", "%s: no tracked party network for handle '%s' (have %s)" % [op, handle, str(_networks.keys())])
	return { "handle": handle, "network": _networks[handle] }


func _attach_network(handle: String, network: Object) -> void:
	_networks[handle] = network
	_peer_sets[handle] = []
	var network_cb: Callable = _on_network_state_changed.bind(handle)
	if network != null and not network.state_changed.is_connected(network_cb):
		network.state_changed.connect(network_cb)
	var peer: Object = _network_peer(network)
	if peer == null:
		return
	_connect_peer_signal(peer, "connection_state_changed", _on_peer_connection_state_changed.bind(handle))
	_connect_peer_signal(peer, "network_error", _on_peer_network_error.bind(handle))
	_connect_peer_signal(peer, "chat_control_added", _on_peer_chat_control_added.bind(handle))
	_connect_peer_signal(peer, "chat_control_removed", _on_peer_chat_control_removed.bind(handle))
	_connect_peer_signal(peer, "text_message_received", _on_peer_text_message_received.bind(handle))
	_connect_peer_signal(peer, "transcription_received", _on_peer_transcription_received.bind(handle))
	_connect_peer_signal(peer, "chat_permissions_changed", _on_peer_chat_permissions_changed.bind(handle))
	_connect_peer_signal(peer, "peer_muted_changed", _on_peer_muted_changed.bind(handle))


func _detach_network(handle: String, network: Object) -> void:
	var network_cb: Callable = _on_network_state_changed.bind(handle)
	if network != null and network.state_changed.is_connected(network_cb):
		network.state_changed.disconnect(network_cb)
	var peer: Object = _network_peer(network)
	if peer != null:
		_disconnect_peer_signal(peer, "connection_state_changed", _on_peer_connection_state_changed.bind(handle))
		_disconnect_peer_signal(peer, "network_error", _on_peer_network_error.bind(handle))
		_disconnect_peer_signal(peer, "chat_control_added", _on_peer_chat_control_added.bind(handle))
		_disconnect_peer_signal(peer, "chat_control_removed", _on_peer_chat_control_removed.bind(handle))
		_disconnect_peer_signal(peer, "text_message_received", _on_peer_text_message_received.bind(handle))
		_disconnect_peer_signal(peer, "transcription_received", _on_peer_transcription_received.bind(handle))
		_disconnect_peer_signal(peer, "chat_permissions_changed", _on_peer_chat_permissions_changed.bind(handle))
		_disconnect_peer_signal(peer, "peer_muted_changed", _on_peer_muted_changed.bind(handle))
	_networks.erase(handle)
	_peer_sets.erase(handle)


func _connect_peer_signal(peer: Object, signal_name: String, callable: Callable) -> void:
	var sig := Signal(peer, signal_name)
	if not sig.is_connected(callable):
		sig.connect(callable)


func _disconnect_peer_signal(peer: Object, signal_name: String, callable: Callable) -> void:
	var sig := Signal(peer, signal_name)
	if sig.is_connected(callable):
		sig.disconnect(callable)


func _on_network_state_changed(change: Object, handle: String) -> void:
	if change == null:
		return
	var payload: Dictionary = {
		"handle": handle,
		"kind": int(change.kind),
		"state": int(change.state),
		"reason": String(change.reason),
		"peer_id": int(change.peer_id),
		"network": _network_snapshot(change.network),
		"result": _result_snapshot(change.result),
	}
	_queue_event("party.network_state_changed", payload)
	if int(change.state) == CONNECTED_STATE:
		_queue_event("party.network_ready", payload)
	elif int(change.state) == DISCONNECTED_STATE or int(change.state) == FAILED_STATE:
		_queue_event("party.network_destroyed", payload)


func _on_peer_connection_state_changed(status: int, handle: String) -> void:
	_queue_event("party.peer_connection_state_changed", { "handle": handle, "status": status })


func _on_peer_network_error(result: Object, handle: String) -> void:
	_queue_event("party.peer_network_error", { "handle": handle, "result": _result_snapshot(result) })


func _on_peer_chat_control_added(peer_id: int, _chat_control: Object, handle: String) -> void:
	_queue_event("party.chat_control_added", { "handle": handle, "peer_id": peer_id })


func _on_peer_chat_control_removed(peer_id: int, handle: String) -> void:
	_queue_event("party.chat_control_removed", { "handle": handle, "peer_id": peer_id })


func _on_peer_text_message_received(peer_id: int, message: Object, handle: String) -> void:
	_queue_event("party.chat.text_received", _chat_message_payload(handle, peer_id, message))


func _on_peer_transcription_received(peer_id: int, message: Object, handle: String) -> void:
	_queue_event("party.chat.transcription_received", _chat_message_payload(handle, peer_id, message))


func _on_peer_chat_permissions_changed(peer_id: int, permissions: int, handle: String) -> void:
	_queue_event("party.chat_permissions_changed", { "handle": handle, "peer_id": peer_id, "permissions": permissions })


func _on_peer_muted_changed(peer_id: int, muted: bool, handle: String) -> void:
	_queue_event("party.peer_muted_changed", { "handle": handle, "peer_id": peer_id, "muted": muted })


func _chat_message_payload(handle: String, peer_id: int, message: Object) -> Dictionary:
	var payload: Dictionary = { "handle": handle, "peer_id": peer_id }
	if message != null:
		payload["text"] = String(message.get_text()) if message.has_method("get_text") else ""
		payload["sender_entity_key"] = message.get_sender_entity_key() if message.has_method("get_sender_entity_key") else {}
		payload["language_code"] = String(message.get_language_code()) if message.has_method("get_language_code") else ""
		payload["metadata"] = message.get_metadata() if message.has_method("get_metadata") else {}
	return payload


func _update_peer_events() -> void:
	for handle in _networks.keys():
		var network: Object = _networks[handle]
		var peer: Object = _network_peer(network)
		if peer == null:
			continue
		if peer.has_method("poll"):
			peer.poll()
		var current: Array = _peer_ids(peer)
		var previous: Array = _peer_sets.get(handle, [])
		for id in current:
			if not previous.has(id):
				_queue_event("party.peer_connected", {
					"handle": handle,
					"peer_id": int(id),
					"network": _network_snapshot(network),
				})
		for id in previous:
			if not current.has(id):
				_queue_event("party.peer_disconnected", {
					"handle": handle,
					"peer_id": int(id),
					"network": _network_snapshot(network),
				})
		_peer_sets[handle] = current


func _drain_packets() -> void:
	for handle in _networks.keys():
		var network: Object = _networks[handle]
		var peer: Object = _network_peer(network)
		if peer == null:
			continue
		if peer.has_method("poll"):
			peer.poll()
		while peer.get_available_packet_count() > 0:
			var packet: PackedByteArray = peer.get_packet()
			var sender_peer_id: int = 0
			if peer.has_method("get_packet_peer"):
				sender_peer_id = int(peer.get_packet_peer())
			var text: String = packet.get_string_from_utf8()
			var parsed: Variant = JSON.parse_string(text)
			if typeof(parsed) != TYPE_DICTIONARY:
				_queue_event("party.rpc.packet_received", { "handle": handle, "peer_id": sender_peer_id, "raw_text": text })
				continue
			var frame: Dictionary = parsed
			if String(frame.get("kind", "")) != "mp_test_rpc":
				_queue_event("party.rpc.packet_received", { "handle": handle, "peer_id": sender_peer_id, "frame": frame })
				continue
			var payload: Dictionary = {
				"handle": handle,
				"peer_id": sender_peer_id,
				"correlation_id": String(frame.get("correlation_id", "")),
				"payload": frame.get("payload", {}),
			}
			match String(frame.get("type", "")):
				"ping":
					_queue_event("party.rpc.ping_received", payload)
					_send_rpc_frame(peer, {
						"kind": "mp_test_rpc",
						"type": "pong",
						"correlation_id": payload["correlation_id"],
						"payload": payload["payload"],
					})
				"pong":
					_queue_event("party.rpc.pong_received", payload)
				_:
					_queue_event("party.rpc.packet_received", payload)


func _send_rpc_frame(peer: Object, frame: Dictionary) -> int:
	var bytes: PackedByteArray = JSON.stringify(frame).to_utf8_buffer()
	return peer.put_packet(bytes)


func _network_peer(network: Object) -> Object:
	if network == null:
		return null
	return network.local_peer if "local_peer" in network else null


func _peer_ids(peer: Object) -> Array:
	var peer_ids: Array = []
	if peer != null and peer.has_method("get_peers"):
		for raw_id in peer.get_peers():
			peer_ids.append(int(raw_id))
	return peer_ids


func _network_snapshot(network: Object) -> Dictionary:
	if network == null:
		return {}
	var snap: Dictionary = {}
	if "network_id" in network:
		snap["network_id"] = String(network.network_id)
	if "descriptor" in network:
		snap["descriptor"] = String(network.descriptor)
	if "state" in network:
		snap["state"] = int(network.state)
	if "is_host" in network:
		snap["is_host"] = bool(network.is_host)
	var peer: Object = _network_peer(network)
	if peer != null:
		snap["local_peer_unique_id"] = peer.get_unique_id() if peer.has_method("get_unique_id") else 0
		snap["connection_status"] = peer.get_connection_status() if peer.has_method("get_connection_status") else 0
		snap["peer_ids"] = _peer_ids(peer)
		snap["peer_count"] = snap["peer_ids"].size()
	else:
		snap["local_peer_unique_id"] = 0
		snap["connection_status"] = 0
		snap["peer_ids"] = []
		snap["peer_count"] = 0
	return snap


func _instantiate(class_name_str: String) -> Object:
	if not ClassDB.class_exists(class_name_str) or not ClassDB.can_instantiate(class_name_str):
		return null
	return ClassDB.instantiate(class_name_str)


func _result_snapshot(result: Object) -> Dictionary:
	if result == null:
		return {}
	return {
		"ok": bool(result.ok),
		"code": String(result.code),
		"message": String(result.message),
	}


func _queue_event(event_type: String, payload: Dictionary) -> void:
	_pending_events.append({
		"event_type": event_type,
		"payload": payload,
	})


func _ok(result: Dictionary) -> Dictionary:
	return { "ok": true, "result": result }


func _err(code: String, message: String, details: Dictionary = {}) -> Dictionary:
	var err: Dictionary = { "code": code, "message": message }
	for key in details.keys():
		err[key] = details[key]
	return { "ok": false, "error": err }


func _err_from_result(result: Variant, label: String) -> Dictionary:
	if result == null:
		return _err("timeout", "%s timed out" % label)
	var code: String = String(result.code) if "code" in result else "playfab_error"
	var message: String = String(result.message) if "message" in result else "%s failed" % label
	return { "ok": false, "error": { "code": code, "message": message, "call": label } }
