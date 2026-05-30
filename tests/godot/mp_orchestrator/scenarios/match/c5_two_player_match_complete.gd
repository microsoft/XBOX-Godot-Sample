## C5a: two clients create tickets in the same queue and both reach matched.
##
## Ports the legacy `two-player match completion` scenario. Arranged-lobby join
## remains deferred until the mp_test_client exposes a join_arranged_lobby op.
extends "res://scenarios/_base/mp_scenario_base.gd"

const SCENARIO_ID: String = "match.ticket.two_player_match.complete"
const SCENARIO_NAME: String = "Two players in the same queue both reach matched"
const CATEGORY: String = "match"
const PRIORITY: String = "P0"
const REQUIRED_ROLES: Array[String] = ["host", "guest"]
const REQUIRED_CAPABILITIES: Array[String] = ["playfab_multiplayer_available", "matchmaking_queue_configured", "multi_host_processes"]
const TIMEOUT_SEC: int = 300


func run(orch) -> Dictionary:
	if orch.env("LIVE_TESTS", "") != "1":
		return skip("LIVE_TESTS != 1")
	if orch.env("PLAYFAB_TITLE_ID", "").is_empty():
		return skip("PLAYFAB_TITLE_ID not set")

	var queue_name: String = orch.env("PLAYFAB_MULTIPLAYER_MATCH_QUEUE", "")
	if queue_name.is_empty():
		return skip("PLAYFAB_MULTIPLAYER_MATCH_QUEUE not set")

	var host = orch.client("host")
	var guest = orch.client("guest")

	var host_signed: Dictionary = await host.send("sign_in", {}, 60_000)
	var err: Variant = assert_ok(host_signed, "host sign_in failed")
	if err != null:
		return err
	var guest_signed: Dictionary = await guest.send("sign_in", {}, 60_000)
	err = assert_ok(guest_signed, "guest sign_in failed")
	if err != null:
		return err

	var run_tag: String = "two-player-%d" % Time.get_unix_time_from_system()
	var host_created: Dictionary = await host.send("create_match_ticket", {
		"as": "ticket",
		"queue_name": queue_name,
		"timeout_seconds": 120,
		"attributes": { "scenario": "two_player_match", "run_id": run_tag, "role": "host" },
	}, 60_000)
	err = assert_ok(host_created, "host create_match_ticket failed")
	if err != null:
		return err
	var guest_created: Dictionary = await guest.send("create_match_ticket", {
		"as": "ticket",
		"queue_name": queue_name,
		"timeout_seconds": 120,
		"attributes": { "scenario": "two_player_match", "run_id": run_tag, "role": "guest" },
	}, 60_000)
	err = assert_ok(guest_created, "guest create_match_ticket failed")
	if err != null:
		return err

	var host_ticket_id: String = String(host_created.get("result", {}).get("ticket", {}).get("ticket_id", ""))
	var guest_ticket_id: String = String(guest_created.get("result", {}).get("ticket", {}).get("ticket_id", ""))
	if host_ticket_id.is_empty() or guest_ticket_id.is_empty():
		return fail("create_match_ticket returned an empty ticket_id", { "host": host_created.get("result", {}), "guest": guest_created.get("result", {}) })

	var host_matched: Dictionary = await host.send("wait_match_ticket", {
		"handle": "ticket",
		"status_name": "matched",
		"timeout_ms": 180_000,
	}, 190_000)
	err = assert_ok(host_matched, "host wait_match_ticket(matched) failed")
	if err != null:
		return err
	var guest_matched: Dictionary = await guest.send("wait_match_ticket", {
		"handle": "ticket",
		"status_name": "matched",
		"timeout_ms": 180_000,
	}, 190_000)
	err = assert_ok(guest_matched, "guest wait_match_ticket(matched) failed")
	if err != null:
		return err

	var host_ticket: Dictionary = host_matched.get("result", {}).get("ticket", {})
	var guest_ticket: Dictionary = guest_matched.get("result", {}).get("ticket", {})
	var host_match_id: String = String(host_ticket.get("match_id", ""))
	var guest_match_id: String = String(guest_ticket.get("match_id", ""))
	if host_match_id.is_empty() or guest_match_id.is_empty():
		return fail("matched tickets did not include match_id", { "host_ticket": host_ticket, "guest_ticket": guest_ticket })
	err = assert_eq(host_match_id, guest_match_id, "host and guest matched into the same match_id")
	if err != null:
		return err
	if String(host_ticket.get("arranged_lobby_connection_string", "")).is_empty() or String(guest_ticket.get("arranged_lobby_connection_string", "")).is_empty():
		return fail("matched tickets did not include arranged lobby connection strings", { "host_ticket": host_ticket, "guest_ticket": guest_ticket })

	return ok({
		"queue_name": queue_name,
		"host_ticket_id": host_ticket_id,
		"guest_ticket_id": guest_ticket_id,
		"match_id": host_match_id,
	})
