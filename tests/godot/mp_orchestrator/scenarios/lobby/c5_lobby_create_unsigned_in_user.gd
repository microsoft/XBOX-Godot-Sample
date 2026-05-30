extends "res://scenarios/_base/mp_scenario_base.gd"

const Flow := preload("res://scenarios/_base/lobby_flows.gd")

const SCENARIO_ID: String = "lobby.create.unsigned_in_user"
const SCENARIO_NAME: String = "Create with unsigned-in user rejected"
const PRIORITY: String = "P1"
const CATEGORY: String = "lobby"
const REQUIRED_ROLES: Array[String] = ["host"]
const REQUIRED_CAPABILITIES: Array[String] = []
const TIMEOUT_SEC: int = 60

func run(orch) -> Dictionary:
	return await Flow.new().run_lobby_create_unsigned_in_user(orch)
