@tool
extends RefCounted
## Copies config and logo assets into a packaging content directory.

const GameConfigManagerScript = preload("res://addons/godot_gdk_packaging/core/game_config_manager.gd")

var _config_mgr: RefCounted


func _init(config_mgr: RefCounted) -> void:
	_config_mgr = config_mgr


func ensure_content_dir_ready(content_dir: String, logger: Callable = Callable(),
		config_path: String = "") -> bool:
	var project_dir: String = ProjectSettings.globalize_path("res://")
	var config_src: String = config_path
	if config_src.is_empty():
		config_src = _config_mgr.get_config_path()
	var config_dest: String = content_dir.path_join("MicrosoftGame.config")

	if not FileAccess.file_exists(config_src):
		_call_logger(logger, "❌ MicrosoftGame.config not found at %s." % config_src)
		return false

	var info: Dictionary = _config_mgr.parse_config(config_src)
	var logo_keys: Dictionary = {
		"store_logo": "StoreLogo",
		"logo_150": "Square150x150Logo",
		"logo_44": "Square44x44Logo",
		"logo_480": "Square480x480Logo",
		"splash_screen": "SplashScreenImage",
	}
	var logo_destinations: Dictionary = {}
	for key: String in logo_keys:
		var rel_path: String = str(info.get(key, ""))
		if rel_path.is_empty():
			rel_path = str(logo_keys[key]) + ".png"
		var dest_path: String = _resolve_logo_destination(content_dir, rel_path)
		if dest_path.is_empty():
			_call_logger(logger,
				"❌ Refusing logo path outside content directory for %s: %s" % [
					str(logo_keys[key]), rel_path,
				])
			return false
		logo_destinations[key] = {
			"dest_path": dest_path,
			"normalized": _normalize_logo_relative_path(rel_path),
		}

	var file: FileAccess = FileAccess.open(config_src, FileAccess.READ)
	if file == null:
		_call_logger(logger, "❌ Cannot read MicrosoftGame.config at %s" % config_src)
		return false
	var content: String = file.get_as_text()
	file.close()

	content = inject_vc14_dependency(content)

	var executable_name: String = _find_primary_executable(content_dir)
	if executable_name != "":
		content = patch_executable_name(content, executable_name)
		_call_logger(logger, "Patched executable name to: %s" % executable_name)

	file = FileAccess.open(config_dest, FileAccess.WRITE)
	if file == null:
		_call_logger(logger, "❌ Cannot write to content directory")
		return false
	file.store_string(content)
	file.close()
	_call_logger(logger, "Copied MicrosoftGame.config to content directory")

	for key: String in logo_keys:
		var destination: Dictionary = logo_destinations[key]
		var normalized: String = str(destination["normalized"])
		var dest_path: String = str(destination["dest_path"])

		var src_path: String = ""
		var filename: String = normalized.get_file()
		var storelogos_src: String = project_dir.path_join("storelogos").path_join(filename)
		var root_src: String = project_dir.path_join(filename)
		if FileAccess.file_exists(storelogos_src):
			src_path = storelogos_src
		elif FileAccess.file_exists(root_src):
			src_path = root_src

		if src_path != "":
			var dest_dir: String = dest_path.get_base_dir()
			DirAccess.make_dir_recursive_absolute(dest_dir)
			var copy_dir: DirAccess = DirAccess.open(project_dir)
			if copy_dir != null:
				copy_dir.copy(src_path, dest_path)

	var dll_count: int = _copy_addon_runtime_dlls(content_dir, logger)
	if dll_count > 0:
		_call_logger(logger, "Copied %d addon runtime DLL(s)" % dll_count)

	return true


## Copies redistributable runtime DLLs that live alongside each addon's main
## GDExtension binary (under [code]addons/<name>/bin/[/code]) into the export
## content directory. Godot's Windows Desktop export only places the main
## library DLL referenced by each [code].gdextension[/code]; the sibling
## support DLLs (e.g. PlayFabCore.dll, Microsoft.Xbox.Services.C.Thunks.dll,
## libHttpClient.dll) must be staged next to the .exe by the packaging step
## or the GDExtension fails to load at runtime with Win32 error 126.
##
## Files matching the GDExtension main-library pattern
## [code]godot_*.windows.<config>.x86_64.dll[/code] are skipped — they are
## either already placed by Godot (correct config) or belong to the opposite
## build config and must not leak into the package.
##
## Returns the number of DLLs copied.
func _copy_addon_runtime_dlls(content_dir: String, logger: Callable) -> int:
	var project_dir: String = ProjectSettings.globalize_path("res://")
	var addons_dir: String = project_dir.path_join("addons")
	if not DirAccess.dir_exists_absolute(addons_dir):
		return 0

	var skip_re: RegEx = RegEx.new()
	skip_re.compile("^godot_.*\\.windows\\..*\\.x86_64\\.dll$")

	var copy_dir: DirAccess = DirAccess.open(project_dir)
	if copy_dir == null:
		return 0

	var copied: int = 0
	var addons: DirAccess = DirAccess.open(addons_dir)
	if addons == null:
		return 0
	addons.list_dir_begin()
	var addon_name: String = addons.get_next()
	while addon_name != "":
		if addons.current_is_dir() and not addon_name.begins_with("."):
			var bin_dir: String = addons_dir.path_join(addon_name).path_join("bin")
			if DirAccess.dir_exists_absolute(bin_dir):
				var bin: DirAccess = DirAccess.open(bin_dir)
				if bin != null:
					bin.list_dir_begin()
					var fname: String = bin.get_next()
					while fname != "":
						if not bin.current_is_dir() and fname.ends_with(".dll") and skip_re.search(fname) == null:
							var src: String = bin_dir.path_join(fname)
							var dest: String = content_dir.path_join(fname)
							if _should_copy_runtime_dll(src, dest):
								if copy_dir.copy(src, dest) == OK:
									copied += 1
									_call_logger(logger, "Copied runtime DLL: addons/%s/bin/%s" % [addon_name, fname])
								else:
									push_warning("[GDK Packaging] Failed to copy %s" % src)
						fname = bin.get_next()
					bin.list_dir_end()
		addon_name = addons.get_next()
	addons.list_dir_end()
	return copied


static func _resolve_logo_destination(content_dir: String, rel_path: String) -> String:
	var raw: String = rel_path.replace("\\", "/").strip_edges()
	if raw.is_empty() or raw.begins_with("/") or raw.contains("://") or _has_windows_drive(raw):
		return ""
	var normalized: String = _normalize_logo_relative_path(raw)
	if normalized.is_empty() or normalized == ".":
		return ""
	var dest_path: String = content_dir.path_join(normalized).replace("\\", "/").simplify_path()
	if not _is_path_inside_dir(dest_path, content_dir):
		return ""
	return dest_path


static func _normalize_logo_relative_path(rel_path: String) -> String:
	return rel_path.replace("\\", "/").strip_edges().simplify_path()


static func _has_windows_drive(path: String) -> bool:
	return path.length() >= 2 and path.substr(1, 1) == ":"


static func _is_path_inside_dir(candidate_path: String, root_dir: String) -> bool:
	var candidate: String = _normalize_path(candidate_path).to_lower()
	var root: String = _normalize_path(root_dir).to_lower()
	if candidate == root:
		return true
	if not root.ends_with("/"):
		root += "/"
	return candidate.begins_with(root)


static func _normalize_path(path: String) -> String:
	var normalized: String = path.replace("\\", "/").simplify_path()
	while normalized.ends_with("/") and normalized.length() > 1 and not normalized.ends_with(":/"):
		normalized = normalized.substr(0, normalized.length() - 1)
	return normalized


static func inject_vc14_dependency(content: String) -> String:
	if not content.contains("</Game>"):
		return content

	var dep_entry: String = '      <KnownDependency Name="VC14"/>\n'
	var existing_dr: RegEx = RegEx.new()
	existing_dr.compile('(?s)(<DesktopRegistration\\b[^>]*>)(.*?)(</DesktopRegistration>)')
	var desktop_match: RegExMatch = existing_dr.search(content)
	if desktop_match:
		var inner: String = desktop_match.get_string(2)
		var dependency_list: RegEx = RegEx.new()
		dependency_list.compile('(?s)(<DependencyList\\b[^>]*>)(.*?)(</DependencyList>)')
		var list_match: RegExMatch = dependency_list.search(inner)
		var new_inner: String = inner
		if list_match:
			if _dependency_list_has_name(list_match.get_string(), "VC14"):
				return content
			var list_body: String = list_match.get_string(2).strip_edges(false, true)
			if list_body.is_empty():
				list_body = "\n"
			elif not list_body.ends_with("\n"):
				list_body += "\n"
			var merged_list: String = list_match.get_string(1) + list_body + dep_entry + "    " + list_match.get_string(3)
			new_inner = inner.substr(0, list_match.get_start()) + merged_list + inner.substr(list_match.get_end())
		else:
			new_inner = inner.strip_edges(false, true)
			if new_inner.is_empty():
				new_inner = "\n"
			elif not new_inner.ends_with("\n"):
				new_inner += "\n"
			new_inner += '    <DependencyList>\n' + dep_entry + '    </DependencyList>\n  '
		var merged_desktop: String = desktop_match.get_string(1) + new_inner + desktop_match.get_string(3)
		return content.substr(0, desktop_match.get_start()) + merged_desktop + content.substr(desktop_match.get_end())

	var dep_xml: String = '  <DesktopRegistration>\n    <DependencyList>\n' + dep_entry + '    </DependencyList>\n  </DesktopRegistration>\n'
	return content.replace("</Game>", dep_xml + "</Game>")


static func patch_executable_name(content: String, executable_name: String) -> String:
	var regex: RegEx = RegEx.new()
	regex.compile('(<Executable\\b[^>]*\\bName=")[^"]*(")')
	var executable_match: RegExMatch = regex.search(content)
	if executable_match == null:
		return content

	var escaped_name: String = GameConfigManagerScript._escape_xml_attr(executable_name)
	var patched: String = ""
	var cursor: int = 0
	while executable_match != null:
		patched += content.substr(cursor, executable_match.get_start() - cursor)
		patched += executable_match.get_string(1) + escaped_name + executable_match.get_string(2)
		cursor = executable_match.get_end()
		executable_match = regex.search(content, cursor)
	return patched + content.substr(cursor)


static func _should_copy_runtime_dll(src_path: String, dest_path: String) -> bool:
	if not FileAccess.file_exists(dest_path):
		return true
	var src_hash: String = FileAccess.get_sha256(src_path)
	var dest_hash: String = FileAccess.get_sha256(dest_path)
	if not src_hash.is_empty() and not dest_hash.is_empty():
		return src_hash != dest_hash
	var src_time: int = FileAccess.get_modified_time(src_path)
	var dest_time: int = FileAccess.get_modified_time(dest_path)
	if src_time <= 0 or dest_time <= 0:
		return true
	return src_time > dest_time


static func _dependency_list_has_name(dependency_list_xml: String, dependency_name: String) -> bool:
	var regex: RegEx = RegEx.new()
	regex.compile("\\bName\\s*=\\s*[\"']" + dependency_name + "[\"']")
	return regex.search(dependency_list_xml) != null


func _find_primary_executable(content_dir: String) -> String:
	var dir: DirAccess = DirAccess.open(content_dir)
	if dir == null:
		return ""

	dir.list_dir_begin()
	var fname: String = dir.get_next()
	while fname != "":
		if fname.ends_with(".exe") and not fname.ends_with(".console.exe"):
			dir.list_dir_end()
			return fname
		fname = dir.get_next()
	dir.list_dir_end()
	return ""


func _call_logger(logger: Callable, message: String) -> void:
	if logger.is_valid():
		logger.call(message)
