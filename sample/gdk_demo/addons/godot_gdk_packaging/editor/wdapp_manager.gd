@tool
extends RefCounted
## Wraps wdapp.exe operations for registration, install, launch, and terminate.

const GDKToolchainScript = preload("res://addons/godot_gdk_packaging/editor/gdk_toolchain.gd")

var _toolchain: RefCounted


func _init(toolchain: RefCounted) -> void:
	_toolchain = toolchain


func is_available() -> bool:
	return FileAccess.file_exists(_get_wdapp_path())


func list_registered_apps() -> Dictionary:
	var wdapp_path = _get_wdapp_path()
	if not FileAccess.file_exists(wdapp_path):
		return {
			"exit_code": -1,
			"stdout": "",
			"stderr": "wdapp.exe not found",
			"apps": [],
		}

	var result = _toolchain.execute_tool(wdapp_path, PackedStringArray(["list"]))
	result["apps"] = parse_registered_apps(result["stdout"])
	return result


func register_loose(build_dir: String) -> Dictionary:
	return _toolchain.execute_tool(_get_wdapp_path(), PackedStringArray(["register", build_dir]))


func launch_app(aumid: String) -> Dictionary:
	return _toolchain.execute_tool(_get_wdapp_path(), PackedStringArray(["launch", aumid]))


func terminate_app(pfn: String, build_dir: String) -> Dictionary:
	var result = _toolchain.execute_tool(_get_wdapp_path(), PackedStringArray(["terminate", pfn]))
	if result["exit_code"] == 0:
		result["terminated_with"] = "wdapp"
		return result

	var exe_name = _find_primary_executable(build_dir)
	if exe_name == "":
		result["terminated_with"] = "wdapp"
		return result

	var output: Array = []
	var exit_code = OS.execute("taskkill", PackedStringArray(["/IM", exe_name, "/F"]), output, true, false)
	return {
		"exit_code": exit_code,
		"stdout": str(output[0]) if output.size() > 0 else "",
		"stderr": "",
		"terminated_with": "taskkill",
	}


func install_package(msixvc_path: String) -> Dictionary:
	return _toolchain.execute_tool(_get_wdapp_path(), PackedStringArray(["install", msixvc_path]))


func uninstall_package(package_full_name: String) -> Dictionary:
	return _toolchain.execute_tool(_get_wdapp_path(), PackedStringArray(["uninstall", package_full_name]))


static func parse_registered_apps(output: String) -> Array[Dictionary]:
	var apps: Array[Dictionary] = []
	var current_pfn := ""
	for line in output.split("\n"):
		var trimmed = line.strip_edges()
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


func _find_primary_executable(build_dir: String) -> String:
	if not DirAccess.dir_exists_absolute(build_dir):
		return ""

	var dir = DirAccess.open(build_dir)
	if dir == null:
		return ""

	dir.list_dir_begin()
	var fname = dir.get_next()
	while fname != "":
		if fname.ends_with(".exe") and not fname.ends_with(".console.exe"):
			dir.list_dir_end()
			return fname
		fname = dir.get_next()
	dir.list_dir_end()
	return ""
