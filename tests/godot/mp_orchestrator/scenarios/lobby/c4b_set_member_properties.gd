## C4b: set_member_properties round-trips for the local member.
##
## Cleanup is handled by the orchestrator's mandatory reset_client after
## every scenario per spec/playfab-multiplayer-test-automation/3-harness-spec.md
## — scenarios do not need to leave_lobby / reset on error paths.
extends "res://scenarios/_base/mp_scenario_base.gd"

const SCENARIO_ID: String = "lobby.set_member_properties_roundtrip"
const SCENARIO_NAME: String = "set_member_properties: local member property round-trip"
const CATEGORY: String = "lobby"
const PRIORITY: String = "P0"
const REQUIRED_ROLES: Array[String] = ["host"]
const REQUIRED_CAPABILITIES: Array[String] = ["playfab_multiplayer_available"]
const TIMEOUT_SEC: int = 240


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

	var created: Dictionary = await host.send("create_lobby", {
		"max_players": 2,
		"member_properties": { "ready": "false" },
	}, 60_000)
	err = assert_ok(created, "create_lobby failed")
	if err != null:
		return err

	var set_resp: Dictionary = await host.send("set_member_properties", {
		"properties": { "ready": "true", "team": "blue" },
	}, 30_000)
	err = assert_ok(set_resp, "set_member_properties failed")
	if err != null:
		return err

	var snapshot: Dictionary = await host.send("get_lobby_snapshot", {}, 30_000)
	err = assert_ok(snapshot, "get_lobby_snapshot failed")
	if err != null:
		return err

	var members: Array = snapshot.get("result", {}).get("lobby", {}).get("members", [])
	if members.is_empty():
		return fail("snapshot reported zero members")

	var local_member: Dictionary = {}
	for m in members:
		if bool(m.get("is_local", false)):
			local_member = m
			break
	if local_member.is_empty():
		return fail("no local member found in snapshot", { "members": members })

	var props: Dictionary = local_member.get("properties", {})
	err = assert_eq(String(props.get("ready", "")), "true", "ready property did not round-trip")
	if err != null:
		return err
	err = assert_eq(String(props.get("team", "")), "blue", "team property did not round-trip")
	if err != null:
		return err

	return ok({ "member_properties": props })
