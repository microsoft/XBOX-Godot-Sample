extends "res://scenarios/_base/mp_scenario_base.gd"

const Flow := preload("res://scenarios/_base/lobby_flows.gd")

const SCENARIO_ID: String = "lobby.search.invalid_filter_string"
const SCENARIO_NAME: String = "Invalid search filter returns typed error"
const PRIORITY: String = "P1"
const CATEGORY: String = "lobby"
const REQUIRED_ROLES: Array[String] = ["observer"]
const REQUIRED_CAPABILITIES: Array[String] = ["playfab_multiplayer_available"]
const TIMEOUT_SEC: int = 120

func run(orch) -> Dictionary:
	return await Flow.new().run_lobby_search_invalid_filter_string(orch)
