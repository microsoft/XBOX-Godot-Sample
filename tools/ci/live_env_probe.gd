extends SceneTree

# Live environment preflight probe.
#
# The live and live-write test tiers are mandatory (see tests/godot/README.md).
# `tools\run_all_tests.ps1` validates the *configuration* (PlayFab title id,
# custom id, matchmaking queue) before any stage runs, but configuration alone
# does not prove the machine can actually exercise live coverage: the GDK
# runtime may be absent, no Xbox identity may be signed in, and no gamepad may
# be attached. Without this probe those conditions surface only as hundreds of
# individual in-test failures, which is slow and hard to read.
#
# This script asserts the required runtime capabilities up front so a machine
# that cannot exercise the live tiers fails fast with one actionable message.
#
# Usage:
#   godot --headless -s res://tools/ci/live_env_probe.gd -- <cap> [cap ...]
#
# Capabilities:
#   gdk        GDK singleton present and GDK.initialize() succeeds.
#   xuser      A primary Xbox user is available (silent sign-in is attempted).
#   playfab    PlayFab singleton present and a title id is configured.
#   gameinput  GameInput singleton present and GameInput.initialize() succeeds.
#   gamepad    At least one GameInput gamepad device is connected.
#
# Exits 0 when every requested capability is satisfied, 1 when any is missing,
# 2 on misuse.

const BUDGET_MSEC := 30000
const POLL_INTERVAL_MSEC := 50
const DEVICE_GAMEPAD := 1

var _required := PackedStringArray()
var _failures: Array[String] = []
var _notes: Array[String] = []
var _deadline_msec := 0
var _signin_state: Dictionary = {}
var _signin_started := false
var _finished := false
var _exit_code := 0


func _initialize() -> void:
	_required = PackedStringArray(OS.get_cmdline_user_args())
	if _required.is_empty():
		printerr("[live-probe] No capabilities requested (pass them after `--`).")
		_finish(2)
		return

	print("[live-probe] Required capabilities: %s" % ", ".join(_required))
	_deadline_msec = Time.get_ticks_msec() + BUDGET_MSEC

	if _needs("gdk") or _needs("xuser"):
		_probe_gdk()
	if _needs("playfab"):
		_probe_playfab()
	if _needs("gameinput") or _needs("gamepad"):
		_probe_gameinput()


func _process(_delta: float) -> bool:
	if _finished:
		return true

	# The only capability that needs to settle asynchronously is `xuser`:
	# silent sign-in is dispatched through the GDK async queue.
	if not _needs("xuser"):
		_evaluate()
		return true

	var gdk := _get_singleton("GDK")
	if gdk != null:
		gdk.dispatch()

	if _signin_state.get("completed", false):
		_evaluate()
		return true

	if _primary_user() != null:
		_evaluate()
		return true

	if Time.get_ticks_msec() >= _deadline_msec:
		_notes.append("Xbox sign-in did not settle within %d ms." % BUDGET_MSEC)
		_evaluate()
		return true

	OS.delay_msec(POLL_INTERVAL_MSEC)
	return false


# ── Capability probes ────────────────────────────────────────────────────

func _probe_gdk() -> void:
	var gdk := _get_singleton("GDK")
	if gdk == null:
		_failures.append("gdk: the GDK singleton is not registered in this host. The godot_gdk GDExtension failed to load -- check that the addon DLL and the GDK redist DLLs are present.")
		return

	if not gdk.is_initialized():
		var result: Variant = gdk.initialize()
		if result != null and not result.ok:
			_failures.append("gdk: GDK.initialize() failed (%s: %s). The Microsoft GDK runtime is not usable on this machine." % [result.code, result.message])
			return

	if not gdk.is_initialized():
		_failures.append("gdk: GDK.initialize() did not bring the runtime up. The Microsoft GDK runtime is not usable on this machine.")
		return

	_notes.append("gdk: runtime initialized.")

	if _needs("xuser"):
		_start_signin(gdk)


func _start_signin(gdk: Object) -> void:
	if _primary_user() != null:
		return

	var users: Object = gdk.get_users()
	if users == null:
		_failures.append("xuser: GDK.users service is unavailable, so no Xbox identity can be resolved.")
		return

	var completion: Variant = users.add_default_user_async()
	if typeof(completion) != TYPE_SIGNAL:
		_failures.append("xuser: GDK.users.add_default_user_async() did not return a signal, so silent sign-in could not be started.")
		return

	_signin_started = true
	_signin_state = {"completed": false, "result": null}
	completion.connect(func(result: Variant) -> void:
		_signin_state["completed"] = true
		_signin_state["result"] = result,
		CONNECT_ONE_SHOT)


func _probe_playfab() -> void:
	var playfab := _get_singleton("PlayFab")
	if playfab == null:
		_failures.append("playfab: the PlayFab singleton is not registered in this host. The godot_playfab GDExtension failed to load.")
		return

	var title_id := str(ProjectSettings.get_setting("playfab/runtime/title_id", ""))
	if title_id.is_empty():
		title_id = OS.get_environment("PLAYFAB_TITLE_ID")
	if title_id.is_empty():
		_failures.append("playfab: no title id is configured. Pass -PlayFabTitleId to tools\\run_all_tests.ps1 or set PLAYFAB_TITLE_ID.")
		return

	_notes.append("playfab: singleton present, title id '%s'." % title_id)


func _probe_gameinput() -> void:
	var gi := _get_singleton("GameInput")
	if gi == null:
		_failures.append("gameinput: the GameInput singleton is not registered in this host. The godot_gameinput GDExtension failed to load.")
		return

	if not gi.initialize():
		_failures.append("gameinput: GameInput.initialize() returned false. The GameInput runtime is not available on this machine.")
		return

	_notes.append("gameinput: runtime initialized.")

	if not _needs("gamepad"):
		return

	# Device enumeration is edge-driven; poll a few times so a connected pad
	# is observed before we judge it absent.
	var device: Variant = null
	for _i in range(10):
		gi.poll()
		device = gi.get_primary_device(DEVICE_GAMEPAD)
		if device != null:
			break
		OS.delay_msec(POLL_INTERVAL_MSEC)

	if device == null:
		_failures.append("gamepad: no GameInput gamepad is connected. Attach a controller -- the GameInput live tier cannot be exercised without one.")
		return

	_notes.append("gamepad: a gamepad device is connected.")


# ── Helpers ──────────────────────────────────────────────────────────────

func _needs(capability: String) -> bool:
	return _required.has(capability)


func _get_singleton(name: String) -> Object:
	if Engine.has_singleton(name):
		return Engine.get_singleton(name)
	return null


func _primary_user() -> Variant:
	var gdk := _get_singleton("GDK")
	if gdk == null:
		return null
	var users: Object = gdk.get_users()
	if users == null:
		return null
	return users.get_primary_user()


func _evaluate() -> void:
	if _needs("xuser") and not _has_xuser_failure():
		var user: Variant = _primary_user()
		if user == null:
			var detail := ""
			var result: Variant = _signin_state.get("result")
			if result != null and not result.ok:
				detail = " Silent sign-in returned %s: %s." % [result.code, result.message]
			elif _signin_started:
				detail = " Silent sign-in never completed."
			_failures.append("xuser: no signed-in Xbox user is available on this machine.%s Sign in to the Xbox app / Gaming Services before running the live tier." % detail)
		else:
			_notes.append("xuser: signed in as '%s'." % str(user.get_gamertag()))

	for note in _notes:
		print("[live-probe] OK   %s" % note)

	if _failures.is_empty():
		print("[live-probe] All required capabilities are available.")
		_finish(0)
		return

	printerr("")
	printerr("[live-probe] === LIVE ENVIRONMENT PREFLIGHT FAILED ===")
	printerr("[live-probe] The live and live-write tiers are mandatory and cannot be skipped,")
	printerr("[live-probe] so a machine that cannot exercise them must fail rather than report")
	printerr("[live-probe] a green tick for coverage it never ran.")
	printerr("")
	for failure in _failures:
		printerr("[live-probe]   * %s" % failure)
	printerr("")
	_finish(1)


func _has_xuser_failure() -> bool:
	for failure in _failures:
		if failure.begins_with("xuser:") or failure.begins_with("gdk:"):
			return true
	return false


func _finish(code: int) -> void:
	_finished = true
	_exit_code = code
	quit(code)
