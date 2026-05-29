## C5a: host creates a lobby; guest and guest2 both join via the host's
## connection_string; all three observe member_count == 3.
##
## Three-role live-gated scenario — the first scenario to exercise the
## orchestrator spawning three client processes simultaneously. Maps to the
## `lobby.join_three_clients` entry in
## spec/playfab-multiplayer-test-automation/1-test-matrix.md.
##
## Cleanup is handled by the orchestrator's mandatory reset_client after every
## scenario.
extends "res://scenarios/_base/mp_scenario_base.gd"

const SCENARIO_ID: String = "lobby.join_three_clients"
const SCENARIO_NAME: String = "Three clients converge on the same lobby"
const CATEGORY: String = "lobby"
const PRIORITY: String = "P0"
const REQUIRED_ROLES: Array[String] = ["host", "guest", "guest2"]
const REQUIRED_CAPABILITIES: Array[String] = ["playfab_multiplayer_available", "multi_host_processes"]
const TIMEOUT_SEC: int = 240

const MEMBER_CONVERGENCE_TIMEOUT_MS: int = 45_000


func run(orch) -> Dictionary:
	if orch.env("LIVE_TESTS", "") != "1":
		return skip("LIVE_TESTS != 1")
	if orch.env("PLAYFAB_TITLE_ID", "").is_empty():
		return skip("PLAYFAB_TITLE_ID not set")

	# Custom ids derived in each test client from --role + env (host ->
	# `<prefix>-host`, guest -> `<prefix>-client`, guest2 -> `<prefix>-client2`);
	# see tests/godot/mp_test_client/scripts/test_client.gd.
	var host = orch.client("host")
	var guest = orch.client("guest")
	var guest2 = orch.client("guest2")

	var roster: Array = [
		{ "client": host, "label": "host" },
		{ "client": guest, "label": "guest" },
		{ "client": guest2, "label": "guest2" },
	]
	for entry in roster:
		var signed: Dictionary = await entry["client"].send("sign_in", {}, 60_000)
		var err: Variant = assert_ok(signed, "%s sign_in failed" % entry["label"])
		if err != null:
			return err

	# Host creates a public lobby with max_players = 3.
	var created: Dictionary = await host.send("create_lobby", {
		"max_players": 3,
		"access_policy": 0,
		"lobby_properties": { "scenario": "join_three_clients" },
	}, 60_000)
	var err: Variant = assert_ok(created, "host create_lobby failed")
	if err != null:
		return err
	var host_lobby: Dictionary = created.get("result", {}).get("lobby", {})
	var lobby_id: String = String(host_lobby.get("lobby_id", ""))
	var connection_string: String = String(host_lobby.get("connection_string", ""))
	if lobby_id.is_empty() or connection_string.is_empty():
		return fail("create_lobby returned empty lobby_id or connection_string", { "result": created.get("result", {}) })

	# Both guests join via connection_string.
	for label in ["guest", "guest2"]:
		var joiner = orch.client(label)
		var joined: Dictionary = await joiner.send("join_lobby", {
			"connection_string": connection_string,
			"member_properties": { "role": label },
		}, 60_000)
		err = assert_ok(joined, "%s join_lobby failed" % label)
		if err != null:
			return err
		err = assert_eq(
			String(joined.get("result", {}).get("lobby", {}).get("lobby_id", "")),
			lobby_id,
			"%s joined a different lobby_id than host created" % label,
		)
		if err != null:
			return err

	# Wait for all three clients to observe member_count == 3.
	var counts: Dictionary = { "host": 0, "guest": 0, "guest2": 0 }
	var deadline_ms: int = Time.get_ticks_msec() + MEMBER_CONVERGENCE_TIMEOUT_MS
	while Time.get_ticks_msec() < deadline_ms:
		var all_converged: bool = true
		for entry in roster:
			var snap: Dictionary = await entry["client"].send("get_lobby_snapshot", {}, 15_000)
			if not bool(snap.get("ok", false)):
				return fail("%s get_lobby_snapshot during convergence failed" % entry["label"], { "response": snap })
			var count: int = int(snap.get("result", {}).get("lobby", {}).get("member_count", 0))
			counts[entry["label"]] = count
			if count != 3:
				all_converged = false
		if all_converged:
			return ok({ "lobby_id": lobby_id, "member_counts": counts })

	return fail("lobby member count never converged to 3 on all roles within %dms" % MEMBER_CONVERGENCE_TIMEOUT_MS, {
		"member_counts": counts,
	})
