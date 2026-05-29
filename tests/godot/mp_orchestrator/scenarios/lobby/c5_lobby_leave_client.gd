## C5a: host creates a lobby, guest joins, guest calls leave_lobby. The host's
## next snapshot must show member_count == 1.
##
## Two-role live-gated scenario. Maps to `lobby.leave_client` in
## spec/playfab-multiplayer-test-automation/1-test-matrix.md.
##
## Cleanup is handled by the orchestrator's mandatory reset_client after every
## scenario.
extends "res://scenarios/_base/mp_scenario_base.gd"

const SCENARIO_ID: String = "lobby.leave_client"
const SCENARIO_NAME: String = "Guest leave is observed by the host's snapshot"
const CATEGORY: String = "lobby"
const PRIORITY: String = "P0"
const REQUIRED_ROLES: Array[String] = ["host", "guest"]
const REQUIRED_CAPABILITIES: Array[String] = ["playfab_multiplayer_available", "multi_host_processes"]
const TIMEOUT_SEC: int = 180

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
		"lobby_properties": { "scenario": "leave_client" },
	}, 60_000)
	err = assert_ok(created, "host create_lobby failed")
	if err != null:
		return err
	var connection_string: String = String(created.get("result", {}).get("lobby", {}).get("connection_string", ""))
	if connection_string.is_empty():
		return fail("create_lobby returned empty connection_string", { "result": created.get("result", {}) })

	var joined: Dictionary = await guest.send("join_lobby", { "connection_string": connection_string }, 60_000)
	err = assert_ok(joined, "guest join_lobby failed")
	if err != null:
		return err

	# Wait for host snapshot to reach member_count == 2 before we leave, so
	# that the "drop back to 1" assertion below proves something.
	err = await _wait_for_host_count(host, 2)
	if err != null:
		return err

	var left: Dictionary = await guest.send("leave_lobby", {}, 30_000)
	err = assert_ok(left, "guest leave_lobby failed")
	if err != null:
		return err

	# Wait for the host to observe the removal.
	err = await _wait_for_host_count(host, 1)
	if err != null:
		return err

	return ok({ "final_host_member_count": 1 })


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
