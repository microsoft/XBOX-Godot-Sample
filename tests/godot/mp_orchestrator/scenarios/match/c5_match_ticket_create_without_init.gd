extends "res://scenarios/_base/mp_scenario_base.gd"

const Flow := preload("res://scenarios/_base/match_flows.gd")

const SCENARIO_ID: String = "match.ticket.create_without_init"
const SCENARIO_NAME: String = "Create ticket before multiplayer init is rejected"
const PRIORITY: String = "P1"
const CATEGORY: String = "match"
const REQUIRED_ROLES: Array[String] = ["host"]
const REQUIRED_CAPABILITIES: Array[String] = ["playfab_multiplayer_available", "matchmaking_queue_configured"]
const TIMEOUT_SEC: int = 120

func run(orch) -> Dictionary:
	return await Flow.new().run_match_ticket_create_without_init(orch)
