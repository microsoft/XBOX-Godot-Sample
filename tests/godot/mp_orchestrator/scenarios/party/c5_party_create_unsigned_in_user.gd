extends "res://scenarios/_base/mp_scenario_base.gd"

const Flow := preload("res://scenarios/_base/party_flows.gd")

const SCENARIO_ID: String = "party.create.unsigned_in_user"
const SCENARIO_NAME: String = "Party create with unsigned-in user rejected"
const PRIORITY: String = "P1"
const CATEGORY: String = "party"
const REQUIRED_ROLES: Array[String] = ["host"]
const REQUIRED_CAPABILITIES: Array[String] = ["playfab_party_available"]
const TIMEOUT_SEC: int = 60

func run(orch) -> Dictionary:
	return await Flow.new().run_party_create_unsigned_in_user(orch)
