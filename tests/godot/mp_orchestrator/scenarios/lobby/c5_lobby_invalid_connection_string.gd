## C5a: invalid lobby connection strings fail without leaving a tracked lobby.
##
## Ports the legacy `invalid connection string typed failure` scenario from
## tools/run_playfab_multiplayer_live.ps1.
extends "res://scenarios/_base/mp_scenario_base.gd"

const SCENARIO_ID: String = "lobby.join.invalid_connection_string"
const SCENARIO_NAME: String = "Invalid lobby connection string returns a typed failure"
const CATEGORY: String = "lobby"
const PRIORITY: String = "P0"
const REQUIRED_ROLES: Array[String] = ["guest"]
const REQUIRED_CAPABILITIES: Array[String] = ["playfab_multiplayer_available"]
const TIMEOUT_SEC: int = 120


func run(orch) -> Dictionary:
	if orch.env("LIVE_TESTS", "") != "1":
		return skip("LIVE_TESTS != 1")
	if orch.env("PLAYFAB_TITLE_ID", "").is_empty():
		return skip("PLAYFAB_TITLE_ID not set")

	var guest = orch.client("guest")
	var signed: Dictionary = await guest.send("sign_in", {}, 60_000)
	var err: Variant = assert_ok(signed, "guest sign_in failed")
	if err != null:
		return err

	var joined: Dictionary = await guest.send("join_lobby", {
		"as": "invalid",
		"connection_string": "not-a-valid-playfab-lobby-connection-string",
	}, 60_000)
	if bool(joined.get("ok", false)):
		return fail("join_lobby unexpectedly accepted an invalid connection string", { "response": joined })
	var code: String = String(joined.get("error", {}).get("code", ""))
	if code.is_empty() or code == "timeout":
		return fail("join_lobby invalid connection string did not return a typed non-timeout error", { "response": joined })

	var snapshot: Dictionary = await guest.send("get_lobby_snapshot", { "handle": "invalid" }, 15_000)
	if bool(snapshot.get("ok", false)):
		return fail("failed join left an invalid lobby handle tracked", { "snapshot": snapshot })
	err = assert_eq(String(snapshot.get("error", {}).get("code", "")), "unknown_handle", "invalid join should not track a lobby handle")
	if err != null:
		return err

	return ok({ "error_code": code })
