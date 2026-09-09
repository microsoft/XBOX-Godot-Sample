extends "res://scenarios/_base/mp_scenario_base.gd"

const Flow := preload("res://scenarios/_base/lobby_flows.gd")

const SCENARIO_ID: String = "lobby.disconnect.reason"
const SCENARIO_NAME: String = "Deliberate leave reports NoLocalMembers and a successful disconnect"
const PRIORITY: String = "P1"
const CATEGORY: String = "lobby"
const REQUIRED_ROLES: Array[String] = ["host", "guest"]
const REQUIRED_CAPABILITIES: Array[String] = ["playfab_multiplayer_available", "live_write_allowed", "multi_host_processes"]
const TIMEOUT_SEC: int = 180

func run(orch) -> Dictionary:
	return await Flow.new().run_lobby_disconnect_reason_on_leave(orch)
