extends "res://addons/godot_gdk_tests/playfab_test_base.gd"
## Offline regression coverage for native matchmaking completion ownership.
## Missing hooks fail debug coverage instead of turning these tests pending.

const STATUS_CREATING := 0
const STATUS_JOINING := 1
const STATUS_WAITING_FOR_PLAYERS := 2
const STATUS_WAITING_FOR_MATCH := 3
const STATUS_MATCHED := 4
const STATUS_CANCELLED := 5
const STATUS_FAILED := 6

const EVENT_CREATED := 100
const EVENT_STATUS_CHANGED := 101
const EVENT_COMPLETED := 102
const EVENT_CANCELLED := 103
const EVENT_FAILED := 104

const S_OK := 0
const S_FALSE := 1
const E_ABORT := 0x80004004
const E_FAIL := 0x80004005
const E_UNEXPECTED := 0x8000FFFF
const E_PF_NOT_AUTHENTICATED := 0x8923546B
const E_PF_MATCHMAKING_QUEUE_NOT_FOUND := 0x89235648
const E_PF_SERVICE_UNAVAILABLE := 0x89235491
const E_PF_MATCHMAKING_NUMBER_OF_PLAYERS_IN_TICKET_TOO_LARGE := 0x89235652
const E_PF_MATCHMAKING_NUMBER_OF_PLAYERS_IN_TICKET_TOO_LARGE_SIGNED := -1994172846
# The live SDK completes a requested cancellation with this failure HRESULT and a Canceled status.
const NATIVE_CANCELLED_HRESULT := 0x89236304
const HRESULT_MASK := 0xFFFFFFFF

var _hooks_available := false
var _services: Array = []
var _fixtures: Array = []


func before_each() -> void:
	get_playfab()
	_services.clear()
	_fixtures.clear()
	_hooks_available = true
	var probe = ClassDB.instantiate("PlayFabMultiplayer")
	assert_not_null(probe, "PlayFabMultiplayer fixture service can be instantiated")
	if probe == null:
		_hooks_available = false
		return

	for hook in [
		"_test_begin_match_ticket",
		"_test_matchmaking_batch",
		"_test_set_match_ticket_cancel_result",
		"_test_matchmaking_snapshot",
	]:
		if probe.has_method(hook):
			continue
		_hooks_available = false
		if OS.is_debug_build():
			fail_test("%s is required in debug coverage builds" % hook)
		else:
			pending("Matchmaking completion coverage requires GODOT_PLAYFAB_TEST_HOOKS")
		return


func after_each() -> void:
	for fixture in _fixtures:
		var ticket = fixture.ticket
		var service = fixture.service
		var ticket_callback: Callable = fixture.ticket_callback
		var service_callback: Callable = fixture.service_callback
		if ticket != null and ticket.is_connected("state_changed", ticket_callback):
			ticket.disconnect("state_changed", ticket_callback)
		if service != null and service.is_connected("state_changed", service_callback):
			service.disconnect("state_changed", service_callback)
	for service in _services:
		if service == null:
			continue
		var shutdown_signal = service.shutdown_async()
		if typeof(shutdown_signal) == TYPE_SIGNAL:
			await await_completion(shutdown_signal)
	_fixtures.clear()
	_services.clear()


func _new_service() -> Object:
	if not _hooks_available:
		return null
	var service = ClassDB.instantiate("PlayFabMultiplayer")
	assert_not_null(service, "PlayFabMultiplayer fixture service")
	if service != null:
		_services.append(service)
	return service


func _snapshot(
		status: int,
		ticket_id: String = "offline-ticket",
		match_id: String = "",
		arrangement: String = "") -> Dictionary:
	return {
		"status": status,
		"ticket_id": ticket_id,
		"match_id": match_id,
		"arranged_lobby_connection_string": arrangement,
	}


func _begin(operation: String, snapshot: Dictionary) -> Dictionary:
	var service = _new_service()
	if service == null:
		return {}
	var native_fixture: Dictionary = service._test_begin_match_ticket(operation, snapshot)
	assert_true(native_fixture.has("ticket"), "Fixture returns a ticket")
	assert_true(native_fixture.has("completion"), "Fixture returns a completion slot")
	if not native_fixture.has("ticket"):
		return {}

	var ticket = native_fixture.ticket
	var fixture := {
		"service": service,
		"ticket": ticket,
		"completion": native_fixture.get("completion"),
		"factory_results": [],
		"ticket_events": [],
		"ticket_event_observations": [],
		"service_events": [],
		"service_event_observations": [],
	}

	var factory_results: Array = fixture.factory_results
	var completion = fixture.completion
	if typeof(completion) == TYPE_SIGNAL:
		completion.connect(func(result):
			factory_results.append(result)
		)

	var ticket_events: Array = fixture.ticket_events
	var ticket_event_observations: Array = fixture.ticket_event_observations
	var ticket_callback := func(change):
		ticket_events.append(change)
		ticket_event_observations.append(service._test_matchmaking_snapshot(ticket))
	ticket.state_changed.connect(ticket_callback)

	var service_events: Array = fixture.service_events
	var service_event_observations: Array = fixture.service_event_observations
	var service_callback := func(change):
		service_events.append(change)
		service_event_observations.append(service._test_matchmaking_snapshot(ticket))
	service.state_changed.connect(service_callback)
	fixture["ticket_callback"] = ticket_callback
	fixture["service_callback"] = service_callback
	_fixtures.append(fixture)
	return fixture


func _collect_signal(async_signal: Variant, fixture: Dictionary) -> Dictionary:
	assert_eq(typeof(async_signal), TYPE_SIGNAL, "Operation returns a completion Signal")
	var collector := {
		"signal": async_signal,
		"results": [],
		"observations": [],
	}
	if typeof(async_signal) != TYPE_SIGNAL:
		return collector

	var results: Array = collector.results
	var observations: Array = collector.observations
	var service = fixture.service
	var ticket = fixture.ticket
	async_signal.connect(func(result):
		results.append(result)
		observations.append(service._test_matchmaking_snapshot(ticket))
	)
	return collector


func _status(
		ticket: Object,
		status: int,
		ticket_id: String = "offline-ticket",
		match_id: String = "",
		arrangement: String = "") -> Dictionary:
	return {
		"type": "status",
		"ticket": ticket,
		"snapshot": _snapshot(status, ticket_id, match_id, arrangement),
	}


func _completed(
		ticket: Object,
		status: int,
		hresult: int,
		ticket_id: String = "offline-ticket",
		match_id: String = "",
		arrangement: String = "") -> Dictionary:
	return {
		"type": "completed",
		"ticket": ticket,
		"snapshot": _snapshot(status, ticket_id, match_id, arrangement),
		"hresult": hresult,
	}


func _batch(fixture: Dictionary, changes: Array, finish_hresult: int = S_OK) -> void:
	fixture.service._test_matchmaking_batch(changes, finish_hresult)


func _events_of_kind(events: Array, kind: int) -> Array:
	var matching: Array = []
	for change in events:
		if int(change.kind) == kind:
			matching.append(change)
	return matching


func _terminal_events(events: Array) -> Array:
	var terminal: Array = []
	for change in events:
		if int(change.kind) in [EVENT_COMPLETED, EVENT_CANCELLED, EVENT_FAILED]:
			terminal.append(change)
	return terminal


func _assert_hresult(result: Variant, expected: int, label: String) -> void:
	assert_not_null(result, "%s returns a result" % label)
	if result == null:
		return
	assert_eq(
		int(result.hresult) & HRESULT_MASK,
		expected & HRESULT_MASK,
		"%s preserves HRESULT low bits" % label)


func _assert_group_hresult(result: Variant, label: String) -> void:
	assert_not_null(result, "%s returns a result" % label)
	if result == null:
		return
	assert_eq(
		int(result.hresult),
		E_PF_MATCHMAKING_NUMBER_OF_PLAYERS_IN_TICKET_TOO_LARGE_SIGNED,
		"%s sign-extends 0x89235652" % label)
	assert_eq(
		int(result.hresult) & HRESULT_MASK,
		E_PF_MATCHMAKING_NUMBER_OF_PLAYERS_IN_TICKET_TOO_LARGE,
		"%s preserves 0x89235652 low bits" % label)


func _assert_terminal_delivery(fixture: Dictionary, expected_kind: int) -> void:
	var ticket_terminal := _terminal_events(fixture.ticket_events)
	var service_terminal := _terminal_events(fixture.service_events)
	assert_eq(ticket_terminal.size(), 1, "Ticket emits one terminal notification")
	assert_eq(service_terminal.size(), 1, "Service emits one terminal notification")
	if ticket_terminal.size() == 1:
		assert_eq(int(ticket_terminal[0].kind), expected_kind, "Ticket terminal event kind")
	if service_terminal.size() == 1:
		assert_eq(int(service_terminal[0].kind), expected_kind, "Service terminal event kind")

	for index in range(fixture.ticket_events.size()):
		if int(fixture.ticket_events[index].kind) not in [EVENT_COMPLETED, EVENT_CANCELLED, EVENT_FAILED]:
			continue
		var observed: Dictionary = fixture.ticket_event_observations[index]
		assert_true(observed.native_handle_live, "Ticket callback observes a live native handle")
		assert_eq(int(observed.ticket_destroys), 0, "Ticket callback runs before destruction")
	for index in range(fixture.service_events.size()):
		if int(fixture.service_events[index].kind) not in [EVENT_COMPLETED, EVENT_CANCELLED, EVENT_FAILED]:
			continue
		var observed: Dictionary = fixture.service_event_observations[index]
		assert_true(observed.native_handle_live, "Service callback observes a live native handle")
		assert_eq(int(observed.ticket_destroys), 0, "Service callback runs before destruction")

	var final_snapshot: Dictionary = fixture.service._test_matchmaking_snapshot(fixture.ticket)
	assert_false(final_snapshot.tracked, "Terminal ticket is untracked after the batch")
	assert_eq(int(final_snapshot.pending), 0, "Terminal ticket has no pending operations")
	assert_false(final_snapshot.native_handle_live, "Terminal ticket native handle is invalidated")
	assert_true(final_snapshot.completion_received, "Terminal ticket records native completion")
	assert_eq(int(final_snapshot.ticket_destroys), 1, "Terminal ticket is destroyed exactly once")


func _assert_operation_before_destroy(collector: Dictionary, label: String) -> void:
	assert_eq(collector.results.size(), 1, "%s completes exactly once" % label)
	assert_eq(collector.observations.size(), 1, "%s records one completion observation" % label)
	if collector.observations.size() == 1:
		assert_true(collector.observations[0].native_handle_live, "%s completes before destruction" % label)
		assert_eq(int(collector.observations[0].ticket_destroys), 0, "%s sees zero destructions" % label)


func test_cancel_then_match_same_batch() -> void:
	if not _hooks_available:
		return
	var fixture := _begin("tracked", _snapshot(STATUS_WAITING_FOR_MATCH))
	var cancel := _collect_signal(fixture.ticket.cancel_async(), fixture)
	_batch(fixture, [
		_status(fixture.ticket, STATUS_MATCHED),
		_completed(
			fixture.ticket,
			STATUS_MATCHED,
			S_OK,
			"offline-ticket",
			"match-same-batch",
			"arrangement-same-batch"),
	])

	_assert_operation_before_destroy(cancel, "Matched cancellation")
	var result = cancel.results[0]
	assert_false(result.ok)
	assert_eq(result.code, "match_ticket_cancel_lost_race")
	assert_eq(result.data, fixture.ticket)
	_assert_hresult(result, E_ABORT, "Matched cancellation")
	assert_eq(fixture.ticket.status, STATUS_MATCHED)
	assert_eq(fixture.ticket.match_id, "match-same-batch")
	assert_eq(fixture.ticket.arranged_lobby_connection_string, "arrangement-same-batch")
	_assert_terminal_delivery(fixture, EVENT_COMPLETED)
	assert_true(_terminal_events(fixture.ticket_events)[0].result.ok)


func test_cancel_then_match_separate_batches() -> void:
	if not _hooks_available:
		return
	var fixture := _begin("tracked", _snapshot(STATUS_WAITING_FOR_MATCH))
	var cancel := _collect_signal(fixture.ticket.cancel_async(), fixture)
	_batch(fixture, [_status(fixture.ticket, STATUS_MATCHED)])

	assert_eq(cancel.results.size(), 0, "Terminal status alone does not settle cancellation")
	assert_eq(fixture.ticket_events.size(), 0, "Terminal status alone emits no ticket event")
	assert_eq(fixture.service_events.size(), 0, "Terminal status alone emits no service event")
	var gap: Dictionary = fixture.service._test_matchmaking_snapshot(fixture.ticket)
	assert_true(gap.tracked)
	assert_eq(int(gap.pending), 1)
	assert_true(gap.native_handle_live)
	assert_false(gap.completion_received)
	assert_eq(int(gap.ticket_destroys), 0)

	_batch(fixture, [
		_completed(
			fixture.ticket,
			STATUS_MATCHED,
			S_OK,
			"offline-ticket",
			"match-separate",
			"arrangement-separate"),
	])
	_assert_operation_before_destroy(cancel, "Separated matched cancellation")
	assert_eq(cancel.results[0].code, "match_ticket_cancel_lost_race")
	assert_eq(fixture.ticket.status, STATUS_MATCHED)
	assert_eq(fixture.ticket.match_id, "match-separate")
	assert_eq(fixture.ticket.arranged_lobby_connection_string, "arrangement-separate")
	_assert_terminal_delivery(fixture, EVENT_COMPLETED)


func test_shutdown_requested_by_status_callback_preserves_same_batch_completion() -> void:
	if not _hooks_available:
		return
	var fixture := _begin("tracked", _snapshot(STATUS_WAITING_FOR_MATCH))
	var shutdown := {"state": null}
	var shutdown_handler := func(change):
		if int(change.kind) == EVENT_STATUS_CHANGED and shutdown["state"] == null:
			shutdown["state"] = track_signal(fixture.service.shutdown_async())
	fixture.ticket.state_changed.connect(shutdown_handler)
	var cancel := _collect_signal(fixture.ticket.cancel_async(), fixture)

	_batch(fixture, [
		_status(fixture.ticket, STATUS_WAITING_FOR_PLAYERS),
		_completed(
			fixture.ticket,
			STATUS_MATCHED,
			S_OK,
			"offline-ticket",
			"match-shutdown",
			"arrangement-shutdown"),
	])
	fixture.ticket.state_changed.disconnect(shutdown_handler)

	assert_not_null(shutdown["state"], "Status callback requests shutdown")
	_assert_terminal_delivery(fixture, EVENT_COMPLETED)
	assert_eq(fixture.ticket.status, STATUS_MATCHED)
	assert_eq(fixture.ticket.match_id, "match-shutdown")
	assert_eq(fixture.ticket.arranged_lobby_connection_string, "arrangement-shutdown")
	if shutdown["state"] != null:
		var shutdown_result = await await_completion_state(shutdown["state"])
		assert_not_null(shutdown_result)
		if shutdown_result != null:
			assert_true(shutdown_result.ok)
	assert_eq(cancel.results.size(), 1, "Mid-batch shutdown cancel completes exactly once")
	if cancel.results.size() == 1:
		assert_false(cancel.results[0].ok)
		assert_eq(cancel.results[0].code, "cancelled")
		_assert_hresult(cancel.results[0], E_ABORT, "Mid-batch shutdown cancellation")
	assert_eq(int(fixture.service._test_matchmaking_snapshot(fixture.ticket).pending), 0)


func test_cancel_then_cancelled() -> void:
	if not _hooks_available:
		return
	for completion_hresult in [S_OK, NATIVE_CANCELLED_HRESULT]:
		for separate in [false, true]:
			var fixture := _begin("tracked", _snapshot(STATUS_WAITING_FOR_MATCH))
			var cancel := _collect_signal(fixture.ticket.cancel_async(), fixture)
			if separate:
				_batch(fixture, [_status(fixture.ticket, STATUS_CANCELLED)])
				assert_eq(cancel.results.size(), 0, "Cancelled status waits for native completion")
				assert_eq(fixture.ticket_events.size(), 0, "Cancelled status emits no terminal event")
				assert_eq(int(fixture.service._test_matchmaking_snapshot(fixture.ticket).ticket_destroys), 0)
				_batch(fixture, [_completed(fixture.ticket, STATUS_CANCELLED, completion_hresult)])
			else:
				_batch(fixture, [
					_status(fixture.ticket, STATUS_CANCELLED),
					_completed(fixture.ticket, STATUS_CANCELLED, completion_hresult),
				])

			_assert_operation_before_destroy(cancel, "Successful cancellation")
			assert_true(cancel.results[0].ok)
			assert_eq(cancel.results[0].data, null)
			_assert_terminal_delivery(fixture, EVENT_CANCELLED)
			assert_true(_terminal_events(fixture.ticket_events)[0].result.ok)


func test_cancel_then_failed() -> void:
	if not _hooks_available:
		return
	var fixture := _begin("tracked", _snapshot(STATUS_WAITING_FOR_MATCH))
	var cancel := _collect_signal(fixture.ticket.cancel_async(), fixture)
	_batch(fixture, [_status(fixture.ticket, STATUS_FAILED)])
	assert_eq(cancel.results.size(), 0, "Failed status alone does not settle cancellation")
	_batch(fixture, [_completed(fixture.ticket, STATUS_FAILED, E_PF_SERVICE_UNAVAILABLE)])

	_assert_operation_before_destroy(cancel, "Failed cancellation")
	assert_false(cancel.results[0].ok)
	assert_eq(cancel.results[0].code, "match_ticket_completed_failed")
	_assert_hresult(cancel.results[0], E_PF_SERVICE_UNAVAILABLE, "Failed cancellation")
	assert_true((int(cancel.results[0].hresult) & HRESULT_MASK) != E_FAIL)
	_assert_terminal_delivery(fixture, EVENT_FAILED)
	_assert_hresult(_terminal_events(fixture.ticket_events)[0].result, E_PF_SERVICE_UNAVAILABLE, "Failed ticket event")


func test_cancel_in_terminal_status_completion_gap() -> void:
	if not _hooks_available:
		return
	for case in [
		{"status": STATUS_MATCHED, "hresult": S_OK, "kind": EVENT_COMPLETED, "code": "match_ticket_cancel_lost_race"},
		{"status": STATUS_CANCELLED, "hresult": NATIVE_CANCELLED_HRESULT, "kind": EVENT_CANCELLED, "code": "ok"},
		{"status": STATUS_FAILED, "hresult": E_PF_SERVICE_UNAVAILABLE, "kind": EVENT_FAILED, "code": "match_ticket_completed_failed"},
	]:
		var fixture := _begin("tracked", _snapshot(STATUS_WAITING_FOR_MATCH))
		_batch(fixture, [_status(fixture.ticket, case.status)])
		var before_cancel: Dictionary = fixture.service._test_matchmaking_snapshot(fixture.ticket)
		assert_true(before_cancel.tracked)
		assert_eq(int(before_cancel.cancel_starts), 0)
		assert_eq(int(before_cancel.pending), 0)

		var cancel := _collect_signal(fixture.ticket.cancel_async(), fixture)
		var waiting: Dictionary = fixture.service._test_matchmaking_snapshot(fixture.ticket)
		assert_eq(int(waiting.cancel_starts), 0, "No native cancel call occurs in the terminal gap")
		assert_eq(int(waiting.pending), 1, "Gap cancellation waits for TicketCompleted")
		assert_eq(cancel.results.size(), 0)

		_batch(fixture, [_completed(fixture.ticket, case.status, case.hresult)])
		_assert_operation_before_destroy(cancel, "Gap cancellation")
		assert_eq(cancel.results[0].code, case.code)
		if case.status == STATUS_CANCELLED:
			assert_true(cancel.results[0].ok)
			assert_eq(cancel.results[0].data, null)
		elif case.status == STATUS_MATCHED:
			assert_false(cancel.results[0].ok)
			assert_eq(cancel.results[0].data, fixture.ticket)
			_assert_hresult(cancel.results[0], E_ABORT, "Gap matched cancellation")
		else:
			assert_false(cancel.results[0].ok)
			_assert_hresult(cancel.results[0], E_PF_SERVICE_UNAVAILABLE, "Gap failed cancellation")
		_assert_terminal_delivery(fixture, case.kind)


func test_cancel_during_and_after_completion_is_deferred_invalid_ticket() -> void:
	if not _hooks_available:
		return
	var fixture := _begin("tracked", _snapshot(STATUS_WAITING_FOR_MATCH))
	var in_handler := {"collector": {}}
	var terminal_handler := func(change):
		if int(change.kind) == EVENT_COMPLETED and in_handler["collector"].is_empty():
			in_handler["collector"] = _collect_signal(fixture.ticket.cancel_async(), fixture)
	fixture.ticket.state_changed.connect(terminal_handler)

	_batch(fixture, [
		_status(fixture.ticket, STATUS_MATCHED),
		_completed(
			fixture.ticket,
			STATUS_MATCHED,
			S_OK,
			"offline-ticket",
			"match-invalid-cancel",
			"arrangement-invalid-cancel"),
	])
	fixture.ticket.state_changed.disconnect(terminal_handler)

	assert_eq(_events_of_kind(fixture.ticket_events, EVENT_COMPLETED).size(), 1)
	var terminal_cancel: Dictionary = in_handler["collector"]
	assert_false(terminal_cancel.is_empty(), "Terminal handler requests cancellation")
	var post_batch_cancel := _collect_signal(fixture.ticket.cancel_async(), fixture)
	assert_eq(terminal_cancel.results.size(), 0, "In-handler invalid cancel is deferred")
	assert_eq(post_batch_cancel.results.size(), 0, "Post-batch invalid cancel is deferred")

	await get_tree().process_frame
	for case in [
		{"name": "in-handler", "collector": terminal_cancel},
		{"name": "post-batch", "collector": post_batch_cancel},
	]:
		assert_eq(case.collector.results.size(), 1, "%s cancel completes exactly once" % case.name)
		if case.collector.results.size() == 1:
			assert_false(case.collector.results[0].ok)
			assert_eq(case.collector.results[0].code, "invalid_match_ticket")
	var snapshot: Dictionary = fixture.service._test_matchmaking_snapshot(fixture.ticket)
	assert_eq(int(snapshot.pending), 0)
	assert_eq(int(snapshot.cancel_starts), 0)


func test_duplicate_cancels_share_one_pending_signal() -> void:
	if not _hooks_available:
		return
	var fixture := _begin("tracked", _snapshot(STATUS_WAITING_FOR_MATCH))
	var first_signal = fixture.ticket.cancel_async()
	var first := _collect_signal(first_signal, fixture)
	var second_signal = fixture.ticket.cancel_async()
	var second := _collect_signal(second_signal, fixture)

	assert_eq(first_signal, second_signal, "Concurrent cancels share the same completion Signal")
	var pending: Dictionary = fixture.service._test_matchmaking_snapshot(fixture.ticket)
	assert_eq(int(pending.cancel_starts), 1, "Concurrent cancels issue one native cancel")
	assert_eq(int(pending.pending), 1, "Concurrent cancels register one operation")

	_batch(fixture, [
		_status(fixture.ticket, STATUS_CANCELLED),
		_completed(fixture.ticket, STATUS_CANCELLED, NATIVE_CANCELLED_HRESULT),
	])
	_assert_operation_before_destroy(first, "First cancel waiter")
	_assert_operation_before_destroy(second, "Second cancel waiter")
	assert_true(first.results[0].ok)
	assert_true(second.results[0].ok)
	_assert_terminal_delivery(fixture, EVENT_CANCELLED)


func test_duplicate_terminal_records_settle_once() -> void:
	if not _hooks_available:
		return
	for operation in ["tracked", "create", "join"]:
		var initial := _snapshot(
			STATUS_WAITING_FOR_MATCH if operation == "tracked" else
			(STATUS_CREATING if operation == "create" else STATUS_JOINING),
			"" if operation == "create" else "offline-ticket")
		var fixture := _begin(operation, initial)
		var cancel: Dictionary = {}
		if operation == "tracked":
			cancel = _collect_signal(fixture.ticket.cancel_async(), fixture)

		_batch(fixture, [
			_completed(
				fixture.ticket,
				STATUS_FAILED,
				E_PF_MATCHMAKING_NUMBER_OF_PLAYERS_IN_TICKET_TOO_LARGE,
				"first-ticket",
				"first-match",
				"first-arrangement"),
			_completed(
				fixture.ticket,
				STATUS_MATCHED,
				E_PF_SERVICE_UNAVAILABLE,
				"mutated-ticket",
				"mutated-match",
				"mutated-arrangement"),
			_status(
				fixture.ticket,
				STATUS_MATCHED,
				"mutated-status-ticket",
				"mutated-status-match",
				"mutated-status-arrangement"),
		])
		_batch(fixture, [
			_completed(
				fixture.ticket,
				STATUS_MATCHED,
				S_OK,
				"late-ticket",
				"late-match",
				"late-arrangement"),
		])

		if operation == "tracked":
			assert_eq(cancel.results.size(), 1)
			_assert_group_hresult(cancel.results[0], "Duplicate tracked cancellation")
		else:
			assert_eq(fixture.factory_results.size(), 1, "%s factory settles once" % operation)
			_assert_group_hresult(fixture.factory_results[0], "Duplicate %s factory" % operation)
		assert_eq(fixture.ticket.status, STATUS_FAILED, "First completion keeps the final status")
		assert_eq(fixture.ticket.ticket_id, "first-ticket", "Duplicate records cannot refresh ticket id")
		assert_eq(fixture.ticket.match_id, "first-match", "Duplicate records cannot refresh match id")
		assert_eq(
			fixture.ticket.arranged_lobby_connection_string,
			"first-arrangement",
			"Duplicate records cannot refresh arrangement data")
		_assert_terminal_delivery(fixture, EVENT_FAILED)
		_assert_group_hresult(_terminal_events(fixture.ticket_events)[0].result, "Duplicate terminal event")


func test_cancel_start_failure_is_deferred() -> void:
	if not _hooks_available:
		return
	var fixture := _begin("tracked", _snapshot(STATUS_WAITING_FOR_MATCH))
	fixture.service._test_set_match_ticket_cancel_result(E_PF_NOT_AUTHENTICATED)
	var cancel := _collect_signal(fixture.ticket.cancel_async(), fixture)

	assert_eq(cancel.results.size(), 0, "Synchronous native start failure is delivered deferred")
	var immediate: Dictionary = fixture.service._test_matchmaking_snapshot(fixture.ticket)
	assert_eq(int(immediate.cancel_starts), 1)
	assert_eq(int(immediate.pending), 0, "Start failure registers no pending cancellation")
	await get_tree().process_frame
	assert_eq(cancel.results.size(), 1, "Deferred start failure settles on a later frame")
	assert_false(cancel.results[0].ok)
	assert_eq(cancel.results[0].code, "match_ticket_cancel_start_failed")
	_assert_hresult(cancel.results[0], E_PF_NOT_AUTHENTICATED, "Cancel start failure")
	assert_eq(fixture.ticket_events.size(), 0, "Start failure emits no ticket cancellation event")


func test_shutdown_after_failed_status_settles_pending_cancel() -> void:
	if not _hooks_available:
		return
	var fixture := _begin("tracked", _snapshot(STATUS_WAITING_FOR_MATCH))
	var cancel := _collect_signal(fixture.ticket.cancel_async(), fixture)
	_batch(fixture, [_status(fixture.ticket, STATUS_FAILED)])
	assert_eq(cancel.results.size(), 0, "Failed status remains pending until completion or shutdown")

	var first_shutdown = fixture.service.shutdown_async()
	var first_state := track_signal(first_shutdown)
	var second_shutdown = fixture.service.shutdown_async()
	var second_state := track_signal(second_shutdown)
	var first_result = await await_completion_state(first_state)
	var second_result = second_state.result
	if not second_state.completed:
		second_result = await await_completion_state(second_state)
	assert_true(first_result.ok)
	assert_true(second_result.ok)
	assert_eq(cancel.results.size(), 1, "Shutdown settles the pending cancel exactly once")
	assert_false(cancel.results[0].ok)
	assert_eq(cancel.results[0].code, "cancelled")
	_assert_hresult(cancel.results[0], E_ABORT, "Shutdown cancellation")
	var final_snapshot: Dictionary = fixture.service._test_matchmaking_snapshot(fixture.ticket)
	assert_eq(int(final_snapshot.pending), 0)
	assert_false(final_snapshot.tracked)
	assert_eq(int(final_snapshot.ticket_destroys), 1)


func test_finish_failure_after_failed_status_settles_pending_cancel() -> void:
	if not _hooks_available:
		return
	for completion_before_finish_failure in [false, true]:
		var fixture := _begin("tracked", _snapshot(STATUS_WAITING_FOR_MATCH))
		var cancel := _collect_signal(fixture.ticket.cancel_async(), fixture)
		var changes: Array = [_status(fixture.ticket, STATUS_FAILED)]
		if completion_before_finish_failure:
			changes.append(_completed(
				fixture.ticket,
				STATUS_FAILED,
				E_PF_MATCHMAKING_NUMBER_OF_PLAYERS_IN_TICKET_TOO_LARGE))
		_batch(fixture, changes, E_UNEXPECTED)
		assert_engine_error("FinishStateChanges failed")

		assert_eq(cancel.results.size(), 1, "Finish-failure variant settles cancellation once")
		assert_false(cancel.results[0].ok)
		if completion_before_finish_failure:
			assert_eq(cancel.results[0].code, "match_ticket_completed_failed")
			_assert_group_hresult(cancel.results[0], "Completion result before failed Finish")
		else:
			assert_eq(cancel.results[0].code, "matchmaking_state_finish_failed")
			_assert_hresult(cancel.results[0], E_UNEXPECTED, "Failed Finish cancellation")
		var final_snapshot: Dictionary = fixture.service._test_matchmaking_snapshot(fixture.ticket)
		assert_eq(int(final_snapshot.pending), 0)
		assert_false(final_snapshot.tracked)
		assert_eq(int(final_snapshot.ticket_destroys), 1)


func test_failed_status_preserves_group_hresult_same_batch() -> void:
	if not _hooks_available:
		return
	var fixture := _begin("tracked", _snapshot(STATUS_WAITING_FOR_MATCH))
	_batch(fixture, [
		_status(fixture.ticket, STATUS_FAILED),
		_completed(fixture.ticket, STATUS_FAILED, E_PF_MATCHMAKING_NUMBER_OF_PLAYERS_IN_TICKET_TOO_LARGE),
	])
	_assert_terminal_delivery(fixture, EVENT_FAILED)
	_assert_group_hresult(_terminal_events(fixture.ticket_events)[0].result, "Same-batch failed ticket")
	_assert_group_hresult(_terminal_events(fixture.service_events)[0].result, "Same-batch service failure")


func test_failed_status_preserves_group_hresult_separate_batches() -> void:
	if not _hooks_available:
		return
	var fixture := _begin("tracked", _snapshot(STATUS_WAITING_FOR_MATCH))
	_batch(fixture, [_status(fixture.ticket, STATUS_FAILED)])
	assert_eq(fixture.ticket_events.size(), 0, "Failed status emits no premature event")
	var gap: Dictionary = fixture.service._test_matchmaking_snapshot(fixture.ticket)
	assert_true(gap.tracked)
	assert_true(gap.native_handle_live)
	assert_eq(int(gap.ticket_destroys), 0)

	_batch(fixture, [
		_completed(fixture.ticket, STATUS_FAILED, E_PF_MATCHMAKING_NUMBER_OF_PLAYERS_IN_TICKET_TOO_LARGE),
	])
	_assert_terminal_delivery(fixture, EVENT_FAILED)
	_assert_group_hresult(_terminal_events(fixture.ticket_events)[0].result, "Separate-batch failed ticket")
	_assert_group_hresult(_terminal_events(fixture.service_events)[0].result, "Separate-batch service failure")


func test_unrelated_failure_hresults_are_forwarded() -> void:
	if not _hooks_available:
		return
	for native_hresult in [
		E_PF_NOT_AUTHENTICATED,
		E_PF_MATCHMAKING_QUEUE_NOT_FOUND,
		E_PF_SERVICE_UNAVAILABLE,
	]:
		for surface in ["tracked", "create", "join", "pending_cancel"]:
			var operation: String = "tracked" if surface in ["tracked", "pending_cancel"] else str(surface)
			var initial_status: int = (
				STATUS_WAITING_FOR_MATCH if operation == "tracked" else
				(STATUS_CREATING if operation == "create" else STATUS_JOINING))
			var initial_id: String = "" if operation == "create" else "offline-ticket"
			var fixture: Dictionary = _begin(operation, _snapshot(initial_status, initial_id))
			var cancel: Dictionary = {}
			if surface == "pending_cancel":
				cancel = _collect_signal(fixture.ticket.cancel_async(), fixture)

			_batch(fixture, [_status(fixture.ticket, STATUS_FAILED, initial_id)])
			if operation in ["create", "join"]:
				assert_eq(fixture.factory_results.size(), 0, "%s waits for native failure" % surface)
			if surface == "pending_cancel":
				assert_eq(cancel.results.size(), 0, "Pending cancel waits for native failure")
			_batch(fixture, [_completed(fixture.ticket, STATUS_FAILED, native_hresult, initial_id)])

			var operation_result = null
			if surface == "create" or surface == "join":
				assert_eq(fixture.factory_results.size(), 1)
				operation_result = fixture.factory_results[0]
			elif surface == "pending_cancel":
				assert_eq(cancel.results.size(), 1)
				operation_result = cancel.results[0]
			if operation_result != null:
				_assert_hresult(operation_result, native_hresult, "%s native failure" % surface)
				assert_true((int(operation_result.hresult) & HRESULT_MASK) != E_FAIL)

			_assert_terminal_delivery(fixture, EVENT_FAILED)
			var event_result = _terminal_events(fixture.ticket_events)[0].result
			_assert_hresult(event_result, native_hresult, "%s terminal event" % surface)
			assert_true((int(event_result.hresult) & HRESULT_MASK) != E_FAIL)


func test_create_without_id_preserves_completion_hresult() -> void:
	if not _hooks_available:
		return
	var fixture := _begin("create", _snapshot(STATUS_CREATING, ""))
	_batch(fixture, [_status(fixture.ticket, STATUS_FAILED, "")])
	assert_eq(fixture.factory_results.size(), 0, "Create remains pending after failed status")
	_batch(fixture, [
		_completed(fixture.ticket, STATUS_FAILED, E_PF_MATCHMAKING_NUMBER_OF_PLAYERS_IN_TICKET_TOO_LARGE, ""),
	])

	assert_eq(fixture.factory_results.size(), 1)
	assert_false(fixture.factory_results[0].ok)
	assert_eq(fixture.factory_results[0].code, "match_ticket_create_failed")
	_assert_group_hresult(fixture.factory_results[0], "Create without id")
	assert_eq(_events_of_kind(fixture.ticket_events, EVENT_CREATED).size(), 0)
	_assert_terminal_delivery(fixture, EVENT_FAILED)
	_assert_group_hresult(_terminal_events(fixture.ticket_events)[0].result, "Create terminal event")


func test_join_waits_for_native_failure() -> void:
	if not _hooks_available:
		return
	var fixture := _begin("join", _snapshot(STATUS_JOINING))
	_batch(fixture, [_status(fixture.ticket, STATUS_JOINING)])
	assert_eq(fixture.factory_results.size(), 0, "Joining status remains pending")
	_batch(fixture, [_status(fixture.ticket, STATUS_FAILED)])
	assert_eq(fixture.factory_results.size(), 0, "Failed status remains pending for native HRESULT")
	_batch(fixture, [
		_completed(fixture.ticket, STATUS_FAILED, E_PF_MATCHMAKING_NUMBER_OF_PLAYERS_IN_TICKET_TOO_LARGE),
	])

	assert_eq(fixture.factory_results.size(), 1)
	assert_false(fixture.factory_results[0].ok)
	assert_eq(fixture.factory_results[0].code, "match_ticket_join_failed")
	_assert_group_hresult(fixture.factory_results[0], "Join failure")
	assert_eq(_events_of_kind(fixture.ticket_events, EVENT_CREATED).size(), 0)
	_assert_terminal_delivery(fixture, EVENT_FAILED)
	_assert_group_hresult(_terminal_events(fixture.ticket_events)[0].result, "Join terminal event")


func test_matched_completion_settles_pending_factories() -> void:
	if not _hooks_available:
		return
	for operation in ["create", "join"]:
		var fixture := _begin(
			operation,
			_snapshot(
				STATUS_CREATING if operation == "create" else STATUS_JOINING,
				"" if operation == "create" else "offline-ticket"))
		var match_id := "match-%s" % operation
		var arrangement := "arrangement-%s" % operation
		_batch(fixture, [
			_status(
				fixture.ticket,
				STATUS_MATCHED,
				"offline-ticket",
				match_id,
				arrangement),
		])
		assert_eq(fixture.factory_results.size(), 0, "%s waits for matched completion" % operation)

		_batch(fixture, [
			_completed(
				fixture.ticket,
				STATUS_MATCHED,
				S_OK,
				"offline-ticket",
				match_id,
				arrangement),
		])
		assert_eq(fixture.factory_results.size(), 1, "%s completes once" % operation)
		if fixture.factory_results.size() == 1:
			assert_true(fixture.factory_results[0].ok)
			assert_eq(fixture.factory_results[0].data, fixture.ticket)
		assert_eq(_events_of_kind(fixture.ticket_events, EVENT_CREATED).size(), 1)
		assert_eq(_events_of_kind(fixture.ticket_events, EVENT_COMPLETED).size(), 1)
		assert_eq(fixture.ticket_events.size(), 2)
		if fixture.ticket_events.size() == 2:
			assert_eq(int(fixture.ticket_events[0].kind), EVENT_CREATED)
			assert_eq(int(fixture.ticket_events[1].kind), EVENT_COMPLETED)
		assert_eq(fixture.ticket.status, STATUS_MATCHED)
		assert_eq(fixture.ticket.match_id, match_id)
		assert_eq(fixture.ticket.arranged_lobby_connection_string, arrangement)
		_assert_terminal_delivery(fixture, EVENT_COMPLETED)

	var empty_id := _begin("create", _snapshot(STATUS_CREATING, ""))
	_batch(empty_id, [
		_status(
			empty_id.ticket,
			STATUS_MATCHED,
			"",
			"match-empty-id",
			"arrangement-empty-id"),
	])
	assert_eq(empty_id.factory_results.size(), 0, "Empty-id create waits for matched completion")
	_batch(empty_id, [
		_completed(
			empty_id.ticket,
			STATUS_MATCHED,
			S_OK,
			"",
			"match-empty-id",
			"arrangement-empty-id"),
	])
	assert_eq(empty_id.factory_results.size(), 1)
	if empty_id.factory_results.size() == 1:
		assert_false(empty_id.factory_results[0].ok)
		assert_eq(empty_id.factory_results[0].code, "match_ticket_create_failed")
		_assert_hresult(empty_id.factory_results[0], E_FAIL, "Empty-id matched create")
	assert_eq(_events_of_kind(empty_id.ticket_events, EVENT_CREATED).size(), 0)
	_assert_terminal_delivery(empty_id, EVENT_COMPLETED)


func test_cancelled_completion_factory_results() -> void:
	if not _hooks_available:
		return
	for operation in ["create", "join"]:
		for completion_hresult in [S_OK, S_FALSE, NATIVE_CANCELLED_HRESULT]:
			var fixture := _begin(
				operation,
				_snapshot(
					STATUS_CREATING if operation == "create" else STATUS_JOINING,
					"" if operation == "create" else "offline-ticket"))
			_batch(fixture, [_status(
				fixture.ticket,
				STATUS_CANCELLED,
				"" if operation == "create" else "offline-ticket")])
			assert_eq(fixture.factory_results.size(), 0, "Cancelled status waits for native completion")
			_batch(fixture, [_completed(
				fixture.ticket,
				STATUS_CANCELLED,
				completion_hresult,
				"" if operation == "create" else "offline-ticket")])

			assert_eq(fixture.factory_results.size(), 1)
			assert_false(fixture.factory_results[0].ok)
			assert_eq(
				fixture.factory_results[0].code,
				"match_ticket_create_cancelled" if operation == "create" else "match_ticket_join_cancelled")
			_assert_hresult(fixture.factory_results[0], E_ABORT, "%s cancellation" % operation)
			assert_true((int(fixture.factory_results[0].hresult) & HRESULT_MASK) != E_FAIL)
			if operation == "create":
				assert_eq(typeof(fixture.factory_results[0].data), TYPE_DICTIONARY)
			else:
				assert_eq(fixture.factory_results[0].data, fixture.ticket)
			assert_eq(_events_of_kind(fixture.ticket_events, EVENT_CREATED).size(), 0)
			_assert_terminal_delivery(fixture, EVENT_CANCELLED)
			assert_true(_terminal_events(fixture.ticket_events)[0].result.ok)


func test_nonterminal_readiness_unchanged() -> void:
	if not _hooks_available:
		return
	for operation in ["create", "join"]:
		var fixture := _begin(
			operation,
			_snapshot(
				STATUS_CREATING if operation == "create" else STATUS_JOINING,
				"" if operation == "create" else "offline-ticket"))
		var ready_status: int = STATUS_WAITING_FOR_PLAYERS if operation == "create" else STATUS_WAITING_FOR_MATCH
		_batch(fixture, [_status(fixture.ticket, ready_status)])

		assert_eq(fixture.factory_results.size(), 1, "%s readiness completes once" % operation)
		assert_true(fixture.factory_results[0].ok)
		assert_eq(fixture.factory_results[0].data, fixture.ticket)
		var created := _events_of_kind(fixture.ticket_events, EVENT_CREATED)
		assert_eq(created.size(), 1, "%s readiness emits CREATED once" % operation)
		if created.size() == 1:
			assert_eq(int(created[0].status), ready_status, "CREATED carries the current status level")
			assert_eq(created[0].ticket, fixture.ticket)

		_batch(fixture, [_status(fixture.ticket, ready_status)])
		assert_eq(fixture.factory_results.size(), 1, "%s readiness cannot settle twice" % operation)
		assert_eq(_events_of_kind(fixture.ticket_events, EVENT_CREATED).size(), 1)
		var current: Dictionary = fixture.service._test_matchmaking_snapshot(fixture.ticket)
		assert_true(current.tracked)
		assert_eq(int(current.pending), 0)
		assert_true(current.native_handle_live)
		assert_false(current.completion_received)
		assert_eq(int(current.ticket_destroys), 0)
