## TestClient: connects to the orchestrator, executes commands.
##
## Lifecycle:
##   1. Parse args: --orchestrator-host, --orchestrator-port, --role, --run-id.
##   2. TCP connect; poll until CONNECTED.
##   3. Send handshake.hello with capabilities.
##   4. Wait for handshake.welcome (or handshake.reject → fatal).
##   5. Loop: read frames, dispatch.
##      - request → dispatch by command name → emit response.
##      - ping    → emit pong.
##      - shutdown → close cleanly, quit.
extends RefCounted

const FrameCodec := preload("res://scripts/frame_codec.gd")
const CommandDispatcher := preload("res://scripts/command_dispatcher.gd")
const PlayFabRuntime := preload("res://scripts/playfab_runtime.gd")
const PlayFabLobbyOps := preload("res://scripts/playfab_lobby_ops.gd")
const PlayFabMatchOps := preload("res://scripts/playfab_match_ops.gd")
const PlayFabPartyOps := preload("res://scripts/playfab_party_ops.gd")

const READ_CHUNK: int = 16 * 1024
const PROTOCOL_VERSION: int = 1
const STATE_CONNECTING: int = 0
const STATE_HANDSHAKE_SENT: int = 1
const STATE_READY: int = 2
const STATE_CLOSED: int = 3

const CONNECT_TIMEOUT_MS: int = 30_000
const HANDSHAKE_TIMEOUT_MS: int = 30_000

var _tree: SceneTree = null
var _host: String = "127.0.0.1"
var _port: int = 18765
var _role: String = ""
var _run_id: String = ""

# Rotates 1..ROTATION_POOL_SIZE per (test client process) so each new
# scenario signs in as the next account in the pool. Spreads PlayFab
# per-(title_player_account) rate-limit budgets across N identities
# instead of stacking every scenario's calls on a single account. The
# index advances inside _handle_reset_client (which runs between
# scenarios), and _derive_custom_id_for_role uses it. The configure
# script provisions {prefix}-{role}-1 .. {prefix}-{role}-N accounts to
# match. See spec/playfab-multiplayer-test-automation/2-detailed-scenarios.md
# "Sign-in pool rotation" for the rotation contract.
const ROTATION_POOL_SIZE: int = 4
var _rotation_index: int = 0

var _peer: StreamPeerTCP = null
var _codec: FrameCodec = null
var _state: int = STATE_CONNECTING
var _shutdown_requested: bool = false
var _shutdown_reason: String = ""
var _dispatcher: CommandDispatcher = null
var _playfab_runtime: PlayFabRuntime = null
var _lobby_ops: PlayFabLobbyOps = null
var _match_ops: PlayFabMatchOps = null
var _party_ops: PlayFabPartyOps = null


func bind_tree(tree: SceneTree) -> void:
	_tree = tree


func run_async() -> int:
	_parse_args()
	if _role.is_empty():
		printerr("[client] --role is required")
		return 2

	print("[client][%s] connecting to %s:%d" % [_role, _host, _port])

	_codec = FrameCodec.new()
	_peer = StreamPeerTCP.new()
	var connect_err: int = _peer.connect_to_host(_host, _port)
	if connect_err != OK:
		printerr("[client][%s] connect_to_host failed: %d" % [_role, connect_err])
		return 2

	# Wait for CONNECTED
	var connect_deadline_ms: int = Time.get_ticks_msec() + CONNECT_TIMEOUT_MS
	while true:
		_peer.poll()
		var status: int = _peer.get_status()
		if status == StreamPeerTCP.STATUS_CONNECTED:
			break
		if status == StreamPeerTCP.STATUS_ERROR:
			printerr("[client][%s] TCP error during connect" % _role)
			return 2
		if Time.get_ticks_msec() >= connect_deadline_ms:
			printerr("[client][%s] timeout connecting to %s:%d" % [_role, _host, _port])
			return 2
		await _tree.process_frame

	_dispatcher = CommandDispatcher.new()
	_playfab_runtime = PlayFabRuntime.new()
	_playfab_runtime.bind_tree(_tree)
	_lobby_ops = PlayFabLobbyOps.new()
	_lobby_ops.bind(_playfab_runtime)
	_match_ops = PlayFabMatchOps.new()
	_match_ops.bind(_playfab_runtime)
	_party_ops = PlayFabPartyOps.new()
	_party_ops.bind(_playfab_runtime)
	_register_commands()

	# Send handshake.hello
	var hello: Dictionary = {
		"kind": "handshake.hello",
		"protocol_version": PROTOCOL_VERSION,
		"client_id": _role,
		"capabilities": _build_capabilities(),
		"run_id": _run_id,
	}
	var send_err: int = _send_frame(hello)
	if send_err != OK:
		printerr("[client][%s] failed to send hello: %d" % [_role, send_err])
		return 2
	_state = STATE_HANDSHAKE_SENT

	# Wait for handshake.welcome
	var handshake_deadline_ms: int = Time.get_ticks_msec() + HANDSHAKE_TIMEOUT_MS
	while _state == STATE_HANDSHAKE_SENT:
		if Time.get_ticks_msec() >= handshake_deadline_ms:
			printerr("[client][%s] timeout waiting for welcome" % _role)
			return 2
		_pump_io_once()
		await _tree.process_frame
		if _state == STATE_CLOSED:
			printerr("[client][%s] closed during handshake: %s" % [_role, _shutdown_reason])
			return 2

	print("[client][%s] handshake complete; entering command loop" % _role)

	# Main loop
	while _state == STATE_READY and not _shutdown_requested:
		_pump_io_once()
		await _tree.process_frame

	# Graceful close
	if _peer != null and _peer.get_status() == StreamPeerTCP.STATUS_CONNECTED:
		_peer.disconnect_from_host()
	print("[client][%s] exiting (shutdown_reason=%s)" % [_role, _shutdown_reason])
	return 0


func _build_capabilities() -> Dictionary:
	var pf_available: bool = _playfab_runtime != null and _playfab_runtime.is_available()
	# Surface env-driven capabilities the matrix at
	# spec/playfab-multiplayer-test-automation/1-test-matrix.md gates on.
	# The wrapper script (tools/run_mp_orchestrator.ps1) propagates these.
	var match_queue: String = OS.get_environment("PLAYFAB_MULTIPLAYER_MATCH_QUEUE").strip_edges()
	var live_write: String = OS.get_environment("LIVE_WRITE_TESTS").strip_edges()
	return {
		"protocol_version": PROTOCOL_VERSION,
		"platform": OS.get_name(),
		"client_version": "0.1.0",
		"playfab_multiplayer_available": pf_available,
		"playfab_party_available": pf_available,
		"matchmaking_queue_configured": pf_available and not match_queue.is_empty(),
		"live_write_allowed": live_write == "1",
	}


# Account-role mapping: the configure script provisions
# `<prefix>-multiplayer-<account_role>` accounts, but the orchestrator
# uses friendlier names (host/guest/guest2/observer) for clarity in
# scenarios. This table maps orchestrator role -> provisioned account
# role. Unknown roles map to themselves (a no-op for any future role).
const ROLE_ACCOUNT_MAP := {
	"host": "host",
	"guest": "client",
	"guest2": "client2",
	"observer": "observer",
}


func _custom_id_prefix() -> String:
	# Explicit prefix env wins (matches the legacy worker script's
	# `PLAYFAB_MULTIPLAYER_CUSTOM_ID_PREFIX` and the
	# configure_playfab_test_title.ps1 `-MultiplayerCustomIdPrefix` knob).
	var explicit: String = OS.get_environment("PLAYFAB_MULTIPLAYER_CUSTOM_ID_PREFIX").strip_edges()
	if not explicit.is_empty():
		return explicit
	var base_id: String = OS.get_environment("PLAYFAB_CUSTOM_ID").strip_edges()
	if not base_id.is_empty():
		return "%s-multiplayer" % base_id
	# Last-resort default mirrors the configure script's `$CustomId`
	# default so a developer running `run_mp_orchestrator.ps1` without
	# any env still hits the canonical sandbox accounts.
	return "godot-gdk-ext-live-smoke-multiplayer"


func _derive_custom_id_for_role() -> String:
	var account_role: String = String(ROLE_ACCOUNT_MAP.get(_role, _role))
	var pool_slot: int = (_rotation_index % ROTATION_POOL_SIZE) + 1
	if account_role.is_empty():
		return "%s-%d" % [_custom_id_prefix(), pool_slot]
	return "%s-%s-%d" % [_custom_id_prefix(), account_role, pool_slot]


func _register_commands() -> void:
	_dispatcher.register("ping", func(params):
		return { "ok": true, "result": { "nonce": String(params.get("nonce", "")), "ts_ms": Time.get_ticks_msec() } })

	_dispatcher.register("sign_in", func(params):
		# Custom-id resolution order:
		#   1. Explicit `custom_id` param (negative tests, suffix overrides).
		#   2. `custom_id_suffix` appended to the configured prefix.
		#   3. Auto-derived from --role + rotation (the normal scenario path).
		# The configure script (tools/configure_playfab_test_title.ps1)
		# provisions `<PLAYFAB_CUSTOM_ID>-multiplayer-{host,client,client2,observer}-{1..N}`
		# pooled accounts; the rotation index advances on every reset_client
		# so successive scenarios sign in as the next account in the pool.
		# This spreads PlayFab per-(title_player_account) rate-limit budgets
		# across N identities — see ROTATION_POOL_SIZE.
		#
		# PlayFab title player creation is typically disabled
		# (E_PF_PLAYER_CREATION_DISABLED / 0x892357BA), so we default
		# `create_account=false` and rely on the configure script.
		var explicit_id: String = String(params.get("custom_id", "")).strip_edges()
		var suffix: String = String(params.get("custom_id_suffix", "")).strip_edges()
		var custom_id: String = explicit_id
		if custom_id.is_empty():
			if not suffix.is_empty():
				custom_id = _custom_id_prefix() + suffix
			else:
				custom_id = _derive_custom_id_for_role()
		var create_account: bool = bool(params.get("create_account", false))
		var sign_in_result: Dictionary = await _playfab_runtime.sign_in_with_custom_id(custom_id, create_account)
		if not bool(sign_in_result.get("ok", false)):
			return { "ok": false, "error": sign_in_result.get("error", { "code": "sign_in_failed", "message": "" }) }
		var data: Dictionary = sign_in_result.duplicate()
		data.erase("ok")
		return { "ok": true, "result": data })

	# Lobby commands
	_dispatcher.register("create_lobby", func(params): return await _lobby_ops.create_lobby(params))
	_dispatcher.register("join_lobby", func(params): return await _lobby_ops.join_lobby(params))
	_dispatcher.register("search_lobbies", func(params): return await _lobby_ops.search_lobbies(params))
	_dispatcher.register("set_lobby_properties", func(params): return await _lobby_ops.set_lobby_properties(params))
	_dispatcher.register("set_member_properties", func(params): return await _lobby_ops.set_member_properties(params))
	_dispatcher.register("get_lobby_snapshot", func(params): return await _lobby_ops.get_lobby_snapshot(params))
	_dispatcher.register("leave_lobby", func(params): return await _lobby_ops.leave_lobby(params))

	# Match-ticket commands
	_dispatcher.register("create_match_ticket", func(params): return await _match_ops.create_match_ticket(params))
	_dispatcher.register("inspect_match_ticket", func(params): return _match_ops.inspect_match_ticket(params))
	_dispatcher.register("wait_match_ticket", func(params): return await _match_ops.wait_match_ticket(params))
	_dispatcher.register("cancel_match_ticket", func(params): return await _match_ops.cancel_match_ticket(params))

	# Party commands
	_dispatcher.register("party_initialize", func(params): return await _party_ops.initialize_party(params))
	_dispatcher.register("party_create_network", func(params): return await _party_ops.create_network(params))
	_dispatcher.register("party_join_network", func(params): return await _party_ops.join_network(params))
	_dispatcher.register("party_snapshot", func(params): return _party_ops.get_snapshot(params))
	_dispatcher.register("party_leave_network", func(params): return await _party_ops.leave_network(params))
	_dispatcher.register("party_send_chat_text", func(params): return await _party_ops.send_chat_text(params))

	# Lifecycle
	_dispatcher.register("reset_client", _handle_reset_client)


func _handle_reset_client(_params: Dictionary) -> Dictionary:
	var lobby_reset: Dictionary = await _lobby_ops.reset({})
	var match_reset: Dictionary = await _match_ops.reset({})
	var party_reset: Dictionary = await _party_ops.reset({})
	# Advance the rotation index AFTER releasing handles so the next
	# scenario's sign_in lands on the next account in the pool. We rotate
	# unconditionally (not just on rate-limit pressure) so the budget
	# spread is deterministic regardless of which scenario ran previously.
	_rotation_index += 1
	return {
		"ok": (
			bool(lobby_reset.get("ok", false))
			and bool(match_reset.get("ok", false))
			and bool(party_reset.get("ok", false))
		),
		"result": {
			"lobby": lobby_reset.get("result", {}),
			"match": match_reset.get("result", {}),
			"party": party_reset.get("result", {}),
			"rotation_index": _rotation_index,
		},
	}


# ---------------------------------------------------------------------------
# Internals
# ---------------------------------------------------------------------------

func _parse_args() -> void:
	var args: PackedStringArray = OS.get_cmdline_user_args()
	var i: int = 0
	while i < args.size():
		var a: String = String(args[i])
		match a:
			"--orchestrator-host":
				if i + 1 < args.size():
					_host = String(args[i + 1]); i += 2
				else: i += 1
			"--orchestrator-port":
				if i + 1 < args.size():
					_port = int(args[i + 1]); i += 2
				else: i += 1
			"--role":
				if i + 1 < args.size():
					_role = String(args[i + 1]); i += 2
				else: i += 1
			"--run-id":
				if i + 1 < args.size():
					_run_id = String(args[i + 1]); i += 2
				else: i += 1
			_:
				i += 1


func _pump_io_once() -> void:
	if _playfab_runtime != null:
		_playfab_runtime.dispatch()
	_peer.poll()
	var status: int = _peer.get_status()
	if status != StreamPeerTCP.STATUS_CONNECTED:
		_state = STATE_CLOSED
		_shutdown_reason = "peer_status_%d" % status
		return

	var available: int = _peer.get_available_bytes()
	while available > 0:
		var chunk: int = min(available, READ_CHUNK)
		var result: Array = _peer.get_partial_data(chunk)
		var err: int = int(result[0])
		var bytes: PackedByteArray = result[1]
		if err != OK:
			_state = STATE_CLOSED
			_shutdown_reason = "read_err_%d" % err
			return
		if bytes.size() == 0:
			break
		_codec.feed(bytes)
		available = _peer.get_available_bytes()

	while true:
		var pop: Dictionary = _codec.try_pop_frame()
		var st: String = String(pop.get("status", ""))
		if st == "empty":
			break
		if st == "error":
			_state = STATE_CLOSED
			_shutdown_reason = "frame_err_%s" % String(pop.get("reason", "?"))
			return
		_handle_frame(pop.get("frame", {}))


func _handle_frame(frame: Dictionary) -> void:
	var kind: String = String(frame.get("kind", ""))
	match kind:
		"handshake.welcome":
			_state = STATE_READY
			print("[client][%s] welcome session_id=%s orchestrator=%s" % [
				_role,
				String(frame.get("session_id", "")),
				String(frame.get("orchestrator_version", "?")),
			])
		"handshake.reject":
			_state = STATE_CLOSED
			_shutdown_reason = "handshake_rejected_%s" % String(frame.get("reason", "?"))
		"request":
			_handle_request(frame)
		"ping":
			_send_frame({ "kind": "pong", "correlation_id": String(frame.get("correlation_id", "")) })
		"shutdown":
			_shutdown_requested = true
			_shutdown_reason = "shutdown_from_orchestrator_%s" % String(frame.get("reason", "ok"))
		_:
			print("[client][%s] unknown frame kind: %s" % [_role, kind])


func _handle_request(frame: Dictionary) -> void:
	var correlation_id: String = String(frame.get("correlation_id", ""))
	var command: String = String(frame.get("command", ""))
	var params: Dictionary = frame.get("params", {})
	var dispatch_result: Dictionary = await _dispatcher.dispatch(command, params)
	var response: Dictionary = {
		"kind": "response",
		"correlation_id": correlation_id,
		"ok": bool(dispatch_result.get("ok", false)),
		"result": dispatch_result.get("result", {}),
		"error": dispatch_result.get("error", {}),
	}
	_send_frame(response)


func _send_frame(frame: Dictionary) -> int:
	if _peer == null:
		return ERR_UNAVAILABLE
	var bytes: PackedByteArray = FrameCodec.encode(frame)
	return _peer.put_data(bytes)
