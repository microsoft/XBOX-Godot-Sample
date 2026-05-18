@tool
extends RefCounted
## Copies config and logo assets into a packaging content directory.

const GameConfigManagerScript = preload("res://addons/godot_gdk_packaging/core/game_config_manager.gd")

var _config_mgr: RefCounted


func _init(config_mgr: RefCounted) -> void:
	_config_mgr = config_mgr


func ensure_content_dir_ready(content_dir: String, logger: Callable = Callable()) -> bool:
	var project_dir: String = ProjectSettings.globalize_path("res://")
	var config_src: String = _config_mgr.get_config_path()
	var config_dest: String = content_dir.path_join("MicrosoftGame.config")

	if not FileAccess.file_exists(config_src):
		_call_logger(logger, "❌ MicrosoftGame.config not found — create one first.")
		return false

	var file: FileAccess = FileAccess.open(config_src, FileAccess.READ)
	if file == null:
		_call_logger(logger, "❌ Cannot read MicrosoftGame.config")
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

	var info: Dictionary = _config_mgr.parse_config()
	var logo_keys: Dictionary = {
		"store_logo": "StoreLogo",
		"logo_150": "Square150x150Logo",
		"logo_44": "Square44x44Logo",
		"logo_480": "Square480x480Logo",
		"splash_screen": "SplashScreenImage",
	}

	for key: String in logo_keys:
		var rel_path: String = info.get(key, "")
		if rel_path == "":
			rel_path = logo_keys[key] + ".png"
		var normalized: String = rel_path.replace("\\", "/")
		var dest_path: String = content_dir.path_join(normalized)

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
							if not FileAccess.file_exists(dest):
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


static func inject_vc14_dependency(content: String) -> String:
	if content.contains('<KnownDependency Name="VC14"/>') or not content.contains("</Game>"):
		return content

	var dep_inner: String = '    <DependencyList>\n      <KnownDependency Name="VC14"/>\n    </DependencyList>\n'
	var existing_dr: RegEx = RegEx.new()
	# Match an existing <DesktopRegistration>...</DesktopRegistration> block (non-greedy, DOTALL so
	# it can span newlines). If found, merge the DependencyList into it instead of appending a new
	# DesktopRegistration block — GDK rejects configs with more than one DesktopRegistration.
	existing_dr.compile('(?s)<DesktopRegistration>(.*?)</DesktopRegistration>')
	var match: RegExMatch = existing_dr.search(content)
	if match:
		var inner: String = match.get_string(1)
		var trimmed: String = inner.strip_edges(false, true)
		var merged: String = "<DesktopRegistration>%s\n%s  </DesktopRegistration>" % [trimmed, dep_inner]
		return content.substr(0, match.get_start()) + merged + content.substr(match.get_end())

	var dep_xml: String = '  <DesktopRegistration>\n' + dep_inner + '  </DesktopRegistration>\n'
	return content.replace("</Game>", dep_xml + "</Game>")


static func patch_executable_name(content: String, executable_name: String) -> String:
	var regex: RegEx = RegEx.new()
	regex.compile('Executable Name="[^"]*"')
	if regex.search(content):
		return regex.sub(content, 'Executable Name="%s"' % executable_name)
	return content


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
