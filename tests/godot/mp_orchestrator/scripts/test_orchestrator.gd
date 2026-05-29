## TestOrchestrator: top-level harness control flow.
##
## Lifecycle:
##   1. Parse command-line args.
##   2. Bind TCP server.
##   3. Spawn N test client subprocesses (one per requested role).
##   4. Wait for all clients to handshake.
##   5. Discover + sort scenarios; filter by --filter regex.
##   6. Run each scenario sequentially with per-scenario timeout.
##   7. After every scenario, send reset_client to every connected client.
##   8. Write results JSON + Markdown.
##   9. Send shutdown to every client, wait for orderly close, exit.
##
## Exit code: 0 if every non-quarantined scenario passed, 1 otherwise.
extends RefCounted

const FrameCodec := preload("res://scripts/frame_codec.gd")
const ClientProxy := preload("res://scripts/client_proxy.gd")
const ResultsWriter := preload("res://scripts/results_writer.gd")

const DEFAULT_PORT: int = 18765
const HANDSHAKE_TIMEOUT_MS: int = 30_000
const RESET_TIMEOUT_MS: int = 10_000
const SHUTDOWN_TIMEOUT_MS: int = 5_000

var _tree: SceneTree = null

# Args
var _port: int = DEFAULT_PORT
var _bind_host: String = "127.0.0.1"
var _roles: Array[String] = ["host"]
var _scenarios_filter: String = ".*"
var _list_only: bool = false
var _results_dir: String = ""
var _scenarios_dir: String = "res://scenarios"
var _client_godot: String = ""
var _client_project: String = ""
var _spawn_clients: bool = true
var _extra_client_args: Array[String] = []
var _allow_quarantined_failures: bool = true

# Runtime
var _server: TCPServer = null
var _clients: Dictionary = {}  # role -> ClientProxy
var _pending_connections: Array[StreamPeerTCP] = []
var _pids_by_role: Dictionary = {}  # role -> PID for kill+respawn
var _client_arg_template: PackedStringArray = PackedStringArray()
var _run_id: String = ""
var _results: ResultsWriter = null


func bind_tree(tree: SceneTree) -> void:
	_tree = tree


func run_async() -> int:
	_parse_args()

	_run_id = OS.get_environment("MP_TEST_RUN_ID")
	if _run_id.is_empty():
		_run_id = "mp-%d" % Time.get_unix_time_from_system()

	if _results_dir.is_empty():
		# Repo-root build/test-results/mp-test/<run_id> matches other repo
		# tooling (e.g. tools/run_all_tests.ps1 writes under build/test-results)
		# and is documented as the default by tools/run_mp_orchestrator.ps1. The
		# wrapper sets MP_TEST_REPO_ROOT so the orchestrator can resolve it
		# without baking an assumption about its own project path. If unset
		# (e.g. someone runs Godot against the project directly without the
		# wrapper) we fall back to a project-local res:// directory.
		var repo_root: String = OS.get_environment("MP_TEST_REPO_ROOT").strip_edges()
		if not repo_root.is_empty():
			_results_dir = "%s/build/test-results/mp-test/%s" % [repo_root.replace("\\", "/"), _run_id]
		else:
			_results_dir = ProjectSettings.globalize_path("res://build/test-results/mp-test/%s" % _run_id)

	_results = ResultsWriter.new()
	_results.run_id = _run_id
	_results.started_at_unix = int(Time.get_unix_time_from_system())

	print("[orch] starting run_id=%s port=%d roles=%s scenarios_dir=%s" % [
		_run_id, _port, str(_roles), _scenarios_dir,
	])

	# 1. Bind TCP server.
	_server = TCPServer.new()
	var listen_err: int = _server.listen(_port, _bind_host)
	if listen_err != OK:
		printerr("[orch] failed to bind %s:%d (err=%d)" % [_bind_host, _port, listen_err])
		return 2

	# 2. Discover scenarios early; --list short-circuits.
	var scenarios: Array[Dictionary] = _discover_scenarios()
	if _list_only:
		print("[orch] discovered %d scenarios:" % scenarios.size())
		for s in scenarios:
			print("  %s (%s) — %s" % [s.id, s.priority, s.name])
		return 0

	# 3. Spawn clients.
	if _spawn_clients:
		var spawn_err: int = _spawn_client_processes()
		if spawn_err != OK:
			_teardown()
			return 2
	else:
		print("[orch] --no-spawn: waiting for %d external clients to connect" % _roles.size())

	# 4. Wait for all clients to handshake.
	var ready_err: int = await _wait_until_clients_ready()
	if ready_err != OK:
		_teardown()
		return 2

	print("[orch] %d/%d clients ready: %s" % [_clients.size(), _roles.size(), str(_clients.keys())])

	# 5/6. Run scenarios.
	for scenario_info in scenarios:
		await _run_one_scenario(scenario_info)

	_results.finished_at_unix = int(Time.get_unix_time_from_system())

	# 7. Write results.
	var json_path: String = "%s/mp-test-results.json" % _results_dir
	var md_path: String = "%s/mp-test-results.md" % _results_dir
	var write_failure: bool = false
	var json_err: int = _results.write_json(json_path)
	if json_err == OK:
		print("[orch] wrote results: %s" % json_path)
	else:
		printerr("[orch] FAILED to write %s (err=%d)" % [json_path, json_err])
		write_failure = true
	var md_err: int = _results.write_markdown(md_path)
	if md_err == OK:
		print("[orch] wrote results: %s" % md_path)
	else:
		printerr("[orch] FAILED to write %s (err=%d)" % [md_path, md_err])
		write_failure = true

	# 8. Shutdown clients.
	await _shutdown_clients()

	_teardown()

	var summary: Dictionary = _results.summary()
	print("[orch] run finished: total=%d passed=%d failed=%d skipped=%d quarantined_failures=%d invalid=%d" % [
		int(summary.total), int(summary.passed), int(summary.failed),
		int(summary.skipped), int(summary.quarantined_failures), int(summary.invalid),
	])
	if write_failure:
		printerr("[orch] one or more results files failed to write; orchestrator exiting non-zero")
		return 2

	return 1 if _results.has_failures(_allow_quarantined_failures) else 0


# ---------------------------------------------------------------------------
# Scenario API surface exposed to scenarios via run(orch)
# ---------------------------------------------------------------------------

func connected_roles() -> Array:
	var out: Array = []
	for role in _clients.keys():
		var c: ClientProxy = _clients[role]
		if c.is_ready():
			out.append(role)
	out.sort()
	return out


func client(role: String) -> ClientProxy:
	if _clients.has(role):
		return _clients[role]
	return null


func get_tree() -> SceneTree:
	return _tree


func env(name: String, default_value: String = "") -> String:
	var v: String = OS.get_environment(name)
	if v.is_empty():
		return default_value
	return v


func run_id() -> String:
	return _run_id


func log(level: String, message: String, context: Dictionary = {}) -> void:
	var ctx_str: String = ""
	if not context.is_empty():
		ctx_str = " " + JSON.stringify(context)
	print("[scenario][%s] %s%s" % [level, message, ctx_str])


# ---------------------------------------------------------------------------
# Argument parsing
# ---------------------------------------------------------------------------

func _parse_args() -> void:
	var args: PackedStringArray = OS.get_cmdline_user_args()
	var i: int = 0
	while i < args.size():
		var arg: String = String(args[i])
		match arg:
			"--port":
				if i + 1 < args.size():
					_port = int(args[i + 1])
					i += 2
				else:
					i += 1
			"--bind-host":
				if i + 1 < args.size():
					_bind_host = String(args[i + 1])
					i += 2
				else:
					i += 1
			"--role":
				if i + 1 < args.size():
					_roles = []
					for r in String(args[i + 1]).split(","):
						var trimmed: String = r.strip_edges()
						if not trimmed.is_empty():
							_roles.append(trimmed)
					i += 2
				else:
					i += 1
			"--filter":
				if i + 1 < args.size():
					_scenarios_filter = String(args[i + 1])
					i += 2
				else:
					i += 1
			"--list":
				_list_only = true
				i += 1
			"--results-dir":
				if i + 1 < args.size():
					_results_dir = String(args[i + 1])
					i += 2
				else:
					i += 1
			"--scenarios-dir":
				if i + 1 < args.size():
					_scenarios_dir = String(args[i + 1])
					i += 2
				else:
					i += 1
			"--client-godot":
				if i + 1 < args.size():
					_client_godot = String(args[i + 1])
					i += 2
				else:
					i += 1
			"--client-project":
				if i + 1 < args.size():
					_client_project = String(args[i + 1])
					i += 2
				else:
					i += 1
			"--no-spawn":
				_spawn_clients = false
				i += 1
			"--strict-quarantined":
				_allow_quarantined_failures = false
				i += 1
			"--client-arg":
				if i + 1 < args.size():
					_extra_client_args.append(String(args[i + 1]))
					i += 2
				else:
					i += 1
			_:
				print("[orch] unknown arg: %s" % arg)
				i += 1


# ---------------------------------------------------------------------------
# Client process management
# ---------------------------------------------------------------------------

func _spawn_client_processes() -> int:
	if _client_godot.is_empty() or _client_project.is_empty():
		printerr("[orch] --client-godot and --client-project are required when spawning clients")
		return ERR_INVALID_PARAMETER
	if not FileAccess.file_exists(_client_godot):
		printerr("[orch] --client-godot does not point at a real file: %s" % _client_godot)
		return ERR_FILE_NOT_FOUND
	if not DirAccess.dir_exists_absolute(_client_project):
		printerr("[orch] --client-project does not point at a real directory: %s" % _client_project)
		return ERR_FILE_NOT_FOUND

	for role in _roles:
		var pid: int = _spawn_one_client(role)
		if pid <= 0:
			printerr("[orch] failed to spawn client for role %s" % role)
			return ERR_CANT_FORK
		_pids_by_role[role] = pid
		print("[orch] spawned client role=%s pid=%d" % [role, pid])
	return OK


## Spawns a Godot test-client subprocess for a given role and returns the PID
## (or <=0 on failure). Shared between initial spawn and respawn.
func _spawn_one_client(role: String) -> int:
	var args: PackedStringArray = PackedStringArray([
		"--headless",
		"--path", _client_project,
		"--script", "res://main.gd",
		"--",
		"--orchestrator-host", _bind_host,
		"--orchestrator-port", str(_port),
		"--role", role,
		"--run-id", _run_id,
	])
	for extra in _extra_client_args:
		args.append(extra)
	return OS.create_process(_client_godot, args)


## Kills the (presumed-lost) client for `role` and spawns a replacement.
## Waits for the replacement to handshake. Returns OK on success, ERR_TIMEOUT
## or ERR_CANT_FORK on failure. Used between scenarios when a client's
## previous in-flight request timed out (which makes any late response a
## state-leak risk per spec/3-harness-spec.md:187).
func _respawn_client(role: String) -> int:
	# Tear down the in-process proxy and the OS process.
	if _clients.has(role):
		var stale: ClientProxy = _clients[role]
		stale.disconnect_client()
		_clients.erase(role)
	var prev_pid: int = int(_pids_by_role.get(role, 0))
	if prev_pid > 0:
		var kill_err: int = OS.kill(prev_pid)
		if kill_err != OK and kill_err != ERR_INVALID_PARAMETER:
			print("[orch] respawn: kill(role=%s pid=%d) returned %d (continuing)" % [role, prev_pid, kill_err])
		_pids_by_role.erase(role)

	var new_pid: int = _spawn_one_client(role)
	if new_pid <= 0:
		printerr("[orch] respawn: failed to spawn replacement for role %s" % role)
		return ERR_CANT_FORK
	_pids_by_role[role] = new_pid
	print("[orch] respawn: spawned replacement role=%s pid=%d" % [role, new_pid])

	var deadline_ms: int = Time.get_ticks_msec() + HANDSHAKE_TIMEOUT_MS
	while not (_clients.has(role) and (_clients[role] as ClientProxy).is_ready()):
		if Time.get_ticks_msec() >= deadline_ms:
			printerr("[orch] respawn: timed out waiting for role %s handshake" % role)
			return ERR_TIMEOUT
		_pump_io()
		await _tree.process_frame
	return OK


# ---------------------------------------------------------------------------
# Per-frame I/O pump (drives all connected clients and accepts new ones)
# ---------------------------------------------------------------------------

func _pump_io() -> void:
	if _server == null:
		return
	while _server.is_connection_available():
		var peer: StreamPeerTCP = _server.take_connection()
		if peer == null:
			break
		var proxy: ClientProxy = ClientProxy.new(peer)
		proxy.bind_tree(_tree)
		_pending_connections.append(peer)
		# Stash on a side bucket keyed by peer; when handshake completes we
		# move the proxy into _clients keyed by role.
		_proxies_by_peer[peer] = proxy

	for peer in _pending_connections.duplicate():
		var proxy: ClientProxy = _proxies_by_peer.get(peer, null)
		if proxy == null:
			_pending_connections.erase(peer)
			continue
		proxy.pump_io()
		if proxy.is_closed():
			_pending_connections.erase(peer)
			_proxies_by_peer.erase(peer)
			continue
		if proxy.is_ready():
			if proxy.role.is_empty():
				proxy.disconnect_client()
			elif _clients.has(proxy.role):
				printerr("[orch] duplicate client role %s — closing the late one" % proxy.role)
				proxy.disconnect_client()
			else:
				_clients[proxy.role] = proxy
				print("[orch] client ready: role=%s capabilities=%s" % [proxy.role, str(proxy.capabilities)])
			_pending_connections.erase(peer)
			_proxies_by_peer.erase(peer)

	for role in _clients.keys():
		var c: ClientProxy = _clients[role]
		c.pump_io()


var _proxies_by_peer: Dictionary = {}


# ---------------------------------------------------------------------------
# Wait helpers
# ---------------------------------------------------------------------------

func _wait_until_clients_ready() -> int:
	var deadline_ms: int = Time.get_ticks_msec() + HANDSHAKE_TIMEOUT_MS
	while not _all_required_roles_ready():
		if Time.get_ticks_msec() >= deadline_ms:
			var missing: Array = []
			for role in _roles:
				if not _clients.has(String(role)) or not (_clients[String(role)] as ClientProxy).is_ready():
					missing.append(role)
			printerr("[orch] handshake timeout: connected=%s expected=%s missing=%s" % [str(_clients.keys()), str(_roles), str(missing)])
			return ERR_TIMEOUT
		_pump_io()
		await _tree.process_frame
	return OK


func _all_required_roles_ready() -> bool:
	for role in _roles:
		var role_str: String = String(role)
		if not _clients.has(role_str):
			return false
		if not (_clients[role_str] as ClientProxy).is_ready():
			return false
	return true


# ---------------------------------------------------------------------------
# Scenario discovery + execution
# ---------------------------------------------------------------------------

func _discover_scenarios() -> Array[Dictionary]:
	var found: Array[Dictionary] = []
	_collect_scenarios_recursive(_scenarios_dir, found)
	var regex: RegEx = RegEx.new()
	if regex.compile(_scenarios_filter) != OK:
		printerr("[orch] invalid --filter regex: %s (no scenarios will run)" % _scenarios_filter)
		return [] as Array[Dictionary]
	var filtered: Array[Dictionary] = []
	for entry in found:
		if regex.search(String(entry.id)) == null:
			continue
		filtered.append(entry)
	filtered.sort_custom(func(a, b):
		var pa: String = String(a.priority)
		var pb: String = String(b.priority)
		if pa != pb:
			return pa < pb
		return String(a.id) < String(b.id)
	)
	return filtered


func _collect_scenarios_recursive(dir_path: String, out: Array[Dictionary]) -> void:
	var dir: DirAccess = DirAccess.open(dir_path)
	if dir == null:
		return
	dir.list_dir_begin()
	while true:
		var name: String = dir.get_next()
		if name.is_empty():
			break
		if name.begins_with("."):
			continue
		var full: String = "%s/%s" % [dir_path, name]
		if dir.current_is_dir():
			_collect_scenarios_recursive(full, out)
			continue
		if not name.ends_with(".gd"):
			continue
		var script: Resource = load(full)
		if script == null:
			out.append({ "id": full, "name": full, "path": full, "priority": "P0", "category": "?", "status_at_discovery": ResultsWriter.STATUS_INVALID, "failure_reason": "load_failed" })
			continue
		var meta: Dictionary = _read_metadata(script, full)
		if meta.is_empty():
			# No SCENARIO_ID constant → not a scenario file (e.g. shared
			# base class or helper). Silently skip rather than flagging as
			# invalid_metadata; the orchestrator's invalid-count is reserved
			# for declared scenarios that fail to load.
			continue
		meta["path"] = full
		out.append(meta)
	dir.list_dir_end()


func _read_metadata(script: Resource, path: String) -> Dictionary:
	var script_gd: GDScript = script as GDScript
	if script_gd == null:
		return {}
	var constants: Dictionary = script_gd.get_script_constant_map()
	if not constants.has("SCENARIO_ID") or not constants.has("SCENARIO_NAME"):
		return {}
	return {
		"id": String(constants.get("SCENARIO_ID", path)),
		"name": String(constants.get("SCENARIO_NAME", path)),
		"category": String(constants.get("CATEGORY", "functional")),
		"priority": String(constants.get("PRIORITY", "P0")),
		"required_roles": constants.get("REQUIRED_ROLES", []),
		"required_capabilities": constants.get("REQUIRED_CAPABILITIES", []),
		"timeout_sec": int(constants.get("TIMEOUT_SEC", 60)),
		"quarantined": bool(constants.get("QUARANTINED", false)),
	}


func _run_one_scenario(info: Dictionary) -> void:
	var id: String = String(info.id)
	var name: String = String(info.name)

	if info.has("status_at_discovery"):
		_results.add_scenario({
			"id": id, "name": name,
			"status": String(info["status_at_discovery"]),
			"duration_ms": 0,
			"failure_reason": String(info.get("failure_reason", "")),
			"details": {},
		})
		print("[orch] scenario %s — %s" % [id, info["status_at_discovery"]])
		return

	# Check required_roles
	var missing_roles: Array = []
	for role in info.get("required_roles", []):
		if not _clients.has(String(role)) or not (_clients[String(role)] as ClientProxy).is_ready():
			missing_roles.append(role)
	if not missing_roles.is_empty():
		_results.add_scenario({
			"id": id, "name": name,
			"status": ResultsWriter.STATUS_SKIPPED,
			"duration_ms": 0,
			"failure_reason": "missing_roles: %s" % str(missing_roles),
			"details": {},
		})
		print("[orch] scenario %s SKIPPED (missing roles %s)" % [id, str(missing_roles)])
		return

	# Check capabilities. Some capabilities are synthesized by the orchestrator
	# (it knows about cross-client topology in a way no individual client can).
	var missing_caps: Array = []
	for cap in info.get("required_capabilities", []):
		var cap_str: String = String(cap)
		if _is_synthetic_capability(cap_str):
			if not _orchestrator_has_capability(cap_str):
				missing_caps.append("%s (orchestrator)" % cap_str)
			continue
		for role in info.get("required_roles", []):
			var role_str: String = String(role)
			var c: ClientProxy = _clients[role_str]
			if not bool(c.capabilities.get(cap_str, false)):
				missing_caps.append("%s on %s" % [cap_str, role_str])
	if not missing_caps.is_empty():
		_results.add_scenario({
			"id": id, "name": name,
			"status": ResultsWriter.STATUS_SKIPPED,
			"duration_ms": 0,
			"failure_reason": "missing_capabilities: %s" % str(missing_caps),
			"details": {},
		})
		print("[orch] scenario %s SKIPPED (missing capabilities %s)" % [id, str(missing_caps)])
		return

	var script: Resource = load(String(info.path))
	var scenario: RefCounted = (script as GDScript).new()

	var quarantined: bool = bool(info.get("quarantined", false))
	var timeout_sec: int = max(1, int(info.get("timeout_sec", 60)))
	var deadline_ms: int = Time.get_ticks_msec() + timeout_sec * 1000

	print("[orch] scenario %s RUN (timeout=%ds)" % [id, timeout_sec])
	var start_ms: int = Time.get_ticks_msec()

	# Optional per-scenario setup() hook per
	# spec/playfab-multiplayer-test-automation/4-scenario-authoring.md.
	# A setup() that returns a skip/fail Dictionary short-circuits run() so
	# precondition checks can refuse the scenario without it running.
	if scenario.has_method("setup"):
		var setup_result: Variant = await scenario.setup(self)
		if setup_result is Dictionary:
			var setup_dict: Dictionary = setup_result
			var has_decision: bool = bool(setup_dict.get("skipped", false)) or setup_dict.has("ok")
			if has_decision and not bool(setup_dict.get("ok", true)):
				_results.add_scenario({
					"id": id, "name": name,
					"status": ResultsWriter.STATUS_FAILED,
					"duration_ms": Time.get_ticks_msec() - start_ms,
					"failure_reason": "setup_failed: %s" % String(setup_dict.get("failure_reason", "")),
					"details": setup_dict.get("details", {}),
				})
				print("[orch] scenario %s FAILED in setup() — skipping run()" % id)
				if scenario.has_method("cleanup"):
					await scenario.cleanup(self)
				await _reset_clients_for_scenario(info)
				return
			if has_decision and bool(setup_dict.get("skipped", false)):
				_results.add_scenario({
					"id": id, "name": name,
					"status": ResultsWriter.STATUS_SKIPPED,
					"duration_ms": Time.get_ticks_msec() - start_ms,
					"failure_reason": String(setup_dict.get("failure_reason", "")),
					"details": setup_dict.get("details", {}),
				})
				print("[orch] scenario %s SKIPPED by setup()" % id)
				if scenario.has_method("cleanup"):
					await scenario.cleanup(self)
				await _reset_clients_for_scenario(info)
				return

	# Spawn the scenario as a coroutine and race it against the wall-clock timeout.
	var scenario_done: Array = [false]
	var scenario_result: Array = [{}]
	_run_scenario_call_async(scenario, scenario_done, scenario_result)

	while not scenario_done[0]:
		if Time.get_ticks_msec() >= deadline_ms:
			break
		_pump_io()
		await _tree.process_frame

	var duration_ms: int = Time.get_ticks_msec() - start_ms
	var entry: Dictionary
	if not scenario_done[0]:
		# Timeout: the scenario coroutine is still alive. To prevent it from
		# resuming during the next scenario and mutating ClientProxy state,
		# forcibly close every involved client's connection. That lets the
		# pending coroutine see STATE_CLOSED on its next await and bail; the
		# reset pass below sees is_closed() and respawns.
		_invalidate_clients_for_timeout(info, "scenario_timeout_%s" % id)
		entry = {
			"id": id, "name": name,
			"status": ResultsWriter.STATUS_FAILED,
			"duration_ms": duration_ms,
			"failure_reason": "timeout after %d seconds" % timeout_sec,
			"details": {},
		}
	else:
		var result: Dictionary = scenario_result[0]
		if bool(result.get("skipped", false)):
			entry = {
				"id": id, "name": name,
				"status": ResultsWriter.STATUS_SKIPPED,
				"duration_ms": duration_ms,
				"failure_reason": String(result.get("failure_reason", "")),
				"details": result.get("details", {}),
			}
		elif bool(result.get("ok", false)):
			entry = {
				"id": id, "name": name,
				"status": ResultsWriter.STATUS_PASSED,
				"duration_ms": duration_ms,
				"failure_reason": "",
				"details": result.get("details", {}),
			}
		else:
			var status: String = (
				ResultsWriter.STATUS_QUARANTINED_FAILURE
				if quarantined else ResultsWriter.STATUS_FAILED
			)
			entry = {
				"id": id, "name": name,
				"status": status,
				"duration_ms": duration_ms,
				"failure_reason": String(result.get("failure_reason", "?")),
				"details": result.get("details", {}),
			}

	_results.add_scenario(entry)
	print("[orch] scenario %s -> %s (%dms)" % [id, entry.status, duration_ms])

	# Optional per-scenario cleanup() hook — runs regardless of pass/fail/timeout
	# per spec/playfab-multiplayer-test-automation/4-scenario-authoring.md:81.
	# Best-effort: swallow null returns.
	if scenario.has_method("cleanup"):
		await scenario.cleanup(self)

	# Mandatory reset between scenarios. Order matters: scenarios that timed
	# out leave their client with a stale in-flight request; the ClientProxy
	# `send(... timeout)` path already marks such a client closed, so the
	# respawn pass below is what restores it. For clients that finished cleanly
	# we just send reset_client and respawn on failure/timeout per
	# spec/playfab-multiplayer-test-automation/3-harness-spec.md:248.
	await _reset_clients_for_scenario(info)


func _run_scenario_call_async(scenario: RefCounted, done_flag: Array, result_slot: Array) -> void:
	var result: Dictionary = await scenario.run(self)
	result_slot[0] = result
	done_flag[0] = true


## Forcibly close every involved client connection so the dangling timed-out
## coroutine's next `send()` returns `client_disconnected` instead of
## continuing to mutate state. The next `_reset_clients_for_scenario` sees
## `is_closed()` and respawns. Mirrors the wire-spec contract at
## spec/playfab-multiplayer-test-automation/3-harness-spec.md (mandatory
## respawn on timeout).
func _invalidate_clients_for_timeout(info: Dictionary, reason: String) -> void:
	var roles: Array = info.get("required_roles", [])
	if roles.is_empty():
		roles = _clients.keys()
	for role_value in roles:
		var role: String = String(role_value)
		var c: ClientProxy = _clients.get(role, null)
		if c == null or c.is_closed():
			continue
		print("[orch] invalidating client role=%s after scenario timeout (%s)" % [role, reason])
		c.disconnect_client()


## Returns true if a capability is satisfied by orchestrator-level topology
## rather than by a per-client handshake bit. Per the matrix at
## spec/playfab-multiplayer-test-automation/1-test-matrix.md:55-60.
func _is_synthetic_capability(cap: String) -> bool:
	return cap == "multi_host_processes" or cap == "multi_machine_eligible"


func _orchestrator_has_capability(cap: String) -> bool:
	match cap:
		"multi_host_processes":
			# Two distinct test-client processes connected (same machine OK).
			return _clients.size() >= 2
		"multi_machine_eligible":
			# Reserved for a future --remote-clients launch mode. Never
			# satisfied today; scenarios that gate on this skip until the
			# mode lands.
			return false
		_:
			return false


## Per-scenario teardown: send reset_client to every involved role; if any
## reset fails or times out (or the client is already lost from a request
## timeout earlier in the scenario), kill + respawn the client before the next
## scenario runs. Implements
## spec/playfab-multiplayer-test-automation/3-harness-spec.md:224-248 and the
## mandatory respawn at 3-harness-spec.md:187.
func _reset_clients_for_scenario(info: Dictionary) -> void:
	# Default to every connected role if the scenario didn't declare
	# required_roles (e.g. _smoke). reset_client is cheap and idempotent.
	var roles: Array = info.get("required_roles", [])
	if roles.is_empty():
		roles = _clients.keys()

	for role_value in roles:
		var role: String = String(role_value)
		var needs_respawn: bool = false
		var lost_reason: String = ""
		var c: ClientProxy = _clients.get(role, null)
		if c == null:
			# Could happen if a role-required client died mid-scenario or was
			# never spawned. Try respawn so the next scenario has a chance.
			needs_respawn = true
			lost_reason = "no_proxy"
		elif c.is_closed():
			needs_respawn = true
			lost_reason = "proxy_closed_%s" % c.close_reason()
		else:
			# Issue reset_client; on failure, respawn.
			var reset: Dictionary = await c.send("reset_client", {}, RESET_TIMEOUT_MS)
			if not bool(reset.get("ok", false)):
				needs_respawn = true
				var err: Dictionary = reset.get("error", {})
				lost_reason = "reset_failed_%s" % String(err.get("code", "unknown"))

		if needs_respawn:
			if not _spawn_clients:
				print("[orch] role=%s would respawn (%s) but --no-spawn is set; scenarios that need this role will skip" % [role, lost_reason])
				continue
			print("[orch] respawning client role=%s (reason=%s)" % [role, lost_reason])
			var rc: int = await _respawn_client(role)
			if rc != OK:
				printerr("[orch] respawn of role=%s failed (err=%d); subsequent scenarios that need this role will skip" % [role, rc])


# ---------------------------------------------------------------------------
# Shutdown
# ---------------------------------------------------------------------------

func _shutdown_clients() -> void:
	for role in _clients.keys():
		var c: ClientProxy = _clients[role]
		c.send_shutdown("orchestrator_done")
	var deadline_ms: int = Time.get_ticks_msec() + SHUTDOWN_TIMEOUT_MS
	while true:
		var any_alive: bool = false
		for role in _clients.keys():
			var c: ClientProxy = _clients[role]
			if not c.is_closed():
				any_alive = true
				break
		if not any_alive:
			break
		if Time.get_ticks_msec() >= deadline_ms:
			print("[orch] shutdown grace expired; closing remaining sockets")
			for role in _clients.keys():
				(_clients[role] as ClientProxy).disconnect_client()
			break
		_pump_io()
		await _tree.process_frame


func _teardown() -> void:
	if _server != null:
		_server.stop()
		_server = null
	for role in _clients.keys():
		(_clients[role] as ClientProxy).disconnect_client()
	_clients.clear()
	_pending_connections.clear()
	_proxies_by_peer.clear()
