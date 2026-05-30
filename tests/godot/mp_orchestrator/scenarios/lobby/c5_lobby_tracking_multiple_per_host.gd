extends "res://scenarios/_base/mp_scenario_base.gd"

const Flow := preload("res://scenarios/_base/lobby_flows.gd")

const SCENARIO_ID: String = "lobby.tracking.multiple_lobbies_per_host"
const SCENARIO_NAME: String = "Host tracks multiple lobbies"
const PRIORITY: String = "P1"
const CATEGORY: String = "lobby"
const REQUIRED_ROLES: Array[String] = ["host"]
const REQUIRED_CAPABILITIES: Array[String] = ["playfab_multiplayer_available", "live_write_allowed"]
const TIMEOUT_SEC: int = 120

func run(orch) -> Dictionary:
	return await Flow.new().run_lobby_tracking_multiple_lobbies_per_host(orch)
