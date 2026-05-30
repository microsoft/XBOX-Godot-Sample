extends "res://scenarios/_base/mp_scenario_base.gd"

const Flow := preload("res://scenarios/_base/lobby_flows.gd")

const SCENARIO_ID: String = "lobby.join.invalid_connection_string"
const SCENARIO_NAME: String = "Invalid connection string fails predictably"
const PRIORITY: String = "P0"
const CATEGORY: String = "lobby"
const REQUIRED_ROLES: Array[String] = ["guest"]
const REQUIRED_CAPABILITIES: Array[String] = ["playfab_multiplayer_available"]
const TIMEOUT_SEC: int = 120

func run(orch) -> Dictionary:
	return await Flow.new().run_lobby_join_invalid_connection_string(orch)
