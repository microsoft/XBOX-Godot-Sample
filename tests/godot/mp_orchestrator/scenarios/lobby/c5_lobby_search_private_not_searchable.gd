## C5a: host creates a PRIVATE lobby with a unique string_key1 tag; observer
## searches with that exact filter; the private lobby must NOT be returned.
##
## This is the inverse of `lobby.search_public_by_string_key`. PlayFab Lobby
## search returns ONLY publicly searchable lobbies (access_policy = 0); private
## lobbies are addressable by connection_string but not by search.
##
## Two-role live-gated scenario (host + observer). Maps to
## `lobby.search_private_not_searchable` in
## spec/playfab-multiplayer-test-automation/1-test-matrix.md.
##
## Cleanup is handled by the orchestrator's mandatory reset_client after every
## scenario.
extends "res://scenarios/_base/mp_scenario_base.gd"

const SCENARIO_ID: String = "lobby.search_private_not_searchable"
const SCENARIO_NAME: String = "Private lobby is never returned by search_lobbies"
const CATEGORY: String = "lobby"
const PRIORITY: String = "P0"
const REQUIRED_ROLES: Array[String] = ["host", "observer"]
const REQUIRED_CAPABILITIES: Array[String] = ["playfab_multiplayer_available", "multi_host_processes"]
const TIMEOUT_SEC: int = 180

# Negative case — we don't need to wait long. A few well-spaced polls are
# enough to surface the failure if indexing happened to leak a private lobby.
const NEGATIVE_POLL_ATTEMPTS: int = 4
const NEGATIVE_POLL_INTERVAL_MS: int = 3_000


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

	# Deterministic per-run unique tag — only this scenario will set it.
	var unique_tag: String = "c5a-prv-%d" % Time.get_unix_time_from_system()

	# access_policy = 2 → PlayFabLobbyConfig.ACCESS_POLICY_PRIVATE.
	var created: Dictionary = await host.send("create_lobby", {
		"max_players": 4,
		"access_policy": 2,
		"search_properties": { "string_key1": unique_tag },
		"lobby_properties": { "scenario": "search_private_not_searchable" },
	}, 60_000)
	err = assert_ok(created, "host create_lobby failed")
	if err != null:
		return err
	var lobby_id: String = String(created.get("result", {}).get("lobby", {}).get("lobby_id", ""))
	if lobby_id.is_empty():
		return fail("create_lobby returned empty lobby_id", { "result": created.get("result", {}) })

	var filter_str: String = "string_key1 eq '%s'" % unique_tag

	# Poll a few times to make sure the private lobby never sneaks into
	# results — if it ever shows up, that's a release-blocking regression.
	for attempt in range(NEGATIVE_POLL_ATTEMPTS):
		var search_resp: Dictionary = await observer.send("search_lobbies", {
			"filter": filter_str,
			"max_results": 10,
		}, 30_000)
		if not bool(search_resp.get("ok", false)):
			return fail("observer search_lobbies failed", { "response": search_resp })
		var lobbies: Array = search_resp.get("result", {}).get("lobbies", [])
		for entry in lobbies:
			if String(entry.get("lobby_id", "")) == lobby_id:
				return fail("private lobby unexpectedly returned by search", {
					"filter": filter_str,
					"lobby_id": lobby_id,
					"result_count": lobbies.size(),
				})
		if attempt + 1 < NEGATIVE_POLL_ATTEMPTS:
			await _sleep_ms(NEGATIVE_POLL_INTERVAL_MS)

	return ok({ "lobby_id": lobby_id, "filter": filter_str, "poll_attempts": NEGATIVE_POLL_ATTEMPTS })


func _sleep_ms(ms: int) -> void:
	var tree: SceneTree = Engine.get_main_loop() as SceneTree
	if tree == null:
		return
	var timer: SceneTreeTimer = tree.create_timer(float(ms) / 1000.0)
	await timer.timeout
