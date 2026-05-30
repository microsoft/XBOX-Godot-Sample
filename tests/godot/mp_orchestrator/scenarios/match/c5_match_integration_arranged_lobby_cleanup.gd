extends "res://scenarios/_base/mp_scenario_base.gd"

const Flow := preload("res://scenarios/_base/match_flows.gd")

const SCENARIO_ID: String = "match.integration.arranged_lobby_cleanup"
const SCENARIO_NAME: String = "Leaving arranged lobby cleans handles"
const PRIORITY: String = "P0"
const CATEGORY: String = "match"
const REQUIRED_ROLES: Array[String] = ["host", "guest"]
const REQUIRED_CAPABILITIES: Array[String] = ["playfab_multiplayer_available", "matchmaking_queue_configured", "live_write_allowed", "multi_host_processes"]
const TIMEOUT_SEC: int = 300

func run(orch) -> Dictionary:
	return await Flow.new().run_match_integration_arranged_lobby_cleanup(orch)
