@tool
extends RefCounted
## Verb-oriented packaging facade — one method per CLI verb.
##
## The runner (`addons/godot_gdk_packaging/run.gd`) instantiates this class
## with a freshly resolved config dict from `packaging_config.gd`, then
## dispatches to the right `run_*` method. Every verb returns a
## `PackagingResult` dictionary built by `packaging_result.gd`.
##
## The constructor accepts an optional `toolchain` instance for dependency
## injection (alternate GDK location, custom executor); pass `null` to use
## the real `core/gdk_toolchain.gd`.

const PackagingResult = preload("res://addons/godot_gdk_packaging/core/packaging_result.gd")
const GDKToolchainScript = preload("res://addons/godot_gdk_packaging/core/gdk_toolchain.gd")
const MakePkgExecutorScript = preload("res://addons/godot_gdk_packaging/core/makepkg_executor.gd")
const GameConfigManagerScript = preload("res://addons/godot_gdk_packaging/core/game_config_manager.gd")
const PackagingContentPreparerScript = preload("res://addons/godot_gdk_packaging/core/packaging_content_preparer.gd")
const WdappManagerScript = preload("res://addons/godot_gdk_packaging/core/wdapp_manager.gd")

const _STORE_ASSOCIATION_SWITCH := "/StoreAssociation"

var _toolchain: RefCounted
var _makepkg: RefCounted
var _config_mgr: RefCounted
var _preparer: RefCounted
var _wdapp: RefCounted


func _init(toolchain: RefCounted = null) -> void:
	if toolchain == null:
		_toolchain = GDKToolchainScript.new()
	else:
		_toolchain = toolchain
	_makepkg = MakePkgExecutorScript.new(_toolchain)
	_config_mgr = GameConfigManagerScript.new(_toolchain)
	_preparer = PackagingContentPreparerScript.new(_config_mgr)


func get_toolchain() -> RefCounted:
	return _toolchain


# ── pack ────────────────────────────────────────────────────────────────────

func run_pack(resolved: Dictionary) -> Dictionary:
	var t0: int = Time.get_ticks_msec()
	var verb: String = "pack"
	var missing: String = _missing_required(resolved, ["source_dir", "output_dir"])
	if not missing.is_empty():
		return PackagingResult.fail(verb, missing, PackagingResult.EXIT_USAGE)
	if not _toolchain.is_gdk_available():
		return PackagingResult.fail(verb, "GDK tools not found (set GDK_BIN or install the GDK)",
			PackagingResult.EXIT_CONFIG)

	var source_dir: String = str(resolved["source_dir"])
	var output_dir: String = str(resolved["output_dir"])
	var map_file: String = str(resolved.get("map_file", ""))
	var skip_prepare: bool = bool(resolved.get("no_prepare", false))

	if not DirAccess.dir_exists_absolute(source_dir):
		return PackagingResult.fail(verb, "Source directory does not exist: %s" % source_dir,
			PackagingResult.EXIT_CONFIG)
	DirAccess.make_dir_recursive_absolute(output_dir)

	# Optional content prep.
	if not skip_prepare:
		var prep_logs: Array[String] = []
		var ok: bool = _preparer.ensure_content_dir_ready(source_dir,
			func(message: String) -> void:
				prep_logs.append(message),
			str(resolved.get("config_path", "")))
		if not ok:
			return PackagingResult.fail(verb,
				_content_prep_error("Content prep failed for %s" % source_dir, prep_logs),
				PackagingResult.EXIT_TOOL,
				"", {"phase": "prepare_content", "logs": prep_logs}, "",
				Time.get_ticks_msec() - t0)

	# Auto-genmap when --map-file wasn't supplied.
	if map_file.is_empty():
		map_file = source_dir.path_join("layout.xml")
		var genmap_result: Dictionary = _makepkg.genmap(source_dir, map_file)
		if int(genmap_result.get("exit_code", -1)) != 0:
			return PackagingResult.fail(verb, "Auto-genmap failed",
				PackagingResult.EXIT_TOOL,
				str(genmap_result.get("stderr", "")),
				{"phase": "genmap", "map_file": map_file},
				str(genmap_result.get("stdout", "")),
				Time.get_ticks_msec() - t0)

	var options: Dictionary = _build_makepkg_options(resolved)
	var pack_result: Dictionary = _makepkg.pack(source_dir, map_file, output_dir, options)
	var exit_code: int = int(pack_result.get("exit_code", -1))
	var duration: int = Time.get_ticks_msec() - t0
	if exit_code != 0:
		return PackagingResult.fail(verb,
			"makepkg pack failed (exit %d)" % exit_code,
			PackagingResult.EXIT_TOOL,
			str(pack_result.get("stderr", "")),
			{"map_file": map_file, "output_dir": output_dir, "options": options},
			str(pack_result.get("stdout", "")), duration)
	return PackagingResult.ok(verb,
		"Packed %s → %s" % [source_dir, output_dir],
		{"map_file": map_file, "output_dir": output_dir, "options": options},
		str(pack_result.get("stdout", "")), duration)


func _build_makepkg_options(resolved: Dictionary) -> Dictionary:
	var options: Dictionary = {}
	var content_id: String = str(resolved.get("content_id", ""))
	if not content_id.is_empty():
		options["content_id"] = content_id
	var product_id: String = str(resolved.get("product_id", ""))
	if not product_id.is_empty():
		options["product_id"] = product_id
	var encrypt: String = str(resolved.get("encrypt", "none"))
	var encrypt_key: String = str(resolved.get("encrypt_key", ""))
	if encrypt == "license":
		options["encrypt"] = true
	elif encrypt == "key" and not encrypt_key.is_empty():
		options["encrypt_key"] = encrypt_key
	options["updcompat"] = int(resolved.get("updcompat", 3))
	return options


# ── genmap ──────────────────────────────────────────────────────────────────

func run_genmap(resolved: Dictionary) -> Dictionary:
	var t0: int = Time.get_ticks_msec()
	var verb: String = "genmap"
	var missing: String = _missing_required(resolved, ["source_dir", "map_file"])
	if not missing.is_empty():
		return PackagingResult.fail(verb, missing, PackagingResult.EXIT_USAGE)
	if not _toolchain.is_gdk_available():
		return PackagingResult.fail(verb, "GDK tools not found",
			PackagingResult.EXIT_CONFIG)
	var source_dir: String = str(resolved["source_dir"])
	var map_file: String = str(resolved["map_file"])
	var result: Dictionary = _makepkg.genmap(source_dir, map_file)
	var exit_code: int = int(result.get("exit_code", -1))
	var duration: int = Time.get_ticks_msec() - t0
	if exit_code != 0:
		return PackagingResult.fail(verb, "makepkg genmap failed (exit %d)" % exit_code,
			PackagingResult.EXIT_TOOL,
			str(result.get("stderr", "")),
			{"map_file": map_file},
			str(result.get("stdout", "")), duration)
	return PackagingResult.ok(verb,
		"Generated map at %s" % map_file,
		{"map_file": map_file},
		str(result.get("stdout", "")), duration)


# ── validate ────────────────────────────────────────────────────────────────

func run_validate(resolved: Dictionary) -> Dictionary:
	var t0: int = Time.get_ticks_msec()
	var verb: String = "validate"
	var missing: String = _missing_required(resolved, ["source_dir", "map_file"])
	if not missing.is_empty():
		return PackagingResult.fail(verb, missing, PackagingResult.EXIT_USAGE)
	if not _toolchain.is_gdk_available():
		return PackagingResult.fail(verb, "GDK tools not found",
			PackagingResult.EXIT_CONFIG)
	var source_dir: String = str(resolved["source_dir"])
	var map_file: String = str(resolved["map_file"])
	# makepkg validate requires /pd, rejects destination == source, and refuses
	# to create the directory itself. When the caller omits --output-dir,
	# default to a sibling "validate-out" directory; always ensure it exists.
	var output_dir: String = str(resolved.get("output_dir", ""))
	if output_dir.is_empty():
		var parent: String = source_dir.get_base_dir()
		output_dir = parent.path_join("validate-out") if not parent.is_empty() else "validate-out"
	DirAccess.make_dir_recursive_absolute(output_dir)
	var result: Dictionary = _makepkg.validate(map_file, source_dir, output_dir)
	var exit_code: int = int(result.get("exit_code", -1))
	var duration: int = Time.get_ticks_msec() - t0
	if exit_code != 0:
		return PackagingResult.fail(verb,
			"makepkg validate failed (exit %d)" % exit_code,
			PackagingResult.EXIT_TOOL,
			str(result.get("stderr", "")),
			{"map_file": map_file, "source_dir": source_dir, "output_dir": output_dir},
			str(result.get("stdout", "")), duration)
	return PackagingResult.ok(verb,
		"Validated %s against %s" % [source_dir, map_file],
		{"map_file": map_file, "source_dir": source_dir, "output_dir": output_dir},
		str(result.get("stdout", "")), duration)


# ── prepare_content ─────────────────────────────────────────────────────────

func run_prepare_content(resolved: Dictionary) -> Dictionary:
	var t0: int = Time.get_ticks_msec()
	var verb: String = "prepare_content"
	var missing: String = _missing_required(resolved, ["content_dir"])
	if not missing.is_empty():
		return PackagingResult.fail(verb, missing, PackagingResult.EXIT_USAGE)
	var content_dir: String = str(resolved["content_dir"])
	if not DirAccess.dir_exists_absolute(content_dir):
		return PackagingResult.fail(verb, "Content directory does not exist: %s" % content_dir,
			PackagingResult.EXIT_CONFIG)
	var prep_logs: Array[String] = []
	var ok: bool = _preparer.ensure_content_dir_ready(content_dir,
		func(message: String) -> void:
			prep_logs.append(message),
		str(resolved.get("config_path", "")))
	var duration: int = Time.get_ticks_msec() - t0
	if not ok:
		return PackagingResult.fail(verb, _content_prep_error("Content prep failed", prep_logs),
			PackagingResult.EXIT_TOOL,
			"", {"content_dir": content_dir, "logs": prep_logs}, "", duration)
	return PackagingResult.ok(verb, "Prepared %s" % content_dir,
		{"content_dir": content_dir}, "", duration)


# ── export ──────────────────────────────────────────────────────────────────

func run_export(resolved: Dictionary) -> Dictionary:
	var t0: int = Time.get_ticks_msec()
	var verb: String = "export"
	var missing: String = _missing_required(resolved, ["preset_name", "output_dir"])
	if not missing.is_empty():
		return PackagingResult.fail(verb, missing, PackagingResult.EXIT_USAGE)
	var preset: String = str(resolved["preset_name"])
	var output_dir: String = str(resolved["output_dir"])
	var release: bool = bool(resolved.get("release", false))
	var skip_prepare: bool = bool(resolved.get("no_prepare", false))
	var app_name: String = str(resolved.get("app_name", "game"))

	DirAccess.make_dir_recursive_absolute(output_dir)
	var executable_path: String = output_dir.path_join("%s.exe" % app_name)

	var godot_exe: String = OS.get_executable_path()
	if godot_exe.is_empty():
		return PackagingResult.fail(verb, "Could not determine current Godot executable path",
			PackagingResult.EXIT_CONFIG)
	var export_flag: String = "--export-release" if release else "--export-debug"
	var args: PackedStringArray = PackedStringArray([
		"--headless",
		"--path", str(resolved.get("project_dir", ProjectSettings.globalize_path("res://"))),
		export_flag, preset, executable_path,
	])
	var export_result: Dictionary = _toolchain.execute_tool(godot_exe, args)
	var exit_code: int = int(export_result.get("exit_code", -1))
	if exit_code != 0:
		return PackagingResult.fail(verb,
			"Godot export failed (exit %d)" % exit_code,
			PackagingResult.EXIT_TOOL,
			str(export_result.get("stderr", "")),
			{"preset": preset, "output_dir": output_dir, "executable": executable_path},
			str(export_result.get("stdout", "")),
			Time.get_ticks_msec() - t0)

	if not skip_prepare:
		var prep_logs: Array[String] = []
		var ok: bool = _preparer.ensure_content_dir_ready(output_dir,
			func(message: String) -> void:
				prep_logs.append(message),
			str(resolved.get("config_path", "")))
		if not ok:
			return PackagingResult.fail(verb,
				_content_prep_error("Post-export content prep failed", prep_logs),
				PackagingResult.EXIT_TOOL,
				"", {"phase": "prepare_content", "output_dir": output_dir, "logs": prep_logs},
				"", Time.get_ticks_msec() - t0)
	return PackagingResult.ok(verb,
		"Exported %s → %s" % [preset, executable_path],
		{"preset": preset, "output_dir": output_dir, "executable": executable_path,
			"release": release, "prepared": not skip_prepare},
		str(export_result.get("stdout", "")),
		Time.get_ticks_msec() - t0)


# ── register_loose ──────────────────────────────────────────────────────────

func run_register_loose(resolved: Dictionary) -> Dictionary:
	var t0: int = Time.get_ticks_msec()
	var verb: String = "register_loose"
	var missing: String = _missing_required(resolved, ["content_dir"])
	if not missing.is_empty():
		return PackagingResult.fail(verb, missing, PackagingResult.EXIT_USAGE)
	if not _get_wdapp().is_available():
		return PackagingResult.fail(verb, "wdapp.exe not found", PackagingResult.EXIT_CONFIG)
	var content_dir: String = str(resolved["content_dir"])
	if not DirAccess.dir_exists_absolute(content_dir):
		return PackagingResult.fail(verb, "Content directory does not exist: %s" % content_dir,
			PackagingResult.EXIT_CONFIG)
	var result: Dictionary = _get_wdapp().register_loose(content_dir)
	var duration: int = Time.get_ticks_msec() - t0
	var exit_code: int = int(result.get("exit_code", -1))
	if exit_code != 0:
		return PackagingResult.fail(verb,
			"wdapp register failed (exit %d)" % exit_code,
			PackagingResult.EXIT_TOOL,
			str(result.get("stderr", "")),
			{"content_dir": content_dir},
			str(result.get("stdout", "")), duration)
	return PackagingResult.ok(verb, "Registered loose-files build at %s" % content_dir,
		{"content_dir": content_dir},
		str(result.get("stdout", "")), duration)


# ── install ─────────────────────────────────────────────────────────────────

func run_install(resolved: Dictionary) -> Dictionary:
	var t0: int = Time.get_ticks_msec()
	var verb: String = "install"
	var missing: String = _missing_required(resolved, ["package_path"])
	if not missing.is_empty():
		return PackagingResult.fail(verb, missing, PackagingResult.EXIT_USAGE)
	if not _get_wdapp().is_available():
		return PackagingResult.fail(verb, "wdapp.exe not found", PackagingResult.EXIT_CONFIG)
	var package_path: String = str(resolved["package_path"])
	if not FileAccess.file_exists(package_path):
		return PackagingResult.fail(verb, "Package file does not exist: %s" % package_path,
			PackagingResult.EXIT_CONFIG)
	var result: Dictionary = _get_wdapp().install_package(package_path)
	var duration: int = Time.get_ticks_msec() - t0
	var exit_code: int = int(result.get("exit_code", -1))
	if exit_code != 0:
		return PackagingResult.fail(verb,
			"wdapp install failed (exit %d)" % exit_code,
			PackagingResult.EXIT_TOOL,
			str(result.get("stderr", "")),
			{"package_path": package_path},
			str(result.get("stdout", "")), duration)
	return PackagingResult.ok(verb, "Installed %s" % package_path,
		{"package_path": package_path},
		str(result.get("stdout", "")), duration)


# ── uninstall ───────────────────────────────────────────────────────────────

func run_uninstall(resolved: Dictionary) -> Dictionary:
	var t0: int = Time.get_ticks_msec()
	var verb: String = "uninstall"
	var missing: String = _missing_required(resolved, ["package_name"])
	if not missing.is_empty():
		return PackagingResult.fail(verb, missing, PackagingResult.EXIT_USAGE)
	if not _get_wdapp().is_available():
		return PackagingResult.fail(verb, "wdapp.exe not found", PackagingResult.EXIT_CONFIG)
	var package_name: String = str(resolved["package_name"])
	var result: Dictionary = _get_wdapp().uninstall_package(package_name)
	var duration: int = Time.get_ticks_msec() - t0
	var exit_code: int = int(result.get("exit_code", -1))
	if exit_code != 0:
		return PackagingResult.fail(verb,
			"wdapp uninstall failed (exit %d)" % exit_code,
			PackagingResult.EXIT_TOOL,
			str(result.get("stderr", "")),
			{"package_name": package_name},
			str(result.get("stdout", "")), duration)
	return PackagingResult.ok(verb, "Uninstalled %s" % package_name,
		{"package_name": package_name},
		str(result.get("stdout", "")), duration)


# ── launch ──────────────────────────────────────────────────────────────────

func run_launch(resolved: Dictionary) -> Dictionary:
	var t0: int = Time.get_ticks_msec()
	var verb: String = "launch"
	if not _get_wdapp().is_available():
		return PackagingResult.fail(verb, "wdapp.exe not found", PackagingResult.EXIT_CONFIG)
	var aumid: String = str(resolved.get("aumid", ""))
	if aumid.is_empty():
		# Resolve AUMID by listing registered apps and matching package_name.
		var package_name: String = str(resolved.get("package_name", ""))
		if package_name.is_empty():
			return PackagingResult.fail(verb,
				"Either --aumid or --package-name is required",
				PackagingResult.EXIT_USAGE)
		var listing: Dictionary = _get_wdapp().list_registered_apps()
		for app: Dictionary in listing.get("apps", []):
			if str(app.get("pfn", "")) == package_name:
				aumid = str(app.get("aumid", ""))
				break
		if aumid.is_empty():
			return PackagingResult.fail(verb,
				"Could not find AUMID for package '%s'" % package_name,
				PackagingResult.EXIT_CONFIG)
	var result: Dictionary = _get_wdapp().launch_app(aumid)
	var duration: int = Time.get_ticks_msec() - t0
	var exit_code: int = int(result.get("exit_code", -1))
	if exit_code != 0:
		return PackagingResult.fail(verb,
			"wdapp launch failed (exit %d)" % exit_code,
			PackagingResult.EXIT_TOOL,
			str(result.get("stderr", "")),
			{"aumid": aumid},
			str(result.get("stdout", "")), duration)
	return PackagingResult.ok(verb, "Launched %s" % aumid,
		{"aumid": aumid}, str(result.get("stdout", "")), duration)


# ── terminate ───────────────────────────────────────────────────────────────

func run_terminate(resolved: Dictionary) -> Dictionary:
	var t0: int = Time.get_ticks_msec()
	var verb: String = "terminate"
	var missing: String = _missing_required(resolved, ["package_name"])
	if not missing.is_empty():
		return PackagingResult.fail(verb, missing, PackagingResult.EXIT_USAGE)
	if not _get_wdapp().is_available():
		return PackagingResult.fail(verb, "wdapp.exe not found", PackagingResult.EXIT_CONFIG)
	var package_name: String = str(resolved["package_name"])
	var build_dir: String = str(resolved.get("content_dir", resolved.get("source_dir", "")))
	var result: Dictionary = _get_wdapp().terminate_app(package_name, build_dir)
	var duration: int = Time.get_ticks_msec() - t0
	var exit_code: int = int(result.get("exit_code", -1))
	var terminated_with: String = str(result.get("terminated_with", "wdapp"))
	if exit_code != 0:
		return PackagingResult.fail(verb,
			"terminate failed via %s (exit %d)" % [terminated_with, exit_code],
			PackagingResult.EXIT_TOOL,
			str(result.get("stderr", "")),
			{"package_name": package_name, "terminated_with": terminated_with},
			str(result.get("stdout", "")), duration)
	return PackagingResult.ok(verb,
		"Terminated %s via %s" % [package_name, terminated_with],
		{"package_name": package_name, "terminated_with": terminated_with},
		str(result.get("stdout", "")), duration)


# ── sandbox ─────────────────────────────────────────────────────────────────

func run_sandbox(resolved: Dictionary) -> Dictionary:
	var t0: int = Time.get_ticks_msec()
	var verb: String = "sandbox"
	var sandbox_exe: String = _toolchain.get_sandbox_path()
	if sandbox_exe.is_empty():
		return PackagingResult.fail(verb, "XblPCSandbox.exe not found",
			PackagingResult.EXIT_CONFIG)

	var action: String = str(resolved.get("action", "get"))
	var args: PackedStringArray = PackedStringArray()
	match action:
		"get":
			args = PackedStringArray(["/get"])
		"set":
			var sandbox_id: String = str(resolved.get("sandbox_id", ""))
			if sandbox_id.is_empty():
				return PackagingResult.fail(verb,
					"--sandbox-id is required when --action=set",
					PackagingResult.EXIT_USAGE)
			args = PackedStringArray(["/set", sandbox_id, "/noApps"])
		"retail":
			args = PackedStringArray(["/retail", "/noApps"])
		_:
			return PackagingResult.fail(verb,
				"Unknown --action '%s' (expected get, set, or retail)" % action,
				PackagingResult.EXIT_USAGE)

	var result: Dictionary = _toolchain.execute_tool(sandbox_exe, args)
	var duration: int = Time.get_ticks_msec() - t0
	var exit_code: int = int(result.get("exit_code", -1))
	var stdout: String = str(result.get("stdout", ""))
	if exit_code != 0:
		return PackagingResult.fail(verb,
			"XblPCSandbox %s failed (exit %d)" % [action, exit_code],
			PackagingResult.EXIT_TOOL,
			str(result.get("stderr", "")),
			{"action": action, "args": args},
			stdout, duration)

	var details: Dictionary = {"action": action, "args": args}
	if action == "get":
		var trimmed: String = stdout.strip_edges()
		var idx: int = trimmed.find(":")
		if idx >= 0:
			details["sandbox"] = trimmed.substr(idx + 1).strip_edges()
		else:
			details["sandbox"] = trimmed
	return PackagingResult.ok(verb,
		"Sandbox %s ok" % action, details, stdout, duration)


# ── config_template ─────────────────────────────────────────────────────────

func run_config_template(resolved: Dictionary) -> Dictionary:
	var t0: int = Time.get_ticks_msec()
	var verb: String = "config_template"
	var output: String = str(resolved.get("output", ""))
	if output.is_empty():
		output = _config_mgr.get_config_path()
	var fs_output: String = GameConfigManagerScript.to_filesystem_path(output)
	var overwrite: bool = bool(resolved.get("overwrite", false))
	if FileAccess.file_exists(fs_output) and not overwrite:
		return PackagingResult.fail(verb,
			"%s already exists (pass --overwrite to replace it)" % fs_output,
			PackagingResult.EXIT_CONFIG,
			"", {"output": fs_output})
	if FileAccess.file_exists(fs_output) and overwrite:
		var remove_err: Error = DirAccess.remove_absolute(fs_output)
		if remove_err != OK and FileAccess.file_exists(fs_output):
			return PackagingResult.fail(verb,
				"Failed to remove existing output %s (%s)" % [fs_output, error_string(remove_err)],
				PackagingResult.EXIT_CONFIG,
				"", {"output": fs_output, "err": remove_err})

	var app_name: String = str(resolved.get("app_name", "MyGodotGame"))
	var publisher: String = "CN=" + str(resolved.get("identity_publisher", "Publisher"))
	if publisher == "CN=":
		publisher = "CN=Publisher"
	var err: int = _config_mgr.create_template(app_name, publisher, app_name, output)
	var duration: int = Time.get_ticks_msec() - t0
	if err != OK:
		return PackagingResult.fail(verb,
			"Failed to create template (%s)" % error_string(err),
			PackagingResult.EXIT_TOOL,
			"", {"output": fs_output, "err": err}, "", duration)
	return PackagingResult.ok(verb, "Created template at %s" % fs_output,
		{"output": fs_output}, "", duration)


# ── config_editor ───────────────────────────────────────────────────────────

func run_config_editor(resolved: Dictionary) -> Dictionary:
	var t0: int = Time.get_ticks_msec()
	var verb: String = "config_editor"
	var editor_exe: String = _toolchain.get_game_config_editor_path()
	if editor_exe.is_empty():
		return PackagingResult.fail(verb, "GameConfigEditor.exe not found",
			PackagingResult.EXIT_CONFIG)
	var config_path: String = str(resolved.get("config_path", _config_mgr.get_config_path()))
	if not FileAccess.file_exists(config_path):
		return PackagingResult.fail(verb,
			"MicrosoftGame.config not found at %s (run config_template first)" % config_path,
			PackagingResult.EXIT_CONFIG)
	var pid: int = _toolchain.launch_detached(editor_exe, PackedStringArray([config_path]))
	var duration: int = Time.get_ticks_msec() - t0
	if pid < 0:
		return PackagingResult.fail(verb, "Failed to launch GameConfigEditor",
			PackagingResult.EXIT_TOOL,
			"", {"editor_exe": editor_exe, "config_path": config_path}, "", duration)
	return PackagingResult.ok(verb, "Launched GameConfigEditor (pid %d)" % pid,
		{"editor_exe": editor_exe, "config_path": config_path, "pid": pid},
		"", duration)


# ── store_wizard ────────────────────────────────────────────────────────────

func run_store_wizard(resolved: Dictionary) -> Dictionary:
	var t0: int = Time.get_ticks_msec()
	var verb: String = "store_wizard"
	var editor_exe: String = _toolchain.get_game_config_editor_path()
	if editor_exe.is_empty():
		return PackagingResult.fail(verb, "GameConfigEditor.exe not found",
			PackagingResult.EXIT_CONFIG)
	var config_path: String = str(resolved.get("config_path", _config_mgr.get_config_path()))
	if not FileAccess.file_exists(config_path):
		return PackagingResult.fail(verb,
			"MicrosoftGame.config not found at %s (run config_template first)" % config_path,
			PackagingResult.EXIT_CONFIG)
	# GameConfigEditor in recent GDK editions hosts the Store Association wizard
	# via the /StoreAssociation switch. Older builds silently ignore the switch
	# and open the editor's main pane; either outcome counts as a successful
	# detached launch.
	var pid: int = _toolchain.launch_detached(editor_exe,
		PackedStringArray([config_path, _STORE_ASSOCIATION_SWITCH]))
	var duration: int = Time.get_ticks_msec() - t0
	if pid < 0:
		return PackagingResult.fail(verb,
			"Failed to launch GameConfigEditor for store association",
			PackagingResult.EXIT_TOOL,
			"", {"editor_exe": editor_exe, "config_path": config_path}, "", duration)
	return PackagingResult.ok(verb,
		"Launched GameConfigEditor /StoreAssociation (pid %d)" % pid,
		{"editor_exe": editor_exe, "config_path": config_path, "pid": pid,
			"switch": _STORE_ASSOCIATION_SWITCH},
		"", duration)


# ── dispatch ────────────────────────────────────────────────────────────────

## Returns the verb method name for [param verb] (e.g. "pack" → "run_pack").
## Empty string if the verb is unknown.
func method_for_verb(verb: String) -> String:
	var candidate: String = "run_" + verb
	if has_method(candidate):
		return candidate
	return ""


## Convenience: looks up the method by verb name and calls it with [param resolved].
## Returns a [code]PackagingResult[/code] regardless of outcome.
func dispatch(verb: String, resolved: Dictionary) -> Dictionary:
	var method: String = method_for_verb(verb)
	if method.is_empty():
		return PackagingResult.fail(verb, "Verb not implemented: %s" % verb,
			PackagingResult.EXIT_UNIMPLEMENTED)
	return call(method, resolved)


# ── internal ────────────────────────────────────────────────────────────────

func _get_wdapp() -> RefCounted:
	if _wdapp == null:
		_wdapp = WdappManagerScript.new(_toolchain)
	return _wdapp


static func _missing_required(resolved: Dictionary, keys: Array) -> String:
	var missing: PackedStringArray = PackedStringArray()
	for key: String in keys:
		var value: Variant = resolved.get(key, "")
		if typeof(value) == TYPE_STRING and (value as String).is_empty():
			missing.append(key)
	if missing.is_empty():
		return ""
	return "Missing required value(s): %s" % ", ".join(missing)


static func _content_prep_error(prefix: String, logs: Array[String]) -> String:
	if logs.is_empty():
		return prefix
	var parts: PackedStringArray = PackedStringArray()
	for log: String in logs:
		parts.append(log)
	return "%s: %s" % [prefix, "; ".join(parts)]
