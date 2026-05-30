extends "res://scenarios/_base/mp_scenario_base.gd"

const Flow := preload("res://scenarios/_base/party_flows.gd")

const SCENARIO_ID: String = "party.state.host_leaves_network_destroyed_on_guest"
const SCENARIO_NAME: String = "Host leave destroys guest network"
const PRIORITY: String = "P1"
const CATEGORY: String = "party"
const REQUIRED_ROLES: Array[String] = ["host", "guest"]
const REQUIRED_CAPABILITIES: Array[String] = ["playfab_party_available", "live_write_allowed", "multi_host_processes"]
const TIMEOUT_SEC: int = 240

func run(orch) -> Dictionary:
	return await Flow.new().run_party_state_host_leaves_network_destroyed_on_guest(orch)
