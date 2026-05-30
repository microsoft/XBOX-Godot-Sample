extends "res://scenarios/_base/mp_scenario_base.gd"

const Flow := preload("res://scenarios/_base/party_flows.gd")

const SCENARIO_ID: String = "party.create.invalid_direct_peer_connectivity"
const SCENARIO_NAME: String = "Invalid Party connectivity rejected"
const PRIORITY: String = "P1"
const CATEGORY: String = "party"
const REQUIRED_ROLES: Array[String] = ["host"]
const REQUIRED_CAPABILITIES: Array[String] = ["playfab_party_available", "live_write_allowed"]
const TIMEOUT_SEC: int = 180

func run(orch) -> Dictionary:
	return await Flow.new().run_party_create_invalid_direct_peer_connectivity(orch)
