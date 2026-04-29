@tool
extends RefCounted
## Manages MicrosoftGame.config — detection, parsing, template generation,
## and launching the GameConfigEditor GUI.

const GDKToolchainScript = preload("res://addons/godot_gdk_packaging/editor/gdk_toolchain.gd")
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


# ── Parsing ─────────────────────────────────────────────────────────────────

## Parses MicrosoftGame.config and returns identity info as a Dictionary:
##   { name, publisher, version, product_id, executable, display_name, description }
## Returns an empty dictionary on failure.
func parse_config() -> Dictionary:
	var path := get_config_path()
	if not FileAccess.file_exists(path):
		return {}

	var parser := XMLParser.new()
	var err := parser.open(path)
	if err != OK:
		push_error("[GDK Packaging] Failed to open MicrosoftGame.config: " + error_string(err))
		return {}

	var result := {
		"name": "",
		"publisher": "",
		"version": "",
		"product_id": "",
		"executable": "",
		"display_name": "",
		"description": "",
	}

	while parser.read() == OK:
		if parser.get_node_type() != XMLParser.NODE_ELEMENT:
			continue

		var node_name := parser.get_node_name()

		if node_name == "Identity":
			for i in parser.get_attribute_count():
				match parser.get_attribute_name(i):
					"Name":
						result["name"] = parser.get_attribute_value(i)
					"Publisher":
						result["publisher"] = parser.get_attribute_value(i)
					"Version":
						result["version"] = parser.get_attribute_value(i)

		elif node_name == "Executable":
			for i in parser.get_attribute_count():
				if parser.get_attribute_name(i) == "Name":
					result["executable"] = parser.get_attribute_value(i)

		elif node_name == "ShellVisuals":
			for i in parser.get_attribute_count():
				match parser.get_attribute_name(i):
					"DefaultDisplayName":
						result["display_name"] = parser.get_attribute_value(i)
					"Description":
						result["description"] = parser.get_attribute_value(i)

		# ProductId can appear as an attribute on the MSStore element
		elif node_name == "MSStore":
			for i in parser.get_attribute_count():
				if parser.get_attribute_name(i) == "ProductId":
					result["product_id"] = parser.get_attribute_value(i)

	return result


# ── Template Generation ─────────────────────────────────────────────────────

## Creates a template MicrosoftGame.config in the project root.
## Returns OK on success, or an error code.
func create_template(game_name: String = "MyGodotGame",
		publisher: String = "CN=Publisher",
		display_name: String = "My Godot Game") -> Error:
	var res_path = get_config_res_path()

	if FileAccess.file_exists(res_path):
		push_warning("[GDK Packaging] MicrosoftGame.config already exists — not overwriting.")
		return ERR_ALREADY_EXISTS

	var exe_name := game_name + ".exe"

	var xml := ""
	xml += '<?xml version="1.0" encoding="utf-8"?>\n'
	xml += '<Game configVersion="1">\n'
	xml += '  <Identity Name="%s"\n' % _escape_xml_attr(game_name)
	xml += '            Publisher="%s"\n' % _escape_xml_attr(publisher)
	xml += '            Version="1.0.0.0" />\n'
	xml += '  <ExecutableList>\n'
	xml += '    <Executable Name="%s"\n' % _escape_xml_attr(exe_name)
	xml += '                Id="Game" />\n'
	xml += '  </ExecutableList>\n'
	xml += '  <ShellVisuals DefaultDisplayName="%s"\n' % _escape_xml_attr(display_name)
	xml += '                PublisherDisplayName="%s"\n' % _escape_xml_attr(publisher.replace("CN=", ""))
	xml += '                StoreLogo="storelogos\\StoreLogo.png"\n'
	xml += '                Square150x150Logo="storelogos\\Logo150.png"\n'
	xml += '                Square44x44Logo="storelogos\\Logo44.png"\n'
	xml += '                Square480x480Logo="storelogos\\Logo480.png"\n'
	xml += '                SplashScreenImage="storelogos\\SplashScreen.png"\n'
	xml += '                Description="A Godot game packaged with GDK"\n'
	xml += '                BackgroundColor="#000000"\n'
	xml += '                ForegroundText="light" />\n'
	xml += '</Game>\n'

	var file = FileAccess.open(res_path, FileAccess.WRITE)
	if file == null:
		push_error("[GDK Packaging] Failed to create MicrosoftGame.config: " + error_string(FileAccess.get_open_error()))
		return FileAccess.get_open_error()

	file.store_string(xml)
	file.close()
	print("[GDK Packaging] Created template MicrosoftGame.config at: ", res_path)

	# Generate placeholder logo images so GameConfigEditor doesn't error
	_ensure_placeholder_images()

	return OK


## Copies the GDK default 480x480 PNG and resizes it to create all placeholder
## images referenced by the MicrosoftGame.config template.
func _ensure_placeholder_images() -> void:
	var project_dir = ProjectSettings.globalize_path("res://")
	var logos_dir = project_dir.path_join("storelogos")
	var default_png = _toolchain.get_bin_dir().path_join(
		"GameConfigEditorDependencies/default480x480.png")

	if not FileAccess.file_exists(default_png):
		push_warning("[GDK Packaging] Default PNG not found at: " + default_png)
		return

	var targets := {
		"Logo480.png": Vector2i(480, 480),
		"Logo150.png": Vector2i(150, 150),
		"Logo44.png": Vector2i(44, 44),
		"StoreLogo.png": Vector2i(50, 50),
		"SplashScreen.png": Vector2i(1920, 1080),
	}

	# Check if any images need generating
	var any_missing := false
	for filename in targets:
		if not FileAccess.file_exists(logos_dir.path_join(filename)):
			any_missing = true
			break

	if not any_missing:
		return

	# Ensure storelogos directory exists
	DirAccess.make_dir_recursive_absolute(logos_dir)

	var source_image = Image.new()
	var err = source_image.load(default_png)
	if err != OK:
		push_warning("[GDK Packaging] Failed to load default PNG: " + error_string(err))
		return

	for filename in targets:
		var dest_path = logos_dir.path_join(filename)
		if FileAccess.file_exists(dest_path):
			continue
		var img = source_image.duplicate()
		var size: Vector2i = targets[filename]
		img.resize(size.x, size.y, Image.INTERPOLATE_LANCZOS)
		err = img.save_png(dest_path)
		if err == OK:
			print("[GDK Packaging] Created placeholder: storelogos/", filename)
		else:
			push_warning("[GDK Packaging] Failed to create " + filename + ": " + error_string(err))


# ── GameConfigEditor Launch ─────────────────────────────────────────────────

## Launches MicrosoftGameConfigEditor.exe with the project's config file.
## Returns the PID, or -1 on failure.
## Escapes special characters for safe use in XML attribute values.
static func _escape_xml_attr(value: String) -> String:
	return value.replace("&", "&amp;").replace('"', "&quot;").replace("<", "&lt;").replace(">", "&gt;")


func launch_editor() -> int:
	var config_path := get_config_path()
	if not FileAccess.file_exists(config_path):
		push_error("[GDK Packaging] MicrosoftGame.config not found — create one first.")
		return -1

	# Ensure placeholder images exist before opening the editor
	_ensure_placeholder_images()

	var args := PackedStringArray([config_path])
	var pid = _toolchain.launch_detached(_toolchain.get_game_config_editor_path(), args)
	if pid >= 0:
		print("[GDK Packaging] Launched GameConfigEditor (PID: ", pid, ")")
	return pid
