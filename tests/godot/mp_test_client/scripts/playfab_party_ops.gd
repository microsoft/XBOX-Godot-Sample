## PlayFabPartyOps — handle-based Party command implementations.
##
## Mirrors the patterns from sample/tutorial_app/autoload/party.gd
## (initialize → create_and_join_network_async → leave_async → send_text_async)
## but tracks networks by orchestrator-supplied handle instead of a singleton
## `_network`, and skips the lobby-descriptor publishing (the orchestrator
## brokers the descriptor between host and guest directly via response
## payloads + scenario params, no live lobby required for the Party-only
## scenarios).
##
## Handle addressing (per spec/playfab-multiplayer-test-automation/4-scenario-authoring.md):
##   * `params.as` on party_create_network / party_join_network names the new
##     network (default "main").
##   * `params.handle` on subsequent ops addresses a tracked network (default "main").
##
## Each command returns the CommandDispatcher shape:
##   { ok: bool, result?: Dictionary, error?: Dictionary }
##
## Deferred for a follow-up: RPC over the Godot MultiplayerAPI binding
## (requires wiring `multiplayer.multiplayer_peer = network.local_peer`),
## chat-control event streaming back to the orchestrator (requires the
## `event` frame kind), and the per-peer mute / set_peer_chat_permissions
## surfaces. The minimal set here is enough to validate that same-host
## Party can stand up at all post PR #132.
extends RefCounted

const PlayFabRuntime := preload("res://scripts/playfab_runtime.gd")

const DEFAULT_HANDLE := "main"
const DEFAULT_AWAIT_TIMEOUT_MS := 60_000
const LEAVE_TIMEOUT_MS := 30_000
const SEND_TEXT_TIMEOUT_MS := 10_000
# Bounded retry for the join chain when the host's network races us. The
# typical surface here is `party_resource_not_ready: Network destroyed
# during join.` (or, post addon improvements, the underlying Party
# errorDetail message), which is transient — the host's network is alive
# at this point, but a same-host Party DoWork batch can collide with the
# guest's join when both processes share one machine.
const JOIN_RETRY_ATTEMPTS := 3
const JOIN_RETRY_BACKOFF_MS := 1_500

var _runtime: PlayFabRuntime = null
var _party_initialized: bool = false
var _networks: Dictionary = {}  # handle (String) -> PlayFabPartyNetwork (Object)


func bind(runtime: PlayFabRuntime) -> void:
	_runtime = runtime


func handles() -> Array:
	return _networks.keys()


func has_network(handle: String) -> bool:
	return _networks.has(handle) and _networks[handle] != null


# ---------------------------------------------------------------------------
# Commands
# ---------------------------------------------------------------------------

func initialize_party(params: Dictionary) -> Dictionary:
	var pf_err: Dictionary = _require_playfab("party_initialize")
	if not pf_err.is_empty():
		return pf_err
	var party: Object = _runtime.get_playfab().get_party()
	if party == null:
		return _err("party_unavailable", "PlayFab.get_party() returned null")
	if party.is_initialized():
		_party_initialized = true
		return _ok({ "already_initialized": true })

	var config: Object = _instantiate("PlayFabPartyConfig")
	if config == null:
		return _err("class_unavailable", "PlayFabPartyConfig not registered in ClassDB")
	config.max_players = int(params.get("max_players", 8))
	var connectivity: int = int(params.get("direct_peer_connectivity",
		ClassDB.class_get_integer_constant("PlayFabParty", "DIRECT_PEER_CONNECTIVITY_ANY")))
	config.direct_peer_connectivity = connectivity
	# Default text-only to keep scenarios deterministic — voice depends on
	# audio device probing which is noisy in headless CI. Scenarios can
	# explicitly pass enable_voice_chat to override.
	config.enable_voice_chat = bool(params.get("enable_voice_chat", false))
	config.enable_text_chat = bool(params.get("enable_text_chat", true))
	config.enable_transcription = bool(params.get("enable_transcription", false))

	var result: Variant = await _runtime.await_completion(party.initialize_async(config), DEFAULT_AWAIT_TIMEOUT_MS)
	if result == null or not bool(result.ok):
		return _err_from_result(result, "PlayFabParty.initialize_async")
	_party_initialized = true
	return _ok({ "max_players": int(config.max_players), "direct_peer_connectivity": connectivity })


func create_network(params: Dictionary) -> Dictionary:
	var session_err: Dictionary = _require_session("party_create_network")
	if not session_err.is_empty():
		return session_err
	var handle: String = String(params.get("as", DEFAULT_HANDLE))
	if has_network(handle):
		return _err("handle_in_use", "party network handle '%s' is already tracked" % handle)
	if not _party_initialized:
		var init_resp: Dictionary = await initialize_party(params)
		if not bool(init_resp.get("ok", false)):
			return init_resp

	var party: Object = _runtime.get_playfab().get_party()
	var user: Object = _runtime.get_user()

	var config: Object = _instantiate("PlayFabPartyConfig")
	if config == null:
		return _err("class_unavailable", "PlayFabPartyConfig not registered in ClassDB")
	config.max_players = int(params.get("max_players", 4))
	config.direct_peer_connectivity = int(params.get("direct_peer_connectivity",
		ClassDB.class_get_integer_constant("PlayFabParty", "DIRECT_PEER_CONNECTIVITY_ANY")))
	config.enable_voice_chat = bool(params.get("enable_voice_chat", false))
	config.enable_text_chat = bool(params.get("enable_text_chat", true))
	# Microsoft Party requires every user to authenticate with the same
	# invitation_id the host created the network with. Default to a stable
	# string derived from the handle so the host/guest scenario can pass
	# the same value on both sides without needing a lobby.
	config.invitation_id = String(params.get("invitation_id", "mp-test-invite-%s" % handle))

	var result: Variant = await _runtime.await_completion(party.create_and_join_network_async(user, config), DEFAULT_AWAIT_TIMEOUT_MS)
	if result == null or not bool(result.ok):
		return _err_from_result(result, "PlayFabParty.create_and_join_network_async")

	var network: Object = result.data
	if network == null:
		return _err("invalid_response", "create_and_join_network_async returned ok with null network")
	_networks[handle] = network
	return _ok({
		"handle": handle,
		"invitation_id": String(config.invitation_id),
		"network": _network_snapshot(network),
	})


func join_network(params: Dictionary) -> Dictionary:
	var session_err: Dictionary = _require_session("party_join_network")
	if not session_err.is_empty():
		return session_err
	var handle: String = String(params.get("as", DEFAULT_HANDLE))
	if has_network(handle):
		return _err("handle_in_use", "party network handle '%s' is already tracked" % handle)
	var descriptor: String = String(params.get("descriptor", ""))
	if descriptor.is_empty():
		return _err("missing_descriptor", "party_join_network requires non-empty descriptor")
	var invitation_id: String = String(params.get("invitation_id", ""))
	if invitation_id.is_empty():
		return _err("missing_invitation_id", "party_join_network requires non-empty invitation_id (the host's create-time value)")

	if not _party_initialized:
		var init_resp: Dictionary = await initialize_party(params)
		if not bool(init_resp.get("ok", false)):
			return init_resp

	var party: Object = _runtime.get_playfab().get_party()
	var user: Object = _runtime.get_user()

	var config: Object = _instantiate("PlayFabPartyConfig")
	if config == null:
		return _err("class_unavailable", "PlayFabPartyConfig not registered in ClassDB")
	config.enable_voice_chat = bool(params.get("enable_voice_chat", false))
	config.enable_text_chat = bool(params.get("enable_text_chat", true))
	config.invitation_id = invitation_id

	# Bounded retry for the transient `party_resource_not_ready` family
	# observed in same-host Party scenarios (host network already alive +
	# the addon already surfaces the real PartyNetworkDestroyed reason via
	# _abort_join_op_if_network_dead, so persistent failures are propagated
	# verbatim after the retries are exhausted).
	var attempts: int = max(1, int(params.get("retry_attempts", JOIN_RETRY_ATTEMPTS)))
	var backoff_ms: int = max(0, int(params.get("retry_backoff_ms", JOIN_RETRY_BACKOFF_MS)))
	var result: Variant = null
	var last_err: Dictionary = {}
	for attempt in range(attempts):
		result = await _runtime.await_completion(party.join_network_async(user, descriptor, config), DEFAULT_AWAIT_TIMEOUT_MS)
		if result != null and bool(result.ok):
			break
		last_err = _err_from_result(result, "PlayFabParty.join_network_async")
		var code: String = String(last_err.get("error", {}).get("code", ""))
		if code != "party_resource_not_ready":
			return last_err
		if attempt + 1 >= attempts:
			return last_err
		# Sleep + pump the SDK so any pending NetworkDestroyed/state-change
		# events get drained before the next attempt re-issues join.
		await _runtime.wait_until(func(): return false, backoff_ms)
	if result == null or not bool(result.ok):
		return last_err if not last_err.is_empty() else _err_from_result(result, "PlayFabParty.join_network_async")

	var network: Object = result.data
	if network == null:
		return _err("invalid_response", "join_network_async returned ok with null network")
	_networks[handle] = network
	return _ok({
		"handle": handle,
		"invitation_id": invitation_id,
		"network": _network_snapshot(network),
	})


func get_snapshot(params: Dictionary) -> Dictionary:
	var lookup: Dictionary = _lookup_network(params, "party_snapshot")
	if not lookup.has("network"):
		return lookup
	var handle: String = lookup.get("handle", DEFAULT_HANDLE)
	var network: Object = lookup.get("network")
	return _ok({ "handle": handle, "network": _network_snapshot(network) })


func leave_network(params: Dictionary) -> Dictionary:
	var lookup: Dictionary = _lookup_network(params, "party_leave_network")
	if not lookup.has("network"):
		return lookup
	var handle: String = lookup.get("handle", DEFAULT_HANDLE)
	var network: Object = lookup.get("network")
	var network_id: String = String(network.network_id) if "network_id" in network else ""
	var result: Variant = await _runtime.await_completion(network.leave_async(), LEAVE_TIMEOUT_MS)
	# Only release the handle on a successful leave so reset_client can retry.
	# Leaking the handle on failure would let Party membership persist into
	# the next scenario.
	if result == null or not bool(result.ok):
		return _err_from_result(result, "PlayFabPartyNetwork.leave_async")
	_networks.erase(handle)
	return _ok({ "handle": handle, "left_network_id": network_id })


func send_chat_text(params: Dictionary) -> Dictionary:
	var lookup: Dictionary = _lookup_network(params, "party_send_chat_text")
	if not lookup.has("network"):
		return lookup
	var network: Object = lookup.get("network")
	var text: String = String(params.get("text", ""))
	if text.is_empty():
		return _err("missing_text", "party_send_chat_text requires non-empty text")
	var peer: Object = network.local_peer if "local_peer" in network else null
	if peer == null:
		return _err("peer_not_ready", "PartyNetwork.local_peer is null (still connecting?)")
	var result: Variant = await _runtime.await_completion(peer.send_text_async(text), SEND_TEXT_TIMEOUT_MS)
	if result == null or not bool(result.ok):
		return _err_from_result(result, "PlayFabPartyPeer.send_text_async")
	return _ok({ "handle": lookup.get("handle", DEFAULT_HANDLE), "text": text })


## Leaves every tracked network. Called from reset_client between scenarios
## so a scenario can't leak a tracked network into the next scenario's
## lifecycle.
func reset(_params: Dictionary) -> Dictionary:
	var left: int = 0
	var failures: Array = []
	for handle in _networks.keys():
		var network: Object = _networks[handle]
		if network == null:
			continue
		var result: Variant = await _runtime.await_completion(network.leave_async(), LEAVE_TIMEOUT_MS)
		if result != null and bool(result.ok):
			left += 1
		else:
			# Surface failures so reset_client returns an error and the
			# orchestrator respawns the client. Clearing _networks while
			# the SDK still tracks the network would let Party membership
			# (chat, RPC routing, voice) bleed into the next scenario.
			var err_payload: Dictionary = _err_from_result(result, "PlayFabPartyNetwork.leave_async")
			var err_details: Dictionary = err_payload.get("error", {})
			err_details["handle"] = handle
			failures.append(err_details)
	_networks.clear()
	# Intentionally do NOT call PlayFab.party.shutdown_async() between
	# scenarios — initialization is expensive (audio engine + network
	# stack) and is meant to be amortized across the client's lifetime.
	if not failures.is_empty():
		return _err(
			"reset_failed",
			"failed to leave %d Party network(s) during reset; respawn required" % failures.size(),
			{ "left": left, "failed": failures },
		)
	return _ok({ "left": left })


# ---------------------------------------------------------------------------
# Helpers
# ---------------------------------------------------------------------------

func _require_playfab(op: String) -> Dictionary:
	if _runtime == null or _runtime.get_playfab() == null:
		return _err("playfab_unavailable", "%s requires PlayFab extension" % op)
	return {}


func _require_session(op: String) -> Dictionary:
	var pf_err: Dictionary = _require_playfab(op)
	if not pf_err.is_empty():
		return pf_err
	if not _runtime.has_user():
		return _err("not_signed_in", "%s requires sign_in first" % op)
	return {}


func _lookup_network(params: Dictionary, op: String) -> Dictionary:
	var handle: String = String(params.get("handle", DEFAULT_HANDLE))
	if not has_network(handle):
		return _err("unknown_handle", "%s: no tracked party network for handle '%s' (have %s)" % [op, handle, str(_networks.keys())])
	return { "handle": handle, "network": _networks[handle] }


func _network_snapshot(network: Object) -> Dictionary:
	if network == null:
		return {}
	var snap: Dictionary = {}
	if "network_id" in network:
		snap["network_id"] = String(network.network_id)
	if "descriptor" in network:
		snap["descriptor"] = String(network.descriptor)
	var peer: Object = network.local_peer if "local_peer" in network else null
	if peer != null:
		snap["local_peer_unique_id"] = peer.get_unique_id() if peer.has_method("get_unique_id") else 0
		if peer.has_method("get_peers"):
			var raw: Array = peer.get_peers()
			var peer_ids: Array = []
			for raw_id in raw:
				peer_ids.append(int(raw_id))
			snap["peer_ids"] = peer_ids
			snap["peer_count"] = peer_ids.size()
		else:
			snap["peer_ids"] = []
			snap["peer_count"] = 0
	else:
		snap["peer_ids"] = []
		snap["peer_count"] = 0
	return snap


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
