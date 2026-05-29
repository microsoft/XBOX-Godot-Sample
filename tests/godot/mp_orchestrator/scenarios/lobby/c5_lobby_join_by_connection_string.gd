## C5a: host creates a lobby, guest joins via its connection_string,
## both observe member_count == 2.
##
## Two-role live-gated scenario. Maps to the 18-scenario PS runner inventory
## (lobby.join.by_connection_string in spec/playfab-multiplayer-test-automation/1-test-matrix.md).
##
## Cleanup is handled by the orchestrator's mandatory reset_client after every
## scenario per spec/playfab-multiplayer-test-automation/3-harness-spec.md
## — scenarios do not need to leave_lobby on error paths.
extends "res://scenarios/_base/mp_scenario_base.gd"

const SCENARIO_ID: String = "lobby.join_by_connection_string"
const SCENARIO_NAME: String = "Guest joins host's lobby via connection_string"
const CATEGORY: String = "lobby"
const PRIORITY: String = "P0"
const REQUIRED_ROLES: Array[String] = ["host", "guest"]
const REQUIRED_CAPABILITIES: Array[String] = ["playfab_multiplayer_available"]
const TIMEOUT_SEC: int = 180

const MEMBER_CONVERGENCE_TIMEOUT_MS: int = 30_000


func run(orch) -> Dictionary:
	if orch.env("LIVE_TESTS", "") != "1":
		return skip("LIVE_TESTS != 1")
	if orch.env("PLAYFAB_TITLE_ID", "").is_empty():
		return skip("PLAYFAB_TITLE_ID not set")

	# Custom ids derived in each test client from --role + env (host ->
	# `<prefix>-host`, guest -> `<prefix>-client`); see
	# tests/godot/mp_test_client/scripts/test_client.gd::_derive_custom_id_for_role.
	var host = orch.client("host")
	var guest = orch.client("guest")

	var host_signed: Dictionary = await host.send("sign_in", {}, 60_000)
	var err: Variant = assert_ok(host_signed, "host sign_in failed")
	if err != null:
		return err
	var guest_signed: Dictionary = await guest.send("sign_in", {}, 60_000)
	err = assert_ok(guest_signed, "guest sign_in failed")
	if err != null:
		return err

	# Host creates a public lobby with max_players=4 so the guest can join.
	var created: Dictionary = await host.send("create_lobby", {
		"max_players": 4,
		"access_policy": 0,
		"lobby_properties": { "scenario": "join_by_connection_string" },
	}, 60_000)
	err = assert_ok(created, "host create_lobby failed")
	if err != null:
		return err
	var host_lobby: Dictionary = created.get("result", {}).get("lobby", {})
	var lobby_id: String = String(host_lobby.get("lobby_id", ""))
	var connection_string: String = String(host_lobby.get("connection_string", ""))
	if lobby_id.is_empty() or connection_string.is_empty():
		return fail("create_lobby returned empty lobby_id or connection_string", { "result": created.get("result", {}) })

	# Guest joins via connection_string.
	var joined: Dictionary = await guest.send("join_lobby", {
		"connection_string": connection_string,
		"member_properties": { "role": "guest" },
	}, 60_000)
	err = assert_ok(joined, "guest join_lobby failed")
	if err != null:
		return err
	var guest_lobby: Dictionary = joined.get("result", {}).get("lobby", {})
	err = assert_eq(String(guest_lobby.get("lobby_id", "")), lobby_id, "guest joined a different lobby_id than host created")
	if err != null:
		return err

	# Wait for both sides to observe member_count == 2. Member-list propagation
	# is eventual on PlayFab's side, so poll with a deadline.
	var host_count: int = 0
	var guest_count: int = 0
	var deadline_ms: int = Time.get_ticks_msec() + MEMBER_CONVERGENCE_TIMEOUT_MS
	while Time.get_ticks_msec() < deadline_ms:
		var host_snap: Dictionary = await host.send("get_lobby_snapshot", {}, 15_000)
		if not bool(host_snap.get("ok", false)):
			return fail("host get_lobby_snapshot during convergence failed", { "response": host_snap })
		var guest_snap: Dictionary = await guest.send("get_lobby_snapshot", {}, 15_000)
		if not bool(guest_snap.get("ok", false)):
			return fail("guest get_lobby_snapshot during convergence failed", { "response": guest_snap })
		host_count = int(host_snap.get("result", {}).get("lobby", {}).get("member_count", 0))
		guest_count = int(guest_snap.get("result", {}).get("lobby", {}).get("member_count", 0))
		if host_count == 2 and guest_count == 2:
			return ok({
				"lobby_id": lobby_id,
				"host_member_count": host_count,
				"guest_member_count": guest_count,
			})

	return fail("lobby member count never converged to 2 within %dms" % MEMBER_CONVERGENCE_TIMEOUT_MS, {
		"host_member_count": host_count,
		"guest_member_count": guest_count,
	})
