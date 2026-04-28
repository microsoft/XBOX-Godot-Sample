@tool
extends EditorExportPlatformExtension
## Xbox/GDK (PC) export platform — packages Godot projects as MSIXVC for Xbox app.

const PLATFORM_NAME := "Xbox GDK (PC)"
const OS_NAME := "Windows"

# GDK tool paths (resolved on init)
var _gdk_root := ""
var _makepkg := ""
var _wdapp := ""
var _gdk_found := false

# Cached sample_config.cfg values (loaded once on init)
var _sample_config := ConfigFile.new()
var _has_sample_config := false

func _initialize() -> void:
	_detect_gdk()
	_has_sample_config = _sample_config.load("res://sample_config.cfg") == OK
	if _has_sample_config:
		print("[GDK Export] Loaded sample_config.cfg — values will auto-populate export fields")

func _get_name() -> String:
	return PLATFORM_NAME

func _get_os_name() -> String:
	return OS_NAME

func _get_logo() -> Texture2D:
	# Create a simple green placeholder icon (16x16)
	var img := Image.create(16, 16, false, Image.FORMAT_RGBA8)
	img.fill(Color(0.0, 0.5, 0.0))  # Xbox green
	return ImageTexture.create_from_image(img)

func _get_binary_extensions(p_preset: EditorExportPreset) -> PackedStringArray:
	return PackedStringArray(["msixvc"])

func _get_platform_features() -> PackedStringArray:
	return PackedStringArray(["pc", "windows", "gdk", "xbox", "d3d12", "x86_64"])

func _get_preset_features(p_preset: EditorExportPreset) -> PackedStringArray:
	return PackedStringArray(["windows", "gdk", "x86_64"])

func _normalize_title_id(p_title_id: String) -> String:
	var normalized: String = p_title_id.strip_edges().to_lower()
	if normalized.begins_with("0x"):
		normalized = normalized.substr(2)
	if normalized == "" or normalized.length() > 8:
		return ""

	for i in range(normalized.length()):
		var digit: String = normalized.substr(i, 1)
		if not "0123456789abcdef".contains(digit):
			return ""

	return normalized.lpad(8, "0")

func _derive_scid_from_title_id(p_title_id: String) -> String:
	var normalized: String = _normalize_title_id(p_title_id)
	if normalized == "":
		return ""
	return "00000000-0000-0000-0000-0000%s" % normalized

func _get_export_options() -> Array[Dictionary]:
	# Load defaults from sample_config.cfg if it exists
	var cfg = ConfigFile.new()
	var has_cfg: bool = cfg.load("res://sample_config.cfg") == OK

	var def_title: String = str(cfg.get_value("xbox_live", "title_id", "")) if has_cfg else ""
	var def_msa: String = str(cfg.get_value("xbox_live", "msa_app_id", "")) if has_cfg else ""
	var def_store: String = str(cfg.get_value("xbox_live", "store_id", "")) if has_cfg else ""
	var def_scid: String = str(cfg.get_value("xbox_live", "scid", "")) if has_cfg else ""
	if def_scid == "":
		def_scid = _derive_scid_from_title_id(def_title)
	var def_sandbox: String = str(cfg.get_value("xbox_live", "sandbox_id", "RETAIL")) if has_cfg else "RETAIL"
	var def_game: String = str(cfg.get_value("identity", "game_name", "My Godot Game")) if has_cfg else "My Godot Game"
	var def_pub: String = str(cfg.get_value("identity", "publisher", "CN=Publisher")) if has_cfg else "CN=Publisher"
	var def_pub_display: String = str(cfg.get_value("identity", "publisher_display_name", "Publisher Name")) if has_cfg else "Publisher Name"
	var def_version: String = str(cfg.get_value("identity", "version", "1.0.0.0")) if has_cfg else "1.0.0.0"

	return [
		# ── Identity ──
		_opt("identity/game_name", TYPE_STRING, def_game,
			PROPERTY_HINT_NONE, "", true),
		_opt("identity/publisher_name", TYPE_STRING, def_pub,
			PROPERTY_HINT_NONE, "", true),
		_opt("identity/publisher_display_name", TYPE_STRING, def_pub_display,
			PROPERTY_HINT_NONE, "", true),
		_opt("identity/version", TYPE_STRING, def_version,
			PROPERTY_HINT_NONE, "", true),

		# ── Xbox Live (optional — requires Partner Center) ──
		_opt("xbox_live/title_id", TYPE_STRING, def_title,
			PROPERTY_HINT_NONE, "Hex Title ID from Partner Center"),
		_opt("xbox_live/msa_app_id", TYPE_STRING, def_msa,
			PROPERTY_HINT_NONE, "MSA App ID from Partner Center"),
		_opt("xbox_live/store_id", TYPE_STRING, def_store,
			PROPERTY_HINT_NONE, "Store ID from Partner Center"),
		_opt("xbox_live/scid", TYPE_STRING, def_scid,
			PROPERTY_HINT_NONE, "Optional override; default derives from Title ID"),

		# ── Packaging ──
		_opt("packaging/content_id", TYPE_STRING, "",
			PROPERTY_HINT_NONE, "Optional content ID override"),
		_opt("packaging/ekb_file", TYPE_STRING, "",
			PROPERTY_HINT_GLOBAL_FILE, "*.ekb",
			false),

		# ── Dev Iteration ──
		_opt("dev/register_loose", TYPE_BOOL, true,
			PROPERTY_HINT_NONE,
			"Use wdapp register for fast iteration instead of full packaging"),
		_opt("dev/sandbox_id", TYPE_STRING, def_sandbox,
			PROPERTY_HINT_NONE, "Xbox Live sandbox ID"),
	]

func _get_export_option_visibility(p_preset: EditorExportPreset, p_option: String) -> bool:
	# Hide packaging options when using loose registration
	if p_option == "packaging/content_id" or p_option == "packaging/ekb_file":
		if p_preset.has("dev/register_loose"):
			return not p_preset.get("dev/register_loose")
	return true

func _get_export_option_warning(p_preset: EditorExportPreset, p_option: StringName) -> String:
	if p_option == "identity/game_name":
		var name = p_preset.get("identity/game_name") if p_preset.has("identity/game_name") else ""
		if name == "" or name == "My Godot Game":
			return "Set a unique game name for your title"

	if p_option == "identity/publisher_name":
		var pub = p_preset.get("identity/publisher_name") if p_preset.has("identity/publisher_name") else ""
		if pub == "" or pub == "CN=Publisher":
			return "Set your publisher identity (CN=YourName)"

	if p_option == "identity/version":
		var ver = p_preset.get("identity/version") if p_preset.has("identity/version") else ""
		if ver != "" and ver.split(".").size() != 4:
			return "Version must be in X.X.X.X format"

	if p_option == "xbox_live/title_id":
		var title_id = p_preset.get("xbox_live/title_id") if p_preset.has("xbox_live/title_id") else ""
		if title_id != "" and _normalize_title_id(str(title_id)) == "":
			return "Title ID must be 1-8 hex characters"

	return ""

func _has_valid_export_configuration(p_preset: EditorExportPreset, p_debug: bool) -> bool:
	if not _gdk_found:
		return false

	# Check required fields
	var name = p_preset.get("identity/game_name") if p_preset.has("identity/game_name") else ""
	var pub = p_preset.get("identity/publisher_name") if p_preset.has("identity/publisher_name") else ""
	if name == "" or pub == "":
		return false

	return true

func _has_valid_project_configuration(p_preset: EditorExportPreset) -> bool:
	return _gdk_found

func _can_export(p_preset: EditorExportPreset, p_debug: bool) -> bool:
	return _has_valid_export_configuration(p_preset, p_debug)

func _export_project(p_preset: EditorExportPreset, p_debug: bool, p_path: String, p_flags: int) -> int:
	if not _gdk_found:
		push_error("GDK not found. Install via: winget install Microsoft.Gaming.GDK")
		return ERR_FILE_NOT_FOUND

	# Resolve output directory
	var out_dir: String = p_path.get_base_dir()
	var staging_dir: String = out_dir.path_join("_gdk_staging")

	print("[GDK Export] Starting export to: ", p_path)
	print("[GDK Export] Staging directory: ", staging_dir)

	# ── Step 1: Create staging directory ──
	var da = DirAccess.open(out_dir)
	if da == null:
		push_error("GDK Export: Cannot access output directory: ", out_dir)
		return ERR_FILE_BAD_PATH

	if da.dir_exists(staging_dir):
		_rmdir_recursive(staging_dir)
	da.make_dir_recursive(staging_dir)

	# ── Step 2: Find Godot export template and export PCK ──
	var exe_name: String = (p_preset.get("identity/game_name").validate_filename() + ".exe") if p_preset.has("identity/game_name") else "game.exe"
	var exe_path: String = staging_dir.path_join(exe_name)
	var pck_path: String = staging_dir.path_join(exe_name.get_basename() + ".pck")

	# Copy Windows export template (or fallback to current Godot exe for dev)
	var template_path: String = _find_windows_template(p_debug)
	if template_path == "":
		# Fallback: use the running Godot executable for dev iteration
		template_path = OS.get_executable_path()
		if template_path == "":
			push_error("GDK Export: No export template found and cannot locate Godot executable. Install export templates via Editor → Manage Export Templates.")
			return ERR_FILE_NOT_FOUND
		push_warning("GDK Export: Using Godot editor binary as template (install export templates for release builds)")

	da.copy(template_path, exe_path)
	print("[GDK Export] Template copied: ", exe_path)

	# Export PCK
	var pck_err: int = _export_pck(p_preset, p_debug, pck_path, p_flags)
	if pck_err != OK:
		push_error("GDK Export: PCK export failed")
		return pck_err

	print("[GDK Export] PCK exported: ", pck_path)

	# ── Step 3: Copy GDK plugin DLL ──
	var plugin_dll: String = _find_plugin_dll(p_debug)
	if plugin_dll != "":
		var addon_dir: String = staging_dir.path_join("addons").path_join("godot_gdk").path_join("bin")
		da.make_dir_recursive(addon_dir)
		da.copy(plugin_dll, addon_dir.path_join(plugin_dll.get_file()))
		print("[GDK Export] Plugin DLL copied")

	# ── Step 4: Generate MicrosoftGame.config ──
	var config_path: String = staging_dir.path_join("MicrosoftGame.config")
	var config_content: String = _generate_microsoft_game_config(p_preset, exe_name)
	var config_file = FileAccess.open(config_path, FileAccess.WRITE)
	if config_file == null:
		push_error("GDK Export: Cannot write MicrosoftGame.config")
		return ERR_FILE_CANT_WRITE
	config_file.store_string(config_content)
	config_file.close()
	print("[GDK Export] MicrosoftGame.config generated")

	# ── Step 5: Package or register ──
	var use_loose = p_preset.get("dev/register_loose") if p_preset.has("dev/register_loose") else true

	if use_loose:
		return _wdapp_register(staging_dir)
	else:
		return _makepkg_pack(staging_dir, p_path, p_preset)

# ── MicrosoftGame.config generation ─────────────────────────────

## Read a preset value, falling back to sample_config.cfg if the preset value is empty.
func _preset_or_config(p_preset: EditorExportPreset, preset_key: String,
		config_section: String, config_key: String, fallback: String = "") -> String:
	var val: String = p_preset.get(preset_key) if p_preset.has(preset_key) else ""
	if val == "" and _has_sample_config:
		val = str(_sample_config.get_value(config_section, config_key, ""))
	if val == "":
		val = fallback
	return val

func _generate_microsoft_game_config(p_preset: EditorExportPreset, exe_name: String) -> String:
	var game_name: String = _preset_or_config(p_preset, "identity/game_name", "identity", "game_name", "MyGodotGame")
	var publisher: String = _preset_or_config(p_preset, "identity/publisher_name", "identity", "publisher", "CN=Publisher")
	var pub_display: String = _preset_or_config(p_preset, "identity/publisher_display_name", "identity", "publisher_display_name", "Publisher")
	var version: String = _preset_or_config(p_preset, "identity/version", "identity", "version", "1.0.0.0")
	var title_id: String = _normalize_title_id(_preset_or_config(p_preset, "xbox_live/title_id", "xbox_live", "title_id"))
	var msa_app_id: String = _preset_or_config(p_preset, "xbox_live/msa_app_id", "xbox_live", "msa_app_id")
	var store_id: String = _preset_or_config(p_preset, "xbox_live/store_id", "xbox_live", "store_id")

	# Clean game name for identity (alphanumeric + dots only)
	var identity_name: String = game_name.replace(" ", "").replace("-", "")

	var xml: String = '<?xml version="1.0" encoding="utf-8"?>\n'
	xml += '<Game configVersion="1">\n'
	xml += '  <Identity Name="%s"\n' % identity_name
	xml += '            Publisher="%s"\n' % publisher
	xml += '            Version="%s" />\n' % version
	xml += '\n'

	if title_id != "":
		xml += '  <TitleId>%s</TitleId>\n' % title_id

	if msa_app_id != "":
		xml += '  <MSAAppId>%s</MSAAppId>\n' % msa_app_id

	if store_id != "":
		xml += '  <StoreId>%s</StoreId>\n' % store_id

	xml += '\n'
	xml += '  <ExecutableList>\n'
	xml += '    <Executable Name="%s"\n' % exe_name
	xml += '               Id="Game"\n'
	xml += '               IsDevOnly="false" />\n'
	xml += '  </ExecutableList>\n'
	xml += '\n'
	xml += '  <ShellVisuals DefaultDisplayName="%s"\n' % game_name
	xml += '                PublisherDisplayName="%s"\n' % pub_display
	xml += '                StoreLogo="StoreLogo.png"\n'
	xml += '                Square150x150Logo="Logo150.png"\n'
	xml += '                Square44x44Logo="Logo44.png"\n'
	xml += '                Square480x480Logo="Logo480.png"\n'
	xml += '                SplashScreenImage="SplashScreen.png"\n'
	xml += '                ForegroundText="light"\n'
	xml += '                BackgroundColor="#1a1a2e" />\n'
	xml += '\n'
	xml += '  <DesktopRegistration>\n'
	xml += '    <DependencyList>\n'
	xml += '      <KnownDependency Name="VC14" />\n'
	xml += '    </DependencyList>\n'
	xml += '  </DesktopRegistration>\n'
	xml += '</Game>\n'

	return xml

# ── Tool execution ──────────────────────────────────────────────

func _wdapp_register(staging_dir: String) -> int:
	print("[GDK Export] Registering loose folder via wdapp...")
	var global_path: String = ProjectSettings.globalize_path(staging_dir)
	var output: Array = []
	var exit_code: int = OS.execute(_wdapp, ["register", global_path], output, true)
	for line in output:
		print("[wdapp] ", line)
	if exit_code != 0:
		push_error("GDK Export: wdapp register failed (exit code %d)" % exit_code)
		return ERR_BUG
	print("[GDK Export] Registered successfully! Launch from Xbox app or Start menu.")
	return OK

func _makepkg_pack(staging_dir: String, output_path: String, p_preset: EditorExportPreset) -> int:
	var global_staging: String = ProjectSettings.globalize_path(staging_dir)
	var global_output: String = ProjectSettings.globalize_path(output_path)

	# Step 1: genmap
	print("[GDK Export] Generating file map...")
	var layout_path: String = global_staging + "\\layout.xml"
	var output1: Array = []
	var exit1: int = OS.execute(_makepkg, [
		"genmap", "/f", layout_path, "/d", global_staging
	], output1, true)
	for line in output1:
		print("[makepkg] ", line)
	if exit1 != 0:
		push_error("GDK Export: makepkg genmap failed")
		return ERR_BUG

	# Step 2: pack
	print("[GDK Export] Packing MSIXVC...")
	var output2: Array = []
	var pack_args: Array = [
		"pack", "/f", layout_path, "/d", global_staging, "/pd", global_output.get_base_dir()
	]

	# Add EKB file if provided
	var ekb = p_preset.get("packaging/ekb_file") if p_preset.has("packaging/ekb_file") else ""
	if ekb != "":
		pack_args.append_array(["/lk", ProjectSettings.globalize_path(ekb)])

	var exit2 := OS.execute(_makepkg, pack_args, output2, true)
	for line in output2:
		print("[makepkg] ", line)
	if exit2 != 0:
		push_error("GDK Export: makepkg pack failed")
		return ERR_BUG

	print("[GDK Export] Package created: ", global_output)
	return OK

# ── Helpers ─────────────────────────────────────────────────────

func _detect_gdk() -> void:
	var base: String = "C:\\Program Files (x86)\\Microsoft GDK"
	var edition_roots: Array[String] = []

	var env_roots: Array[String] = [OS.get_environment("GameDKCoreLatest"), OS.get_environment("GameDKLatest")]
	for raw_root in env_roots:
		if raw_root == "":
			continue
		var normalized_root: String = raw_root.trim_suffix("\\").trim_suffix("/")
		if not edition_roots.has(normalized_root):
			edition_roots.append(normalized_root)

	if DirAccess.dir_exists_absolute(base):
		var da = DirAccess.open(base)
		if da == null:
			_gdk_found = false
			return

		var editions: Array[String] = []
		da.list_dir_begin()
		var entry: String = da.get_next()
		while entry != "":
			if da.current_is_dir() and entry[0].is_valid_int():
				editions.append(entry)
			entry = da.get_next()
		da.list_dir_end()

		editions.sort()
		if not editions.is_empty():
			var latest_root: String = base + "\\" + editions[-1]
			if not edition_roots.has(latest_root):
				edition_roots.append(latest_root)

	if edition_roots.is_empty():
		push_warning("GDK Export: Microsoft GDK not found at ", base)
		_gdk_found = false
		return

	for root in edition_roots:
		if DirAccess.dir_exists_absolute(root + "\\windows"):
			_gdk_root = root
			break

	if _gdk_root == "":
		push_warning("GDK Export: No Windows-layout GDK installation was found")
		_gdk_found = false
		return

	var tools_root: String = _gdk_root.get_base_dir()
	_makepkg = tools_root + "\\bin\\makepkg.exe"
	_wdapp = tools_root + "\\bin\\wdapp.exe"

	if FileAccess.file_exists(_makepkg) and FileAccess.file_exists(_wdapp):
		_gdk_found = true
		print("[GDK Export] GDK found: ", _gdk_root)
		print("[GDK Export] Windows layout: ", _gdk_root + "\\windows")
		print("[GDK Export] makepkg: ", _makepkg)
		print("[GDK Export] wdapp: ", _wdapp)
	else:
		push_warning("GDK Export: GDK tools not found")
		_gdk_found = false

func _find_windows_template(p_debug: bool) -> String:
	var app_data: String = OS.get_environment("APPDATA")
	var templates_dir: String = app_data.path_join("Godot").path_join("export_templates")
	var ver: Dictionary = Engine.get_version_info()
	var ver_string: String = "%d.%d.%s" % [ver["major"], ver["minor"], ver["status"]]
	var suffix: String = "debug" if p_debug else "release"
	var template: String = templates_dir.path_join(ver_string).path_join("windows_%s_x86_64.exe" % suffix)
	if FileAccess.file_exists(template):
		return template
	# Fallback: try without status
	var ver_string2: String = "%d.%d" % [ver["major"], ver["minor"]]
	template = templates_dir.path_join(ver_string2).path_join("windows_%s_x86_64.exe" % suffix)
	if FileAccess.file_exists(template):
		return template
	return ""

func _find_plugin_dll(p_debug: bool) -> String:
	var suffix: String = "debug" if p_debug else "release"
	var dll_name: String = "godot_gdk.windows.%s.x86_64.dll" % suffix
	var path: String = "res://addons/godot_gdk/bin/" + dll_name
	if FileAccess.file_exists(path):
		return path
	return ""

func _export_pck(p_preset: EditorExportPreset, p_debug: bool, p_path: String, p_flags: int) -> int:
	return export_pack(p_preset, p_debug, p_path, p_flags)

func _rmdir_recursive(path: String) -> void:
	var da = DirAccess.open(path)
	if da == null:
		return
	da.list_dir_begin()
	var entry: String = da.get_next()
	while entry != "":
		var full: String = path.path_join(entry)
		if da.current_is_dir():
			_rmdir_recursive(full)
		else:
			da.remove(entry)
		entry = da.get_next()
	da.list_dir_end()
	DirAccess.remove_absolute(path)

# Export option builder helper — flat dict format for EditorExportPlatformExtension
func _opt(p_name: String, type: int, default_value = null,
		hint: int = PROPERTY_HINT_NONE, hint_string: String = "",
		required: bool = false) -> Dictionary:
	var d := {
		"name": p_name,
		"type": type,
		"hint": hint,
		"hint_string": hint_string,
		"default_value": default_value,
		"update_visibility": p_name.begins_with("dev/"),
	}
	if required:
		d["required"] = true
	return d
