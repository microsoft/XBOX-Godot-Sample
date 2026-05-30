extends "res://scenarios/_base/mp_scenario_utils.gd"

func run_party_network_create_smoke(orch) -> Dictionary:
	var gate: Variant = requires_live_write(orch)
	if gate != null: return gate
	var signed: Variant = await _sign_in_roles(orch, ["host"])
	if _is_failure(signed): return signed
	var network: Variant = await _party_create_network(orch, "host", "party", _unique_token(orch, "party-create"), false, 4)
	if _is_failure(network): return network
	var err: Variant = assert_true(not String(network.get("descriptor", "")).is_empty(), "Party descriptor should be populated", { "network": network })
	if err != null: return err
	return ok({ "network_id": network.get("network_id", "") })


func run_party_network_join_smoke(orch) -> Dictionary:
	var gate: Variant = requires_live_write(orch)
	if gate != null: return gate
	var pair: Variant = await _party_pair(orch, false)
	if _is_failure(pair): return pair
	return ok()


func run_party_network_leave_smoke(orch) -> Dictionary:
	var gate: Variant = requires_live_write(orch)
	if gate != null: return gate
	var pair: Variant = await _party_pair(orch, false)
	if _is_failure(pair): return pair
	var wait_disc = _client(orch, "host").expect_event("party.peer_disconnected", {})
	var left: Variant = await _command_ok(orch, "guest", "party_leave_network", { "handle": "party" }, COMMAND_TIMEOUT_MS)
	if _is_failure(left): return left
	var event: Dictionary = await wait_disc.wait(PARTY_WAIT_MS)
	if not bool(event.get("ok", false)):
		var host_network: Variant = await _wait_party_peer_count(orch, "host", "party", 0, PARTY_WAIT_MS)
		if _is_failure(host_network): return host_network
	return ok()


func run_party_descriptor_round_trip(orch) -> Dictionary:
	var gate: Variant = requires_live_write(orch)
	if gate != null: return gate
	var pair: Variant = await _party_pair(orch, false)
	if _is_failure(pair): return pair
	var err: Variant = assert_true(String(pair.get("descriptor", "")) == String(pair.get("host_network", {}).get("descriptor", "")), "descriptor should round trip through host snapshot", { "pair": pair })
	if err != null: return err
	return ok()


func run_party_lifecycle_host_create_join_destroy(orch) -> Dictionary:
	var gate: Variant = requires_live_write(orch)
	if gate != null: return gate
	var pair: Variant = await _party_pair(orch, false)
	if _is_failure(pair): return pair
	var wait_destroy = _client(orch, "guest").expect_event("party.network_destroyed", {})
	var left: Variant = await _command_ok(orch, "host", "party_leave_network", { "handle": "party" }, COMMAND_TIMEOUT_MS)
	if _is_failure(left): return left
	var event: Dictionary = await wait_destroy.wait(PARTY_WAIT_MS)
	if not bool(event.get("ok", false)):
		return fail("guest did not observe network_destroyed after host leave", { "event": event })
	return ok()


func run_party_rpc_round_trip_post_join_first_message(orch) -> Dictionary:
	var gate: Variant = requires_live_write(orch)
	if gate != null: return gate
	var pair: Variant = await _party_pair(orch, false)
	if _is_failure(pair): return pair
	var corr: String = _unique_token(orch, "rpc-first")
	var host_wait = _client(orch, "host").expect_event("party.rpc.ping_received", { "correlation_id": corr })
	var guest_wait = _client(orch, "guest").expect_event("party.rpc.pong_received", { "correlation_id": corr })
	var sent: Variant = await _party_send_rpc_ping(orch, "guest", corr, { "from": "guest" })
	if _is_failure(sent): return sent
	var host_event: Dictionary = await host_wait.wait(PARTY_WAIT_MS)
	if not bool(host_event.get("ok", false)): return fail("host did not receive first guest RPC", { "event": host_event })
	var guest_event: Dictionary = await guest_wait.wait(PARTY_WAIT_MS)
	if not bool(guest_event.get("ok", false)): return fail("guest did not receive RPC pong", { "event": guest_event })
	return ok()


func run_party_rpc_bidirectional(orch) -> Dictionary:
	var gate: Variant = requires_live_write(orch)
	if gate != null: return gate
	var pair: Variant = await _party_pair(orch, false)
	if _is_failure(pair): return pair
	var host_corr: String = _unique_token(orch, "rpc-host")
	var guest_corr: String = _unique_token(orch, "rpc-guest")
	var guest_wait_ping = _client(orch, "guest").expect_event("party.rpc.ping_received", { "correlation_id": host_corr })
	var host_wait_pong = _client(orch, "host").expect_event("party.rpc.pong_received", { "correlation_id": host_corr })
	var sent_host: Variant = await _party_send_rpc_ping(orch, "host", host_corr, { "from": "host" })
	if _is_failure(sent_host): return sent_host
	if not bool((await guest_wait_ping.wait(PARTY_WAIT_MS)).get("ok", false)): return fail("guest did not receive host RPC")
	if not bool((await host_wait_pong.wait(PARTY_WAIT_MS)).get("ok", false)): return fail("host did not receive guest pong")
	var host_wait_ping = _client(orch, "host").expect_event("party.rpc.ping_received", { "correlation_id": guest_corr })
	var guest_wait_pong = _client(orch, "guest").expect_event("party.rpc.pong_received", { "correlation_id": guest_corr })
	var sent_guest: Variant = await _party_send_rpc_ping(orch, "guest", guest_corr, { "from": "guest" })
	if _is_failure(sent_guest): return sent_guest
	if not bool((await host_wait_ping.wait(PARTY_WAIT_MS)).get("ok", false)): return fail("host did not receive guest RPC")
	if not bool((await guest_wait_pong.wait(PARTY_WAIT_MS)).get("ok", false)): return fail("guest did not receive host pong")
	return ok()


func run_party_transport_peer_id_assignment(orch) -> Dictionary:
	var gate: Variant = requires_live_write(orch)
	if gate != null: return gate
	var pair: Variant = await _party_pair(orch, false)
	if _is_failure(pair): return pair
	var host_id: int = int(pair.get("host_network", {}).get("local_peer_unique_id", 0))
	var guest_id: int = int(pair.get("guest_network", {}).get("local_peer_unique_id", 0))
	var err: Variant = assert_eq(host_id, 1, "host should use Godot peer id 1")
	if err != null: return err
	err = assert_true(guest_id > 1, "guest should receive a positive non-host peer id", { "guest_id": guest_id })
	if err != null: return err
	return ok({ "host_peer_id": host_id, "guest_peer_id": guest_id })


func run_party_chat_text_round_trip(orch) -> Dictionary:
	var gate: Variant = requires_live_write(orch)
	if gate != null: return gate
	var pair: Variant = await _party_pair(orch, true)
	if _is_failure(pair): return pair
	var text: String = _unique_token(orch, "chat-text")
	var wait_host = _client(orch, "host").expect_event("party.chat.text_received", { "text": text })
	var sent: Variant = await _party_send_chat(orch, "guest", text)
	if _is_failure(sent): return sent
	var event: Dictionary = await wait_host.wait(PARTY_WAIT_MS)
	if not bool(event.get("ok", false)): return fail("host did not receive guest chat text", { "event": event })
	return ok({ "text": text })


func run_party_chat_text_three_clients(orch) -> Dictionary:
	var gate: Variant = requires_live_write(orch)
	if gate != null: return gate
	var triplet: Variant = await _party_triplet(orch, true)
	if _is_failure(triplet): return triplet
	var text: String = _unique_token(orch, "chat-three")
	var host_wait = _client(orch, "host").expect_event("party.chat.text_received", { "text": text })
	var guest2_wait = _client(orch, "guest2").expect_event("party.chat.text_received", { "text": text })
	var sent: Variant = await _party_send_chat(orch, "guest", text)
	if _is_failure(sent): return sent
	if not bool((await host_wait.wait(PARTY_WAIT_MS)).get("ok", false)): return fail("host did not receive guest chat text")
	if not bool((await guest2_wait.wait(PARTY_WAIT_MS)).get("ok", false)): return fail("guest2 did not receive guest chat text")
	return ok()


func run_party_chat_mute_peer(orch) -> Dictionary:
	var gate: Variant = requires_live_write(orch)
	if gate != null: return gate
	var pair: Variant = await _party_pair(orch, true)
	if _is_failure(pair): return pair
	var guest_id: int = int(pair.get("guest_network", {}).get("local_peer_unique_id", 0))
	var muted: Variant = await _command_ok(orch, "host", "party_set_peer_muted", { "handle": "party", "peer_id": guest_id, "muted": true }, COMMAND_TIMEOUT_MS)
	if _is_failure(muted): return muted
	var text: String = _unique_token(orch, "chat-muted")
	var wait_host = _client(orch, "host").expect_event("party.chat.text_received", { "text": text })
	var sent: Variant = await _party_send_chat(orch, "guest", text)
	if _is_failure(sent): return sent
	var event: Dictionary = await wait_host.wait(SHORT_NO_EVENT_MS)
	if bool(event.get("ok", false)):
		return fail("muted peer chat was still delivered", { "event": event })
	return ok()


func run_party_join_invalid_descriptor(orch) -> Dictionary:
	var gate: Variant = requires_live(orch)
	if gate != null: return gate
	var signed: Variant = await _sign_in_roles(orch, ["guest"])
	if _is_failure(signed): return signed
	var err: Variant = await _expect_command_error(orch, "guest", "party_join_network", { "as": "bad", "descriptor": "not-a-party-descriptor", "invitation_id": "bad" }, [])
	if _is_failure(err): return err
	return ok()


func run_party_join_expired_descriptor(orch) -> Dictionary:
	var gate: Variant = requires_live_write(orch)
	if gate != null: return gate
	var signed: Variant = await _sign_in_roles(orch, ["host", "guest"])
	if _is_failure(signed): return signed
	var invitation_id: String = _unique_token(orch, "expired-desc")
	var network: Variant = await _party_create_network(orch, "host", "party", invitation_id, false, 4)
	if _is_failure(network): return network
	var descriptor: String = String(network.get("descriptor", ""))
	var left: Variant = await _command_ok(orch, "host", "party_leave_network", { "handle": "party" }, COMMAND_TIMEOUT_MS)
	if _is_failure(left): return left
	var err: Variant = await _expect_command_error(orch, "guest", "party_join_network", { "as": "expired", "descriptor": descriptor, "invitation_id": invitation_id, "enable_text_chat": false }, [])
	if _is_failure(err): return err
	return ok()


func run_party_create_invalid_direct_peer_connectivity(orch) -> Dictionary:
	var gate: Variant = requires_live_write(orch)
	if gate != null: return gate
	var signed: Variant = await _sign_in_roles(orch, ["host"])
	if _is_failure(signed): return signed
	var err: Variant = await _expect_command_error(orch, "host", "party_create_network", { "as": "bad", "invitation_id": _unique_token(orch, "bad-connectivity"), "direct_peer_connectivity": 1, "enable_text_chat": false }, [])
	if _is_failure(err): return err
	return ok()


func run_party_create_unsigned_in_user(orch) -> Dictionary:
	var err: Variant = await _expect_command_error(orch, "host", "party_create_network", { "as": "bad", "invitation_id": "unsigned" }, ["not_signed_in"])
	if _is_failure(err): return err
	return ok()


func run_party_state_create_join_leave_full_cycle(orch) -> Dictionary:
	var gate: Variant = requires_live_write(orch)
	if gate != null: return gate
	var pair: Variant = await _party_pair(orch, false)
	if _is_failure(pair): return pair
	var wait_disc = _client(orch, "host").expect_event("party.peer_disconnected", {})
	var left_guest: Variant = await _command_ok(orch, "guest", "party_leave_network", { "handle": "party" }, COMMAND_TIMEOUT_MS)
	if _is_failure(left_guest): return left_guest
	if not bool((await wait_disc.wait(PARTY_WAIT_MS)).get("ok", false)):
		return fail("host did not observe guest peer_disconnected")
	var left_host: Variant = await _command_ok(orch, "host", "party_leave_network", { "handle": "party" }, COMMAND_TIMEOUT_MS)
	if _is_failure(left_host): return left_host
	return ok()


func run_party_state_host_leaves_network_destroyed_on_guest(orch) -> Dictionary:
	return await run_party_lifecycle_host_create_join_destroy(orch)


func run_party_chaos_host_kill_network_destroyed(orch) -> Dictionary:
	var gate: Variant = requires_live_write(orch)
	if gate != null: return gate
	var pair: Variant = await _party_pair(orch, false)
	if _is_failure(pair): return pair
	var wait_destroy = _client(orch, "guest").expect_event("party.network_destroyed", {})
	_client(orch, "host").disconnect_client("scenario_party_host_kill")
	var event: Dictionary = await wait_destroy.wait(PARTY_WAIT_MS)
	if not bool(event.get("ok", false)): return fail("guest did not observe network_destroyed after host process exit", { "event": event })
	return ok()


func run_party_chaos_guest_kill_peer_disconnected(orch) -> Dictionary:
	var gate: Variant = requires_live_write(orch)
	if gate != null: return gate
	var pair: Variant = await _party_pair(orch, false)
	if _is_failure(pair): return pair
	var wait_disc = _client(orch, "host").expect_event("party.peer_disconnected", {})
	_client(orch, "guest").disconnect_client("scenario_party_guest_kill")
	var event: Dictionary = await wait_disc.wait(PARTY_WAIT_MS)
	if not bool(event.get("ok", false)): return fail("host did not observe peer_disconnected after guest process exit", { "event": event })
	return ok()


func run_party_lobby_descriptor_via_lobby_property(orch) -> Dictionary:
	var gate: Variant = requires_live_write(orch)
	if gate != null: return gate
	var signed: Variant = await _sign_in_roles(orch, ["host", "guest"])
	if _is_failure(signed): return signed
	var lobby_token: String = _unique_token(orch, "party-lobby")
	var lobby: Variant = await _create_lobby(orch, "host", "lobby", _public_lobby_config(4, { "string_key1": lobby_token }, {}, _role_member_properties("host")))
	if _is_failure(lobby): return lobby
	var invitation_id: String = _unique_token(orch, "party-lobby-invite")
	var network: Variant = await _party_create_network(orch, "host", "party", invitation_id, false, 4)
	if _is_failure(network): return network
	var descriptor: String = String(network.get("descriptor", ""))
	var set_result: Variant = await _command_ok(orch, "host", "set_lobby_properties", { "handle": "lobby", "properties": { "party_descriptor": descriptor, "party_invitation_id": invitation_id } }, COMMAND_TIMEOUT_MS)
	if _is_failure(set_result): return set_result
	var guest_lobby: Variant = await _join_lobby(orch, "guest", "lobby", String(lobby.get("connection_string", "")), _role_member_properties("guest"))
	if _is_failure(guest_lobby): return guest_lobby
	guest_lobby = await _wait_lobby_property(orch, "guest", "lobby", "party_descriptor", descriptor)
	if _is_failure(guest_lobby): return guest_lobby
	var joined: Variant = await _party_join_network(orch, "guest", "party", descriptor, invitation_id, false)
	if _is_failure(joined): return joined
	var host_network: Variant = await _wait_party_peer_count(orch, "host", "party", 1)
	if _is_failure(host_network): return host_network
	return ok()


func run_party_match_descriptor_via_arranged_lobby_property(orch) -> Dictionary:
	var gate: Variant = requires_live_write(orch)
	if gate != null: return gate
	var match: Variant = await _create_two_player_match(orch)
	if _is_failure(match) or _is_skip(match): return match
	var connection_string: String = String(match.get("connection_string", ""))
	for role in ["host", "guest"]:
		var lobby: Variant = await _join_arranged_lobby(orch, role, "arranged", connection_string, _role_member_properties(role))
		if _is_failure(lobby): return lobby
	var invitation_id: String = _unique_token(orch, "party-match")
	var network: Variant = await _party_create_network(orch, "host", "party", invitation_id, false, 4)
	if _is_failure(network): return network
	var descriptor: String = String(network.get("descriptor", ""))
	var set_result: Variant = await _command_ok(orch, "host", "set_lobby_properties", { "handle": "arranged", "properties": { "party_descriptor": descriptor, "party_invitation_id": invitation_id } }, COMMAND_TIMEOUT_MS)
	if _is_failure(set_result): return set_result
	var guest_lobby: Variant = await _wait_lobby_property(orch, "guest", "arranged", "party_descriptor", descriptor)
	if _is_failure(guest_lobby): return guest_lobby
	var joined: Variant = await _party_join_network(orch, "guest", "party", descriptor, invitation_id, false)
	if _is_failure(joined): return joined
	var host_network: Variant = await _wait_party_peer_count(orch, "host", "party", 1)
	if _is_failure(host_network): return host_network
	return ok()


func run_e2e_full_session_match_then_party_play(orch) -> Dictionary:
	var gate: Variant = requires_live_write(orch)
	if gate != null: return gate
	var match: Variant = await _create_two_player_match(orch)
	if _is_failure(match) or _is_skip(match): return match
	var connection_string: String = String(match.get("connection_string", ""))
	for role in ["host", "guest"]:
		var lobby: Variant = await _join_arranged_lobby(orch, role, "arranged", connection_string, _role_member_properties(role))
		if _is_failure(lobby): return lobby
	var invitation_id: String = _unique_token(orch, "e2e-party")
	var network: Variant = await _party_create_network(orch, "host", "party", invitation_id, true, 4)
	if _is_failure(network): return network
	var descriptor: String = String(network.get("descriptor", ""))
	var set_result: Variant = await _command_ok(orch, "host", "set_lobby_properties", { "handle": "arranged", "properties": { "party_descriptor": descriptor, "party_invitation_id": invitation_id } }, COMMAND_TIMEOUT_MS)
	if _is_failure(set_result): return set_result
	var guest_lobby: Variant = await _wait_lobby_property(orch, "guest", "arranged", "party_descriptor", descriptor)
	if _is_failure(guest_lobby): return guest_lobby
	var joined: Variant = await _party_join_network(orch, "guest", "party", descriptor, invitation_id, true)
	if _is_failure(joined): return joined
	var host_network: Variant = await _wait_party_peer_count(orch, "host", "party", 1)
	if _is_failure(host_network): return host_network
	var rpc_corr: String = _unique_token(orch, "e2e-rpc")
	var host_rpc = _client(orch, "host").expect_event("party.rpc.ping_received", { "correlation_id": rpc_corr })
	var guest_pong = _client(orch, "guest").expect_event("party.rpc.pong_received", { "correlation_id": rpc_corr })
	var sent_rpc: Variant = await _party_send_rpc_ping(orch, "guest", rpc_corr, { "phase": "e2e" })
	if _is_failure(sent_rpc): return sent_rpc
	if not bool((await host_rpc.wait(PARTY_WAIT_MS)).get("ok", false)): return fail("e2e host did not receive RPC")
	if not bool((await guest_pong.wait(PARTY_WAIT_MS)).get("ok", false)): return fail("e2e guest did not receive RPC pong")
	var chat_text: String = _unique_token(orch, "e2e-chat")
	var host_chat = _client(orch, "host").expect_event("party.chat.text_received", { "text": chat_text })
	var sent_chat: Variant = await _party_send_chat(orch, "guest", chat_text)
	if _is_failure(sent_chat): return sent_chat
	if not bool((await host_chat.wait(PARTY_WAIT_MS)).get("ok", false)): return fail("e2e host did not receive chat")
	var left_guest: Variant = await _command_ok(orch, "guest", "party_leave_network", { "handle": "party" }, COMMAND_TIMEOUT_MS)
	if _is_failure(left_guest): return left_guest
	var left_host: Variant = await _command_ok(orch, "host", "party_leave_network", { "handle": "party" }, COMMAND_TIMEOUT_MS)
	if _is_failure(left_host): return left_host
	return ok()
