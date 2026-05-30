## C5a: host leaves a two-member lobby and ownership migrates to guest.
##
## Ports the legacy `owner migration after host leave` scenario.
extends "res://scenarios/_base/mp_scenario_base.gd"

const SCENARIO_ID: String = "lobby.leave.host.owner_migration"
const SCENARIO_NAME: String = "Host leave migrates lobby ownership to guest"
const CATEGORY: String = "lobby"
const PRIORITY: String = "P0"
const REQUIRED_ROLES: Array[String] = ["host", "guest"]
const REQUIRED_CAPABILITIES: Array[String] = ["playfab_multiplayer_available", "multi_host_processes"]
const TIMEOUT_SEC: int = 240

const HANDLE := "shared_lobby"
const OWNER_CONVERGENCE_TIMEOUT_MS: int = 60_000


func run(orch) -> Dictionary:
	if orch.env("LIVE_TESTS", "") != "1":
		return skip("LIVE_TESTS != 1")
	if orch.env("PLAYFAB_TITLE_ID", "").is_empty():
		return skip("PLAYFAB_TITLE_ID not set")

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
	var guest_entity_id: String = String(guest_signed.get("result", {}).get("entity_key", {}).get("id", ""))
	if guest_entity_id.is_empty():
		return fail("guest sign_in did not return entity_key.id", { "sign_in": guest_signed })

	var created: Dictionary = await host.send("create_lobby", {
		"as": HANDLE,
		"max_players": 2,
		"access_policy": 0,
		"lobby_properties": { "scenario": "owner_migration_after_host_leave" },
		"member_properties": { "role": "host" },
	}, 60_000)
	err = assert_ok(created, "host create_lobby failed")
	if err != null:
		return err
	var connection_string: String = String(created.get("result", {}).get("lobby", {}).get("connection_string", ""))
	if connection_string.is_empty():
		return fail("create_lobby returned empty connection_string", { "result": created.get("result", {}) })

	var joined: Dictionary = await guest.send("join_lobby", {
		"as": HANDLE,
		"connection_string": connection_string,
		"member_properties": { "role": "guest" },
	}, 60_000)
	err = assert_ok(joined, "guest join_lobby failed")
	if err != null:
		return err

	err = await _wait_guest_owner(guest, "", 2)
	if err != null:
		return err

	var left: Dictionary = await host.send("leave_lobby", { "handle": HANDLE }, 30_000)
	err = assert_ok(left, "host leave_lobby failed")
	if err != null:
		return err

	err = await _wait_guest_owner(guest, guest_entity_id, 1)
	if err != null:
		return err
	return ok({ "new_owner_entity_id": guest_entity_id })


func _wait_guest_owner(guest, expected_owner_id: String, expected_count: int) -> Variant:
	var deadline_ms: int = Time.get_ticks_msec() + OWNER_CONVERGENCE_TIMEOUT_MS
	var last_owner_id: String = ""
	var last_count: int = -1
	while Time.get_ticks_msec() < deadline_ms:
		var snap: Dictionary = await guest.send("get_lobby_snapshot", { "handle": HANDLE }, 15_000)
		if not bool(snap.get("ok", false)):
			return fail("guest get_lobby_snapshot during owner convergence failed", { "response": snap })
		var lobby: Dictionary = snap.get("result", {}).get("lobby", {})
		last_owner_id = String(lobby.get("owner_entity_key", {}).get("id", ""))
		last_count = int(lobby.get("member_count", -1))
		var owner_matches: bool = expected_owner_id.is_empty() or last_owner_id == expected_owner_id
		if owner_matches and last_count == expected_count:
			return null
	return fail("guest snapshot never converged to expected owner/count within %dms" % OWNER_CONVERGENCE_TIMEOUT_MS, { "expected_owner_id": expected_owner_id, "last_owner_id": last_owner_id, "expected_count": expected_count, "last_count": last_count })
