extends "res://addons/godot_gdk_tests/playfab_test_base.gd"

const MATCH_QUEUE_ENV := "PLAYFAB_MULTIPLAYER_MATCH_QUEUE"
const MATCH_TEST_TIMEOUT_MSEC := 60_000
const DEFAULT_MATCH_QUEUE := "godot_gdk_ext_live_smoke_queue"


func after_each() -> void:
	reset_playfab_runtime()


func test_create_match_ticket_returns_ticket_id_before_cancel() -> void:
	var session: Dictionary = await _begin_match_session("create_match_ticket returns ticket_id")
	var playfab: Object = session.get("playfab", null)
	var multiplayer: Object = session.get("multiplayer", null)
	var playfab_user: Object = session.get("playfab_user", null)
	if playfab == null or multiplayer == null or playfab_user == null:
		return

	var config: Object = instantiate_class("PlayFabMatchmakingTicketConfig")
	assert_not_null(config, "PlayFabMatchmakingTicketConfig registered")
	if config == null:
		return
	config.queue_name = _match_queue_name()
	config.timeout_seconds = 60

	var member: Object = instantiate_class("PlayFabMatchmakingMember")
	assert_not_null(member, "PlayFabMatchmakingMember registered")
	if member == null:
		return
	member.user = playfab_user
	member.attributes = { "scenario": "create_and_cancel", "run_id": with_unique_id("match-ticket") }
	config.members = [member]

	var create_result = await await_completion(
		multiplayer.create_match_ticket_async(playfab_user, config),
		MATCH_TEST_TIMEOUT_MSEC,
	)
	assert_not_null(create_result, "create_match_ticket_async completes")
	if create_result == null:
		return
	if not create_result.ok:
		pending("create_match_ticket_async skipped: %s" % create_result.message)
		return

	var ticket: Object = create_result.data
	assert_object_is(ticket, "PlayFabMatchTicket", "create_match_ticket_async returns PlayFabMatchTicket")
	if ticket == null:
		return

	var ticket_id := String(ticket.get_ticket_id())
	assert_false(ticket_id.is_empty(), "create_match_ticket_async resolves only after ticket_id is assigned")
	if ticket_id.is_empty():
		return

	var cancel_result = await await_completion(ticket.cancel_async(), MATCH_TEST_TIMEOUT_MSEC)
	assert_not_null(cancel_result, "cancel_async completes")
	if cancel_result == null:
		return
	assert_true(cancel_result.ok, "cancel_async succeeds")
	assert_eq(String(ticket.get_ticket_id()), ticket_id, "cancel preserves ticket_id")
	assert_true(ticket.is_cancelled(), "ticket reaches cancelled status")

	playfab.shutdown()


func _begin_match_session(label: String) -> Dictionary:
	var outcome := {
		"playfab": null,
		"multiplayer": null,
		"playfab_user": null,
	}
	if not requires_live_write():
		return outcome
	if pending_unless_playfab_available():
		return outcome

	var queue_name := _match_queue_name()
	if queue_name.is_empty():
		pending("Set %s to exercise %s." % [MATCH_QUEUE_ENV, label])
		return outcome

	reset_playfab_runtime()
	var playfab: Object = get_playfab()
	if playfab == null:
		pending("PlayFab singleton is not available in this host")
		return outcome

	var initialize_result = playfab.initialize()
	if initialize_result == null:
		pending("%s skipped: PlayFab.initialize() returned null." % label)
		return outcome
	if not initialize_result.ok:
		pending("%s skipped: %s" % [label, initialize_result.message])
		return outcome

	var sign_in := await sign_in_with_configured_custom_id(playfab, label, DEFAULT_ASYNC_TIMEOUT_MSEC)
	var playfab_user: Object = sign_in.get("playfab_user", null)
	if playfab_user == null:
		playfab.shutdown()
		return outcome

	var multiplayer: Object = playfab.multiplayer
	assert_not_null(multiplayer, "PlayFab.multiplayer exists")
	if multiplayer == null:
		playfab.shutdown()
		return outcome

	var multiplayer_result = await await_completion(multiplayer.initialize_async(), DEFAULT_ASYNC_TIMEOUT_MSEC)
	if multiplayer_result == null:
		pending("%s skipped: PlayFab.multiplayer.initialize_async() timed out." % label)
		playfab.shutdown()
		return outcome
	if not multiplayer_result.ok:
		pending("%s skipped: %s" % [label, multiplayer_result.message])
		playfab.shutdown()
		return outcome

	outcome["playfab"] = playfab
	outcome["multiplayer"] = multiplayer
	outcome["playfab_user"] = playfab_user
	return outcome


func _match_queue_name() -> String:
	return OS.get_environment(MATCH_QUEUE_ENV).strip_edges() if not OS.get_environment(MATCH_QUEUE_ENV).strip_edges().is_empty() else DEFAULT_MATCH_QUEUE

