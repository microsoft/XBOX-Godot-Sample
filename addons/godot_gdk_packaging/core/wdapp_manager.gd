@tool
extends RefCounted
## Wraps wdapp.exe operations for registration, install, launch, and terminate.
##
## Provides both:
##   * Sync methods (existing API) -- safe for headless / CLI use via
##     [code]packaging_service.gd[/code]; called inline by the editor tabs.
##   * Async variants for [code]list_registered_apps[/code],
##     [code]install_package[/code], and [code]uninstall_package[/code] that
##     spawn a [Thread] and emit completion signals on the main thread via
##     [method Object.call_deferred]. Async is intended for editor-UI
##     consumers (e.g. the Package Manager dialog) so the editor does not
##     freeze for the seconds an MSIXVC install can take.
##
## Async lifecycle:
##   1. Connect to [signal list_completed] / [signal install_completed] /
##      [signal uninstall_completed] once at setup time.
##   2. Call [method list_registered_apps_async], [method install_package_async],
##      or [method uninstall_package_async]. Returns [code]false[/code] if a
##      previous async op is still in flight (only one wdapp call at a time
##      per manager) or if the manager has been disposed.
##   3. The handler receives the same Dictionary shape the sync method would
##      return: [code]{exit_code, stdout, stderr, ...}[/code].
##   4. Call [method dispose] before dropping the last reference. The Package
##      Manager dialog does this from [code]NOTIFICATION_PREDELETE[/code]. The
##      manager also disposes itself from its own [code]NOTIFICATION_PREDELETE[/code]
##      as a safety net, but explicit disposal makes ownership clearer and
##      prevents a destructor-time block.
##
## Thread safety: [member _toolchain] is immutable after construction (paths
## are populated once in [code]_detect_gdk()[/code]). Reading them from a
## worker thread is safe. The Dictionary result is constructed in the worker
## and marshalled to the main thread via [method Object.call_deferred].

signal list_completed(result: Dictionary)
signal install_completed(result: Dictionary)
signal uninstall_completed(result: Dictionary)

const GDKToolchainScript = preload("res://addons/godot_gdk_packaging/core/gdk_toolchain.gd")

var _toolchain: RefCounted
var _taskkill_runner: Callable

# Async dispatch state. _busy / _disposed are only written from the main
# thread (in _start_async / _join_thread / dispose). The worker thread only
# calls the sync methods and posts a deferred result; it does not read or
# write these flags.
var _thread: Thread = null
var _busy: bool = false
var _disposed: bool = false


func _init(toolchain: RefCounted, taskkill_runner: Callable = Callable()) -> void:
	_toolchain = toolchain
	_taskkill_runner = taskkill_runner


func is_available() -> bool:
	return FileAccess.file_exists(_get_wdapp_path())


func list_registered_apps() -> Dictionary:
	var wdapp_path: String = _get_wdapp_path()
	if not FileAccess.file_exists(wdapp_path):
		return {
			"exit_code": -1,
			"stdout": "",
			"stderr": "wdapp.exe not found",
			"apps": [],
		}

	var result: Dictionary = _toolchain.execute_tool(wdapp_path, PackedStringArray(["list"]))
	result["apps"] = parse_registered_apps(result["stdout"])
	return result


func register_loose(build_dir: String) -> Dictionary:
	return _toolchain.execute_tool(_get_wdapp_path(), PackedStringArray(["register", build_dir]))


func launch_app(aumid: String) -> Dictionary:
	return _toolchain.execute_tool(_get_wdapp_path(), PackedStringArray(["launch", aumid]))


func terminate_app(pfn: String, build_dir: String) -> Dictionary:
	var result: Dictionary = _toolchain.execute_tool(_get_wdapp_path(), PackedStringArray(["terminate", pfn]))
	if result["exit_code"] == 0:
		result["terminated_with"] = "wdapp"
		return result

	var exe_name: String = _resolve_taskkill_executable(build_dir)
	if exe_name == "":
		result["terminated_with"] = "wdapp"
		result["taskkill_skipped_reason"] = "no configured executable available"
		return result

	return _execute_taskkill(exe_name)


func install_package(msixvc_path: String) -> Dictionary:
	return _toolchain.execute_tool(_get_wdapp_path(), PackedStringArray(["install", msixvc_path]))


func uninstall_package(package_full_name: String) -> Dictionary:
	return _toolchain.execute_tool(_get_wdapp_path(), PackedStringArray(["uninstall", package_full_name]))


# --- Async API -------------------------------------------------------------

## Returns true while a *_async dispatch is in flight.
func is_busy() -> bool:
	return _busy


## Returns true after dispose() has been called. Disposed managers reject
## new *_async dispatches and skip emitting signals from any in-flight op.
func is_disposed() -> bool:
	return _disposed


## Releases the worker thread and prevents further async dispatches. Idempotent.
## Blocks until any in-flight wdapp operation completes (can be many seconds
## for install) -- preferable to leaking a live Godot Thread on shutdown.
func dispose() -> void:
	if _disposed:
		return
	_disposed = true
	if _thread != null and _thread.is_started():
		push_warning("[GDK Packaging] Waiting for in-flight wdapp operation to finish before shutdown...")
		_thread.wait_to_finish()
		_thread = null
	_busy = false


func list_registered_apps_async() -> bool:
	return _start_async(_run_list, [])


func install_package_async(msixvc_path: String) -> bool:
	return _start_async(_run_install, [msixvc_path])


func uninstall_package_async(package_full_name: String) -> bool:
	return _start_async(_run_uninstall, [package_full_name])


# Starts a worker thread that calls `target(*args)`. Returns true on success,
# false if already busy / disposed / Thread.start failed. _busy is only set
# after a successful start so a failed dispatch leaves the manager idle.
func _start_async(target: Callable, args: Array) -> bool:
	if _disposed or _busy:
		return false
	var thread: Thread = Thread.new()
	var err: int = thread.start(target.bindv(args))
	if err != OK:
		push_error("[GDK Packaging] Thread.start failed for wdapp async dispatch (err %d)" % err)
		return false
	_thread = thread
	_busy = true
	return true


func _run_list() -> void:
	var result: Dictionary = list_registered_apps()
	_finish_list.call_deferred(result)


func _finish_list(result: Dictionary) -> void:
	_join_thread()
	if _disposed:
		return
	list_completed.emit(result)


func _run_install(msixvc_path: String) -> void:
	var result: Dictionary = install_package(msixvc_path)
	_finish_install.call_deferred(result)


func _finish_install(result: Dictionary) -> void:
	_join_thread()
	if _disposed:
		return
	install_completed.emit(result)


func _run_uninstall(package_full_name: String) -> void:
	var result: Dictionary = uninstall_package(package_full_name)
	_finish_uninstall.call_deferred(result)


func _finish_uninstall(result: Dictionary) -> void:
	_join_thread()
	if _disposed:
		return
	uninstall_completed.emit(result)


# Called on the main thread from each _finish_* handler. The worker has
# already returned by the time the deferred call runs (it does nothing after
# call_deferred), so wait_to_finish() returns immediately in practice.
func _join_thread() -> void:
	if _thread != null:
		_thread.wait_to_finish()
		_thread = null
	_busy = false


# Safety net -- if a caller drops the manager without calling dispose(),
# still join the worker before the RefCounted destructor runs to avoid
# leaking a live Godot Thread.
func _notification(what: int) -> void:
	if what != NOTIFICATION_PREDELETE or _disposed:
		return
	_disposed = true
	if _thread != null and _thread.is_started():
		_thread.wait_to_finish()
		_thread = null
	_busy = false


# --- Static helpers --------------------------------------------------------

static func parse_registered_apps(output: String) -> Array[Dictionary]:
	var apps: Array[Dictionary] = []
	var current_pfn: String = ""
	for line: String in output.split("\n"):
		var trimmed: String = line.strip_edges()
		if trimmed == "" or trimmed.begins_with("Run this") or trimmed.begins_with("The operation") or trimmed.begins_with("Registered"):
			continue
		if trimmed.contains("!"):
			if current_pfn != "":
				apps.append({"pfn": current_pfn, "aumid": trimmed})
		elif trimmed.contains("_"):
			current_pfn = trimmed
	return apps


func _get_wdapp_path() -> String:
	return _toolchain.get_bin_dir().path_join("wdapp.exe")


func _execute_taskkill(exe_name: String) -> Dictionary:
	if _taskkill_runner.is_valid():
		var custom_result: Variant = _taskkill_runner.call(exe_name)
		if typeof(custom_result) == TYPE_DICTIONARY:
			return custom_result as Dictionary
		return {"exit_code": 1, "stdout": "", "stderr": "taskkill runner did not return a Dictionary", "terminated_with": "taskkill"}

	var output: Array = []
	var exit_code: int = OS.execute("taskkill", PackedStringArray(["/IM", exe_name, "/F"]), output, true, false)
	return {
		"exit_code": exit_code,
		"stdout": "\n".join(output),
		"stderr": "",
		"terminated_with": "taskkill",
	}


static func _resolve_taskkill_executable(build_dir: String) -> String:
	if build_dir.is_empty() or not DirAccess.dir_exists_absolute(build_dir):
		return ""

	var exe_name: String = _read_config_executable(build_dir.path_join("MicrosoftGame.config"))
	if not _is_safe_taskkill_image_name(exe_name):
		return ""
	if not FileAccess.file_exists(build_dir.path_join(exe_name)):
		return ""
	return exe_name


static func _read_config_executable(config_path: String) -> String:
	if config_path.is_empty() or not FileAccess.file_exists(config_path):
		return ""

	var parser := XMLParser.new()
	if parser.open(config_path) != OK:
		return ""

	while parser.read() == OK:
		if parser.get_node_type() != XMLParser.NODE_ELEMENT or parser.get_node_name() != "Executable":
			continue
		for index: int in parser.get_attribute_count():
			if parser.get_attribute_name(index) == "Name":
				return parser.get_attribute_value(index).strip_edges()
	return ""


static func _is_safe_taskkill_image_name(exe_name: String) -> bool:
	if exe_name.is_empty() or exe_name != exe_name.get_file():
		return false
	if not exe_name.to_lower().ends_with(".exe"):
		return false
	for forbidden: String in ["\\", "/", ":", "*", "?", "\"", "<", ">", "|"]:
		if exe_name.contains(forbidden):
			return false
	return true
