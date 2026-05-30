## C5a: observer search with a non-matching filter returns no host lobby.
##
## Ports the legacy `search no-results isolation` scenario.
extends "res://scenarios/_base/mp_scenario_base.gd"

const SCENARIO_ID: String = "lobby.search.no_results.isolation"
const SCENARIO_NAME: String = "Search filter that excludes created lobby returns no results"
const CATEGORY: String = "lobby"
const PRIORITY: String = "P1"
const REQUIRED_ROLES: Array[String] = ["host", "observer"]
const REQUIRED_CAPABILITIES: Array[String] = ["playfab_multiplayer_available", "multi_host_processes"]
const TIMEOUT_SEC: int = 180


func run(orch) -> Dictionary:
	if orch.env("LIVE_TESTS", "") != "1":
		return skip("LIVE_TESTS != 1")
	if orch.env("PLAYFAB_TITLE_ID", "").is_empty():
		return skip("PLAYFAB_TITLE_ID not set")

	var host = orch.client("host")
	var observer = orch.client("observer")

	var host_signed: Dictionary = await host.send("sign_in", {}, 60_000)
	var err: Variant = assert_ok(host_signed, "host sign_in failed")
	if err != null:
		return err
	var observer_signed: Dictionary = await observer.send("sign_in", {}, 60_000)
	err = assert_ok(observer_signed, "observer sign_in failed")
	if err != null:
		return err

	var unique_tag: String = "c5-no-results-%d" % Time.get_unix_time_from_system()
	var created: Dictionary = await host.send("create_lobby", {
		"as": "search_target",
		"max_players": 2,
		"access_policy": 0,
		"search_properties": { "string_key1": unique_tag },
		"lobby_properties": { "scenario": "search_no_results_isolation" },
	}, 60_000)
	err = assert_ok(created, "host create_lobby failed")
	if err != null:
		return err
	var lobby_id: String = String(created.get("result", {}).get("lobby", {}).get("lobby_id", ""))
	if lobby_id.is_empty():
		return fail("create_lobby returned empty lobby_id", { "result": created.get("result", {}) })

	var missing_filter: String = "string_key1 eq '%s-missing'" % unique_tag
	var search: Dictionary = await observer.send("search_lobbies", {
		"filter": missing_filter,
		"max_results": 10,
	}, 30_000)
	err = assert_ok(search, "observer search_lobbies failed")
	if err != null:
		return err

	var lobbies: Array = search.get("result", {}).get("lobbies", [])
	for entry in lobbies:
		if String(entry.get("lobby_id", "")) == lobby_id:
			return fail("search returned a lobby excluded by the filter", { "filter": missing_filter, "lobby_id": lobby_id, "results": lobbies })
	return ok({ "filter": missing_filter, "result_count": lobbies.size() })
