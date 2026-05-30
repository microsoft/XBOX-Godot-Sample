## C5a: host-set lobby properties propagate to all three members.
##
## Ports the legacy `lobby property propagation` scenario with three roles.
extends "res://scenarios/_base/mp_scenario_base.gd"

const SCENARIO_ID: String = "lobby.properties.lobby.propagation"
const SCENARIO_NAME: String = "Lobby property update propagates to host, guest, and guest2"
const CATEGORY: String = "lobby"
const PRIORITY: String = "P0"
const REQUIRED_ROLES: Array[String] = ["host", "guest", "guest2"]
const REQUIRED_CAPABILITIES: Array[String] = ["playfab_multiplayer_available", "multi_host_processes"]
const TIMEOUT_SEC: int = 240

const HANDLE := "tri_lobby"
const PROPERTY_CONVERGENCE_TIMEOUT_MS: int = 45_000


func run(orch) -> Dictionary:
	if orch.env("LIVE_TESTS", "") != "1":
		return skip("LIVE_TESTS != 1")
	if orch.env("PLAYFAB_TITLE_ID", "").is_empty():
		return skip("PLAYFAB_TITLE_ID not set")

	var host = orch.client("host")
	var guest = orch.client("guest")
	var guest2 = orch.client("guest2")
	var roster: Array = [
		{ "client": host, "label": "host" },
		{ "client": guest, "label": "guest" },
		{ "client": guest2, "label": "guest2" },
	]
	var err: Variant = await _sign_in_roster(roster)
	if err != null:
		return err

	var created: Dictionary = await host.send("create_lobby", {
		"as": HANDLE,
		"max_players": 3,
		"access_policy": 0,
		"lobby_properties": { "scenario": "lobby_property_propagation", "phase": "created" },
		"member_properties": { "role": "host" },
	}, 60_000)
	err = assert_ok(created, "host create_lobby failed")
	if err != null:
		return err
	var connection_string: String = String(created.get("result", {}).get("lobby", {}).get("connection_string", ""))
	if connection_string.is_empty():
		return fail("create_lobby returned empty connection_string", { "result": created.get("result", {}) })

	for label in ["guest", "guest2"]:
		var joined: Dictionary = await orch.client(label).send("join_lobby", {
			"as": HANDLE,
			"connection_string": connection_string,
			"member_properties": { "role": label },
		}, 60_000)
		err = assert_ok(joined, "%s join_lobby failed" % label)
		if err != null:
			return err

	var expected: Dictionary = { "phase": "propagated", "mode": "expanded" }
	var set_resp: Dictionary = await host.send("set_lobby_properties", {
		"handle": HANDLE,
		"properties": expected,
	}, 30_000)
	err = assert_ok(set_resp, "host set_lobby_properties failed")
	if err != null:
		return err

	err = await _wait_all_roles_for_lobby_properties(roster, expected)
	if err != null:
		return err
	return ok({ "properties": expected })


func _sign_in_roster(roster: Array) -> Variant:
	for entry in roster:
		var signed: Dictionary = await entry["client"].send("sign_in", {}, 60_000)
		var err: Variant = assert_ok(signed, "%s sign_in failed" % entry["label"])
		if err != null:
			return err
	return null


func _wait_all_roles_for_lobby_properties(roster: Array, expected: Dictionary) -> Variant:
	var deadline_ms: int = Time.get_ticks_msec() + PROPERTY_CONVERGENCE_TIMEOUT_MS
	var last_props: Dictionary = {}
	while Time.get_ticks_msec() < deadline_ms:
		var all_match: bool = true
		for entry in roster:
			var snap: Dictionary = await entry["client"].send("get_lobby_snapshot", { "handle": HANDLE }, 15_000)
			if not bool(snap.get("ok", false)):
				return fail("%s get_lobby_snapshot during property convergence failed" % entry["label"], { "response": snap })
			var props: Dictionary = snap.get("result", {}).get("lobby", {}).get("properties", {})
			last_props[entry["label"]] = props
			for key in expected.keys():
				if String(props.get(key, "")) != String(expected[key]):
					all_match = false
		if all_match:
			return null
	return fail("lobby properties did not propagate to all roles within %dms" % PROPERTY_CONVERGENCE_TIMEOUT_MS, { "last_props": last_props, "expected": expected })
