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


# Regression coverage for the PlayFabPartyPeer packet-attribution defect
# (netrumble issue #6).
#
# SceneMultiplayer::poll() reads get_packet_peer() BEFORE get_packet(), so those
# accessors must describe the packet at the head of the queue. A peer that
# instead answers from the last dequeue books every packet against the PREVIOUS
# packet's sender. Two clients cannot detect that -- with a single remote sender
# the shifted attribution still lands on the right id -- which is exactly why
# every pre-existing Party RPC scenario was 2-client and this shipped.
#
# Three clients make the host's inbound queue interleave two distinct senders,
# so the shift becomes observable. The test client stamps its own
# get_unique_id() into each frame and the receiver compares that against
# get_packet_peer(); a mismatch is the defect signature.
func run_party_rpc_three_clients(orch) -> Dictionary:
	var gate: Variant = requires_live_write(orch)
	if gate != null: return gate
	var triplet: Variant = await _party_triplet(orch, false)
	if _is_failure(triplet): return triplet
	var guest_roles: Array = ["guest", "guest2"]
	var observed_ids: Dictionary = {}
	var rounds: int = 3
	for round_index in range(rounds):
		var correlations: Dictionary = {}
		var ping_waits: Dictionary = {}
		var pong_waits: Dictionary = {}
		for role in guest_roles:
			var corr: String = _unique_token(orch, "rpc3-%s-%d" % [role, round_index])
			correlations[role] = corr
			ping_waits[role] = _client(orch, "host").expect_event("party.rpc.ping_received", { "correlation_id": corr })
			pong_waits[role] = _client(orch, role).expect_event("party.rpc.pong_received", { "correlation_id": corr })
		# Fire both guests before awaiting either delivery. _party_send_rpc_ping
		# only awaits the client's local command ack, not delivery, so the two
		# pings stay in flight together and can land in a single host drain --
		# the same-drain interleave that makes the head-vs-last-dequeue
		# difference observable.
		for role in guest_roles:
			var sent: Variant = await _party_send_rpc_ping(orch, role, String(correlations[role]), { "from": role })
			if _is_failure(sent): return sent
		for role in guest_roles:
			var result: Dictionary = await ping_waits[role].wait(PARTY_WAIT_MS)
			if not bool(result.get("ok", false)):
				return fail("host did not receive %s RPC in round %d" % [role, round_index], { "event": result })
			var payload: Dictionary = result.get("event", {}).get("payload", {})
			var peer_id: int = int(payload.get("peer_id", 0))
			var sender_id: int = int(payload.get("sender_unique_id", 0))
			var err: Variant = assert_true(sender_id > 1, "%s should report a positive non-host unique id" % role, { "payload": payload })
			if err != null: return err
			err = assert_true(peer_id == sender_id, "host mis-attributed the %s packet in round %d: get_packet_peer must describe the queue head" % [role, round_index], { "payload": payload })
			if err != null: return err
			if observed_ids.has(role):
				err = assert_true(peer_id == int(observed_ids[role]), "%s peer id changed between rounds" % role, { "payload": payload, "observed": observed_ids })
				if err != null: return err
			else:
				observed_ids[role] = peer_id
		for role in guest_roles:
			var pong: Dictionary = await pong_waits[role].wait(PARTY_WAIT_MS)
			if not bool(pong.get("ok", false)):
				return fail("%s did not receive host pong in round %d" % [role, round_index], { "event": pong })
			var pong_payload: Dictionary = pong.get("event", {}).get("payload", {})
			var pong_err: Variant = assert_true(int(pong_payload.get("peer_id", 0)) == 1, "%s should attribute the host pong to peer 1" % role, { "payload": pong_payload })
			if pong_err != null: return pong_err
	var distinct_err: Variant = assert_true(int(observed_ids.get("guest", 0)) != int(observed_ids.get("guest2", 0)), "the two guests must be attributed distinct peer ids", { "observed": observed_ids })
	if distinct_err != null: return distinct_err
	return ok({ "guest_peer_id": int(observed_ids.get("guest", 0)), "guest2_peer_id": int(observed_ids.get("guest2", 0)), "rounds": rounds })


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
	# Host-centric star: the host is the only endpoint that registers (and routes
	# chat to) every guest, so a single host broadcast fans out to both guests.
	# Direct guest->guest text delivery is full-mesh territory (Phase B) and is
	# intentionally not exercised here — see spec/gdext-playfab-party.md.
	#
	# Each guest registers the host as peer_id 1, so we explicitly grant (and await)
	# RECEIVE_TEXT for the host before sending instead of leaning on the async
	# auto-grant fired on chat_control_added — the awaited grant is the same
	# "chat plumbing ready" gate the issue #73 rejoin scenario relies on.
	const PARTY_CHAT_PERMISSION_RECEIVE_TEXT: int = 4
	for role in ["guest", "guest2"]:
		var ready: Variant = await _retry_party_set_peer_chat_permissions(orch, role, "party", 1, PARTY_CHAT_PERMISSION_RECEIVE_TEXT, PARTY_WAIT_MS)
		if _is_failure(ready): return ready
	var text: String = _unique_token(orch, "chat-three")
	return await _party_broadcast_chat_until_received(orch, "host", ["guest", "guest2"], text)


func run_party_chat_mute_peer(orch) -> Dictionary:
	var gate: Variant = requires_live_write(orch)
	if gate != null: return gate
	var pair: Variant = await _party_pair(orch, true)
	if _is_failure(pair): return pair
	var guest_id: int = int(pair.get("guest_network", {}).get("local_peer_unique_id", 0))
	var muted: Variant = await _command_ok(orch, "host", "party_set_peer_muted", { "handle": "party", "peer_id": guest_id, "muted": true, "channel": "text" }, COMMAND_TIMEOUT_MS)
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


# Regression cover for issue #73. A guest that voluntarily leaves a Party
# network and immediately rejoins it must still be able to send AND receive
# text chat with the host. Pre-fix, PlayFabParty::_process_leave_network_completed
# kept the dying wrapper in m_networks until the later PartyNetworkDestroyed
# landed, which let the rejoin's PartyChatControlCreated for the host bind
# to the stale wrapper's peer record instead of the new one. With the chat
# control mapping wrong, the new local_peer's m_peer_records[host_id]
# .chat_control is null, so send_text_async finds zero targets and
# ChatTextReceived routes incoming text to the detached wrapper — both
# directions silently fail. This scenario drives the exact leave -> immediate
# rejoin sequence the tutorial T7 panel reported, then asserts text moves
# both ways. It is deliberately ordered so the rejoin happens before the
# host has had a chance to observe peer_disconnected, mirroring the original
# race so the test still catches a regression of the underlying root cause.
func run_party_leave_rejoin_chat_round_trip(orch) -> Dictionary:
	var gate: Variant = requires_live_write(orch)
	if gate != null: return gate
	var pair: Variant = await _party_pair(orch, true)
	if _is_failure(pair): return pair
	var descriptor: String = String(pair.get("descriptor", ""))
	var invitation_id: String = String(pair.get("invitation_id", ""))

	# Arm the host's peer_disconnected waiter BEFORE issuing leave so the
	# event can't be missed even if the host processes it before we get
	# back here. Then issue leave + rejoin back-to-back to reproduce the
	# exact race that triggered #73 (waiting for the host-side disconnect
	# first would give Party extra DoWork cycles and could let the unfixed
	# bug self-heal via the later PartyNetworkDestroyed cleanup).
	var wait_host_disconnect = _client(orch, "host").expect_event("party.peer_disconnected", {})
	var left: Variant = await _command_ok(orch, "guest", "party_leave_network", { "handle": "party" }, COMMAND_TIMEOUT_MS)
	if _is_failure(left): return left
	var rejoined: Variant = await _party_join_network(orch, "guest", "party", descriptor, invitation_id, true)
	if _is_failure(rejoined): return rejoined

	# Let the host observe the leave + reconvergence (order between
	# disconnect and the new endpoint isn't guaranteed across processes).
	if not bool((await wait_host_disconnect.wait(PARTY_WAIT_MS)).get("ok", false)):
		return fail("host did not observe peer_disconnected after guest leave/rejoin race")
	var host_converged: Variant = await _wait_party_peer_count(orch, "host", "party", 1)
	if _is_failure(host_converged): return host_converged
	var guest_converged: Variant = await _wait_party_peer_count(orch, "guest", "party", 1)
	if _is_failure(guest_converged): return guest_converged

	# Chat-readiness probe on the guest side (the side affected by #73).
	# set_chat_permissions_async returns party_peer_not_connected
	# while the local_peer.m_peer_records[peer_id].chat_control is still
	# null/stale — which is the exact failure surface the bug produced on
	# the rejoining client — so we retry briefly until it succeeds. The
	# probe doubles as a deterministic "chat plumbing is ready" gate
	# before we exercise send/receive, and mirrors what the tutorial
	# autoload does in response to chat_control_added. The orchestrator
	# process does not load the PlayFab GDExtension so we hardcode the
	# CHAT_PERMISSION_RECEIVE_TEXT enum value (matches
	# PlayFabParty::CHAT_PERMISSION_RECEIVE_TEXT in playfab_party.h)
	# instead of going through ClassDB.
	const PARTY_CHAT_PERMISSION_RECEIVE_TEXT: int = 4
	var ready: Variant = await _retry_party_set_peer_chat_permissions(orch, "guest", "party", 1, PARTY_CHAT_PERMISSION_RECEIVE_TEXT, PARTY_WAIT_MS)
	if _is_failure(ready): return ready

	# Bidirectional chat round trip. Pre-fix, guest.send_text_async()
	# enumerates m_peer_records and finds zero targets with a valid
	# chat_control (so the host never sees the text) AND host->guest
	# text routes to the detached wrapper (so the guest never sees it).
	# Both directions must succeed post-fix.
	var text_g_to_h: String = _unique_token(orch, "rejoin-g2h")
	var wait_host_chat = _client(orch, "host").expect_event("party.chat.text_received", { "text": text_g_to_h })
	var sent_g: Variant = await _party_send_chat(orch, "guest", text_g_to_h)
	if _is_failure(sent_g): return sent_g
	if not bool((await wait_host_chat.wait(PARTY_WAIT_MS)).get("ok", false)):
		return fail("host did not receive guest chat text after rejoin (issue #73 regression)", { "text": text_g_to_h })

	var text_h_to_g: String = _unique_token(orch, "rejoin-h2g")
	var wait_guest_chat = _client(orch, "guest").expect_event("party.chat.text_received", { "text": text_h_to_g })
	var sent_h: Variant = await _party_send_chat(orch, "host", text_h_to_g)
	if _is_failure(sent_h): return sent_h
	if not bool((await wait_guest_chat.wait(PARTY_WAIT_MS)).get("ok", false)):
		return fail("guest did not receive host chat text after rejoin (issue #73 regression)", { "text": text_h_to_g })

	return ok({ "guest_to_host": text_g_to_h, "host_to_guest": text_h_to_g })


func run_party_destroy_local_chat_control_rejoin(orch) -> Dictionary:
	var gate: Variant = requires_live_write(orch)
	if gate != null: return gate
	var signed: Variant = await _sign_in_roles(orch, ["host", "guest"], {
		"host": { "create_account": false, "initialize_multiplayer": false },
		"guest": { "create_account": false, "initialize_multiplayer": false },
	})
	if _is_failure(signed): return signed

	const CHAT_PERMISSION_RECEIVE_TEXT: int = 4
	const DESTROY_COMMAND_TIMEOUT_MS: int = 120_000
	const CYCLE_COUNT: int = 3
	var invitation_id: String = _unique_token(orch, "destroy-chat-control")
	var host_created: Variant = await _command_ok(orch, "host", "party_create_network", {
		"as": "party",
		"invitation_id": invitation_id,
		"enable_text_chat": true,
		"max_players": 4,
	}, PARTY_WAIT_MS)
	if _is_failure(host_created): return host_created
	var host_network: Dictionary = host_created.get("network", {})
	var descriptor: String = String(host_network.get("descriptor", ""))
	var network_id: String = String(host_network.get("network_id", ""))
	var err: Variant = assert_true(not descriptor.is_empty(), "host Party descriptor should be populated", { "network": host_network })
	if err != null: return err
	err = assert_true(not network_id.is_empty(), "host Party network id should be populated", { "network": host_network })
	if err != null: return err
	err = assert_true(int(host_network.get("local_chat_control_instance_id", 0)) != 0, "host should create a local chat control before joining", { "network": host_network })
	if err != null: return err

	var guest_control_instance_id: int = 0
	var guest_peer_id: int = 0
	var cycles_completed: int = 0
	for round_index in range(CYCLE_COUNT + 1):
		var wait_host_disconnect: Variant = null
		if round_index > 0:
			var cycle_number: int = round_index
			_client(orch, "host").event_log.clear()
			wait_host_disconnect = _client(orch, "host").expect_event("party.peer_disconnected", {
				"handle": "party",
				"peer_id": guest_peer_id,
			})
			var left: Variant = await _command_ok(
				orch, "guest", "party_leave_network", { "handle": "party" }, COMMAND_TIMEOUT_MS)
			if _is_failure(left): return left
			err = assert_eq(String(left.get("left_network_id", "")), network_id, "guest leave should report the hosted network id")
			if err != null: return err

			var destroyed: Variant = await _command_ok(orch, "guest", "party_destroy_local_chat_control", {
				"require_existing": true,
			}, DESTROY_COMMAND_TIMEOUT_MS)
			if _is_failure(destroyed): return destroyed
			err = assert_true(bool(destroyed.get("had_control", false)), "destroy cycle %d should start with a local chat control" % cycle_number, { "destroyed": destroyed })
			if err != null: return err
			err = assert_eq(int(destroyed.get("control_instance_id", 0)), guest_control_instance_id, "destroy cycle %d should target the expected wrapper" % cycle_number)
			if err != null: return err
			err = assert_eq(int(destroyed.get("completion_count", 0)), 1, "destroy cycle %d should complete exactly once" % cycle_number)
			if err != null: return err
			err = assert_true(bool(destroyed.get("local_control_absent_at_completion", false)), "destroy cycle %d should remove the local control before completion" % cycle_number, { "destroyed": destroyed })
			if err != null: return err
			err = assert_true(bool(destroyed.get("local_control_absent", false)), "destroy cycle %d should leave no cached local control" % cycle_number, { "destroyed": destroyed })
			if err != null: return err
			err = assert_eq(int(destroyed.get("pending_operation_count", -1)), 0, "destroy cycle %d should release pending storage" % cycle_number)
			if err != null: return err
			err = assert_true(bool(destroyed.get("result_data_is_null", false)), "destroy cycle %d success should carry null data" % cycle_number, { "destroyed": destroyed })
			if err != null: return err
			err = assert_true(bool(destroyed.get("old_wrapper_destroy_ok", false)), "destroy cycle %d old wrapper should be detached and idempotent" % cycle_number, { "destroyed": destroyed })
			if err != null: return err

			var repeated_destroy: Variant = await _command_ok(orch, "guest", "party_destroy_local_chat_control", {
				"require_existing": false,
			}, DESTROY_COMMAND_TIMEOUT_MS)
			if _is_failure(repeated_destroy): return repeated_destroy
			err = assert_false(bool(repeated_destroy.get("had_control", true)), "destroy cycle %d repeated call should be an idempotent no-op" % cycle_number)
			if err != null: return err
			err = assert_eq(int(repeated_destroy.get("completion_count", 0)), 1, "destroy cycle %d repeated call should still complete once" % cycle_number)
			if err != null: return err
			err = assert_eq(int(repeated_destroy.get("pending_operation_count", -1)), 0, "destroy cycle %d repeated call should allocate no pending operation" % cycle_number)
			if err != null: return err

		var joined: Variant = await _command_ok(orch, "guest", "party_join_network", {
			"as": "party",
			"descriptor": descriptor,
			"invitation_id": invitation_id,
			"enable_text_chat": true,
			"retry_attempts": 1,
		}, PARTY_WAIT_MS)
		if _is_failure(joined): return joined
		var guest_network: Dictionary = joined.get("network", {})
		var current_control_instance_id: int = int(guest_network.get("local_chat_control_instance_id", 0))
		err = assert_true(current_control_instance_id != 0, "guest should create a local chat control before joining", { "network": guest_network })
		if err != null: return err
		err = assert_eq(String(guest_network.get("network_id", "")), network_id, "guest should join the host network")
		if err != null: return err
		if round_index > 0:
			err = assert_true(current_control_instance_id != guest_control_instance_id, "rejoin cycle %d should use a new local chat-control wrapper" % round_index, { "old": guest_control_instance_id, "new": current_control_instance_id })
			if err != null: return err

		if wait_host_disconnect != null:
			var disconnect_event: Dictionary = await wait_host_disconnect.wait(PARTY_WAIT_MS)
			if not bool(disconnect_event.get("ok", false)):
				return fail("host did not observe guest disconnect in destroy/rejoin cycle %d" % round_index, {
					"expected_peer_id": guest_peer_id,
					"event": disconnect_event,
				})
		host_network = await _wait_party_peer_count(orch, "host", "party", 1)
		if _is_failure(host_network): return host_network
		guest_network = await _wait_party_peer_count(orch, "guest", "party", 1)
		if _is_failure(guest_network): return guest_network
		var host_mesh: Variant = await _wait_party_chat_mesh(orch, "host", "party", 1)
		if _is_failure(host_mesh): return host_mesh
		var guest_mesh: Variant = await _wait_party_chat_mesh(orch, "guest", "party", 1)
		if _is_failure(guest_mesh): return guest_mesh

		var current_guest_peer_id: int = int(guest_network.get("local_peer_unique_id", 0))
		err = assert_true(current_guest_peer_id > 1, "guest should receive a positive non-host peer id", { "network": guest_network })
		if err != null: return err
		var host_ready: Variant = await _retry_party_set_peer_chat_permissions(
			orch, "host", "party", current_guest_peer_id, CHAT_PERMISSION_RECEIVE_TEXT, PARTY_WAIT_MS)
		if _is_failure(host_ready): return host_ready
		var guest_ready: Variant = await _retry_party_set_peer_chat_permissions(
			orch, "guest", "party", 1, CHAT_PERMISSION_RECEIVE_TEXT, PARTY_WAIT_MS)
		if _is_failure(guest_ready): return guest_ready
		var round_label: String = "destroy-baseline" if round_index == 0 else "destroy-cycle-%d" % round_index
		var round_trip: Variant = await _party_bidirectional_chat_round_trip(orch, round_label)
		if _is_failure(round_trip): return round_trip

		guest_control_instance_id = current_control_instance_id
		guest_peer_id = current_guest_peer_id
		if round_index > 0:
			cycles_completed += 1

	err = assert_eq(cycles_completed, CYCLE_COUNT, "all destroy/recreate/rejoin cycles should complete")
	if err != null: return err

	for role in ["guest", "host"]:
		var final_leave: Variant = await _command_ok(
			orch, role, "party_leave_network", { "handle": "party" }, COMMAND_TIMEOUT_MS)
		if _is_failure(final_leave): return final_leave

		var require_existing: bool = true
		if role == "guest":
			var released: Variant = await _command_ok(orch, role, "party_release_local_user", {
				"require_existing": true,
			}, DESTROY_COMMAND_TIMEOUT_MS)
			if _is_failure(released): return released
			err = assert_true(bool(released.get("had_control", false)), "guest release should retain and detach an existing local chat-control wrapper", { "released": released })
			if err != null: return err
			err = assert_eq(int(released.get("control_instance_id", 0)), guest_control_instance_id, "guest release should target the current wrapper")
			if err != null: return err
			err = assert_true(bool(released.get("local_control_absent", false)), "guest release should remove the cached local chat control", { "released": released })
			if err != null: return err
			err = assert_eq(int(released.get("pending_operation_count", -1)), 0, "guest release should drain chat-control destruction")
			if err != null: return err
			err = assert_true(bool(released.get("result_data_is_null", false)), "guest release success should carry null data", { "released": released })
			if err != null: return err
			err = assert_true(bool(released.get("old_wrapper_indicator_detached", false)), "retained guest wrapper should report the detached local indicator safely", { "released": released })
			if err != null: return err
			err = assert_true(bool(released.get("old_wrapper_send_failed_safely", false)), "retained guest wrapper should reject chat after release without touching freed native state", { "released": released })
			if err != null: return err
			err = assert_true(bool(released.get("old_wrapper_destroy_ok", false)), "retained guest wrapper destroy should be an idempotent detached no-op", { "released": released })
			if err != null: return err
			require_existing = false

		var final_destroy: Variant = await _command_ok(orch, role, "party_destroy_local_chat_control", {
			"require_existing": require_existing,
		}, DESTROY_COMMAND_TIMEOUT_MS)
		if _is_failure(final_destroy): return final_destroy
		err = assert_eq(int(final_destroy.get("pending_operation_count", -1)), 0, "final %s destroy should release pending storage" % role)
		if err != null: return err

	var shutdowns: Dictionary = {}
	for role in ["guest", "host"]:
		var shutdown: Variant = await _command_ok(
			orch, role, "party_shutdown", {}, COMMAND_TIMEOUT_MS)
		if _is_failure(shutdown): return shutdown
		shutdowns[role] = shutdown
		err = assert_eq(int(shutdown.get("pending_operation_count_before", -1)), 0, "%s shutdown should start with no pending operations" % role)
		if err != null: return err
		err = assert_eq(int(shutdown.get("pending_operation_count_after", -1)), 0, "%s shutdown should end with no pending operations" % role)
		if err != null: return err
		err = assert_false(bool(shutdown.get("initialized_after", true)), "%s Party service should be uninitialized after shutdown" % role)
		if err != null: return err
		err = assert_eq(int(shutdown.get("network_count_after", -1)), 0, "%s shutdown should leave no tracked networks" % role)
		if err != null: return err
		err = assert_eq(int(shutdown.get("chat_control_count_after", -1)), 0, "%s shutdown should leave no tracked chat controls" % role)
		if err != null: return err

	return ok({
		"cycles_completed": cycles_completed,
		"bidirectional_round_trips": cycles_completed + 1,
		"guest_shutdown": shutdowns.get("guest", {}),
		"host_shutdown": shutdowns.get("host", {}),
	})


func _party_bidirectional_chat_round_trip(orch, label: String) -> Variant:
	var exchanged: Dictionary = {}
	for direction in [
		{ "sender": "guest", "receiver": "host", "tag": "g2h", "result_key": "guest_to_host" },
		{ "sender": "host", "receiver": "guest", "tag": "h2g", "result_key": "host_to_guest" },
	]:
		var sender: String = String(direction.sender)
		var receiver: String = String(direction.receiver)
		var text: String = _unique_token(orch, "%s-%s" % [label, direction.tag])
		var waiter = _client(orch, receiver).expect_event("party.chat.text_received", {
			"text": text,
		})
		var sent: Variant = await _party_send_chat(orch, sender, text)
		if _is_failure(sent): return sent
		var received: Dictionary = await waiter.wait(PARTY_WAIT_MS)
		if not bool(received.get("ok", false)):
			return fail("%s did not receive %s chat during %s" % [receiver, sender, label], {
				"event": received,
				"text": text,
			})
		exchanged[String(direction.result_key)] = text
	return ok(exchanged)


# Retry party_set_peer_chat_permissions until the target peer's chat
# control is attached to the local peer record (or the deadline expires).
# Used as a "chat ready" gate after a rejoin: the only retryable error
# is party_peer_not_connected — every other failure mode is fatal and
# returned immediately rather than burning the deadline.
func _retry_party_set_peer_chat_permissions(orch, role: String, handle: String, peer_id: int, permissions: int, timeout_ms: int) -> Variant:
	var deadline: int = Time.get_ticks_msec() + timeout_ms
	var last: Dictionary = {}
	while Time.get_ticks_msec() < deadline:
		var resp: Dictionary = await _command(orch, role, "party_set_peer_chat_permissions", { "handle": handle, "peer_id": peer_id, "permissions": permissions }, COMMAND_TIMEOUT_MS)
		if bool(resp.get("ok", false)):
			return ok({ "handle": handle, "peer_id": peer_id, "permissions": permissions })
		last = resp
		var code: String = String(resp.get("error", {}).get("code", ""))
		if code != "party_peer_not_connected":
			return fail("party_set_peer_chat_permissions failed: %s" % code, { "response": resp })
		await _sleep_ms(orch, 100)
	return fail("party_set_peer_chat_permissions readiness probe timed out (%d ms)" % timeout_ms, { "role": role, "peer_id": peer_id, "last_response": last })


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
