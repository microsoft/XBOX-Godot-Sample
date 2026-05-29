## C4b: create a lobby, inspect it, leave it. Single-client live-gated.
##
## Ports the legacy create_lobby / inspect_lobby / leave_lobby flow from
## tools/run_playfab_multiplayer_live.ps1's bootstrap exercise.
##
## Cleanup is handled by the orchestrator's mandatory reset_client after
## every scenario per spec/playfab-multiplayer-test-automation/3-harness-spec.md
## — scenarios do not need to leave_lobby / reset on error paths.
extends "res://scenarios/_base/mp_scenario_base.gd"

const SCENARIO_ID: String = "lobby.create_inspect_leave"
const SCENARIO_NAME: String = "Host creates, inspects, and leaves a lobby"
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
		"as": "main",
		"max_players": 4,
		"lobby_properties": { "scenario": "create_inspect_leave" },
	}, 60_000)
	err = assert_ok(created, "create_lobby failed")
	if err != null:
		return err

	var created_lobby: Dictionary = created.get("result", {}).get("lobby", {})
	var lobby_id: String = String(created_lobby.get("lobby_id", ""))
	if lobby_id.is_empty():
		return fail("create_lobby returned empty lobby_id", { "result": created.get("result", {}) })

	var snapshot: Dictionary = await host.send("get_lobby_snapshot", { "handle": "main" }, 30_000)
	err = assert_ok(snapshot, "get_lobby_snapshot failed")
	if err != null:
		return err

	var snap_lobby: Dictionary = snapshot.get("result", {}).get("lobby", {})
	err = assert_eq(String(snap_lobby.get("lobby_id", "")), lobby_id, "snapshot lobby_id drift")
	if err != null:
		return err
	err = assert_eq(int(snap_lobby.get("member_count", 0)), 1, "expected exactly the host as member")
	if err != null:
		return err
	var props: Dictionary = snap_lobby.get("properties", {})
	err = assert_eq(String(props.get("scenario", "")), "create_inspect_leave", "lobby_properties.scenario round-trip")
	if err != null:
		return err

	var left: Dictionary = await host.send("leave_lobby", { "handle": "main" }, 30_000)
	err = assert_ok(left, "leave_lobby failed")
	if err != null:
		return err
	err = assert_eq(String(left.get("result", {}).get("left_lobby_id", "")), lobby_id, "leave_lobby returned wrong lobby_id")
	if err != null:
		return err

	return ok({
		"lobby_id": lobby_id,
		"member_count": snap_lobby.get("member_count", 0),
	})
