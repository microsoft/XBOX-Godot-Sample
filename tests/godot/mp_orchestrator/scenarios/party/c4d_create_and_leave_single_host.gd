## C4d: single-host Party network create + leave.
##
## Validates same-host Party viability post PR #132's
## `_resolve_handshake_assignment` fix — the previous regression made
## *any* in-process Party network creation hang. This is the cheapest
## live exercise: sign in, initialize Party, create a network, observe
## a non-empty network_id/descriptor, leave it cleanly. No second peer
## involved — that's intentional. If this scenario hangs or fails on a
## sandbox title, same-host Party is still broken and the multi-role
## scenarios in c4d_host_guest_* will fail too; this scenario isolates
## the regression class.
##
## Chat is disabled (both voice and text) to keep the scenario focused
## on the transport-layer (PartyEndpoint) path. The Party SDK only
## allocates a chat control when chat is enabled, and chat-control
## allocation on Windows touches audio device probing and shared OS
## resources that collide between two Party processes on the same
## machine. Same-host coverage of the chat path needs a separate
## scenario marked REQUIRES_MULTI_MACHINE (see
## tests/godot/mp_orchestrator/scenarios/_smoke/c4_capabilities_smoke.gd
## once that gate lands).
##
## Cleanup is handled by the orchestrator's mandatory reset_client between
## scenarios per spec/playfab-multiplayer-test-automation/3-harness-spec.md.
extends "res://scenarios/_base/mp_scenario_base.gd"

const SCENARIO_ID: String = "party.create_and_leave_single_host"
const SCENARIO_NAME: String = "Host creates a Party network, observes descriptor, leaves"
const CATEGORY: String = "party"
const PRIORITY: String = "P0"
const REQUIRED_ROLES: Array[String] = ["host"]
const REQUIRED_CAPABILITIES: Array[String] = ["playfab_party_available"]
const TIMEOUT_SEC: int = 180


func run(orch) -> Dictionary:
	if orch.env("LIVE_TESTS", "") != "1":
		return skip("LIVE_TESTS != 1")
	if orch.env("PLAYFAB_TITLE_ID", "").is_empty():
		return skip("PLAYFAB_TITLE_ID not set")

	# Custom id derived in the test client from --role + env (see
	# tests/godot/mp_test_client/scripts/test_client.gd::_derive_custom_id_for_role).
	var host = orch.client("host")

	var signed_in: Dictionary = await host.send("sign_in", {}, 60_000)
	var err: Variant = assert_ok(signed_in, "sign_in failed")
	if err != null:
		return err

	var initialized: Dictionary = await host.send("party_initialize", {
		# Chat-free transport scenario — see file docstring. The
		# default in playfab_party_ops.gd is enable_text_chat=true to
		# match the sample autoload, so the scenario must opt out
		# explicitly.
		"enable_voice_chat": false,
		"enable_text_chat": false,
	}, 60_000)
	err = assert_ok(initialized, "party_initialize failed")
	if err != null:
		return err

	var created: Dictionary = await host.send("party_create_network", {
		"as": "main",
		"max_players": 4,
		"enable_voice_chat": false,
		"enable_text_chat": false,
	}, 90_000)
	err = assert_ok(created, "party_create_network failed")
	if err != null:
		return err

	var created_net: Dictionary = created.get("result", {}).get("network", {})
	var network_id: String = String(created_net.get("network_id", ""))
	if network_id.is_empty():
		return fail("party_create_network returned empty network_id", { "result": created.get("result", {}) })

	# The descriptor may arrive asynchronously — give it a short polling
	# window. The sample autoload waits for NETWORK_CHANGE_DESCRIPTOR_UPDATED
	# but here we just poll the snapshot every ~250ms until it shows up,
	# capped at 30s.
	var descriptor: String = String(created_net.get("descriptor", ""))
	var deadline_ms: int = Time.get_ticks_msec() + 30_000
	while descriptor.is_empty() and Time.get_ticks_msec() < deadline_ms:
		var snap: Dictionary = await host.send("party_snapshot", { "handle": "main" }, 10_000)
		if not bool(snap.get("ok", false)):
			return fail("party_snapshot during descriptor wait failed", { "response": snap })
		descriptor = String(snap.get("result", {}).get("network", {}).get("descriptor", ""))
	if descriptor.is_empty():
		return fail("descriptor never populated within 30s", { "network_id": network_id })

	var left: Dictionary = await host.send("party_leave_network", { "handle": "main" }, 30_000)
	err = assert_ok(left, "party_leave_network failed")
	if err != null:
		return err
	err = assert_eq(String(left.get("result", {}).get("left_network_id", "")), network_id, "leave returned wrong network_id")
	if err != null:
		return err

	return ok({
		"network_id": network_id,
		"descriptor_length": descriptor.length(),
	})
