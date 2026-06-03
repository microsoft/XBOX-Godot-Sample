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
	xml += '                StoreLogo="StoreLogo.png"\n'
	xml += '                Square150x150Logo="Square150x150Logo.png"\n'
	xml += '                Square44x44Logo="Square44x44Logo.png"\n'
	xml += '                Square480x480Logo="Square480x480Logo.png"\n'
	xml += '                SplashScreenImage="SplashScreenImage.png"\n'
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
	# Pass the config's base dir so the placeholders are created next to the
	# config file (not under res://) when output_path lands outside the project
	# root; the config's root-relative logo attributes resolve against the
	# config's parent directory.
	_ensure_placeholder_images(target_dir)

	return OK


## Copies the GDK default 480x480 PNG and resizes it to create all placeholder
## images referenced by the MicrosoftGame.config template. Writes them directly
## into [param base_dir] (defaults to the project root) — the same location
## Microsoft's GameConfigEditor writes picked tiles to — so the config's
## root-relative logo attributes resolve next to the config file and we never
## end up with duplicate copies under a separate storelogos/ folder.
func _ensure_placeholder_images(base_dir: String = "") -> void:
	var logos_dir: String = base_dir
	if logos_dir.is_empty():
		logos_dir = ProjectSettings.globalize_path("res://")
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
			print("[GDK Packaging] Created placeholder: ", filename)
		else:
			push_warning("[GDK Packaging] Failed to create " + filename + ": " + error_string(err))


## Regenerates the derived logo size variants from the 480x480 master the config
## points at, writing each variant to the exact path its ShellVisuals attribute
## declares (resolved against the project root). Microsoft's GameConfigEditor
## writes a newly-picked tile to the project root but does not refresh the
## derived sizes; this closes that gap without moving files or rewriting the
## config. Call after GameConfigEditor saves changes.
## Returns the number of variant files regenerated.
func sync_store_logos() -> int:
	if not config_exists():
		return 0

	var info: Dictionary = parse_config()
	var project_dir: String = ProjectSettings.globalize_path("res://")

	# The 480x480 logo is the master every other size is derived from.
	var master_rel: String = str(info.get("logo_480", ""))
	if master_rel.is_empty():
		return 0
	var master_path: String = project_dir.path_join(master_rel.replace("\\", "/"))
	if not FileAccess.file_exists(master_path):
		push_warning("[GDK Packaging] 480x480 master logo not found at: " + master_path)
		return 0

	var source_image: Image = Image.new()
	var err: Error = source_image.load(master_path)
	if err != OK:
		push_warning("[GDK Packaging] Failed to load 480x480 master logo: " + error_string(err))
		return 0

	# Derived variants: config key -> [target size, default root filename]. The
	# 480x480 master is intentionally excluded — it is the picked source, not a
	# derived size, so we never overwrite the user's chosen image.
	var variants: Dictionary = {
		"logo_150": [Vector2i(150, 150), "Square150x150Logo.png"],
		"logo_44": [Vector2i(44, 44), "Square44x44Logo.png"],
		"store_logo": [Vector2i(100, 100), "StoreLogo.png"],
		"splash_screen": [Vector2i(1920, 1080), "SplashScreenImage.png"],
	}

	var updated: int = 0
	for key: String in variants:
		var size: Vector2i = variants[key][0]
		# Write each variant to the path the config actually references so the
		# packaging staging step finds a fresh file there. Fall back to the
		# standard root filename when the attribute is absent from the config.
		var rel: String = str(info.get(key, ""))
		if rel.is_empty():
			rel = str(variants[key][1])
		var dest_path: String = project_dir.path_join(rel.replace("\\", "/"))
		var dest_dir: String = dest_path.get_base_dir()
		if not dest_dir.is_empty():
			DirAccess.make_dir_recursive_absolute(dest_dir)
		var img: Image = source_image.duplicate()
		img.resize(size.x, size.y, Image.INTERPOLATE_LANCZOS)
		err = img.save_png(dest_path)
		if err == OK:
			updated += 1
			print("[GDK Packaging] Synced logo: ", rel)
		else:
			push_warning("[GDK Packaging] Failed to sync " + rel + ": " + error_string(err))

	return updated


## Reconciles logo assets after a GameConfigEditor session. Microsoft's
## GameConfigEditor writes a newly-picked tile image to the project root (with
## no option to redirect it) and leaves the derived size variants stale. We
## align with that behaviour by keeping every logo at the project root, so the
## only reconciliation needed is regenerating the stale size variants from the
## picked 480x480 master via sync_store_logos(). No files are moved and the
## config is not rewritten, so MCGE's root output never produces duplicates.
## Returns { "synced": int }.
func reconcile_logos_after_edit() -> Dictionary:
	return {"synced": sync_store_logos()}


# ── GameConfigEditor Launch ─────────────────────────────────────────────────

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


## Launches MicrosoftGameConfigEditor.exe with the project's config file.
## Returns the PID, or -1 on failure.
func launch_editor() -> int:
	var config_path: String = get_config_path()
	if not FileAccess.file_exists(config_path):
		push_error("[GDK Packaging] MicrosoftGame.config not found — create one first.")
		return -1

	# Ensure placeholder images exist at the project root (where GameConfigEditor
	# also writes picked tiles) before opening the editor.
	_ensure_placeholder_images()

	var args: PackedStringArray = PackedStringArray([config_path])
	var pid: int = _toolchain.launch_detached(_toolchain.get_game_config_editor_path(), args)
	if pid >= 0:
		print("[GDK Packaging] Launched GameConfigEditor (PID: ", pid, ")")
	return pid
