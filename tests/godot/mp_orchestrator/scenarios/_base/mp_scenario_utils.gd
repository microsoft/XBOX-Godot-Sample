extends RefCounted

const COMMAND_TIMEOUT_MS := 60_000
const LOBBY_WAIT_MS := 60_000
const MATCH_WAIT_MS := 180_000
const PARTY_WAIT_MS := 90_000
const SHORT_NO_EVENT_MS := 5_000

func ok(details: Dictionary = {}) -> Dictionary:
	return { "ok": true, "failure_reason": "", "details": details }


func fail(reason: String, details: Dictionary = {}) -> Dictionary:
	return { "ok": false, "failure_reason": reason, "details": details }


func skip(reason: String, details: Dictionary = {}) -> Dictionary:
	return { "ok": true, "skipped": true, "skip": true, "skip_reason": reason, "failure_reason": reason, "details": details }


func assert_true(value: bool, reason: String, details: Dictionary = {}) -> Variant:
	if not value:
		return fail(reason, details)
	return null


func assert_eq(got: Variant, expected: Variant, reason: String = "assertion_failed") -> Variant:
	if got != expected:
		return fail(reason, { "got": got, "expected": expected })
	return null


func assert_false(value: bool, reason: String = "assertion_failed") -> Variant:
	if value:
		return fail(reason)
	return null


func assert_ok(response: Dictionary, reason: String = "response_not_ok") -> Variant:
	if bool(response.get("ok", false)):
		return null
	return fail(reason, { "error": response.get("error", {}), "response": response })


func assert_has(dict: Dictionary, key: String, reason: String = "missing_key") -> Variant:
	if dict.has(key):
		return null
	return fail(reason, { "key": key, "dict": dict })


func requires_live(orch) -> Variant:
	if orch.env("LIVE_TESTS", "") != "1":
		return skip("LIVE_TESTS != 1")
	if String(orch.env("PLAYFAB_TITLE_ID", "")).strip_edges().is_empty():
		return skip("PLAYFAB_TITLE_ID is not set")
	return null


func requires_live_write(orch) -> Variant:
	var live_gate: Variant = requires_live(orch)
	if live_gate != null:
		return live_gate
	if orch.env("LIVE_WRITE_TESTS", "") != "1":
		return skip("LIVE_WRITE_TESTS != 1")
	return null


func _is_failure(value: Variant) -> bool:
	# ok() returns { ok: true, failure_reason: "" } and fail() returns
	# { ok: false, failure_reason: reason }, so a presence check on
	# "failure_reason" alone treats every helper result (including
	# successes) as a failure. Check the "ok" flag instead, defaulting to
	# true for command-result payloads that don't include the flag.
	return typeof(value) == TYPE_DICTIONARY and not bool(value.get("ok", true))


func _is_skip(value: Variant) -> bool:
	return typeof(value) == TYPE_DICTIONARY and bool(value.get("skip", false))


func _client(orch, role: String):
	return orch.client(role)


func _unique_token(orch, prefix: String) -> String:
	var clean: String = prefix.replace(".", "-").replace("_", "-")
	return "%s-%s-%d" % [clean, orch.run_id(), Time.get_ticks_msec()]


func _sleep_ms(orch, ms: int) -> void:
	var tree: SceneTree = orch.get_tree()
	if tree == null:
		OS.delay_msec(ms)
		return
	await tree.create_timer(float(ms) / 1000.0).timeout


func _command(orch, role: String, command: String, params: Dictionary = {}, timeout_ms: int = COMMAND_TIMEOUT_MS) -> Dictionary:
	return await _client(orch, role).send(command, params, timeout_ms)


func _command_ok(orch, role: String, command: String, params: Dictionary = {}, timeout_ms: int = COMMAND_TIMEOUT_MS) -> Variant:
	var resp: Dictionary = await _command(orch, role, command, params, timeout_ms)
	if not bool(resp.get("ok", false)):
		return fail("%s.%s failed" % [role, command], { "response": resp })
	return resp.get("result", {})


func _expect_command_error(orch, role: String, command: String, params: Dictionary, expected_codes: Array = [], timeout_ms: int = COMMAND_TIMEOUT_MS) -> Variant:
	var resp: Dictionary = await _command(orch, role, command, params, timeout_ms)
	if bool(resp.get("ok", false)):
		return fail("%s.%s unexpectedly succeeded" % [role, command], { "response": resp })
	var code: String = String(resp.get("error", {}).get("code", ""))
	if not expected_codes.is_empty() and not expected_codes.has(code):
		return fail("%s.%s failed with unexpected code" % [role, command], { "code": code, "expected": expected_codes, "response": resp })
	return ok({ "code": code, "response": resp })


func _sign_in_roles(orch, roles: Array, params_by_role: Dictionary = {}) -> Variant:
	for role_v in roles:
		var role: String = String(role_v)
		var params: Dictionary = params_by_role.get(role, {})
		var result: Variant = await _command_ok(orch, role, "sign_in", params, COMMAND_TIMEOUT_MS)
		if _is_failure(result):
			return result
	return ok({ "signed_in": roles })


func _role_member_properties(role: String, extra: Dictionary = {}) -> Dictionary:
	var props: Dictionary = { "role": role }
	for key in extra.keys():
		props[key] = extra[key]
	return props


func _eq_filter(key: String, value: String) -> String:
	return "%s eq '%s'" % [key, value]


func _public_lobby_config(max_players: int = 4, search_props: Dictionary = {}, lobby_props: Dictionary = {}, member_props: Dictionary = {}) -> Dictionary:
	return { "max_players": max_players, "access_policy": 0, "search_properties": search_props, "lobby_properties": lobby_props, "member_properties": member_props }


func _private_lobby_config(max_players: int = 4, search_props: Dictionary = {}, lobby_props: Dictionary = {}, member_props: Dictionary = {}) -> Dictionary:
	return { "max_players": max_players, "access_policy": 2, "search_properties": search_props, "lobby_properties": lobby_props, "member_properties": member_props }


func _create_lobby(orch, role: String, handle: String, config: Dictionary, timeout_ms: int = COMMAND_TIMEOUT_MS) -> Variant:
	var result: Variant = await _command_ok(orch, role, "create_lobby", { "as": handle, "config": config }, timeout_ms)
	if _is_failure(result):
		return result
	return result.get("lobby", {})


func _join_lobby(orch, role: String, handle: String, connection_string: String, member_props: Dictionary = {}) -> Variant:
	var result: Variant = await _command_ok(orch, role, "join_lobby", { "as": handle, "connection_string": connection_string, "member_properties": member_props }, COMMAND_TIMEOUT_MS)
	if _is_failure(result):
		return result
	return result.get("lobby", {})


func _join_arranged_lobby(orch, role: String, handle: String, connection_string: String, member_props: Dictionary = {}) -> Variant:
	var result: Variant = await _command_ok(orch, role, "join_arranged_lobby", { "as": handle, "connection_string": connection_string, "member_properties": member_props }, COMMAND_TIMEOUT_MS)
	if _is_failure(result):
		return result
	return result.get("lobby", {})


func _lobby_snapshot(orch, role: String, handle: String) -> Variant:
	var result: Variant = await _command_ok(orch, role, "get_lobby_snapshot", { "handle": handle }, COMMAND_TIMEOUT_MS)
	if _is_failure(result):
		return result
	return result.get("lobby", {})


func _leave_lobby(orch, role: String, handle: String) -> Variant:
	return await _command_ok(orch, role, "leave_lobby", { "handle": handle }, COMMAND_TIMEOUT_MS)


func _wait_lobby_member_count(orch, role: String, handle: String, expected_count: int, timeout_ms: int = LOBBY_WAIT_MS) -> Variant:
	var deadline: int = Time.get_ticks_msec() + timeout_ms
	var last: Dictionary = {}
	while Time.get_ticks_msec() < deadline:
		var lobby: Variant = await _lobby_snapshot(orch, role, handle)
		if _is_failure(lobby):
			return lobby
		last = lobby
		if int(lobby.get("member_count", 0)) == expected_count:
			return lobby
		await _sleep_ms(orch, 500)
	return fail("lobby member count did not converge", { "role": role, "handle": handle, "expected": expected_count, "last": last })


func _wait_lobby_owner_role(orch, role: String, handle: String, owner_role: String, timeout_ms: int = LOBBY_WAIT_MS) -> Variant:
	var deadline: int = Time.get_ticks_msec() + timeout_ms
	var last: Dictionary = {}
	while Time.get_ticks_msec() < deadline:
		var lobby: Variant = await _lobby_snapshot(orch, role, handle)
		if _is_failure(lobby):
			return lobby
		last = lobby
		var want_entity_id: String = _member_entity_id_for_role(lobby, owner_role)
		var owner_entity_id: String = String(lobby.get("owner_entity_key", {}).get("id", ""))
		if not want_entity_id.is_empty() and owner_entity_id == want_entity_id:
			return lobby
		await _sleep_ms(orch, 500)
	return fail("lobby owner did not converge", { "expected_owner_role": owner_role, "last": last })


func _wait_lobby_property(orch, role: String, handle: String, key: String, value: String, timeout_ms: int = LOBBY_WAIT_MS) -> Variant:
	var deadline: int = Time.get_ticks_msec() + timeout_ms
	var last: Dictionary = {}
	while Time.get_ticks_msec() < deadline:
		var lobby: Variant = await _lobby_snapshot(orch, role, handle)
		if _is_failure(lobby):
			return lobby
		last = lobby
		if String(lobby.get("properties", {}).get(key, "")) == value:
			return lobby
		await _sleep_ms(orch, 500)
	return fail("lobby property did not converge", { "role": role, "handle": handle, "key": key, "value": value, "last": last })


func _wait_member_property(orch, role: String, handle: String, member_role: String, key: String, value: String, timeout_ms: int = LOBBY_WAIT_MS) -> Variant:
	var deadline: int = Time.get_ticks_msec() + timeout_ms
	var last: Dictionary = {}
	while Time.get_ticks_msec() < deadline:
		var lobby: Variant = await _lobby_snapshot(orch, role, handle)
		if _is_failure(lobby):
			return lobby
		last = lobby
		var member: Dictionary = _member_for_role(lobby, member_role)
		if String(member.get("properties", {}).get(key, "")) == value:
			return lobby
		await _sleep_ms(orch, 500)
	return fail("member property did not converge", { "role": role, "handle": handle, "member_role": member_role, "key": key, "value": value, "last": last })


func _member_for_role(lobby: Dictionary, member_role: String) -> Dictionary:
	for raw_member in lobby.get("members", []):
		var member: Dictionary = raw_member
		if String(member.get("properties", {}).get("role", "")) == member_role:
			return member
	return {}


func _member_entity_id_for_role(lobby: Dictionary, member_role: String) -> String:
	var member: Dictionary = _member_for_role(lobby, member_role)
	return String(member.get("entity_key", {}).get("id", ""))


func _search_lobbies(orch, role: String, filter: String, max_results: int = 10) -> Variant:
	return await _command_ok(orch, role, "search_lobbies", { "filter": filter, "max_results": max_results }, COMMAND_TIMEOUT_MS)


func _search_contains_lobby_id(search_result: Dictionary, lobby_id: String) -> bool:
	for summary in search_result.get("lobbies", []):
		if String(summary.get("lobby_id", "")) == lobby_id:
			return true
	return false


func _create_join_lobby(orch, roles: Array, handle: String = "main", access_policy: int = 0, max_players: int = 4, search_props: Dictionary = {}, lobby_props: Dictionary = {}) -> Variant:
	var signed: Variant = await _sign_in_roles(orch, roles)
	if _is_failure(signed):
		return signed
	var config: Dictionary = { "max_players": max_players, "access_policy": access_policy, "search_properties": search_props, "lobby_properties": lobby_props, "member_properties": _role_member_properties("host") }
	var host_lobby: Variant = await _create_lobby(orch, "host", handle, config)
	if _is_failure(host_lobby):
		return host_lobby
	var connection_string: String = String(host_lobby.get("connection_string", ""))
	for i in range(1, roles.size()):
		var role: String = String(roles[i])
		var joined: Variant = await _join_lobby(orch, role, handle, connection_string, _role_member_properties(role))
		if _is_failure(joined):
			return joined
	for role_v in roles:
		var waited: Variant = await _wait_lobby_member_count(orch, String(role_v), handle, roles.size())
		if _is_failure(waited):
			return waited
	return { "connection_string": connection_string, "host_lobby": await _lobby_snapshot(orch, "host", handle) }


func _configured_queue(orch) -> Variant:
	var queue: String = String(orch.env("PLAYFAB_MULTIPLAYER_MATCH_QUEUE", "")).strip_edges()
	if queue.is_empty():
		return skip("PLAYFAB_MULTIPLAYER_MATCH_QUEUE is not set")
	return queue


func _create_match_ticket(orch, role: String, handle: String, queue: String, attributes: Dictionary = {}, timeout_seconds: int = 60) -> Variant:
	var result: Variant = await _command_ok(orch, role, "create_match_ticket", { "as": handle, "queue_name": queue, "timeout_seconds": timeout_seconds, "attributes": attributes }, COMMAND_TIMEOUT_MS)
	if _is_failure(result):
		return result
	return result.get("ticket", {})


func _wait_match_ticket(orch, role: String, handle: String, status_name: String = "matched", timeout_ms: int = MATCH_WAIT_MS) -> Variant:
	var result: Variant = await _command_ok(orch, role, "wait_match_ticket", { "handle": handle, "status_name": status_name, "timeout_ms": timeout_ms }, timeout_ms + 5_000)
	if _is_failure(result):
		return result
	return result.get("ticket", {})


func _create_two_player_match(orch, attributes: Dictionary = {}) -> Variant:
	var queue_v: Variant = _configured_queue(orch)
	if _is_failure(queue_v) or _is_skip(queue_v):
		return queue_v
	var signed: Variant = await _sign_in_roles(orch, ["host", "guest"])
	if _is_failure(signed):
		return signed
	var token: String = _unique_token(orch, "match")
	var attrs: Dictionary = attributes.duplicate()
	attrs["scenario_token"] = token
	var host_ticket: Variant = await _create_match_ticket(orch, "host", "match", String(queue_v), attrs)
	if _is_failure(host_ticket):
		return host_ticket
	var guest_ticket: Variant = await _create_match_ticket(orch, "guest", "match", String(queue_v), attrs)
	if _is_failure(guest_ticket):
		return guest_ticket
	host_ticket = await _wait_match_ticket(orch, "host", "match", "matched")
	if _is_failure(host_ticket):
		return host_ticket
	guest_ticket = await _wait_match_ticket(orch, "guest", "match", "matched")
	if _is_failure(guest_ticket):
		return guest_ticket
	return { "host_ticket": host_ticket, "guest_ticket": guest_ticket, "connection_string": String(host_ticket.get("arranged_lobby_connection_string", "")), "queue_name": String(queue_v), "token": token }


func _party_create_network(orch, role: String, handle: String, invitation_id: String, enable_text: bool = true, max_players: int = 4) -> Variant:
	var result: Variant = await _command_ok(orch, role, "party_create_network", { "as": handle, "invitation_id": invitation_id, "enable_text_chat": enable_text, "max_players": max_players }, PARTY_WAIT_MS)
	if _is_failure(result):
		return result
	return result.get("network", {})


func _party_join_network(orch, role: String, handle: String, descriptor: String, invitation_id: String, enable_text: bool = true) -> Variant:
	var result: Variant = await _command_ok(orch, role, "party_join_network", { "as": handle, "descriptor": descriptor, "invitation_id": invitation_id, "enable_text_chat": enable_text }, PARTY_WAIT_MS)
	if _is_failure(result):
		return result
	return result.get("network", {})


func _party_snapshot(orch, role: String, handle: String = "party") -> Variant:
	var result: Variant = await _command_ok(orch, role, "party_get_network_snapshot", { "handle": handle }, COMMAND_TIMEOUT_MS)
	if _is_failure(result):
		return result
	return result.get("network", {})


func _wait_party_peer_count(orch, role: String, handle: String, expected_count: int, timeout_ms: int = PARTY_WAIT_MS) -> Variant:
	var deadline: int = Time.get_ticks_msec() + timeout_ms
	var last: Dictionary = {}
	while Time.get_ticks_msec() < deadline:
		var snap: Variant = await _party_snapshot(orch, role, handle)
		if _is_failure(snap):
			return snap
		last = snap
		if int(snap.get("peer_count", 0)) == expected_count:
			return snap
		await _sleep_ms(orch, 500)
	return fail("party peer count did not converge", { "role": role, "handle": handle, "expected": expected_count, "last": last })


func _wait_party_chat_mesh(orch, role: String, handle: String, expected_remote: int, timeout_ms: int = PARTY_WAIT_MS) -> Variant:
	var deadline: int = Time.get_ticks_msec() + timeout_ms
	var last: Dictionary = {}
	while Time.get_ticks_msec() < deadline:
		var snap: Variant = await _party_snapshot(orch, role, handle)
		if _is_failure(snap):
			return snap
		last = snap
		if int(snap.get("remote_chat_control_count", 0)) >= expected_remote:
			return snap
		await _sleep_ms(orch, 500)
	return fail("party chat mesh did not converge", { "role": role, "handle": handle, "expected": expected_remote, "last": last })


func _party_pair(orch, enable_text: bool = true) -> Variant:
	var signed: Variant = await _sign_in_roles(orch, ["host", "guest"])
	if _is_failure(signed):
		return signed
	var invitation_id: String = _unique_token(orch, "party-invite")
	var host_network: Variant = await _party_create_network(orch, "host", "party", invitation_id, enable_text, 4)
	if _is_failure(host_network):
		return host_network
	var descriptor: String = String(host_network.get("descriptor", ""))
	if descriptor.is_empty():
		return fail("host Party descriptor is empty", { "network": host_network })
	var guest_network: Variant = await _party_join_network(orch, "guest", "party", descriptor, invitation_id, enable_text)
	if _is_failure(guest_network):
		return guest_network
	host_network = await _wait_party_peer_count(orch, "host", "party", 1)
	if _is_failure(host_network):
		return host_network
	guest_network = await _wait_party_peer_count(orch, "guest", "party", 1)
	if _is_failure(guest_network):
		return guest_network
	return { "invitation_id": invitation_id, "descriptor": descriptor, "host_network": host_network, "guest_network": guest_network }


func _party_triplet(orch, enable_text: bool = true) -> Variant:
	var signed: Variant = await _sign_in_roles(orch, ["host", "guest", "guest2"])
	if _is_failure(signed):
		return signed
	var invitation_id: String = _unique_token(orch, "party-invite-3")
	var host_network: Variant = await _party_create_network(orch, "host", "party", invitation_id, enable_text, 4)
	if _is_failure(host_network):
		return host_network
	var descriptor: String = String(host_network.get("descriptor", ""))
	for role in ["guest", "guest2"]:
		var joined: Variant = await _party_join_network(orch, role, "party", descriptor, invitation_id, enable_text)
		if _is_failure(joined):
			return joined
	# Host-centric star: the host registers every guest, but each guest only
	# registers the host (full mesh is Phase B, deferred). So the host converges
	# to 2 transport peers while each guest converges to 1 (the host). Party chat
	# is still meshed, so cross-guest chat is exercised separately by callers.
	var host_waited: Variant = await _wait_party_peer_count(orch, "host", "party", 2)
	if _is_failure(host_waited):
		return host_waited
	for role in ["guest", "guest2"]:
		var waited: Variant = await _wait_party_peer_count(orch, role, "party", 1)
		if _is_failure(waited):
			return waited
	# Transport peers converge on the host star above, but cross-guest chat needs
	# the meshed chat surface to settle: every endpoint must surface the other two
	# chat controls (and grant them RECEIVE_TEXT on chat_control_added) before a
	# broadcast send can reach all peers. Wait on that mesh when text is enabled so
	# guest→guest2 delivery isn't raced against chat-control discovery.
	if enable_text:
		for role in ["host", "guest", "guest2"]:
			var meshed: Variant = await _wait_party_chat_mesh(orch, role, "party", 2)
			if _is_failure(meshed):
				return meshed
	return { "invitation_id": invitation_id, "descriptor": descriptor }


func _wait_event(client, event_type: String, filter: Dictionary = {}, timeout_ms: int = PARTY_WAIT_MS) -> Variant:
	var waiter = client.expect_event(event_type, filter)
	var event: Dictionary = await waiter.wait(timeout_ms)
	if not bool(event.get("ok", false)):
		return fail("event timeout: %s" % event_type, { "filter": filter, "event": event })
	return event


func _party_send_rpc_ping(orch, role: String, correlation_id: String, payload: Dictionary = {}) -> Variant:
	return await _command_ok(orch, role, "party_send_rpc_ping", { "handle": "party", "correlation_id": correlation_id, "payload": payload }, COMMAND_TIMEOUT_MS)


func _party_send_chat(orch, role: String, text: String) -> Variant:
	return await _command_ok(orch, role, "party_send_chat_text", { "handle": "party", "text": text }, COMMAND_TIMEOUT_MS)


# Broadcasts a chat text from `sender` and waits until every role in
# `receiver_roles` has surfaced a party.chat.text_received for it, re-sending on
# a cadence until they all arrive or the deadline expires. RECEIVE_TEXT grants
# are auto-issued on chat_control_added but propagate asynchronously, so the
# first broadcast can land before a receiver has authorized the sender. A single
# armed waiter per receiver catches whichever resend finally gets through, which
# keeps multi-client text delivery robust against grant-propagation latency
# instead of racing a one-shot send against it.
func _party_broadcast_chat_until_received(orch, sender: String, receiver_roles: Array, text: String, total_timeout_ms: int = PARTY_WAIT_MS, resend_interval_ms: int = 6000) -> Variant:
	var waiters: Dictionary = {}
	for role in receiver_roles:
		waiters[role] = _client(orch, role).expect_event("party.chat.text_received", { "text": text })
	var deadline: int = Time.get_ticks_msec() + total_timeout_ms
	var last_target_count: int = -1
	var last_target_ids: Array = []
	while Time.get_ticks_msec() < deadline:
		var sent: Variant = await _party_send_chat(orch, sender, text)
		if _is_failure(sent):
			return sent
		if typeof(sent) == TYPE_DICTIONARY:
			last_target_count = int(sent.get("broadcast_target_count", last_target_count))
			last_target_ids = sent.get("broadcast_target_ids", last_target_ids)
		var settle_deadline: int = Time.get_ticks_msec() + resend_interval_ms
		while Time.get_ticks_msec() < settle_deadline:
			var all_delivered: bool = true
			for role in receiver_roles:
				if not waiters[role].is_delivered():
					all_delivered = false
					break
			if all_delivered:
				return ok({ "text": text })
			await _sleep_ms(orch, 250)
	var missing: Array = []
	for role in receiver_roles:
		if not waiters[role].is_delivered():
			missing.append(role)
	if missing.is_empty():
		return ok({ "text": text })
	return fail("receivers did not get broadcast chat text", { "missing": missing, "text": text, "sender": sender, "sender_broadcast_target_count": last_target_count, "sender_broadcast_target_ids": last_target_ids })
