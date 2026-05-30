extends "res://scenarios/_base/mp_scenario_base.gd"

const Flow := preload("res://scenarios/_base/party_flows.gd")

const SCENARIO_ID: String = "party.join.invalid_descriptor"
const SCENARIO_NAME: String = "Invalid Party descriptor rejected"
const PRIORITY: String = "P0"
const CATEGORY: String = "party"
const REQUIRED_ROLES: Array[String] = ["guest"]
const REQUIRED_CAPABILITIES: Array[String] = ["playfab_party_available"]
const TIMEOUT_SEC: int = 120

func run(orch) -> Dictionary:
	return await Flow.new().run_party_join_invalid_descriptor(orch)
