@tool
extends RefCounted
## Manages MicrosoftGame.config — detection, parsing, template generation,
## and launching the GameConfigEditor GUI.

const GDKToolchainScript = preload("res://addons/godot_gdk_packaging/core/gdk_toolchain.gd")
const CONFIG_FILENAME := "MicrosoftGame.config"

var _toolchain: RefCounted


func _init(toolchain: RefCounted) -> void:
	_toolchain = toolchain


# ── Detection ───────────────────────────────────────────────────────────────

## Returns the expected path of MicrosoftGame.config in the project root.
func get_config_path() -> String:
	return ProjectSettings.globalize_path("res://" + CONFIG_FILENAME)

## Returns the res:// path of MicrosoftGame.config.
func get_config_res_path() -> String:
	return "res://" + CONFIG_FILENAME

## Returns true if MicrosoftGame.config exists in the project root.
func config_exists() -> bool:
	return FileAccess.file_exists("res://" + CONFIG_FILENAME)


## Normalizes a Godot path to a filesystem-absolute path so it can safely be
## passed to [code]DirAccess.*_absolute()[/code] and [code]DirAccess.remove_absolute()[/code]:
## - [code]res://[/code] / [code]user://[/code] paths are globalized.
## - Already-absolute filesystem paths are returned unchanged.
## - Relative paths (e.g. [code]Configs/Alt.config[/code] from a headless
##   [code]--output[/code] flag) are resolved against the project root so the
##   write target lands inside the project rather than CWD-dependent
##   unpredictability.
## Empty input is returned unchanged.
static func to_filesystem_path(p: String) -> String:
	if p.is_empty():
		return p
	if p.begins_with("res://") or p.begins_with("user://"):
		return ProjectSettings.globalize_path(p)
	if p.is_absolute_path():
		return p
	return ProjectSettings.globalize_path("res://").path_join(p)


# ── Parsing ─────────────────────────────────────────────────────────────────

## Parses MicrosoftGame.config and returns identity info as a Dictionary:
##   { name, publisher, version, product_id, executable, display_name, description }
## Returns an empty dictionary on failure.
func parse_config(config_path: String = "") -> Dictionary:
	var path: String = config_path
	if path.is_empty():
		path = get_config_path()
	if not FileAccess.file_exists(path):
		return {}

	var parser: XMLParser = XMLParser.new()
	var err: Error = parser.open(path)
	if err != OK:
		push_error("[GDK Packaging] Failed to open MicrosoftGame.config: " + error_string(err))
		return {}

	var result: Dictionary = {
		"name": "",
		"publisher": "",
		"version": "",
		"product_id": "",
		"executable": "",
		"display_name": "",
		"description": "",
		"title_id": "",
		"msa_app_id": "",
		"store_id": "",
		"config_version": "",
		"background_color": "",
		"foreground_text": "",
	}

	while parser.read() == OK:
		if parser.get_node_type() != XMLParser.NODE_ELEMENT:
			continue

		var node_name: String = parser.get_node_name()

		if node_name == "Game":
			for i: int in parser.get_attribute_count():
				if parser.get_attribute_name(i) == "configVersion":
					result["config_version"] = parser.get_attribute_value(i)

		elif node_name == "Identity":
			for i: int in parser.get_attribute_count():
				match parser.get_attribute_name(i):
					"Name":
						result["name"] = parser.get_attribute_value(i)
					"Publisher":
						result["publisher"] = parser.get_attribute_value(i)
					"Version":
						result["version"] = parser.get_attribute_value(i)

		elif node_name == "Executable":
			for i: int in parser.get_attribute_count():
				if parser.get_attribute_name(i) == "Name":
					result["executable"] = parser.get_attribute_value(i)

		elif node_name == "TitleId":
			if not parser.is_empty():
				if parser.read() == OK and parser.get_node_type() == XMLParser.NODE_TEXT:
					result["title_id"] = parser.get_node_data().strip_edges()

		elif node_name == "MSAAppId":
			if not parser.is_empty():
				if parser.read() == OK and parser.get_node_type() == XMLParser.NODE_TEXT:
					result["msa_app_id"] = parser.get_node_data().strip_edges()

		elif node_name == "StoreId":
			if not parser.is_empty():
				if parser.read() == OK and parser.get_node_type() == XMLParser.NODE_TEXT:
					result["store_id"] = parser.get_node_data().strip_edges()

		elif node_name == "ShellVisuals":
			for i: int in parser.get_attribute_count():
				match parser.get_attribute_name(i):
					"DefaultDisplayName":
						result["display_name"] = parser.get_attribute_value(i)
					"Description":
						result["description"] = parser.get_attribute_value(i)
					"Square480x480Logo":
						result["logo_480"] = parser.get_attribute_value(i)
					"Square150x150Logo":
						result["logo_150"] = parser.get_attribute_value(i)
					"Square44x44Logo":
						result["logo_44"] = parser.get_attribute_value(i)
					"StoreLogo":
						result["store_logo"] = parser.get_attribute_value(i)
					"SplashScreenImage":
						result["splash_screen"] = parser.get_attribute_value(i)
					"BackgroundColor":
						result["background_color"] = parser.get_attribute_value(i)
					"ForegroundText":
						result["foreground_text"] = parser.get_attribute_value(i)

		# ProductId can appear as an attribute on the MSStore element
		elif node_name == "MSStore":
			for i: int in parser.get_attribute_count():
				if parser.get_attribute_name(i) == "ProductId":
					result["product_id"] = parser.get_attribute_value(i)

	return result


# ── Template Generation ─────────────────────────────────────────────────────

## Creates a template MicrosoftGame.config. If [param output_path] is empty,
## writes to the project-root `res://MicrosoftGame.config`.
## [param game_name] becomes the Executable Name (must match the exported .exe);
## a sanitized form (spaces and underscores stripped) is used for the Identity
## Name, since MicrosoftGame.config rejects those characters there.
## [param display_name] is used verbatim for ShellVisuals/DefaultDisplayName
## and may contain spaces.
## Returns OK on success, or an error code.
func create_template(game_name: String = "MyGodotGame",
		publisher: String = "CN=Publisher",
		display_name: String = "My Godot Game",
		output_path: String = "") -> Error:
	var target_path: String = output_path
	if target_path.is_empty():
		target_path = get_config_res_path()
	var fs_target: String = to_filesystem_path(target_path)

	if FileAccess.file_exists(fs_target):
		push_warning("[GDK Packaging] MicrosoftGame.config already exists — not overwriting.")
		return ERR_ALREADY_EXISTS

	var exe_name: String = game_name + ".exe"
	var identity_name: String = _sanitize_identity_name(game_name)

	var xml: String = ""
	xml += '<?xml version="1.0" encoding="utf-8"?>\n'
	xml += '<Game configVersion="1">\n'
	xml += '  <Identity Name="%s"\n' % _escape_xml_attr(identity_name)
	xml += '            Publisher="%s"\n' % _escape_xml_attr(publisher)
	xml += '            Version="1.0.0.0" />\n'
	xml += '  <ExecutableList>\n'
	xml += '    <Executable Name="%s"\n' % _escape_xml_attr(exe_name)
	xml += '                Id="Game"\n'
	xml += '                TargetDeviceFamily="PC" />\n'
	xml += '  </ExecutableList>\n'
	xml += '  <ShellVisuals DefaultDisplayName="%s"\n' % _escape_xml_attr(display_name)
	xml += '                PublisherDisplayName="%s"\n' % _escape_xml_attr(publisher.replace("CN=", ""))
	xml += '                StoreLogo="storelogos\\StoreLogo.png"\n'
	xml += '                Square150x150Logo="storelogos\\Square150x150Logo.png"\n'
	xml += '                Square44x44Logo="storelogos\\Square44x44Logo.png"\n'
	xml += '                Square480x480Logo="storelogos\\Square480x480Logo.png"\n'
	xml += '                SplashScreenImage="storelogos\\SplashScreenImage.png"\n'
	xml += '                Description="A Godot game packaged with GDK"\n'
	xml += '                BackgroundColor="#000000"\n'
	xml += '                ForegroundText="light" />\n'
	xml += '  <AdvancedUserModel>false</AdvancedUserModel>\n'
	xml += '</Game>\n'

	var target_dir: String = fs_target.get_base_dir()
	if not target_dir.is_empty() and not DirAccess.dir_exists_absolute(target_dir):
		DirAccess.make_dir_recursive_absolute(target_dir)

	var file: FileAccess = FileAccess.open(fs_target, FileAccess.WRITE)
	if file == null:
		push_error("[GDK Packaging] Failed to create MicrosoftGame.config: " + error_string(FileAccess.get_open_error()))
		return FileAccess.get_open_error()

	file.store_string(xml)
	file.close()
	print("[GDK Packaging] Created template MicrosoftGame.config at: ", fs_target)

	# Generate placeholder logo images so GameConfigEditor doesn't error.
	# Pass the config's base dir so the storelogos/ folder is created next to
	# the config file (not under res://) when output_path lands outside the
	# project root; the config's relative "storelogos\..." attributes resolve
	# against the config's parent directory.
	_ensure_placeholder_images(target_dir)

	return OK


## Copies the GDK default 480x480 PNG and resizes it to create all placeholder
## images referenced by the MicrosoftGame.config template. Writes them under
## a "storelogos/" folder inside [param base_dir] (defaults to the project
## root for backward compatibility) so the relative paths the template
## embeds resolve next to the config file.
func _ensure_placeholder_images(base_dir: String = "") -> void:
	var logos_root: String = base_dir
	if logos_root.is_empty():
		logos_root = ProjectSettings.globalize_path("res://")
	var logos_dir: String = logos_root.path_join("storelogos")
	var default_png: String = _toolchain.get_bin_dir().path_join(
		"GameConfigEditorDependencies/default480x480.png")

	if not FileAccess.file_exists(default_png):
		push_warning("[GDK Packaging] Default PNG not found at: " + default_png)
		return

	var targets: Dictionary = {
		"Square480x480Logo.png": Vector2i(480, 480),
		"Square150x150Logo.png": Vector2i(150, 150),
		"Square44x44Logo.png": Vector2i(44, 44),
		"StoreLogo.png": Vector2i(100, 100),
		"SplashScreenImage.png": Vector2i(1920, 1080),
	}

	# Check if any images need generating
	var any_missing: bool = false
	for filename: String in targets:
		if not FileAccess.file_exists(logos_dir.path_join(filename)):
			any_missing = true
			break

	if not any_missing:
		return

	# Ensure storelogos directory exists
	DirAccess.make_dir_recursive_absolute(logos_dir)

	var source_image: Image = Image.new()
	var err: Error = source_image.load(default_png)
	if err != OK:
		push_warning("[GDK Packaging] Failed to load default PNG: " + error_string(err))
		return

	for filename: String in targets:
		var dest_path: String = logos_dir.path_join(filename)
		if FileAccess.file_exists(dest_path):
			continue
		var img: Image = source_image.duplicate()
		var size: Vector2i = targets[filename]
		img.resize(size.x, size.y, Image.INTERPOLATE_LANCZOS)
		err = img.save_png(dest_path)
		if err == OK:
			print("[GDK Packaging] Created placeholder: storelogos/", filename)
		else:
			push_warning("[GDK Packaging] Failed to create " + filename + ": " + error_string(err))


## Reads the largest logo (480x480) from the config and regenerates all other
## logo sizes from it. Call after GameConfigEditor saves changes.
## Returns the number of logos updated.
func sync_store_logos() -> int:
	if not config_exists():
		return 0

	var info: Dictionary = parse_config()
	var project_dir: String = ProjectSettings.globalize_path("res://")

	# Find the 480x480 source logo — this is the primary logo to derive others from
	var logo_480_rel: String = info.get("logo_480", "")
	if logo_480_rel == "":
		return 0

	# Resolve relative path (config paths use backslashes)
	var logo_480_path: String = project_dir.path_join(logo_480_rel.replace("\\", "/"))
	if not FileAccess.file_exists(logo_480_path):
		print("[GDK Packaging] 480x480 logo not found at: ", logo_480_path)
		return 0

	var source_image: Image = Image.new()
	var err: Error = source_image.load(logo_480_path)
	if err != OK:
		push_warning("[GDK Packaging] Failed to load 480x480 logo: " + error_string(err))
		return 0

	var logos_dir: String = project_dir.path_join("storelogos")
	DirAccess.make_dir_recursive_absolute(logos_dir)

	# All logos including the 480x480 itself
	var logo_map: Dictionary = {
		"logo_480": Vector2i(480, 480),
		"logo_150": Vector2i(150, 150),
		"logo_44": Vector2i(44, 44),
		"store_logo": Vector2i(100, 100),
		"splash_screen": Vector2i(1920, 1080),
	}

	# Standard filenames for each logo
	var standard_names: Dictionary = {
		"logo_480": "Square480x480Logo.png",
		"logo_150": "Square150x150Logo.png",
		"logo_44": "Square44x44Logo.png",
		"store_logo": "StoreLogo.png",
		"splash_screen": "SplashScreenImage.png",
	}

	var updated: int = 0
	for key: String in logo_map:
		var dest_path: String = logos_dir.path_join(standard_names[key])

		var img: Image = source_image.duplicate()
		var size: Vector2i = logo_map[key]
		img.resize(size.x, size.y, Image.INTERPOLATE_LANCZOS)
		err = img.save_png(dest_path)
		if err == OK:
			updated += 1
			print("[GDK Packaging] Synced logo: storelogos/", standard_names[key])
		else:
			push_warning("[GDK Packaging] Failed to sync " + standard_names[key] + ": " + error_string(err))

	return updated


## Detects logo PNGs that GameConfigEditor wrote to the project root,
## moves them into storelogos/, and updates MicrosoftGame.config paths.
## Returns the number of files relocated.
func relocate_logos_to_storelogos() -> int:
	if not config_exists():
		return 0

	var project_dir: String = ProjectSettings.globalize_path("res://")
	var logos_dir: String = project_dir.path_join("storelogos")

	# Known logo filenames that GameConfigEditor writes to the project root
	var logo_files: Dictionary = {
		"StoreLogo.png": "StoreLogo",
		"Square44x44Logo.png": "Square44x44Logo",
		"Square150x150Logo.png": "Square150x150Logo",
		"Square480x480Logo.png": "Square480x480Logo",
		"SplashScreenImage.png": "SplashScreenImage",
	}

	# Also detect the source image (e.g. fhl_logo.png) referenced in the config
	var info: Dictionary = parse_config()
	var config_logos: Dictionary = {
		"store_logo": "StoreLogo",
		"logo_44": "Square44x44Logo",
		"logo_150": "Square150x150Logo",
		"logo_480": "Square480x480Logo",
		"splash_screen": "SplashScreenImage",
	}

	# Build a mapping of root files that need to move
	var files_to_move: Dictionary = {}  # src_abs -> dest_filename
	var path_replacements: Dictionary = {}  # old_config_value -> new_config_value

	for key: String in config_logos:
		var rel_path: String = info.get(key, "")
		if rel_path == "":
			continue
		var normalized: String = rel_path.replace("\\", "/")
		# Only relocate if the file is at the project root (no directory component)
		if normalized.contains("/"):
			continue
		var src_abs: String = project_dir.path_join(normalized)
		if not FileAccess.file_exists(src_abs):
			continue
		var dest_filename: String = normalized.get_file()
		files_to_move[src_abs] = dest_filename
		path_replacements[rel_path] = "storelogos\\" + dest_filename

	# Also check for standard GameConfigEditor output names at root
	for filename: String in logo_files:
		var src_abs: String = project_dir.path_join(filename)
		if FileAccess.file_exists(src_abs) and not files_to_move.has(src_abs):
			files_to_move[src_abs] = filename
			path_replacements[filename] = "storelogos\\" + filename

	if files_to_move.is_empty():
		return 0

	# Ensure storelogos directory exists
	DirAccess.make_dir_recursive_absolute(logos_dir)

	# Move files
	var moved: int = 0
	var dir: DirAccess = DirAccess.open(project_dir)
	for src_abs: String in files_to_move:
		var dest_filename: String = files_to_move[src_abs]
		var dest_abs: String = logos_dir.path_join(dest_filename)
		var err: Error = dir.rename(src_abs, dest_abs)
		if err == OK:
			moved += 1
			print("[GDK Packaging] Moved ", src_abs.get_file(), " -> storelogos/", dest_filename)
		else:
			push_warning("[GDK Packaging] Failed to move " + src_abs.get_file() + ": " + error_string(err))

	# Update MicrosoftGame.config with new paths
	if moved > 0 and not path_replacements.is_empty():
		var config_path: String = get_config_path()
		var file: FileAccess = FileAccess.open(config_path, FileAccess.READ)
		if file != null:
			var content: String = file.get_as_text()
			file.close()
			for old_val: String in path_replacements:
				var new_val: String = path_replacements[old_val]
				content = content.replace('"' + old_val + '"', '"' + new_val + '"')
			file = FileAccess.open(config_path, FileAccess.WRITE)
			if file != null:
				file.store_string(content)
				file.close()
				print("[GDK Packaging] Updated MicrosoftGame.config logo paths")

	return moved


# ── GameConfigEditor Launch ─────────────────────────────────────────────────

## Launches MicrosoftGameConfigEditor.exe with the project's config file.
## Returns the PID, or -1 on failure.
## Escapes special characters for safe use in XML attribute values.
static func _escape_xml_attr(value: String) -> String:
	return value.replace("&", "&amp;").replace('"', "&quot;").replace("<", "&lt;").replace(">", "&gt;")


## Strips characters disallowed in MicrosoftGame.config Identity/@Name (spaces
## and underscores, per the [code][^_ ]+[/code] pattern constraint enforced by
## makepkg). Falls back to "MyGodotGame" if the sanitized result is empty.
static func _sanitize_identity_name(value: String) -> String:
	var sanitized: String = value.replace(" ", "").replace("_", "")
	if sanitized.is_empty():
		return "MyGodotGame"
	return sanitized


func launch_editor() -> int:
	var config_path: String = get_config_path()
	if not FileAccess.file_exists(config_path):
		push_error("[GDK Packaging] MicrosoftGame.config not found — create one first.")
		return -1

	# Ensure placeholder images exist before opening the editor
	_ensure_placeholder_images()

	# Relocate any root-level logos and update config paths to storelogos/
	# so GameConfigEditor writes directly to storelogos/ on save
	relocate_logos_to_storelogos()
	_rewrite_config_paths_to_storelogos()

	var args: PackedStringArray = PackedStringArray([config_path])
	var pid: int = _toolchain.launch_detached(_toolchain.get_game_config_editor_path(), args)
	if pid >= 0:
		print("[GDK Packaging] Launched GameConfigEditor (PID: ", pid, ")")
	return pid


## Rewrites all logo paths in MicrosoftGame.config to use storelogos/ prefix.
## This ensures GameConfigEditor saves directly into the storelogos/ folder.
func _rewrite_config_paths_to_storelogos() -> void:
	var config_path: String = get_config_path()
	var file: FileAccess = FileAccess.open(config_path, FileAccess.READ)
	if file == null:
		return
	var content: String = file.get_as_text()
	file.close()

	var logo_attrs: Array = [
		"StoreLogo",
		"Square44x44Logo",
		"Square150x150Logo",
		"Square480x480Logo",
		"SplashScreenImage",
	]

	var changed: bool = false
	for attr: String in logo_attrs:
		# Match attr="something.png" where something.png doesn't already start with storelogos
		var regex: RegEx = RegEx.new()
		regex.compile(attr + '="(?!storelogos[/\\\\])([^"]+)"')
		var result: RegExMatch = regex.search(content)
		if result:
			var old_path: String = result.get_string(1)
			var filename: String = old_path.replace("\\", "/").get_file()
			var new_path: String = "storelogos\\" + filename
			content = content.replace(
				attr + '="' + old_path + '"',
				attr + '="' + new_path + '"')
			changed = true

	if changed:
		file = FileAccess.open(config_path, FileAccess.WRITE)
		if file != null:
			file.store_string(content)
			file.close()
			print("[GDK Packaging] Rewrote config logo paths to storelogos/")
