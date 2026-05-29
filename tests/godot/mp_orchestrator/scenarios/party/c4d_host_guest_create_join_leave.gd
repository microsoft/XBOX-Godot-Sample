## C4d: host creates a Party network, guest joins it via descriptor +
## invitation_id, both leave cleanly.
##
## First multi-role exercise of the orchestrator framework for Party.
## Validates:
##   * Multi-role spawning works end to end (host + guest in separate
##     Godot processes coordinated via TCP).
##   * Same-host Party join works post PR #132's
##     `_resolve_handshake_assignment` fix — two distinct custom_id
##     sign-ins are valid distinct Party devices because PlayFab Party
##     uses entity tokens, not XUser.
##   * Descriptor brokered orchestrator-side (host returns it via
##     party_create_network's response → scenario passes it as the
##     `descriptor` param to guest's party_join_network).
##   * invitation_id matches on both sides (mandatory for Party auth;
##     scenarios pick a deterministic per-scenario invitation_id so
##     host and guest agree without needing a live lobby).
##
## Chat is disabled (voice + text) so the scenario covers the transport-
## only path. The Party SDK only allocates a chat control when chat is
## enabled, and two Party processes on the same machine collide when
## both try to allocate chat controls (audio device probing + shared OS
## resources). The collision was first observed in PR #134 live run
## mp-1780069603, where the host's create_and_join_network_async failed
## with `party_chat_control_create_failed`. Same-host coverage of the
## chat path needs a separate scenario marked REQUIRES_MULTI_MACHINE
## once that capability gate lands.
##
## Same-host UDP collision (transport layer): originally the second
## Party process on the same host failed `ConnectToNetwork` with
## `PartyNetworkDestroyed: failed to bind or connect the UDP socket
## because the address is already in local use` (PR #134 live run
## mp-1780075674). Root-caused to the addon never calling
## `PartyManager::SetOption(LocalUdpSocketBindAddress)` — the SDK
## defaulted to a fixed port that collides between processes. Fixed
## in the same PR by setting the addon to bind ephemeral port (0) +
## the ExcludeGameCorePreferredUdpMultiplayerPort flag in
## `PlayFabParty::_ensure_initialized` (addons/godot_playfab/src/playfab_party.cpp).
## No scenario-level workaround needed — relay-only transport is not
## required for same-host coverage after that fix.
##
## If this scenario hangs, the single-host canary
## (party.create_and_leave_single_host) should also be hanging — that
## isolates whether the regression is in network creation or in the
## multi-peer join path.
extends "res://scenarios/_base/mp_scenario_base.gd"

const SCENARIO_ID: String = "party.host_guest_create_join_leave"
const SCENARIO_NAME: String = "Host creates, guest joins, both leave a Party network"
const CATEGORY: String = "party"
const PRIORITY: String = "P0"
const REQUIRED_ROLES: Array[String] = ["host", "guest"]
const REQUIRED_CAPABILITIES: Array[String] = ["playfab_party_available"]
const TIMEOUT_SEC: int = 240


func run(orch) -> Dictionary:
	if orch.env("LIVE_TESTS", "") != "1":
		return skip("LIVE_TESTS != 1")
	if orch.env("PLAYFAB_TITLE_ID", "").is_empty():
		return skip("PLAYFAB_TITLE_ID not set")

	# Custom ids derived in each test client from --role + env; see
	# tests/godot/mp_test_client/scripts/test_client.gd::_derive_custom_id_for_role.
	# Each role uses a distinct provisioned identity per
	# tools/configure_playfab_test_title.ps1.
	var host = orch.client("host")
	var guest = orch.client("guest")

	# Both clients sign in (in parallel-ish — we await sequentially for
	# determinism; PlayFab sign-in is fast enough that overlapping isn't
	# worth the test complexity).
	var host_signed: Dictionary = await host.send("sign_in", {}, 60_000)
	var err: Variant = assert_ok(host_signed, "host sign_in failed")
	if err != null:
		return err
	var guest_signed: Dictionary = await guest.send("sign_in", {}, 60_000)
	err = assert_ok(guest_signed, "guest sign_in failed")
	if err != null:
		return err

	# Both clients initialize Party in transport-only mode (no chat). See
	# file docstring for the chat-control collision rationale and for the
	# addon-side UDP bind fix that makes same-host transport work.
	var init_params: Dictionary = {
		"enable_voice_chat": false,
		"enable_text_chat": false,
	}
	var host_init: Dictionary = await host.send("party_initialize", init_params, 60_000)
	err = assert_ok(host_init, "host party_initialize failed")
	if err != null:
		return err
	var guest_init: Dictionary = await guest.send("party_initialize", init_params, 60_000)
	err = assert_ok(guest_init, "guest party_initialize failed")
	if err != null:
		return err

	# Deterministic per-scenario invitation_id so host and guest agree
	# without needing a live lobby to broker it.
	var invitation_id: String = "c4d-host-guest-%d" % Time.get_unix_time_from_system()

	# Host creates the network — transport-only, chat disabled per init_params.
	var created: Dictionary = await host.send("party_create_network", {
		"as": "main",
		"max_players": 4,
		"invitation_id": invitation_id,
		"enable_voice_chat": false,
		"enable_text_chat": false,
	}, 90_000)
	err = assert_ok(created, "host party_create_network failed")
	if err != null:
		return err

	var created_net: Dictionary = created.get("result", {}).get("network", {})
	var network_id: String = String(created_net.get("network_id", ""))
	if network_id.is_empty():
		return fail("host party_create_network returned empty network_id", { "result": created.get("result", {}) })

	# Wait for descriptor to populate on the host.
	var descriptor: String = String(created_net.get("descriptor", ""))
	var deadline_ms: int = Time.get_ticks_msec() + 30_000
	while descriptor.is_empty() and Time.get_ticks_msec() < deadline_ms:
		var snap: Dictionary = await host.send("party_snapshot", { "handle": "main" }, 10_000)
		if not bool(snap.get("ok", false)):
			return fail("host party_snapshot during descriptor wait failed", { "response": snap })
		descriptor = String(snap.get("result", {}).get("network", {}).get("descriptor", ""))
	if descriptor.is_empty():
		return fail("host descriptor never populated within 30s", { "network_id": network_id })

	# Guest joins via descriptor + invitation_id — transport-only, chat disabled.
	var joined: Dictionary = await guest.send("party_join_network", {
		"as": "main",
		"descriptor": descriptor,
		"invitation_id": invitation_id,
		"enable_voice_chat": false,
		"enable_text_chat": false,
	}, 90_000)
	err = assert_ok(joined, "guest party_join_network failed")
	if err != null:
		return err

	var joined_net: Dictionary = joined.get("result", {}).get("network", {})
	var guest_network_id: String = String(joined_net.get("network_id", ""))
	if guest_network_id.is_empty():
		return fail("guest party_join_network returned empty network_id", { "result": joined.get("result", {}) })

	# Wait for both sides to see at least one remote peer in their snapshot.
	# Same 250ms-ish polling pattern; cap at 30s.
	var both_see_peers: bool = false
	deadline_ms = Time.get_ticks_msec() + 30_000
	var host_peer_count: int = 0
	var guest_peer_count: int = 0
	while not both_see_peers and Time.get_ticks_msec() < deadline_ms:
		var host_snap: Dictionary = await host.send("party_snapshot", { "handle": "main" }, 10_000)
		var guest_snap: Dictionary = await guest.send("party_snapshot", { "handle": "main" }, 10_000)
		host_peer_count = int(host_snap.get("result", {}).get("network", {}).get("peer_count", 0))
		guest_peer_count = int(guest_snap.get("result", {}).get("network", {}).get("peer_count", 0))
		# peer_count typically excludes self; both seeing >=1 means the
		# remote peer is registered.
		if host_peer_count >= 1 and guest_peer_count >= 1:
			both_see_peers = true
	if not both_see_peers:
		return fail("peers never converged on each other within 30s", {
			"host_peer_count": host_peer_count,
			"guest_peer_count": guest_peer_count,
		})

	# Both leave. Guest first, so the host observes a peer disappearing.
	var guest_left: Dictionary = await guest.send("party_leave_network", { "handle": "main" }, 30_000)
	err = assert_ok(guest_left, "guest party_leave_network failed")
	if err != null:
		return err
	var host_left: Dictionary = await host.send("party_leave_network", { "handle": "main" }, 30_000)
	err = assert_ok(host_left, "host party_leave_network failed")
	if err != null:
		return err

	return ok({
		"network_id": network_id,
		"descriptor_length": descriptor.length(),
		"invitation_id": invitation_id,
		"host_peer_count_at_convergence": host_peer_count,
		"guest_peer_count_at_convergence": guest_peer_count,
	})
