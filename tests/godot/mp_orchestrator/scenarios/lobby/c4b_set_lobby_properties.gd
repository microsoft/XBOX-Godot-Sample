## C4b: set_lobby_properties round-trips. Single-client live-gated.
##
## Asserts that a property set via set_lobby_properties shows up on the next
## get_lobby_snapshot. Also verifies that overwriting a property and setting
## one to null (= delete) behave correctly.
##
## Cleanup is handled by the orchestrator's mandatory reset_client after
## every scenario per spec/playfab-multiplayer-test-automation/3-harness-spec.md
## — scenarios do not need to leave_lobby / reset on error paths.
extends "res://scenarios/_base/mp_scenario_base.gd"

const SCENARIO_ID: String = "lobby.set_lobby_properties_roundtrip"
const SCENARIO_NAME: String = "set_lobby_properties: set, overwrite, delete round-trip"
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
		"lobby_properties": { "initial": "yes" },
	}, 60_000)
	err = assert_ok(created, "create_lobby failed")
	if err != null:
		return err

	var set1: Dictionary = await host.send("set_lobby_properties", {
		"properties": { "score": "42", "initial": "overwritten" },
	}, 30_000)
	err = assert_ok(set1, "first set_lobby_properties failed")
	if err != null:
		return err
	var props1: Dictionary = set1.get("result", {}).get("lobby", {}).get("properties", {})
	err = assert_eq(String(props1.get("score", "")), "42", "score did not round-trip")
	if err != null:
		return err
	err = assert_eq(String(props1.get("initial", "")), "overwritten", "initial was not overwritten")
	if err != null:
		return err

	var set2: Dictionary = await host.send("set_lobby_properties", {
		"properties": { "score": null },
	}, 30_000)
	err = assert_ok(set2, "deleting score via null failed")
	if err != null:
		return err
	var props2: Dictionary = set2.get("result", {}).get("lobby", {}).get("properties", {})
	if props2.has("score") and props2["score"] != null:
		return fail("score property still present after delete", { "properties": props2 })

	return ok({ "after_set": props1, "after_delete": props2 })
