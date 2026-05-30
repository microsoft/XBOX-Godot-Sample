extends "res://scenarios/_base/mp_scenario_base.gd"

const Flow := preload("res://scenarios/_base/party_flows.gd")

const SCENARIO_ID: String = "party_match.descriptor_via_arranged_lobby_property"
const SCENARIO_NAME: String = "Party descriptor exchanged via arranged lobby property"
const PRIORITY: String = "P1"
const CATEGORY: String = "party_match"
const REQUIRED_ROLES: Array[String] = ["host", "guest"]
const REQUIRED_CAPABILITIES: Array[String] = ["playfab_party_available", "playfab_multiplayer_available", "matchmaking_queue_configured", "live_write_allowed", "multi_host_processes"]
const TIMEOUT_SEC: int = 360

func run(orch) -> Dictionary:
	return await Flow.new().run_party_match_descriptor_via_arranged_lobby_property(orch)
