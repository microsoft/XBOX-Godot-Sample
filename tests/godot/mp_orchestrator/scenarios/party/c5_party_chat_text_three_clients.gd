extends "res://scenarios/_base/mp_scenario_base.gd"

const Flow := preload("res://scenarios/_base/party_flows.gd")

const SCENARIO_ID: String = "party.chat.text.three_clients"
const SCENARIO_NAME: String = "Three-client Party text chat"
const PRIORITY: String = "P1"
const CATEGORY: String = "party"
const REQUIRED_ROLES: Array[String] = ["host", "guest", "guest2"]
const REQUIRED_CAPABILITIES: Array[String] = ["playfab_party_available", "live_write_allowed", "multi_host_processes"]
const TIMEOUT_SEC: int = 300

func run(orch) -> Dictionary:
	return await Flow.new().run_party_chat_text_three_clients(orch)
