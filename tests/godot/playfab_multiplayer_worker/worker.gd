extends SceneTree

const PLAYFAB_EXTENSION_PATH := "res://addons/godot_playfab/godot_playfab.gdextension"
const PLAYFAB_TITLE_ID_SETTING := "playfab/titleid"
const PLAYFAB_ENDPOINT_SETTING := "playfab/endpoint"
const PLAYFAB_TITLE_ID_ENV := "PLAYFAB_TITLE_ID"
const PLAYFAB_ENDPOINT_ENV := "PLAYFAB_ENDPOINT"
const POLL_MSEC := 25
const DEFAULT_TIMEOUT_MSEC := 60000

var _worker_id := ""
var _run_dir := ""
var _exit_requested := false
var _exit_code := 0
var _playfab_extension: Resource = null
var _playfab: Object = null
var _playfab_user: Object = null
var _multiplayer: Object = null
var _primary_lobby: Object = null
var _primary_ticket: Object = null
var _lobbies: Array = []
var _tickets: Array = []


func _initialize() -> void:
	_parse_args()
	if _worker_id.is_empty() or _run_dir.is_empty():
		printerr("worker requires --worker-id and --run-dir")
		quit(2)
		return

	DirAccess.make_dir_recursive_absolute(_run_dir)
	_apply_env_configuration()
	_write_json(_ready_path(), {
		"ok": true,
		"worker_id": _worker_id,
		"pid": OS.get_process_id(),
	})
	call_deferred("_run")


func _run() -> void:
	while not _exit_requested:
		_pump()
		var command := _read_command()
		if not command.is_empty():
			var response := await _execute_command(command)
			_write_json(_response_path(int(command.get("seq", 0))), response)
		await process_frame
		OS.delay_msec(POLL_MSEC)
	quit(_exit_code)


func _parse_args() -> void:
	var args := OS.get_cmdline_user_args()
	var index := 0
	while index < args.size():
		match str(args[index]):
			"--worker-id":
				if index + 1 < args.size():
					_worker_id = str(args[index + 1])
				index += 2
			"--run-dir":
				if index + 1 < args.size():
					_run_dir = str(args[index + 1])
				index += 2
			_:
				index += 1


func _apply_env_configuration() -> void:
	var title_id := OS.get_environment(PLAYFAB_TITLE_ID_ENV).strip_edges()
	if not title_id.is_empty():
		ProjectSettings.set_setting(PLAYFAB_TITLE_ID_SETTING, title_id)
	var endpoint := OS.get_environment(PLAYFAB_ENDPOINT_ENV).strip_edges()
	if not endpoint.is_empty():
		ProjectSettings.set_setting(PLAYFAB_ENDPOINT_SETTING, endpoint)


func _ensure_playfab() -> Object:
	_apply_env_configuration()
	if Engine.has_singleton("PlayFab"):
		_playfab = Engine.get_singleton("PlayFab")
		return _playfab

	if _playfab_extension == null and FileAccess.file_exists(PLAYFAB_EXTENSION_PATH):
		_playfab_extension = load(PLAYFAB_EXTENSION_PATH)

	if Engine.has_singleton("PlayFab"):
		_playfab = Engine.get_singleton("PlayFab")
	return _playfab


func _pump() -> void:
	if _playfab != null:
		_playfab.dispatch()


func _await_completion(async_signal: Variant, timeout_msec: int = DEFAULT_TIMEOUT_MSEC) -> Variant:
	if typeof(async_signal) != TYPE_SIGNAL:
		return null

	var state := {
		"completed": false,
		"result": null,
	}
	async_signal.connect(
		func(result):
			state["completed"] = true
			state["result"] = result,
		CONNECT_ONE_SHOT)

	var started_msec := Time.get_ticks_msec()
	while not bool(state["completed"]):
		_pump()
		if Time.get_ticks_msec() - started_msec >= timeout_msec:
			return null
		await process_frame
		OS.delay_msec(POLL_MSEC)
	_pump()
	return state["result"]


func _execute_command(command: Dictionary) -> Dictionary:
	var op := str(command.get("op", ""))
	var response := {
		"ok": true,
		"worker_id": _worker_id,
		"seq": int(command.get("seq", 0)),
		"op": op,
		"data": {},
	}

	match op:
		"sign_in":
			response["data"] = await _op_sign_in(command)
			response["ok"] = bool(response["data"].get("ok", false))
		"create_lobby":
			response["data"] = await _op_create_lobby(command)
			response["ok"] = bool(response["data"].get("ok", false))
		"search_lobbies":
			response["data"] = await _op_search_lobbies(command)
			response["ok"] = bool(response["data"].get("ok", false))
		"join_lobby":
			response["data"] = await _op_join_lobby(command)
			response["ok"] = bool(response["data"].get("ok", false))
		"join_arranged_lobby":
			response["data"] = await _op_join_arranged_lobby(command)
			response["ok"] = bool(response["data"].get("ok", false))
		"set_lobby_properties":
			response["data"] = await _op_set_lobby_properties(command)
			response["ok"] = bool(response["data"].get("ok", false))
		"set_member_properties":
			response["data"] = await _op_set_member_properties(command)
			response["ok"] = bool(response["data"].get("ok", false))
		"inspect_lobby":
			response["data"] = _op_inspect_lobby(command)
			response["ok"] = bool(response["data"].get("ok", false))
		"inspect_lobbies":
			response["data"] = _op_inspect_lobbies()
			response["ok"] = bool(response["data"].get("ok", false))
		"leave_lobby":
			response["data"] = await _op_leave_lobby(command)
			response["ok"] = bool(response["data"].get("ok", false))
		"create_match_ticket":
			response["data"] = await _op_create_match_ticket(command)
			response["ok"] = bool(response["data"].get("ok", false))
		"inspect_match_ticket":
			response["data"] = _op_inspect_match_ticket(command)
			response["ok"] = bool(response["data"].get("ok", false))
		"wait_match_ticket":
			response["data"] = await _op_wait_match_ticket(command)
			response["ok"] = bool(response["data"].get("ok", false))
		"cancel_match_ticket":
			response["data"] = await _op_cancel_match_ticket()
			response["ok"] = bool(response["data"].get("ok", false))
		"leave_all":
			response["data"] = await _op_leave_all()
			response["ok"] = bool(response["data"].get("ok", false))
		"shutdown":
			response["data"] = await _op_shutdown()
			response["ok"] = bool(response["data"].get("ok", false))
		"exit":
			response["data"] = _ok({})
			response["ok"] = true
			_exit_requested = true
		_:
			response["ok"] = false
			response["data"] = _error("unknown_op", "Unknown worker op: %s" % op)

	if not bool(response["ok"]) and not bool(command.get("allow_error", false)):
		_exit_code = 1
	return response


func _op_sign_in(command: Dictionary) -> Dictionary:
	var playfab := _ensure_playfab()
	if playfab == null:
		return _error("playfab_unavailable", "PlayFab singleton is unavailable in worker.")

	if not playfab.is_initialized():
		var init_result = playfab.initialize()
		if init_result == null or not init_result.ok:
			return _result_payload(init_result, "PlayFab.initialize")

	var custom_id := str(command.get("custom_id", "")).strip_edges()
	if custom_id.is_empty():
		return _error("invalid_custom_id", "sign_in requires a custom_id.")

	var sign_in_result = await _await_completion(playfab.get_users().sign_in_with_custom_id_async(custom_id, bool(command.get("create_account", true))))
	if sign_in_result == null or not sign_in_result.ok:
		return _result_payload(sign_in_result, "PlayFab.users.sign_in_with_custom_id_async")
	_playfab_user = sign_in_result.data

	_multiplayer = playfab.get_multiplayer()
	var mp_result = await _await_completion(_multiplayer.initialize_async())
	if mp_result == null or not mp_result.ok:
		return _result_payload(mp_result, "PlayFab.multiplayer.initialize_async")

	return _ok({
		"custom_id": custom_id,
		"entity_key": _playfab_user.get_entity_key(),
		"multiplayer_initialized": _multiplayer.is_initialized(),
	})


func _op_create_lobby(command: Dictionary) -> Dictionary:
	if not _has_session():
		return _error("not_signed_in", "create_lobby requires sign_in first.")

	var config_data: Dictionary = command.get("config", {})
	var config = _instantiate_class("PlayFabLobbyConfig")
	config.max_players = int(config_data.get("max_players", 2))
	config.access_policy = int(config_data.get("access_policy", _class_constant("PlayFabLobbyConfig", "ACCESS_POLICY_PUBLIC")))
	config.search_properties = config_data.get("search_properties", {})
	config.lobby_properties = config_data.get("lobby_properties", {})
	config.member_properties = config_data.get("member_properties", {})

	var result = await _await_completion(_multiplayer.create_lobby_async(_playfab_user, config))
	if result == null or not result.ok:
		return _result_payload(result, "PlayFab.multiplayer.create_lobby_async")

	var created_lobby = result.data
	if bool(command.get("make_primary", true)):
		_primary_lobby = created_lobby
	_lobbies.append(created_lobby)
	return _ok({"lobby": _lobby_snapshot(created_lobby)})


func _op_search_lobbies(command: Dictionary) -> Dictionary:
	if not _has_session():
		return _error("not_signed_in", "search_lobbies requires sign_in first.")

	var config = _instantiate_class("PlayFabLobbySearchConfig")
	config.filter = str(command.get("filter", ""))
	config.order_by = str(command.get("order_by", ""))
	config.max_results = int(command.get("max_results", 10))

	var result = await _await_completion(_multiplayer.find_lobbies_async(_playfab_user, config))
	if result == null or not result.ok:
		return _result_payload(result, "PlayFab.multiplayer.find_lobbies_async")

	var summaries := []
	for summary in result.data.get_lobbies():
		summaries.append(_summary_snapshot(summary))
	return _ok({"lobbies": summaries})


func _op_join_lobby(command: Dictionary) -> Dictionary:
	if not _has_session():
		return _error("not_signed_in", "join_lobby requires sign_in first.")

	var connection_string := str(command.get("connection_string", "")).strip_edges()
	var join_config = _instantiate_class("PlayFabLobbyJoinConfig")
	join_config.member_properties = command.get("member_properties", {})

	var result = await _await_completion(_multiplayer.join_lobby_async(_playfab_user, connection_string, join_config))
	if result == null or not result.ok:
		return _result_payload(result, "PlayFab.multiplayer.join_lobby_async")

	_primary_lobby = result.data
	_lobbies.append(_primary_lobby)
	return _ok({"lobby": _lobby_snapshot(_primary_lobby)})


func _op_join_arranged_lobby(command: Dictionary) -> Dictionary:
	if not _has_session():
		return _error("not_signed_in", "join_arranged_lobby requires sign_in first.")

	var connection_string := str(command.get("connection_string", "")).strip_edges()
	if connection_string.is_empty() and _primary_ticket != null:
		connection_string = str(_primary_ticket.get_arranged_lobby_connection_string()).strip_edges()
	if connection_string.is_empty():
		return _error("invalid_arranged_lobby_connection_string", "join_arranged_lobby requires a non-empty connection string.")

	var join_config = _instantiate_class("PlayFabLobbyJoinConfig")
	join_config.member_properties = command.get("member_properties", {})

	var result = await _await_completion(_multiplayer.join_arranged_lobby_async(_playfab_user, connection_string, join_config))
	if result == null or not result.ok:
		return _result_payload(result, "PlayFab.multiplayer.join_arranged_lobby_async")

	_primary_lobby = result.data
	_lobbies.append(_primary_lobby)
	return _ok({"lobby": _lobby_snapshot(_primary_lobby)})


func _op_set_lobby_properties(command: Dictionary) -> Dictionary:
	var lobby := _lobby_for_command(command)
	if lobby == null:
		return _error("invalid_lobby", "set_lobby_properties requires a tracked lobby.")
	var result = await _await_completion(lobby.set_properties_async(command.get("properties", {})))
	if result == null or not result.ok:
		return _result_payload(result, "PlayFabLobby.set_properties_async")
	return _ok({"lobby": _lobby_snapshot(lobby)})


func _op_set_member_properties(command: Dictionary) -> Dictionary:
	var lobby := _lobby_for_command(command)
	if lobby == null:
		return _error("invalid_lobby", "set_member_properties requires a tracked lobby.")
	var result = await _await_completion(lobby.set_member_properties_async(command.get("properties", {})))
	if result == null or not result.ok:
		return _result_payload(result, "PlayFabLobby.set_member_properties_async")
	return _ok({"lobby": _lobby_snapshot(lobby)})


func _op_inspect_lobby(command: Dictionary) -> Dictionary:
	var lobby := _lobby_for_command(command)
	if lobby == null:
		return _error("invalid_lobby", "inspect_lobby requires a tracked lobby.")
	return _ok({"lobby": _lobby_snapshot(lobby)})


func _op_inspect_lobbies() -> Dictionary:
	var lobbies := []
	for lobby in _lobbies:
		if lobby != null:
			lobbies.append(_lobby_snapshot(lobby))
	return _ok({"lobbies": lobbies})


func _op_leave_lobby(command: Dictionary) -> Dictionary:
	var lobby := _lobby_for_command(command)
	if lobby == null:
		return _error("invalid_lobby", "leave_lobby requires a tracked lobby.")

	var lobby_id := str(lobby.get_lobby_id())
	var result = await _await_completion(lobby.leave_async())
	if result == null or not result.ok:
		return _result_payload(result, "PlayFabLobby.leave_async")
	_lobbies.erase(lobby)
	if _primary_lobby == lobby:
		_primary_lobby = _lobbies[0] if not _lobbies.is_empty() else null
	return _ok({"left": lobby_id})


func _op_create_match_ticket(command: Dictionary) -> Dictionary:
	if not _has_session():
		return _error("not_signed_in", "create_match_ticket requires sign_in first.")
	var queue_name := str(command.get("queue_name", "")).strip_edges()
	if queue_name.is_empty():
		return _error("matchmaking_queue_unconfigured", "No matchmaking queue was provided.")

	var config = _instantiate_class("PlayFabMatchmakingTicketConfig")
	config.queue_name = queue_name
	config.timeout_seconds = int(command.get("timeout_seconds", 60))
	var member = _instantiate_class("PlayFabMatchmakingMember")
	member.user = _playfab_user
	member.attributes = command.get("attributes", {})
	config.members = [member]

	var result = await _await_completion(_multiplayer.create_match_ticket_async(_playfab_user, config))
	if result == null or not result.ok:
		return _result_payload(result, "PlayFab.multiplayer.create_match_ticket_async")

	_primary_ticket = result.data
	_tickets.append(_primary_ticket)
	return _ok({"ticket": _ticket_snapshot(_primary_ticket)})


func _op_inspect_match_ticket(command: Dictionary) -> Dictionary:
	var ticket := _ticket_for_command(command)
	if ticket == null:
		return _error("invalid_match_ticket", "inspect_match_ticket requires a tracked ticket.")
	return _ok({"ticket": _ticket_snapshot(ticket)})


func _op_wait_match_ticket(command: Dictionary) -> Dictionary:
	var ticket := _ticket_for_command(command)
	if ticket == null:
		return _error("invalid_match_ticket", "wait_match_ticket requires a tracked ticket.")

	var expected_status := str(command.get("status_name", "matched")).strip_edges()
	var timeout_msec := int(command.get("timeout_msec", 120000))
	var started_msec := Time.get_ticks_msec()
	var status_name := ""
	while Time.get_ticks_msec() - started_msec < timeout_msec:
		_pump()
		var snapshot := _ticket_snapshot(ticket)
		status_name = str(snapshot.get("status_name", ""))
		if status_name == expected_status:
			return _ok({"ticket": snapshot})
		if status_name in ["cancelled", "failed"] and status_name != expected_status:
			return _error("match_ticket_%s" % status_name, "Match ticket reached terminal status '%s' before '%s'." % [status_name, expected_status])
		await process_frame
		OS.delay_msec(POLL_MSEC)
	return _error("timeout", "Timed out waiting for match ticket status '%s'; current status is '%s'." % [expected_status, status_name])


func _op_cancel_match_ticket() -> Dictionary:
	if _primary_ticket == null:
		return _error("invalid_match_ticket", "cancel_match_ticket requires a primary ticket.")
	var ticket = _primary_ticket
	var result = await _await_completion(_primary_ticket.cancel_async())
	if result == null or not result.ok:
		return _result_payload(result, "PlayFabMatchTicket.cancel_async")
	var snapshot := _ticket_snapshot(ticket)
	_tickets.erase(ticket)
	if _primary_ticket == ticket:
		_primary_ticket = null
	return _ok({"ticket": snapshot})


func _op_leave_all() -> Dictionary:
	var left := 0
	for lobby in _lobbies:
		if lobby == null:
			continue
		var result = await _await_completion(lobby.leave_async())
		if result != null and result.ok:
			left += 1
	_lobbies.clear()
	_primary_lobby = null
	return _ok({"left": left})


func _op_shutdown() -> Dictionary:
	if _multiplayer != null and _multiplayer.is_initialized():
		await _await_completion(_multiplayer.shutdown_async())
	if _playfab != null and _playfab.is_initialized():
		_playfab.shutdown()
	_playfab_user = null
	_multiplayer = null
	_primary_lobby = null
	_primary_ticket = null
	_lobbies.clear()
	_tickets.clear()
	return _ok({})


func _has_session() -> bool:
	return _playfab_user != null and _multiplayer != null and _multiplayer.is_initialized()


func _lobby_for_command(command: Dictionary) -> Object:
	var lobby_id := str(command.get("lobby_id", "")).strip_edges()
	if lobby_id.is_empty():
		return _primary_lobby
	return _find_lobby_by_id(lobby_id)


func _find_lobby_by_id(lobby_id: String) -> Object:
	for lobby in _lobbies:
		if lobby != null and str(lobby.get_lobby_id()) == lobby_id:
			return lobby
	return null


func _ticket_for_command(command: Dictionary) -> Object:
	var ticket_id := str(command.get("ticket_id", "")).strip_edges()
	if ticket_id.is_empty():
		return _primary_ticket
	for ticket in _tickets:
		if ticket != null and str(ticket.get_ticket_id()) == ticket_id:
			return ticket
	return null


func _instantiate_class(target_class: String) -> Object:
	if not ClassDB.class_exists(target_class) or not ClassDB.can_instantiate(target_class):
		return null
	return ClassDB.instantiate(target_class)


func _class_constant(target_class: String, constant_name: String) -> int:
	return ClassDB.class_get_integer_constant(target_class, constant_name)


func _result_payload(result: Variant, label: String) -> Dictionary:
	if result == null:
		return _error("timeout", "%s timed out." % label)
	return {
		"ok": bool(result.ok),
		"code": str(result.code),
		"message": str(result.message),
		"data": _json_safe(result.data),
	}


func _ok(data: Dictionary) -> Dictionary:
	var payload := data.duplicate(true)
	payload["ok"] = true
	return payload


func _error(code: String, message: String) -> Dictionary:
	return {
		"ok": false,
		"code": code,
		"message": message,
	}


func _lobby_snapshot(lobby: Object) -> Dictionary:
	if lobby == null:
		return {}
	var members := []
	for member in lobby.get_members():
		members.append({
			"user_id": member.get_user_id(),
			"entity_key": member.get_entity_key(),
			"properties": member.get_properties(),
			"is_local": member.is_local_member(),
		})
	return {
		"lobby_id": lobby.get_lobby_id(),
		"connection_string": lobby.get_connection_string(),
		"owner_entity_key": lobby.get_owner_entity_key(),
		"max_member_count": lobby.get_max_member_count(),
		"member_count": lobby.get_member_count(),
		"properties": lobby.get_properties(),
		"search_properties": lobby.get_search_properties(),
		"members": members,
	}


func _summary_snapshot(summary: Object) -> Dictionary:
	if summary == null:
		return {}
	return {
		"lobby_id": summary.get_lobby_id(),
		"connection_string": summary.get_connection_string(),
		"owner_entity_key": summary.get_owner_entity_key(),
		"max_member_count": summary.get_max_member_count(),
		"member_count": summary.get_member_count(),
		"search_properties": summary.get_search_properties(),
		"lobby_properties": summary.get_lobby_properties(),
	}


func _ticket_snapshot(ticket: Object) -> Dictionary:
	if ticket == null:
		return {}
	var properties: Dictionary = ticket.get_properties()
	return {
		"ticket_id": ticket.get_ticket_id(),
		"queue_name": ticket.get_queue_name(),
		"status": ticket.get_status(),
		"status_name": str(properties.get("status_name", "")),
		"match_id": ticket.get_match_id(),
		"arranged_lobby_connection_string": ticket.get_arranged_lobby_connection_string(),
		"is_complete": ticket.is_complete(),
		"is_cancelled": ticket.is_cancelled(),
		"properties": properties,
	}


func _json_safe(value: Variant) -> Variant:
	match typeof(value):
		TYPE_NIL, TYPE_BOOL, TYPE_INT, TYPE_FLOAT, TYPE_STRING:
			return value
		TYPE_DICTIONARY:
			var output := {}
			for key in value.keys():
				output[str(key)] = _json_safe(value[key])
			return output
		TYPE_ARRAY:
			var output := []
			for item in value:
				output.append(_json_safe(item))
			return output
		TYPE_OBJECT:
			var object_value: Object = value
			if object_value == null:
				return null
			if object_value.has_method("get_lobby_id"):
				return _lobby_snapshot(object_value)
			if object_value.has_method("get_ticket_id"):
				return _ticket_snapshot(object_value)
			if object_value.has_method("get_entity_key"):
				return object_value.get_entity_key()
			return str(object_value)
		_:
			return str(value)


func _command_path() -> String:
	return _run_dir.path_join("%s.command.json" % _worker_id)


func _response_path(seq: int) -> String:
	return _run_dir.path_join("%s.response.%d.json" % [_worker_id, seq])


func _ready_path() -> String:
	return _run_dir.path_join("%s.ready.json" % _worker_id)


func _read_command() -> Dictionary:
	var path := _command_path()
	if not FileAccess.file_exists(path):
		return {}

	var text := FileAccess.get_file_as_string(path)
	DirAccess.remove_absolute(path)
	var parsed = JSON.parse_string(text)
	if typeof(parsed) != TYPE_DICTIONARY:
		return {"op": "invalid_json", "seq": 0}
	return parsed


func _write_json(path: String, payload: Dictionary) -> void:
	var file := FileAccess.open(path, FileAccess.WRITE)
	if file == null:
		printerr("failed to write worker json: ", path)
		return
	file.store_string(JSON.stringify(payload))
	file.close()
