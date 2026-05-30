extends "res://scenarios/_base/mp_scenario_base.gd"

const Flow := preload("res://scenarios/_base/party_flows.gd")

const SCENARIO_ID: String = "e2e.full_session.match_then_party_play"
const SCENARIO_NAME: String = "Match then Party gameplay session"
const PRIORITY: String = "P1"
const CATEGORY: String = "e2e"
const REQUIRED_ROLES: Array[String] = ["host", "guest"]
const REQUIRED_CAPABILITIES: Array[String] = ["playfab_party_available", "playfab_multiplayer_available", "matchmaking_queue_configured", "live_write_allowed", "multi_host_processes"]
const TIMEOUT_SEC: int = 420

func run(orch) -> Dictionary:
	return await Flow.new().run_e2e_full_session_match_then_party_play(orch)
