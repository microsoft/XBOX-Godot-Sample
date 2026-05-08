@tool
extends RefCounted
## Discovers GDK tool paths and executes external processes.
##
## Resolution order for the GDK bin directory:
##   1. GDK_BIN environment variable
##   2. Default install path: C:\Program Files (x86)\Microsoft GDK\bin

const _DEFAULT_GDK_BIN := "C:/Program Files (x86)/Microsoft GDK/bin"

var _bin_dir: String = ""
var _makepkg_path: String = ""
var _game_config_editor_path: String = ""
var _sandbox_path: String = ""
var _dev_account_path: String = ""
var _gdk_version: String = ""
var _is_available: bool = false

func _init() -> void:
	_detect_gdk()


# ── Public API ──────────────────────────────────────────────────────────────

func is_gdk_available() -> bool:
	return _is_available

func get_makepkg_path() -> String:
	return _makepkg_path

func get_game_config_editor_path() -> String:
	return _game_config_editor_path

func get_sandbox_path() -> String:
	return _sandbox_path

func get_dev_account_path() -> String:
	return _dev_account_path

func get_gdk_version() -> String:
	return _gdk_version

func get_bin_dir() -> String:
	return _bin_dir


## Runs an executable synchronously and returns a dictionary:
##   { "exit_code": int, "stdout": String, "stderr": String }
func execute_tool(exe_path: String, args: PackedStringArray) -> Dictionary:
	if not FileAccess.file_exists(exe_path):
		return { "exit_code": -1, "stdout": "", "stderr": "Tool not found: " + exe_path }

	var output: Array = []
	# read_stderr=false: Godot merges stderr into stdout in output[0]
	var exit_code: int = OS.execute(exe_path, args, output, true, false)

	var stdout_text: String = ""
	if output.size() > 0:
		stdout_text = str(output[0])

	return {
		"exit_code": exit_code,
		"stdout": stdout_text,
		"stderr": ""  # stderr is merged into stdout by OS.execute
	}


## Launches an executable as a detached process (fire-and-forget).
## Returns the PID, or -1 on failure.
func launch_detached(exe_path: String, args: PackedStringArray) -> int:
	if not FileAccess.file_exists(exe_path):
		push_error("[GDK Packaging] Tool not found: " + exe_path)
		return -1
	return OS.create_process(exe_path, args)


# ── Private ─────────────────────────────────────────────────────────────────

## Detects GDK installation: extracts version from GameDKCoreLatest env var
## (e.g. "C:\Program Files (x86)\Microsoft GDK\260400\" → "260400"),
## then resolves the bin directory from GDK_BIN env var or default path.
func _detect_gdk() -> void:
	# GameDKCoreLatest is set by the GDK installer, e.g. "C:\...\Microsoft GDK\260400\"
	# Extract the 6-digit edition number from the path segments
	var gdk_core: String = OS.get_environment("GameDKCoreLatest")
	if gdk_core != "":
		var parts: PackedStringArray = gdk_core.replace("\\", "/").split("/")
		for part: String in parts:
			if part.length() == 6 and part.is_valid_int():
				_gdk_version = part
				break

	# 1. Check GDK_BIN env var (user override)
	var env_bin: String = OS.get_environment("GDK_BIN")
	if env_bin != "" and DirAccess.dir_exists_absolute(env_bin):
		_try_bin_dir(env_bin)
		if _is_available:
			return

	# 2. Default install path
	_try_bin_dir(_DEFAULT_GDK_BIN)

## Validates a bin directory by checking for required tools (makepkg, GameConfigEditor)
## and optional tools (XblPCSandbox, XblDevAccount). Sets paths and _is_available.
func _try_bin_dir(dir: String) -> void:
	var makepkg: String = dir.path_join("makepkg.exe")
	var config_editor: String = dir.path_join("GameConfigEditor.exe")
	var sandbox: String = dir.path_join("XblPCSandbox.exe")

	if FileAccess.file_exists(makepkg) and FileAccess.file_exists(config_editor):
		_bin_dir = dir
		_makepkg_path = makepkg
		_game_config_editor_path = config_editor
		if FileAccess.file_exists(sandbox):
			_sandbox_path = sandbox
		var dev_account: String = dir.path_join("XblDevAccount.exe")
		if FileAccess.file_exists(dev_account):
			_dev_account_path = dev_account
		_is_available = true
		print("[GDK Packaging] GDK tools found at: ", dir)
	else:
		if not FileAccess.file_exists(makepkg):
			push_warning("[GDK Packaging] makepkg.exe not found in: " + dir)
		if not FileAccess.file_exists(config_editor):
			push_warning("[GDK Packaging] GameConfigEditor.exe not found in: " + dir)
