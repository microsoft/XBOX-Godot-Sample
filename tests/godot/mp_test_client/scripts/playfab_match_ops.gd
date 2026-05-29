## PlayFabMatchOps — handle-based match-ticket command implementations.
##
## Mirrors the legacy worker.gd _op_create_match_ticket / _op_inspect_match_ticket /
## _op_wait_match_ticket / _op_cancel_match_ticket handlers (worker.gd:358-422), but
## tracks tickets by orchestrator-supplied handle instead of a singleton
## `_primary_ticket`.
##
## Handle addressing (per spec/playfab-multiplayer-test-automation/4-scenario-authoring.md):
##   * `params.as` on create_match_ticket names the new ticket (default "main").
##   * `params.handle` on subsequent ops addresses a tracked ticket (default "main").
##
## Each command returns the CommandDispatcher shape:
##   { ok: bool, result?: Dictionary, error?: Dictionary }
extends RefCounted

const PlayFabRuntime := preload("res://scripts/playfab_runtime.gd")

const DEFAULT_HANDLE := "main"
const DEFAULT_CREATE_TIMEOUT_MS := 60_000
const DEFAULT_WAIT_TIMEOUT_MS := 120_000
const DEFAULT_CANCEL_TIMEOUT_MS := 30_000
const DEFAULT_QUEUE_TIMEOUT_SEC := 60
const POLL_INTERVAL_MS := 25
const TERMINAL_STATUSES: PackedStringArray = ["cancelled", "failed"]
# PFMultiplayerCreateMatchmakingTicket returns synchronously with a ticket
# handle still in `Creating` state. The ticket_id is populated when the
# first PFMatchmakingStateChangeType::TicketStatusChanged event arrives
# (typically within 1-2s). Scenarios need the ticket_id to address the
# ticket externally, so create_match_ticket waits for it to populate before
# returning — independent of the longer match-found wait gated by
# wait_match_ticket().
const TICKET_ID_POPULATE_TIMEOUT_MS := 20_000

var _runtime: PlayFabRuntime = null
var _tickets: Dictionary = {}  # handle (String) -> PlayFabMatchTicket (Object)


func bind(runtime: PlayFabRuntime) -> void:
	_runtime = runtime


func handles() -> Array:
	return _tickets.keys()


func has_ticket(handle: String) -> bool:
	return _tickets.has(handle) and _tickets[handle] != null


# ---------------------------------------------------------------------------
# Commands
# ---------------------------------------------------------------------------

func create_match_ticket(params: Dictionary) -> Dictionary:
	var session_err: Dictionary = _require_session("create_match_ticket")
	if not session_err.is_empty():
		return session_err
	var handle: String = String(params.get("as", DEFAULT_HANDLE))
	if has_ticket(handle):
		return _err("handle_in_use", "match-ticket handle '%s' is already tracked" % handle)

	var queue_name: String = String(params.get("queue_name", "")).strip_edges()
	if queue_name.is_empty():
		return _err("matchmaking_queue_unconfigured", "create_match_ticket requires a non-empty queue_name")

	var config: Object = _instantiate("PlayFabMatchmakingTicketConfig")
	if config == null:
		return _err("class_unavailable", "PlayFabMatchmakingTicketConfig not registered in ClassDB")
	config.queue_name = queue_name
	config.timeout_seconds = int(params.get("timeout_seconds", DEFAULT_QUEUE_TIMEOUT_SEC))

	var member: Object = _instantiate("PlayFabMatchmakingMember")
	if member == null:
		return _err("class_unavailable", "PlayFabMatchmakingMember not registered in ClassDB")
	member.user = _runtime.get_user()
	member.attributes = params.get("attributes", {})
	config.members = [member]

	var result: Variant = await _runtime.await_completion_with_rate_limit_retry(
		func(): return _runtime.get_multiplayer().create_match_ticket_async(_runtime.get_user(), config),
		"create_match_ticket_async",
		int(params.get("timeout_ms", DEFAULT_CREATE_TIMEOUT_MS)),
	)
	if result == null or not bool(result.ok):
		return _err_from_result(result, "create_match_ticket_async")
	var ticket: Object = result.data
	if ticket == null:
		return _err("invalid_response", "create_match_ticket_async returned ok with null ticket")
	# create_match_ticket_async resolves as soon as the local handle is
	# allocated; ticket_id is filled in asynchronously by the SDK's first
	# TicketStatusChanged dispatch. Block until it populates so scenarios
	# always see a non-empty id in the response payload.
	var populate_timeout_ms: int = int(params.get("ticket_id_timeout_ms", TICKET_ID_POPULATE_TIMEOUT_MS))
	var populated: bool = await _runtime.wait_until(
		func(): return not String(ticket.get_ticket_id()).is_empty(),
		populate_timeout_ms,
	)
	if not populated:
		# Roll back the local allocation so reset_client doesn't try to
		# cancel a half-formed ticket whose handle never carried an id.
		await _runtime.await_completion(ticket.cancel_async(), DEFAULT_CANCEL_TIMEOUT_MS)
		return _err(
			"match_ticket_id_timeout",
			"PFMatchmakingTicket id did not populate within %dms" % populate_timeout_ms,
		)
	_tickets[handle] = ticket
	return _ok({ "handle": handle, "ticket": _ticket_snapshot(ticket) })


func inspect_match_ticket(params: Dictionary) -> Dictionary:
	var lookup: Dictionary = _lookup_ticket(params, "inspect_match_ticket")
	if lookup.has("ok") and not bool(lookup["ok"]):
		return lookup
	var ticket: Object = lookup["ticket"]
	return _ok({ "handle": lookup["handle"], "ticket": _ticket_snapshot(ticket) })


func wait_match_ticket(params: Dictionary) -> Dictionary:
	var lookup: Dictionary = _lookup_ticket(params, "wait_match_ticket")
	if lookup.has("ok") and not bool(lookup["ok"]):
		return lookup
	var ticket: Object = lookup["ticket"]
	var expected_status: String = String(params.get("status_name", "matched")).strip_edges()
	var timeout_ms: int = int(params.get("timeout_ms", DEFAULT_WAIT_TIMEOUT_MS))
	var deadline_ms: int = Time.get_ticks_msec() + timeout_ms
	var current_status: String = ""
	var tree: SceneTree = _runtime_tree()
	while Time.get_ticks_msec() < deadline_ms:
		_runtime.dispatch()
		var snapshot: Dictionary = _ticket_snapshot(ticket)
		current_status = String(snapshot.get("status_name", ""))
		if current_status == expected_status:
			return _ok({ "handle": lookup["handle"], "ticket": snapshot })
		if current_status in TERMINAL_STATUSES and current_status != expected_status:
			return _err(
				"match_ticket_%s" % current_status,
				"match ticket reached terminal status '%s' before '%s'" % [current_status, expected_status],
			)
		if tree != null:
			await tree.process_frame
		OS.delay_msec(POLL_INTERVAL_MS)
	return _err(
		"timeout",
		"timed out waiting for match-ticket status '%s' (last='%s')" % [expected_status, current_status],
	)


func cancel_match_ticket(params: Dictionary) -> Dictionary:
	var lookup: Dictionary = _lookup_ticket(params, "cancel_match_ticket")
	if lookup.has("ok") and not bool(lookup["ok"]):
		return lookup
	var handle: String = lookup["handle"]
	var ticket: Object = lookup["ticket"]
	var result: Variant = await _runtime.await_completion_with_rate_limit_retry(
		func(): return ticket.cancel_async(),
		"cancel_match_ticket_async",
		int(params.get("timeout_ms", DEFAULT_CANCEL_TIMEOUT_MS)),
	)
	if result == null or not bool(result.ok):
		return _err_from_result(result, "PlayFabMatchTicket.cancel_async")
	var snapshot: Dictionary = _ticket_snapshot(ticket)
	_tickets.erase(handle)
	return _ok({ "handle": handle, "ticket": snapshot })


## Leaves every tracked ticket. Called from reset_client between scenarios so a
## scenario can't leak a tracked ticket into the next scenario's lifecycle.
func reset(_params: Dictionary) -> Dictionary:
	var cancelled: int = 0
	var failures: Array = []
	for handle in _tickets.keys():
		var ticket: Object = _tickets[handle]
		if ticket == null:
			continue
		if bool(ticket.is_complete()):
			continue
		var result: Variant = await _runtime.await_completion(ticket.cancel_async(), DEFAULT_CANCEL_TIMEOUT_MS)
		if result != null and bool(result.ok):
			cancelled += 1
		else:
			# Surface failures so reset_client returns an error and the
			# orchestrator respawns the client process. Silently dropping
			# the ticket leaves it queued server-side and would race with
			# the next scenario's ticket-create rate limit.
			var err_payload: Dictionary = _err_from_result(result, "cancel_async")
			var err_details: Dictionary = err_payload.get("error", {})
			err_details["handle"] = handle
			failures.append(err_details)
	_tickets.clear()
	if not failures.is_empty():
		return _err(
			"reset_failed",
			"failed to cancel %d match ticket(s) during reset; respawn required" % failures.size(),
			{ "cancelled": cancelled, "failed": failures },
		)
	return _ok({ "cancelled": cancelled })


# ---------------------------------------------------------------------------
# Helpers
# ---------------------------------------------------------------------------

func _require_session(op: String) -> Dictionary:
	if _runtime == null or not _runtime.has_user():
		return _err("not_signed_in", "%s requires sign_in first" % op)
	if _runtime.get_multiplayer() == null or not _runtime.get_multiplayer().is_initialized():
		return _err("multiplayer_not_initialized", "%s requires multiplayer.initialize first" % op)
	return {}


func _lookup_ticket(params: Dictionary, op: String) -> Dictionary:
	var handle: String = String(params.get("handle", DEFAULT_HANDLE))
	if not has_ticket(handle):
		return _err("unknown_handle", "%s: no tracked match ticket for handle '%s' (have %s)" % [op, handle, str(_tickets.keys())])
	return { "handle": handle, "ticket": _tickets[handle] }


func _ticket_snapshot(ticket: Object) -> Dictionary:
	if ticket == null:
		return {}
	var properties: Dictionary = ticket.get_properties()
	return {
		"ticket_id": ticket.get_ticket_id(),
		"queue_name": ticket.get_queue_name(),
		"status": ticket.get_status(),
		"status_name": String(properties.get("status_name", "")),
		"match_id": ticket.get_match_id(),
		"arranged_lobby_connection_string": ticket.get_arranged_lobby_connection_string(),
		"is_complete": ticket.is_complete(),
		"is_cancelled": ticket.is_cancelled(),
		"properties": properties,
	}


func _runtime_tree() -> SceneTree:
	if _runtime == null:
		return null
	return Engine.get_main_loop() as SceneTree


func _instantiate(class_name_str: String) -> Object:
	if not ClassDB.class_exists(class_name_str) or not ClassDB.can_instantiate(class_name_str):
		return null
	return ClassDB.instantiate(class_name_str)


func _ok(result: Dictionary) -> Dictionary:
	return { "ok": true, "result": result }


func _err(code: String, message: String, details: Dictionary = {}) -> Dictionary:
	var err: Dictionary = { "code": code, "message": message }
	for key in details.keys():
		err[key] = details[key]
	return { "ok": false, "error": err }


func _err_from_result(result: Variant, label: String) -> Dictionary:
	if result == null:
		return _err("timeout", "%s timed out" % label)
	var code: String = String(result.code) if "code" in result else "playfab_error"
	var message: String = String(result.message) if "message" in result else "%s failed" % label
	return { "ok": false, "error": { "code": code, "message": message, "call": label } }
