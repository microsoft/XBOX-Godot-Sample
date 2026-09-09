extends "res://scenarios/_base/mp_scenario_base.gd"

const Flow := preload("res://scenarios/_base/lobby_flows.gd")

const SCENARIO_ID: String = "lobby.update.membership_lock"
const SCENARIO_NAME: String = "Membership lock propagates and blocks new joins"
const PRIORITY: String = "P1"
const CATEGORY: String = "lobby"
const REQUIRED_ROLES: Array[String] = ["host", "guest", "guest2"]
const REQUIRED_CAPABILITIES: Array[String] = ["playfab_multiplayer_available", "live_write_allowed", "multi_host_processes"]
const TIMEOUT_SEC: int = 240

func run(orch) -> Dictionary:
	return await Flow.new().run_lobby_update_membership_lock(orch)
