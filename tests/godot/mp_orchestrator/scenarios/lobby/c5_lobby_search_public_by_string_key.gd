extends "res://scenarios/_base/mp_scenario_base.gd"

const Flow := preload("res://scenarios/_base/lobby_flows.gd")

const SCENARIO_ID: String = "lobby.search.public.by_string_key"
const SCENARIO_NAME: String = "Public lobby search by string key"
const PRIORITY: String = "P0"
const CATEGORY: String = "lobby"
const REQUIRED_ROLES: Array[String] = ["host", "observer"]
const REQUIRED_CAPABILITIES: Array[String] = ["playfab_multiplayer_available", "live_write_allowed", "multi_host_processes"]
const TIMEOUT_SEC: int = 120

func run(orch) -> Dictionary:
	return await Flow.new().run_lobby_search_public_by_string_key(orch)
