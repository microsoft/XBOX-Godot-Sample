extends "res://scenarios/_base/mp_scenario_base.gd"

const Flow := preload("res://scenarios/_base/lobby_flows.gd")

const SCENARIO_ID: String = "lobby.join.by_connection_string"
const SCENARIO_NAME: String = "Client joins by connection string"
const PRIORITY: String = "P0"
const CATEGORY: String = "lobby"
const REQUIRED_ROLES: Array[String] = ["host", "guest"]
const REQUIRED_CAPABILITIES: Array[String] = ["playfab_multiplayer_available", "live_write_allowed", "multi_host_processes"]
const TIMEOUT_SEC: int = 120

func run(orch) -> Dictionary:
	return await Flow.new().run_lobby_join_by_connection_string(orch)
