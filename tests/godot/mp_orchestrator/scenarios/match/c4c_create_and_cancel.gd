## C4c: create a match ticket, cancel it, observe `cancelled` status.
##
## Ports the legacy `match ticket create and cancel` smoke (live-gated). The
## single-client cancel path is one of the few match-ticket scenarios that
## doesn't need a second peer in the same queue, so it's the C4c canary that
## proves matchmaking command plumbing end-to-end before the multi-role
## scenarios land in C5.
##
## Cleanup is handled by the orchestrator's mandatory reset_client after
## every scenario per spec/playfab-multiplayer-test-automation/3-harness-spec.md
## — scenarios do not need to cancel/reset on error paths.
extends "res://scenarios/_base/mp_scenario_base.gd"

const SCENARIO_ID: String = "match.ticket.create_and_cancel"
const SCENARIO_NAME: String = "Host creates a ticket, cancels it, ticket reaches cancelled"
const CATEGORY: String = "match"
const PRIORITY: String = "P0"
const REQUIRED_ROLES: Array[String] = ["host"]
const REQUIRED_CAPABILITIES: Array[String] = ["playfab_multiplayer_available", "live_write_allowed"]
const TIMEOUT_SEC: int = 180


func run(orch) -> Dictionary:
	var live_gate: Variant = requires_live_write(orch)
	if live_gate != null:
		return live_gate

	# Custom id derived in the test client from --role + env (see
	# tests/godot/mp_test_client/scripts/test_client.gd::_derive_custom_id_for_role).
	# Default mirrors tools/configure_playfab_test_title.ps1's provisioned queue.
	var queue_name: String = orch.env("PLAYFAB_MULTIPLAYER_MATCH_QUEUE", "godot_gdk_ext_live_smoke_queue")
	var host = orch.client("host")

	var signed_in: Dictionary = await host.send("sign_in", {}, 60_000)
	var err: Variant = assert_ok(signed_in, "sign_in failed")
	if err != null:
		return err

	var created: Dictionary = await host.send("create_match_ticket", {
		"as": "ticket",
		"queue_name": queue_name,
		"timeout_seconds": 60,
		"attributes": { "skill": 1 },
	}, 60_000)
	err = assert_ok(created, "create_match_ticket failed")
	if err != null:
		return err

	var created_ticket: Dictionary = created.get("result", {}).get("ticket", {})
	var ticket_id: String = String(created_ticket.get("ticket_id", ""))
	if ticket_id.is_empty():
		return fail("create_match_ticket returned empty ticket_id", { "result": created.get("result", {}) })

	# Cancel and verify the ticket reaches terminal state. We use the
	# explicit cancel command rather than wait_match_ticket("matched") here —
	# there's no second peer in the queue, so a "matched" wait would only ever
	# time out.
	var cancelled: Dictionary = await host.send("cancel_match_ticket", { "ticket_id": ticket_id }, 30_000)
	err = assert_ok(cancelled, "cancel_match_ticket failed")
	if err != null:
		return err

	var cancel_ticket: Dictionary = cancelled.get("result", {}).get("ticket", {})
	err = assert_eq(String(cancel_ticket.get("ticket_id", "")), ticket_id, "cancel returned different ticket_id")
	if err != null:
		return err
	err = assert_true(bool(cancel_ticket.get("is_cancelled", false)), "ticket should report is_cancelled after cancel")
	if err != null:
		return err

	return ok({
		"ticket_id": ticket_id,
		"queue_name": queue_name,
		"status_name": String(cancel_ticket.get("status_name", "")),
	})
