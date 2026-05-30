## C5a: guest2 leaves a three-member lobby and both survivors converge.
##
## Ports the legacy `third member leave propagation` scenario.
extends "res://scenarios/_base/mp_scenario_base.gd"

const SCENARIO_ID: String = "lobby.leave.third_member"
const SCENARIO_NAME: String = "Third member leave propagates to host and guest"
const CATEGORY: String = "lobby"
const PRIORITY: String = "P0"
const REQUIRED_ROLES: Array[String] = ["host", "guest", "guest2"]
const REQUIRED_CAPABILITIES: Array[String] = ["playfab_multiplayer_available", "multi_host_processes"]
const TIMEOUT_SEC: int = 240

const HANDLE := "tri_lobby"
const CONVERGENCE_TIMEOUT_MS: int = 45_000


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
		"lobby_properties": { "scenario": "leave_third_member" },
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

	err = await _wait_roles_for_count(roster, 3, true)
	if err != null:
		return err

	var left: Dictionary = await guest2.send("leave_lobby", { "handle": HANDLE }, 30_000)
	err = assert_ok(left, "guest2 leave_lobby failed")
	if err != null:
		return err

	var survivors: Array = [
		{ "client": host, "label": "host" },
		{ "client": guest, "label": "guest" },
	]
	err = await _wait_roles_for_count(survivors, 2, false)
	if err != null:
		return err
	return ok({ "final_member_count": 2 })


func _sign_in_roster(roster: Array) -> Variant:
	for entry in roster:
		var signed: Dictionary = await entry["client"].send("sign_in", {}, 60_000)
		var err: Variant = assert_ok(signed, "%s sign_in failed" % entry["label"])
		if err != null:
			return err
	return null


func _wait_roles_for_count(roster: Array, expected_count: int, expect_guest2: bool) -> Variant:
	var deadline_ms: int = Time.get_ticks_msec() + CONVERGENCE_TIMEOUT_MS
	var last_counts: Dictionary = {}
	var last_members: Dictionary = {}
	while Time.get_ticks_msec() < deadline_ms:
		var all_match: bool = true
		for entry in roster:
			var snap: Dictionary = await entry["client"].send("get_lobby_snapshot", { "handle": HANDLE }, 15_000)
			if not bool(snap.get("ok", false)):
				return fail("%s get_lobby_snapshot during leave convergence failed" % entry["label"], { "response": snap })
			var lobby: Dictionary = snap.get("result", {}).get("lobby", {})
			var count: int = int(lobby.get("member_count", -1))
			var members: Array = lobby.get("members", [])
			last_counts[entry["label"]] = count
			last_members[entry["label"]] = members
			if count != expected_count:
				all_match = false
			if _has_member_role(members, "guest2") != expect_guest2:
				all_match = false
		if all_match:
			return null
	return fail("lobby membership did not converge to expected third-member state within %dms" % CONVERGENCE_TIMEOUT_MS, { "expected_count": expected_count, "expect_guest2": expect_guest2, "last_counts": last_counts, "last_members": last_members })


func _has_member_role(members: Array, role: String) -> bool:
	for member in members:
		var props: Dictionary = member.get("properties", {})
		if String(props.get("role", "")) == role:
			return true
	return false
