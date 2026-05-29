## Per-client connection state on the orchestrator side.
##
## Owns a single StreamPeerTCP, its FrameCodec, the outstanding request slot,
## and the list of pending EventWaiters. Polled per frame by TestOrchestrator.
extends RefCounted

const FrameCodec := preload("res://scripts/frame_codec.gd")
const EventWaiter := preload("res://scripts/event_waiter.gd")

const STATE_AWAIT_HELLO: int = 0
const STATE_READY: int = 1
const STATE_CLOSED: int = 2

const READ_CHUNK: int = 16 * 1024
const HEARTBEAT_INTERVAL_MS: int = 10_000
const HEARTBEAT_PONG_DEADLINE_MS: int = 5_000

var role: String = ""
var capabilities: Dictionary = {}
var session_id: String = ""
var event_log: Array[Dictionary] = []  # last 200 events for diagnostics

var _tree: SceneTree = null
var _peer: StreamPeerTCP = null
var _codec: FrameCodec = null
var _state: int = STATE_AWAIT_HELLO
var _correlation_seq: int = 0
var _outstanding_correlation_id: String = ""
var _last_response: Dictionary = {}
var _event_waiters: Array[EventWaiter] = []
var _next_heartbeat_at_ms: int = 0
var _heartbeat_correlation: String = ""
var _heartbeat_deadline_ms: int = 0
var _close_reason: String = ""


func _init(peer: StreamPeerTCP) -> void:
	_peer = peer
	_codec = FrameCodec.new()
	_next_heartbeat_at_ms = Time.get_ticks_msec() + HEARTBEAT_INTERVAL_MS


func bind_tree(tree: SceneTree) -> void:
	_tree = tree


# ---------------------------------------------------------------------------
# Public API used by scenarios
# ---------------------------------------------------------------------------

func send(command: String, params: Dictionary, timeout_ms: int = 30_000) -> Dictionary:
	if _tree == null:
		return _error_response("client_not_bound", "ClientProxy.%s.send: SceneTree not bound (orchestrator must call bind_tree before exposing this proxy to scenarios)" % role)
	if _state != STATE_READY:
		return _error_response("client_not_ready", "client %s not ready (state=%d)" % [role, _state])
	if not _outstanding_correlation_id.is_empty():
		return _error_response("request_in_flight", "client %s already has request %s in flight" % [role, _outstanding_correlation_id])

	_correlation_seq += 1
	var correlation_id: String = "%s-%d" % [role, _correlation_seq]
	_outstanding_correlation_id = correlation_id

	var frame: Dictionary = {
		"kind": "request",
		"correlation_id": correlation_id,
		"command": command,
		"params": params,
	}
	var send_err: int = _send_frame(frame)
	if send_err != OK:
		_outstanding_correlation_id = ""
		return _error_response("send_failed", "failed to send request frame (err=%d)" % send_err)

	var start_ms: int = Time.get_ticks_msec()
	var deadline_ms: int = start_ms + timeout_ms
	_last_response = {}

	while _last_response.is_empty():
		if Time.get_ticks_msec() >= deadline_ms:
			# Per spec/playfab-multiplayer-test-automation/3-harness-spec.md:187 a
			# timed-out request marks the client lost; the orchestrator must
			# kill+respawn before the next scenario, otherwise the late response
			# could mutate state during a later scenario.
			_outstanding_correlation_id = ""
			_close("request_timeout_%s_%s" % [role, command])
			return {
				"ok": false,
				"duration_ms": Time.get_ticks_msec() - start_ms,
				"result": {},
				"error": {
					"code": "timeout",
					"message": "timed out after %d ms waiting for response to %s.%s" % [timeout_ms, role, command],
				},
			}
		if _state == STATE_CLOSED:
			_outstanding_correlation_id = ""
			return _error_response("client_disconnected", "client %s disconnected: %s" % [role, _close_reason])
		# Pump our own peer so the response is drained even when no outer loop
		# (e.g., the orchestrator's _reset_clients_for_scenario, called between
		# scenarios) is running pump_io for us. Pump is idempotent so it's safe
		# even when an outer scenario-runner loop is also pumping.
		pump_io()
		await _tree.process_frame

	var response: Dictionary = _last_response
	_last_response = {}
	_outstanding_correlation_id = ""

	response["duration_ms"] = Time.get_ticks_msec() - start_ms
	return response


func expect_event(event_type: String, filter: Dictionary = {}) -> EventWaiter:
	var waiter: EventWaiter = EventWaiter.new(event_type, filter, _tree)
	_event_waiters.append(waiter)
	# Replay any already-buffered events that match this filter (subscribe-late safety).
	for ev in event_log:
		if waiter.matches(ev):
			waiter.deliver(ev)
			break
	return waiter


# ---------------------------------------------------------------------------
# Frame I/O and per-frame pump
# ---------------------------------------------------------------------------

func pump_io() -> void:
	if _state == STATE_CLOSED:
		return

	_peer.poll()
	var status: int = _peer.get_status()
	if status != StreamPeerTCP.STATUS_CONNECTED:
		_close("peer_not_connected_status_%d" % status)
		return

	# Drain available bytes into the codec.
	var available: int = _peer.get_available_bytes()
	while available > 0:
		var chunk_size: int = min(available, READ_CHUNK)
		var result: Array = _peer.get_partial_data(chunk_size)
		var err: int = int(result[0])
		var bytes: PackedByteArray = result[1]
		if err != OK:
			_close("read_error_%d" % err)
			return
		if bytes.size() == 0:
			break
		_codec.feed(bytes)
		available = _peer.get_available_bytes()

	# Pop as many frames as we can.
	while true:
		var pop: Dictionary = _codec.try_pop_frame()
		var status_str: String = String(pop.get("status", ""))
		if status_str == "empty":
			break
		if status_str == "error":
			_close("frame_error_%s" % String(pop.get("reason", "unknown")))
			return
		_handle_frame(pop.get("frame", {}))

	_pump_heartbeat()


func is_ready() -> bool:
	return _state == STATE_READY


func is_closed() -> bool:
	return _state == STATE_CLOSED


func close_reason() -> String:
	return _close_reason


func send_shutdown(reason: String) -> void:
	if _state != STATE_READY:
		return
	_send_frame({ "kind": "shutdown", "reason": reason })


func disconnect_client() -> void:
	if _state == STATE_CLOSED:
		return
	if _peer != null and _peer.get_status() == StreamPeerTCP.STATUS_CONNECTED:
		_peer.disconnect_from_host()
	_close("orchestrator_initiated")


# ---------------------------------------------------------------------------
# Internals
# ---------------------------------------------------------------------------

func _handle_frame(frame: Dictionary) -> void:
	var kind: String = String(frame.get("kind", ""))
	match kind:
		"handshake.hello":
			_handle_hello(frame)
		"response":
			_handle_response(frame)
		"event":
			_handle_event(frame)
		"pong":
			_handle_pong(frame)
		"log":
			_handle_log(frame)
		"ping":
			# Client → orchestrator ping (rare; we just pong).
			_send_frame({ "kind": "pong", "correlation_id": String(frame.get("correlation_id", "")) })
		_:
			print("[orch][%s] unknown frame kind: %s" % [role, kind])


func _handle_hello(frame: Dictionary) -> void:
	if _state != STATE_AWAIT_HELLO:
		_send_frame({ "kind": "handshake.reject", "reason": "duplicate_hello", "message": "" })
		_close("duplicate_hello")
		return
	role = String(frame.get("client_id", ""))
	capabilities = frame.get("capabilities", {})
	var protocol_version: int = int(frame.get("protocol_version", 0))
	if protocol_version != 1:
		_send_frame({ "kind": "handshake.reject", "reason": "unsupported_protocol_version", "message": "expected 1" })
		_close("unsupported_protocol_version")
		return
	if role.is_empty():
		_send_frame({ "kind": "handshake.reject", "reason": "missing_client_id", "message": "" })
		_close("missing_client_id")
		return

	session_id = "%d" % Time.get_unix_time_from_system()
	_send_frame({
		"kind": "handshake.welcome",
		"protocol_version": 1,
		"orchestrator_version": "0.1.0",
		"session_id": session_id,
	})
	_state = STATE_READY


func _handle_response(frame: Dictionary) -> void:
	var correlation_id: String = String(frame.get("correlation_id", ""))
	if correlation_id != _outstanding_correlation_id:
		print("[orch][%s] response for unknown correlation_id %s (outstanding=%s)" % [role, correlation_id, _outstanding_correlation_id])
		return
	var ok: bool = bool(frame.get("ok", false))
	var response: Dictionary = {
		"ok": ok,
		"result": frame.get("result", {}),
		"error": frame.get("error", {}),
	}
	_last_response = response


func _handle_event(frame: Dictionary) -> void:
	event_log.append(frame)
	if event_log.size() > 200:
		event_log = event_log.slice(event_log.size() - 200)
	for waiter in _event_waiters:
		if not waiter.is_delivered() and waiter.matches(frame):
			waiter.deliver(frame)
			break


func _handle_pong(_frame: Dictionary) -> void:
	_heartbeat_correlation = ""
	_heartbeat_deadline_ms = 0
	_next_heartbeat_at_ms = Time.get_ticks_msec() + HEARTBEAT_INTERVAL_MS


func _handle_log(frame: Dictionary) -> void:
	var level: String = String(frame.get("level", "info"))
	var msg: String = String(frame.get("message", ""))
	print("[client][%s][%s] %s" % [role, level, msg])


func _pump_heartbeat() -> void:
	var now_ms: int = Time.get_ticks_msec()
	if _heartbeat_correlation.is_empty() and now_ms >= _next_heartbeat_at_ms:
		_correlation_seq += 1
		_heartbeat_correlation = "hb-%d" % _correlation_seq
		_heartbeat_deadline_ms = now_ms + HEARTBEAT_PONG_DEADLINE_MS
		_send_frame({ "kind": "ping", "correlation_id": _heartbeat_correlation })
	elif not _heartbeat_correlation.is_empty() and now_ms >= _heartbeat_deadline_ms:
		_close("heartbeat_timeout")


func _send_frame(frame: Dictionary) -> int:
	if _peer == null:
		return ERR_UNAVAILABLE
	var bytes: PackedByteArray = FrameCodec.encode(frame)
	var send_err: int = _peer.put_data(bytes)
	return send_err


func _close(reason: String) -> void:
	if _state == STATE_CLOSED:
		return
	_state = STATE_CLOSED
	_close_reason = reason
	if _peer != null and _peer.get_status() == StreamPeerTCP.STATUS_CONNECTED:
		_peer.disconnect_from_host()


func _error_response(code: String, message: String) -> Dictionary:
	return {
		"ok": false,
		"duration_ms": 0,
		"result": {},
		"error": { "code": code, "message": message },
	}
