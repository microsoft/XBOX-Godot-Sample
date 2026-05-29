## C4d: party_leave_network on an unknown handle returns unknown_handle.
##
## Offline negative-path scenario — validates dispatcher hygiene and
## handle-lookup logic without depending on PlayFab credentials or live
## network connectivity. Mirrors c4c_cancel_unknown_handle.
##
## Cleanup is handled by the orchestrator's mandatory reset_client between
## scenarios per spec/playfab-multiplayer-test-automation/3-harness-spec.md.
extends "res://scenarios/_base/mp_scenario_base.gd"

const SCENARIO_ID: String = "party.leave_network_unknown_handle"
const SCENARIO_NAME: String = "party_leave_network with bogus handle returns unknown_handle"
const CATEGORY: String = "party"
const PRIORITY: String = "P2"
const REQUIRED_ROLES: Array[String] = ["host"]
const REQUIRED_CAPABILITIES: Array[String] = []
const TIMEOUT_SEC: int = 30


func run(orch) -> Dictionary:
	var host = orch.client("host")

	var response: Dictionary = await host.send("party_leave_network", {
		"handle": "__definitely_not_a_real_party_handle__",
	}, 10_000)

	if bool(response.get("ok", false)):
		return fail("party_leave_network on a bogus handle unexpectedly succeeded", {
			"response": response,
		})

	var error: Dictionary = response.get("error", {})
	var code: String = String(error.get("code", ""))
	var err: Variant = assert_eq(code, "unknown_handle", "expected unknown_handle, got " + code)
	if err != null:
		return err

	return ok({ "error_code": code, "error_message": String(error.get("message", "")) })
