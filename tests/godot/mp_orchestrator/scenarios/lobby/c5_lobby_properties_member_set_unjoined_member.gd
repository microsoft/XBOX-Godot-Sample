extends "res://scenarios/_base/mp_scenario_base.gd"

const Flow := preload("res://scenarios/_base/lobby_flows.gd")

const SCENARIO_ID: String = "lobby.properties.member.set.unjoined_member"
const SCENARIO_NAME: String = "Set member properties without lobby is rejected"
const PRIORITY: String = "P1"
const CATEGORY: String = "lobby"
const REQUIRED_ROLES: Array[String] = ["guest"]
const REQUIRED_CAPABILITIES: Array[String] = ["playfab_multiplayer_available"]
const TIMEOUT_SEC: int = 120

func run(orch) -> Dictionary:
	return await Flow.new().run_lobby_properties_member_set_unjoined_member(orch)
