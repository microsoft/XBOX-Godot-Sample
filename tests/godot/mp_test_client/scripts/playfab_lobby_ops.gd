## PlayFabLobbyOps — handle-based lobby command implementations.
##
## Each command method takes a Dictionary `params` (the orchestrator request
## payload) and returns a Dictionary in CommandDispatcher's shape:
##   { ok: bool, result?: Dictionary, error?: Dictionary }
##
## Handle addressing (per the locked architecture in
## spec/playfab-multiplayer-test-automation/4-scenario-authoring.md):
##   * `params.as` on create_lobby/join_lobby names the new lobby (default "main").
##   * `params.handle` on subsequent ops addresses a tracked lobby (default "main").
##
## Lifecycle: scenarios that call create/join are expected to call leave_lobby
## or reset before declaring success. reset_client() also closes any tracked
## lobbies as part of the locked "fresh-process per scenario tier" contract.
extends RefCounted

const PlayFabRuntime := preload("res://scripts/playfab_runtime.gd")

const DEFAULT_HANDLE := "main"
const DEFAULT_TIMEOUT_MS := 60_000
const LEAVE_TIMEOUT_MS := 30_000
# How long to wait for the local SDK cache to converge after a
# set_*_properties_async call. PlayFab acknowledges the property write
# at the service synchronously (via the awaited Result), but the local
# Lobby cache is updated when the corresponding state-change event is
# dispatched in a later tick. Polling for ~10s is generous; convergence
# in practice is in the low hundreds of milliseconds.
const CONVERGENCE_TIMEOUT_MS := 10_000

var _runtime: PlayFabRuntime = null
var _lobbies: Dictionary = {}  # handle (String) -> PlayFabLobby (Object)


func bind(runtime: PlayFabRuntime) -> void:
	_runtime = runtime


func handles() -> Array:
	return _lobbies.keys()


func has_lobby(handle: String) -> bool:
	return _lobbies.has(handle) and _lobbies[handle] != null


# ---------------------------------------------------------------------------
# Commands
# ---------------------------------------------------------------------------

func create_lobby(params: Dictionary) -> Dictionary:
	var session_err: Dictionary = _require_session("create_lobby")
	if not session_err.is_empty():
		return session_err
	var handle: String = String(params.get("as", DEFAULT_HANDLE))
	if has_lobby(handle):
		return _err("handle_in_use", "lobby handle '%s' is already tracked" % handle)

	var config: Object = _instantiate("PlayFabLobbyConfig")
	if config == null:
		return _err("class_unavailable", "PlayFabLobbyConfig not registered in ClassDB")
	var cfg_dict: Dictionary = params.get("config", params)
	config.max_players = int(cfg_dict.get("max_players", 4))
	var access_policy_value: Variant = cfg_dict.get("access_policy", null)
	if access_policy_value != null:
		config.access_policy = int(access_policy_value)
	config.search_properties = cfg_dict.get("search_properties", {})
	config.lobby_properties = cfg_dict.get("lobby_properties", {})
	config.member_properties = cfg_dict.get("member_properties", {})

	var result: Variant = await _runtime.await_completion_with_rate_limit_retry(
		func(): return _runtime.get_multiplayer().create_lobby_async(_runtime.get_user(), config),
		"create_lobby_async",
		int(params.get("timeout_ms", DEFAULT_TIMEOUT_MS)),
	)
	if result == null or not bool(result.ok):
		return _err_from_result(result, "create_lobby_async")
	_lobbies[handle] = result.data
	return _ok({ "handle": handle, "lobby": _lobby_snapshot(result.data) })


func join_lobby(params: Dictionary) -> Dictionary:
	var session_err: Dictionary = _require_session("join_lobby")
	if not session_err.is_empty():
		return session_err
	var handle: String = String(params.get("as", DEFAULT_HANDLE))
	if has_lobby(handle):
		return _err("handle_in_use", "lobby handle '%s' is already tracked" % handle)
	var connection_string: String = String(params.get("connection_string", "")).strip_edges()
	if connection_string.is_empty():
		return _err("invalid_connection_string", "join_lobby requires a non-empty connection_string")

	var join_config: Object = _instantiate("PlayFabLobbyJoinConfig")
	if join_config == null:
		return _err("class_unavailable", "PlayFabLobbyJoinConfig not registered in ClassDB")
	join_config.member_properties = params.get("member_properties", {})

	var result: Variant = await _runtime.await_completion_with_rate_limit_retry(
		func(): return _runtime.get_multiplayer().join_lobby_async(_runtime.get_user(), connection_string, join_config),
		"join_lobby_async",
		int(params.get("timeout_ms", DEFAULT_TIMEOUT_MS)),
	)
	if result == null or not bool(result.ok):
		return _err_from_result(result, "join_lobby_async")
	_lobbies[handle] = result.data
	return _ok({ "handle": handle, "lobby": _lobby_snapshot(result.data) })


func search_lobbies(params: Dictionary) -> Dictionary:
	var session_err: Dictionary = _require_session("search_lobbies")
	if not session_err.is_empty():
		return session_err
	var config: Object = _instantiate("PlayFabLobbySearchConfig")
	if config == null:
		return _err("class_unavailable", "PlayFabLobbySearchConfig not registered in ClassDB")
	config.filter = String(params.get("filter", ""))
	config.order_by = String(params.get("order_by", ""))
	config.max_results = int(params.get("max_results", 10))

	var result: Variant = await _runtime.await_completion_with_rate_limit_retry(
		func(): return _runtime.get_multiplayer().find_lobbies_async(_runtime.get_user(), config),
		"find_lobbies_async",
		int(params.get("timeout_ms", DEFAULT_TIMEOUT_MS)),
	)
	if result == null or not bool(result.ok):
		return _err_from_result(result, "find_lobbies_async")
	var summaries: Array = []
	for s in result.data.get_lobbies():
		summaries.append(_summary_snapshot(s))
	return _ok({ "lobbies": summaries, "count": summaries.size() })


func set_lobby_properties(params: Dictionary) -> Dictionary:
	var lookup: Dictionary = _lookup_lobby(params, "set_lobby_properties")
	if not lookup.has("lobby"):
		return lookup
	var lobby: Object = lookup["lobby"]
	var requested: Dictionary = params.get("properties", {})
	var result: Variant = await _runtime.await_completion_with_rate_limit_retry(
		func(): return lobby.set_properties_async(requested),
		"set_properties_async",
		int(params.get("timeout_ms", DEFAULT_TIMEOUT_MS)),
	)
	if result == null or not bool(result.ok):
		return _err_from_result(result, "set_properties_async")
	# Wait for the local SDK cache to reflect the change. set_properties_async
	# ACKs at the service synchronously but the local Lobby cache is
	# event-driven; without this wait, an immediate _lobby_snapshot can
	# return stale (pre-change) properties.
	var converged: bool = await _runtime.wait_until(
		func(): return _properties_match(lobby.get_properties(), requested),
		int(params.get("convergence_timeout_ms", CONVERGENCE_TIMEOUT_MS)),
	)
	if not converged:
		return _err(
			"convergence_timeout",
			"lobby properties did not converge to set values within %dms (last=%s, expected=%s)" % [
				int(params.get("convergence_timeout_ms", CONVERGENCE_TIMEOUT_MS)),
				str(lobby.get_properties()),
				str(requested),
			],
		)
	return _ok({ "handle": lookup["handle"], "lobby": _lobby_snapshot(lobby) })


func set_member_properties(params: Dictionary) -> Dictionary:
	var lookup: Dictionary = _lookup_lobby(params, "set_member_properties")
	if not lookup.has("lobby"):
		return lookup
	var lobby: Object = lookup["lobby"]
	var requested: Dictionary = params.get("properties", {})
	var result: Variant = await _runtime.await_completion_with_rate_limit_retry(
		func(): return lobby.set_member_properties_async(requested),
		"set_member_properties_async",
		int(params.get("timeout_ms", DEFAULT_TIMEOUT_MS)),
	)
	if result == null or not bool(result.ok):
		return _err_from_result(result, "set_member_properties_async")
	# Wait for the local member's properties to reflect the change. Same
	# eventual-consistency rationale as set_lobby_properties above.
	var converged: bool = await _runtime.wait_until(
		func(): return _local_member_properties_match(lobby, requested),
		int(params.get("convergence_timeout_ms", CONVERGENCE_TIMEOUT_MS)),
	)
	if not converged:
		var local_props: Variant = _get_local_member_properties(lobby)
		return _err(
			"convergence_timeout",
			"local member properties did not converge to set values within %dms (last=%s, expected=%s)" % [
				int(params.get("convergence_timeout_ms", CONVERGENCE_TIMEOUT_MS)),
				str(local_props),
				str(requested),
			],
		)
	return _ok({ "handle": lookup["handle"], "lobby": _lobby_snapshot(lobby) })


# ---------------------------------------------------------------------------
# Convergence helpers (eventual-consistency for property writes)
# ---------------------------------------------------------------------------

## Returns true when `current` contains every entry in `expected`:
## non-null expected values must match (as String), null expected values
## must be absent (or also null) in current. Extra keys in `current` are
## ignored — scenarios assert membership, not equality.
func _properties_match(current: Dictionary, expected: Dictionary) -> bool:
	for key in expected.keys():
		var want: Variant = expected[key]
		var got: Variant = current.get(key, null)
		if want == null:
			if current.has(key) and got != null:
				return false
		else:
			if String(got) != String(want):
				return false
	return true


func _get_local_member_properties(lobby: Object) -> Variant:
	if lobby == null:
		return null
	for member in lobby.get_members():
		# The addon's PlayFabLobbyMember binding exposes the local-self flag
		# via the `is_local` property / `is_local_member()` method (see
		# addons/godot_playfab/doc_classes/PlayFabLobbyMember.xml). The earlier
		# `get_is_local()` name does not exist and would always return null,
		# making this loop silently skip every member.
		if bool(member.is_local_member()):
			return member.get_properties()
	return null


func _local_member_properties_match(lobby: Object, expected: Dictionary) -> bool:
	var props: Variant = _get_local_member_properties(lobby)
	if typeof(props) != TYPE_DICTIONARY:
		return false
	return _properties_match(props, expected)


func get_lobby_snapshot(params: Dictionary) -> Dictionary:
	var lookup: Dictionary = _lookup_lobby(params, "get_lobby_snapshot")
	if not lookup.has("lobby"):
		return lookup
	return _ok({ "handle": lookup["handle"], "lobby": _lobby_snapshot(lookup["lobby"]) })


func leave_lobby(params: Dictionary) -> Dictionary:
	var lookup: Dictionary = _lookup_lobby(params, "leave_lobby")
	if not lookup.has("lobby"):
		return lookup
	var lobby: Object = lookup["lobby"]
	var handle: String = lookup["handle"]
	var lobby_id: String = String(lobby.get_lobby_id())
	var result: Variant = await _runtime.await_completion_with_rate_limit_retry(
		func(): return lobby.leave_async(),
		"leave_lobby_async",
		int(params.get("timeout_ms", LEAVE_TIMEOUT_MS)),
	)
	# Only release the handle on a successful leave so reset_client can
	# retry the leave if this call failed/timed out. Leaking the handle on
	# failure would let lobby membership persist into the next scenario.
	if result == null or not bool(result.ok):
		return _err_from_result(result, "leave_async")
	_lobbies.erase(handle)
	return _ok({ "handle": handle, "left_lobby_id": lobby_id })


func reset(_params: Dictionary = {}) -> Dictionary:
	var left: int = 0
	var failures: Array = []
	for handle in _lobbies.keys():
		var lobby = _lobbies[handle]
		if lobby == null:
			continue
		var result: Variant = await _runtime.await_completion(lobby.leave_async(), LEAVE_TIMEOUT_MS)
		if result != null and bool(result.ok):
			left += 1
		else:
			# Surface the failure so reset_client returns an error and the
			# orchestrator respawns the client process. Swallowing the error
			# and clearing _lobbies would leave the lobby joined server-side
			# (the local handle drops but the server still tracks
			# membership), leaking state into the next scenario.
			var err_payload: Dictionary = _err_from_result(result, "leave_async")
			var err_details: Dictionary = err_payload.get("error", {})
			err_details["handle"] = handle
			failures.append(err_details)
	_lobbies.clear()
	if not failures.is_empty():
		return _err(
			"reset_failed",
			"failed to leave %d lobby/lobbies during reset; respawn required" % failures.size(),
			{ "left": left, "failed": failures },
		)
	return _ok({ "left": left })


# ---------------------------------------------------------------------------
# Helpers
# ---------------------------------------------------------------------------

func _require_session(op: String) -> Dictionary:
	if _runtime == null or not _runtime.has_user():
		return _err("not_signed_in", "%s requires sign_in first" % op)
	if _runtime.get_multiplayer() == null or not _runtime.get_multiplayer().is_initialized():
		return _err("multiplayer_not_initialized", "%s requires multiplayer.initialize first" % op)
	return {}


func _lookup_lobby(params: Dictionary, op: String) -> Dictionary:
	var handle: String = String(params.get("handle", DEFAULT_HANDLE))
	if not has_lobby(handle):
		return _err("unknown_handle", "%s: no tracked lobby for handle '%s' (have %s)" % [op, handle, str(_lobbies.keys())])
	return { "handle": handle, "lobby": _lobbies[handle] }


func _lobby_snapshot(lobby: Object) -> Dictionary:
	if lobby == null:
		return {}
	var members: Array = []
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
