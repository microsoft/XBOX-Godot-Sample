@tool
extends RefCounted
## Copies config and logo assets into a packaging content directory.

const GameConfigManagerScript = preload("res://addons/godot_gdk_packaging/editor/game_config_manager.gd")

var _config_mgr: RefCounted


func _init(config_mgr: RefCounted) -> void:
	_config_mgr = config_mgr


func ensure_content_dir_ready(content_dir: String, logger: Callable = Callable()) -> bool:
	var project_dir = ProjectSettings.globalize_path("res://")
	var config_src = _config_mgr.get_config_path()
	var config_dest = content_dir.path_join("MicrosoftGame.config")

	if not FileAccess.file_exists(config_src):
		_call_logger(logger, "❌ MicrosoftGame.config not found — create one first.")
		return false

	var file = FileAccess.open(config_src, FileAccess.READ)
	if file == null:
		_call_logger(logger, "❌ Cannot read MicrosoftGame.config")
		return false
	var content = file.get_as_text()
	file.close()

	content = inject_vc14_dependency(content)

	var executable_name = _find_primary_executable(content_dir)
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

	var info = _config_mgr.parse_config()
	var logo_keys := {
		"store_logo": "StoreLogo",
		"logo_150": "Square150x150Logo",
		"logo_44": "Square44x44Logo",
		"logo_480": "Square480x480Logo",
		"splash_screen": "SplashScreenImage",
	}

	for key in logo_keys:
		var rel_path: String = info.get(key, "")
		if rel_path == "":
			rel_path = logo_keys[key] + ".png"
		var normalized = rel_path.replace("\\", "/")
		var dest_path = content_dir.path_join(normalized)

		var src_path = ""
		var filename = normalized.get_file()
		var storelogos_src = project_dir.path_join("storelogos").path_join(filename)
		var root_src = project_dir.path_join(filename)
		if FileAccess.file_exists(storelogos_src):
			src_path = storelogos_src
		elif FileAccess.file_exists(root_src):
			src_path = root_src

		if src_path != "":
			var dest_dir = dest_path.get_base_dir()
			DirAccess.make_dir_recursive_absolute(dest_dir)
			var copy_dir = DirAccess.open(project_dir)
			if copy_dir != null:
				copy_dir.copy(src_path, dest_path)

	return true


static func inject_vc14_dependency(content: String) -> String:
	if content.contains('<KnownDependency Name="VC14"/>') or not content.contains("</Game>"):
		return content

	var dep_inner = '    <DependencyList>\n      <KnownDependency Name="VC14"/>\n    </DependencyList>\n'
	var existing_dr := RegEx.new()
	# Match an existing <DesktopRegistration>...</DesktopRegistration> block (non-greedy, DOTALL so
	# it can span newlines). If found, merge the DependencyList into it instead of appending a new
	# DesktopRegistration block — GDK rejects configs with more than one DesktopRegistration.
	existing_dr.compile('(?s)<DesktopRegistration>(.*?)</DesktopRegistration>')
	var match = existing_dr.search(content)
	if match:
		var inner: String = match.get_string(1)
		var trimmed: String = inner.strip_edges(false, true)
		var merged: String = "<DesktopRegistration>%s\n%s  </DesktopRegistration>" % [trimmed, dep_inner]
		return content.substr(0, match.get_start()) + merged + content.substr(match.get_end())

	var dep_xml = '  <DesktopRegistration>\n' + dep_inner + '  </DesktopRegistration>\n'
	return content.replace("</Game>", dep_xml + "</Game>")


static func patch_executable_name(content: String, executable_name: String) -> String:
	var regex = RegEx.new()
	regex.compile('Executable Name="[^"]*"')
	if regex.search(content):
		return regex.sub(content, 'Executable Name="%s"' % executable_name)
	return content


func _find_primary_executable(content_dir: String) -> String:
	var dir = DirAccess.open(content_dir)
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


func _call_logger(logger: Callable, message: String) -> void:
	if logger.is_valid():
		logger.call(message)
