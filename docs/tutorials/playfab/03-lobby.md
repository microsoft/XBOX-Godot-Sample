# PlayFab Tutorial 3 — Lobby

## What you'll build

Create a pure PlayFab lobby flow: sign in with `PlayFabAuth`, lazily initialize PlayFab Multiplayer, host a public lobby with indexed search properties, join by connection string, search with `find_lobbies_async`, update lobby/member properties, surface received invites, and leave cleanly. There is no Xbox privilege gate, Social Manager, MPA, or presence in this track.

## Prerequisites

- Complete [PlayFab Tutorial 2 — Leaderboard](02-leaderboard.md).

- Enable PlayFab Multiplayer/Lobby for your title.

- Use two clients or devices to validate host/join and member updates.

- Configure any title-side Lobby feature switches documented in [PlayFab prerequisites](../../playfab/prerequisites.md).

- Tip: you can test host/join with two or more local instances by launching each with a distinct `--pf-user=<name>`; see [PlayFab Tutorial 1 — Running multiple instances](01-signin.md#running-multiple-instances).

## Relevant addon surfaces

- [`PlayFabMultiplayer`](../../../addons/godot_playfab/doc_classes/PlayFabMultiplayer.xml) — `initialize_async`, `create_lobby_async`, `join_lobby_async`, `find_lobbies_async`, invite/error signals.

- [`PlayFabLobby`](../../../addons/godot_playfab/doc_classes/PlayFabLobby.xml) — connection string, members, properties, `leave_async`, `state_changed`.

- [`PlayFabLobbyConfig`](../../../addons/godot_playfab/doc_classes/PlayFabLobbyConfig.xml), [`PlayFabLobbyJoinConfig`](../../../addons/godot_playfab/doc_classes/PlayFabLobbyJoinConfig.xml), [`PlayFabLobbySearchConfig`](../../../addons/godot_playfab/doc_classes/PlayFabLobbySearchConfig.xml).

## Steps

### Step 1 — Add the PlayFab-only `Lobby` autoload

```gdscript

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

```

### Step 2 — Add the UI scene

```gdscript

extends Control

const AddonApi = preload("res://shared/addon_api.gd")

## PlayFab Tutorial 3 reference scene — host / join / leave a PlayFab lobby.

##

## Demonstrates the Lobby autoload's host/join/leave flow with manual

## connection-string handoff. Members are shown live as MEMBER_ADDED /

## MEMBER_REMOVED events fire through autoload/lobby.gd.

##

## Source: docs/tutorials/playfab/03-lobby.md

@onready var _log: RichTextLabel = $Root/LogPanel/Log

@onready var _conn_edit: LineEdit = $Root/JoinRow/ConnEdit

@onready var _host_btn: Button = $Root/Buttons/HostBtn

@onready var _join_btn: Button = $Root/Buttons/JoinBtn

@onready var _leave_btn: Button = $Root/Buttons/LeaveBtn

@onready var _loadout_btn: Button = $Root/Buttons/LoadoutBtn

@onready var _map_btn: Button = $Root/Buttons/MapBtn

@onready var _back_btn: Button = $Root/Buttons/BackBtn

@onready var _members: ItemList = $Root/MembersPanel/Members

var _lobby_node: Node = null

func _ready() -> void:

	_back_btn.pressed.connect(_on_back_pressed)

	_host_btn.pressed.connect(func(): await _lobby_node.host_lobby())

	_join_btn.pressed.connect(_on_join_pressed)

	_leave_btn.pressed.connect(func(): await _lobby_node.leave_lobby())

	_loadout_btn.pressed.connect(func(): await _lobby_node.push_loadout_change("smg"))

	_map_btn.pressed.connect(func(): await _lobby_node.change_map("docks"))

	_lobby_node = get_node_or_null("/root/Lobby")

	if _lobby_node == null:

		_append("[color=red]Lobby autoload missing.[/color]")

		_set_buttons_enabled(false)

		return

	_lobby_node.lobby_joined.connect(_on_lobby_joined)

	_lobby_node.lobby_left.connect(_on_lobby_left)

	_lobby_node.lobby_disconnected.connect(_on_lobby_disconnected)

	# Drive in-progress feedback off the autoload's state machine so the

	# log shows "Hosting…" / "Joining…" / "Leaving…" the moment the user

	# clicks (or an invite-accept cascades through). Without this the UI

	# looks frozen between the click and the lobby_joined fire.

	_lobby_node.state_changed.connect(_on_lobby_state_changed)

	# Wait for sign-in, then enable buttons. PlayFab Multiplayer init

	# happens lazily inside Lobby.host_lobby() / join_lobby() so we

	# don't pay the cost until the user actually clicks Host or Join.

	var auth: Node = get_node_or_null("/root/PlayFabAuth")

	if auth == null or not await auth.call("sign_in"):

		_append("[color=red]Sign-in failed.[/color]")

		_set_buttons_enabled(false)

		return

	_append("Signed in. Click Host or Join to bring up multiplayer.")

	_set_buttons_enabled(true)

	_leave_btn.disabled = true

func _on_lobby_joined(lobby) -> void:

	_append("[color=green]Joined lobby %s[/color]" % lobby.lobby_id)

	# Surface the connection string in the same LineEdit the client

	# pastes into, so the host can select-and-copy it (Ctrl+C) and hand

	# it to a second device. select_all() puts the field in a state

	# where Ctrl+C copies the full string without an extra click.

	_conn_edit.text = lobby.connection_string

	_conn_edit.caret_column = lobby.connection_string.length()

	_conn_edit.select_all()

	_conn_edit.grab_focus()

	_refresh_members(lobby)

	_leave_btn.disabled = false

	lobby.state_changed.connect(func(_change): _refresh_members(lobby))

func _on_lobby_left() -> void:

	_append("Left lobby.")

	_members.clear()

	_leave_btn.disabled = true

func _on_lobby_disconnected() -> void:

	_append("[color=orange]Disconnected from lobby (kicked or network error).[/color]")

	_members.clear()

	_leave_btn.disabled = true

func _on_lobby_state_changed(state) -> void:

	# State enum lives on the Lobby autoload; resolve at runtime since the

	# parse gate doesn't see autoload types. Only the busy transitions

	# need feedback — IN_LOBBY / READY are already announced by

	# lobby_joined / lobby_left below.

	var s = _lobby_node.State

	match state:

		s.HOSTING:

			_append("[color=yellow]Hosting lobby…[/color]")

		s.JOINING:

			_append("[color=yellow]Joining lobby…[/color]")

		s.LEAVING:

			_append("[color=yellow]Leaving lobby…[/color]")

	# Gate buttons against the in-flight op so a second click can't race

	# the cascade. host_lobby / join_lobby already refuse non-READY

	# entries but the user shouldn't see a clickable Host button while

	# we're already hosting.

	var busy: bool = state == s.HOSTING or state == s.JOINING or state == s.LEAVING

	var in_lobby: bool = state == s.IN_LOBBY

	_host_btn.disabled = busy or in_lobby

	_join_btn.disabled = busy or in_lobby

	_leave_btn.disabled = not in_lobby

	_loadout_btn.disabled = not in_lobby

	_map_btn.disabled = not in_lobby

func _refresh_members(lobby) -> void:

	_members.clear()

	for member in lobby.members:

		var marker := " (local)" if member.is_local else ""

		_members.add_item("%s%s" % [member.user_id, marker])

func _on_join_pressed() -> void:

	var conn := _conn_edit.text.strip_edges()

	if conn.is_empty():

		_append("[color=orange]Paste a connection string into the field first.[/color]")

		return

	await _lobby_node.join_lobby(conn)

func _set_buttons_enabled(enabled: bool) -> void:

	_host_btn.disabled = not enabled

	_join_btn.disabled = not enabled

	_loadout_btn.disabled = not enabled

	_map_btn.disabled = not enabled

func _append(line: String) -> void:

	_log.append_text(line + "\n")

	print(line)

func _on_back_pressed() -> void:

	get_tree().change_scene_to_file("res://shared/tutorial_picker.tscn")

```

### Step 3 — Search and invites are PlayFab-native here

The autoload's `find_lobbies_async()` searches for `string_key1 eq 'casual'`, matching the host config. The `invite_received` signal forwards `connection_string` and `sender_user_id` from PlayFab Multiplayer; this track does not send Xbox Multiplayer Activity invites.

## Verify

Run `p03_lobby`, click **Host**, copy the connection string, and paste it into a second client. Members should update as clients join/leave. Loadout and map buttons should print property updates. Search should find public lobbies that match the sample's `casual` search property.

## Common failures

| Output | Diagnosis | Fix |

|---|---|---|

| `PlayFab.multiplayer init failed` | Multiplayer/Lobby is disabled or title id is wrong. | Enable Lobby and verify title id. |

| `host_lobby rejected — already in a lobby` | Single-slot sample already owns a lobby. | Leave before hosting/joining another. |

| Search returns no rows | No public lobby matches `string_key1 eq 'casual'`. | Host with the sample config or update the filter. |

| Received invite cannot join | Invite connection string is stale or from another title. | Ask sender to create a fresh lobby in the same title. |

## Reference implementation

- Scene: [`sample/tutorial_playfab/p03_lobby.tscn`](../../../sample/tutorial_playfab/p03_lobby.tscn)

- Scene script: [`sample/tutorial_playfab/p03_lobby.gd`](../../../sample/tutorial_playfab/p03_lobby.gd)

- Autoload: [`sample/tutorial_playfab/autoload/lobby.gd`](../../../sample/tutorial_playfab/autoload/lobby.gd)

## Next

Continue to [PlayFab Tutorial 4 — Party](04-party.md).
