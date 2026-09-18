extends "res://scenarios/_base/mp_scenario_base.gd"

const Flow := preload("res://scenarios/_base/party_flows.gd")

const SCENARIO_ID: String = "party.chat.destroy_local_control.rejoin"
const SCENARIO_NAME: String = "Local chat destruction completes across repeated guest rejoins (issue #169)"
const PRIORITY: String = "P0"
const CATEGORY: String = "party"
const REQUIRED_ROLES: Array[String] = ["host", "guest"]
const REQUIRED_CAPABILITIES: Array[String] = [
	"playfab_party_available", "live_write_allowed", "multi_host_processes"
]
const TIMEOUT_SEC: int = 600

func run(orch) -> Dictionary:
	return await Flow.new().run_party_destroy_local_chat_control_rejoin(orch)


func cleanup(orch) -> void:
	for role in REQUIRED_ROLES:
		var response: Dictionary = await orch.client(role).send("party_shutdown", {}, 60_000)
		if not bool(response.get("ok", false)):
			push_warning("[issue169 cleanup] %s party_shutdown failed: %s" % [role, str(response)])
