extends "res://scenarios/_base/mp_scenario_base.gd"

const SCENARIO_ID: String = "_smoke.ping"
const SCENARIO_NAME: String = "C3 bootstrap: ping all connected clients"
const CATEGORY: String = "functional"
const PRIORITY: String = "P0"
const REQUIRED_ROLES: Array[String] = ["host"]
const REQUIRED_CAPABILITIES: Array[String] = []
const TIMEOUT_SEC: int = 10


func run(orch) -> Dictionary:
	var roles: Array = orch.connected_roles()
	if roles.is_empty():
		return fail("no clients connected")

	var pinged: Array[String] = []
	for role in roles:
		var c = orch.client(role)
		var nonce: String = "%s-c3-ping" % role
		var response: Dictionary = await c.send("ping", { "nonce": nonce }, 5000)
		var err: Variant = assert_ok(response, "ping to %s failed" % role)
		if err != null:
			return err
		var returned_nonce: String = String(response.get("result", {}).get("nonce", ""))
		if returned_nonce != nonce:
			return fail("ping to %s returned wrong nonce" % role, {
				"expected": nonce,
				"actual": returned_nonce,
			})
		pinged.append(role)

	return ok({ "pinged": pinged, "count": pinged.size() })
