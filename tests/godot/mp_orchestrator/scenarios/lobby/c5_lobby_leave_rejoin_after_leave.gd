## C5a: host creates a lobby; guest joins, leaves, then joins again. The
## second join should succeed and the host should observe two distinct join
## events at member_count == 2.
##
## Two-role live-gated scenario. Maps to `lobby.leave_rejoin_after_leave` in
## spec/playfab-multiplayer-test-automation/1-test-matrix.md.
##
## Cleanup is handled by the orchestrator's mandatory reset_client after every
## scenario.
extends "res://scenarios/_base/mp_scenario_base.gd"

const SCENARIO_ID: String = "lobby.leave_rejoin_after_leave"
const SCENARIO_NAME: String = "Guest can rejoin the same lobby after leaving"
const CATEGORY: String = "lobby"
const PRIORITY: String = "P0"
const REQUIRED_ROLES: Array[String] = ["host", "guest"]
const REQUIRED_CAPABILITIES: Array[String] = ["playfab_multiplayer_available", "multi_host_processes"]
const TIMEOUT_SEC: int = 240

const CONVERGENCE_TIMEOUT_MS: int = 30_000


func run(orch) -> Dictionary:
	if orch.env("LIVE_TESTS", "") != "1":
		return skip("LIVE_TESTS != 1")
	if orch.env("PLAYFAB_TITLE_ID", "").is_empty():
		return skip("PLAYFAB_TITLE_ID not set")

	# Custom ids derived in each test client from --role + env.
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

	var created: Dictionary = await host.send("create_lobby", {
		"max_players": 4,
		"access_policy": 0,
		"lobby_properties": { "scenario": "leave_rejoin_after_leave" },
	}, 60_000)
	err = assert_ok(created, "host create_lobby failed")
	if err != null:
		return err
	var lobby_id: String = String(created.get("result", {}).get("lobby", {}).get("lobby_id", ""))
	var connection_string: String = String(created.get("result", {}).get("lobby", {}).get("connection_string", ""))
	if lobby_id.is_empty() or connection_string.is_empty():
		return fail("create_lobby returned empty lobby_id or connection_string", { "result": created.get("result", {}) })

	# First join.
	var first_join: Dictionary = await guest.send("join_lobby", { "connection_string": connection_string }, 60_000)
	err = assert_ok(first_join, "guest first join_lobby failed")
	if err != null:
		return err
	err = await _wait_for_host_count(host, 2)
	if err != null:
		return err

	# Leave.
	var left: Dictionary = await guest.send("leave_lobby", {}, 30_000)
	err = assert_ok(left, "guest leave_lobby failed")
	if err != null:
		return err
	err = await _wait_for_host_count(host, 1)
	if err != null:
		return err

	# Second join — same connection_string, fresh handle since leave erased
	# the prior "main" handle on success.
	var second_join: Dictionary = await guest.send("join_lobby", { "connection_string": connection_string }, 60_000)
	err = assert_ok(second_join, "guest rejoin_lobby failed")
	if err != null:
		return err
	err = assert_eq(
		String(second_join.get("result", {}).get("lobby", {}).get("lobby_id", "")),
		lobby_id,
		"guest rejoined a different lobby_id than host created",
	)
	if err != null:
		return err
	err = await _wait_for_host_count(host, 2)
	if err != null:
		return err

	return ok({ "lobby_id": lobby_id, "rejoined": true })


func _wait_for_host_count(host, target: int) -> Variant:
	var deadline_ms: int = Time.get_ticks_msec() + CONVERGENCE_TIMEOUT_MS
	var last_count: int = -1
	while Time.get_ticks_msec() < deadline_ms:
		var snap: Dictionary = await host.send("get_lobby_snapshot", {}, 15_000)
		if not bool(snap.get("ok", false)):
			return fail("host get_lobby_snapshot during convergence failed", { "response": snap })
		last_count = int(snap.get("result", {}).get("lobby", {}).get("member_count", -1))
		if last_count == target:
			return null
	return fail(
		"host snapshot never reached member_count == %d within %dms" % [target, CONVERGENCE_TIMEOUT_MS],
		{ "last_member_count": last_count, "target": target },
	)
