extends Node

## Tutorials 5 + 6 — Lobby + Multiplayer Activity + Presence + privilege gates.
##
## Owns the live PlayFabLobby reference, mirrors lobby state into the Xbox
## shell via GDK.multiplayer_activity, advertises rich presence to friends,
## and gates host/join on XUserPrivilege::Multiplayer + the per-target
## "play_multiplayer" permission. Emits high-level signals so the Party
## autoload (T7) and tutorial scenes can react without depending on
## PlayFab Multiplayer's firehose directly.
##
## Item 15 — state machine. The autoload exposes a tracked `State` and a
## single `state_changed` firehose so panels can drive button state
## without holding their own bookkeeping. `lobby_joined`,
## `lobby_left`, and `lobby_disconnected` are kept on top as
## convenience signals because the PlayFabLobby payload is more useful
## than a guarded getter fetch.
##
## Item 9 — single-slot design (intentional for the sample). This
## autoload owns exactly one PlayFabLobby at a time (`_lobby`). The
## host/join APIs reject re-entrant calls and the panels assume a
## single live lobby. That keeps the tutorial scenes thin: panels
## render one lobby's state without juggling lobby IDs.
##
## A shipping game that needs to be in more than one PlayFab lobby
## simultaneously (e.g. a clan/social lobby alongside a match lobby)
## should refactor away from the singleton: turn `host_lobby` /
## `join_lobby` into methods that return the new PlayFabLobby
## (instead of stashing it on `_lobby`), drop the `_state`
## bookkeeping or move it into a per-lobby controller, and have the
## caller hold the PlayFabLobby reference plus connect to it
## directly. The PlayFab addon supports multiple live lobbies per
## process — the single-slot choice lives entirely in this sample
## autoload.
##
## Source:
##   docs/tutorials/05-multiplayer-lobby.md
##   docs/tutorials/06-multiplayer-activity.md

# XGameRuntime XUserPrivilege values (from <XUser.h>).
const XUSER_PRIVILEGE_MULTIPLAYER := 254

# MPA join restriction tokens.
const MPA_JOIN_RESTRICTION_FOLLOWED := "followed"
const MPA_JOIN_RESTRICTION_PUBLIC := "public"
const MPA_JOIN_RESTRICTION_INVITE_ONLY := "invite_only"

# Rich-presence ID registered against the title in Partner Center.
# Substitute with the ID you registered (or leave empty to skip the
# presence write).
const PRESENCE_IN_LOBBY := "in_lobby"

enum State {
	UNINITIALIZED,    ## Autoload _ready has not finished sign-in.
	READY,            ## Signed in, no lobby; host/join allowed.
	HOSTING,          ## host_lobby() in flight (create_lobby_async).
	JOINING,          ## join_lobby() in flight (join_lobby_async).
	IN_LOBBY,         ## Active PlayFabLobby; leave allowed.
	LEAVING,          ## leave_lobby() in flight (leave_async).
}

signal state_changed(state: State)
signal lobby_joined(lobby: PlayFabLobby)
signal lobby_left
signal lobby_disconnected  ## PlayFab fired DISCONNECTED firehose; involuntary.

## Item 3 / B3 — invite-accept confirmation. Fires when an MPA invite
## arrives while the user is already IN_LOBBY, so a UI consumer can
## prompt before destroying the current session. invite_id is a
## monotonically-increasing token so a stale dialog (a second invite
## landed and overwrote the first) can be detected on confirm.
signal invite_pending_confirmation(invite_id: int, connection_string: String)
## Item 3 / B3 — pending invite was withdrawn (user manually left, got
## disconnected, accepted/rejected, or shutdown). Consumers should
## close any visible confirmation dialog.
signal invite_pending_cleared(invite_id: int)

var _state: State = State.UNINITIALIZED
var _lobby: PlayFabLobby = null
var _auth: Node = null
var _gdk_signals_connected: bool = false
var _pf_multiplayer_signals_connected: bool = false
var _watched_xuids: PackedStringArray = PackedStringArray()
# Item 5 / B2 — social-graph state. _friends_group is loaded lazily on
# first get_friends_async() call and torn down in _exit_tree so the
# Social Manager doesn't keep tracking after the autoload is gone.
var _social_graph_started: bool = false
var _friends_group: GDKSocialGroup = null
# Item 3 / B3 — pending-invite slot. Only populated while IN_LOBBY and
# waiting for a UI confirmation. _pending_invite_id is a token bound
# to the emitted signal so confirm/reject calls from a stale dialog
# (overwritten by a newer invite) are rejected.
var _pending_invite_id: int = 0
var _pending_invite_cs: String = ""
var _pending_invite_confirming: bool = false

## Guarded accessor — only returns the live PlayFabLobby while we are
## actually in IN_LOBBY. During LEAVING / on disconnect callers should
## listen for lobby_left / lobby_disconnected to do their cleanup, not
## poke this getter for a stale ref.
var current_lobby: PlayFabLobby:
	get:
		return _lobby if _state == State.IN_LOBBY else null

func _ready() -> void:
	_auth = get_node_or_null("/root/Auth")
	if _auth == null:
		push_error("[Lobby] Auth autoload missing")
		return
	# _ensure_ready awaits sign-in, wires the GDK-side signals once, and
	# transitions UNINITIALIZED -> READY. If sign-in fails we stay in
	# UNINITIALIZED; subsequent host/join attempts will re-await sign-in
	# and re-attempt the transition (Auth.sign_in is idempotent).
	await _ensure_ready()

func _exit_tree() -> void:
	# Item 5 / B2 — release the social-graph group so the Social Manager
	# stops issuing background work after the autoload is gone. Stopping
	# the graph itself is a no-op if start_social_graph was never called.
	if Engine.has_singleton("GDK"):
		if _friends_group != null:
			GDK.social.destroy_social_group(_friends_group)
			_friends_group = null
		if _social_graph_started:
			var user: GDKUser = _auth.get("xbox_user") if _auth != null else null
			if user != null:
				GDK.social.stop_social_graph(user)
			_social_graph_started = false

func get_state() -> State:
	return _state

func is_ready() -> bool:
	return _state == State.READY

func is_in_lobby() -> bool:
	return _state == State.IN_LOBBY

func is_busy() -> bool:
	return _state == State.HOSTING or _state == State.JOINING or _state == State.LEAVING

# Kept for back-compat with sample / test code that called the getter
# directly. New consumers should use the `current_lobby` property.
func get_current_lobby() -> PlayFabLobby:
	return current_lobby

func _set_state(next: State) -> void:
	if _state == next:
		return
	# Item 3 / B3 — any transition out of IN_LOBBY invalidates a pending
	# invite: the user manually left, was disconnected, or we're driving
	# the confirm path itself. Clear the slot so a stale dialog can't be
	# Accept-tapped 10 minutes later and trigger a leave+join the user
	# no longer intends. The `_pending_invite_confirming` path already
	# snapshot-and-cleared its own state, so this is a no-op in that case.
	var was_in_lobby := _state == State.IN_LOBBY
	_state = next
	state_changed.emit(_state)
	if was_in_lobby and _state != State.IN_LOBBY:
		_clear_pending_invite()

## Awaits sign-in (idempotent across concurrent callers, since
## Auth.sign_in itself coalesces) and connects the GDK-side signal
## handlers exactly once. Returns true when we are in a usable post-
## sign-in state, false on terminal sign-in failure.
func _ensure_ready() -> bool:
	if _state == State.READY or _state == State.IN_LOBBY:
		return true
	if is_busy():
		return false
	# state == UNINITIALIZED.
	if not await _auth.call("sign_in"):
		push_warning("[Lobby] sign-in failed (%s) — staying UNINITIALIZED" % _auth.call("get_last_error_stage"))
		return false
	if not Engine.has_singleton("GDK"):
		push_error("[Lobby] GDK extension not loaded")
		return false

	# Another concurrent _ensure_ready may have raced us through the
	# sign_in await. Re-check the state before claiming the wiring slot.
	if _state != State.UNINITIALIZED:
		return _state == State.READY or _state == State.IN_LOBBY

	if not _gdk_signals_connected:
		# GDK-side signal handlers connect eagerly here. These are cheap,
		# don't require PlayFab to be initialized, and
		# pending_invite_received needs to be wired before the engine can
		# hand us a deferred launch-with-invite.
		GDK.multiplayer_activity.pending_invite_received.connect(_on_pending_invite_received)
		GDK.multiplayer_activity.invite_accepted.connect(_on_invite_accepted)
		GDK.multiplayer_activity.activities_updated.connect(_on_activities_updated)
		GDK.presence.device_presence_changed.connect(_on_device_presence_changed)
		GDK.presence.title_presence_changed.connect(_on_title_presence_changed)
		GDK.presence.presence_changed.connect(_on_presence_changed)
		_gdk_signals_connected = true
		print("[Lobby] GDK MPA + presence handlers connected. PlayFab Multiplayer init is lazy.")

	_set_state(State.READY)
	return true

# Item 10 — lazy initialization. PlayFab.multiplayer is large and most
# games never touch it (single-player titles, story-mode-only sessions).
# Bring it up on first lobby action instead of in _ready, and connect
# the PlayFab Multiplayer signals exactly once on success.
func _ensure_pf_multiplayer_initialized() -> bool:
	if not Engine.has_singleton("PlayFab"):
		push_error("[Lobby] PlayFab extension not loaded")
		return false

	if not PlayFab.multiplayer.is_initialized():
		var init: PlayFabResult = await PlayFab.multiplayer.initialize_async()
		if not init.ok:
			push_warning("[Lobby] PlayFab.multiplayer init failed: %s" % init.message)
			return false
		print("[Lobby] PlayFab.multiplayer initialized lazily")

	if not _pf_multiplayer_signals_connected:
		PlayFab.multiplayer.state_changed.connect(_on_state_changed)
		PlayFab.multiplayer.invite_received.connect(_on_pf_invite_received)
		PlayFab.multiplayer.multiplayer_error.connect(_on_multiplayer_error)
		_pf_multiplayer_signals_connected = true

	return true

## Centralized lobby-firehose lifetime. Always go through these helpers
## instead of touching `_lobby` directly; a stale lobby with a still-
## connected signal will keep delivering events to us.
func _attach_lobby(lobby: PlayFabLobby) -> void:
	_detach_lobby()
	_lobby = lobby
	_lobby.state_changed.connect(_on_lobby_state_changed)

func _detach_lobby() -> void:
	if _lobby == null:
		return
	if _lobby.state_changed.is_connected(_on_lobby_state_changed):
		_lobby.state_changed.disconnect(_on_lobby_state_changed)
	_lobby = null

# Item 5 / B1 — XUID-by-peer mapping support. The lobby autoload writes
# the local user's Xbox XUID into member_properties on host/join so the
# Party autoload can map a PlayFabPartyPeer peer_id (via the peer's
# PlayFab entity key) back to an XUID for per-peer permission checks.
# Returns "" for non-Xbox sign-ins (the per-peer check is skipped in
# that case — the privilege gate already filtered out non-comms users).
func _local_xuid() -> String:
	var user: GDKUser = _auth.get("xbox_user") if _auth != null else null
	if user == null:
		return ""
	return user.xuid

# Tutorial 5 Step 2 — Multiplayer privilege gate.
func can_use_multiplayer() -> bool:
	var user: GDKUser = _auth.get("xbox_user") if _auth != null else null
	if user == null:
		return false

	var pf: GDKResult = await GDK.users.check_privilege_async(
			user, XUSER_PRIVILEGE_MULTIPLAYER)
	if pf.ok and bool(pf.data.get("has_privilege", false)):
		return true

	print("[Lobby] multiplayer denied (%s) — resolving with UI" % pf.data.get("deny_reason", ""))
	var resolved: GDKResult = await GDK.users.resolve_privilege_with_ui_async(
			user, XUSER_PRIVILEGE_MULTIPLAYER)
	if not resolved.ok:
		push_warning("[Lobby] resolve_privilege_with_ui failed: %s" % resolved.message)
		return false
	return bool(resolved.data.get("has_privilege", false))

# Tutorial 5 Step 2 (filter helper) / Tutorial 6 Step 5 (cert).
func filter_invitable(xuids: PackedStringArray) -> PackedStringArray:
	var user: GDKUser = _auth.get("xbox_user") if _auth != null else null
	if user == null or xuids.is_empty():
		return PackedStringArray()
	var pf: GDKResult = await GDK.privacy.batch_check_permission_async(
			user, "play_multiplayer", xuids)
	if not pf.ok:
		push_warning("[Lobby] permission batch failed: %s" % pf.message)
		return PackedStringArray()
	var allowed := PackedStringArray()
	for entry: Dictionary in pf.data:
		if bool(entry.get("allowed", false)):
			allowed.append(String(entry.get("target_xuid", "")))
	return allowed

# Tutorial 6 Step 5 (helper) — friends list via the Xbox Social Manager.
#
# Returns an Array of GDKSocialUser for the local user's friends list,
# or [] on failure. Lazily starts the social graph and creates the
# friends group on first call; subsequent calls reuse the same group.
# _exit_tree destroys the group so the Social Manager doesn't keep
# servicing the title after the autoload is gone.
#
# The Social Manager fires `social_graph_changed` when friends join,
# leave, or change presence — callers wanting live updates should
# connect to that signal and re-call this method.
func get_friends_async() -> Array:
	var user: GDKUser = _auth.get("xbox_user") if _auth != null else null
	if user == null or not Engine.has_singleton("GDK"):
		return []
	if not _social_graph_started:
		var sg: GDKResult = GDK.social.start_social_graph(user)
		if not sg.ok:
			push_warning("[Lobby] start_social_graph failed: %s" % sg.message)
			return []
		_social_graph_started = true
	if _friends_group == null:
		var f: GDKResult = await GDK.social.get_friends_async(user)
		if not f.ok:
			push_warning("[Lobby] get_friends failed: %s" % f.message)
			return []
		_friends_group = f.data
	var users: GDKResult = GDK.social.get_group_users(_friends_group)
	if not users.ok:
		push_warning("[Lobby] get_group_users failed: %s" % users.message)
		return []
	return users.data

# Tutorial 5 Step 3 — host a lobby. Returns true once we reach IN_LOBBY,
# false if we were rejected (busy / already in a lobby) or if the
# create_lobby_async call failed.
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
	if not await can_use_multiplayer():
		push_warning("[Lobby] host blocked — multiplayer privilege denied")
		_set_state(State.READY)
		return false

	var user: PlayFabUser = _auth.get("playfab_user")

	var config := PlayFabLobbyConfig.new()
	config.max_players = 4
	config.access_policy = PlayFabLobbyConfig.ACCESS_POLICY_PUBLIC
	config.owner_migration_policy = PlayFabLobbyConfig.OWNER_MIGRATION_AUTOMATIC
	# Item 12 / C1 — property scopes:
	#   search_properties: indexed for find_lobbies_async; reserved
	#     key namespace (string_keyN / number_keyN). Visible to anyone
	#     who can discover the lobby for this access_policy.
	#   lobby_properties: lobby-wide state visible only to members.
	#     Free of the search-key naming constraint.
	#   member_properties: per-member, visible to members. Only the
	#     local member can edit their own entry.
	# See docs/tutorials/05-multiplayer-lobby.md "Property scopes" for
	# the full rules of thumb.
	config.search_properties = {
		"string_key1": "casual",
	}
	config.lobby_properties = {
		"map": "harbor",
		"mode": "deathmatch",
	}
	config.member_properties = {
		"loadout": "rifle",
		"xuid": _local_xuid(),
	}

	var result: PlayFabResult = await PlayFab.multiplayer.create_lobby_async(user, config)
	if not result.ok:
		push_warning("[Lobby] create_lobby failed: %s (%s)" % [result.message, result.code])
		_set_state(State.READY)
		return false

	_attach_lobby(result.data)
	_set_state(State.IN_LOBBY)
	print("[Lobby] Lobby created: id=%s max=%d" % [_lobby.lobby_id, _lobby.max_member_count])
	print("[Lobby] connection string ready — copy to second client")
	print("[Lobby] %s" % _lobby.connection_string)

	# MPA + presence publication failures are warnings only — they don't
	# fail the host_lobby coroutine because the lobby itself is real and
	# usable. Consumers can listen for state_changed(IN_LOBBY) and treat
	# host as successful.
	await _publish_activity()
	await _publish_lobby_presence()
	lobby_joined.emit(_lobby)
	return true

# Tutorial 8 panel helper — alias of join_lobby that takes a string from a
# LineEdit. The cumulative-track tutorial wires the panel against this name
# (Tutorial 5 uses a hard-coded JOIN_STRING constant).
func join_lobby_with_string(connection_string: String) -> bool:
	return await join_lobby(connection_string)

# Tutorial 5 Step 4 — join a lobby by connection string. Returns true once
# we reach IN_LOBBY, false otherwise.
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
	if not await can_use_multiplayer():
		push_warning("[Lobby] join blocked — multiplayer privilege denied")
		_set_state(State.READY)
		return false

	var user: PlayFabUser = _auth.get("playfab_user")

	var config := PlayFabLobbyJoinConfig.new()
	config.member_properties = {
		"loadout": "shotgun",
		"xuid": _local_xuid(),
	}

	var result: PlayFabResult = await PlayFab.multiplayer.join_lobby_async(user, connection_string, config)
	if not result.ok:
		push_warning("[Lobby] join_lobby failed: %s (%s)" % [result.message, result.code])
		_set_state(State.READY)
		return false

	_attach_lobby(result.data)
	_set_state(State.IN_LOBBY)
	print("[Lobby] Joined lobby id=%s with %d member(s)" % [_lobby.lobby_id, _lobby.member_count])

	await _publish_activity()
	await _publish_lobby_presence()
	lobby_joined.emit(_lobby)
	return true

# Tutorial 5 Step 6 — update member properties.
func push_loadout_change(loadout: String) -> void:
	if not is_in_lobby():
		return
	var pf: PlayFabResult = await _lobby.set_member_properties_async({ "loadout": loadout })
	if not pf.ok:
		push_warning("[Lobby] member props failed: %s" % pf.message)

# Tutorial 5 Step 6 — update lobby-wide properties (owner only).
func change_map(new_map: String) -> void:
	if not is_in_lobby():
		return
	var pf_user: PlayFabUser = _auth.get("playfab_user")
	if not _lobby.is_owner(pf_user):
		return
	var pf: PlayFabResult = await _lobby.set_properties_async({ "map": new_map })
	if not pf.ok:
		push_warning("[Lobby] lobby props failed: %s" % pf.message)

# Tutorial 5 Step 7 + Tutorial 6 Step 3 — leave with activity + presence cleanup.
# Returns true if we transitioned IN_LOBBY -> READY (regardless of whether
# PlayFab's leave_async reported ok), false if we were rejected.
func leave_lobby() -> bool:
	if _state != State.IN_LOBBY:
		# Voluntary leave is a no-op from READY / DISCONNECTED-aftermath /
		# UNINITIALIZED. Don't try to push state during HOSTING / JOINING /
		# LEAVING — let the in-flight operation finish first.
		if is_busy():
			push_warning("[Lobby] leave_lobby rejected — busy (state=%d)" % _state)
		return false

	_set_state(State.LEAVING)
	await _clear_activity()
	var pf: PlayFabResult = await _lobby.leave_async()
	if pf.ok:
		print("[Lobby] left lobby")
	else:
		push_warning("[Lobby] leave failed: %s" % pf.message)
	await _clear_lobby_presence()
	_detach_lobby()
	_set_state(State.READY)
	lobby_left.emit()
	return true

# Tutorial 6 Step 5 — targeted invite. Returns true when send_invites_async
# succeeded for the requested xuid, false when the invite was suppressed
# (not in a lobby, permission-filtered out, or send_invites_async failed).
func invite_friend(xuid: String) -> bool:
	if not is_in_lobby():
		push_warning("[MPA] Cannot invite — not in a lobby")
		return false

	var xuids: PackedStringArray = [xuid]
	var allowed: PackedStringArray = await filter_invitable(xuids)
	if allowed.is_empty():
		push_warning("[MPA] Invite blocked by play_multiplayer permission for %s" % xuid)
		return false

	var result: GDKResult = await GDK.multiplayer_activity.send_invites_async(
		_auth.get("xbox_user"),
		allowed,
		false,
		_lobby.connection_string)
	if result.ok:
		print("[MPA] Sent invite to %s" % allowed[0])
		return true
	push_warning("[MPA] send_invites failed: %s (%s)" % [result.message, result.code])
	return false

# Tutorial 6 Step 5 — system invite picker.
func open_invite_picker() -> void:
	if not is_in_lobby():
		push_warning("[MPA] Cannot open picker — not in a lobby")
		return

	var result: GDKResult = await GDK.multiplayer_activity.show_invite_ui_async(_auth.get("xbox_user"))
	if not result.ok:
		push_warning("[MPA] show_invite_ui failed: %s" % result.message)

# Tutorial 6 Steps 6 + 7 — friend activity + presence tracking.
func track_friend_activities(xuids: PackedStringArray) -> void:
	_watched_xuids = xuids
	var user: GDKUser = _auth.get("xbox_user")

	var activities: GDKResult = await GDK.multiplayer_activity.get_activities_async(user, xuids)
	if not activities.ok:
		push_warning("[MPA] get_activities failed: %s" % activities.message)

	GDK.presence.track_presence(user, xuids)

	var presence: GDKResult = await GDK.presence.get_presence_async(xuids)
	if not presence.ok:
		push_warning("[Pres] get_presence failed: %s" % presence.message)

	for xuid in xuids:
		_print_activity(xuid)
		_print_presence(xuid)

func stop_tracking_friends() -> void:
	if _watched_xuids.is_empty():
		return
	GDK.presence.stop_tracking_presence(_auth.get("xbox_user"), _watched_xuids)
	_watched_xuids = PackedStringArray()

# --- Internal: MPA + presence helpers ---

func _publish_activity(allow_cross_platform_join: bool = false) -> void:
	if _lobby == null:
		return
	var user: GDKUser = _auth.get("xbox_user")
	if user == null:
		return

	var current_players: int = _lobby.member_count
	var max_players: int = _lobby.max_member_count
	var connection_string: String = _lobby.connection_string

	var result: GDKResult = await GDK.multiplayer_activity.set_activity_async(
		user,
		connection_string,
		MPA_JOIN_RESTRICTION_FOLLOWED,
		max_players,
		current_players,
		"",
		allow_cross_platform_join)
	if not result.ok:
		push_warning("[MPA] set_activity failed: %s (%s)" % [result.message, result.code])
		return

	print("[MPA] Activity advertised: max=%d current=%d cross_platform=%s" % [
		max_players, current_players, str(allow_cross_platform_join)])

func _clear_activity() -> void:
	var user: GDKUser = _auth.get("xbox_user")
	if user == null:
		return
	var result: GDKResult = await GDK.multiplayer_activity.delete_activity_async(user)
	if result.ok:
		print("[MPA] Activity cleared")
	else:
		push_warning("[MPA] delete_activity failed: %s" % result.message)

func _publish_lobby_presence() -> void:
	if PRESENCE_IN_LOBBY.is_empty():
		return
	var user: GDKUser = _auth.get("xbox_user")
	if user == null:
		return
	var pf: GDKResult = await GDK.presence.set_presence_async(user, PRESENCE_IN_LOBBY)
	if not pf.ok:
		push_warning("[Lobby] presence write failed: %s" % pf.message)

func _clear_lobby_presence() -> void:
	var user: GDKUser = _auth.get("xbox_user")
	if user == null:
		return
	var pf: GDKResult = await GDK.presence.clear_presence_async(user)
	if not pf.ok:
		push_warning("[Lobby] presence clear failed: %s" % pf.message)

func _print_activity(xuid: String) -> void:
	var info: GDKMultiplayerActivityInfo = GDK.multiplayer_activity.get_cached_activity(xuid)
	if info == null:
		print("[MPA] Friend %s is offline / not in a session" % xuid)
		return
	var conn: String = info.get_connection_string()
	if conn.is_empty():
		print("[MPA] Friend %s cleared their session" % xuid)
		return
	print("[MPA] Friend %s is in session: %s" % [xuid, conn])

func _print_presence(xuid: String) -> void:
	var record: GDKPresenceRecord = GDK.presence.get_cached_presence(xuid)
	if record == null:
		print("[Pres] %s: (unknown)" % xuid)
		return
	var title_records: Array = record.get_title_records()
	var rich: String = ""
	if not title_records.is_empty():
		var first: Dictionary = title_records[0]
		rich = first.get("rich_presence_string", "")
	print("[Pres] %s: state=%s rich=%s" % [
		xuid, record.get_user_state_name(), rich])

# --- Internal: signal handlers ---

func _on_state_changed(_change: PlayFabMultiplayerStateChange) -> void:
	pass # Per-lobby changes route to _on_lobby_state_changed below.

func _on_pf_invite_received(invite: PlayFabLobbyInvite) -> void:
	print("[Lobby] invite from %s: %s" % [invite.sender_entity_key, invite.connection_string])

func _on_multiplayer_error(result: PlayFabResult) -> void:
	push_warning("[Lobby] multiplayer error: %s (%s)" % [result.message, result.code])

func _on_lobby_state_changed(change: PlayFabLobbyStateChange) -> void:
	match change.kind:
		PlayFabLobby.MEMBER_ADDED:
			var m: PlayFabLobbyMember = change.member
			print("[Lobby] member added: %s (local=%s)" % [m.user_id, str(m.is_local)])
			await _publish_activity()
		PlayFabLobby.MEMBER_REMOVED:
			var m: PlayFabLobbyMember = change.member
			print("[Lobby] member removed: %s" % m.user_id)
			await _publish_activity()
		PlayFabLobby.MEMBER_UPDATED:
			var m: PlayFabLobbyMember = change.member
			print("[Lobby] member updated: %s" % m.user_id)
		PlayFabLobby.PROPERTIES_UPDATED:
			print("[Lobby] lobby properties: %s" % str(change.properties))
		PlayFabLobby.OWNER_CHANGED:
			print("[Lobby] owner changed: %s" % str(change.lobby.owner_entity_key))
		PlayFabLobby.DISCONNECTED:
			await _handle_pf_disconnected()

# DISCONNECTED is fired by PlayFab when the server kicks us off the lobby
# (network error, lobby destroyed, evicted). Treat as a transient event:
# clean up local state, fire lobby_disconnected so the UI can show a
# message, then transition to READY so the user can host/join again.
# LEAVING -> ignored: we're already voluntarily leaving and the leave
# coroutine will land us in READY on its own.
func _handle_pf_disconnected() -> void:
	if _state != State.IN_LOBBY:
		push_warning("[Lobby] DISCONNECTED received in state=%d — ignoring" % _state)
		return
	if not is_inside_tree():
		# Autoload teardown ordering — PlayFab bootstrap may emit
		# DISCONNECTED from _exit_tree after we've already been removed
		# from the SceneTree. Skip the async cleanup; the engine is
		# tearing down anyway.
		return
	push_warning("[Lobby] disconnected from lobby")
	await _clear_activity()
	await _clear_lobby_presence()
	_detach_lobby()
	lobby_disconnected.emit()
	_set_state(State.READY)

func _on_pending_invite_received(invite: Dictionary) -> void:
	print("[MPA] Pending invite (not yet accepted): %s" % invite.get("raw_uri", ""))

func _on_invite_accepted(invite: Dictionary) -> void:
	print("[MPA] Invite accepted from Game Bar: scheme=%s action=%s" % [
		invite.get("scheme", ""), invite.get("action", "")])

	var connection_string: String = invite.get("connectionstring", "")
	if connection_string.is_empty():
		push_warning("[MPA] Invite did not carry a connection string: %s" % invite.get("raw_uri", ""))
		return

	# Don't fight an in-flight host/join/leave. The user accepted an
	# invite mid-operation — let the operation finish, then they can
	# manually re-trigger the invite. (A production title might queue
	# the invite for after the in-flight operation; the sample stays
	# simple and just warns.)
	if is_busy():
		push_warning("[MPA] Invite accepted while busy (state=%d) — ignoring" % _state)
		return

	# Item 3 / B3 — if we're not in a lobby right now there's nothing
	# to leave; join directly without prompting. The prompt only fires
	# when accepting the invite would destroy the user's current
	# session.
	if _state != State.IN_LOBBY:
		await join_lobby(connection_string)
		return

	# IN_LOBBY — stash the invite and ask UI to confirm. Don't await
	# anything here: the user might never tap a button (closed the
	# dialog, switched scenes), and we don't want to hold this
	# coroutine alive for that whole window.
	if _pending_invite_confirming:
		# A previous confirm() is already mid-flight (leave_lobby is
		# awaiting). Ignore the new invite — when the in-flight
		# leave+join settles, the user can re-trigger from the Game Bar.
		push_warning("[MPA] Invite arrived while confirming another — dropping new invite")
		return
	_pending_invite_id += 1
	_pending_invite_cs = connection_string
	invite_pending_confirmation.emit(_pending_invite_id, connection_string)
	print("[MPA] Invite pending confirmation (id=%d) — waiting for UI accept/reject" % _pending_invite_id)

# Item 3 / B3 — UI tapped "Leave current lobby and join". invite_id is
# the token from `invite_pending_confirmation`; mismatched tokens (the
# dialog was for a now-overwritten invite) return false without taking
# any action. Returns true if we successfully joined the invited lobby.
func confirm_pending_invite(invite_id: int) -> bool:
	if _pending_invite_cs.is_empty() or invite_id != _pending_invite_id:
		push_warning("[MPA] confirm_pending_invite(id=%d) rejected — stale or empty" % invite_id)
		return false
	if _pending_invite_confirming:
		# Defensive: a confirm is already in flight. The dialog should
		# not have been double-tappable, but in case it was, no-op.
		return false
	_pending_invite_confirming = true
	# Snapshot+clear: from this point on a new invite stashed via
	# _on_invite_accepted is its own pending slot, not the one we're
	# acting on. _set_state(LEAVING) below would otherwise call
	# _clear_pending_invite and emit invite_pending_cleared with our
	# id, racing the consumer's local "in-flight confirm" state.
	var cs := _pending_invite_cs
	var id := _pending_invite_id
	_pending_invite_cs = ""

	# Re-check state — the user may have left manually while the
	# dialog was up. If we're already not in a lobby, skip the leave
	# step and go straight to join.
	if _state == State.IN_LOBBY:
		if not await leave_lobby():
			_pending_invite_confirming = false
			invite_pending_cleared.emit(id)
			return false

	var ok := await join_lobby(cs)
	_pending_invite_confirming = false
	invite_pending_cleared.emit(id)
	return ok

# Item 3 / B3 — UI tapped "Stay in current lobby". Mismatched tokens
# (the dialog is for a now-overwritten invite) no-op.
func reject_pending_invite(invite_id: int) -> void:
	if invite_id != _pending_invite_id:
		return
	_clear_pending_invite()

func _clear_pending_invite() -> void:
	if _pending_invite_cs.is_empty():
		return
	var id := _pending_invite_id
	_pending_invite_cs = ""
	invite_pending_cleared.emit(id)

func _on_activities_updated(xuids: PackedStringArray) -> void:
	print("[MPA] Activity updated for friends: %s" % str(xuids))
	for xuid in xuids:
		_print_activity(xuid)

func _on_device_presence_changed(xuid: String) -> void:
	if xuid in _watched_xuids:
		await GDK.presence.get_presence_async([xuid])

func _on_title_presence_changed(xuid: String, _title_id: int) -> void:
	if xuid in _watched_xuids:
		await GDK.presence.get_presence_async([xuid])

func _on_presence_changed(xuid: String, _record) -> void:
	_print_presence(xuid)
