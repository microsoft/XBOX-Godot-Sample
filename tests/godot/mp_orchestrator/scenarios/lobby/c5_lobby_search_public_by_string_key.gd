## C5a: host creates a public lobby with a unique string_key1 tag; observer
## searches for it via OData `string_key1 eq '<tag>'` filter and finds it.
##
## Two-role live-gated scenario (host + observer). Maps to
## `lobby.search_public_by_string_key` in
## spec/playfab-multiplayer-test-automation/1-test-matrix.md.
##
## Cleanup is handled by the orchestrator's mandatory reset_client after every
## scenario.
extends "res://scenarios/_base/mp_scenario_base.gd"

const SCENARIO_ID: String = "lobby.search_public_by_string_key"
const SCENARIO_NAME: String = "Observer finds host's public lobby by string_key1 filter"
const CATEGORY: String = "lobby"
const PRIORITY: String = "P0"
const REQUIRED_ROLES: Array[String] = ["host", "observer"]
const REQUIRED_CAPABILITIES: Array[String] = ["playfab_multiplayer_available", "multi_host_processes"]
const TIMEOUT_SEC: int = 180

const SEARCH_CONVERGENCE_TIMEOUT_MS: int = 45_000
const SEARCH_POLL_INTERVAL_MS: int = 1_500


func run(orch) -> Dictionary:
	if orch.env("LIVE_TESTS", "") != "1":
		return skip("LIVE_TESTS != 1")
	if orch.env("PLAYFAB_TITLE_ID", "").is_empty():
		return skip("PLAYFAB_TITLE_ID not set")

	# Custom ids derived in each test client from --role + env.
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

	# Deterministic per-run unique tag so concurrent runs against the same
	# title don't collide on the OData filter. Time-unix + roll id keeps it
	# stable for the scenario's lifetime.
	var unique_tag: String = "c5a-pub-%d" % Time.get_unix_time_from_system()

	var created: Dictionary = await host.send("create_lobby", {
		"max_players": 4,
		"access_policy": 0,
		"search_properties": { "string_key1": unique_tag },
		"lobby_properties": { "scenario": "search_public_by_string_key" },
	}, 60_000)
	err = assert_ok(created, "host create_lobby failed")
	if err != null:
		return err
	var lobby_id: String = String(created.get("result", {}).get("lobby", {}).get("lobby_id", ""))
	if lobby_id.is_empty():
		return fail("create_lobby returned empty lobby_id", { "result": created.get("result", {}) })

	var filter_str: String = "string_key1 eq '%s'" % unique_tag

	# Search indexing is eventual on PlayFab's side; poll until the host's
	# lobby appears in the observer's result set.
	var deadline_ms: int = Time.get_ticks_msec() + SEARCH_CONVERGENCE_TIMEOUT_MS
	var last_results: Array = []
	while Time.get_ticks_msec() < deadline_ms:
		var search_resp: Dictionary = await observer.send("search_lobbies", {
			"filter": filter_str,
			"max_results": 10,
		}, 30_000)
		if not bool(search_resp.get("ok", false)):
			return fail("observer search_lobbies failed", { "response": search_resp })
		last_results = search_resp.get("result", {}).get("lobbies", [])
		for entry in last_results:
			if String(entry.get("lobby_id", "")) == lobby_id:
				return ok({
					"lobby_id": lobby_id,
					"filter": filter_str,
					"result_count": last_results.size(),
				})
		await _sleep_ms(SEARCH_POLL_INTERVAL_MS)

	return fail(
		"observer search never returned host's lobby_id within %dms" % SEARCH_CONVERGENCE_TIMEOUT_MS,
		{ "filter": filter_str, "expected_lobby_id": lobby_id, "last_results": last_results },
	)


func _sleep_ms(ms: int) -> void:
	# Yield-style sleep using the SceneTree main loop. Keeps the orchestrator
	# I/O pump alive while we wait for PlayFab search indexing to converge.
	var tree: SceneTree = Engine.get_main_loop() as SceneTree
	if tree == null:
		return
	var timer: SceneTreeTimer = tree.create_timer(float(ms) / 1000.0)
	await timer.timeout
