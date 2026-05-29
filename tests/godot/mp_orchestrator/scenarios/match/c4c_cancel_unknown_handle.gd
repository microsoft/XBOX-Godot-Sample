## C4c: cancel_match_ticket against an unknown handle is rejected with a
## typed error.
##
## Fully offline: no PlayFab calls happen — the client's PlayFabMatchOps
## handler rejects the request before it would dispatch to the addon. Validates
## the dispatcher's `_lookup_ticket` hygiene per
## spec/playfab-multiplayer-test-automation/1-test-matrix.md
## (`match.ticket.cancel_unknown_handle`).
extends "res://scenarios/_base/mp_scenario_base.gd"

const SCENARIO_ID: String = "match.ticket.cancel_unknown_handle"
const SCENARIO_NAME: String = "Cancel against unknown handle returns typed error"
const CATEGORY: String = "match"
const PRIORITY: String = "P2"
const REQUIRED_ROLES: Array[String] = ["host"]
const REQUIRED_CAPABILITIES: Array[String] = []
const TIMEOUT_SEC: int = 30


func run(orch) -> Dictionary:
	var host = orch.client("host")

	# Hardcode a handle that no scenario in the suite would ever use, so we
	# don't accidentally cancel a real tracked ticket if scenario ordering ever
	# changes.
	var response: Dictionary = await host.send("cancel_match_ticket", {
		"handle": "__definitely_not_a_real_handle__",
	}, 10_000)

	# We expect the dispatcher's "ok": false envelope (not a transport error).
	if bool(response.get("ok", false)):
		return fail("cancel_match_ticket on unknown handle unexpectedly succeeded", {
			"response": response,
		})

	var error: Dictionary = response.get("error", {})
	var code: String = String(error.get("code", ""))
	var err: Variant = assert_eq(code, "unknown_handle", "expected unknown_handle, got: %s" % str(error))
	if err != null:
		return err

	return ok({ "rejected_with": code })
