extends "res://scenarios/_base/mp_scenario_base.gd"

const Flow := preload("res://scenarios/_base/party_flows.gd")

const SCENARIO_ID: String = "party.rpc.bidirectional"
const SCENARIO_NAME: String = "Bidirectional Party RPC sequence"
const PRIORITY: String = "P0"
const CATEGORY: String = "party"
const REQUIRED_ROLES: Array[String] = ["host", "guest"]
const REQUIRED_CAPABILITIES: Array[String] = ["playfab_party_available", "live_write_allowed", "multi_host_processes"]
const TIMEOUT_SEC: int = 240

func run(orch) -> Dictionary:
	return await Flow.new().run_party_rpc_bidirectional(orch)
