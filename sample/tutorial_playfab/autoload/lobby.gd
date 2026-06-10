extends Node

const AddonApi = preload("res://shared/addon_api.gd")

## PlayFab Tutorial 3 — Lobby (host / join / leave / search).
##
## PlayFab-only variant of the lobby autoload. Unlike the integrated
## track's lobby, this one is pure PlayFab: there is no Xbox multiplayer
## privilege gate, no Xbox Social friends list, no Multiplayer Activity
## mirroring, and no Xbox presence. Identity comes from the PlayFabAuth
## autoload (a custom-id PlayFab session). Discovery uses PlayFab-native
## connection strings plus find_lobbies_async; cross-device handoff is by
## connection string (the binding surfaces inbound invites through
## invite_received but does not expose an invite-send call).
##
## Item 9 — single-slot design (intentional for the sample). This autoload
## owns exactly one PlayFabLobby at a time (`_lobby`). The host/join APIs
## reject re-entrant calls and the panels assume a single live lobby.
##
## Source: docs/tutorials/playfab/03-lobby.md

enum State {
	UNINITIALIZED,    ## Autoload _ready has not finished sign-in.
	READY,            ## Signed in, no lobby; host/join allowed.
	HOSTING,          ## host_lobby() in flight (create_lobby_async).
	JOINING,          ## join_lobby() in flight (join_lobby_async).
	IN_LOBBY,         ## Active PlayFabLobby; leave allowed.
	LEAVING,          ## leave_lobby() in flight (leave_async).
}

signal state_changed(state: State)
signal lobby_joined(lobby)
signal lobby_left
signal lobby_disconnected  ## PlayFab fired DISCONNECTED firehose; involuntary.
signal invite_received(connection_string: String, sender_user_id: String)

var _state: State = State.UNINITIALIZED
var _lobby = null
var _auth: Node = null
var _pf_multiplayer_signals_connected: bool = false

## Guarded accessor — only returns the live PlayFabLobby while we are
## actually in IN_LOBBY.
var current_lobby:
	get:
		return _lobby if _state == State.IN_LOBBY else null

func _ready() -> void:
	_auth = get_node_or_null("/root/PlayFabAuth")
	if _auth == null:
		push_error("[Lobby] PlayFabAuth autoload missing")
		return
	await _ensure_ready()

func get_state() -> State:
	return _state

func is_ready() -> bool:
	return _state == State.READY

func is_in_lobby() -> bool:
	return _state == State.IN_LOBBY

func is_busy() -> bool:
	return _state == State.HOSTING or _state == State.JOINING or _state == State.LEAVING

func get_current_lobby():
	return current_lobby

func _set_state(next: State) -> void:
	if _state == next:
		return
	_state = next
	state_changed.emit(_state)

## Awaits sign-in (idempotent across concurrent callers, since
## PlayFabAuth.sign_in itself coalesces). Returns true when we are in a
## usable post-sign-in state, false on terminal sign-in failure.
func _ensure_ready() -> bool:
	if _state == State.READY or _state == State.IN_LOBBY:
		return true
	if is_busy():
		return false
	if not await _auth.call("sign_in"):
		push_warning("[Lobby] sign-in failed (%s) — staying UNINITIALIZED" % _auth.call("get_last_error_stage"))
		return false
	# Another concurrent _ensure_ready may have raced us through the
	# sign_in await. Re-check the state before claiming the slot.
	if _state != State.UNINITIALIZED:
		return _state == State.READY or _state == State.IN_LOBBY
	_set_state(State.READY)
	return true

# Item 10 — lazy initialization. PlayFab.multiplayer is large and most
# single-player titles never touch it; bring it up on first lobby action
# and connect the Multiplayer signals exactly once on success.
func _ensure_pf_multiplayer_initialized() -> bool:
	if not Engine.has_singleton("PlayFab"):
		push_error("[Lobby] PlayFab extension not loaded")
		return false

	if not AddonApi.singleton("PlayFab").multiplayer.is_initialized():
		var init = await AddonApi.singleton("PlayFab").multiplayer.initialize_async()
		if not init.ok:
			push_warning("[Lobby] PlayFab.multiplayer init failed: %s" % init.message)
			return false
		print("[Lobby] PlayFab.multiplayer initialized lazily")

	if not _pf_multiplayer_signals_connected:
		AddonApi.singleton("PlayFab").multiplayer.invite_received.connect(_on_pf_invite_received)
		AddonApi.singleton("PlayFab").multiplayer.multiplayer_error.connect(_on_multiplayer_error)
		_pf_multiplayer_signals_connected = true

	return true

func _attach_lobby(lobby) -> void:
	_detach_lobby()
	_lobby = lobby
	_lobby.state_changed.connect(_on_lobby_state_changed)

func _detach_lobby() -> void:
	if _lobby == null:
		return
	if _lobby.state_changed.is_connected(_on_lobby_state_changed):
		_lobby.state_changed.disconnect(_on_lobby_state_changed)
	_lobby = null

# Tutorial 3 Step 3 — host a lobby. Returns true once we reach IN_LOBBY.
func host_lobby() -> bool:
	if not await _ensure_ready():
		return false
	if _state == State.IN_LOBBY:
		push_warning("[Lobby] host_lobby rejected — already in a lobby; leave first")
		return false
	if _state != State.READY:
		push_warning("[Lobby] host_lobby rejected — busy (state=%d)" % _state)
		return false

	_set_state(State.HOSTING)
	if not await _ensure_pf_multiplayer_initialized():
		_set_state(State.READY)
		return false

	var user = _auth.get("playfab_user")

	var config := AddonApi.instantiate("PlayFabLobbyConfig")
	config.max_players = 4
	config.access_policy = AddonApi.constant("PlayFabLobbyConfig", "ACCESS_POLICY_PUBLIC")
	config.owner_migration_policy = AddonApi.constant("PlayFabLobbyConfig", "OWNER_MIGRATION_AUTOMATIC")
	# Property scopes:
	#   search_properties: indexed for find_lobbies_async; reserved key
	#     namespace (string_keyN / number_keyN).
	#   lobby_properties: lobby-wide state visible only to members.
	#   member_properties: per-member, visible to members.
	config.search_properties = {
		"string_key1": "casual",
	}
	config.lobby_properties = {
		"map": "harbor",
		"mode": "deathmatch",
	}
	config.member_properties = {
		"loadout": "rifle",
	}

	var result = await AddonApi.singleton("PlayFab").multiplayer.create_lobby_async(user, config)
	if not result.ok:
		push_warning("[Lobby] create_lobby failed: %s (%s)" % [result.message, result.code])
		_set_state(State.READY)
		return false

	_attach_lobby(result.data)
	_set_state(State.IN_LOBBY)
	print("[Lobby] Lobby created: id=%s max=%d" % [_lobby.lobby_id, _lobby.max_member_count])
	print("[Lobby] connection string ready — copy to second client")
	print("[Lobby] %s" % _lobby.connection_string)
	lobby_joined.emit(_lobby)
	return true

# Alias of join_lobby that takes a string from a LineEdit.
func join_lobby_with_string(connection_string: String) -> bool:
	return await join_lobby(connection_string)

# Tutorial 3 Step 4 — join a lobby by connection string.
func join_lobby(connection_string: String) -> bool:
	if not await _ensure_ready():
		return false
	if _state == State.IN_LOBBY:
		push_warning("[Lobby] join_lobby rejected — already in a lobby; leave first")
		return false
	if _state != State.READY:
		push_warning("[Lobby] join_lobby rejected — busy (state=%d)" % _state)
		return false

	_set_state(State.JOINING)
	if not await _ensure_pf_multiplayer_initialized():
		_set_state(State.READY)
		return false

	var user = _auth.get("playfab_user")

	var config := AddonApi.instantiate("PlayFabLobbyJoinConfig")
	config.member_properties = {
		"loadout": "shotgun",
	}

	var result = await AddonApi.singleton("PlayFab").multiplayer.join_lobby_async(user, connection_string, config)
	if not result.ok:
		push_warning("[Lobby] join_lobby failed: %s (%s)" % [result.message, result.code])
		_set_state(State.READY)
		return false

	_attach_lobby(result.data)
	_set_state(State.IN_LOBBY)
	print("[Lobby] Joined lobby id=%s with %d member(s)" % [_lobby.lobby_id, _lobby.member_count])
	lobby_joined.emit(_lobby)
	return true

# Tutorial 3 Step 5 — discover public lobbies via the search index. Returns
# an Array of PlayFabLobbySearchResult (empty on failure / no matches).
func find_lobbies_async() -> Array:
	if not await _ensure_ready():
		return []
	if not await _ensure_pf_multiplayer_initialized():
		return []
	var user = _auth.get("playfab_user")
	var search := AddonApi.instantiate("PlayFabLobbySearchConfig")
	# Match the search_property the host advertises in host_lobby above.
	search.filter_string = "string_key1 eq 'casual'"
	var result = await AddonApi.singleton("PlayFab").multiplayer.find_lobbies_async(user, search)
	if not result.ok:
		push_warning("[Lobby] find_lobbies failed: %s (%s)" % [result.message, result.code])
		return []
	return result.data

# Tutorial 3 Step 6 — update member properties.
func push_loadout_change(loadout: String) -> void:
	if not is_in_lobby():
		return
	var pf = await _lobby.set_member_properties_async({ "loadout": loadout })
	if not pf.ok:
		push_warning("[Lobby] member props failed: %s" % pf.message)

# Tutorial 3 Step 6 — update lobby-wide properties (owner only).
func change_map(new_map: String) -> void:
	if not is_in_lobby():
		return
	var pf_user = _auth.get("playfab_user")
	if not _lobby.is_owner(pf_user):
		return
	var pf = await _lobby.set_properties_async({ "map": new_map })
	if not pf.ok:
		push_warning("[Lobby] lobby props failed: %s" % pf.message)

# Tutorial 3 Step 7 — leave.
func leave_lobby() -> bool:
	if _state != State.IN_LOBBY:
		if is_busy():
			push_warning("[Lobby] leave_lobby rejected — busy (state=%d)" % _state)
		return false

	_set_state(State.LEAVING)
	var pf = await _lobby.leave_async()
	if pf.ok:
		print("[Lobby] left lobby")
	else:
		push_warning("[Lobby] leave failed: %s" % pf.message)
	_detach_lobby()
	_set_state(State.READY)
	lobby_left.emit()
	return true

# --- Internal: signal handlers ---

func _on_pf_invite_received(invite) -> void:
	print("[Lobby] invite from %s: %s" % [invite.sender_user_id, invite.connection_string])
	invite_received.emit(invite.connection_string, invite.sender_user_id)

func _on_multiplayer_error(result) -> void:
	push_warning("[Lobby] multiplayer error: %s (%s)" % [result.message, result.code])

func _on_lobby_state_changed(change) -> void:
	var kind: int = change.kind
	if kind == AddonApi.constant("PlayFabLobby", "MEMBER_ADDED"):
		var m = change.member
		print("[Lobby] member added: %s (local=%s)" % [m.user_id, str(m.is_local)])
	elif kind == AddonApi.constant("PlayFabLobby", "MEMBER_REMOVED"):
		var m = change.member
		print("[Lobby] member removed: %s" % m.user_id)
	elif kind == AddonApi.constant("PlayFabLobby", "MEMBER_UPDATED"):
		var m = change.member
		print("[Lobby] member updated: %s" % m.user_id)
	elif kind == AddonApi.constant("PlayFabLobby", "PROPERTIES_UPDATED"):
		print("[Lobby] lobby properties: %s" % str(change.properties))
	elif kind == AddonApi.constant("PlayFabLobby", "OWNER_CHANGED"):
		print("[Lobby] owner changed: %s" % str(change.lobby.owner_entity_key))
	elif kind == AddonApi.constant("PlayFabLobby", "DISCONNECTED"):
		await _handle_pf_disconnected()

# DISCONNECTED is fired by PlayFab when the server kicks us off the lobby.
# Clean up local state, fire lobby_disconnected so the UI can react, then
# transition to READY so the user can host/join again.
func _handle_pf_disconnected() -> void:
	if _state != State.IN_LOBBY:
		push_warning("[Lobby] DISCONNECTED received in state=%d — ignoring" % _state)
		return
	if not is_inside_tree():
		return
	push_warning("[Lobby] disconnected from lobby")
	_detach_lobby()
	lobby_disconnected.emit()
	_set_state(State.READY)
